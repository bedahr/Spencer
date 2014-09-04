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

bool ConstraintStatement::act(DialogStrategy::DialogState state, CritiqueRecommender* r) const
{
    Critique *c = new Critique(m_relationship);
    return r->critique(c);
}


bool ConstraintStatement::comparePrivate(const Statement *s) const
{
    const ConstraintStatement* other = dynamic_cast<const ConstraintStatement*>(s);
    if (!other)
        return false;
    qDebug() << "Constraint statement: ";
    qDebug() << other->m_relationship->toString();
    qDebug() << m_relationship->toString();
    return other->m_relationship->toString() == m_relationship->toString();
}
