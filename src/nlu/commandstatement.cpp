#include "commandstatement.h"
#include "recommender/critiquerecommender.h"
#include <QObject>

CommandStatement::CommandStatement(CommandStatement::Type type) : m_type(type)
{
}

QString CommandStatement::toString() const
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


bool CommandStatement::act(CritiqueRecommender* r) const
{
    switch (m_type) {
        case CommandStatement::RequestForHelp:
            //TODO
            break;
        case CommandStatement::Back:
            r->undo();
            return true;
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
