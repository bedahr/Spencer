#include "offer.h"
#include <QObject>

static const QString nameId = QObject::tr("name");
static const QString nameName = QObject::tr("Name");
static const QString priceId = QObject::tr("price");
static const QString priceName = QObject::tr("Preis");
int Offer::maxRank = 0;

Offer::Offer(QSharedPointer<Attribute> name, QSharedPointer<Attribute> price,
             int priorRank, const QStringList &images,
             const RecordMap records, const SentimentMap &userSentiment) :
    m_name(name), m_price(price), m_priorRank(priorRank), m_images(images), m_records(records),
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

    if (m_records.contains(id))
        return m_records.value(id);

    return Record(QString(), QSharedPointer<Attribute>());
}

float Offer::priorPropability() const
{
    return (maxRank - m_priorRank) / ((float) maxRank);
}
