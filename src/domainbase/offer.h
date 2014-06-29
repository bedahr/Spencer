#ifndef OFFER_H
#define OFFER_H

#include "attribute.h"
#include "aspect.h"
#include "stringattribute.h"
#include "numericalattribute.h"
#include <QString>
#include <QPixmap>
#include <QHash>
#include <QSharedPointer>
#include <QStringList>
#include <QMetaType>
#include <QObject>

typedef QHash<QString, Record> RecordMap;
typedef QHash<Aspect, double> SentimentMap;
class Offer : public QObject
{
public:
    Offer(QSharedPointer<Attribute> name, QSharedPointer<Attribute> price,
          QSharedPointer<Attribute> rating,
          int priorRank, const QStringList &images,
          const RecordMap records,
          const SentimentMap& userSentiment);
    ~Offer();

    /// returns an attribute record or an invalid one if there is no such attribute
    Record getRecord(const QString& id) const;

    QString getName() const;
    double getRating() const { return static_cast<NumericalAttribute*>(m_rating.data())->getValue(); }
    double getPrice() const { return static_cast<NumericalAttribute*>(m_price.data())->getValue(); }
    QStringList getImages() const { return m_images; }
    float priorPropability() const;
    RecordMap getRecords() const { return m_records; }
    SentimentMap getUserSentiment() const { return m_userSentiment; }

private:
    /// User visible name of the offer (product)
    QSharedPointer<Attribute> m_name;
    /// User price of the offer (product)
    QSharedPointer<Attribute> m_price;
    /// User rating of the offer (product)
    QSharedPointer<Attribute> m_rating;

    /// prior rank of this item; Can be used
    /// to give more initial weight to certain offers
    /// (e.g. for promotion);
    /// Lower is better
    int m_priorRank;

    /// Product images
    QStringList m_images;

    /// other attributes
    RecordMap m_records;

    /// user sentiment
    SentimentMap m_userSentiment;

    static int maxRank;
};

Q_DECLARE_METATYPE(const Offer*)
Q_DECLARE_METATYPE(RecordMap)
Q_DECLARE_METATYPE(SentimentMap)

#endif // OFFER_H
