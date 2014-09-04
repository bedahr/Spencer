#ifndef ASPECTSTATEMENT_H
#define ASPECTSTATEMENT_H
#include "statement.h"
#include "domainbase/aspect.h"

class AspectStatement : public Statement
{
public:
    AspectStatement(const Aspect* aspect, double lexicalPolarity=1.0, double quality=1.0);
    QString toString() const;
    bool act(DialogStrategy::DialogState state, CritiqueRecommender *r) const;
protected:
    virtual bool comparePrivate(const Statement *s) const;
private:
    const Aspect* m_aspect;
};

#endif // ASPECTSTATEMENT_H
