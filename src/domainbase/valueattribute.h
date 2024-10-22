#ifndef VALUEATTRIBUTE_H
#define VALUEATTRIBUTE_H

#include "attribute.h"
#include <QDebug>

template <typename T>
class ValueAttribute : public Attribute
{
public:
    ValueAttribute(Relationship::Type definedFor, bool internal, bool shownByDefault, T value) :
        Attribute(definedFor, internal, shownByDefault),
        m_value(value)
    {}

    bool operator ==(const Attribute& other) const
    {
        const ValueAttribute<T>* valueAttribute = dynamic_cast<const ValueAttribute<T>*>(&other);
        if (!valueAttribute)
            return false; // wrong type;
        return m_value == valueAttribute->m_value;
    }

    bool operator !=(const Attribute& other) const
    {
        const ValueAttribute<T>* valueAttribute = dynamic_cast<const ValueAttribute<T>*>(&other);
        if (!valueAttribute) {
            qDebug() << "Failed cast";
            return false; // wrong type;
        }
        return m_value != valueAttribute->m_value;
    }

    T getValue() const {
        return m_value;
    }

    virtual Relationship::Type definedRelationships()=0;

protected:
    T m_value;
};

#endif // VALUEATTRIBUTE_H
