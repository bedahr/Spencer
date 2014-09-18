#ifndef APPLIEDCRITIQUE_H
#define APPLIEDCRITIQUE_H

#include "recommenderitem.h"
#include "domainbase/offer.h"

class AppliedRecommenderItem
{
public:
    AppliedRecommenderItem(const RecommenderItem* r, const Offer& o) :
        m_recommenderItem(r), m_utility(r->utility(o))
    { }
    AppliedRecommenderItem(const RecommenderItem* r, float utility) :
        m_recommenderItem(r), m_utility(utility)
    { }
    const RecommenderItem* recommenderItem() const { return m_recommenderItem; }
    float utility() const { return m_utility; }
private:
    RecommenderItem const *m_recommenderItem;
    float m_utility;
};

#endif // APPLIEDCRITIQUE_H
