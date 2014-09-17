#include "token.h"
#include "observedtoken.h"
#include "statement.h"
#include "aspectstatement.h"
#include "constraintstatement.h"
#include "commandstatement.h"
#include "usecasestatement.h"
#include "domainbase/relationship.h"
#include "domainbase/offer.h"
#include "domainbase/attribute.h"
#include "domainbase/attributefactory.h"
#include "domainbase/aspectfactory.h"
#include <QDebug>

static QSharedPointer<Attribute> getAttribute(const QString& target, const QString& value)
{
    return AttributeFactory::getInstance()->getAttribute(target, value).second;
}
static ConstraintStatement* getConstraintStatement(const QString& target, Relationship::Type type, QSharedPointer<Attribute> attribute, double factor, double quality = 1.0)
{
    return new ConstraintStatement(new Relationship(target, type, attribute, factor), 1.0, quality);
}
static ConstraintStatement* getConstraintStatement(const QString& target, Relationship::Type type, const QString& value, double factor, double quality = 1.0)
{
    QSharedPointer<Attribute> attribute = getAttribute(target, value);
    return new ConstraintStatement(new Relationship(target, type, attribute, factor), 1.0, quality);
}

static QString parseTarget(const QString& targetDef, const QString& data=QString())
{
    QString out;
    // May be a condition of the form <32?storageMedia[_].capacity:mainMemoryCapacity
    if (targetDef.contains('?')) {
        int questionIndex = targetDef.indexOf('?');
        int separatorIndex = targetDef.indexOf(':');
        QString condition = targetDef.left(questionIndex);
        QString attributeValue = condition.mid(1).trimmed();
        QString a = targetDef.mid(questionIndex+1, separatorIndex - questionIndex - 1);
        if (data.isEmpty())
            return a; // default to a
        QString b = targetDef.mid(separatorIndex+1);
        if (condition[0] == '=')
            out = (data == attributeValue) ? a : b;
        else if (condition[0] == '<')
            out = (data.toDouble() < attributeValue.toDouble()) ? a : b;
        else if (condition[0] == '>')
            out = (data.toDouble() > attributeValue.toDouble()) ? a : b;
        return out;
    } else
        return targetDef;

}

QList<ObservedToken*> Token::findIn(const QString& input) const
{
    QList<ObservedToken*> foundTokens;
    int matchIndex = -1;
    foreach (const LexicalFeature& feature, m_features) {
        int offset = 0;
        const QRegExp& regExp(feature.regExp);
        while ((matchIndex = regExp.indexIn(input, offset)) != -1) {
            QString matchedValue;
            if (regExp.captureCount() > 0)
                matchedValue = regExp.cap(1);
            if (!feature.value.isNull())
                matchedValue = feature.value;
            QString inputValue = input.mid(matchIndex);
            inputValue  = inputValue.left(regExp.matchedLength());
            ObservedToken *obs = new ObservedToken(this, inputValue, feature.polarity,
                                                   matchedValue, matchIndex,
                                                   input.left(matchIndex).count(" "));
            foundTokens << obs;
            offset = matchIndex + regExp.matchedLength();
            qDebug() << "Offeset" << offset << matchIndex << input << regExp.pattern() << regExp.matchedLength() << toString();
        }
    }
    return foundTokens;
}
double Token::filterMetaModifiers(QList<const ObservedToken*>& list) const
{
    double factor = 1.0;
    for (QList<const ObservedToken*>::iterator i = list.begin(); i != list.end();) {
        Q_ASSERT((*i));
        Q_ASSERT((*i)->token());

        const MetaModifierToken *thisModifier = dynamic_cast<const MetaModifierToken*>((*i)->token());
        if (thisModifier) {
            factor *= thisModifier->getFactor();
            i = list.erase(i);
        } else
            ++i;
    }
    return factor;
}

