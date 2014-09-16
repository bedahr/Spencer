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
    NumericalAttribute(bool internal, bool shownByDefault, double value, const QString& format, Optimality optimality,
                       const double& min, const double& max) :
        ValueAttribute<double>(definedRelationships(),
                               internal, shownByDefault, value), m_format(format), m_optimality(optimality),
                               m_min(min), m_max(max)
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

    const double& m_min;
    const double& m_max;

    //static double distance(double a, double b);
};

#endif // NUMERICALATTRIBUTE_H
