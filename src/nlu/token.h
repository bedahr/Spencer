#ifndef TOKEN_H
#define TOKEN_H
#include "domainbase/relationship.h"
#include <QRegExp>
#include <QString>
#include <QList>
#include <QStringList>
#include <QFlags>

class ObservedToken;
class Statement;

static const float defaultPolarity = 1.0;

class LexicalFeature
{
public:
    LexicalFeature(const QRegExp& regExp) :
        regExp(regExp), polarity(defaultPolarity)
    {
    }
    LexicalFeature(const QRegExp& regExp, const QString& value) :
        regExp(regExp), value(value), polarity(defaultPolarity)
    {
    }
    LexicalFeature(const QRegExp& regExp, const QString& value, float polarity) :
        regExp(regExp), value(value), polarity(polarity)
    {
    }

    QRegExp regExp;
    QString value;
    float polarity;
};

class Token
{
public:
    Token(const QList<LexicalFeature>& features) : m_features(features) {}
    virtual ~Token() {}

    virtual int matches(const QString& token) const {
        return matches(token, 0);
    }
    int matches(const QString &token, QString* matched) const {
        foreach (const LexicalFeature& feature, m_features) {
            if (feature.regExp.indexIn(token) == 0) {
                if (matched && feature.regExp.captureCount() > 0)
                    *matched = feature.regExp.cap(1);
                return feature.regExp.matchedLength();
            }
        }
        return 0;
    }
    QString descriptor() const {
        return m_features.first().regExp.pattern();
    }

    QList<ObservedToken*> findIn(const QString& input) const;

    /**
     * @brief Builds statements from the given tokens
     * Only return a statement if you used all given tokens to build it and
     * if no other token in that chain could make a better (more informed)
     * version of the statement
     * @return A list of statements, or an empty one if none was built
     */
    virtual QList<Statement*> makeStatements(const Offer *currentRecommendation, QList<const ObservedToken*> tokens, const ObservedToken* observation) const=0;

    virtual QString toString() const = 0;

protected:
    QList<LexicalFeature> m_features;
    double filterMetaModifiers(QList<const ObservedToken*>& list) const;
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


    ModifierToken(const QList<LexicalFeature>& features, const QStringList& on, Relationship::Type relationship, Binding binding, const QStringList& defaultTargets) :
        Token(features), m_on(on), m_relationship(relationship), m_binding(binding), m_defaultTargets(defaultTargets)
    {}
    QStringList getOn() const { return m_on; }
    Relationship::Type getRelationship() const { return m_relationship; }
    Binding getBinding() const { return m_binding; }
    QStringList getDefaultTargets() const { return m_defaultTargets; }
    QString toString() const {
        return QString("Modifier: %1 %2").arg(m_on.join(",")).arg(m_relationship);
    }
    QList<Statement*> makeStatements(const Offer *currentRecommendation, QList<const ObservedToken*> tokens, const ObservedToken* observation) const;

private:
    QStringList m_on;
    Relationship::Type m_relationship;
    Binding m_binding;
    QStringList m_defaultTargets;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ModifierToken::Binding)

class MetaModifierToken : public Token
{
public:
    MetaModifierToken(const QList<LexicalFeature>& features, double factor) :
        Token(features), m_factor(factor)
    {}
    double getFactor() const { return m_factor; }
    QString toString() const {
        return QString("MetaModifier: %1").arg(m_factor);
    }
    QList<Statement*> makeStatements(const Offer *currentRecommendation, QList<const ObservedToken*> tokens, const ObservedToken* observation) const;

private:
    double m_factor;
};


class AttributeToken : public Token
{
public:
    AttributeToken(const QList<LexicalFeature>& features, const QStringList& types, const QStringList& targets) :
        Token(features), m_types(types), m_targets(targets)
    {}
    QStringList getTypes() const { return m_types; }
    QStringList getTargets() const { return m_targets; }
    QString toString() const {
        return QString("Attribute: %1").arg(m_targets.join(','));
    }
    QList<Statement*> makeStatements(const Offer *currentRecommendation, QList<const ObservedToken*> tokens, const ObservedToken* observation) const;

private:
    QStringList m_types;
    QStringList m_targets;
};

class AspectToken : public Token
{
public:
    AspectToken(const QString& name, const QStringList& synset) :
        Token(QList<LexicalFeature>() << LexicalFeature(QRegExp("\\b" + name + "\\b", Qt::CaseInsensitive))),
        m_parent(0),
        m_name(name), m_synset(synset)
    {
        foreach (const QString& s, synset)
            m_features << LexicalFeature(QRegExp("\\b" + s + "\\b", Qt::CaseInsensitive));
    }
    QString toString() const {
        return QString("Aspect: %1").arg(m_name);
    }
    void setParent(AspectToken *parent) {
        m_parent = parent;
    }
    QStringList names() const {
        QStringList n(m_synset);
        n << m_name;
        return n;
    }
    QList<Statement*> makeStatements(const Offer *currentRecommendation, QList<const ObservedToken*> tokens, const ObservedToken* observation) const;

private:
    AspectToken* m_parent;
    QString m_name;
    QStringList m_synset;
};

class ValueToken : public Token
{
public:
    ValueToken() : Token(QList<LexicalFeature>() << QRegExp("(\\d+( ?[,.]( ?\\d\\d*)?)?)"))
    {}
    QString toString() const {
        return QString("Value");
    }
    QList<Statement*> makeStatements(const Offer *currentRecommendation, QList<const ObservedToken*> tokens, const ObservedToken* observation) const;
};

class CommandToken : public Token
{
public:
    class Action {
    public:
        enum ActionType {
            Critique,
            Back,
            Yes,
            No,
            Accept,
            UseCase
        };

        ActionType m_actionType;
        Relationship::Type m_relationship;
        QString m_on;
        QString m_value;

        Action(ActionType actionType) :
            m_actionType(actionType)
        {}

        Action(Relationship::Type relationship,
               const QString& on, const QString& value) :
            m_actionType(Critique), m_relationship(relationship),
            m_on(on), m_value(value)
        {}

        Action(const QString& useCase) :
            m_actionType(UseCase), m_value(useCase)
        {}
    };

    CommandToken(const QList<LexicalFeature>& features, const QList<Action>& actions) :
        Token(features), m_actions(actions)
    {}
    QList<Action> getActions() const { return m_actions; }
    QString toString() const {
        QStringList actions;
        foreach (const Action& a, m_actions)
            actions << QString("%1 %2").arg(a.m_on).arg(a.m_relationship);
        return QString("Command: %1").arg(actions.join(", "));
    }
    QList<Statement*> makeStatements(const Offer *currentRecommendation, QList<const ObservedToken*> tokens, const ObservedToken* observation) const;

private:
    QList<Action> m_actions;
};

#endif // TOKEN_H
