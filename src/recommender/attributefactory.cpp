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

AttributeFactory::AttributeFactory()
{
}
AttributeFactory::~AttributeFactory()
{
    qDeleteAll(m_creators);
}

bool AttributeFactory::parseStructure(const QString& path)
{
    qDeleteAll(m_creators);
    m_creators.clear();

    QDomDocument doc;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file: " << path;
        return false;
    }
    if (!doc.setContent(f.readAll())) {
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
        QDomElement nameElem = featureElement.firstChildElement("name");
        QDomElement typeElem = featureElement.firstChildElement("type").firstChildElement();

        QString name = nameElem.text();
        QString type = typeElem.tagName();
        attributes.insert(featureElement.attribute("nr").toInt(), name);
        bool internal = featureElement.attribute("internal") == "true";
        //qDebug() << "Name: " << name << " type: " << type;

        //"symbol" => StringAttribute
        //"double" => NumericalAttribute
        if (type == "symbol") {
            m_creators.insert(name, new StringAttributeCreator(internal));
        } else {
            QDomElement minElem = typeElem.firstChildElement("minInclusiveValue");
            QDomElement maxElem = typeElem.firstChildElement("maxInclusiveValue");
            bool minSet = !minElem.isNull();
            bool maxSet = !maxElem.isNull();
            double min = minElem.text().toDouble();
            double max = maxElem.text().toDouble();
            if (type == "double") {
                m_creators.insert(name, new NumericalAttributeCreator(internal, minSet, min,
                                                                  maxSet, max));
            } else if (type == "compound") {
                QString subType = typeElem.attribute("type");
                QString separator = typeElem.attribute("separator");
                m_creators.insert(name, new CompoundAttributeCreator(internal, minSet, min, maxSet, max,
                                                                     separator, subType));
            } else {
                qWarning() << "Unknown type " << type << " for attribute " << name;
                return false;
            }
        }

        featureElement = featureElement.nextSiblingElement("feature");
    }
    m_attributeNames.clear();
    QList<int> attributeNumbers(attributes.keys());
    qSort(attributeNumbers);
    foreach (int nr, attributeNumbers) {
        m_attributeNames << attributes[nr];
    }
    return true;
}

QSharedPointer<Attribute> AttributeFactory::getAttribute(const QString& name, const QVariant& data) const
{
    if (!m_creators.contains(name))
        return QSharedPointer<Attribute>();

    return m_creators.value(name)->getAttribute(data);
}

AttributeCreator* AttributeFactory::getCreator(const QString& name) const
{
    return m_creators.value(name);
}
