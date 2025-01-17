#include "databaseconnector.h"
#include "domainbase/offer.h"
#include "domainbase/aspect.h"
#include "domainbase/aspectfactory.h"
#include "domainbase/attributefactory.h"
#include <QString>

#include "mongo/client/dbclient.h"
#include "mongo/client/dbclientcursor.h"

//#define BUILD_SIMILARITY_CACHE

DatabaseConnector::DatabaseConnector() : c(new mongo::DBClientConnection)
{
}

DatabaseConnector::~DatabaseConnector()
{
    delete c;
}

bool DatabaseConnector::init()
{
    try {
        c->connect("localhost");
        qDebug() << "Connected to MongoDB";
    } catch( const mongo::DBException &e ) {
        qWarning() << "Caught: " << e.what();
        return false;
    }
    return true;
}

static QStringList buildEssentialSentimentDimensions() {
    QStringList out;
    out << "Display";
    out << "Speed";
    out << "Ports";
    out << "Battery";
    out << "Workmanship";
    out << "Input Devices";
    return out;
}

static QStringList buildOptionalSentimentDimensions() {
    QStringList out;
    out << "Display Brightness";
    out << "Display Size";
    out << "Viewing Angles";
    out << "Display Resolution";
    out << "Picture Quality";
    out << "Main Memory";
    out << "Processor";
    out << "Graphics Card";
    out << "Touchscreen";
    out << "Features";
    out << "Support";
    out << "Sound";
    out << "Looks";
    out << "Weight";
    out << "Cooling";
    out << "Included Software";
    out << "Connectivity";
    out << "Webcam";
    out << "Size";
    out << "Brand";
    out << "Price";
    out << "Upgradeability";
    return out;
}

