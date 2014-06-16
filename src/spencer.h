#ifndef SPENCER_H
#define SPENCER_H

#include <QObject>
#include <QList>

class CritiqueRecommender;
class Offer;
class Relationship;
class Offer;
class AttributeFactory;
class NLU;

class Spencer : public QObject
{
    Q_OBJECT

signals:
    void recommend(const Offer* offer, const QString& description);
    void noMatchFor(const QString& critique);

public:
    explicit Spencer(QObject *parent = 0);
    ~Spencer();

    AttributeFactory* getAttributeFactory() const {
        return m_attributeFactory;
    }

public slots:
    /// Initialize spencer
    bool init();
    void reset();

    /// Parses the given voice command to one or more
    /// critiques
    /// Returns the interpretation (user presentable)
    QString critique(const QString& command);

private:
    AttributeFactory *m_attributeFactory;
    NLU *m_nlu;
    CritiqueRecommender *m_recommender;
    const Offer *m_currentRecommendation;

    /// parses the database of offers from the given casebase XML file
    /// and returns the list of found offers; if an error occured,
    /// *okay will be set to false
    QList<Offer*> parseCasebase(const QString& path, const QString& imageBasePath, bool* okay) const;


private slots:
    void recommendationChanged(const Offer *o, const QString &explanation);
    
};

#endif // SPENCER_H
