#ifndef STRINGATTRIBUTE_H
#define STRINGATTRIBUTE_H

#include "valueattribute.h"
#include <QString>

class StringAttribute : public ValueAttribute<QString>
{
public:
    StringAttribute(bool internal, bool shownByDefault, const QString& value) :
        ValueAttribute<QString>(definedRelationships(), internal, shownByDefault, value)
    {}
    Relationship::Type definedRelationships() {
        return Relationship::Equality | Relationship::Inequality;
    }

    QString toString() const {
        return m_value;
    }
};

#endif // STRINGATTRIBUTE_H
