#include "simpledialogmanager.h"
#include "nlu/statement.h"
#include "domainbase/attributefactory.h"
#include "recommender/critiquerecommender.h"
#include "recommender/recommendation.h"
#include "logger/logger.h"
#include <QDebug>
#include <QEventLoop>

// after we received at least one actionable statement, we wait this long
// for the user to speak again, otherwise we act on what we got.
static const int turnTimeoutAfterStatements = 1400;

// if we receive input that doesn't contain any useful statement we (initially) wait
// for a pause this long before we act (likely by taking initiative)
// This pause will decrease after every non-statement utterance from the user down to
// a minimum of tunTimeoutAfterStatements
static const int turnTimeoutWithoutStatements = 5000;
static const int turnTimeoutWithoutStatementsDecay = 2000;

static const QStringList performanceAttributes(QStringList() << "processorSpeed");
static const QStringList performanceAspects(QStringList() << "Speed");

static const QStringList priceAttributes(QStringList() << "price");
static const QStringList priceAspects(QStringList() << "Price");

static const QStringList portabilityAttributes(QStringList() << "screenSize" << "averageRuntimeOnBattery");
static const QStringList portabilityAspects(QStringList() << "Portability");


SimpleDialogManager::SimpleDialogManager() :
    userCurrentlyTalking(false),
    state(DialogStrategy::NullState), upcomingState(DialogStrategy::InitState),
    previousState(DialogStrategy::InitState), recommender(0),
    currentOffer(0), oldOffer(0), acceptedStatementsOfThisTurn(0),
    consecutiveMisunderstandingCounter(0),
    absoluteMisunderstandingCounter(0),
    allAskedDomainQuestion(0),
    lastAskedDomainQuestion(DialogStrategy::InitState)
{
    connect(&turnCompletionTimer, SIGNAL(timeout()), this, SLOT(completeTurn()));
    connect(&failedRecognitionTimer, SIGNAL(timeout()), this, SLOT(completeTurn()));
    turnCompletionTimer.setSingleShot(true);
    failedRecognitionTimer.setSingleShot(true);
}

void SimpleDialogManager::userIsTalking()
{
    userCurrentlyTalking = true;
    turnCompletionTimer.stop();
    failedRecognitionTimer.stop();
}

void SimpleDialogManager::userFinishedTalking()
{
    userCurrentlyTalking = false;
    failedRecognitionTimer.start(3000);
    //turnCompletionTimer.start();
}

void SimpleDialogManager::userInput(const QList<Statement*> statements)
{
    failedRecognitionTimer.stop();
    qDebug() << "Got statements: " << statements.count();
    int acceptedStatements = 0;
    foreach (Statement *s, statements) {
        if (!s->act(state, this, currentOffer)) {
            qWarning() << "Failed to act on statement " << s->toString();
            //qWarning() << "No match for " << s->toString();
            //emit elicit(AvatarTask(AvatarTask::Custom, tr("Leider konnte ich kein passendes Produkt finden mit %1").arg(s->toString()),
            //                       tr("Kein passendes Produkt.")));
        } else {
            // FIXME: explanation!
            qDebug() << "Processed: " << s->toString();
            ++acceptedStatements;
        }
    }

    if (acceptedStatements > 0) {
        turnCompletionTimer.setInterval(turnTimeoutAfterStatements);
    } else {
        turnCompletionTimer.setInterval(qMax(turnTimeoutAfterStatements,
                                             turnCompletionTimer.interval() - turnTimeoutWithoutStatementsDecay));
    }
    acceptedStatementsOfThisTurn += acceptedStatements;
    if (!userCurrentlyTalking)
        turnCompletionTimer.start();
}

void SimpleDialogManager::processRecommendation(Recommendation* r)
{
    oldOffer = currentOffer;
    currentOffer = r->offer();
    QString explanation = "Explanations not implemented yet."; // TODO

    const Offer *o = r->offer();
    QList<RecommendationAttribute*> description;
    Logger::log("New recommendation: " + o->getId());
    foreach (const QString& key, AttributeFactory::getInstance()->getAttributeIds()) {
        Record r(o->getRecord(key));
        if (!r.second)
            continue;

        if (r.second->getInternal())
            continue;
        QString name = r.first;
        QSharedPointer<Attribute> attr = r.second;
        float expressedUserInterest = recommender->userInterest(key);

        bool showThisAttribute = true;

        // there are two reasons that warrant showing the attribute:
        // 1. A certain set of attributes is shown by default.
        showThisAttribute = attr->getShownByDefault();

        // 2. The user expressed interest in the attribute either directly or indirectly
        showThisAttribute |= expressedUserInterest > 0.1;
        float sentiment = 0;
        float completionFactor = recommender->completionFactor(key, *o);
        QString value = attr->toString();
        qDebug() << "User interest for attribute " << r.first << " (" << key << " = " <<
                    value << ")" << expressedUserInterest << " showing: " << showThisAttribute;

        Logger::log("  " + r.first + " (\"" + key + "\") = \"" + value + "\"; user interest: " +
                    QString::number(expressedUserInterest) + " show: " + (showThisAttribute ? "yes" : "no") +
                    " completion factor: " + QString::number(completionFactor));

        if (!showThisAttribute)
            continue;

        qDebug() << "Complection factor for attribute " << r.first << completionFactor;

        description << new RecommendationAttribute(name, value, expressedUserInterest, completionFactor, sentiment);
    }
    emit recommendation(o, o->getName(), o->getPrice(), o->getRating(), o->getImages(),
                        description, o->getUserSentiment(), explanation);
    queueState(DialogStrategy::Recommendation);
}

