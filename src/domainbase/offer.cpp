#include "offer.h"
#include <QObject>

static const QString nameId = QObject::tr("Name");

Offer::Offer(const QString& name, float priorPropability, const QPixmap& image, const QStringList &attributeNames,
             const QHash<QString, QSharedPointer<Attribute> > attributes) :
    m_name(new StringAttribute(false, name)), m_priorPropability(priorPropability),
    m_image(image), m_attributes(attributes)
{
    foreach (const QString& name, attributeNames)
        if (attributes.keys().contains(name))
            m_attributeNames.append(name);
}

Offer::~Offer()
{
}

const QSharedPointer<Attribute> Offer::getAttribute(const QString& name) const
{
    if (name == nameId)
        return m_name;

    if (m_attributes.contains(name))
        return m_attributes.value(name);

    return QSharedPointer<Attribute>();
}
