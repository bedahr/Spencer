#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include "dialogstrategy.h"
#include "ui/avatar/avatartask.h"
#include "domainbase/recommendationattribute.h"
#include "domainbase/offer.h"
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
    void recommendation(const Offer* offer, const QString& offerName, double price, double rating,
                        const QStringList& images,
                        const QList<RecommendationAttribute*>& product,
                        const SentimentMap& userSentiment, const QString& explanation);

public:
    DialogManager();
    void init(CritiqueRecommender *recommender);
    void userInput(const QList<Statement *> statements);

public slots:
    //these should be private slots, just here for the wizard of oz evaluation
    void askForUseCase();
    void askForMostImportantAttribute();
    void askForPerformanceImportant();
    void askForPriceImportant();
    void askForPortabilityImportant();
    void randomRecommendation();

private:
    DialogStrategy::DialogState state;
    CritiqueRecommender *recommender;
    int consecutiveMisunderstandingCounter;

    void completeTurn();

private slots:
    void greet();
};

#endif // DIALOGMANAGER_H
