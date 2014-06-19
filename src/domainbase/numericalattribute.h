#ifndef NUMERICALATTRIBUTE_H
#define NUMERICALATTRIBUTE_H

#include "valueattribute.h"
#include <QString>
#include <QLocale>

class NumericalAttribute : public ValueAttribute<double>
{
public:
    NumericalAttribute(bool internal, double value, const QString& format) :
        ValueAttribute<double>(definedRelationships(),
                               internal, value), m_format(format)
    {
    }
    bool operator <(const Attribute& other) const;
    bool operator >(const Attribute& other) const;
    double distance(const Attribute& other) const;
    QString toString() const;
    double value() const { return m_value; }

    Relationship::Type definedRelationships() {
        return Relationship::Equality|Relationship::Inequality|Relationship::LargerThan|Relationship::SmallerThan;
    }

protected:
    QString m_format;

    static double distance(double a, double b);
};

#endif // NUMERICALATTRIBUTE_H
