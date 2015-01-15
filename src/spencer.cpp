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
#include "logger/logger.h"
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
    delete m_databaseConnector;
    delete m_dialogManager;
    delete m_nlu;
}

bool Spencer::init()
{
    if (!Logger::init())
        return false;
    if (!AttributeFactory::getInstance()->parseStructure(dbPath + "structure.xml"))
        return false;
    if (!AspectFactory::getInstance()->parseStructure(dbPath + "sentiment.xml"))
        return false;

    if (!m_databaseConnector->init())
        return false;

    bool okay;
    QList<Offer*> availableOffers = parseCasebase(&okay);
    if (!okay)
        return false;

    m_recommender->setupDatabase(availableOffers);
    m_recommender->init();

    if (!m_nlu->setupLanguage(dbPath + "nlp.xml", dbPath + "synsets.hierarchical"))
        return false;

    m_dialogManager->init(m_recommender);

    Logger::log("Successfully initialized Spencer");
#ifdef SPENCER_UNNUANCED
    Logger::log("Nuances ignored");
#else
    Logger::log("Using full Spencer capabilities");
#endif
    return true;
}

void Spencer::userInput(const RecognitionResultList& input)
{
    if (input.empty()) {
        Logger::log("Received empty ASR result");
        m_dialogManager->userInput(QList<Statement*>());
        return;
    }

    RecognitionResult asrResult = input.first();

    // NLU
    QList<Statement*> statements = m_nlu->interpret(m_currentRecommendation, asrResult.sentence());

    float arousal = 1.0 + asrResult.arousal();
    qDebug() << "Adjusting statement importance based on arousal: " << arousal;
    Logger::log("Adjusting polarity based on arousal:" + QString::number(arousal));
    foreach (Statement *s, statements)
        s->adjustImportance(arousal);

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

void Spencer::listening()
{
    m_dialogManager->userIsTalking();
}

void Spencer::recognizing()
{
    m_dialogManager->userFinishedTalking();
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