void SimpleDialogManager::completeTurn()
{
    qDebug() << "Completing turn";
    failedRecognitionTimer.stop();

    if (state != DialogStrategy::InitState && acceptedStatementsOfThisTurn == 0) {
        //qDebug() << "Misunderstanding counter: " << consecutiveMisunderstandingCounter;
        ++consecutiveMisunderstandingCounter;
        ++absoluteMisunderstandingCounter;
    } else {
        consecutiveMisunderstandingCounter = 0;
        if ((upcomingState == DialogStrategy::NullState) || (upcomingState == DialogStrategy::Recommendation)) {
            Recommendation* r = 0;
            //we don't have any plans yet, default to Recommedation
            recommender->feedbackCycleComplete();
            r = recommender->suggestOffer();

            if (r) {
                processRecommendation(r);
            } else {
                qDebug() << "No recommendation at this point";
                if (upcomingState == DialogStrategy::Recommendation)
                    upcomingState = DialogStrategy::NullState;
            }

            delete r;
        }
    }

    qDebug() << "Current state " << state << " upcoming state " << upcomingState;
    // (still) no plan?
    if (upcomingState == DialogStrategy::NullState) {
        switch (consecutiveMisunderstandingCounter) {
        case 0:
            // no misunderstanding but no recommendation -> ask domain question
            askDomainQuestion();
            break;
        case 1:
            //if we have too many misunderstandings in one session, just take initiative
            if (absoluteMisunderstandingCounter < 3) {
                queueState(DialogStrategy::MisunderstoodInput);
                break;
            }
        default:
            // too many misunderstandings -> ask domain question
            askDomainQuestion();
            break;
        }
    }

    enterState();
    turnCompletionTimer.setInterval(turnTimeoutWithoutStatements);
    acceptedStatementsOfThisTurn = 0;
}

void SimpleDialogManager::enterState()
{
    previousState = state;
    state = upcomingState;
    Logger::log("Changed state from " + DialogStrategy::dialogStrategyString(previousState)
                + " to " + DialogStrategy::dialogStrategyString(state));
    upcomingState = DialogStrategy::NullState;
    switch (state) {
    case DialogStrategy::NullState:
        qFatal("Attempted to enter null state");
        break;
    case DialogStrategy::InitState:
        greet();
        break;

    case DialogStrategy::AskForUseCase:
        askForUseCase();
        break;

    case DialogStrategy::AskForImportantAttribute:
        askForMostImportantAttribute();
        break;

    case DialogStrategy::AskForPerformanceImportant:
        askForPerformanceImportant();
        break;

    case DialogStrategy::AskForPriceImportant:
        askForPriceImportant();
        break;

    case DialogStrategy::AskForPortabilityImportant:
        askForPortabilityImportant();
        break;

    case DialogStrategy::Recommendation:
        if (currentOffer == oldOffer) {
            emit elicit(AvatarTask(AvatarTask::Custom, tr("Ich denke noch immer, dass dieses Produkt am besten "
                                                          "zu Ihren Anforderungen passen würde."), tr("Empfehlung gleich geblieben")), false);
        } else
            emit elicit(AvatarTask(AvatarTask::PresentItem), false);
        break;

    case DialogStrategy::MisunderstoodInput:
        emit elicit(AvatarTask(AvatarTask::Misunderstood));
        state = previousState;
        break;

    case DialogStrategy::AskForHelp:
        emit elicit(AvatarTask(AvatarTask::Custom, tr("Sie können mit mir gleich sprechen, wie mit einem menschlichen Verkäufer. "
                                                      "Sollte ich einmal etwas nicht auf Anhieb verstehen, so versuchen Sie es bitte "
                                                      "Ihren in anderen Worten auszudrücken. Viel Spaß!"), tr("Hilfestellung")));
        break;

    case DialogStrategy::FinalState:
        emit elicit(AvatarTask(AvatarTask::Custom, tr("Sollten Sie dieses Produkt auswählen wollen, sagen Sie dies bitte dem Studienbetreuer. "
                                                      "Ansonsten können Sie sich natürlich auch gerne noch weiter umsehen."), tr("Produkt akzeptieren?")));
        break;
    }
}

