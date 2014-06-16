#include "relationship.h"
#include "offer.h"
#include <math.h>
#include <QDebug>

float Relationship::utility(const Offer& offer) const
{
    const QSharedPointer<Attribute> offerAttribute = offer.getAttribute(m_name);
    if (!offerAttribute)
        return 0.0; // doesn't even have this attribute

    double distance = m_attribute->distance(*offerAttribute);

    if (m_type & Relationship::Equality) {
        if (distance < 0)
            distance *= -1; // distance is non directional for equality
        //qDebug() << "Got distance for" << offerAttribute->toString() << m_attribute.toString() << distance;
        return (0.5 - distance) * m_modifierFactor;
    }

    //qDebug() << "Got distance for" << offerAttribute->toString() << m_attribute.toString() << distance;

    if (m_type & Relationship::SmallerThan || (m_type & Relationship::Inequality &&
                                               (((distance < 0) && (m_modifierFactor > 0)) ||
                                                ((distance > 0) && (m_modifierFactor < 0)))
                                               ))
        // re-establish > 0 as better
        distance *= -1;

    //we treat the "perfect" distance as "50 % off" to not move too quickly
    double perfectDistance = 0.5 * m_modifierFactor;

    bool violated = false;

    if (perfectDistance < 0) {
        if (distance < 0) {
            distance *= -1;
            perfectDistance *= -1;
        } else
            violated = true;
    }
    if (perfectDistance > 0 && distance < 0) {
        violated = true;
    }
    if (violated) {
        distance = -fabs(distance - perfectDistance);
    } else {
        //both perfectDistance and distance are > 0;
        // calculate distance smartly
        if (distance < perfectDistance)
             distance = sqrt(distance / perfectDistance);
        else
            distance = fmax(perfectDistance - distance + 1, 0.00001);
    }

    //qDebug() << "Returning distance for" << offerAttribute->toString() << m_attribute.toString() << distance;
    return distance; //fmax(-0.1, distance);
}

bool Relationship::supersedes(const Relationship& other) const
{
    if (other.m_name != m_name) {
        return false;
    }
    if (other.m_type != m_type) {
        // supersede contradictions
        if (m_type & Relationship::SmallerThan && other.m_type & Relationship::LargerThan) {
            qDebug() << "heyo";
            if (! (*(other.m_attribute) > *m_attribute))
                return true;
            else
                qDebug() << m_attribute->toString() << " <= " << other.m_attribute->toString();
        } else if (m_type & Relationship::LargerThan && other.m_type & Relationship::SmallerThan) {
            if (!(*(other.m_attribute) < *m_attribute))
                return true;
            else
                qDebug() << m_attribute->toString() << " >= " << other.m_attribute->toString();
        }
        return false;
    }

    //qDebug() << m_type << other.m_type << m_attribute.toString() << other.m_attribute.toString();
    if (m_type & Relationship::Equality)
        return true;

    if (*m_attribute == *(other.m_attribute))
        return true;

    //same type, same attribute
    if (m_type & Relationship::SmallerThan) {
        //qDebug() << "smallerThan";
        if (*m_attribute > *(other.m_attribute)) {
            return true;
        }
    }
    if (m_type & Relationship::LargerThan) {
        //qDebug() << "largerThan";
        if (*m_attribute < *(other.m_attribute)) {
            qDebug() << m_attribute->toString() << " > " << other.m_attribute->toString();
            return true;
        } //else
            //qDebug() << m_attribute.toString() << " <= " << other.m_attribute.toString() << m_attribute.distance(other.m_attribute);
    }
    return false;
}

QString Relationship::toString() const
{
    QStringList strType;
    if (m_type & Relationship::Equality)
        strType << QObject::tr("gleich");
    if (m_type & Relationship::Inequality)
        strType << QObject::tr("ungleich");
    if (m_type & Relationship::SmallerThan)
        strType << QObject::tr("weniger als");
    if (m_type & Relationship::LargerThan)
        strType << QObject::tr("mehr als");
    QString modifierStr;
    if (m_modifierFactor < 0)
        modifierStr = QObject::tr("nicht ");
    else if (m_modifierFactor < 1)
        modifierStr = QObject::tr("marginal ");
    else if (m_modifierFactor > 1)
        modifierStr = QObject::tr("signifikant ");

    return QString("%1 %2%3 %4").arg(m_name).arg(modifierStr).arg(strType.join(QObject::tr(" und "))).arg(m_attribute->toString());
}
