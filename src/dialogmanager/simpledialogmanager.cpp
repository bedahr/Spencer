#include "simpledialogmanager.h"
#include "nlu/statement.h"
#include "domainbase/attributefactory.h"
#include "recommender/critiquerecommender.h"
#include "recommender/recommendation.h"
#include <QDebug>
#include <QEventLoop>

// after we received at least one actionable statement, we wait this long
// for the user to speak again, otherwise we act on what we got.
static const int turnTimeoutAfterStatements = 1500;

// if we receive input that doesn't contain any useful statement we (initially) wait
// for a pause this long before we act (likely by taking initiative)
// This pause will decrease after every non-statement utterance from the user down to
// a minimum of tunTimeoutAfterStatements
static const int turnTimeoutWithoutStatements = 6000;
static const int turnTimeoutWithoutStatementsDecay = 2000;

SimpleDialogManager::SimpleDialogManager() :
    state(DialogStrategy::InitState), upcomingState(DialogStrategy::InitState),
    previousState(DialogStrategy::InitState), recommender(0),
    currentOffer(0), acceptedStatementsOfThisTurn(0),
    consecutiveMisunderstandingCounter(0)
{
    connect(&turnCompletionTimer, SIGNAL(timeout()), this, SLOT(completeTurn()));
    turnCompletionTimer.setSingleShot(true);
}

void SimpleDialogManager::userIsTalking()
{
    turnCompletionTimer.stop();
}

void SimpleDialogManager::userFinishedTalking()
{
    //turnCompletionTimer.start();
}

void SimpleDialogManager::userInput(const QList<Statement*> statements)
{
    if (state == DialogStrategy::FinalState)
        return;

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
    turnCompletionTimer.start();
}

void SimpleDialogManager::completeTurn()
{
    qDebug() << "Completing turn";
    Recommendation* r = 0;

    if (acceptedStatementsOfThisTurn == 0) {
        //qDebug() << "Misunderstanding counter: " << consecutiveMisunderstandingCounter;
        ++consecutiveMisunderstandingCounter;
    } else {
        consecutiveMisunderstandingCounter = 0;
        recommender->feedbackCycleComplete();
        r = recommender->suggestOffer();
    }

    if (r) {
        currentOffer = r->offer();
        QString explanation = "Explanations not implemented yet."; // TODO

        const Offer *o = r->offer();
        QList<RecommendationAttribute*> description;

        foreach (const QString& key, AttributeFactory::getInstance()->getAttributeIds()) {
            Record r(o->getRecord(key));
            if (!r.second)
                continue;

            if (r.second->getInternal())
                continue;
            QString name = r.first;
            QSharedPointer<Attribute> attr = r.second;
            float expressedUserInterest = recommender->userInterest(r);

            bool showThisAttribute = true;

            // there are two reasons that warrant showing the attribute:
            // 1. A certain set of attributes is shown by default.
            showThisAttribute = attr->getShownByDefault();

            // 2. The user expressed interest in the attribute either directly or indirectly
            showThisAttribute |= expressedUserInterest > 0.1;

            if (!showThisAttribute)
                continue;
            QString value = attr->toString();

            float sentiment = 0;

            description << new RecommendationAttribute(name, value, expressedUserInterest, sentiment);
        }
        emit recommendation(o, o->getName(), o->getPrice(), o->getRating(), o->getImages(),
                            description, o->getUserSentiment(), explanation);
        queueState(DialogStrategy::Recommendation);
    } else {
        qDebug() << "No recommendation at this point";
        switch (consecutiveMisunderstandingCounter) {
        case 0:
            // no misunderstanding but no recommendation -> ask domain question
            askDomainQuestion();
            break;
        case 1:
            queueState(DialogStrategy::MisunderstoodInput);
            break;
        default:
            // too many misunderstandings -> ask domain question
            askDomainQuestion();
            break;
        }
    }

    //based on the assumption that the user wouldn't have critizised the
    // model if he was interested in it, add a *slight* bias against it
    //m_recommender->critique(new Critique(new Relationship(modelName,
    //               Relationship::Inequality, m_currentRecommendation->getAttribute(modelName)), 0.1));

    delete r;

    previousState = state;
    state = upcomingState;
    enterState();
    turnCompletionTimer.setInterval(turnTimeoutWithoutStatements);
    acceptedStatementsOfThisTurn = 0;
}

void SimpleDialogManager::enterState()
{
    switch (state) {
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
        emit elicit(AvatarTask(AvatarTask::Custom, tr("Vielen Dank für Ihre Teilnahme an dieser Studie"), tr("Aufgabe abgeschlossen")));
        break;
    }
}

void SimpleDialogManager::init(CritiqueRecommender *recommender)
{
    this->recommender = recommender;
    previousState = state;
    state = DialogStrategy::InitState;
    upcomingState = DialogStrategy::InitState;
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
    //  b. was not the last asked domain question
    // TODO
    queueState(DialogStrategy::AskForImportantAttribute);
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
    return recommender->critique(c);
}

bool SimpleDialogManager::applyAspect(MentionedAspect* a)
{
    return recommender->applyAspect(a);
}
bool SimpleDialogManager::accept(double strength)
{
    // TODO: experiment with cutoff
    if (strength > 0.5) {
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
