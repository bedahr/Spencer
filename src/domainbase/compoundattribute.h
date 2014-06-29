#ifndef COMPOUNDATTRIBUTE_H
#define COMPOUNDATTRIBUTE_H

#include <QList>
#include <QSharedPointer>
#include "attribute.h"

class CompoundAttribute : public Attribute
{
public:
    CompoundAttribute(bool internal, bool shownByDefault, const QString& separator, Relationship::Type relationships, const QList<QSharedPointer<Attribute> > children) :
        Attribute(relationships, internal, shownByDefault), m_separator(separator), m_supportedRelationships(relationships), m_children(children)
    {
    }

    bool operator ==(const Attribute& other) const;
    bool operator !=(const Attribute& other) const;
    bool operator <(const Attribute& other) const;
    bool operator >(const Attribute& other) const;
    double distance(const Attribute& other) const;
    QString toString() const;
    double value() const;

    Relationship::Type definedRelationships() {
        return m_supportedRelationships;
    }

private:
    QString m_separator;
    Relationship::Type m_supportedRelationships;
    QList<QSharedPointer<Attribute> > m_children;
};

#endif // COMPOUNDATTRIBUTE_H
