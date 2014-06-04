#ifndef CRITIQUERECOMMENDER_H
#define CRITIQUERECOMMENDER_H

#include "critique.h"
#include "offer.h"

class CritiqueRecommender : public QObject
{
    Q_OBJECT

signals:
    void recommend(const Offer* offer, const QString& explanation) const;

public:
    CritiqueRecommender();

public slots:
    /// Sets the product database to the given list of offers
    void setupDatabase(const QList<Offer*> &offers);

    /// Resets user model and suggests an initial offer
    void init();

    /// Updates the given critique to the user model
    /// If there are no matching products, the critique is rejected and
    /// this method returns false;
    /// The CritiqueRecommender will take over ownership of @p critique
    /// in any case.
    bool critique(Critique* critique);

    /// Removes the most current critique
    void undo();

    /// Call once after critiquing has been complete (one or more
    /// new critiques may be added in one feedback cycle)
    void feedbackCycleComplete();

private:
    QList<Critique*> m_critiques;
    QList<Offer*> m_offers;

    void suggestOffer() const;

};

#endif // CRITIQUERECOMMENDER_H
