#include "usecasestatement.h"
#include "aspectstatement.h"
#include "constraintstatement.h"
#include "domainbase/aspectfactory.h"
#include "domainbase/attributefactory.h"

#include "recommender/critiquerecommender.h"
#include <QObject>
#include <QList>

UsecaseStatement::UsecaseStatement(const QString& useCase, double lexicalPolarity, double quality) :
    Statement(lexicalPolarity, quality), m_useCase(useCase)
{
}

QString UsecaseStatement::toString() const
{
    return formatStatementString(QObject::tr("Use case: %1").arg(m_useCase));
}

bool UsecaseStatement::act(DialogStrategy::DialogState state, CritiqueRecommender* r) const
{
    //expand use case to relevant domain information
    //expand to substatements
    QList<Statement*> subStatements;
    if (m_useCase == "office") {
        subStatements << new ConstraintStatement(new Relationship("price", Relationship::Good));
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Input Devices"));
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Support"));
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Workmanship"));
    } else if (m_useCase == "cpuheavy") {
        subStatements << new ConstraintStatement(new Relationship("processorSpeed", Relationship::Large), 2);
        subStatements << new ConstraintStatement(new Relationship("mainMemoryCapacity", Relationship::Large));
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Speed"));
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Cooling"), 0.5);
    } else if (m_useCase == "multimedia") {
        subStatements << new ConstraintStatement(new Relationship("processorSpeed", Relationship::Large));
        subStatements << new ConstraintStatement(new Relationship("screenSize", Relationship::Large), 0.7);
        subStatements << new ConstraintStatement(new Relationship("screenHResolution", Relationship::Large));
        subStatements << new ConstraintStatement(new Relationship("screenVResolution", Relationship::Large));
        subStatements << new ConstraintStatement(new Relationship("mainMemoryCapacity", Relationship::Large), 1.5);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Speed"), 0.7);
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Display"), 1.5);
    } else if (m_useCase == "gaming") {
        subStatements << new ConstraintStatement(new Relationship("processorSpeed", Relationship::Large));
        subStatements << new ConstraintStatement(new Relationship("dedicatedGraphicsMemoryCapacity", Relationship::Large), 0.7);
        subStatements << new ConstraintStatement(new Relationship("graphicsType", Relationship::Equality,
                                                                  AttributeFactory::getInstance()->getAttribute("graphicsType", "Nvidia").second));
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Speed"), 0.7);\
        subStatements << new AspectStatement(AspectFactory::getInstance()->getAspect("Graphics Card"), 0.7);
    } else {
        qWarning() << "Invalid use case: " << m_useCase;
        return false;
    }

    bool out = true;
    foreach (Statement *s, subStatements) {
        if (!s->act(state, r)) {
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
