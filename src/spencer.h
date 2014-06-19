#ifndef SPENCER_H
#define SPENCER_H

#include "ui/avatar/avatartask.h"
#include <QObject>
#include <QList>

class CritiqueRecommender;
class Offer;
class Relationship;
class Offer;
class AttributeFactory;
class NLU;
class DialogManager;
class DatabaseConnector;

class Spencer : public QObject
{
    Q_OBJECT

signals:
    void recommend(const Offer* offer, const QString& description);
    void elicit(AvatarTask, bool immediately);

public:
    explicit Spencer(QObject *parent = 0);
    ~Spencer();

public slots:
    /// Initialize spencer
    bool init();
    void reset();

    void userInput(const QString& input);

private:
    DatabaseConnector *m_databaseConnector;
    AttributeFactory *m_attributeFactory;
    NLU *m_nlu;
    DialogManager *m_dialogManager;
    CritiqueRecommender *m_recommender;
    const Offer *m_currentRecommendation;

    /// parses the database of offers from the given casebase XML file
    /// and returns the list of found offers; if an error occured,
    /// *okay will be set to false
    QList<Offer*> parseCasebase(bool* okay) const;


private slots:
    void recommendationChanged(const Offer *o, const QString &explanation);
    
};

#endif // SPENCER_H
