#ifndef BOOLEANATTRIBUTE_H
#define BOOLEANATTRIBUTE_H

#include "valueattribute.h"
#include <QString>
#include <QObject>

class BooleanAttribute : public ValueAttribute<bool>
{
public:
    BooleanAttribute(bool internal, bool shownByDefault, bool value) :
        ValueAttribute<bool>(definedRelationships(), internal, shownByDefault, value)
    {}
    QString toString() const {
        return m_value ? QObject::tr("Ja") : QObject::tr("Nein");
    }
    Relationship::Type definedRelationships() {
        return Relationship::Equality | Relationship::Inequality;
    }

};

#endif // BOOLEANATTRIBUTE_H
