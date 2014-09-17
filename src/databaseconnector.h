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
    ~DatabaseConnector();
    bool init();
    QList<Offer*> loadOffers(bool *okay) const;

private:
    mongo::DBClientConnection* c;
};

#endif // DATABASECONNECTOR_H
