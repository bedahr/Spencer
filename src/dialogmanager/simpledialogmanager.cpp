#include "simpledialogmanager.h"
#include "nlu/statement.h"
#include "domainbase/attributefactory.h"
#include "recommender/critiquerecommender.h"
#include "recommender/recommendation.h"
#include <QDebug>
#include <QEventLoop>

/*
 * This dialog manager implements a few simple behaviors:
 * TODO
 */

/*
 * Potentials:
    Display Brightness
    Display Size
    Viewing Angles
    Display-Resolution
    Picture Quality
    Display
    Main Memory
    Processor
    Graphics Card
    Speed
    Ports
    Touchscreen
    Mouse
    Keyboard
    Input Devices
    Usage
    Packaging
    Features
    Support
    Drive
    Sound
    Looks
    Windows
    OS
    Documentation
    Battery
    Hard Drive
    Workmanship
    Name
    Weight
    Cooling
    Included Software
    Charger
    Connectivity
    Mainboard
    Webcam
    Size
    Brand
    Price
    Upgradeability
    Card Reader
 */

SimpleDialogManager::SimpleDialogManager() :
    state(DialogStrategy::InitState), recommender(0),
    currentOffer(0), consecutiveMisunderstandingCounter(0)
{
}

void SimpleDialogManager::userInput(const QList<Statement*> statements)
{
    if (state == DialogStrategy::FinalState)
        return;

    //qDebug() << "Misunderstanding counter: " << consecutiveMisunderstandingCounter;
    if (!statements.isEmpty())
        consecutiveMisunderstandingCounter = 0;
    else ++consecutiveMisunderstandingCounter;

    if (statements.isEmpty()) {
        emit elicit(AvatarTask(AvatarTask::Misunderstood));
        return;
    }

    qDebug() << "Got statements: " << statements.count();
    foreach (Statement *s, statements) {
        if (!s->act(state, this, currentOffer)) {
            qWarning() << "Failed to act on statement " << s->toString();
            //qWarning() << "No match for " << s->toString();
            //emit elicit(AvatarTask(AvatarTask::Custom, tr("Leider konnte ich kein passendes Produkt finden mit %1").arg(s->toString()),
            //                       tr("Kein passendes Produkt.")));
        } else {
            // FIXME: explanation!
            qDebug() << "Processed: " << s->toString();
        }
    }
    completeTurn();
}

void SimpleDialogManager::completeTurn()
{
    recommender->feedbackCycleComplete();
    Recommendation* r = recommender->suggestOffer();

    if (r) {
        currentOffer = r->offer();
        QString explanation = "Explanations not implemented yet."; // TODO

        emit elicit(AvatarTask(AvatarTask::PresentItem), false);

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
    } else {
        qDebug() << "No recommendation at this point";
        // TODO: Ask domain questions
    }

    //based on the assumption that the user wouldn't have critizised the
    // model if he was interested in it, add a *slight* bias against it
    //m_recommender->critique(new Critique(new Relationship(modelName,
    //               Relationship::Inequality, m_currentRecommendation->getAttribute(modelName)), 0.1));
    delete r;
}

void SimpleDialogManager::init(CritiqueRecommender *recommender)
{
    this->recommender = recommender;
    state = DialogStrategy::InitState;
    greet();
}

void SimpleDialogManager::greet()
{
    qDebug() << "Greeting";
    emit elicit(AvatarTask(AvatarTask::Intro), true);
    //emit elicit(AvatarTask(AvatarTask::Custom, "Hallo du Ei"), true);
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
        emit elicit(AvatarTask(AvatarTask::Custom, tr("Vielen Dank für Ihre Teilnahme an dieser Studie"), tr("Aufgabe abgeschlossen")));
        state = DialogStrategy::FinalState;
        return true;
    }
    return false;
}

bool SimpleDialogManager::requestForHelp(double strength)
{
    return false;
}