#ifndef LISTATTRIBUTE_H
#define LISTATTRIBUTE_H

#include <QList>
#include <QSharedPointer>
#include "attribute.h"

class ListAttribute : public Attribute
{
public:
    ListAttribute(bool internal, const QList<QSharedPointer<Attribute> > children) :
        Attribute(definedRelationships(), internal), m_children(children)
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
        return Relationship::Equality|Relationship::Inequality|Relationship::LargerThan|Relationship::SmallerThan;
    }

private:
    QList<QSharedPointer<Attribute> > m_children;
};


#endif // LISTATTRIBUTE_H