QList<Statement*> AttributeToken::makeStatements(const Offer *currentRecommendation, QList<const ObservedToken *> tokens, const ObservedToken* observation) const
{
    Q_UNUSED(currentRecommendation);
    double factor = filterMetaModifiers(tokens);
    QList<Statement*> out;

    // a sole attribute token
    QString cap = observation->capturedValue();
    if (tokens.length() == 1) {
        // if no captured value, add low ranked "good" relation
        // if captured value - add equality constraint
        Relationship::Type constraintType;
        double quality = 1.0;
        if (cap.isNull()) {
            bool allTypesUnopinionated = true;
            foreach (const QString& type, getTypes()) {
                if (!type.startsWith("unOpinionated")) {
                    allTypesUnopinionated = false;
                    break;
                }
            }
            //don't add "Good" for unopinionated attributes
            if (allTypesUnopinionated)
                return out;

            if (getTypes().count() == 1 && getTypes()[0] == "boolean") {
                constraintType = Relationship::IsTrue;
            } else {
                constraintType = Relationship::Good;
                quality = (factor > 0) ? 0.5 : 0.1; // make implied negatives unlikely
            }
        } else {
            constraintType = Relationship::Equality;
            // if we set same value to multiple targets, reduce quality
            // as this is likely not (really) what we want
            if (getTargets().size() != 1)
                quality = 0.4;
        }

        foreach (const QString& target, getTargets()) {
            out << getConstraintStatement(parseTarget(target, cap), constraintType, cap, factor, quality);

        }
    } else {
        //don't accept value tokens if we have a captured value
        if (!cap.isNull())
            return out;

        //check if all other tokens are value tokens
        bool allTokensValueTokens = true;
        QStringList values;
        foreach (const ObservedToken* t, tokens) {
            if (t->token() == this)
                continue;

            if (!dynamic_cast<const ValueToken*>(t->token())) {
                allTokensValueTokens = false;
                break;
            } else {
                values << t->capturedValue();
            }
        }
        // we are paired with exactly the right amount of value tokens
        QList<Statement*> valueStatements;
        if (allTokensValueTokens && getTargets().count() == values.count()) {
            for (int i = 0; i < values.count(); ++i) {
                QString target = parseTarget(getTargets()[i], values[i]);
                QSharedPointer<Attribute> attribute = getAttribute(target, values[i]);
                if (attribute.isNull())
                    break;
                valueStatements << getConstraintStatement(target, Relationship::Equality,
                                                                attribute, factor);
            }
            if (valueStatements.count() == values.count()) {
                //all values could be intantiated, assume correctness
                out << valueStatements;
            }
        }
    }

    return out;
}

static bool checkTarget(const QStringList& a, const QStringList& b)
{
    foreach (const QString& x, b)
        if (!a.contains(x))
            return false;
    return true;
}

QList<Statement*> ModifierToken::makeStatements(const Offer *currentRecommendation, QList<const ObservedToken *> tokens, const ObservedToken* observation) const
{
    double factor = filterMetaModifiers(tokens) * observation->polarity();
    QList<Statement*> out;
    // find value token if there is one
    QList<const ObservedToken*> valueTokens;
    QString valueOverride;
    for (QList<const ObservedToken*>::iterator i = tokens.begin(); i != tokens.end();) {
        if (dynamic_cast<const ValueToken*>((*i)->token())) {
            valueTokens << *i;
            i = tokens.erase(i);
        } else
            ++i;
    }
    QStringList targets;
    if (tokens.length() == 1) {
        //only the modifier token is here
        // check if it has a default recipient...
        targets = m_defaultTargets;
    }
    if (tokens.length() == 2) {
        int modifierPosition = observation->position();
        const ObservedToken* otherToken = tokens[1 - tokens.indexOf(observation)];
        int otherTokenPosition = otherToken->position();
        //constellation: {modifier, attribute}
        const AttributeToken *attributeToken = dynamic_cast<const AttributeToken*>(otherToken->token());
        // if otherToken is an attribute, build constraint against it
        if (attributeToken) {
            // check binding for viability
            if ((((modifierPosition < otherTokenPosition) && (m_binding & ModifierToken::Post)) ||
                    ((modifierPosition > otherTokenPosition) && (m_binding & ModifierToken::Pre))) &&
                    // make sure types are compatible
                    (checkTarget(m_on, attributeToken->getTypes())))
            {
                targets = attributeToken->getTargets();
                valueOverride = otherToken->capturedValue();
                // don't accept value tokens when we override the value through a captured one
                if (!valueOverride.isNull() && !valueTokens.isEmpty())
                    return out;
            }
        }

    }
    for (int i = 0; i < targets.count(); ++i) {
        QString target = targets[i];
        QSharedPointer<Attribute> attribute;
        double quality = 1.0;
        if (!valueOverride.isNull()) {
            target = parseTarget(target, valueOverride);
            attribute = getAttribute(target, valueOverride);
            if (!attribute) // failed to build attribute
                quality = 0.3;
        } else if (valueTokens.count() == targets.count()) {
            // if valueToken is set, use this attribute *value* instead of the one from
            // the current recommendation
            QString cap = valueTokens[i]->capturedValue();
            target = parseTarget(target, cap);
            attribute = getAttribute(target, cap);
            if (!attribute) // failed to build attribute
                quality = 0.3;
        } else if (currentRecommendation) {
            //check if currentRecommendation has this
            target = parseTarget(target);
            attribute = currentRecommendation->getRecord(target).second;
        }

        //build relationship
        out << getConstraintStatement(target, m_relationship, attribute, factor, quality);
    }

    return out;
}

