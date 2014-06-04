#include "spencer.h"
#include "recommender/critiquerecommender.h"
#include "recommender/offer.h"
#include "recommender/numericalattribute.h"
#include "recommender/relationship.h"
#include "recommender/attributefactory.h"
#include "recommender/stringattribute.h"
#include <QHash>
#include <QFile>
#include <QSet>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QDebug>
#include <QRegExp>
#include <QFlags>

static const QString dbPath = "app/native/res/db/";
static const QString modelName = "Modell";
static const QString manufacturerName = "Hersteller";
static const QString imageName = "Bild";

/// Call this macro to process the pending modifier as a standalone one (with default target) if possible
/// Will set the passed pointer to 0 after processing
#define FINALIZE_PENDING_MODIFIER(x)  \
if (x) { \
    relationships << buildRelationship(m_currentRecommendation, 0, QString(), x, factorTillPendingModifier); \
    x = 0; \
}

/// Call this macro to process the pending attributes a standalone one (with value) if possible
/// Will set the passed pointer to 0 after processing
// we have an attribute and an associated value
// default to an equality association.
// This allows e.g., "100 EURO" to become Price == 100 EURO
#define FINALIZE_PENDING_ATTRIBUTE(x)  \
if (x) { \
    if (!matchedAttributeForPendingAttribute.isEmpty() && x) { \
        QSharedPointer<Attribute> attribute = m_attributeFactory->getAttribute(x->getTarget(), matchedAttributeForPendingAttribute); \
        if (!attribute.isNull()) {\
    qDebug() << "Adding relationship based on pending attribute" << x->getNames().first().pattern(); \
            relationships << new Relationship(x->getTarget(), Relationship::Equality, attribute, factor); \
        }\
    } \
    x = 0; \
}

class Token
{
public:
    Token(const QList<QRegExp>& names) : m_names(names) {}
    virtual ~Token() {}
    QList<QRegExp> getNames() const { return m_names; }

    virtual int matches(const QString& token) const {
        return matches(token, 0);
    }
    int matches(const QString &token, QString* matched) const {
        foreach (const QRegExp& regExp, m_names) {
            if (regExp.indexIn(token) == 0) {
                if (matched && regExp.captureCount() > 0)
                    *matched = regExp.cap(1);
                return regExp.matchedLength();
            }
        }
        return 0;
    }
    virtual QString toString() const = 0;

protected:
    QList<QRegExp> m_names;
};

class ModifierToken : public Token
{
public:
    enum BindingE {
        None=0,
        Pre=1,
        Post=2
    };
    Q_DECLARE_FLAGS(Binding, BindingE)


    ModifierToken(const QList<QRegExp>& names, const QStringList& on, const QString& relationship, Binding binding, const QString& defaultTarget) :
        Token(names), m_on(on), m_relationship(relationship), m_binding(binding), m_defaultTarget(defaultTarget)
    {}
    QStringList getOn() const { return m_on; }
    QString getRelationship() const { return m_relationship; }
    Binding getBinding() const { return m_binding; }
    QString getDefaultTarget() const { return m_defaultTarget; }
    QString toString() const {
        return QString("Modifier: %1 %2").arg(m_on.join(",")).arg(m_relationship);
    }

private:
    QStringList m_on;
    QString m_relationship;
    Binding m_binding;
    QString m_defaultTarget;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ModifierToken::Binding)

class MetaModifierToken : public Token
{
public:
    MetaModifierToken(const QList<QRegExp>& names, double factor) :
        Token(names), m_factor(factor)
    {}
    double getFactor() const { return m_factor; }
    QString toString() const {
        return QString("MetaModifier: %1").arg(m_factor);
    }

private:
    double m_factor;
};


class AttributeToken : public Token
{
public:
    AttributeToken(const QList<QRegExp>& names, const QString& type, const QString& target) :
        Token(names), m_type(type), m_target(target)
    {}
    QString getType() const { return m_type; }
    QString getTarget() const { return m_target; }
    QString toString() const {
        return QString("Attribute: %1").arg(m_target);
    }

private:
    QString m_type;
    QString m_target;
};


