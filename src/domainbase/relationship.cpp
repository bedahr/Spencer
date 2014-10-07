#include "relationship.h"
#include "attributefactory.h"
#include "offer.h"
#include <math.h>
#include <limits>
#include <QDebug>

static float logisticScale(float in)
{
    in *= 6;
    float out = -0.5 + 1 / (1 + exp(-in));
    return 2.0 * out;
}

float Relationship::utility(const Offer& offer, const QString& id, const QSharedPointer<Attribute> &offerAttribute) const
{
    float out = 0.0f;
    double attributeDistance = -10;
    if (!m_attribute.isNull())
        attributeDistance = m_attribute->distance(*offerAttribute);

    if (m_type & Relationship::Equality) {
        // distance is non directional for equality
        double eqDistance = qAbs(attributeDistance);
        out += (0.5 - eqDistance) * 2;
    }
    if (m_type & Relationship::Inequality) {
        // distance is non directional for inequality
        double ineqDistance = qAbs(attributeDistance);
        out += (ineqDistance - 0.5) * 2;
    }
    if (m_type & Relationship::LargerThan) {
        out += attributeDistance;
    }
    if (m_type & Relationship::SmallerThan) {
        out += (-attributeDistance);
    }

    if ((m_type & Relationship::Small) || (m_type & Relationship::Large)) {
        QSharedPointer<Attribute> goal;
        bool useMedian = AttributeFactory::getInstance()->supportsMedianInstance(id);
        if (useMedian) {
            goal = AttributeFactory::getInstance()->getMedianInstance(id);
        } else {
            if (m_type & Relationship::Small)
                goal = AttributeFactory::getInstance()->getSmallestInstance(id);
            else
                goal = AttributeFactory::getInstance()->getLargestInstance(id);
        }
        if (!goal.isNull()) {
            // size is defined
            double distance = goal->distance(*offerAttribute);

            if (useMedian) {
                if (m_type & Relationship::Small)
                    distance *= -1;
                out += distance;
            } else {
                out += (0.5 - qAbs(distance)) * 2;
            }
            //qDebug() << "Perfect: " << goal->toString() << " this: " << offerAttribute->toString() << " distance = " << distance << " (" << (0.5 - distance) * 2 * m_modifierFactor << ")";
        }
    }

    if (m_type & Relationship::IsTrue)
        out += offerAttribute->booleanValue() ? 1 : 0;
    if (m_type & Relationship::IsFalse)
        out += offerAttribute->booleanValue() ? 0 : 1;
    if ((m_type & Relationship::Good) || (m_type & Relationship::Bad) ||
            (m_type & Relationship::BetterThan) || (m_type & Relationship::WorseThan)) {
        QSharedPointer<Attribute> goal;
        if ((m_type & Relationship::Good) || (m_type & Relationship::BetterThan))
            goal = AttributeFactory::getInstance()->getBestInstance(id);
        else
            goal = AttributeFactory::getInstance()->getWorstInstance(id);

        bool useMedian = AttributeFactory::getInstance()->supportsMedianInstance(id);
        QSharedPointer<Attribute> median;
        if (useMedian)
            median = AttributeFactory::getInstance()->getMedianInstance(id);
        if (!goal.isNull()) {
            // optimality is defined
            double distance = qAbs(goal->distance(*offerAttribute));

            if ((m_type & Relationship::BetterThan) || (m_type & Relationship::WorseThan) || useMedian) {
                //discount based on distance of m_attribute / median
                double oldDistance;
                if ((m_type & Relationship::BetterThan) || (m_type & Relationship::WorseThan))
                    oldDistance = qAbs(goal->distance(*m_attribute));
                else
                    oldDistance = qAbs(goal->distance(*median));
                distance = oldDistance - distance;

                // rescale
                distance *= 1 / (1 - oldDistance);
            } else
                distance = 0.5 - distance;
            //qDebug() << "plain distance: " << distance << " adjusted: " << (0.5 - distance) * m_modifierFactor << " comparing: " << offerAttribute->toString() << " to " << goal->toString();
            out += distance;
        }
    }
    //qDebug() << toString() << " distance for offer " << offer.getName() << ": " << out;
    return logisticScale(out) * m_modifierFactor;
}

QList<QSharedPointer<Attribute> > getAttributes(const Offer& offer, const QString& id)
{
    QList<QSharedPointer<Attribute> > out;
    int squareBracketIdx = id.indexOf("[_]");
    if (squareBracketIdx != -1) {
        int i = 0;
        QList<QSharedPointer<Attribute> > child;
        forever {
            child = getAttributes(offer, QString(id).replace(
                                      squareBracketIdx, 3, QString("[%1]").arg(i)));
            if (!child.isEmpty()) {
                out << child;
            } else {
                break;
            }
            ++i;
        }
    } else {
        const QSharedPointer<Attribute> offerAttribute = offer.getRecord(id).second;
        if (offerAttribute)
            out << offerAttribute;
    }
    return out;
}

/**
 * Call getAttributes to retrieve offer attribute(s), calls the utility function for each of them
 * and returns either the max of those or 0 (or 1 in case the type is IsFalse) if we don't find
 * any offer attributes
 */
float Relationship::utility(const Offer& offer) const
{
    float out;
    QList<QSharedPointer<Attribute> > offerAttributes = getAttributes(offer, m_id);
    //qDebug() << "Getting offer record of id " << m_id;
    if (offerAttributes.isEmpty()) {
        //qDebug() << " and it's null";
        // doesn't even have this attribute
        if (m_type == Relationship::IsFalse)
            out = logisticScale(1.0f) * m_modifierFactor;
        else
            out = 0.0f;
    } else {
        float maxUtility = std::numeric_limits<float>::lowest();
        foreach (const QSharedPointer<Attribute>& offerAttribute, offerAttributes) {
            //qDebug() << " and it's " << offerAttribute->toString();
            float thisUtility = utility(offer, m_id, offerAttribute);
            if (thisUtility > maxUtility)
                maxUtility = thisUtility;
        }
        out = maxUtility;
    }
    return out;
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

    if ((m_attribute.isNull() && other.m_attribute.isNull()) ||
            (!m_attribute.isNull() && !other.m_attribute.isNull() && (*m_attribute == *(other.m_attribute))))
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
    return id.left(id.indexOf('[')) == m_id.left(m_id.indexOf('['));
}
