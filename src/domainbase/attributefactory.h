#ifndef ATTRIBUTEFACTORY_H
#define ATTRIBUTEFACTORY_H

class QString;
class QVariant;
class AttributeCreator;

#include "attribute.h"
#include <QHash>
#include <QStringList>
#include <QSharedPointer>

typedef QPair<QString /* attribute name */, AttributeCreator*> AttributeCreatorInfo;

class AttributeFactory
{
public:
    ~AttributeFactory();

    /// Reads the given structure.xml to build the internal
    /// representation of available attributes and their
    /// properties
    /// Call this before calling getAttribute!
    bool parseStructure(const QString& path);

    /// Returns an attribute instance of the attribute with the given
    /// name set to provided data. Data may be set to null if the attribute
    /// does not require it.
    /// The returned attribute is null if an error occured
    Record getAttribute(const QString& name, const QVariant& data, bool inDomain=false);

    /// Creates an "ideal" / "worst" attribute instance based on
    /// the creator's optimality within the constraints of the data set
    QSharedPointer<Attribute> getBestInstance(const QString& id);
    QSharedPointer<Attribute> getWorstInstance(const QString& id);
    QSharedPointer<Attribute> getLargestInstance(const QString& id);
    QSharedPointer<Attribute> getSmallestInstance(const QString& id);
    bool supportsMedianInstance(const QString& id);
    QSharedPointer<Attribute> getMedianInstance(const QString& id);

    /// Returns the list of attribute ids; Sorted as per sorting criteria
    /// defined in the db structure
    QStringList getAttributeIds() const { return m_attributeKeys; }

    /// Instance method for the Singleton
    static AttributeFactory* getInstance() {
        if (!instance) instance = new AttributeFactory;
        return instance;
    }

private:
    static AttributeFactory* instance;

    QStringList m_attributeKeys;
    QHash<QString /* attribute id */, AttributeCreatorInfo> m_creators;
    AttributeCreatorInfo getCreator(const QString& id) const;

    AttributeFactory();
};

#endif // ATTRIBUTEFACTORY_H
