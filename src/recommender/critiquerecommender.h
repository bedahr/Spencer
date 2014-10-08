#ifndef CRITIQUERECOMMENDER_H
#define CRITIQUERECOMMENDER_H

#include "domainbase/offer.h"
#include "domainbase/attribute.h"
#include <QStack>

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
    double userInterest(const QString &id) const;

    /// Returns a value between 0 and 1 telling us how much the current
    /// user model already constricts the search space (1 meaning that
    /// no known products fit all constraints)
    double userModelRichness() const;

    /// Returns a value between 0 and 1 telling us how beneficial
    /// information about the given attributes and aspects would be,
    /// given the current user model.
    /// The higher the value, the more "new" information would be
    /// introduced.
    /// Note that this does not directly translate to increases in
    /// user model "richness" as it is defined above as we don't yet
    /// know the polarity of the statements about the given attributes
    /// and aspects.
    double assertUsefulness(const QStringList& attributeIds,
                            const QStringList& aspectIds) const;

    /// Returns a value between -1 and 1 telling us how well this
    /// attribute matches the user demands based on our user model
    /// (-1 means not at all, 1 means perfectly, 0 is average /
    /// user model makes no statement about this attribute)
    double completionFactor(const QString& attributeId,
                            const Offer& offer) const;

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
    QList<MentionedAspect*> m_aspects;
    QList<RecommenderItem*> m_userModel;
    QList<Offer*> m_offers;
    QStack<const Offer*> m_lastRecommendation;

    enum LimitBehavior {
        MatchAny,
        MatchAll
    };

    QList<Offer*> limitOffers(const QList<Critique*> constrains,
                              QList<Offer*> products, LimitBehavior limitBehavior) const;
};

#endif // CRITIQUERECOMMENDER_H
