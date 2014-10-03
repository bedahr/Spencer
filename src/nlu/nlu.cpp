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
#include <iostream>
#include <limits>
#include <QRegExp>
#include <QFile>
#include <QStringList>
#include <QDomElement>
#include <QDebug>

//#define PRINT_NLP_CORPUS

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

#ifdef PRINT_NLP_CORPUS
static QStringList getSentences(const Token *t) {
    QStringList out;
    foreach (const LexicalFeature& feature, t->lexicalFeatures()) {
        QString pattern = feature.regExp.pattern();
        pattern.replace(QRegExp("\\( \\\\w\\+\\)\\{.,.\\}"), "");
        pattern.replace("\\b", "");
        int sectionIdx = pattern.indexOf(QRegExp("\\(.*|.*\\)"));
        if (sectionIdx != -1) {
            QString prefix = pattern.left(sectionIdx);
            int sectionCloseIdx = pattern.indexOf(')', sectionIdx + 1) ;
            QString postfix = pattern.mid(sectionCloseIdx + 1);
            QStringList options = pattern.mid(sectionIdx+1, sectionCloseIdx - sectionIdx - 1).split('|');
            foreach (const QString& option, options) {
                QString o = prefix + option + postfix;
                out << o;
            }
        } else
            out << pattern;
    }
    return out;
}

static QStringList expandNumbers(const QStringList& modifierList, const QStringList& attributeIds)
{
    QStringList out;

    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    foreach (const QString& attributeId, attributeIds) {
        QSharedPointer<Attribute> m = AttributeFactory::getInstance()->getSmallestInstance(attributeId);
        QSharedPointer<Attribute> ma = AttributeFactory::getInstance()->getLargestInstance(attributeId);
        if (m.isNull()) {
            qDebug() << "Min Attribute is null: " << attributeId;
            return modifierList;
        }
        if (ma.isNull()) {
            qDebug() << "Max Attribute is null: " << attributeId;
            return modifierList;
        }
        min = qMin(min, m->value());
        max = qMax(max, ma->value());
        //qDebug() << "min: " << min << attributeId << m-> value();
        //qDebug() << "max: " << max << attributeId << ma-> value();
    }
    double range = max - min;
    static QStringList numberInvitations(QStringList() << " WIE" << " ALS" << "MINDESTENS" << "MAXIMAL" << "ÜBER" << "UNTER" << "WENIGSTENS" << "HÖCHSTENS");

    QStringList additions;
    foreach (const QString& modifier, modifierList) {
        foreach (const QString& inv, numberInvitations) {
            if (modifier.endsWith(inv)) {
                for (int i = (int) min; i < (int) ceil(max); ++i)
                    additions << modifier + ' ' + QString::number(i);

                if (range < 10) {
                    for (int i = (int) min; i < (int) ceil(max); ++i)
                        for (int j = 1; j <= 9; ++j)
                            additions << modifier + ' ' + QString::number(i) + " KOMMA " + QString::number(j);
                }
            }
        }
    }
    out << modifierList;
    out << additions;
    return out;
}

static QStringList cross(QList<QStringList> lists)
{
    int n = lists.count();
    if (n == 0)
        return QStringList();
    else if (n == 1)
        return lists[0];

    // n > 1
    QStringList head = lists.takeAt(0);
    QStringList tail = cross(lists);
    QStringList out;
    foreach (const QString& left, head) {
        foreach (const QString& right, tail) {
            out << left + ' ' + right;
        }
    }

    return out;
}
static void print(const QStringList& list)
{
    foreach (const QString& l, list)
        std::cout << l.toUpper().toUtf8().constData() << std::endl;
}
#endif

bool NLU::setupLanguage(const QString& nlpDefinition, const QString &aspectsDefinition)
{
    qDeleteAll(m_acceptedTokens);
    m_acceptedTokens.clear();

    bool out = setupNLP(nlpDefinition) && setupAspects(aspectsDefinition);
    // print LM
#ifdef PRINT_NLP_CORPUS
    QList<const ModifierToken*> modifierTokens;
    QList<const AttributeToken*> attributeTokens;
    QList<const MetaModifierToken*> metaModifierTokens;
    QList<const AspectToken*> aspectTokens;
    QList<const CommandToken*> commandTokens;
    foreach (const Token * t, m_acceptedTokens) {
        const ModifierToken* modifierToken = dynamic_cast<const ModifierToken*>(t);
        const AttributeToken* attributeToken = dynamic_cast<const AttributeToken*>(t);
        const MetaModifierToken* metaModifierToken = dynamic_cast<const MetaModifierToken*>(t);
        const AspectToken* aspectToken = dynamic_cast<const AspectToken*>(t);
        const CommandToken* commandToken = dynamic_cast<const CommandToken*>(t);
        if (modifierToken)
            modifierTokens << modifierToken;
        if (attributeToken)
            attributeTokens << attributeToken;
        if (metaModifierToken)
            metaModifierTokens << metaModifierToken;
        if (aspectToken)
            aspectTokens << aspectToken;
        if (commandToken)
            commandTokens << commandToken;
    }
    // set up plain sentence fragments
    foreach (const Token * t, m_acceptedTokens)
        print(getSentences(t));

    //combine modifiers and attributes
    QStringList metaModifierSentences;
    foreach (const MetaModifierToken* metaModifierToken, metaModifierTokens)
        metaModifierSentences << getSentences(metaModifierToken);
    foreach (const AttributeToken* attributeToken, attributeTokens) {
        QStringList types = attributeToken->getTypes();
        QStringList attributeSentences = getSentences(attributeToken);
        // find modifiers that match those types
        foreach (const ModifierToken* modifierToken, modifierTokens) {
            bool matches = false;
            foreach (const QString& attributeType, types) {
                if (modifierToken->getOn().contains(attributeType)) {
                    matches = true;
                    break;
                }
            }
            if (!matches)
                continue;
            QStringList modifierSentences = getSentences(modifierToken);
            QStringList expandedModifierSentences;
            if (types.contains("countable") || types.contains("unOpinionatedCountable"))
                expandedModifierSentences = expandNumbers(modifierSentences, attributeToken->getTargets());
            if (modifierToken->getBinding() & ModifierToken::Pre) {
                print(cross(QList<QStringList>() << attributeSentences << expandedModifierSentences));
                print(cross(QList<QStringList>() << metaModifierSentences << attributeSentences << modifierSentences));
            }
            if (modifierToken->getBinding() & ModifierToken::Post) {
                print(cross(QList<QStringList>() << expandedModifierSentences << attributeSentences));
                print(cross(QList<QStringList>() << metaModifierSentences << modifierSentences << attributeSentences));
            }
        }
    }
    //combine metamodifiers and commands
    foreach (const CommandToken* commandToken, commandTokens) {
        QStringList commandSentences = getSentences(commandToken);
        print(cross(QList<QStringList>() << metaModifierSentences << commandSentences));
    }
#endif

    // end print LM
    return out;
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
