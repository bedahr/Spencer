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
        QString value = nameElem.attribute("value");
        if (nameElem.hasAttribute("polarity")) {
            names << LexicalFeature(re, value, nameElem.attribute("polarity").toFloat());
        } else {
            names << LexicalFeature(re, value);
        }
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

    return out;
}

static int calculateDispersion(const QList<const ObservedToken*>& tokens)
{
    int maxDistance = 0;
    for (int i = 1; i < tokens.count(); ++i) {
        maxDistance = qMax(maxDistance,
                           tokens[i]->startWordIndex() - tokens[i-1]->endWordIndex());
    }
    return maxDistance;
}

QList<Statement*> findOptimalStatementAssociationNonOverlapping(const Offer *currentRecommendation, const QList<ObservedToken*>& tokens, double& bestSelectionErrors, int& bestDispersion)
{
    bestSelectionErrors = tokens.length();
    bestDispersion = tokens.empty() ? 0 : tokens.last()->endWordIndex();
    if (tokens.isEmpty())
        return QList<Statement*>();

    QList<const ObservedToken*> thisSelection;
    QList<ObservedToken*> others(tokens);

    QList<Statement*> bestSelection;

    for (int i = 0; i < tokens.length(); i++) {
        thisSelection << others.takeFirst();
        double thisSelectionErrors = thisSelection.length();
        int thisDispersion = calculateDispersion(thisSelection);
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
        int otherSelectionDispersion = 0;
        thisSelectionResult << findOptimalStatementAssociationNonOverlapping(currentRecommendation, others, otherSelectionErrors, otherSelectionDispersion);
        thisSelectionErrors += otherSelectionErrors;
        thisDispersion += otherSelectionDispersion;
        if ((thisSelectionErrors < bestSelectionErrors) ||
                ((thisSelectionErrors == bestSelectionErrors) && (thisDispersion < bestDispersion))) {
            bestSelection = thisSelectionResult;
            bestSelectionErrors = thisSelectionErrors;
            bestDispersion = thisDispersion;
        }
    }
    return bestSelection;
}

double relativePenalty(ObservedToken* token, int maxPos)
{
    return token->matchLength() / (double) maxPos;
}

typedef QPair< QList<ObservedToken*>, double /*penalty*/ > NonOverlappingTokenInfo;

QList< NonOverlappingTokenInfo >
      unrollOverlappingTokens(int i, const QList<ObservedToken*>& tokens, int maxPos, double skipPenalty)
{
    for (; i < tokens.length(); ++i) {
        //overlap: act!
        if (tokens[i-1]->overlaps(tokens[i])) {
            QList<ObservedToken*> incl(tokens);
            double inclSkipPenalty = skipPenalty;
            for (int j = i; (j < incl.length()) && (tokens[i - 1]->overlaps(tokens[j])); ++j)
                inclSkipPenalty += relativePenalty(incl.takeAt(i), maxPos);
            double exclSkipPenalty = skipPenalty;
            QList<ObservedToken*> excl(tokens);
            exclSkipPenalty += relativePenalty(excl.takeAt(i-1), maxPos) ;

            QList< NonOverlappingTokenInfo > out;
            out << unrollOverlappingTokens(i, incl, maxPos, inclSkipPenalty);
            out << unrollOverlappingTokens(i, excl, maxPos, exclSkipPenalty);
            return out;
        }
    }

    // no overlap
    return QList< NonOverlappingTokenInfo >() << qMakePair(QList<ObservedToken*>() << tokens, skipPenalty);
}

QList<Statement*> findOptimalStatementAssociation(int maxPos, const Offer *currentRecommendation, const QList<ObservedToken*>& tokens, double& bestError, int& bestDispersion)
{
    // find overlapping tokens and launch individual subproblems for all non-overlapping combinations;
    QList< QPair< QList<ObservedToken*>, double /*penalty*/ > > unrolled = unrollOverlappingTokens(1, tokens, maxPos, 0);
    bestError = tokens.count();
    double thisError = bestError;
    QList<Statement*> bestStatements;
    QList<Statement*> thisStatements;
    int thisDispersion;
    foreach (const NonOverlappingTokenInfo& nonOverlappingTokenInfo, unrolled) {
        const QList<ObservedToken*>& nonOverlappingTokens(nonOverlappingTokenInfo.first);
        qDebug() << "# no overlapping tokens: ";
        foreach (ObservedToken* t, nonOverlappingTokens)
            qDebug() << "   " << t->toString();
        double skipPenalty = nonOverlappingTokenInfo.second;
        thisStatements = findOptimalStatementAssociationNonOverlapping(currentRecommendation, nonOverlappingTokens, thisError, thisDispersion);

        qDebug() << thisError << thisError + skipPenalty << thisDispersion;
        qDebug() << "###";
        thisError += skipPenalty;
        if ((thisError < bestError) ||
                ((thisError == bestError) && (thisDispersion < bestDispersion))) {
            bestError = thisError;
            bestStatements = thisStatements;
            bestDispersion = thisDispersion;
        }
    }
    return bestStatements;
}

QList<Statement*> NLU::interpret(const Offer *currentRecommendation, const QString& input)
{
    qDebug() << "Interpreting: " << input;
    QList<Statement*> foundStatements;
    QString saneInput(input);
    saneInput.replace("ß", "ss");
    //QString saneInput = input;

    // 1. Extract list of largest, non overlapping token observations
    QList<ObservedToken*> foundTokens = findTokens(saneInput);
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
    qSort(foundTokens.begin(), foundTokens.end(), matchSizeLargerThan);
    qStableSort(foundTokens.begin(), foundTokens.end(), occuredBefore);
    qDebug() << "Parsing on " << foundTokens.size() << " matching tokens";
    foreach (ObservedToken* t, foundTokens) {
        qDebug() << "   " << t->toString();
    }
    double errors = 0;
    int dispersion = 0;
    foundStatements << findOptimalStatementAssociation(saneInput.length(), currentRecommendation, foundTokens, errors, dispersion);
    qDebug() << "Found " << foundStatements.size() << " matching statements; " << errors << " errors, dispersion: " << dispersion;
    foreach (Statement* s, foundStatements) {
        qDebug() << "   " << s->toString();
    }
    return foundStatements;
}
