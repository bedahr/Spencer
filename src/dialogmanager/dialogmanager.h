#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include "ui/avatar/avatartask.h"
#include <QStateMachine>
#include <QList>

class CritiqueRecommender;
class Statement;
class Recommendation;
class Offer;

class DialogManager : public QObject
{
Q_OBJECT
signals:
    void elicit(AvatarTask, bool immediately=true);
    void recommendation(const Offer* product, const QString& explanation);

public:
    DialogManager();
    void init(CritiqueRecommender *recommender);
    void userInput(const QList<Statement *> statements);

private:
    CritiqueRecommender *recommender;
    QStateMachine dialogStateMachine;
    int consecutiveMisunderstandingCounter;

    void completeTurn();

private slots:
    void greet();
};

#endif // DIALOGMANAGER_H
