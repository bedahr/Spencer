#ifndef OFFER_H
#define OFFER_H

#include "attribute.h"
#include "stringattribute.h"
#include <QString>
#include <QPixmap>
#include <QHash>
#include <QSharedPointer>
#include <QMetaType>
#include <QObject>

class Offer : public QObject
{
public:
    Offer(const QString& name, float priorPropability, const QPixmap& image,
          const QStringList& attributeNames, const QHash<QString, QSharedPointer<Attribute> > attributes);
    ~Offer();

    /// returns a pointer to the attribute with the given name
    /// or 0 if there is no such attribute
    const QSharedPointer<Attribute> getAttribute(const QString& name) const;

    QString getName() const { return m_name->getValue(); }
    QPixmap getImage() const { return m_image; }
    float priorPropability() const { return m_priorPropability; }
    QHash<QString, QSharedPointer<Attribute> > getAttributes() const { return m_attributes; }
    QStringList getAttributeNames() const { return m_attributeNames; }

private:
    /// User visible name of the offer (product)
    QSharedPointer<StringAttribute> m_name;

    /// prior propability of this item; Can be used
    /// to give more initial weight to certain offers
    /// (e.g. for promotion);
    /// Will determine the initially presented product
    float m_priorPropability;

    /// Product image
    QPixmap m_image;

    /// Sorted list of attribute names
    QStringList m_attributeNames;

    /// other attributes
    QHash<QString /* name */, QSharedPointer<Attribute> > m_attributes;
};

Q_DECLARE_METATYPE(const Offer*)

#endif // OFFER_H
