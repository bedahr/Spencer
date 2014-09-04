#ifndef COMMANDSTATEMENT_H
#define COMMANDSTATEMENT_H

#include "statement.h"

class CommandStatement : public Statement
{
public:
    enum Type {
        RequestForHelp,
        Back,
        AcceptProduct,
        Yes,
        No
    };

    CommandStatement(Type type, double lexicalPolarity=1.0, double quality=1.0);
    QString toString() const;
    bool act(DialogStrategy::DialogState state, CritiqueRecommender* r) const;
protected:
    virtual bool comparePrivate(const Statement *s) const;

private:
    Type m_type;
    QString commandToString() const;

};

#endif // COMMANDSTATEMENT_H
