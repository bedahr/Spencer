#include "nlu.h"
#include "statement.h"
#include "token.h"
#include "observedtoken.h"
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

static QList<LexicalFeature> parseNames(const QDomElement& namesElem)
{
    QList<LexicalFeature> names;
    QDomElement nameElem = namesElem.firstChildElement("name");
    while (!nameElem.isNull()) {
        QRegExp re(QLatin1String("\\b") + nameElem.text() + QLatin1String("\\b"), Qt::CaseInsensitive);
        nameElem = nameElem.nextSiblingElement("name");
        if (nameElem.hasAttribute("polarity"))
            names << LexicalFeature(re, nameElem.attribute("polarity").toFloat());
        else
            names << LexicalFeature(re);
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
    else if (relationshipTypeStr == "SmallerThan")
        relationshipType = Relationship::SmallerThan;
    else if (relationshipTypeStr == "LargerThan")
        relationshipType = Relationship::LargerThan;
    else if (relationshipTypeStr == "IsTrue")
        relationshipType = Relationship::IsTrue;
    else if (relationshipTypeStr == "IsFalse")
        relationshipType = Relationship::IsFalse;
    else if (relationshipTypeStr == "Better")
        relationshipType = Relationship::BetterThan;
    else if (relationshipTypeStr == "Good")
        relationshipType = Relationship::Good;
    else if (relationshipTypeStr == "Worse")
        relationshipType = Relationship::WorseThan;
    else if (relationshipTypeStr == "Bad")
        relationshipType = Relationship::Bad;
    else if (relationshipTypeStr == "Small")
        relationshipType = Relationship::Small;
    else if (relationshipTypeStr == "Large")
        relationshipType = Relationship::Large;
    return relationshipType;
}

QString parseTarget(const QString& targetDef, const QString& data)
{
    QString out;
    // May be a condition of the form <32?storageMedia[_].capacity:mainMemoryCapacity
    if (targetDef.contains('?')) {
        int questionIndex = targetDef.indexOf('?');
        int separatorIndex = targetDef.indexOf(':');
        QString condition = targetDef.left(questionIndex);
        QString attributeValue = condition.mid(1).trimmed();
        QString a = targetDef.mid(questionIndex+1, separatorIndex - questionIndex - 1);
        QString b = targetDef.mid(separatorIndex+1);
        if (condition[0] == '=')
            out = (data == attributeValue) ? a : b;
        else if (condition[0] == '<')
            out = (data.toDouble() < attributeValue.toDouble()) ? a : b;
        else if (condition[0] == '>')
            out = (data.toDouble() > attributeValue.toDouble()) ? a : b;
        return out;
    } else
        return out;

}

bool NLU::setupNLP(const QString& nlpDefinition)
{
    QDomDocument doc;
    QFile f(nlpDefinition);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file: " << nlpDefinition;
        return false;
    }
    if (!doc.setContent(f.readAll())) {
        qWarning() << "Failed to parse XML at " << nlpDefinition;
        return false;
    }

    QDomElement rootElement(doc.documentElement());
    QDomElement attributesElement(rootElement.firstChildElement("attributes"));
    QDomElement modifiersElement(rootElement.firstChildElement("modifiers"));
    if (attributesElement.isNull() || modifiersElement.isNull()) {
        // no case or invalid format
        qWarning() << "Invalid XML: " << nlpDefinition;
        return false;
    }
    // attributes
    QDomElement attributeElem = attributesElement.firstChildElement("attribute");
    while (!attributeElem.isNull()) {
        QStringList types = attributeElem.attribute("type").split(',', QString::SkipEmptyParts);
        QStringList actOn = attributeElem.firstChildElement("actOn").text().split(',', QString::SkipEmptyParts);
        m_acceptedTokens << new AttributeToken(parseNames(attributeElem.firstChildElement("names")), types, actOn);
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
        m_acceptedTokens << new ModifierToken(parseNames(modifierElement.firstChildElement("names")), on.split(",", QString::SkipEmptyParts),
                                              getRelationshipType(relationship), binding, modifierElement.attribute("default").split(',', QString::SkipEmptyParts));
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
            QString specialTag = actionElem.attribute("special");
            if (specialTag == QLatin1String("back")) {
                actions << CommandToken::Action(CommandToken::Action::Back);
            } else if (specialTag == QLatin1String("yes")) {
                actions << CommandToken::Action(CommandToken::Action::Yes);
            } else if (specialTag == QLatin1String("no")) {
                actions << CommandToken::Action(CommandToken::Action::No);
            } else if (specialTag == QLatin1String("accept")) {
                actions << CommandToken::Action(CommandToken::Action::Accept);
            } else if (specialTag.startsWith("use=")) {
                QString useCase = specialTag.mid(4);
                actions << CommandToken::Action(useCase);
            } else {
                QString attribute = actionElem.firstChildElement("attribute").text();
                QDomElement relationshipElement(actionElem.firstChildElement("relationship"));
                QString relationship = relationshipElement.text();
                QString value = relationshipElement.attribute("value");
                actions << CommandToken::Action(getRelationshipType(relationship), attribute, value);
            }
            actionElem = actionElem.nextSiblingElement("action");
        }
        m_acceptedTokens << new CommandToken(parseNames(commandElement.firstChildElement("names")), actions);
        commandElement = commandElement.nextSiblingElement("command");
    }
    m_acceptedTokens << new ValueToken();
    return true;
}

