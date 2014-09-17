#ifndef MENTIONEDASPECT_H
#define MENTIONEDASPECT_H

#include "recommender/recommenderitem.h"

class Aspect;

class MentionedAspect : public RecommenderItem
{
public:
    MentionedAspect();
    MentionedAspect(const Aspect* aspect, float baseInfluence=1.0) :
        RecommenderItem(baseInfluence),
        m_aspect(aspect)
    {}
private:
    const Aspect* m_aspect;
};

#endif // MENTIONEDASPECT_H
