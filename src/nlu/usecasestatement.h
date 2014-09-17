#ifndef USECASESTATEMENT_H
#define USECASESTATEMENT_H

#include <QString>
#include "statement.h"

class UsecaseStatement : public Statement
{
public:
    UsecaseStatement(const QString& useCase, double lexicalPolarity = defaultLexiconPolarity,
                     double quality = defaultQuality, double importance = defaultImportance);
    QString toString() const;
    bool act(DialogStrategy::DialogState state, DialogManager *dm) const;
protected:
    virtual bool comparePrivate(const Statement *s) const;
private:
    QString m_useCase;
};

#endif // USECASESTATEMENT_H