class CommandToken : public Token
{
public:
    class Action {
    public:
        QString m_relationship;
        QString m_on;
        bool m_isBack;

        Action() :
            m_isBack(true)
        {}

        Action(const QString& relationship, const QString& on) :
            m_relationship(relationship), m_on(on), m_isBack(false)
        {}
    };

    CommandToken(const QList<QRegExp>& names, const QList<Action>& actions) :
        Token(names), m_actions(actions)
    {}
    QList<Action> getActions() const { return m_actions; }
    QString toString() const {
        QStringList actions;
        foreach (const Action& a, m_actions)
            actions << QString("%1 %2").arg(a.m_on).arg(a.m_relationship);
        return QString("Command: %1").arg(actions.join(", "));
    }

private:
    QList<Action> m_actions;
};

static QList<QRegExp> parseNames(const QDomElement& namesElem)
{
    QList<QRegExp> names;
    QDomElement nameElem = namesElem.firstChildElement("name");
    while (!nameElem.isNull()) {
        names << QRegExp(QLatin1String("\\b") + nameElem.text() + QLatin1String("\\b"));
        nameElem = nameElem.nextSiblingElement("name");
    }
    return names;
}

static Relationship::Type getRelationshipType(const QString& relationshipTypeStr)
{
    Relationship::Type relationshipType;
    if (relationshipTypeStr == "Inequal")
        relationshipType = Relationship::Inequality;
    else if (relationshipTypeStr == "Equal")
        relationshipType = Relationship::Equality;
    else {
        if (relationshipTypeStr == "SmallerThan")
            relationshipType = Relationship::SmallerThan;// | Relationship::Inequality;
        else if (relationshipTypeStr == "LargerThan")
            relationshipType = Relationship::LargerThan;// | Relationship::Inequality;
    }
    return relationshipType;
}


Spencer::Spencer(QObject *parent) :
    QObject(parent),
    m_recommender(new CritiqueRecommender),
    m_attributeFactory(new AttributeFactory),
    m_currentRecommendation(0)
{
    connect(m_recommender, SIGNAL(recommend(const Offer*, QString)), this, SLOT(recommendationChanged(const Offer*, QString)));
}

Spencer::~Spencer()
{
    delete m_recommender;
}

QList<Offer*> Spencer::parseCasebase(const QString& path, const QString& imageBasePath, bool* okay) const
{
    QList<Offer*> availableOffers;
    *okay = false;

    QDomDocument doc;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file: " << path;
        return availableOffers;
    }
    if (!doc.setContent(f.readAll())) {
        qWarning() << "Failed to parse XML at " << path;
        return availableOffers;
    }

    QDomElement rootElement(doc.documentElement());
    QDomElement caseElement(rootElement.firstChildElement().firstChildElement("case"));

    //we need unique, descriptive names (user visible);
    // make sure we don't re-use them
    QSet<QString> uniqueNames;

    while (!caseElement.isNull()) {
        QHash<QString, QSharedPointer<Attribute> > attributes;
        QDomElement featureElement = caseElement.firstChildElement("feature");
        while (!featureElement.isNull()) {
            QDomElement nameElem = featureElement.firstChildElement("name");
            QDomElement valueElem = featureElement.firstChildElement("value");

            QString name = nameElem.text();
            QString value = valueElem.text();
            //qDebug() << "Name: " << name << " value: " << value;

            QSharedPointer<Attribute> a = m_attributeFactory->getAttribute(name, value);
            if (!a) {
                qDebug() << "Attribute failed to parse" << name << value;
                return availableOffers;
            } else
                attributes.insert(name, a);
            featureElement = featureElement.nextSiblingElement("feature");
        }

        QString name;
        if (attributes.contains(modelName) && attributes.contains(manufacturerName))
            name = attributes.value(manufacturerName)->toString() + " " + attributes.value(modelName)->toString();
        else
            name = caseElement.attribute("id");

        name = name.trimmed();
        while (uniqueNames.contains(name)) {
            qDebug() << "Duplicate: " << name;
            name += '_';
        }
        uniqueNames.insert(name);

        QPixmap image(imageBasePath+attributes.value("Bild")->toString());
        float priorProbability = 0;
        //add prior probability to top 100
        if (attributes.contains("Rang"))
            priorProbability = 0.005 * (1 - (attributes.value("Rang").staticCast<NumericalAttribute>())->getValue() / 100);

        availableOffers << new Offer(name, priorProbability, image, m_attributeFactory->getAttributeNames(), attributes);
        caseElement = caseElement.nextSiblingElement("case");
    }

    *okay = true;
    return availableOffers;
}

