#include "critiquerecommender.h"
#include "appliedrecommenderitem.h"
#include "recommendation.h"
#include "float.h"
#include "critique.h"
#include "mentionedaspect.h"
#include "recommenderitem.h"
#include <limits>
#include <QDebug>

CritiqueRecommender::CritiqueRecommender() : m_lastRecommendation(0)
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
    qDeleteAll(m_userModel);
    m_userModel.clear();
    m_critiques.clear();
    m_aspects.clear();
}

double CritiqueRecommender::userInterest(const QString& id) const
{
    float userInterest = 0;
    foreach (Critique* c, m_critiques)
        if (c->appliesTo(id))
            userInterest = qMax(qAbs(c->influence()), userInterest);
    return userInterest;
}

static float bound(float in) {
    return qMin(1.0f, qMax(-1.0f, in));
}

double CritiqueRecommender::completionFactor(const QString& attributeId,
                        const Offer& offer) const
{
    double completionFactor = 0;
    int count = 0;
    foreach (Critique* c, m_critiques) {
        if (c->appliesTo(attributeId)) {
            float utility = c->utility(offer);
            completionFactor += bound(utility);
            ++count;
        }
    }
    return (count > 0) ? completionFactor / count : 0;
}

bool CritiqueRecommender::critique(Critique* critique)
{
    Q_ASSERT(critique);

    bool matchFound = false;
    foreach (const Offer* o, m_offers) {
        float u = critique->utility(*o);
        if (u >= 0) {
            matchFound = true;
            break;
        }
    }
    if (!matchFound) {
        qDebug() << "No matching product for critique: " << critique->getDescription();
        return false;
    }

    //superseding old critiques
    QList<Critique*>::iterator i = m_critiques.begin();
    while (i != m_critiques.end()) {
        if (critique->supersedes(**i)) {
            qDebug() << "deleting";
            Critique *c = *i;
            m_userModel.removeAll(c);
            i = m_critiques.erase(i);
            if (c->getAge() == -1)
                delete c; //added in this round
            else
                critique->addSuperseding(c);
        } else
             ++i;
    }
    m_critiques << critique;
    m_userModel << critique;
    return true;
}


bool CritiqueRecommender::applyAspect(MentionedAspect *a)
{
    Q_ASSERT(a);

    m_userModel << a;
    m_aspects << a;
    return true;
}

void CritiqueRecommender::undo()
{
    if (m_userModel.isEmpty())
        return;
    QList<RecommenderItem*>::iterator i = m_userModel.begin();
    while (i != m_userModel.end()) {
        int newAge = (*i)->antiAge();
        if (newAge > RecommenderItem::maxTTL) {
            RecommenderItem *r = *i;
            i = m_userModel.erase(i);

            Critique *c = dynamic_cast<Critique*>(r);
            if (c) {
                Critique *c = static_cast<Critique*>(r);
                int critiqueIndex = m_critiques.indexOf(c);
                foreach (Critique *super, c->getSuperseding()) {
                    i = m_userModel.insert(i, super);
                    m_critiques.insert(critiqueIndex, super);
                    ++i;
                }
                c->clearSuperseding();
                m_critiques.removeAll(c);
            }

            delete r;
        } else {
            qDebug() << "Keeping: " << (*i)->getAge() << (*i)->getDescription();
             ++i;
        }
    }
}

void CritiqueRecommender::feedbackCycleComplete()
{
    QList<RecommenderItem*>::iterator i = m_userModel.begin();
    while (i != m_userModel.end()) {
        if ((*i)->age() == 0) {
            RecommenderItem *c = *i;
            i = m_userModel.erase(i);
            delete c;
        } else
            ++i;
    }
}

QList<Offer*> CritiqueRecommender::limitOffers(const QList<Critique*> constraints,
                                               QList<Offer*> products, LimitBehavior limitBehavior) const
{
    QList<Offer*> consideredProducts;
    if (limitBehavior == MatchAll) {
        consideredProducts << products;
    }
    foreach (Critique *c, constraints) {
        foreach (Offer* o, products) {
            if (c->utility(*o) >= 0) {
                if (limitBehavior == MatchAny)
                    consideredProducts << o;
            } else {
                if (limitBehavior == MatchAll)
                    consideredProducts.removeAll(o);
            }
        }
    }
    return consideredProducts;
}

Recommendation* CritiqueRecommender::suggestOffer()
{
    //random selection
    //return new Recommendation(m_offers[qrand() % m_offers.size()], 0, QList<AppliedCritique>());

    // apply m_critiques to m_offers to find best offer
    const Offer* bestOffer = 0;
    float bestUtility = std::numeric_limits<float>::lowest();
    QList<AppliedRecommenderItem> bestOfferExplanations;

    //qDebug() << "Complete db: ";
    //foreach (Critique *c, m_critiques)
    //    qDebug() << c->getDescription();


    int currentAge = INT_MAX;
    //build "considered Products" that are all the products, that match at least one of
    // the constraints added in the last critiquing round
    QList<Critique*> lastAddedCritiques;
    for (int i = m_critiques.count() - 1; (i >= 0) && (m_critiques[i]->getAge() <= currentAge); --i) {
        Critique *c = m_critiques[i];
        currentAge = c->getAge();
        lastAddedCritiques << c;
    }
    QList<Offer*> consideredProducts = limitOffers(lastAddedCritiques, m_offers, MatchAny);

    //if we have no matching products, let us know
    if (consideredProducts.isEmpty())
        return 0;

    // out of consideredProducts (or the whole set, if we have none of those)
    // select the one offer with the highest combined utility
    QList<float> utilitiesOfConsideredProducts;
    foreach (const Offer* o, consideredProducts) {
        double thisUtility = o->priorPropability();
        QList<AppliedRecommenderItem> thisExplanations;
        foreach (const RecommenderItem* ri, m_userModel) {
            AppliedRecommenderItem ari(ri, *o);
            thisExplanations << ari;
            thisUtility += ari.utility();
        }
        thisUtility += o->getRating() / 20.0;
        // introduce similarity
        if (m_lastRecommendation) {
            double normalizedProductDistance = o->productDistance(m_lastRecommendation->getId());
            //normalizedProductDistance *= 2;
            thisUtility -= normalizedProductDistance;
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
        double overallScore = bestUtility / (m_userModel.size() + 1.0);

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

        m_lastRecommendation = bestOffer;
        return new Recommendation(bestOffer, overallScore, bestOfferExplanations);
    } else
        qWarning() << "No best offer!";

    return 0;
}

double CritiqueRecommender::assertUsefulness(const QStringList& attributeIds,
                        const QStringList& aspectIds) const
{
    double usefulness = 1;
    double factor =  1.0 / (attributeIds.count() + aspectIds.count());
    foreach (const QString& attribute, attributeIds) {
        foreach (Critique *c, m_critiques) {
            if (c->appliesTo(attribute))
                usefulness -= (factor * qAbs(c->influence()));
        }
    }
    foreach (const QString& aspect, aspectIds) {
        foreach (MentionedAspect *m, m_aspects) {
            if (m->appliesTo(aspect))
                usefulness -= (factor * qAbs(m->influence()));
        }
    }
    return usefulness;
}

double CritiqueRecommender::userModelRichness() const
{
    QList<Offer*> matchingOffers = limitOffers(m_critiques, m_offers, MatchAll);
    return 1 - ((double) matchingOffers.count()) / m_offers.count();
}
