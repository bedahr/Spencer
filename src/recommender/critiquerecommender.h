#ifndef CRITIQUERECOMMENDER_H
#define CRITIQUERECOMMENDER_H

#include "domainbase/offer.h"
#include "domainbase/attribute.h"

class Recommendation;
class Critique;
class MentionedAspect;
class RecommenderItem;

class CritiqueRecommender : public QObject
{
    Q_OBJECT

public:
    CritiqueRecommender();

    /// Uses the user model to determine the expected user interest in
    /// a given attribute
    double userInterest(const Record& record) const;

public slots:
    /// Sets the product database to the given list of offers
    void setupDatabase(const QList<Offer*> &offers);

    /// Resets user model and suggests an initial offer
    void init();

    /// Adds the given critique to the user model
    /// If there are no matching products, the critique is rejected and
    /// this method returns false;
    /// The CritiqueRecommender will take over ownership of @p critique
    /// in any case.
    bool critique(Critique* critique);

    /// Adds the given aspect observation to the user model
    /// The CritiqueRecommender will take over ownership of aspect @a
    /// in any case.
    bool applyAspect(MentionedAspect* a);

    /// Removes the most current critique
    void undo();

    /// Call once after critiquing has been complete (one or more
    /// new critiques may be added in one feedback cycle)
    void feedbackCycleComplete();

    /// Yield a recommendation based on the current, potentially
    /// empty, user model
    Recommendation* suggestOffer();

private:
    QList<Critique*> m_critiques;
    QList<RecommenderItem*> m_userModel;
    QList<Offer*> m_offers;
    const Offer* m_lastRecommendation;

};

#endif // CRITIQUERECOMMENDER_H
