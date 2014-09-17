#ifndef ASPECT_H
#define ASPECT_H

#include <QString>
#include <QList>
#include <QObject>

class Aspect
{
public:
    Aspect()
    {}
    explicit Aspect(const QString& id) :
        m_id(id), m_name(id)
    {}

    Aspect(const QString& id, const QString& name, const QList<Aspect*> children) :
        m_parent(0), m_id(id), m_name(name)
    {
        foreach (Aspect* c, children) {
            m_children << c;
            c->setParent(this);
        }
    }

    Aspect* parent() const { return m_parent; }
    QString id() const { return m_id; }
    QString name() const { return m_name; }
    QList<const Aspect*> children() const {
        return m_children;
    }

    void setParent(Aspect* p) {
        m_parent = p;
    }

    bool operator==(const Aspect& other) const {
        return m_id == other.m_id;
    }

private:
    Aspect* m_parent;
    QString m_id;
    QString m_name;
    QList<const Aspect*> m_children;
};

Q_DECLARE_METATYPE(Aspect);

uint qHash(const Aspect& a);

#endif // ASPECT_H
