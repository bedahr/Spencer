#ifndef DATABASECONNECTOR_H
#define DATABASECONNECTOR_H

#include <QList>
class Offer;
class AttributeFactory;

namespace mongo {
    class DBClientConnection;
}

class DatabaseConnector
{
public:
    DatabaseConnector();
    bool init();
    QList<Offer*> loadOffers(AttributeFactory *factory, bool *okay) const;

private:
    mongo::DBClientConnection* c;
};

#endif // DATABASECONNECTOR_H
