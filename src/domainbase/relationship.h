#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include <QString>
#include <QSharedPointer>
#include <QFlags>
#include <QDebug>

class Offer;
class Attribute;

class Relationship
{
public:
    enum TypeE {
        Equality=1,
        Inequality=2,
        LargerThan=4,
        SmallerThan=8,
        IsTrue=16,
        IsFalse=32,
        BetterThan=64,
        WorseThan=128,
        Good=256,
        Bad=512,
        Large=1024,
        Small=2048
    };
    Q_DECLARE_FLAGS(Type, TypeE)

    Relationship(const QString& id, Type type, QSharedPointer<Attribute> attribute, double modifierFactor=1.0) :
        m_id(id), m_type(type), m_attribute(attribute), m_modifierFactor(modifierFactor)
    {
        if (attribute.isNull()) {
            //qDebug() << "No comparison attribute specified, backing off type";
            m_type = backoffRelative(type);
        }
    }

    // turns all relative types (smaller, larger, etc.) into absolute ones (small, large, etc.)
    Type backoffRelative(Type original) {
        if (original & Relationship::SmallerThan)
            original |= Relationship::Small;
        if (original & Relationship::LargerThan)
            original |= Relationship::Large;
        if (original & Relationship::BetterThan)
            original |= Relationship::Good;
        if (original & Relationship::WorseThan)
            original |= Relationship::Bad;
        original &= ~(Relationship::SmallerThan | Relationship::LargerThan |
                      Relationship::BetterThan | Relationship::WorseThan);
        return original;
    }

    QString toString() const;

    virtual float utility(const Offer& offer) const;

    /// Returns true if the relationship makes a statement about the given attribute id
    virtual bool appliesTo(const QString& id) const;

    /// Returns true if *this is more restrictive
    /// (in the same dimension) than  the given relationship
    /// Ex.: (Price > 100).supersedes(Price > 50) == true
    virtual bool supersedes(const Relationship& other) const;
    virtual ~Relationship() {}

protected:
    /// attribute id
    QString m_id;

    /// type of this relationship
    Type m_type;

    QSharedPointer<Attribute> m_attribute;

    /// Represents modifiers like "very [different]"; 1.0 means no modification,
    /// ]0, 1[ means we are looking for something "a little bit" in the direction
    /// of the relationship
    /// Everything larger than 1 means we are looking something far off in the search
    /// direction
    /// < 0 means we want to invert the result (example modifier: "not")
    double m_modifierFactor;
    bool isRelative() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Relationship::Type)

#endif // RELATIONSHIP_H
