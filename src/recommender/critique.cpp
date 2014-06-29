#include "critique.h"
#include "domainbase/offer.h"
#include "domainbase/attribute.h"

Critique::~Critique()
{
    delete m_relationship;
    qDeleteAll(m_supersededCritiques);
}

float Critique::influence() const
{
    float inf = ((float) m_ttl) / maxTTL;
    return inf * inf * m_baseInfluence;
}

float Critique::utility(const Offer& offer) const
{
    float relUtility = m_relationship->utility(offer);
    float critUtility = relUtility * influence();
    //qDebug() << "Relationship utility: " << relUtility << " critique: " << critUtility;
    return critUtility;
}

int Critique::age()
{
    return --m_ttl;
}
int Critique::antiAge()
{
    return ++m_ttl;
}

int Critique::getAge() const
{
    return maxTTL - m_ttl;
}

QString Critique::getDescription() const
{
    return m_relationship->toString();
}

bool Critique::supersedes(const Critique& other) const
{
    bool ret = m_relationship->supersedes(*(other.m_relationship));
    qDebug() << getDescription() << ((ret) ? " supersedes " : " does not supersede ") << other.getDescription();

    return ret;
}

bool Critique::appliesTo(const Record& record) const
{
    return m_relationship->appliesTo(record.first);
}