#ifdef PRINT_NLP
static void printNames(const QStringList& names, const QStringList& metaModifier)
{
    foreach (const QString& name, names) {
        qDebug() << name;
        foreach (const QString& mod, metaModifier) {
            qDebug() << mod << " " << name;
        }
    }
}
static QStringList toNames(const QList<QRegExp>& regs)
{
    QStringList out;
    foreach (const QRegExp& e, regs)
        out << e.pattern().remove("\\b");
    return out;
}
#endif

bool Spencer::init()
{
    if (!m_attributeFactory->parseStructure(dbPath + "structure.xml"))
        return false;
    if (!setupLanguage(dbPath + "nlp.xml"))
        return false;

#if PRINT_NLP
    qDebug() << "======================================";
    QHash<QString, QStringList> attributesByType;
    QStringList sentences;
    QStringList metaModifiers;
    foreach (Token *t, m_acceptedTokens) {
        if (dynamic_cast<CommandToken*>(t))
            sentences << toNames(t->getNames());

        if (dynamic_cast<MetaModifierToken*>(t)) {
            //print directly first
            QStringList modifierNames(toNames(t->getNames()));
            printNames(modifierNames, QStringList());
            metaModifiers << modifierNames;
        }

        if (dynamic_cast<AttributeToken*>(t)) {
            AttributeToken *aT = static_cast<AttributeToken*>(t);
            attributesByType.insertMulti(aT->getType(), toNames(aT->getNames()));
        }
    }
    foreach (Token *t, m_acceptedTokens) {
        if (dynamic_cast<ModifierToken*>(t)) {
            ModifierToken *mT = static_cast<ModifierToken*>(t);
            foreach (const QString& on, mT->getOn()) {
                foreach (const QStringList& names, attributesByType.values(on)) {
                    foreach (const QString& name, names) {
                        foreach (const QString& modName, toNames(mT->getNames())) {
                            if (mT->getBinding() & ModifierToken::Pre)
                                sentences << name + QLatin1String(" ") + modName;
                            else
                                sentences << modName + QLatin1String(" ") + name;
                        }
                    }
                }
            }
        }

    }
    printNames(sentences, metaModifiers);

    qDebug() << "======================================";
#endif

    bool okay;
    QList<Offer*> availableOffers = parseCasebase(dbPath + "casebase.xml",
                                                  dbPath + "img/", &okay);
    if (!okay)
        return false;

    m_recommender->setupDatabase(availableOffers);
    m_recommender->init();
    return true;
}

void Spencer::reset()
{
    m_recommender->init();
}

