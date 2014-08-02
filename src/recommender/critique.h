#ifndef CRITIQUE_H
#define CRITIQUE_H

#include "domainbase/relationship.h"
#include "domainbase/attribute.h"

class Offer;

class Critique
{
public:
    static const int maxTTL = 15;

    Critique(const Relationship* relationship, float baseInfluence=1.0) :
        m_ttl(maxTTL + 1),
        m_baseInfluence(baseInfluence),
        m_relationship(relationship)
    {}
    Critique(const Critique& other) : m_ttl(other.m_ttl),
        m_baseInfluence(other.m_baseInfluence),
        m_name(other.m_name), m_relationship(new Relationship(*other.m_relationship)),
        m_supersededCritiques(other.m_supersededCritiques)
    {
    }
    ~Critique();

    /// How well the offer fullfills the critique (1 is
    /// a perfect match, 0 is a violation)
    float utility(const Offer& offer) const;

    /// Decrements the time to life and returns the new value
    int age();
    /// Increments the time to life and returns the new value
    int antiAge();

    /// returns a description of the constraints entailed in this critique
    QString getDescription() const;

    /// returns the current age
    int getAge() const;

    /// returns the base influence
    bool getIsInternal() const { return m_baseInfluence < 0.5; }

    /// returns true if this critique replaces the given one
    bool supersedes(const Critique& other) const;

    /// setter method for @sa m_supersededCritique (for undo)
    void addSuperseding(Critique* c) { m_supersededCritiques << c; }
    QList<Critique*> getSuperseding() const { return m_supersededCritiques; }
    void clearSuperseding() { m_supersededCritiques.clear(); }

    /// Returns true iff the critique makes a statement about the given record
    bool appliesTo(const Record& record) const;

    /// Influence modifier [1..0]; higher, the close
    /// m_ttl is to maxTTL (the younger the critique)
    float influence() const;

private:
    int m_ttl;
    float m_baseInfluence;

    const QString m_name;
    const Relationship* m_relationship;

    QList<Critique*> m_supersededCritiques;

};

#endif // CRITIQUE_H
