#include "numericalattribute.h"
#include <float.h>
#include <math.h>
#include <QDebug>

bool NumericalAttribute::operator <(const Attribute& other) const
{
    return distance(other) < 0;
}
bool NumericalAttribute::operator >(const Attribute& other) const
{
    return distance(other) > 0;
}

//TODO
double NumericalAttribute::distance(double a, double b)
{
    //qDebug() << "Distance from a=" << a << " to b=" << b;
    if (a < b)
        return -distance(b, a);

    //if (minSet) {
    //    a -= min;
    //    b -= min;
    //}
    if (b == 0) {
        //qDebug() << "b is minimal returning " << a;
        return a;
    }

    return a / b - 1;
}

double NumericalAttribute::distance(const Attribute& other) const
{
    if (dynamic_cast<const NumericalAttribute*>(&other)) {
        double otherVal = static_cast<const NumericalAttribute&>(other).m_value;
        return distance(otherVal, m_value);
    }

    qDebug() << "Returning infinity";
    return DBL_MAX;
}

QString NumericalAttribute::toString() const
{
    QString out;
    if (!m_format.isNull()) {
        out.sprintf(m_format.toLatin1().constData(), m_value);
    } else {
        double rounded = qRound(m_value);
        if (qAbs(m_value - rounded) < 0.01)
           out = QString::number((int) rounded);

        //somehow Qt's QLocale doesn't seem to like the German decimal separator "," very much; enforce it
        out = QString::number(m_value, 'f', 1).replace('.', ',');
    }
    qDebug() << "To string of " << m_value << " results: " << out;
    return out;
}
