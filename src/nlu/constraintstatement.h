#ifndef CONSTRAINTSTATEMENT_H
#define CONSTRAINTSTATEMENT_H

#include "statement.h"
class Relationship;

class ConstraintStatement : public Statement
{
public:
    ConstraintStatement(Relationship *relationship, double lexicalPolarity = defaultLexiconPolarity,
                        double quality = defaultQuality, double importance = defaultImportance);
    QString toString() const;
    bool act(DialogStrategy::DialogState state, DialogManager *dm) const;
protected:
    virtual bool comparePrivate(const Statement *s) const;

private:
    Relationship *m_relationship;

};

#endif // CONSTRAINTSTATEMENT_H
