#include "commandstatement.h"
#include "constraintstatement.h"
#include "aspectstatement.h"
#include "domainbase/aspectfactory.h"
#include "domainbase/attributefactory.h"
#include "recommender/critiquerecommender.h"
#include <QObject>

CommandStatement::CommandStatement(CommandStatement::Type type, double lexicalPolarity,
                                   double quality, double importance) :
    Statement(lexicalPolarity, quality, importance), m_type(type)
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

bool CommandStatement::act(DialogStrategy::DialogState state, DialogManager *dm, const Offer *currentOffer) const
{
    QList<Statement*> subStatements;
    switch (m_type) {
        case CommandStatement::Back:
            dm->undo();
            return true;
        case CommandStatement::RequestForHelp:
            dm->requestForHelp(effect());
            break;
        case CommandStatement::AcceptProduct:
            dm->accept(effect());
            break;
        case CommandStatement::Yes:
            switch (state) {
            case DialogStrategy::NullState:
            case DialogStrategy::InitState:
            case DialogStrategy::AskForUseCase:
            case DialogStrategy::AskForImportantAttribute:
            case DialogStrategy::AskForHelp:
            case DialogStrategy::FinalState:
                break;
            case DialogStrategy::MisunderstoodInput:
            case DialogStrategy::Recommendation:
                dm->accept(effect());
                return true;
              case DialogStrategy::AskForPerformanceImportant:
                //expand
                subStatements << new ConstraintStatement(new Relationship("processorSpeed", Relationship::Large,
                                                                          QSharedPointer<Attribute>(), m_lexicalPolarity), 1.0, m_quality, m_importance);
                subStatements << new ConstraintStatement(new Relationship("mainMemoryCapacity", Relationship::Large,
                                                                          QSharedPointer<Attribute>(),  m_lexicalPolarity), 1.0, m_quality, m_importance);
                subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Speed"), m_lexicalPolarity, m_quality, m_importance);
                break;
              case DialogStrategy::AskForPortabilityImportant:
                subStatements << new ConstraintStatement(new Relationship("weight", Relationship::Small,
                                                                          QSharedPointer<Attribute>(),  m_lexicalPolarity), 1.0, m_quality, m_importance);
                subStatements << new ConstraintStatement(new Relationship("screenSize", Relationship::Small,
                                                                          QSharedPointer<Attribute>(),  m_lexicalPolarity), 1.0, m_quality, m_importance);
                subStatements << new ConstraintStatement(new Relationship("averageRuntimeOnBattery", Relationship::Large,
                                                                          QSharedPointer<Attribute>(),  m_lexicalPolarity), 1.0, m_quality, m_importance);
                subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Weight"), m_lexicalPolarity, m_quality, m_importance);
                subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Display Size"), m_lexicalPolarity, m_quality, m_importance);
                subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Battery"), m_lexicalPolarity, m_quality, m_importance);
                break;
              case DialogStrategy::AskForPriceImportant:
                subStatements << new ConstraintStatement(new Relationship("price", Relationship::Small,
                                                                          QSharedPointer<Attribute>(),  m_lexicalPolarity), 1.0, m_quality, m_importance);
                subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Price"), m_lexicalPolarity, m_quality, m_importance);
                break;
            }
            break;
        case CommandStatement::No:
            switch (state) {
                case DialogStrategy::MisunderstoodInput:
                case DialogStrategy::Recommendation:
                    if (currentOffer)
                        subStatements << new ConstraintStatement(new Relationship("name", Relationship::Inequality,
                                                                                AttributeFactory::getInstance()->getAttribute("name", currentOffer->getName()).second));
                    break;
                default:
                    break;
            }
    }

    bool out = true;
    foreach (Statement *s, subStatements) {
        if (!s->act(state, dm, currentOffer)) {
            out = false;
            break;
        }
    }
    qDeleteAll(subStatements);

    return out;
}

bool CommandStatement::comparePrivate(const Statement *s) const
{
    const CommandStatement* other = dynamic_cast<const CommandStatement*>(s);
    if (!other)
        return false;
    return other->m_type == m_type;
}

bool CommandStatement::overrides(const Statement *) const
{
    // no command overrides any other
    return false;
}
