#include "recommenderitem.h"

float RecommenderItem::influence() const
{
    float inf = ((float) m_ttl) / maxTTL;
    return inf * m_baseInfluence;
}

int RecommenderItem::age()
{
    return --m_ttl;
}
int RecommenderItem::antiAge()
{
    return ++m_ttl;
}

int RecommenderItem::getAge() const
{
    return maxTTL - m_ttl;
}
