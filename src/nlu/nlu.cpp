#include "nlu.h"
#include "statement.h"
#include "commandstatement.h"
#include "constraintstatement.h"
#include "domainbase/relationship.h"
#include "domainbase/offer.h"
#include "domainbase/attribute.h"
#include "domainbase/attributefactory.h"
#include <QRegExp>
#include <QFile>
#include <QStringList>
#include <QDomElement>
#include <QDebug>

/// Call this macro to process the pending modifier as a standalone one (with default target) if possible
/// Will set the passed pointer to 0 after processing
#define FINALIZE_PENDING_MODIFIER(x)  \
if (x) { \
    relationships << buildRelationship(currentRecommendation, 0, QString(), x, factorTillPendingModifier); \
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


Relationship* NLU::buildRelationship(const Offer *offer,
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


bool NLU::setupLanguage(const QString& path)
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

NLU::NLU(AttributeFactory* attributeFactory) : m_attributeFactory(attributeFactory)
{
}

QList<Statement*> NLU::interpret(const Offer *currentRecommendation, const QString& command)
{
    QList<Statement*> foundStatements;

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
                            attribute = currentRecommendation->getAttribute(a.m_on);
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
                        Relationship *r = buildRelationship(currentRecommendation, attributeToken, matchedAttribute, pendingModifier, factor);
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
                            r = buildRelationship(currentRecommendation, pendingAttribute, matchedAttribute, modifierToken, factor);;
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

    // build Statements and return those
    if (undoLast) {
        qDebug() << "Undoing last feedback cycle";
        foundStatements.append(new CommandStatement(CommandStatement::Back));
    } else {
        qDebug() << "built " << relationships.count() << " relationships:";
        foreach (Relationship* r, relationships)
            foundStatements.append(new ConstraintStatement(r));
        /*
         * No further handling of misunderstandings at this stage...
        else {
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
            emit recommendationChanged(currentRecommendation, explanation);
        }
        */
    }



    return foundStatements;
}
