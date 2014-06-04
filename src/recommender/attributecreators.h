#ifndef ATTRIBUTECREATORS_H
#define ATTRIBUTECREATORS_H
#include "numericalattribute.h"
#include "stringattribute.h"
#include "compoundattribute.h"
#include <QStringList>

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
    NumericalAttributeCreator(bool internal, bool minSet, double min, bool maxSet, double max) :
        AttributeCreator(internal),
        m_minSet(minSet), m_min(min), m_maxSet(maxSet), m_max(max)
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

        return (okay) ? QSharedPointer<NumericalAttribute>(new NumericalAttribute(m_internal, value, m_minSet, m_min, m_maxSet, m_max)) : QSharedPointer<NumericalAttribute>();
    }

    bool getMinSet() const { return m_minSet; }
    bool getMaxSet() const { return m_maxSet; }
    double getMin() const { return m_min; }
    double getMax() const { return m_max; }

protected:
    bool m_minSet;
    double m_min;

    bool m_maxSet;
    double m_max;
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

class CompoundAttributeCreator : public AttributeCreator
{
public:
    CompoundAttributeCreator(bool internal, bool minSet, double min, bool maxSet, double max, const QString& separator, const QString& type) :
        AttributeCreator(internal), m_separator(separator), m_type(type)
    {
        if (m_type == "double")
            m_childCreator = new NumericalAttributeCreator(true, minSet, min, maxSet, max);
        else if (m_type == "symbol")
            m_childCreator = new StringAttributeCreator(true);
    }
    ~CompoundAttributeCreator() {
        delete m_childCreator;
    }

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data) const {
        if (!data.canConvert(QVariant::String) || !m_childCreator)
            return QSharedPointer<Attribute>();
        QList<QSharedPointer<Attribute> > childCommands;
        foreach (const QString& childCommandString, data.toString().split(m_separator)) {
            QSharedPointer<Attribute> child = m_childCreator->getAttribute(childCommandString);
            if (!child)
                return QSharedPointer<Attribute>();
            childCommands.append(child);
        }
        // build
        return QSharedPointer<Attribute>(new CompoundAttribute(m_internal, m_separator, m_type, childCommands));
    }

    bool getMinSet() const {
        NumericalAttributeCreator *num = dynamic_cast<NumericalAttributeCreator*>(m_childCreator);
        if (num) return num->getMinSet();
        return false;
    }
    bool getMaxSet() const {
        NumericalAttributeCreator *num = dynamic_cast<NumericalAttributeCreator*>(m_childCreator);
        if (num) return num->getMaxSet();
        return false;
    }
    double getMin() const {
        NumericalAttributeCreator *num = dynamic_cast<NumericalAttributeCreator*>(m_childCreator);
        if (num) return num->getMin();
        return 0;
    }
    double getMax() const {
        NumericalAttributeCreator *num = dynamic_cast<NumericalAttributeCreator*>(m_childCreator);
        if (num) return num->getMax();
        return 0;
    }
private:
    AttributeCreator *m_childCreator;
    QString m_separator;
    QString m_type;
};


#endif // ATTRIBUTECREATORS_H
