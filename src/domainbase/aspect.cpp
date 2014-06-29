#include "aspect.h"
#include <QHash>
#include <QDebug>

uint qHash(const Aspect& a) {
    return qHash(a.id());
}
