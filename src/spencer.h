#ifndef SPENCER_H
#define SPENCER_H

#include "ui/avatar/avatartask.h"
#include "domainbase/offer.h"
#include "domainbase/recommendationattribute.h"
#include <QObject>
#include <QList>
#include <QStringList>

class CritiqueRecommender;
class Offer;
class Relationship;
class AttributeFactory;
class NLU;
class DialogManager;
class DatabaseConnector;

class Spencer : public QObject
{
    Q_OBJECT

signals:
    void recommend(const QString &offerName, double price, double rating, const QStringList &images,
                   const QList<RecommendationAttribute*>& offer, SentimentMap userSentiment,
                   const QString& description);
    void elicit(AvatarTask, bool immediately);

public:
    explicit Spencer(QObject *parent = 0);
    ~Spencer();

public slots:
    /// Initialize spencer
    bool init();
    void reset();

    void userInput(const QString& input);

private:
    DatabaseConnector *m_databaseConnector;
    AttributeFactory *m_attributeFactory;
    NLU *m_nlu;
    DialogManager *m_dialogManager;
    CritiqueRecommender *m_recommender;
    const Offer *m_currentRecommendation;

    /// parses the database of offers from the given casebase XML file
    /// and returns the list of found offers; if an error occured,
    /// *okay will be set to false
    QList<Offer*> parseCasebase(bool* okay) const;


private slots:
    void recommendationChanged(const Offer *currentOffer,
                               const QString &offerName, double price, double rating, const QStringList &images,
                               const QList<RecommendationAttribute *> &offer, SentimentMap userSentiment, const QString &explanation);
    
};

#endif // SPENCER_H
