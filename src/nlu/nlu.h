#ifndef NLU_H
#define NLU_H

#include <QString>
#include <QList>

class Token;
class ModifierToken;
class AttributeToken;
class Statement;
class Relationship;
class ObservedToken;
class Offer;

class NLU
{
public:
    NLU();

    /// Reads the given nlp.xml to build the internal
    /// representation of the target language
    bool setupLanguage(const QString& nlpDefinition, const QString& aspectsDefinition);

    QList<Statement*> interpret(const Offer *currentRecommendation, const QString& userInput);
private:
    QList<Token*> m_acceptedTokens;

    bool setupNLP(const QString& path);
    bool setupAspects(const QString& path);

    QList<Relationship*> buildRelationships(const Offer *offer,
                                        const AttributeToken* attributeToken,
                                        const QString& attributeValue,
                                        const ModifierToken* modifierToken, double modifierFactor) const;
    QList<ObservedToken*> findTokens(const QString& input);
};

#endif // NLU_H
