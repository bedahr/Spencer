#ifndef LISTATTRIBUTE_H
#define LISTATTRIBUTE_H

#include <QList>
#include <QSharedPointer>
#include "attribute.h"
#include "collectionattribute.h"

class ListAttribute : public Attribute, public CollectionAttribute
{
public:
    ListAttribute(bool internal, bool shownByDefault, const QList<QSharedPointer<Attribute> > children) :
        Attribute(definedRelationships(), internal, shownByDefault), m_children(children)
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

    QSharedPointer<Attribute> getChild(int i) const;

private:
    QList<QSharedPointer<Attribute> > m_children;
};


#endif // LISTATTRIBUTE_H