QList<Statement*> MetaModifierToken::makeStatements(const Offer *currentRecommendation, QList<const ObservedToken *> tokens, const ObservedToken* observation) const
{
    Q_UNUSED(observation);
    Q_UNUSED(currentRecommendation);
    QList<Statement*> out;

    // ground meta modifiers in yes / no
    double factor = filterMetaModifiers(tokens);
    if (tokens.isEmpty()) { // only meta modifiers
        if (factor < 0)
            out << new CommandStatement(CommandStatement::No, -1*factor, 0.3);
        else
            out << new CommandStatement(CommandStatement::Yes, factor, 0.3);
    }
    return out;
}

QList<Statement*> AspectToken::makeStatements(const Offer *currentRecommendation, QList<const ObservedToken *> tokens, const ObservedToken* observation) const
{
    QList<Statement*> out;
    if (m_parent) {
        out << m_parent->makeStatements(currentRecommendation, tokens, observation);
        foreach (Statement *s, out)
            s->discountPolarity(0.1);
    }
    qDebug() << "===== Creating aspect by name: " << m_name;
    out << new AspectStatement(AspectFactory::getInstance()->getAspectByName(m_name));
    return out;
}

QList<Statement*> ValueToken::makeStatements(const Offer *currentRecommendation, QList<const ObservedToken *> tokens, const ObservedToken* observation) const
{
    Q_UNUSED(currentRecommendation);
    Q_UNUSED(tokens);
    Q_UNUSED(observation);
    return QList<Statement*>();
}

QList<Statement*> CommandToken::makeStatements(const Offer *currentRecommendation, QList<const ObservedToken*> tokens, const ObservedToken* observation) const
{
    double factor = filterMetaModifiers(tokens) * observation->polarity();
    QList<Statement*> out;
    if (tokens.length() != 1)
        return out;

    //only the command token in here, so go ahead
    foreach (const Action& a, m_actions) {
        switch (a.m_actionType) {
        case Action::Critique: {
            QSharedPointer<Attribute> attribute;
            QString matchedAttribute = observation->capturedValue();
            QString on = parseTarget(a.m_on, matchedAttribute);
            QString value = a.m_value;
            if (!matchedAttribute.isNull() || !value.isNull()) {
                if (value.isNull()) {
                    value = matchedAttribute;
                }
                attribute = AttributeFactory::getInstance()->getAttribute(on, value).second;
            } else {
                if (currentRecommendation)
                    attribute = currentRecommendation->getRecord(on).second;
            }

            //qDebug() << "Attribute is null: " << attribute.isNull();
            //if (!attribute.isNull())
            //    qDebug() << attribute->toString();
            qDebug() << "factor: " << factor;

            out << getConstraintStatement(on, a.m_relationship, attribute, factor);
            break;
        }
        case Action::Back:
            out << new CommandStatement(CommandStatement::Back, factor);
            break;
        case Action::Yes:
            out << new CommandStatement(CommandStatement::Yes, factor);
            break;
        case Action::No:
            out << new CommandStatement(CommandStatement::No, factor);
            break;
        case Action::Accept:
            out << new CommandStatement(CommandStatement::AcceptProduct, factor);
            break;
        case Action::UseCase:
            out << new UsecaseStatement(a.m_value, factor);
            break;
        }
    }
    return out;
}
