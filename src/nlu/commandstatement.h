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

    CommandStatement(Type type, double lexicalPolarity = defaultLexiconPolarity,
                     double quality = defaultQuality, double importance = defaultImportance);
    QString toString() const;
    bool act(DialogStrategy::DialogState state, DialogManager *dm, const Offer* currentOffer) const;
protected:
    virtual bool comparePrivate(const Statement *s) const;

private:
    Type m_type;
    QString commandToString() const;

};

#endif // COMMANDSTATEMENT_H
