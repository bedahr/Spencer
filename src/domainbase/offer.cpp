#include "offer.h"
#include "collectionattribute.h"
#include <QObject>

static const QString nameId = QObject::tr("name");
static const QString nameName = QObject::tr("Name");
static const QString priceId = QObject::tr("price");
static const QString priceName = QObject::tr("Preis");
static const QString ratingId = QObject::tr("rating");
static const QString ratingName = QObject::tr("Rating");
int Offer::maxRank = 0;

Offer::Offer(QSharedPointer<Attribute> name, QSharedPointer<Attribute> price,
             QSharedPointer<Attribute> rating,
             int priorRank, const QStringList &images,
             const RecordMap records, const SentimentMap &userSentiment) :
    m_name(name), m_price(price), m_rating(rating),
    m_priorRank(priorRank), m_images(images), m_records(records),
    m_userSentiment(userSentiment)
{
    maxRank = (maxRank > m_priorRank) ? maxRank : m_priorRank;
}

Offer::~Offer()
{
}

QString Offer::getName() const
{
    return m_name->toString();
    return static_cast<StringAttribute*>(m_name.data())->toString();
}
Record Offer::getRecord(const QString& id) const
{
    if (id == nameId)
        return Record(nameName, m_name);

    if (id == priceId)
        return Record(priceName, m_price);

    if (id == ratingId)
        return Record(ratingName, m_rating);

    Record fail = Record(QString(), QSharedPointer<Attribute>());
    //decode: foo[0][1]
    if (id.contains('[')) {
        int idxIdx;
        idxIdx = id.indexOf('[');
        QString plainAttributeName = id.left(idxIdx);
        QSharedPointer<Attribute> attr = m_records.value(plainAttributeName).second;
        do {
            QSharedPointer<CollectionAttribute> cAttr = attr.dynamicCast<CollectionAttribute>();
            if (!cAttr) {
                //qWarning() << "Not a collection attribute: " << id << " at index " << idxIdx << plainAttributeName;
                return fail;
            }
            // access by index
            bool indexExtractionOkay = true;
            QString idxString = id.mid(idxIdx + 1, id.indexOf(']', idxIdx) - idxIdx - 1);
            int idx = idxString.toInt(&indexExtractionOkay);
            if (!indexExtractionOkay) {
                qWarning() << "Index extraction failed for " << idx << idxString;
                return fail;
            }
            attr = cAttr->getChild(idx);
        } while((idxIdx = id.indexOf('[', idxIdx + 1)) != -1);
        return Record(id /* best name substitute we have */, attr);
    } else
        if (m_records.contains(id))
            return m_records.value(id);

    return fail;
}

float Offer::priorPropability() const
{
    return (maxRank - m_priorRank) / ((float) maxRank);
}