QString Spencer::critique(const QString& command)
{
    qDebug() << "Setting up critique(s) based on command: " << command;
    QString extractedCritiques;

    //some random attribute from the text
    // might be null after the parsing if no attribute was found
    AttributeToken *extractedAttribute = 0;

    QString tempText(command);
    QList<Relationship*> relationships;
    //matches the word at the beginning of the text
    QRegExp dummyToken("^[^\\s]+(\\s+|$)");

    int matched = 0;
    AttributeToken *pendingAttribute = 0;
    ModifierToken *pendingModifier = 0;
    double factor = 1.0;
    double factorTillPendingModifier = factor;

    // do not undo more than one feedback cycle at once
    bool undoLast = false;
    QString matchedAttribute;
    QString matchedAttributeForPendingAttribute;
    while (!tempText.isEmpty()) {
        bool fragmentComplete = false;
        foreach (Token *t, m_acceptedTokens) {
            QString thisAttributeMatch;
            if ((matched = t->matches(tempText, &thisAttributeMatch)) != 0) {
                if (!thisAttributeMatch.isNull())
                    matchedAttribute = thisAttributeMatch;
                //found current token:
                qDebug() << "Token: " << t->toString() << matchedAttributeForPendingAttribute;

                if (dynamic_cast<CommandToken*>(t)) {
                    FINALIZE_PENDING_MODIFIER(pendingModifier);
                    FINALIZE_PENDING_ATTRIBUTE(pendingAttribute);

                    foreach (const CommandToken::Action& a, static_cast<CommandToken*>(t)->getActions()) {
                        if (a.m_isBack) {
                            undoLast = true;
                            continue;
                        }

                        QSharedPointer<Attribute> attribute;
                        if (!matchedAttribute.isEmpty())
                            attribute = m_attributeFactory->getAttribute(a.m_on, matchedAttribute);
                        else
                            attribute = m_currentRecommendation->getAttribute(a.m_on);
                        if (!attribute) {
                            qWarning() << "Offer has no attribute " << a.m_on;
                            continue;
                        }

                        Relationship::Type relationshipType = getRelationshipType(a.m_relationship);
                        relationships << new Relationship(a.m_on, relationshipType, attribute, factor);
                    }

                    fragmentComplete = true;

                } else if (dynamic_cast<MetaModifierToken*>(t)) {
                    factor *= static_cast<MetaModifierToken*>(t)->getFactor();
                    if (!pendingModifier)
                        factorTillPendingModifier = factor;
                } else if (dynamic_cast<AttributeToken*>(t)) {
                    AttributeToken *attributeToken = static_cast<AttributeToken*>(t);
                    extractedAttribute = attributeToken;

                    FINALIZE_PENDING_ATTRIBUTE(pendingAttribute);
                    matchedAttributeForPendingAttribute.clear();
                    // this is an attribute
                    if (pendingModifier) {
                        // we have a post-binding modifier that's waiting
                        // for an attribute; connect and add if possible
                        Relationship *r = buildRelationship(m_currentRecommendation, attributeToken, matchedAttribute, pendingModifier, factor);
                        if (!r) {
                            //incompatible, let's try to add the modifier on it's own
                            FINALIZE_PENDING_MODIFIER(pendingModifier);
                            //the attribute now becomes pending
                            pendingAttribute = attributeToken;
                            matchedAttributeForPendingAttribute = thisAttributeMatch;
                        } else {
                            relationships << r;
                            pendingModifier = 0;
                            pendingAttribute = 0;
                            fragmentComplete = true;
                        }
                    } else {
                        pendingAttribute = attributeToken;
                        matchedAttributeForPendingAttribute = thisAttributeMatch;
                    }
                } else {
                    // this is a modifier
                    ModifierToken *modifierToken = static_cast<ModifierToken*>(t);
                    FINALIZE_PENDING_MODIFIER(pendingModifier);
                    if (pendingAttribute) {
                        Relationship *r = 0;
                        if (modifierToken->getBinding() & ModifierToken::Pre) {
                            //connect and add
                            r = buildRelationship(m_currentRecommendation, pendingAttribute, matchedAttribute, modifierToken, factor);;
                            if (r) {
                                relationships << r;
                                pendingAttribute = 0;
                                fragmentComplete = true;
                            }
                        }

                        //forget pending attribute; it was either processed or rejected
                        FINALIZE_PENDING_ATTRIBUTE(pendingAttribute);

                        if (!r) {
                            if (modifierToken->getBinding() & ModifierToken::Post)
                                pendingModifier = modifierToken;
                            else
                                continue; // rejected token, keep looking
                        }
                    } else {
                        if (modifierToken->getBinding() & ModifierToken::Post) {
                            pendingModifier = modifierToken;
                        } else {
                            FINALIZE_PENDING_MODIFIER(modifierToken);
                            matchedAttribute.clear();
                            continue; // keep looking
                        }
                    }
                }
                break;
            }
        }
        if (fragmentComplete) {
            factor = factorTillPendingModifier = 1.0;
            matchedAttribute.clear();
            matchedAttributeForPendingAttribute.clear();
        }
        if (!matched) {
            dummyToken.indexIn(tempText);
            matched = dummyToken.matchedLength();
        }

        tempText = tempText.mid(matched).trimmed();
    }
    FINALIZE_PENDING_MODIFIER(pendingModifier);
    FINALIZE_PENDING_ATTRIBUTE(pendingAttribute);

    relationships.removeAll(0);

    static int misunderstandingCounter = 0;
    qDebug() << "Misunderstanding counter: " << misunderstandingCounter;

    if (undoLast) {
        qDebug() << "Undoing last feedback cycle";
        extractedCritiques += QString::fromUtf8("Zurück\n");
        m_recommender->undo();
    } else {
        qDebug() << "built " << relationships.count() << " relationships:";
        if (!relationships.isEmpty()) {
            misunderstandingCounter = 0;
            foreach (Relationship* r, relationships) {
                qDebug() << r->toString();
                Critique *c = new Critique(r);
                QString descr = c->getDescription();
                extractedCritiques += descr + '\n';
                if (!m_recommender->critique(c))
                    emit noMatchFor(descr);
            }

            //based on the assumption that the user wouldn't have critizised the
            // model if he was interested in it, add a *slight* bias against it
            m_recommender->critique(new Critique(new Relationship(modelName,
                           Relationship::Inequality, m_currentRecommendation->getAttribute(modelName)), 0.1));
            if (!relationships.isEmpty())
                m_recommender->feedbackCycleComplete();
        } else {
            QString explanation;
            if (extractedAttribute != 0) {
                QString name = extractedAttribute->getTarget().replace(QRegExp("\\(.*\\)"), "").trimmed();
                QString sent;
                if (extractedAttribute->getType() == "countable")
                    sent = QLatin1String("Mehr ") + name;
                else if (extractedAttribute->getType() == "mitable")
                    sent = QLatin1String("Mit ") + name;
                else
                    sent = name + QString::fromUtf8(" ändern");

                explanation = tr("Versuchen Sie bspw. \"%1\" oder \"Billiger\"").arg(sent);
                ++misunderstandingCounter;
            } else if (misunderstandingCounter++ >= 1) {
                    explanation = tr("Versuchen Sie bspw. \"Anderer Hersteller\" oder \"Niedrigerer Preis\"");
            } else
                explanation = tr("Wie bitte?");
            emit recommendationChanged(m_currentRecommendation, explanation);
        }
    }
    return extractedCritiques;
}

