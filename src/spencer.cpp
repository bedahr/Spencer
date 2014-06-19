#include "spencer.h"
#include "databaseconnector.h"
#include "nlu/nlu.h"
#include "nlu/statement.h"
#include "recommender/critiquerecommender.h"
#include "domainbase/offer.h"
#include "domainbase/numericalattribute.h"
#include "domainbase/relationship.h"
#include "domainbase/attributefactory.h"
#include "domainbase/stringattribute.h"
#include "dialogmanager/dialogmanager.h"
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
    m_databaseConnector(new DatabaseConnector),
    m_attributeFactory(new AttributeFactory),
    m_nlu(new NLU(m_attributeFactory)),
    m_dialogManager(new DialogManager),
    m_recommender(new CritiqueRecommender),
    m_currentRecommendation(0)
{
    connect(m_dialogManager, SIGNAL(recommendation(const Offer*, QString)), this, SLOT(recommendationChanged(const Offer*, QString)));
    connect(m_dialogManager, SIGNAL(elicit(AvatarTask, bool)), this, SIGNAL(elicit(AvatarTask, bool)));
}

Spencer::~Spencer()
{
    delete m_recommender;
}

bool Spencer::init()
{
    if (!m_attributeFactory->parseStructure(dbPath + "structure.xml"))
        return false;
    if (!m_nlu->setupLanguage(dbPath + "nlp.xml"))
        return false;

    if (!m_databaseConnector->init())
        return false;

    bool okay;
    QList<Offer*> availableOffers = parseCasebase(&okay);
    if (!okay)
        return false;

    m_recommender->setupDatabase(availableOffers);
    m_recommender->init();

    m_dialogManager->init(m_recommender);
    return true;
}

void Spencer::userInput(const QString& input)
{
    // NLU
    QList<Statement*> statements = m_nlu->interpret(m_currentRecommendation, input);

    // Dialog Manager
    m_dialogManager->userInput(statements);
}


QList<Offer*> Spencer::parseCasebase(bool* okay) const
{
    return m_databaseConnector->loadOffers(m_attributeFactory, okay);
    /*
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

        QString imageSrc = imageBasePath+attributes.value("Bild")->toString();
        float priorProbability = 0;
        //add prior probability to top 100
        if (attributes.contains("Rang"))
            priorProbability = 0.005 * (1 - (attributes.value("Rang").staticCast<NumericalAttribute>())->getValue() / 100);

        availableOffers << new Offer(name, priorProbability, QStringList() << imageSrc, m_attributeFactory->getAttributeNames(), attributes);
        caseElement = caseElement.nextSiblingElement("case");
    }

    *okay = true;
    return availableOffers;
    */
}

void Spencer::reset()
{
    m_recommender->init();
}

void Spencer::recommendationChanged(const Offer *o, const QString& explanation)
{
    qDebug() << "Recommendation changed!";
    m_currentRecommendation = o;
    emit recommend(o, explanation);
}
