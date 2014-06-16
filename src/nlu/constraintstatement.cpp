#include "constraintstatement.h"
#include "recommender/critiquerecommender.h"

ConstraintStatement::ConstraintStatement(Relationship *relationship) : m_relationship(relationship)
{
}

QString ConstraintStatement::toString() const
{
    return m_relationship->toString();
}

bool ConstraintStatement::act(CritiqueRecommender* r) const
{
    Critique *c = new Critique(m_relationship);
    return r->critique(c);
}
