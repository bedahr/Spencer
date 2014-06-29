#ifndef NUMERICALATTRIBUTE_H
#define NUMERICALATTRIBUTE_H

#include "valueattribute.h"
#include <QString>
#include <QLocale>

class NumericalAttribute : public ValueAttribute<double>
{
public:
    enum Optimality {
        Max,
        Min
    };
    NumericalAttribute(bool internal, bool shownByDefault, double value, const QString& format, Optimality optimality) :
        ValueAttribute<double>(definedRelationships(),
                               internal, shownByDefault, value), m_format(format), m_optimality(optimality)
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
    Optimality m_optimality;

    static double distance(double a, double b);
};

#endif // NUMERICALATTRIBUTE_H
