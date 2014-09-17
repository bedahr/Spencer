#include "aspectstatement.h"
#include "recommender/mentionedaspect.h"
#include <QObject>

AspectStatement::AspectStatement(const Aspect* aspect, double lexicalPolarity, double quality, double importance) :
    Statement(lexicalPolarity, quality, importance), m_aspect(aspect)
{
}

QString AspectStatement::toString() const
{
    return formatStatementString(QObject::tr("Aspect: %1").arg(m_aspect->id()));
}

bool AspectStatement::act(DialogStrategy::DialogState state, DialogManager *dm, const Offer *currentOffer) const
{
    Q_UNUSED(state);
    Q_UNUSED(currentOffer);
    return dm->applyAspect(new MentionedAspect(m_aspect, effect()));
}
bool AspectStatement::comparePrivate(const Statement *s) const
{
    const AspectStatement* other = dynamic_cast<const AspectStatement*>(s);
    if (!other)
        return false;
    return (other->m_aspect == m_aspect);
}
