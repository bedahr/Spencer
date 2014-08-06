#ifndef CONSTRAINTSTATEMENT_H
#define CONSTRAINTSTATEMENT_H

#include "statement.h"
class Relationship;

class ConstraintStatement : public Statement
{
public:
    ConstraintStatement(Relationship *relationship, double lexicalPolarity=1.0, double quality=1.0);
    QString toString() const;
    bool act(CritiqueRecommender *r) const;
protected:
    virtual bool comparePrivate(const Statement *s) const;

private:
    Relationship *m_relationship;

};

#endif // CONSTRAINTSTATEMENT_H
