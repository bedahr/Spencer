#ifndef ATTRIBUTEFACTORY_H
#define ATTRIBUTEFACTORY_H

class QString;
class QVariant;
class AttributeCreator;
class Attribute;

#include <QHash>
#include <QStringList>
#include <QSharedPointer>

class AttributeFactory
{
public:
    AttributeFactory();
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
    QSharedPointer<Attribute> getAttribute(const QString& name, const QVariant& data) const;

    /// Returns the list of attribute names; Sorted as per sorting criteria
    /// defined in the db structure
    QStringList getAttributeNames() const { return m_attributeNames; }
    AttributeCreator* getCreator(const QString& name) const;

private:
    QStringList m_attributeNames;
    QHash<QString /* attribute name */, AttributeCreator*> m_creators;
};

#endif // ATTRIBUTEFACTORY_H
