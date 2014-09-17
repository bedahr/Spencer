#include "constraintstatement.h"
#include "recommender/critiquerecommender.h"

ConstraintStatement::ConstraintStatement(Relationship *relationship, double lexicalPolarity, double quality, double importance) :
    Statement(lexicalPolarity, quality, importance), m_relationship(relationship)
{
}

QString ConstraintStatement::toString() const
{
    return formatStatementString(m_relationship->toString());
}

bool ConstraintStatement::act(DialogStrategy::DialogState state, DialogManager *dm, const Offer *currentOffer) const
{
    Q_UNUSED(state);
    Q_UNUSED(currentOffer);
    Critique *c = new Critique(m_relationship);
    return dm->constrain(c);
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
