#ifndef ASPECTSTATEMENT_H
#define ASPECTSTATEMENT_H
#include "statement.h"
#include "domainbase/aspect.h"

class AspectStatement : public Statement
{
public:
    AspectStatement(const Aspect* aspect, double lexicalPolarity = defaultLexiconPolarity,
                    double quality = defaultQuality, double importance = defaultImportance);
    QString toString() const;
    bool act(DialogStrategy::DialogState state, DialogManager *dm, const Offer* currentOffer) const;
protected:
    virtual bool comparePrivate(const Statement *s) const;
private:
    const Aspect* m_aspect;
};

#endif // ASPECTSTATEMENT_H
