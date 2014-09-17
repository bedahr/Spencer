#include "statement.h"

Statement::Statement(double lexiconPolarity, double statementQuality, double statementImportance) :
    m_quality(statementQuality),
    m_lexicalPolarity(lexiconPolarity), m_importance(statementImportance)
{
}

QString Statement::formatStatementString(const QString& userData) const
{
    return QString("Q: %1; P: %2; I: %3; D: %4").arg(m_quality).arg(m_lexicalPolarity).arg(m_importance).arg(userData);
}

bool fuzzyCompare(double a, double b) {
    return qAbs(a - b) < 0.05;
}

bool Statement::compare(const Statement *s) const
{
    if (!fuzzyCompare(s->m_quality, m_quality) ||
            !fuzzyCompare(s->m_lexicalPolarity, m_lexicalPolarity) ||
            !fuzzyCompare(s->m_importance, m_importance))
        return false;
    return comparePrivate(s);
}

double Statement::effect() const
{
    return m_quality * (m_lexicalPolarity + m_importance);
}
