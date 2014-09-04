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

    bool operator ==(const Attribute& other) const
    {
        const StringAttribute* stringAttribute = dynamic_cast<const StringAttribute*>(&other);
        if (!stringAttribute)
            return false;
        return QRegExp(m_value, Qt::CaseInsensitive, QRegExp::Wildcard).exactMatch(stringAttribute->m_value) ||
                QRegExp(stringAttribute->m_value, Qt::CaseInsensitive, QRegExp::Wildcard).exactMatch(m_value);
    }

    bool operator !=(const Attribute& other) const
    {
        const StringAttribute* stringAttribute = dynamic_cast<const StringAttribute*>(&other);
        if (!stringAttribute)
            return false;
        return !(*this == other);
    }

    QString toString() const {
        return m_value;
    }
};

#endif // STRINGATTRIBUTE_H
