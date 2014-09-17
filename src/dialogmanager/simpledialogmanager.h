#ifndef SIMPLEDIALOGMANAGER_H
#define SIMPLEDIALOGMANAGER_H

#include "dialogstrategy.h"
#include "ui/avatar/avatartask.h"
#include "nlu/dialogmanager.h"
#include "domainbase/recommendationattribute.h"
#include "domainbase/offer.h"
#include <QStateMachine>
#include <QList>

class CritiqueRecommender;
class Statement;
class Recommendation;
class Offer;

class SimpleDialogManager : public QObject, public DialogManager
{
Q_OBJECT
signals:
    void elicit(AvatarTask, bool immediately=true);
    void recommendation(const Offer* offer, const QString& offerName, double price, double rating,
                        const QStringList& images,
                        const QList<RecommendationAttribute*>& product,
                        const SentimentMap& userSentiment, const QString& explanation);

public:
    SimpleDialogManager();
    void init(CritiqueRecommender *recommender);
    void userInput(const QList<Statement *> statements);
    virtual bool undo();
    virtual bool constrain(Critique* c);
    virtual bool applyAspect(MentionedAspect *c);
    virtual bool accept(double strength);
    virtual bool reject(double strength);
    virtual bool requestForHelp(double strength);

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

#endif // SIMPLEDIALOGMANAGER_H