QList<Offer*> DatabaseConnector::loadOffers(bool* okay) const
{
    QList<Offer*> availableOffers;
    QStringList  essentialSentimentDimensions = buildEssentialSentimentDimensions();
    QStringList  optionalSentimentDimensions = buildOptionalSentimentDimensions();
    *okay = false;

    std::unique_ptr<mongo::DBClientCursor> cursor = c->query("legilimens.laptops", mongo::BSONObj());

    int products = 0;
    while (cursor->more()) {
        mongo::BSONObj l = cursor->next();
        QHash<QString, Record > records;
        //get common fields
        //QString id = l.getStringField("_id");
        //int price = l.getIntField("price");
        //int rating = l.getIntField("rating");

        //int rank = INT_MAX;
        //if (l.hasElement("Bestseller Rank"))
            //rank = QString::fromStdString(l.getStringField("Bestseller Rank")).toInt();
        std::vector<mongo::BSONElement> elements;
        l.elems(elements);
        for (auto i = elements.begin(); i != elements.end(); ++i) {
            mongo::BSONElement& property = *i;
            QString fieldName(QString::fromUtf8(property.fieldName()));
            if (!property.isSimpleType()) {
                //qWarning() << "Skipping complex property: " << fieldName;
                continue;
            }
            QVariant data;
            switch(property.type()) {
            case mongo::BSONType::NumberDouble:
                data = property.Double();
                break;
            case mongo::BSONType::NumberInt:
                data = property.Int();
                break;
            case mongo::BSONType::String:
                data = QString::fromStdString(property.String()).replace('_', ' ');
                break;
            case mongo::BSONType::Bool:
                data = property.Bool();
                break;
            default:
                qWarning() << "Unhandled data type: " << property.type();
            }
            Record record = AttributeFactory::getInstance()->getAttribute(fieldName, data, true);
            if (record.second.isNull()) {
                //qWarning() << "Failed to deserialize: " << fieldName << data;
                continue;
            }
            //qDebug() << fieldName << " = " << record.second->toString();// << " (" << data << ")";
            records.insert(fieldName, record);
        }

        mongo::BSONObj storageObjects = l.getObjectField("storageMedia");
        std::list<mongo::BSONElement> storageElements;
        storageObjects.elems(storageElements);
        QVariantList extractedStorageElements;
        for (std::list<mongo::BSONElement>::const_iterator i = storageElements.begin(); i != storageElements.end(); ++i) {
            QVariantList extractedStorageElement;
            const mongo::BSONObj& storageObject((*i).Obj());
            QString type = storageObject.getStringField("type");
            int capacity = storageObject.getIntField("capacity");
            extractedStorageElement.append(type);
            extractedStorageElement.append(capacity);
            extractedStorageElements.append(QVariant::fromValue(extractedStorageElement));

        }
        Record storageElementsRecord = AttributeFactory::getInstance()->getAttribute("storageMedia", QVariant::fromValue(extractedStorageElements), true);
        records.insert("storageMedia", storageElementsRecord);

        QVariantList opticalDrives;
        mongo::BSONObj opticalDrivesObjects = l.getObjectField("opticalMediaTypes");
        std::list<mongo::BSONElement> opticalDrivesElements;
        opticalDrivesObjects.elems(opticalDrivesElements);
        for (std::list<mongo::BSONElement>::const_iterator i = opticalDrivesElements.begin(); i != opticalDrivesElements.end(); ++i) {
            opticalDrives << QString::fromStdString((*i).String());
        }
        records.insert("opticalMediaTypes", AttributeFactory::getInstance()->getAttribute("opticalMediaTypes", QVariant::fromValue(opticalDrives), true));

        QVariantList connectivity;
        mongo::BSONObj connectivityObjects = l.getObjectField("connectivity");
        std::list<mongo::BSONElement> connectivityElements;
        connectivityObjects.elems(connectivityElements);
        for (std::list<mongo::BSONElement>::const_iterator i = connectivityElements.begin(); i != connectivityElements.end(); ++i) {
            connectivity << QString::fromStdString((*i).String());
        }
        records.insert("connectivity", AttributeFactory::getInstance()->getAttribute("connectivity", QVariant::fromValue(connectivity), true));

        QVariantList ports;
        mongo::BSONObj portsObjects = l.getObjectField("ports");
        std::list<mongo::BSONElement> portsElements;
        portsObjects.elems(portsElements);
        for (std::list<mongo::BSONElement>::const_iterator i = portsElements.begin(); i != portsElements.end(); ++i) {
            ports << QString::fromStdString((*i).String());
        }
        records.insert("ports", AttributeFactory::getInstance()->getAttribute("ports", QVariant::fromValue(ports), true));

        QVariantList memoryCardTypes;
        mongo::BSONObj memoryCardTypeObjects = l.getObjectField("memoryCardTypes");
        std::list<mongo::BSONElement> memoryCardElements;
        memoryCardTypeObjects.elems(memoryCardElements);
        for (std::list<mongo::BSONElement>::const_iterator i = memoryCardElements.begin(); i != memoryCardElements.end(); ++i) {
            memoryCardTypes << QString::fromStdString((*i).String());
        }
        records.insert("memoryCardTypes", AttributeFactory::getInstance()->getAttribute("memoryCardTypes", QVariant::fromValue(memoryCardTypes), true));

        /*
         * Really don't need the reviews here
        mongo::BSONObj reviewObjects = l.getObjectField("reviews");
        std::list<mongo::BSONElement> reviewElements;
        reviewObjects.elems(reviewElements);
        for (std::list<mongo::BSONElement>::const_iterator i = reviewElements.begin(); i != reviewElements.end(); ++i) {
            const mongo::BSONObj& reviewObject((*i).Obj());
            QString review = reviewObject.getStringField("review");
            int rating = reviewObject.getIntField("rating");
        }*/

        mongo::BSONObj distanceObjects = l.getObjectField("distance");

        mongo::BSONObjIterator distanceFieldIteror(distanceObjects);
        QHash<QString, double> productDistances;
        while(distanceFieldIteror.more())
        {
          mongo::BSONElement distanceElement(distanceFieldIteror.next());
          QString otherId(QString::fromUtf8(distanceElement.fieldName()));
          double distance = distanceElement.Double();
          productDistances.insert(otherId, distance);
        }

        QStringList imageSrcs;
        std::vector<mongo::BSONElement> imageSrcsElements = l.getField("imageSrcs").Array();
        for (std::vector<mongo::BSONElement>::const_iterator i = imageSrcsElements.begin(); i != imageSrcsElements.end(); ++i) {
            imageSrcs << QString::fromStdString((*i).String());
        }

        SentimentMap extractedSentiment;
        mongo::BSONObj userSentiments = l.getObjectField("userSentiment");
        std::set<std::string> extractedSentimentFieldNames;
        userSentiments.getFieldNames(extractedSentimentFieldNames);
        int essentialsCovered = 0;
        for (std::set<std::string>::const_iterator i = extractedSentimentFieldNames.begin(); i != extractedSentimentFieldNames.end(); ++i) {
            QString key = QString::fromStdString(*i);
            if (essentialSentimentDimensions.contains(key))
                ++essentialsCovered;
            else if (!optionalSentimentDimensions.contains(key))
                continue;

            const Aspect* thisAspect(AspectFactory::getInstance()->getAspect(key));
            double value = userSentiments.getField(*i).numberDouble();
            if (thisAspect)
                extractedSentiment.insert(Aspect(*thisAspect), value);
            else
                qWarning() << "Skipping unknown aspect: " << key;
        }
        QSharedPointer<Attribute> name (new StringAttribute(true, false, QString("%1 %2").arg(
                                                                records.value("manufacturer").second->toString()).arg(
                                                                records.value("model").second->toString())));
        QSharedPointer<Attribute> price (records.take("price").second);
        QSharedPointer<Attribute> rating (records.take("rating").second);

        double speed;
        speed = records.value("processorFrequency").second->value();
        double cores = records.value("processorCores").second->value();
        speed *= cores;
        if (cores < 1)
            cores = 1;
        //qDebug() << " dedicated graphics memory: " << records.value("dedicatedGraphicsMemoryCapacity").second->value();
        Record processorSpeedAttribute = AttributeFactory::getInstance()->getAttribute("processorSpeed", speed, true);
        records.insert("processorSpeed", processorSpeedAttribute);

        QSharedPointer<Attribute> bestsellerRank(records.take("Bestseller Rank").second);
        int priorRank = INT_MAX;
        if (!bestsellerRank.isNull())
            priorRank = bestsellerRank->value();

        if (imageSrcs.empty())
            continue;

        //if (essentialsCovered < 2) {
        //    qWarning() << "Poor sentiment coverage for offer " << name->toString();
        //    continue;
        //}

        availableOffers << new Offer(name, price, rating, priorRank, imageSrcs, records, extractedSentiment, productDistances);
        ++products;
    }

    //qDebug() << "Median processor speed: " << AttributeFactory::getInstance()->getMedianInstance("processorSpeed")->toString();
    //qDebug() << "Median dedicated graphics memory: " << AttributeFactory::getInstance()->getMedianInstance("dedicatedGraphicsMemoryCapacity")->toString();
    qDebug() << "Total products: " << products;
    *okay = true;


#ifdef BUILD_SIMILARITY_CACHE
    foreach (Offer* o, availableOffers) {
        qDebug() << "Analyzing offer " << o->getName();
        foreach (Offer* b, availableOffers) {
            if (o == b)
                continue;

            double productDistance = 0;
            const RecordMap& r = b->getRecords();
            int recordCount = 0;
            for (RecordMap::const_iterator i = r.begin(); i != r.end(); ++i) {
                QString fieldId = i.key();
                if (!(*i).second->getShownByDefault() && fieldId != "manufacturer") {
                    //qDebug() << "              " << fieldId;
                    continue;
                } else {
                    ++recordCount;
                    //qDebug() << fieldId;
                }
                Record offerRecord = o->getRecord(fieldId);
                if (offerRecord.first.isNull()) {
                    productDistance += 0.1;
                    continue;
                }

                double thisAttributeDistance = qAbs(offerRecord.second->distance(*(i.value().second)));
                productDistance += thisAttributeDistance;
            }
            double normalizedProductDistance = (productDistance / recordCount);
            QString oId = o->getRecord("_id").second->toString();
            QString bId = b->getRecord("_id").second->toString();
            c->update("legilimens.laptops", BSON("_id" << oId.toUtf8().constData()),
                      BSON("$set" << BSON(QString("distance."+bId.replace('.', '-')).toUtf8().constData() << normalizedProductDistance)));
        }
    }
    qDebug() << "Distance calculation complete";

    exit(0);
#endif

    return availableOffers;
}
