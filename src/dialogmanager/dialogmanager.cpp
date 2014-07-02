#include "dialogmanager.h"
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

DialogManager::DialogManager() : recommender(0), consecutiveMisunderstandingCounter(0)
{
    // set up dialog strategy
    QState *initState = new QState();
    dialogStateMachine.addState(initState);
    dialogStateMachine.setInitialState(initState);

    connect(initState, SIGNAL(entered()), this, SLOT(greet()));
}

void DialogManager::userInput(const QList<Statement*> statements)
{
    qDebug() << "Misunderstanding counter: " << consecutiveMisunderstandingCounter;
    if (!statements.isEmpty())
        consecutiveMisunderstandingCounter = 0;
    else ++consecutiveMisunderstandingCounter;

    if (statements.isEmpty()) {
        emit elicit(AvatarTask(AvatarTask::Misunderstood));
        return;
    }

    foreach (Statement *s, statements) {
        if (!s->act(recommender)) {
            qWarning() << "No match for " << s->toString();
            emit elicit(AvatarTask(AvatarTask::Custom, tr("Leider konnte ich kein passendes Produkt finden mit %1").arg(s->toString()),
                                   tr("Kein passendes Produkt.")));
        }
        // FIXME: explanation!
        qDebug() << "Processed: " << s->toString();
    }
    recommender->feedbackCycleComplete();
    completeTurn();
}

void DialogManager::completeTurn()
{
    Recommendation* r = recommender->suggestOffer();

    if (r) {
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
}

void DialogManager::init(CritiqueRecommender *recommender)
{
    if (dialogStateMachine.isRunning()) {
        //hackety hack
        QEventLoop l;
        connect(&dialogStateMachine, SIGNAL(stopped()), &l, SLOT(quit()));
        dialogStateMachine.stop();
        l.exec();
    }

    this->recommender = recommender;
    qDebug() << "Starting state machine";

    dialogStateMachine.start();
    completeTurn();
}

void DialogManager::greet()
{
    qDebug() << "Greeting";
    emit elicit(AvatarTask(AvatarTask::Intro), true);
    //emit elicit(AvatarTask(AvatarTask::Custom, "Hallo du Ei"), true);
}
