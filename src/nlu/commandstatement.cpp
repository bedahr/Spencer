#include "commandstatement.h"
#include "recommender/critiquerecommender.h"
#include <QObject>

CommandStatement::CommandStatement(CommandStatement::Type type, double lexicalPolarity, double quality) :
    Statement(lexicalPolarity, quality), m_type(type)
{
}

QString CommandStatement::commandToString() const
{
    switch (m_type) {
        case CommandStatement::RequestForHelp:
            return QObject::tr("Um Hilfe gebeten");
        case CommandStatement::Back:
            return QObject::tr("Zurück");
        case CommandStatement::AcceptProduct:
            return QObject::tr("Produkt akzeptiert");
        case CommandStatement::Yes:
            return QObject::tr("Ja");
        case CommandStatement::No:
            return QObject::tr("Nein");
    }
    return QObject::tr("Unknown command");
}

QString CommandStatement::toString() const
{
    QString cmd = commandToString();
    return formatStatementString(cmd);
}

bool CommandStatement::act(DialogStrategy::DialogState state, CritiqueRecommender* r) const
{
    switch (m_type) {
        case CommandStatement::Back:
            r->undo();
            return true;
        case CommandStatement::RequestForHelp:
            //TODO
            break;
        case CommandStatement::AcceptProduct:
            //TODO
            break;
        case CommandStatement::Yes:
            //TODO
            break;
        case CommandStatement::No:
            //TODO
            break;
    }
    return false;
}

bool CommandStatement::comparePrivate(const Statement *s) const
{
    const CommandStatement* other = dynamic_cast<const CommandStatement*>(s);
    if (!other)
        return false;
    return other->m_type == m_type;
}
