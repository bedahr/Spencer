#ifndef RECOMMENDERITEM_H
#define RECOMMENDERITEM_H

#include <QString>

class Offer;

class RecommenderItem
{
public:
    static const int maxTTL = 15;

    virtual ~RecommenderItem() {}

    RecommenderItem(float baseInfluence=1.0) :
        m_ttl(maxTTL + 1),
        m_baseInfluence(baseInfluence)
    {}
    RecommenderItem(const RecommenderItem& other) : m_ttl(other.m_ttl),
        m_baseInfluence(other.m_baseInfluence)
    {}

    /// How well the offer fullfills the critique (1 is
    /// a perfect match, 0 is a violation)
    virtual float utility(const Offer& offer) const = 0;

    /// Decrements the time to life and returns the new value
    int age();
    /// Increments the time to life and returns the new value
    int antiAge();

    /// Influence modifier [1..0]; higher, the close
    /// m_ttl is to maxTTL (the younger the critique)
    float influence() const;

    /// returns the current age
    int getAge() const;

    /// returns a description of the constraints entailed in this critique
    virtual QString getDescription() const = 0;

    float baseInfluence() const { return m_baseInfluence; }
    void bumpBaseInfluence(float baseInfluence) {
        m_baseInfluence += baseInfluence;
    }
private:
    int m_ttl;
protected:
    float m_baseInfluence;
};

#endif // RECOMMENDERITEM_H