void SimpleDialogManager::init(CritiqueRecommender *recommender)
{
    this->recommender = recommender;
    previousState = state;
    state = DialogStrategy::NullState;
    upcomingState = DialogStrategy::InitState;
    lastAskedDomainQuestion = DialogStrategy::InitState;
    allAskedDomainQuestion = 0;
    consecutiveMisunderstandingCounter = 0;
    absoluteMisunderstandingCounter = 0;
    enterState();
}

void SimpleDialogManager::greet()
{
    emit elicit(AvatarTask(AvatarTask::Intro), true);
}

void SimpleDialogManager::askForUseCase()
{
    emit elicit(AvatarTask(AvatarTask::AskUseCase));
}

void SimpleDialogManager::askForMostImportantAttribute()
{
    emit elicit(AvatarTask(AvatarTask::AskMostImportantAttribute));
}

void SimpleDialogManager::askForPerformanceImportant()
{
    emit elicit(AvatarTask(AvatarTask::AskPerformanceImportant));
}

void SimpleDialogManager::askForPriceImportant()
{
    emit elicit(AvatarTask(AvatarTask::AskPriceImporant));
}

void SimpleDialogManager::askForPortabilityImportant()
{
    emit elicit(AvatarTask(AvatarTask::AskEasyTransportImportant));
}

void SimpleDialogManager::askDomainQuestion()
{
    // select the domain question that:
    //  a. we expect to prompt the user for the "most valuable" information
    //  b. we haven't asked before
    //  c. was not the last asked domain question
    QMap<double, DialogStrategy::DialogState> plan;

    //QList<DialogStrategy::DialogState> plan;
    double userModelRichness = 100*recommender->userModelRichness();
    qDebug() << "user model richness: " << userModelRichness;
    plan.insertMulti(userModelRichness /* bias */, DialogStrategy::AskForUseCase);
    plan.insertMulti(userModelRichness + 1 /* bias */, DialogStrategy::AskForImportantAttribute);

    double performanceCoverage = 100*recommender->assertUsefulness(performanceAttributes, performanceAspects);
    plan.insertMulti(100 - performanceCoverage + 2 /* bias */, DialogStrategy::AskForPerformanceImportant);
    double priceCoverage = 100*recommender->assertUsefulness(priceAttributes, priceAspects);
    plan.insertMulti(100 - priceCoverage + 2 /* bias */, DialogStrategy::AskForPriceImportant);
    double portabilityCoverage = 100*recommender->assertUsefulness(portabilityAttributes, portabilityAspects);
    plan.insertMulti(100 - portabilityCoverage + 2 /* bias */, DialogStrategy::AskForPortabilityImportant);

    for (int round = 0; round < 2; ++round) {
        // 2 rounds. At the first one we skip everything we asked before
        // if we can't find anything, ask whatever is necessary in the second round
        foreach (double score, plan.keys()) {
            QList<DialogStrategy::DialogState> equivalentlyScored(plan.values(score));

            qDebug() << equivalentlyScored.count() << " decisions at score " << score;
            foreach (DialogStrategy::DialogState s, equivalentlyScored) {
                qDebug() << " " << s;
                if ((s == lastAskedDomainQuestion) ||
                        ((round == 0) && (allAskedDomainQuestion & s)))
                    continue;
                qDebug() << " asking:" << s;
                lastAskedDomainQuestion = s;
                allAskedDomainQuestion |= s;
                queueState(s);
                return;
            }
        }
    }
    // we should never hit this
    Q_ASSERT(false);
}

void SimpleDialogManager::randomRecommendation()
{
    completeTurn();
}

bool SimpleDialogManager::undo()
{
    recommender->undo();
    return true;
}

bool SimpleDialogManager::constrain(Critique* c)
{
    queueState(DialogStrategy::Recommendation);
    return recommender->critique(c);
}

bool SimpleDialogManager::applyAspect(MentionedAspect* a)
{
    queueState(DialogStrategy::Recommendation);
    return recommender->applyAspect(a);
}
bool SimpleDialogManager::accept(double strength)
{
    // TODO: experiment with cutoff
    qDebug() << "accept Strength: " << strength;
    if (currentOffer && (strength >= 0.8) &&
            (upcomingState == DialogStrategy::NullState)) {
        queueState(DialogStrategy::FinalState);
        return true;
    }
    return false;
}

bool SimpleDialogManager::requestForHelp(double strength)
{
    if (strength > 0.2) {
        queueState(DialogStrategy::AskForHelp);
        return true;
    }
    return false;
}

void SimpleDialogManager::queueState(DialogStrategy::DialogState newState)
{
    if (upcomingState != state) {
        qWarning() << "Overwriting previously scheduled state: " << upcomingState;
    }
    upcomingState = newState;
}
