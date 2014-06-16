#include "dialogmanager.h"
#include <QDebug>

/*
 * This dialog manager implements a few simple behaviors:
 * TODO
 */

DialogManager::DialogManager()
{
    // set up dialog strategy
    QState *initState = new QState();
    dialogStateMachine.addState(initState);
    dialogStateMachine.setInitialState(initState);

    connect(initState, SIGNAL(entered()), this, SLOT(greet()));

}

void DialogManager::userInput(const QString& input)
{
    qDebug() << "Dialog input: " << input;
}

void DialogManager::init()
{
    qDebug() << "Starting state machine";
    dialogStateMachine.start();
}

void DialogManager::greet()
{
    qDebug() << "Greeting";
    //emit elicit(AvatarTask(AvatarTask::Intro), true);
    //emit elicit(AvatarTask(AvatarTask::Custom, "Das ist jetzt ja aber wirklich einmal eine positive Überraschung. So schlecht sieht das nämlich gar nicht aus, oder?"), true);
    //emit elicit(AvatarTask(AvatarTask::Custom, "Hallo du Ei"), true);
}
