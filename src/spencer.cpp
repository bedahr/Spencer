#include "spencer.h"
#include "databaseconnector.h"
#include "domainbase/aspectfactory.h"
#include "nlu/nlu.h"
#include "nlu/statement.h"
#include "recommender/critiquerecommender.h"
#include "domainbase/offer.h"
#include "domainbase/numericalattribute.h"
#include "domainbase/relationship.h"
#include "domainbase/attributefactory.h"
#include "domainbase/stringattribute.h"
#include "dialogmanager/simpledialogmanager.h"
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
    m_nlu(new NLU),
    m_dialogManager(new SimpleDialogManager),
    m_recommender(new CritiqueRecommender),
    m_currentRecommendation(0)
{
    connect(m_dialogManager, SIGNAL(recommendation(const Offer*, QString, double, double, QStringList,
                                                   QList<RecommendationAttribute*>, SentimentMap, QString)),
            this, SLOT(recommendationChanged(const Offer*, QString, double, double, QStringList,
                                             QList<RecommendationAttribute*>, SentimentMap, QString)));
    connect(m_dialogManager, SIGNAL(elicit(AvatarTask, bool)), this, SIGNAL(elicit(AvatarTask, bool)));
}

Spencer::~Spencer()
{
    delete m_recommender;
}

bool Spencer::init()
{
    if (!AttributeFactory::getInstance()->parseStructure(dbPath + "structure.xml"))
        return false;
    if (!AspectFactory::getInstance()->parseStructure(dbPath + "sentiment.xml"))
        return false;
    if (!m_nlu->setupLanguage(dbPath + "nlp.xml", dbPath + "synsets.hierarchical"))
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
    return m_databaseConnector->loadOffers(okay);
}

void Spencer::reset()
{
    qDebug() << "Resetting";
    m_recommender->init();
    m_dialogManager->init(m_recommender);
}

void Spencer::recommendationChanged(const Offer *currentOffer,
                                    const QString& offerName, double price, double rating,
                                    const QStringList& images,
                                    const QList<RecommendationAttribute*> &offer,
                                    SentimentMap userSentiment,
                                    const QString& explanation)
{
    qDebug() << "Recommendation changed!";
    m_currentRecommendation = currentOffer;
    emit recommend(offerName, price, rating, images, offer, userSentiment, explanation);
}

void Spencer::overwriteDialogStrategy(int code)
{
    switch (code)
    {
    case 1:
        m_dialogManager->askForUseCase();
        break;
    case 2:
        m_dialogManager->askForMostImportantAttribute();
        break;
    case 3:
        m_dialogManager->askForPerformanceImportant();
        break;
    case 4:
        m_dialogManager->askForPriceImportant();
        break;
    case 5:
        m_dialogManager->askForPortabilityImportant();
        break;
    case 6:
        m_dialogManager->randomRecommendation();
        break;
    }
}
