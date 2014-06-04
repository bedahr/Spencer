#ifndef STRINGATTRIBUTE_H
#define STRINGATTRIBUTE_H

#include "valueattribute.h"
#include <QString>

class StringAttribute : public ValueAttribute<QString>
{
public:
    StringAttribute(bool internal, const QString& value) :
        ValueAttribute<QString>(Relationship::Equality | Relationship::Inequality, internal, value)
    {}
    QString toString() const {
        return m_value;
    }
};

#endif // STRINGATTRIBUTE_H
