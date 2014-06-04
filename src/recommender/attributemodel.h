#ifndef ATTRIBUTEMODEL_H
#define ATTRIBUTEMODEL_H

#include "attribute.h"
#include <QAbstractTableModel>
#include <QHash>
#include <QStringList>
#include <QSharedPointer>

class AttributeFactory;

class AttributeModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AttributeModel(AttributeFactory *factory, QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role) const;
    void setAttributes(const QStringList& names, const QHash<QString, QSharedPointer<Attribute> > &attributes);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QHash<int, QByteArray> roleNames() const;

private:
    QStringList m_names;
    QHash<QString /* name */, QSharedPointer<Attribute> > m_attributes;
    AttributeFactory *m_factory;
    
};
Q_DECLARE_METATYPE(AttributeModel*)

#endif // ATTRIBUTEMODEL_H
