#ifndef ASPECTFACTORY_H
#define ASPECTFACTORY_H

#include "aspect.h"
#include <QString>

class AspectFactory
{
public:
    /// Reads the given sentiment.xml to build the internal
    /// representation of available aspects and their
    /// hierarchie
    /// Call this before calling getAspect!
    bool parseStructure(const QString& path);

    /// Returns an instance of the aspect
    /// The returned aspect is null if an error occured (e.g. unassigned id)
    const Aspect* getAspect(const QString& id);

    /// Returns an instance of the aspect
    /// The returned aspect is null if an error occured (e.g. unassigned name)
    const Aspect* getAspectByName(const QString& name);

    /// Instance method for the Singleton
    static AspectFactory* getInstance() {
        if (!instance) instance = new AspectFactory;
        return instance;
    }

private:
    static AspectFactory* instance;

    QList<const Aspect*> m_aspects;

    AspectFactory() {}
};

#endif // ASPECTFACTORY_H