void Spencer::recommendationChanged(const Offer *o, const QString& explanation)
{
    m_currentRecommendation = o;
    emit recommend(o, explanation);
}

bool Spencer::setupLanguage(const QString& path)
{
    qDeleteAll(m_acceptedTokens);
    m_acceptedTokens.clear();

    QDomDocument doc;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file: " << path;
        return false;
    }
    if (!doc.setContent(f.readAll())) {
        qWarning() << "Failed to parse XML at " << path;
        return false;
    }

    QDomElement rootElement(doc.documentElement());
    QDomElement attributesElement(rootElement.firstChildElement("attributes"));
    QDomElement modifiersElement(rootElement.firstChildElement("modifiers"));
    if (attributesElement.isNull() || modifiersElement.isNull()) {
        // no case or invalid format
        qWarning() << "Invalid XML: " << path;
        return false;
    }
    // attributes
    QDomElement attributeElem = attributesElement.firstChildElement("attribute");
    while (!attributeElem.isNull()) {
        QString type = attributeElem.attribute("type");
        QString actOn = attributeElem.firstChildElement("actOn").text();
        m_acceptedTokens << new AttributeToken(parseNames(attributeElem.firstChildElement("names")), type, actOn);
        attributeElem = attributeElem.nextSiblingElement("attribute");
    }
    // modifiers
    QDomElement modifierElement = modifiersElement.firstChildElement("modifier");
    while (!modifierElement.isNull()) {
        QString on = modifierElement.attribute("on");
        QString relationship = modifierElement.firstChildElement("relationship").text();
        QString bindingStr = modifierElement.attribute("binding");

        ModifierToken::Binding binding = 0;
        if (bindingStr.contains("pre"))
            binding |= ModifierToken::Pre;
        if (bindingStr.contains("post"))
            binding |= ModifierToken::Post;
        m_acceptedTokens << new ModifierToken(parseNames(modifierElement.firstChildElement("names")), on.split(","), relationship, binding, modifierElement.attribute("default"));
        modifierElement = modifierElement.nextSiblingElement("modifier");
    }

    // metamodifiers
    QDomElement metaModifiersElement(rootElement.firstChildElement("metaModifiers"));
    QDomElement metaModifierElement(metaModifiersElement.firstChildElement("metaModifier"));
    while (!metaModifierElement.isNull()) {
        QDomElement factorElem = metaModifierElement.firstChildElement("factor");
        bool okay = true;
        double factor = factorElem.text().toDouble(&okay);
        if (okay)
            m_acceptedTokens << new MetaModifierToken(parseNames(metaModifierElement.firstChildElement("names")), factor);
        else
            qWarning() << "Invalid meta modifier factor: " << factorElem.text();
        metaModifierElement = metaModifierElement.nextSiblingElement("metaModifier");
    }

    // commands
    QDomElement commandsElement(rootElement.firstChildElement("commands"));
    QDomElement commandElement(commandsElement.firstChildElement("command"));
    while (!commandElement.isNull()) {
        QDomElement actionsElem = commandElement.firstChildElement("actions");
        QDomElement actionElem = actionsElem.firstChildElement("action");
        QList<CommandToken::Action> actions;
        while (!actionElem.isNull()) {
            if (actionElem.attribute("special") == QLatin1String("BACK")) {
                actions << CommandToken::Action(); //special "undo" action
            } else {
                QString attribute = actionElem.firstChildElement("attribute").text();
                QString relationship = actionElem.firstChildElement("relationship").text();
                actions << CommandToken::Action(relationship, attribute);
            }
            actionElem = actionElem.nextSiblingElement("action");
        }
        m_acceptedTokens << new CommandToken(parseNames(commandElement.firstChildElement("names")), actions);
        commandElement = commandElement.nextSiblingElement("command");
    }
    return true;
}