bool NLU::setupAspects(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    QList<AspectToken*> aspectTokens;
    while (!f.atEnd()) {
        QString line = QString::fromUtf8(f.readLine());
        int separatorIndex = line.indexOf(':');
        QString tag = line.left(separatorIndex);
        QStringList body = line.mid(separatorIndex+1).trimmed().split(';', QString::SkipEmptyParts);
        AspectToken *thisToken = new AspectToken(tag, body);
        foreach (AspectToken *token, aspectTokens) {
            if (token->names().contains(tag)) {
                thisToken->setParent(token);
            }
        }
        aspectTokens << thisToken;
    }
    foreach (AspectToken *a, aspectTokens)
        m_acceptedTokens << a;
    return true;
}

bool NLU::setupLanguage(const QString& nlpDefinition, const QString &aspectsDefinition)
{
    qDeleteAll(m_acceptedTokens);
    m_acceptedTokens.clear();

    return setupNLP(nlpDefinition) && setupAspects(aspectsDefinition);
}

NLU::NLU()
{
}

bool matchSizeLargerThan(ObservedToken* a, ObservedToken* b) {
    return a->matchLength() > b->matchLength();
}

bool occuredBefore(ObservedToken* a, ObservedToken* b) {
    return a->position() < b->position();
}

QList<ObservedToken*> NLU::findTokens(const QString& input)
{
    QList<ObservedToken*> out;

    foreach (Token* t, m_acceptedTokens)
        out << t->findIn(input);

    //remove overlapping tokens, favor longer ones
    // first, sort on descending match size
    qSort(out.begin(), out.end(), matchSizeLargerThan);
    qDebug() << "Sort done";
    // then, remove overlaps
    /*
    for (QList<ObservedToken*>::iterator i = out.begin(); i != out.end();) {
        bool erased = false;
        foreach (ObservedToken* comp, out) {
            if (comp == *i) // sorted by size => everything after obs will be smaller
                break;
            // if comp overlaps *i and is larger than obs, remove *i
            if (comp->overlaps(*i)) {
                i = out.erase(i);
                erased = true;
                break;
            }
        }
        if (!erased)
             ++i;
    }*/

    return out;
}

#if 0

QList<Statement*> findOptimalStatementAssociation(const Offer *currentRecommendation, const QList<ObservedToken*>& tokens, double& bestSelectionErrors)
{
    bestSelectionErrors = tokens.length();
    if (tokens.isEmpty())
        return QList<Statement*>();

    QList<const ObservedToken*> thisSelection;
    QList<ObservedToken*> others(tokens);

    QList<Statement*> bestSelection;

    for (int i = 0; i < tokens.length(); i++) {
        thisSelection << others.takeFirst();
        double thisSelectionErrors = thisSelection.length();
        QList<Statement*> thisSelectionResult;
        foreach (const ObservedToken* ot, thisSelection) {
            QList<Statement*> thisVoteForThisSelection = ot->token()->makeStatements(currentRecommendation, thisSelection, ot);
            if (!thisVoteForThisSelection.isEmpty()) {
                thisSelectionResult << thisVoteForThisSelection;
                thisSelectionErrors = 0;
                foreach (Statement* s, thisVoteForThisSelection)
                    thisSelectionErrors = qMax(1.0 - s->quality(), thisSelectionErrors);
                break;
            }
        }
        double otherSelectionErrors = 0;
        thisSelectionResult << findOptimalStatementAssociation(currentRecommendation, others, otherSelectionErrors);
        thisSelectionErrors += otherSelectionErrors;
        if (thisSelectionErrors <= bestSelectionErrors) { // prefer larger association
            bestSelection = thisSelectionResult;
            bestSelectionErrors = thisSelectionErrors;
        }
    }
    return bestSelection;
}
#endif

