#ifndef CONSTRAINTSTATEMENT_H
#define CONSTRAINTSTATEMENT_H

#include "statement.h"
class Relationship;

class ConstraintStatement : public Statement
{
public:
    ConstraintStatement(Relationship *relationship);
    QString toString() const;
    bool act(CritiqueRecommender *r) const;

private:
    Relationship *m_relationship;

};

#endif // CONSTRAINTSTATEMENT_H
