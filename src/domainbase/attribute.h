#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <QDebug>
#include <QString>
#include "relationship.h"

class Attribute
{
public:
    Attribute(Relationship::Type definedFor, bool internal) :
        m_definedFor(definedFor), m_internal(internal)
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
    Relationship::Type getDefinedFor() const { return m_definedFor; }
    virtual double value() const { return 0; }

private:
    Relationship::Type m_definedFor;
    bool m_internal;
};

#endif // ATTRIBUTE_H
