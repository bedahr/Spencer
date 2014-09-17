#ifndef STATEMENT_H
#define STATEMENT_H

#include "nlu/dialogmanager.h"
#include "dialogmanager/dialogstrategy.h"
#include <QString>

class Offer;

static const double defaultLexiconPolarity = 1.0;
static const double defaultQuality = 1.0;
static const double defaultImportance = 1.0;

class Statement
{
public:
    Statement(double lexicalPolarity, double statementQuality, double statementImportance = defaultImportance);
    virtual QString toString() const=0;
    virtual bool act(DialogStrategy::DialogState state, DialogManager* dm, const Offer *currentOffer) const=0;
    void setLexicalPolarity(double polarity) {
        m_lexicalPolarity = polarity;
    }
    void setImportance(double importance) {
        m_importance = importance;
    }
    double quality() const {
        return m_quality;
    }
    void discountPolarity(double diff) {
        if (m_lexicalPolarity > 0 && m_lexicalPolarity < diff)
            m_lexicalPolarity = 0.01;
        else
            m_lexicalPolarity -= diff;
    }

    bool compare(const Statement *s) const;

    /// Total, combined effect of this statements
    /// Caluclated using quality, importance and lexical polarity
    double effect() const;

protected:
    /// returns a description of the full statement based on userData
    /// (with added information from the base class
    /// well suited for debug output
    virtual QString formatStatementString(const QString& userData) const;
    virtual bool comparePrivate(const Statement *s) const=0;

    /// determines how secure we are in identifying this statement as true
    /// user intention; [0,1]
    double m_quality;

    /// how "extreme" is the requirement
    /// for example, a high polarity with the constraint "cheaper" means much cheaper
    double m_lexicalPolarity;

    /// the importance of the statement
    /// the higher this value, the more important it seems to be for the user
    /// that this statement is taken into account
    double m_importance;
};

#endif // STATEMENT_H
