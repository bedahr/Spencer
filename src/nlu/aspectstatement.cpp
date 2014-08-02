#include "aspectstatement.h"
#include <QObject>

AspectStatement::AspectStatement(const QString& aspect, double lexicalPolarity, double quality) :
    Statement(lexicalPolarity, quality), m_aspect(aspect)
{
}

QString AspectStatement::toString() const
{
    return formatStatementString(QObject::tr("Aspect: %1").arg(m_aspect));
}

bool AspectStatement::act(CritiqueRecommender *r) const
{
    //TODO
    return false;
}
