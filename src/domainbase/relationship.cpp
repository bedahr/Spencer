#include "relationship.h"
#include "attributefactory.h"
#include "offer.h"
#include <math.h>
#include <QDebug>

float Relationship::utility(const Offer& offer) const
{
    const QSharedPointer<Attribute> offerAttribute = offer.getRecord(m_id).second;
    if (offerAttribute.isNull()) {
        // doesn't even have this attribute
        if (m_type == Relationship::IsFalse)
            return false;

        return 0.0f;
    }

    float out = 0.0f;
    double attributeDistance = -10;
    if (!m_attribute.isNull())
        attributeDistance = m_attribute->distance(*offerAttribute);

    if (m_type & Relationship::Equality) {
        // distance is non directional for equality
        double eqDistance = qAbs(attributeDistance);
        out += (1 - eqDistance) * m_modifierFactor;
    }
    if (m_type & Relationship::Inequality) {
        // distance is non directional for inequality
        double ineqDistance = qAbs(attributeDistance);
        out += ineqDistance * m_modifierFactor;
    }
    if (m_type & Relationship::LargerThan) {
        out += attributeDistance * m_modifierFactor;
    }
    if (m_type & Relationship::SmallerThan) {
        out += (-attributeDistance) * m_modifierFactor;
    }

    if ((m_type & Relationship::Small) || (m_type & Relationship::Large)) {
        QSharedPointer<Attribute> goal;

        if (m_type & Relationship::Small)
            goal = AttributeFactory::getInstance()->getSmallestInstance(m_id);
        else
            goal = AttributeFactory::getInstance()->getLargestInstance(m_id);
        if (!goal.isNull()) {
            // size is defined
            double distance = qAbs(goal->distance(*offerAttribute));
            //qDebug() << "Perfect: " << goal->toString() << " this: " << offerAttribute->toString() << " distance = " << distance << " (" << (0.5 - distance) * 2 * m_modifierFactor << ")";
            out += (0.5 - distance) * 2 * m_modifierFactor;
        }
    }

    if (m_type & Relationship::IsTrue)
        out += offerAttribute->booleanValue() ? 1 : 0;
    if (m_type & Relationship::IsFalse)
        out += offerAttribute->booleanValue() ? 0 : 1;
    if ((m_type & Relationship::Good) || (m_type & Relationship::Bad) ||
            (m_type & Relationship::BetterThan) || (m_type & Relationship::WorseThan)) {
        QSharedPointer<Attribute> goal;
        if (m_type & Relationship::Good)
            goal = AttributeFactory::getInstance()->getBestInstance(m_id);
        else
            goal = AttributeFactory::getInstance()->getWorstInstance(m_id);
        if (!goal.isNull()) {
            // optimality is defined
            double distance = qAbs(goal->distance(*offerAttribute));

            if ((m_type & Relationship::BetterThan) || (m_type & Relationship::WorseThan)) {
                //discount based on distance of m_attribute
                double oldDistance = qAbs(goal->distance(*m_attribute));
                distance -= oldDistance;

                // rescale
                distance *= 1 / (1 - oldDistance);
            }

            out += (0.5 - distance) * 2 * m_modifierFactor;
        }
    }

    return out;
#if 0
    if (isRelative()) {
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
    } else {
        return 0.0;
    }
#endif
}

bool Relationship::supersedes(const Relationship& other) const
{
    if (other.m_id != m_id) {
        return false;
    }
    if (other.m_type != m_type) {
        // supersede contradictions
        if (m_type & Relationship::SmallerThan && other.m_type & Relationship::LargerThan) {
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

bool Relationship::isRelative() const
{
    return (m_type & Relationship::Equality) ||
            (m_type & Relationship::Inequality) ||
            (m_type & Relationship::SmallerThan) ||
            (m_type & Relationship::LargerThan) ||
            (m_type & Relationship::BetterThan) ||
            (m_type & Relationship::WorseThan);
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
    if (m_type & Relationship::IsTrue)
        strType << QObject::tr("ja");
    if (m_type & Relationship::IsFalse)
        strType << QObject::tr("nein");
    if (m_type & Relationship::BetterThan)
        strType << QObject::tr("besser als");
    if (m_type & Relationship::WorseThan)
        strType << QObject::tr("schlechter als");
    if (m_type & Relationship::Good)
        strType << QObject::tr("gut");
    if (m_type & Relationship::Bad)
        strType << QObject::tr("schlecht");
    if (m_type & Relationship::Large)
        strType << QObject::tr("groß");
    if (m_type & Relationship::Small)
        strType << QObject::tr("klein");
    QString modifierStr;
    if (m_modifierFactor < 0)
        modifierStr = QObject::tr("nicht ");
    else if (m_modifierFactor < 1)
        modifierStr = QObject::tr("marginal ");
    else if (m_modifierFactor > 1)
        modifierStr = QObject::tr("signifikant ");

    return QString("%1 %2%3 %4").arg(m_id).arg(modifierStr).arg(strType.join(QObject::tr(" und ")))
            .arg((m_attribute && isRelative()) ? m_attribute->toString() : QString());
}

bool Relationship::appliesTo(const QString& id) const
{
    return id == m_id;
}
