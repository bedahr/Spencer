#include "mentionedaspect.h"
#include "domainbase/aspect.h"
#include "domainbase/offer.h"

float MentionedAspect::utility(const Offer& offer) const
{
    return offer.getUserSentiment().value(*m_aspect, 0) * influence();
}

QString MentionedAspect::getDescription() const
{
    return m_aspect->name();
}