QList<Statement*> findOptimalStatementAssociation(int currentPos, const Offer *currentRecommendation, const QList<ObservedToken*>& tokens, double& bestSelectionErrors)
{
    bestSelectionErrors = tokens.length();
    if (tokens.isEmpty())
        return QList<Statement*>();

    QList<const ObservedToken*> thisSelection;
    QList<ObservedToken*> others(tokens);

    QList<Statement*> bestSelection;

    double bestPreSelectionErrors = 0;
    QList<Statement*> bestPreSelectionResult;
    for (int i = 0; i < tokens.length(); i++) {
        ObservedToken* t;
        do {
            if (others.isEmpty())
                return bestSelection;
            t = others.takeFirst();
        } while (t->position() < currentPos); //overlaps, do not consider

        thisSelection << t;

        // now peak ahead and launch subproblems for all tokens overlapping t
        QList<ObservedToken*> overlapping;
        while (!others.isEmpty() && others.first()->overlaps(t)) {
            overlapping << others.takeFirst();
        }
        for (int j = 0; j < overlapping.count(); ++j) {
            int subSkippingPenalty = j + 1;
            // 1 because already "skip" t
            // token list that doesn't include the skipped elements
            QList<ObservedToken*> subTokens(others);
            for (int k = overlapping.count() - 1; k > j; --k)
                subTokens.insert(0, overlapping[i]);
            // result of best pre-selection so far (which obviously doesn't include the offending
            // t), is stored in bestPreSelectionResult. So go off of this.
            double adjustedBestPreSelectionErrors = bestPreSelectionErrors + subSkippingPenalty;
            double subPostSelectionErrors = 0;
            QList<Statement*> subResult = findOptimalStatementAssociation(currentPos, currentRecommendation, subTokens, subPostSelectionErrors);
            double subSelectionErrors = subPostSelectionErrors + adjustedBestPreSelectionErrors;
            if (subSelectionErrors <= bestSelectionErrors) {
                bestSelection = bestPreSelectionResult;
                bestSelection << subResult;
                bestSelectionErrors = subSelectionErrors;
            }
        }

        currentPos = t->position() + t->matchLength();

        double thisPreSelectionErrors = thisSelection.length();
        QList<Statement*> thisPreSelectionResult;
        foreach (const ObservedToken* ot, thisSelection) {
            QList<Statement*> thisVoteForThisSelection = ot->token()->makeStatements(currentRecommendation, thisSelection, ot);
            if (!thisVoteForThisSelection.isEmpty()) {
                thisPreSelectionResult << thisVoteForThisSelection;
                thisPreSelectionErrors = 0;
                foreach (Statement* s, thisVoteForThisSelection)
                    thisPreSelectionErrors = qMax(1.0 - s->quality(), thisPreSelectionErrors);
                break;
            }
        }
        //store best selection of preamble separately
        if (thisPreSelectionErrors <= bestPreSelectionErrors) {
            bestPreSelectionResult = thisPreSelectionResult;
            bestPreSelectionErrors = thisPreSelectionErrors;
        }

        double otherSelectionErrors = 0;
        QList<Statement*> thisSelectionResult(thisPreSelectionResult);
        thisSelectionResult << findOptimalStatementAssociation(currentPos, currentRecommendation, others, otherSelectionErrors);
        double thisSelectionErrors = thisPreSelectionErrors + otherSelectionErrors;
        if (thisSelectionErrors <= bestSelectionErrors) { // prefer larger association
            bestSelection = thisSelectionResult;
            bestSelectionErrors = thisSelectionErrors;
        }
    }
    return bestSelection;
}

QList<Statement*> NLU::interpret(const Offer *currentRecommendation, const QString& input)
{
    QList<Statement*> foundStatements;
    qDebug() << "Interpreting: " << input;

    // 1. Extract list of largest, non overlapping token observations
    QList<ObservedToken*> foundTokens = findTokens(input);
    qDebug() << "Found " << foundTokens.size() << " matching tokens";
    foreach (ObservedToken* t, foundTokens) {
        qDebug() << "   " << t->toString();
    }

    // 2. Extract aspect tokens
    for (QList<ObservedToken*>::iterator i = foundTokens.begin(); i != foundTokens.end();) {
        const AspectToken* aspect = dynamic_cast<const AspectToken*>((*i)->token());
        if (aspect) {
            foundStatements << aspect->makeStatements(currentRecommendation, QList<const ObservedToken*>() << *i, *i);
            i = foundTokens.erase(i);
        } else
            ++i;
    }

    // 3. Find token associations that minimizes #unassigned tokens and build statements
    double errors = 0;
    qSort(foundTokens.begin(), foundTokens.end(), occuredBefore);
    foundStatements << findOptimalStatementAssociation(0, currentRecommendation, foundTokens, errors);
    qDebug() << "Found " << foundStatements.size() << " matching statements; " << errors << " errors";
    foreach (Statement* s, foundStatements) {
        qDebug() << "   " << s->toString();
    }
    return QList<Statement*>();
}
