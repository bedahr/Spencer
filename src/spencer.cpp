#include "spencer.h"
#include "nlu/nlu.h"
#include "nlu/statement.h"
#include "recommender/critiquerecommender.h"
#include "domainbase/offer.h"
#include "domainbase/numericalattribute.h"
#include "domainbase/relationship.h"
#include "domainbase/attributefactory.h"
#include "domainbase/stringattribute.h"
#include <QHash>
#include <QFile>
#include <QSet>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QDebug>
#include <QRegExp>
#include <QFlags>

static const QString dbPath = "app/native/res/db/";
static const QString modelName = "Modell";
static const QString manufacturerName = "Hersteller";
static const QString imageName = "Bild";

Spencer::Spencer(QObject *parent) :
    QObject(parent),
    m_attributeFactory(new AttributeFactory),
    m_nlu(new NLU(m_attributeFactory)),
    m_recommender(new CritiqueRecommender),
    m_currentRecommendation(0)
{
    connect(m_recommender, SIGNAL(recommend(const Offer*, QString)), this, SLOT(recommendationChanged(const Offer*, QString)));
}

Spencer::~Spencer()
{
    delete m_recommender;
}

QList<Offer*> Spencer::parseCasebase(const QString& path, const QString& imageBasePath, bool* okay) const
{
    QList<Offer*> availableOffers;
    *okay = false;

    QDomDocument doc;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file: " << path;
        return availableOffers;
    }
    if (!doc.setContent(f.readAll())) {
        qWarning() << "Failed to parse XML at " << path;
        return availableOffers;
    }

    QDomElement rootElement(doc.documentElement());
    QDomElement caseElement(rootElement.firstChildElement().firstChildElement("case"));

    //we need unique, descriptive names (user visible);
    // make sure we don't re-use them
    QSet<QString> uniqueNames;

    while (!caseElement.isNull()) {
        QHash<QString, QSharedPointer<Attribute> > attributes;
        QDomElement featureElement = caseElement.firstChildElement("feature");
        while (!featureElement.isNull()) {
            QDomElement nameElem = featureElement.firstChildElement("name");
            QDomElement valueElem = featureElement.firstChildElement("value");

            QString name = nameElem.text();
            QString value = valueElem.text();
            //qDebug() << "Name: " << name << " value: " << value;

            QSharedPointer<Attribute> a = m_attributeFactory->getAttribute(name, value);
            if (!a) {
                qDebug() << "Attribute failed to parse" << name << value;
                return availableOffers;
            } else
                attributes.insert(name, a);
            featureElement = featureElement.nextSiblingElement("feature");
        }

        QString name;
        if (attributes.contains(modelName) && attributes.contains(manufacturerName))
            name = attributes.value(manufacturerName)->toString() + " " + attributes.value(modelName)->toString();
        else
            name = caseElement.attribute("id");

        name = name.trimmed();
        while (uniqueNames.contains(name)) {
            qDebug() << "Duplicate: " << name;
            name += '_';
        }
        uniqueNames.insert(name);

        QPixmap image(imageBasePath+attributes.value("Bild")->toString());
        float priorProbability = 0;
        //add prior probability to top 100
        if (attributes.contains("Rang"))
            priorProbability = 0.005 * (1 - (attributes.value("Rang").staticCast<NumericalAttribute>())->getValue() / 100);

        availableOffers << new Offer(name, priorProbability, image, m_attributeFactory->getAttributeNames(), attributes);
        caseElement = caseElement.nextSiblingElement("case");
    }

    *okay = true;
    return availableOffers;
}

bool Spencer::init()
{
    if (!m_attributeFactory->parseStructure(dbPath + "structure.xml"))
        return false;
    if (!m_nlu->setupLanguage(dbPath + "nlp.xml"))
        return false;

    bool okay;
    QList<Offer*> availableOffers = parseCasebase(dbPath + "casebase.xml",
                                                  dbPath + "img/", &okay);
    if (!okay)
        return false;

    m_recommender->setupDatabase(availableOffers);
    m_recommender->init();
    return true;
}

void Spencer::reset()
{
    m_recommender->init();
}

QString Spencer::critique(const QString& command)
{
    QString extractedCommands;

    QList<Statement*> statements = m_nlu->interpret(m_currentRecommendation, command);


    static int misunderstandingCounter = 0;
    qDebug() << "Misunderstanding counter: " << misunderstandingCounter;
    if (!statements.isEmpty())
        misunderstandingCounter = 0;
    else ++misunderstandingCounter;

    if (statements.isEmpty()) {
        extractedCommands = tr("Wie bitte?");
        emit recommendationChanged(m_currentRecommendation, extractedCommands);
    } else {
        foreach (Statement *s, statements) {
            if (!s->act(m_recommender)) {
                emit noMatchFor(s->toString());
            }
            extractedCommands += s->toString() + '\n';
        }
        m_recommender->feedbackCycleComplete();
    }

    //based on the assumption that the user wouldn't have critizised the
    // model if he was interested in it, add a *slight* bias against it
    //m_recommender->critique(new Critique(new Relationship(modelName,
    //               Relationship::Inequality, m_currentRecommendation->getAttribute(modelName)), 0.1));

    return extractedCommands;
}

void Spencer::recommendationChanged(const Offer *o, const QString& explanation)
{
    m_currentRecommendation = o;
    emit recommend(o, explanation);
}
