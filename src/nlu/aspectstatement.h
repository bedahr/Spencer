#ifndef ASPECTSTATEMENT_H
#define ASPECTSTATEMENT_H
#include "statement.h"

class AspectStatement : public Statement
{
public:
    AspectStatement(const QString& aspect, double lexicalPolarity=1.0, double quality=1.0);
    QString toString() const;
    bool act(CritiqueRecommender *r) const;
protected:
    virtual bool comparePrivate(const Statement *s) const;
private:
    QString m_aspect;
};

#endif // ASPECTSTATEMENT_H
