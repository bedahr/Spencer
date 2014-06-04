#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include "ui/avatar/avatartask.h"
#include <QStateMachine>

class DialogManager : public QObject
{
Q_OBJECT
signals:
    void elicit(AvatarTask, bool immediately);

public:
    DialogManager();
    void init();

public slots:
    void userInput(const QString& input);

private:
    QStateMachine dialogStateMachine;

private slots:
    void greet();
};

#endif // DIALOGMANAGER_H
