#ifndef COMPOUNDATTRIBUTE_H
#define COMPOUNDATTRIBUTE_H

#include <QList>
#include <QSharedPointer>
#include "attribute.h"

class CompoundAttribute : public Attribute
{
public:
    CompoundAttribute(bool internal, const QString& separator, const QString& type, const QList<QSharedPointer<Attribute> > children) :
        Attribute((type == "double") ? Relationship::Equality|Relationship::Inequality|Relationship::LargerThan|Relationship::SmallerThan :
                                       Relationship::Equality|Relationship::Inequality,
                  internal), m_separator(separator), m_type(type), m_children(children)
    {
    }

    bool operator ==(const Attribute& other) const;
    bool operator !=(const Attribute& other) const;
    bool operator <(const Attribute& other) const;
    bool operator >(const Attribute& other) const;
    double distance(const Attribute& other) const;
    QString toString() const;
    double value() const;

private:
    QString m_separator;
    QString m_type;
    QList<QSharedPointer<Attribute> > m_children;
};

#endif // COMPOUNDATTRIBUTE_H
