#include "aspectstatement.h"
#include <QObject>

AspectStatement::AspectStatement(const Aspect* aspect, double lexicalPolarity, double quality) :
    Statement(lexicalPolarity, quality), m_aspect(aspect)
{
    Q_ASSERT(aspect);
}

QString AspectStatement::toString() const
{
    return formatStatementString(QObject::tr("Aspect: %1").arg(m_aspect->id()));
}

bool AspectStatement::act(DialogStrategy::DialogState state, CritiqueRecommender *r) const
{
    //TODO
    return false;
}
bool AspectStatement::comparePrivate(const Statement *s) const
{
    const AspectStatement* other = dynamic_cast<const AspectStatement*>(s);
    if (!other)
        return false;
    return (other->m_aspect == m_aspect);
}
