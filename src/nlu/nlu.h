#ifndef NLU_H
#define NLU_H

#include <QString>
#include <QList>

class Token;
class ModifierToken;
class AttributeToken;
class AttributeFactory;
class Statement;
class Relationship;
class Offer;

class NLU
{
public:
    NLU(AttributeFactory *attributeFactory);

    /// Reads the given nlp.xml to build the internal
    /// representation of the target language
    bool setupLanguage(const QString& path);

    QList<Statement*> interpret(const Offer *currentRecommendation, const QString& userInput);
private:
    AttributeFactory *m_attributeFactory;
    QList<Token*> m_acceptedTokens;


    Relationship* buildRelationship(const Offer *offer, const AttributeToken* attributeToken,
                                    const QString &attributeValue,
                                    const ModifierToken* modifierToken, double modifierFactor) const;
};

#endif // NLU_H
