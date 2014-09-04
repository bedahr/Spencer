#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <QDebug>
#include <QString>
#include <QSharedPointer>
#include <QPair>
#include "relationship.h"

typedef QPair<QString /*name */, QSharedPointer<Attribute> > Record;

class Attribute
{
public:
    Attribute(Relationship::Type definedFor, bool internal, bool shownByDefault) :
        m_definedFor(definedFor), m_internal(internal), m_shownByDefault(shownByDefault)
    {}
    virtual ~Attribute() {}

    virtual bool operator ==(const Attribute& other) const = 0;
    virtual bool operator !=(const Attribute& other) const = 0;

    virtual double distance(const Attribute& other) const {
        return other != *this;
    }
    virtual bool operator <(const Attribute& other) const {
        return other != *this;
    }
    virtual bool operator >(const Attribute& other) const {
        return other != *this;
    }
    virtual QString toString() const = 0;
    bool getInternal() const { return m_internal; }
    bool getShownByDefault() const { return m_shownByDefault; }
    Relationship::Type getDefinedFor() const { return m_definedFor; }
    virtual double value() const { return 0; }
    virtual bool booleanValue() const { return false; }

private:
    Relationship::Type m_definedFor;
    bool m_internal;
    bool m_shownByDefault;

};

#endif // ATTRIBUTE_H
