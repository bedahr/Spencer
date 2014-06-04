#include "compoundattribute.h"
#include "numericalattribute.h"
#include <math.h>
#include <float.h>
#include <QStringList>
#include <QDebug>

bool CompoundAttribute::operator ==(const Attribute& other) const
{
    const CompoundAttribute *compoundOther = dynamic_cast<const CompoundAttribute*>(&other);
    if (!compoundOther)
        return false;

    if (m_children.count() != compoundOther->m_children.count())
        return false;

    for (int i = 0; i < m_children.count(); ++i)
        if (*(m_children[i]) != *(compoundOther->m_children[i])) {
            qDebug() << m_children[i]->toString() << " != " << compoundOther->m_children[i]->toString();
            return false;
        }

    return true;
}

bool CompoundAttribute::operator !=(const Attribute& other) const
{
    if (!dynamic_cast<const CompoundAttribute*>(&other))
        return false;
    return !(other == *this);
}
double CompoundAttribute::distance(const Attribute& other) const
{
    const CompoundAttribute *compoundOther = dynamic_cast<const CompoundAttribute*>(&other);
    if (!compoundOther) // || (m_children.count() != compoundOther->m_children.count()))
        return DBL_MAX;

    double distance = 0;
    int minCommon = std::min(m_children.count(), compoundOther->m_children.count());
    int maxCommon = std::max(m_children.count(), compoundOther->m_children.count());
    for (int i = 0; i < minCommon; ++i) {
        //qDebug() << m_children[i]->toString() << " to " << compoundOther->m_children[i]->toString() << " = " << m_children[i]->distance(*(compoundOther->m_children[i]));
        distance += m_children[i]->distance(*(compoundOther->m_children[i]));
    }
    for (int i = minCommon; i < maxCommon; ++i)
        distance += 1;

    return distance / maxCommon;
}

double CompoundAttribute::value() const
{
    if (m_children.isEmpty())
        return 0;

    double value;
    foreach (QSharedPointer<Attribute> c, m_children)
        value += c->value();
    return (value /m_children.count());
}

bool CompoundAttribute::operator <(const Attribute& other) const
{
    return distance(other) < 0;
}

bool CompoundAttribute::operator >(const Attribute& other) const
{
    return distance(other) > 0;
}

QString CompoundAttribute::toString() const
{
    QStringList out;
    foreach (const QSharedPointer<Attribute>& a, m_children)
        out << a->toString();

    return out.join(m_separator);
}
