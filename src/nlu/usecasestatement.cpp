#include "usecasestatement.h"
#include "recommender/critiquerecommender.h"
#include <QObject>

UsecaseStatement::UsecaseStatement(const QString& useCase, double lexicalPolarity, double quality) :
    Statement(lexicalPolarity, quality), m_useCase(useCase)
{
}

QString UsecaseStatement::toString() const
{
    return formatStatementString(QObject::tr("Use case: %1").arg(m_useCase));
}

bool UsecaseStatement::act(CritiqueRecommender* r) const
{
    return false;
}


bool UsecaseStatement::comparePrivate(const Statement *s) const
{
    const UsecaseStatement* other = dynamic_cast<const UsecaseStatement*>(s);
    if (!other)
        return false;
    return other->m_useCase == m_useCase;
}
