#ifndef ATTRIBUTECREATORS_H
#define ATTRIBUTECREATORS_H
#include "numericalattribute.h"
#include "stringattribute.h"
#include "booleanattribute.h"
#include "compoundattribute.h"
#include "listattribute.h"
#include <cfloat>
#include <QStringList>
#include <QVariantList>

class AttributeCreator
{
public:
    AttributeCreator(bool internal, bool shownByDefault) :
        m_internal(internal), m_shownByDefault(shownByDefault)
    {}
    virtual ~AttributeCreator() {}
    /**
     * @p inDomain Set this to true, if the attribute you are instantiating is to become
     *             part of an offer from the domain base (influences best and worst instance results)
     */
    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data, bool inDomain = false) = 0;
    virtual QSharedPointer<Attribute> getBestInstance() = 0;
    virtual QSharedPointer<Attribute> getWorstInstance() = 0;
    virtual QSharedPointer<Attribute> getLargestInstance() {
        return QSharedPointer<Attribute>();
    }
    virtual QSharedPointer<Attribute> getSmallestInstance() {
        return QSharedPointer<Attribute>();
    }
protected:
    bool m_internal;
    bool m_shownByDefault;
};

class NumericalAttributeCreator : public AttributeCreator
{
public:
    NumericalAttributeCreator(bool internal, bool shownByDefault, const QString& format,
                              NumericalAttribute::Optimality optimality, double multiplier) :
        AttributeCreator(internal, shownByDefault),
        m_format(format), m_optimality(optimality), m_multiplier(multiplier)
    {
        if (m_optimality == NumericalAttribute::Min) {
            m_bestValue = DBL_MAX;
            m_worstValue = 0;
        } else {
            m_bestValue = 0;
            m_worstValue = DBL_MAX;
        }
    }

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data, bool inDomain = false) {
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
        if (inDomain)
            value *= m_multiplier;

        if (okay && inDomain) {
            if (m_optimality == NumericalAttribute::Min) {
                if (value < m_bestValue)
                    m_bestValue = value;
                if (value > m_worstValue)
                    m_worstValue = value;
            }
            if (m_optimality == NumericalAttribute::Max) {
                if (value > m_bestValue)
                    m_bestValue = value;
                if (value < m_worstValue)
                    m_worstValue = value;
            }
        }

        return (okay) ? QSharedPointer<NumericalAttribute>(new NumericalAttribute(m_internal, m_shownByDefault,
                                                                                  value, m_format, m_optimality,
                                                                                  m_bestValue < m_worstValue ? m_bestValue : m_worstValue,
                                                                                  m_bestValue > m_worstValue ? m_bestValue : m_worstValue))
                      : QSharedPointer<NumericalAttribute>();
    }
    QSharedPointer<Attribute> getBestInstance() {
        return getAttribute(m_bestValue);
    }
    QSharedPointer<Attribute> getWorstInstance() {
        return getAttribute(m_worstValue);
    }
    QSharedPointer<Attribute> getLargestInstance() {
        return getAttribute(m_bestValue > m_worstValue ? m_bestValue : m_worstValue);
    }
    QSharedPointer<Attribute> getSmallestInstance() {
        return getAttribute(m_bestValue < m_worstValue ? m_bestValue : m_worstValue);
    }

protected:
    QString m_format;
    NumericalAttribute::Optimality m_optimality;
    double m_multiplier;

    double m_bestValue;
    double m_worstValue;
};

class StringAttributeCreator : public AttributeCreator
{
public:
    StringAttributeCreator(const QString& optimality, const QString& worst, bool internal, bool shownByDefault) :
        AttributeCreator(internal, shownByDefault), optimality(optimality), worst(worst)
    {}

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data, bool inDomain = false) {
        Q_UNUSED(inDomain);
        if (!data.canConvert(QVariant::String))
            return QSharedPointer<Attribute>();
        return QSharedPointer<Attribute>(new StringAttribute(m_internal, m_shownByDefault, data.toString()));
    }
    QSharedPointer<Attribute> getBestInstance() {
        return getAttribute(optimality);
    }
    QSharedPointer<Attribute> getWorstInstance() {
        return getAttribute(worst);
    }
