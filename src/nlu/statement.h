#ifndef STATEMENT_H
#define STATEMENT_H

#include <QString>

class CritiqueRecommender;

class Statement
{
public:
    Statement();
    virtual QString toString() const=0;
    virtual bool act(CritiqueRecommender* r) const=0;
};

#endif // STATEMENT_H
