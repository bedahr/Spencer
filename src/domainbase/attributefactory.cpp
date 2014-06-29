#include "attributefactory.h"
#include "numericalattribute.h"
#include "stringattribute.h"
#include "compoundattribute.h"
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QFile>
#include <QStringList>
#include <QLocale>
#include <QDebug>
#include "attributecreators.h"

AttributeFactory* AttributeFactory::instance = 0;

AttributeFactory::AttributeFactory()
{
}
AttributeFactory::~AttributeFactory()
{
    foreach (AttributeCreatorInfo i, m_creators.values())
        delete i.second;
}

static AttributeCreator* createAttributeCreator(const QDomElement& typeElem, bool internal, bool shownByDefault)
{
    //"symbol" => StringAttribute
    //"number" => NumericalAttribute
    //"bool" => BooleanAttribute
    //"compound" => CompoundAttribute
    //"list" => ListAttribute
    QString type = typeElem.tagName();
    if (type == "symbol") {
        return new StringAttributeCreator(internal, shownByDefault);
    } else if (type == "bool") {
        return new BooleanAttributeCreator(internal, shownByDefault);
    } else if (type == "number") {
        QString format;
        double multiplier = 1;
        QDomElement formatElem = typeElem.firstChildElement("format");
        QDomElement multiplierElem = typeElem.firstChildElement("multiplier");

        if (!formatElem.isNull())
            format = formatElem.text();
        if (!multiplierElem.isNull())
            multiplier = multiplierElem.text().toDouble();

        NumericalAttribute::Optimality optimality;
        if (typeElem.attribute("optimality") == "max")
            optimality = NumericalAttribute::Max;
        else
            optimality = NumericalAttribute::Max;

        return new NumericalAttributeCreator(internal, shownByDefault, format,
                                             optimality, multiplier);
    } else if (type == "compound") {
        QString separator = typeElem.attribute("separator");
        AttributeCreator *child = createAttributeCreator(typeElem.firstChildElement(), true, false);
        if (!child) {
            qWarning() << "Failed to deserialize child of compound creator";
            return 0;
        }
        return new CompoundAttributeCreator(internal, shownByDefault, separator, child);
    } else if (type == "list") {
        QDomElement subTypeElement = typeElem.firstChildElement();
        QList<AttributeCreator*> subCreators;
        while (!subTypeElement.isNull()) {
            subCreators << createAttributeCreator(subTypeElement, true, false);
            subTypeElement = subTypeElement.nextSiblingElement();
        }
        if (subCreators.contains(0)) {
            qDeleteAll(subCreators);
            return 0;
        }
        return new ListAttributeCreator(internal, shownByDefault, subCreators);
    }

    qWarning() << "Unknown type " << type;
    return 0;
}

bool AttributeFactory::parseStructure(const QString& path)
{
    foreach (AttributeCreatorInfo i, m_creators.values())
        delete i.second;
    m_creators.clear();

    QDomDocument doc;
    QFile f(path);

    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file: " << path;
        return false;
    }
    if (!doc.setContent(QString::fromUtf8(f.readAll()))) {
        qWarning() << "Failed to parse XML at " << path;
        return false;
    }

    QDomElement rootElement(doc.documentElement());
    QDomElement caseElement(rootElement.firstChildElement("case"));
    if (caseElement.isNull()) {
        // no case or invalid format
        qWarning() << "Invalid XML: " << path;
        return false;
    }
    QHash<int, QString> attributes;
    QDomElement featureElement = caseElement.firstChildElement("feature");
    while (!featureElement.isNull()) {
        QDomElement idElem = featureElement.firstChildElement("id");
        QDomElement nameElem = featureElement.firstChildElement("name");
        QDomElement typeElem = featureElement.firstChildElement("type").firstChildElement();

        QString id = idElem.text();
        QString name = nameElem.text();
        bool internal = featureElement.attribute("internal") == "true";
        bool shownByDefault = featureElement.attribute("shownByDefault") == "true";
        //qDebug() << "Name: " << name << " type: " << type;

        AttributeCreator *creator = createAttributeCreator(typeElem, internal, shownByDefault);
        if (creator) {
            attributes.insert(featureElement.attribute("nr").toInt(), id);
            AttributeCreatorInfo creatorInfo;
            creatorInfo.first = name;
            creatorInfo.second = creator;
            m_creators.insert(id, creatorInfo);
        }

        featureElement = featureElement.nextSiblingElement("feature");
    }
    m_attributeKeys.clear();
    QList<int> attributeNumbers(attributes.keys());
    qSort(attributeNumbers);
    foreach (int nr, attributeNumbers) {
        m_attributeKeys << attributes[nr];
    }
    return true;
}

Record AttributeFactory::getAttribute(const QString& name, const QVariant& data) const
{
    if (!m_creators.contains(name))
        return Record(QString(), QSharedPointer<Attribute>());

    AttributeCreatorInfo creatorInfo = m_creators.value(name);
    AttributeCreator* creator = creatorInfo.second;
    QSharedPointer<Attribute> att = creator->getAttribute(data);
    return Record(creatorInfo.first, att);
}

AttributeCreator* AttributeFactory::getCreator(const QString& id) const
{
    return m_creators.value(id).second;
}
