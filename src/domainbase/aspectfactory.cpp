#include "aspectfactory.h"
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QFile>
#include <QDebug>

AspectFactory* AspectFactory::instance;


QList<Aspect*> parseAspects(const QDomElement& parent)
{
    QList<Aspect*> aspects;

    QDomElement aspectElement = parent.firstChildElement("aspect");
    while (!aspectElement.isNull()) {
        QDomElement idElem = aspectElement.firstChildElement("id");
        QDomElement nameElem = aspectElement.firstChildElement("name");
        QDomElement childrenElem = aspectElement.firstChildElement("children");

        QList<Aspect*> children = parseAspects(childrenElem);

        aspects << new Aspect(idElem.text(), nameElem.text(), children);

        aspectElement = aspectElement.nextSiblingElement("aspect");
    }

    return aspects;
}

bool AspectFactory::parseStructure(const QString& path)
{
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
    if (rootElement.tagName() != "aspects") {
        qWarning() << "Invalid XML: " << path;
        return false;
    }

    foreach (Aspect *a, parseAspects(rootElement))
        m_aspects << a;
    return true;
}

static const Aspect* findAspect(const QString& id, QList<const Aspect*> list, bool isId=true)
{
    foreach (const Aspect *c, list) {
        if ((isId && (c->id() == id)) || (!isId && (c->name().toUpper() == id.toUpper())))
            return c;
        const Aspect* found = findAspect(id, c->children(), isId);
        if (found != 0)
            return found;
    }
    return 0;
}

const Aspect* AspectFactory::getAspect(const QString& id)
{
    return findAspect(id, m_aspects);
}
const Aspect* AspectFactory::getAspectByName(const QString& name)
{
    return findAspect(name, m_aspects, false);
}
