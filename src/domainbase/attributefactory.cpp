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
    QString optimality = typeElem.attribute("optimality");
    QString worst = typeElem.attribute("worst");

    if (type == "symbol") {
        return new StringAttributeCreator(optimality, worst, internal, shownByDefault);
    } else if (type == "bool") {
        bool bOptimality = optimality.isNull() ? true : optimality == "true";
        return new BooleanAttributeCreator(bOptimality, internal, shownByDefault);
    } else if (type == "number") {
        QString format;
        double multiplier = 1;
        QDomElement formatElem = typeElem.firstChildElement("format");
        QDomElement multiplierElem = typeElem.firstChildElement("multiplier");

        if (!formatElem.isNull())
            format = formatElem.text();
        if (!multiplierElem.isNull())
            multiplier = multiplierElem.text().toDouble();

        NumericalAttribute::Optimality nOptimality;
        if (optimality == "min")
            nOptimality = NumericalAttribute::Min;
        else
            nOptimality = NumericalAttribute::Max;

        return new NumericalAttributeCreator(internal, shownByDefault, format,
                                             nOptimality, multiplier);
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

    //debug
    foreach (const QString& id, m_creators.keys()) {
        QString name = m_creators.value(id).first;
        QString type = "uncountable";
        if (dynamic_cast<NumericalAttributeCreator*>(m_creators.value(id).second)) {
            type = "countable";
        }
    }

    return true;
}

Record AttributeFactory::getAttribute(const QString& name, const QVariant& data, bool inDomain)
{
    AttributeCreatorInfo creatorInfo = getCreator(name);
    AttributeCreator* creator = creatorInfo.second;
    if (!creator) return qMakePair(QString(), QSharedPointer<Attribute>());
    QSharedPointer<Attribute> att = creator->getAttribute(data, inDomain);
    return Record(creatorInfo.first, att);
}

QSharedPointer<Attribute> AttributeFactory::getBestInstance(const QString& id)
{
    AttributeCreatorInfo creatorInfo = getCreator(id);
    if (!creatorInfo.second)
        return QSharedPointer<Attribute>();
    return creatorInfo.second->getBestInstance();
}

QSharedPointer<Attribute> AttributeFactory::getWorstInstance(const QString& id)
{
    AttributeCreatorInfo creatorInfo = getCreator(id);
    if (!creatorInfo.second)
        return QSharedPointer<Attribute>();
    return creatorInfo.second->getWorstInstance();
}

QSharedPointer<Attribute> AttributeFactory::getLargestInstance(const QString& id)
{
    AttributeCreatorInfo creatorInfo = getCreator(id);
    if (!creatorInfo.second)
        return QSharedPointer<Attribute>();
    return creatorInfo.second->getLargestInstance();
}

QSharedPointer<Attribute> AttributeFactory::getSmallestInstance(const QString& id)
{
    AttributeCreatorInfo creatorInfo = getCreator(id);
    if (!creatorInfo.second)
        return QSharedPointer<Attribute>();
    return creatorInfo.second->getSmallestInstance();
}

AttributeCreatorInfo AttributeFactory::getCreator(const QString& name) const
{
    AttributeCreatorInfo creatorInfo;
    QString creatorName(name);
    creatorInfo.second = 0;
    while (creatorName.contains('[')) {
        int idxIdx = creatorName.indexOf('[');
        QString plainCreatorName = creatorName.left(idxIdx);
        if (!creatorInfo.second) {
            if (!m_creators.contains(plainCreatorName)) {
                qWarning() << "No list creator called " << plainCreatorName;
                return qMakePair(QString(), (AttributeCreator*) 0);
            }
            creatorInfo = m_creators.value(plainCreatorName);
        }
        // access by index
        bool indexExtractionOkay = true;
        QString idxString = creatorName.mid(idxIdx + 1, creatorName.indexOf(']') - idxIdx - 1);
        int idx;
        if (idxString == "_") {
            // means any, so all creators must obviously be the same - go for the one at index 0
            idx = 0;
        } else {
            idx = idxString.toInt(&indexExtractionOkay);
            if (!indexExtractionOkay) {
                qWarning() << "Index extraction failed for " << creatorName << idxString;
                return qMakePair(QString(), (AttributeCreator*) 0);
            }
        }
        ListAttributeCreator *lac(dynamic_cast<ListAttributeCreator*>(creatorInfo.second));
        if (!lac) {
            qWarning() << "Creator is not a list attribute creator, indexing won't work: " << name;
            return qMakePair(QString(), (AttributeCreator*) 0);
        }
        creatorInfo.second = lac->getCreator(idx);
        creatorName = creatorName.mid(creatorName.indexOf(']') + 1);
    }

    if (!creatorInfo.second) {
        if (!m_creators.contains(name)) {
            qDebug() << "Didn't create attribute for " << name;
            //qDebug() << m_creators.keys();
            return qMakePair(QString(), (AttributeCreator*) 0);
        }
        creatorInfo = m_creators.value(name);
    }
    return creatorInfo;
}
