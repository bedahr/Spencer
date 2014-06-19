#ifndef APPLIEDCRITIQUE_H
#define APPLIEDCRITIQUE_H

#include "domainbase/offer.h"

class Critique;

class AppliedCritique
{
public:
    AppliedCritique(const Critique* c, const Offer& o) :
        m_critique(c), m_utility(c->utility(o))
    { }
    AppliedCritique(const Critique* c, float utility) :
        m_critique(c), m_utility(utility)
    { }
    const Critique* critique() const { return m_critique; }
    float utility() const { return m_utility; }
private:
    Critique const *m_critique;
    float m_utility;
};

#endif // APPLIEDCRITIQUE_H