private:
    QString optimality;
    QString worst;
};

class BooleanAttributeCreator : public AttributeCreator
{
public:
    BooleanAttributeCreator(bool optimality, bool internal, bool shownByDefault) :
        AttributeCreator(internal, shownByDefault), optimality(optimality)
    {}

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data, bool inDomain = false) {
        Q_UNUSED(inDomain);
        if (!data.canConvert(QVariant::Bool)) {
            qDebug() << "Can not convert to bool: " << data.toString();
            return QSharedPointer<Attribute>();
        }
        return QSharedPointer<Attribute>(new BooleanAttribute(m_internal, m_shownByDefault, data.toBool()));
    }
    QSharedPointer<Attribute> getBestInstance() {
        return getAttribute(optimality);
    }
    QSharedPointer<Attribute> getWorstInstance() {
        return getAttribute(!optimality);
    }
private:
    bool optimality;

};


class CompoundAttributeCreator : public AttributeCreator
{
public:
    CompoundAttributeCreator(bool internal, bool shownByDefault, const QString& separator, AttributeCreator* childCreator) :
        AttributeCreator(internal, shownByDefault), m_separator(separator), m_childCreator(childCreator)
    {
    }
    ~CompoundAttributeCreator() {
        delete m_childCreator;
    }

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data, bool inDomain = false) {
        if (!data.canConvert(QVariant::String) || !m_childCreator)
            return QSharedPointer<Attribute>();
        QList<QSharedPointer<Attribute> > childCommands;
        Relationship::Type relationships;
        foreach (const QString& childCommandString, data.toString().split(m_separator)) {
            QSharedPointer<Attribute> child = m_childCreator->getAttribute(childCommandString, inDomain);
            if (!child)
                return QSharedPointer<Attribute>();
            childCommands.append(child);
            relationships |= child->getDefinedFor();
        }
        // build
        return QSharedPointer<Attribute>(new CompoundAttribute(m_internal, m_shownByDefault, m_separator, relationships, childCommands));
    }
    QSharedPointer<Attribute> getBestInstance() {
        return QSharedPointer<Attribute>();
    }
    QSharedPointer<Attribute> getWorstInstance() {
        return QSharedPointer<Attribute>();
    }

private:
    QString m_separator;
    AttributeCreator *m_childCreator;
};


class ListAttributeCreator : public AttributeCreator
{
public:
    ListAttributeCreator(bool internal, bool shownByDefault, QList<AttributeCreator *> childCreators) :
        AttributeCreator(internal, shownByDefault), m_childCreators(childCreators)
    {
    }
    ~ListAttributeCreator() {
        qDeleteAll(m_childCreators);
    }

    AttributeCreator* getCreator(int index) {
        if (index < 0 || index > m_childCreators.count())
            return 0;
        return m_childCreators[index];
    }

    virtual QSharedPointer<Attribute> getAttribute(const QVariant& data, bool inDomain = false) {
        if (!data.canConvert<QVariantList>() || m_childCreators.contains(0))
            return QSharedPointer<Attribute>();
        QVariantList childDescriptions = data.value<QVariantList>();
        if (childDescriptions.size() % m_childCreators.size() != 0)
            return QSharedPointer<Attribute>();

        // assumption: longer lists are better
        if (inDomain) {
            if (childDescriptions.size() > m_optimality.size())
                m_optimality = childDescriptions;
        }

        QList<QSharedPointer<Attribute> > childCommands;
        int i = 0;
        foreach (const QVariant& data, childDescriptions) {
            childCommands << m_childCreators[i % m_childCreators.count()]->getAttribute(data, inDomain);
            ++i;
        }
        return QSharedPointer<Attribute>(new ListAttribute(m_internal, m_shownByDefault, childCommands));
    }
    QSharedPointer<Attribute> getBestInstance() {
        return getAttribute(m_optimality);
    }
    QSharedPointer<Attribute> getWorstInstance() {
        return getAttribute(QVariantList());
    }
    QSharedPointer<Attribute> getLargestInstance() {
        return getBestInstance();
    }
private:
    QList<AttributeCreator *> m_childCreators;
    QVariantList m_optimality;
};


#endif // ATTRIBUTECREATORS_H
