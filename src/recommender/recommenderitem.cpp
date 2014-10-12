#include "recommenderitem.h"

float RecommenderItem::influence() const
{
    float inf = ((float) m_ttl) / maxTTL;
#ifndef SPENCER_UNNUANCED
    inf = inf * inf * m_baseInfluence;
#endif
    return inf;
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
