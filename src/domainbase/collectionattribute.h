#ifndef COLLECTIONATTRIBUTE_H
#define COLLECTIONATTRIBUTE_H

#include "attribute.h"

class CollectionAttribute
{
public:
    virtual QSharedPointer<Attribute> getChild(int i) const = 0;
};

#endif // COLLECTIONATTRIBUTE_H
