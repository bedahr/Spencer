#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include <QString>
#include <QSharedPointer>
#include <QFlags>

class Offer;
class Attribute;

class Relationship
{
public:
    enum TypeE {
        Equality=1,
        Inequality=2,
        LargerThan=4,
        SmallerThan=8
    };
    Q_DECLARE_FLAGS(Type, TypeE)

    Relationship(const QString& name, Type type, QSharedPointer<Attribute> attribute, double modifierFactor=1.0) :
        m_name(name), m_type(type), m_attribute(attribute), m_modifierFactor(modifierFactor)
    {}

    QString toString() const;

    virtual float utility(const Offer& offer) const;

    /// Returns true if *this is more restrictive
    /// (in the same dimension) than  the given relationship
    /// Ex.: (Price > 100).supersedes(Price > 50) == true
    virtual bool supersedes(const Relationship& other) const;
    virtual ~Relationship() {}

protected:
    /// attribute name
    QString m_name;

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
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Relationship::Type)

#endif // RELATIONSHIP_H
