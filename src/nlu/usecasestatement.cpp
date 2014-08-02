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
