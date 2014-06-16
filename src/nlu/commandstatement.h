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

    CommandStatement(Type type);
    QString toString() const;
    bool act(CritiqueRecommender* r) const;

private:
    Type m_type;

};

#endif // COMMANDSTATEMENT_H
