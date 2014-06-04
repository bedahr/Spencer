#ifndef NUMERICALATTRIBUTE_H
#define NUMERICALATTRIBUTE_H

#include "valueattribute.h"
#include <QString>
#include <QLocale>

class NumericalAttribute : public ValueAttribute<double>
{
public:
    NumericalAttribute(bool internal, double value, bool minSet, double min, bool maxSet, double max) :
        ValueAttribute<double>(Relationship::Equality|Relationship::Inequality|Relationship::LargerThan|Relationship::SmallerThan,
                               internal, value), m_minSet(minSet), m_min(min), m_maxSet(maxSet), m_max(max)
    {
        if (minSet && value < min)
            m_value = min;
        if (maxSet && value > max)
            m_value = max;
    }
    bool operator <(const Attribute& other) const;
    bool operator >(const Attribute& other) const;
    double distance(const Attribute& other) const;
    QString toString() const;
    double value() const { return m_value; }

protected:
    bool m_minSet;
    double m_min;

    bool m_maxSet;
    double m_max;

    static double distance(double a, double b, bool minSet, double min, bool maxSet, double max);
};

#endif // NUMERICALATTRIBUTE_H
