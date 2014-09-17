#ifndef CRITIQUE_H
#define CRITIQUE_H

#include "domainbase/relationship.h"
#include "domainbase/attribute.h"
#include "recommender/recommenderitem.h"

class Offer;

class Critique : public RecommenderItem
{
public:
    Critique(const Relationship* relationship, float baseInfluence=1.0) :
        RecommenderItem(baseInfluence),
        m_relationship(relationship)
    {}
    Critique(const Critique& other) : RecommenderItem(other),
        m_name(other.m_name), m_relationship(new Relationship(*other.m_relationship)),
        m_supersededCritiques(other.m_supersededCritiques)
    {
    }
    ~Critique();

    /// How well the offer fullfills the critique (1 is
    /// a perfect match, 0 is a violation)
    float utility(const Offer& offer) const;

    /// returns a description of the constraints entailed in this critique
    QString getDescription() const;

    /// returns true if this critique replaces the given one
    bool supersedes(const Critique& other) const;

    /// setter method for @sa m_supersededCritique (for undo)
    void addSuperseding(Critique* c) { m_supersededCritiques << c; }
    QList<Critique*> getSuperseding() const { return m_supersededCritiques; }
    void clearSuperseding() { m_supersededCritiques.clear(); }

    /// Returns true iff the critique makes a statement about the given record
    bool appliesTo(const Record& record) const;

private:
    int m_ttl;
    float m_baseInfluence;

    const QString m_name;
    const Relationship* m_relationship;

    QList<Critique*> m_supersededCritiques;

};

#endif // CRITIQUE_H
