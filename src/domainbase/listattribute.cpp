#include "listattribute.h"
#include <math.h>
#include <float.h>
#include <QStringList>
#include <QDebug>

bool ListAttribute::operator ==(const Attribute& other) const
{
    const ListAttribute *listOther = dynamic_cast<const ListAttribute*>(&other);
    if (!listOther)
        return false;

    if (m_children.count() != listOther->m_children.count())
        return false;

    for (int i = 0; i < m_children.count(); ++i)
        if (*(m_children[i]) != *(listOther->m_children[i])) {
            qDebug() << m_children[i]->toString() << " != " << listOther->m_children[i]->toString();
            return false;
        }

    return true;
}

bool ListAttribute::operator !=(const Attribute& other) const
{
    if (!dynamic_cast<const ListAttribute*>(&other))
        return false;
    return !(other == *this);
}

double ListAttribute::distance(const Attribute& other) const
{
    const ListAttribute *listOther = dynamic_cast<const ListAttribute*>(&other);
    if (!listOther) // || (m_children.count() != compoundOther->m_children.count()))
        return DBL_MAX;

    double distance = 0;
    int minCommon = std::min(m_children.count(), listOther->m_children.count());
    int maxCommon = std::max(m_children.count(), listOther->m_children.count());
    for (int i = 0; i < minCommon; ++i) {
        distance += m_children[i]->distance(*(listOther->m_children[i]));
    }
    for (int i = minCommon; i < maxCommon; ++i)
        distance += 1;

    return distance / maxCommon;
}

double ListAttribute::value() const
{
    if (m_children.isEmpty())
        return 0;

    double value;
    foreach (QSharedPointer<Attribute> c, m_children)
        value += c->value();
    return (value /m_children.count());
}

bool ListAttribute::operator <(const Attribute& other) const
{
    return distance(other) < 0;
}

bool ListAttribute::operator >(const Attribute& other) const
{
    return distance(other) > 0;
}

QString ListAttribute::toString() const
{
    QStringList out;
    foreach (const QSharedPointer<Attribute>& a, m_children)
        out << a->toString();

    return out.join("; ");
}
