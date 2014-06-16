#include "attributemodel.h"
#include "attributefactory.h"
#include "attributecreators.h"

AttributeModel::AttributeModel(AttributeFactory *factory, QObject *parent) :
    QAbstractTableModel(parent),
    m_factory(factory)
{
}

QHash<int, QByteArray> AttributeModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names.insert(Qt::UserRole, "key");
    names.insert(Qt::UserRole+1, "value");
    names.insert(Qt::UserRole+2, "isCountable");
    names.insert(Qt::UserRole+3, "realValue");
    names.insert(Qt::UserRole+4, "minValue");
    names.insert(Qt::UserRole+5, "maxValue");
    return names;
}

void AttributeModel::setAttributes(const QStringList& names, const QHash<QString, QSharedPointer<Attribute> >& attributes)
{
    beginResetModel();
    m_attributes = attributes;
    m_names = names;
    Q_ASSERT(m_names.count() == m_attributes.count());
    //no insertMulti, please
    Q_ASSERT(m_attributes.keys().count() == m_attributes.count());
    endResetModel();
}

QVariant AttributeModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    QSharedPointer<Attribute> thisAttribute;
    QString name;
    foreach (const QString& key, m_names) {
        QSharedPointer<Attribute> a = m_attributes.value(key);
        if (a->getInternal())
            continue;
        if (row == 0) {
            thisAttribute = a;
            name = key;
            break;
        }
        --row;
    }
    if (!thisAttribute)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return name;
    case Qt::UserRole:
        return name;
    case Qt::UserRole + 1:
        return thisAttribute->toString();
    case Qt::UserRole + 2:
        return (bool) (thisAttribute->getDefinedFor() & Relationship::LargerThan);

    case Qt::UserRole + 3:
        return thisAttribute->value();
    case Qt::UserRole + 4: {
        AttributeCreator *c = m_factory->getCreator(name);
        if (c && dynamic_cast<NumericalAttributeCreator*>(c)) {
            NumericalAttributeCreator* num = static_cast<NumericalAttributeCreator*>(c);
            if (num->getMinSet())
                return num->getMin();
        } else
        if (c && dynamic_cast<CompoundAttributeCreator*>(c)) {
            CompoundAttributeCreator* ca = static_cast<CompoundAttributeCreator*>(c);
            if (ca->getMinSet())
                return ca->getMin();
        }
        break;
    }
    case Qt::UserRole + 5: {
        AttributeCreator *c = m_factory->getCreator(name);
        if (c && dynamic_cast<NumericalAttributeCreator*>(c)) {
            NumericalAttributeCreator* num = static_cast<NumericalAttributeCreator*>(c);
            if (num->getMaxSet())
                return num->getMax();
        } else
        if (c && dynamic_cast<CompoundAttributeCreator*>(c)) {
            CompoundAttributeCreator* ca = static_cast<CompoundAttributeCreator*>(c);
            if (ca->getMaxSet())
                return ca->getMax();
        }
        break;
    }
    }
    return QVariant();
}

int AttributeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    int count = 0;
    foreach (const QSharedPointer<Attribute>& a, m_attributes.values())
        if (!a->getInternal())
            ++count;
    return count;
}

int AttributeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}
