#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include <QList>
#include "appliedcritique.h"

class Offer;

class Recommendation
{
public:
    Recommendation(const Offer* offer, const double score, const QList<AppliedCritique>& explanations) :
        m_offer(offer), m_score(score), m_explanations(explanations)
    {}

    const Offer* offer() const { return m_offer; }
    double score() const { return m_score; }
    QList<AppliedCritique> explanations() const { return m_explanations; }

private:
    Offer const *m_offer;
    double m_score;
    QList<AppliedCritique> m_explanations;
};

#endif // RECOMMENDATION_H