Relationship* Spencer::buildRelationship(const Offer *offer,
                                                const AttributeToken* attributeToken,
                                                const QString& attributeValue,
                                                const ModifierToken* modifierToken, double modifierFactor) const
{
    qDebug() << "Building relationship";
    if (attributeToken && !modifierToken->getOn().contains(attributeToken->getType())) {
        qWarning() << "Incomplete type" << attributeToken->getNames().first().pattern() << attributeValue;
        return 0;
    }
    QString attributeName;
    if (attributeToken)
        attributeName = attributeToken->getTarget();
    else if (!modifierToken->getDefaultTarget().isEmpty()) {
        // "Modifier has a default attribute, so let's create that one if possible";
        attributeName = modifierToken->getDefaultTarget();
    } else
        qDebug() << "No attribute and empty default target";

    if (attributeName.isEmpty()) {
        qWarning() << "Invalid target attribute" << modifierToken->getNames().first().pattern();
        return 0;
    }

    QSharedPointer<Attribute> attribute;
    if (attributeValue.isNull())
        attribute = offer->getAttribute(attributeName);
    else {
        qDebug() << "Attribute value: " << attributeValue;
        attribute = m_attributeFactory->getAttribute(attributeName, attributeValue);
        qDebug() << "Got attribute value: " << attribute->toString();
    }
    if (!attribute) {
        qWarning() << "Offer has no attribute " << attributeName;
        return 0;
    }

    Relationship::Type relationshipType = getRelationshipType(modifierToken->getRelationship());
    return new Relationship(attributeName, relationshipType, attribute, modifierFactor);
}

AttributeFactory* Spencer::getAttributeFactory() const
{
    return m_attributeFactory;
}
