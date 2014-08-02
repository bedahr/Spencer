#include "critiquerecommender.h"
#include "appliedcritique.h"
#include "recommendation.h"
#include "float.h"
#include <limits>
#include <QDebug>

CritiqueRecommender::CritiqueRecommender()
{
    qsrand(5);
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
}


double CritiqueRecommender::userInterest(const Record& record) const
{
    double userInterest = 0;
    foreach (Critique* c, m_critiques) {
        if (c->appliesTo(record)) {
            userInterest += c->influence();
        }
    }
    return userInterest;
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
    m_critiques << new Critique(*critique);
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
}

Recommendation* CritiqueRecommender::suggestOffer() const
{
    //random selection for now
    return new Recommendation(m_offers[qrand() % m_offers.size()], 0, QList<AppliedCritique>());

    // apply m_critiques to m_offers to find best offer
    const Offer* bestOffer = 0;
    float bestUtility = std::numeric_limits<float>::min();
    QList<AppliedCritique> bestOfferExplanations;

    //qDebug() << "Complete db: ";
    //foreach (Critique *c, m_critiques)
    //    qDebug() << c->getDescription();

    QList<Offer*> consideredProducts;

    QString explanation;
    int currentAge = INT_MAX;
    //build "considered Products" that are all the products, that match at least one of
    // the constraints added in the last critiquing round
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

    //if we have no matching products, let us know
    if (consideredProducts.isEmpty())
        return 0;

    // out of consideredProducts (or the whole set, if we have none of those)
    // select the one offer with the highest combined utility
    QList<float> utilitiesOfConsideredProducts;
    foreach (const Offer* o, consideredProducts) {
        double thisUtility = o->priorPropability();
        QList<AppliedCritique> thisExplanations;
        foreach (const Critique* c, m_critiques) {
            AppliedCritique ac(c, *o);
            thisExplanations << ac;
            thisUtility += ac.utility();
        }

        //qDebug() << "Offer " << o->getName() << thisUtility;
        utilitiesOfConsideredProducts << thisUtility;

        if (thisUtility > bestUtility ||
                ((thisUtility == bestUtility) &&
                 //prefer cheaper models
                 (bestOffer->getPrice() > o->getPrice()))) {
            bestUtility = thisUtility;
            bestOfferExplanations = thisExplanations;
            bestOffer = o;
        }
    }

    if (bestOffer) {
        qDebug() << "Recommending " << bestOffer->getName() << bestUtility;
        double overallScore = bestUtility / (m_critiques.size() + 1.0);

        //discount score based on other utilites (3 * score ) / (sum_i=1^3{runnerUp[i])
        qSort(utilitiesOfConsideredProducts.begin(), utilitiesOfConsideredProducts.end(), qGreater<float>());
        //(max) 3 runner ups
        int runnerUpCount = 3;
        double discreditation = 0;
        for (int i = 1; i <= runnerUpCount; i++) {
            if (i >= utilitiesOfConsideredProducts.count()) {
                runnerUpCount = utilitiesOfConsideredProducts.count() - 1;
                break;
            }
            discreditation += utilitiesOfConsideredProducts[i];
        }
        if (runnerUpCount == 0)
            overallScore *= 3;
        else
            overallScore = (runnerUpCount * overallScore) / discreditation;

        return new Recommendation(bestOffer, overallScore, bestOfferExplanations);
    } else
        qWarning() << "No best offer!";

    return 0;
}
