#ifndef USECASESTATEMENT_H
#define USECASESTATEMENT_H

#include <QString>
#include "statement.h"

class UsecaseStatement : public Statement
{
public:
    UsecaseStatement(const QString& useCase, double lexicalPolarity=1.0, double quality=1.0);
    QString toString() const;
    bool act(CritiqueRecommender *r) const;
protected:
    virtual bool comparePrivate(const Statement *s) const;
private:
    QString m_useCase;
};

#endif // USECASESTATEMENT_H
