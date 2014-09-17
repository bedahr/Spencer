#ifndef CONSTRAINTSTATEMENT_H
#define CONSTRAINTSTATEMENT_H

#include "statement.h"
class Relationship;

/**
 * @brief The ConstraintStatement class
 *
 * @warning Lexical polarity extracted from the wording should be included in the relationship's factor
 * not here. The lexical polarity of the statement itself, is most likely then a constant 1.0.
 *
 * For example, sondier the command "Slightly cheaper, please"
 * This means that we want to slightly decrease the price attribute.
 * This can be expressed by a smallerthan relationship on the price attribute with a low modifier
 * factor. This relationship codifies "decrease price slightly". The polarity for the statement
 * "decrease price slightly" given our input is naturally a neutral 1.0.
 */
class ConstraintStatement : public Statement
{
public:
    ConstraintStatement(Relationship *relationship, double lexicalPolarity = defaultLexiconPolarity,
                        double quality = defaultQuality, double importance = defaultImportance);
    QString toString() const;
    bool act(DialogStrategy::DialogState state, DialogManager *dm) const;
protected:
    virtual bool comparePrivate(const Statement *s) const;

private:
    Relationship *m_relationship;

};

#endif // CONSTRAINTSTATEMENT_H
