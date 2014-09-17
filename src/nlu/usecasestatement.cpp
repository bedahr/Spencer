#include "usecasestatement.h"
#include "aspectstatement.h"
#include "constraintstatement.h"
#include "domainbase/aspectfactory.h"
#include "domainbase/attributefactory.h"

#include "recommender/critiquerecommender.h"
#include <QObject>
#include <QList>

UsecaseStatement::UsecaseStatement(const QString& useCase, double lexicalPolarity, double quality, double importance) :
    Statement(lexicalPolarity, quality, importance), m_useCase(useCase)
{
}

QString UsecaseStatement::toString() const
{
    return formatStatementString(QObject::tr("Use case: %1").arg(m_useCase));
}

bool UsecaseStatement::act(DialogStrategy::DialogState state, DialogManager *dm) const
{
    //expand use case to relevant domain information
    //expand to substatements
    QList<Statement*> subStatements;
    if (m_useCase == "office") {
        subStatements << new ConstraintStatement(new Relationship("price", Relationship::Good), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Input Devices"), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Support"), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Workmanship"), m_lexicalPolarity, m_quality, m_importance);
    } else if (m_useCase == "cpuheavy") {
        subStatements << new ConstraintStatement(new Relationship("processorSpeed", Relationship::Large, QSharedPointer<Attribute>(), 2), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new ConstraintStatement(new Relationship("mainMemoryCapacity", Relationship::Large), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Speed"), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Cooling"), m_lexicalPolarity, m_quality, m_importance * 0.5);
    } else if (m_useCase == "multimedia") {
        subStatements << new ConstraintStatement(new Relationship("processorSpeed", Relationship::Large), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new ConstraintStatement(new Relationship("screenSize", Relationship::Large), m_lexicalPolarity, m_quality * 0.7, m_importance * 0.5);
        subStatements << new ConstraintStatement(new Relationship("screenHResolution", Relationship::Large), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new ConstraintStatement(new Relationship("screenVResolution", Relationship::Large), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new ConstraintStatement(new Relationship("mainMemoryCapacity", Relationship::Large, QSharedPointer<Attribute>(), 1.5), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Speed"), m_lexicalPolarity, m_quality, m_importance * 0.7);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Display"), m_lexicalPolarity, m_quality, m_importance * 1.5);
    } else if (m_useCase == "gaming") {
        subStatements << new ConstraintStatement(new Relationship("processorSpeed", Relationship::Large), m_lexicalPolarity, m_quality, m_importance);
        subStatements << new ConstraintStatement(new Relationship("dedicatedGraphicsMemoryCapacity", Relationship::Large), m_lexicalPolarity, 0.7 * m_quality, 0.5 * m_importance);
        subStatements << new ConstraintStatement(new Relationship("graphicsType", Relationship::Equality,
                                                                  AttributeFactory::getInstance()->getAttribute("graphicsType", "Nvidia").second),
                                                 m_lexicalPolarity, m_quality, m_importance);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Speed"), m_lexicalPolarity, m_quality, m_importance * 0.8);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Graphics Card"), m_lexicalPolarity, m_quality, m_importance * 0.7);
    } else {
        qWarning() << "Invalid use case: " << m_useCase;
        return false;
    }

    bool out = true;
    foreach (Statement *s, subStatements) {
        if (!s->act(state, dm)) {
            out = false;
            break;
        }
    }
    qDeleteAll(subStatements);

    return out;
}

bool UsecaseStatement::comparePrivate(const Statement *s) const
{
    const UsecaseStatement* other = dynamic_cast<const UsecaseStatement*>(s);
    if (!other)
        return false;
    return other->m_useCase == m_useCase;
}
