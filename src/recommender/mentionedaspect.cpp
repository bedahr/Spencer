#include "mentionedaspect.h"
#include "domainbase/aspect.h"
#include "domainbase/offer.h"

float MentionedAspect::utility(const Offer& offer) const
{
    float u = offer.getUserSentiment().value(*m_aspect, 0) * influence() * 0.1;
    //qDebug() << "Aspect utility: " << u;
    return u;
}

QString MentionedAspect::getDescription() const
{
    return m_aspect->name();
}

bool MentionedAspect::appliesTo(const QString& id) const
{
    return m_aspect->id() == id;
}
