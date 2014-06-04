#include "critiquerecommender.h"
#include "float.h"
#include <limits>
#include <QDebug>

CritiqueRecommender::CritiqueRecommender()
{
}

void CritiqueRecommender::setupDatabase(const QList<Offer*>& offers)
{
    qDeleteAll(m_offers);
    m_offers = offers;
}

void CritiqueRecommender::init()
{
    qDeleteAll(m_critiques);
    m_critiques.clear();
    suggestOffer();
}

bool CritiqueRecommender::critique(Critique* critique)
{
    Q_ASSERT(critique);

    bool matchFound = false;
    foreach (const Offer* o, m_offers) {
        if (critique->utility(*o) > 0) {
            matchFound = true;
            break;
        }
    }
    if (!matchFound) {
        qDebug() << "No matching product for critique: " << critique->getDescription();
        delete critique;
        return false;
    }


    QList<Critique*>::iterator i = m_critiques.begin();
    while (i != m_critiques.end()) {
        if (critique->supersedes(**i)) {
            qDebug() << "deleting";
            Critique *c = *i;
            i = m_critiques.erase(i);
            if (c->getAge() == -1)
                delete c; //added in this round
            else
                critique->addSuperseding(c);
        } else
             ++i;
    }
    m_critiques << critique;
    return true;
}

void CritiqueRecommender::undo()
{
    if (m_critiques.isEmpty())
        return;
    QList<Critique*>::iterator i = m_critiques.begin();
    while (i != m_critiques.end()) {
        int newAge = (*i)->antiAge();
        if (newAge > Critique::maxTTL) {
            Critique *c = *i;
            i = m_critiques.erase(i);
            foreach (Critique *super, c->getSuperseding()) {
                i = m_critiques.insert(i, super);
                ++i;
            }
            c->clearSuperseding();
            delete c;
        } else {
            qDebug() << "Keeping: " << (*i)->getAge() << (*i)->getDescription();
             ++i;
        }
    }
    suggestOffer();
}

void CritiqueRecommender::feedbackCycleComplete()
{
    QList<Critique*>::iterator i = m_critiques.begin();
    while (i != m_critiques.end()) {
        if ((*i)->age() == 0) {
            Critique *c = *i;
            i = m_critiques.erase(i);
            delete c;
        } else
            ++i;
    }
    suggestOffer();
}

void CritiqueRecommender::suggestOffer() const
{
    // apply m_critiques to m_offers to find best offer
    const Offer* bestOffer = 0;
    float bestUtility = -std::numeric_limits<float>::max();

    //qDebug() << "Complete db: ";
    //foreach (Critique *c, m_critiques)
    //    qDebug() << c->getDescription();

    // at least one of the last given constraint(s) *must* be matched for
    // the product to be considered in this iteration
    QList<Offer*> consideredProducts;

    QString explanation;
    int currentAge = INT_MAX;
    for (int i = m_critiques.count() - 1; (i >= 0) && (m_critiques[i]->getAge() <= currentAge); --i) {
        Critique *c = m_critiques[i];
        if (c->getIsInternal())
            continue;
        explanation += c->getDescription() + '\n';
        currentAge = c->getAge();

        foreach (Offer* o, m_offers) {
            if (c->utility(*o) > 0) {
                consideredProducts << o;
            }
        }
    }

    foreach (const Offer* o, (currentAge == INT_MAX) ? m_offers : consideredProducts) {
        double thisUtility = o->priorPropability();
        foreach (const Critique* c, m_critiques)
            thisUtility += c->utility(*o);

        //qDebug() << "Offer " << o->getName() << thisUtility;

        if (thisUtility > bestUtility ||
                ((thisUtility == bestUtility) &&
                 //prefer cheaper models
                 (bestOffer->getAttribute(QString::fromUtf8("Preis (€)"))->distance(*o->getAttribute(QString::fromUtf8("Preis (€)"))) < 0))) {
            bestUtility = thisUtility;
            bestOffer = o;
        }
    }

    if (bestOffer) {
        qDebug() << "Recommending " << bestOffer->getName() << bestUtility;
        emit recommend(bestOffer, explanation.trimmed());
    }
}
