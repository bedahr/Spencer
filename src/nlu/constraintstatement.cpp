#include "constraintstatement.h"
#include "recommender/critiquerecommender.h"

ConstraintStatement::ConstraintStatement(Relationship *relationship, double lexicalPolarity, double quality) :
    Statement(lexicalPolarity, quality), m_relationship(relationship)
{
}

QString ConstraintStatement::toString() const
{
    return formatStatementString(m_relationship->toString());
}

bool ConstraintStatement::act(CritiqueRecommender* r) const
{
    Critique *c = new Critique(m_relationship);
    return r->critique(c);
}
