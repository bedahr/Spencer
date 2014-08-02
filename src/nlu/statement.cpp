#include "statement.h"

Statement::Statement(double lexiconPolarity, double statementQuality) :
    m_quality(statementQuality),
    m_lexicalPolarity(lexiconPolarity), m_importance(defaultImportance)
{
}

QString Statement::formatStatementString(const QString& userData) const
{
    return QString("Q: %1; P: %2; I: %3; D: %4").arg(m_quality).arg(m_lexicalPolarity).arg(m_importance).arg(userData);
}
