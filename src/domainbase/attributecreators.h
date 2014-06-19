#ifndef ATTRIBUTECREATORS_H
#define ATTRIBUTECREATORS_H
#include "numericalattribute.h"
#include "stringattribute.h"
#include "booleanattribute.h"
#include "compoundattribute.h"
#include "listattribute.h"
#include <QStringList>
#include <QVariantList>

class AttributeCreator
{
public:
    AttributeCreator(bool internal) : m_internal(internal)
    {}
    virtual ~AttributeCreator() {}
    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data) const = 0;
protected:
    bool m_internal;
};

class NumericalAttributeCreator : public AttributeCreator
{
public:
    NumericalAttributeCreator(bool internal, const QString& format, double multiplier) :
        AttributeCreator(internal),
        m_format(format), m_multiplier(multiplier)
    {}

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data) const {
        double value = 0;
        bool okay = false;
        QString dataStr;
        if (data.canConvert(QVariant::String)) {
            dataStr = data.toString();
            if (dataStr.contains('/')) {
                QString preStr = dataStr.section('/', 0, 0);
                double pre = preStr.toDouble(&okay);

                double post = okay ? dataStr.mid(preStr.size() + 1).toDouble(&okay) : 0;
                if (okay) {
                    value = pre / post;
                    okay = true;
                } else {
                    qDebug() << "failed: " << dataStr << preStr;
                }
            }
            dataStr = dataStr.replace(" . ", QLocale::c().decimalPoint());
            dataStr = dataStr.replace(" , ", QLocale::c().decimalPoint());
            dataStr.replace(" ", QLocale::c().decimalPoint()); //allows "One Twenty" to be converted to 1.20
        }
        if (!okay && !dataStr.isNull()) {
            value = dataStr.toDouble(&okay);
        }
        if (!okay && data.canConvert(QVariant::Double)) {
            value = data.toDouble();
            okay = true;
        }
        value *= m_multiplier;

        return (okay) ? QSharedPointer<NumericalAttribute>(new NumericalAttribute(m_internal, value, m_format)) : QSharedPointer<NumericalAttribute>();
    }


protected:
    QString m_format;
    double m_multiplier;
};

class StringAttributeCreator : public AttributeCreator
{
public:
    StringAttributeCreator(bool internal) :
        AttributeCreator(internal)
    {}

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data) const {
        if (!data.canConvert(QVariant::String))
            return QSharedPointer<Attribute>();
        return QSharedPointer<Attribute>(new StringAttribute(m_internal, data.toString()));
    }
};

class BooleanAttributeCreator : public AttributeCreator
{
public:
    BooleanAttributeCreator(bool internal) :
        AttributeCreator(internal)
    {}

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data) const {
        if (!data.canConvert(QVariant::Bool)) {
            qDebug() << "Can not convert to bool: " << data.toString();
            return QSharedPointer<Attribute>();
        }
        return QSharedPointer<Attribute>(new BooleanAttribute(m_internal, data.toBool()));
    }
};


class CompoundAttributeCreator : public AttributeCreator
{
public:
    CompoundAttributeCreator(bool internal, const QString& separator, AttributeCreator* childCreator) :
        AttributeCreator(internal), m_separator(separator), m_childCreator(childCreator)
    {
    }
    ~CompoundAttributeCreator() {
        delete m_childCreator;
    }

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data) const {
        if (!data.canConvert(QVariant::String) || !m_childCreator)
            return QSharedPointer<Attribute>();
        QList<QSharedPointer<Attribute> > childCommands;
        Relationship::Type relationships;
        foreach (const QString& childCommandString, data.toString().split(m_separator)) {
            QSharedPointer<Attribute> child = m_childCreator->getAttribute(childCommandString);
            if (!child)
                return QSharedPointer<Attribute>();
            childCommands.append(child);
            relationships |= child->getDefinedFor();
        }
        // build
        return QSharedPointer<Attribute>(new CompoundAttribute(m_internal, m_separator, relationships, childCommands));
    }

private:
    QString m_separator;
    AttributeCreator *m_childCreator;
};


class ListAttributeCreator : public AttributeCreator
{
public:
    ListAttributeCreator(bool internal, QList<AttributeCreator *> childCreators) :
        AttributeCreator(internal), m_childCreators(childCreators)
    {
    }
    ~ListAttributeCreator() {
        qDeleteAll(m_childCreators);
    }

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data) const {
        if (!data.canConvert<QVariantList>() || m_childCreators.contains(0))
            return QSharedPointer<Attribute>();
        QVariantList childDescriptions = data.value<QVariantList>();
        if (childDescriptions.size() % m_childCreators.size() != 0)
            return QSharedPointer<Attribute>();

        QList<QSharedPointer<Attribute> > childCommands;
        int i = 0;
        foreach (const QVariant& data, childDescriptions) {
            childCommands << m_childCreators[i % m_childCreators.count()]->getAttribute(data);
            ++i;
        }
        return QSharedPointer<Attribute>(new ListAttribute(m_internal, childCommands));
    }
private:
    QList<AttributeCreator *> m_childCreators;
};


#endif // ATTRIBUTECREATORS_H
