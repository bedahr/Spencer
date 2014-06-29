#ifndef RECOMMENDATIONATTRIBUTE_H
#define RECOMMENDATIONATTRIBUTE_H

#include <QObject>
#include <QString>
#include <QVariant>

/**
 * @brief Represents one attribute of a recommended offer
 *
 * Adds elements of the domain base and user model to the product knowledge
 */
class RecommendationAttribute : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(QVariant value READ getValue)
    Q_PROPERTY(float expressedUserInterest READ getExpressedUserInterest)
    Q_PROPERTY(float reviewSentiment READ getReviewSentiment)
public:
    RecommendationAttribute(const QString& name, const QVariant& value, float expressedUserInterest,
                            float reviewSentiment);
    QString getName() const { return name; }
    QVariant getValue() const { return value; }
    float getExpressedUserInterest() const { return expressedUserInterest; }
    float getReviewSentiment() const { return reviewSentiment; }

private:
    QString name;
    QVariant value;
    float expressedUserInterest;
    float reviewSentiment;
};

Q_DECLARE_METATYPE(RecommendationAttribute*)
Q_DECLARE_METATYPE(QList<RecommendationAttribute*>)

#endif // RECOMMENDATIONATTRIBUTE_H
