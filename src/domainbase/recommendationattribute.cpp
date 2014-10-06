#include "recommendationattribute.h"

RecommendationAttribute::RecommendationAttribute(const QString& name, const QVariant& value,
                                                 float expressedUserInterest, float completionFactor,
                                                 float reviewSentiment) :
    name(name), value(value), expressedUserInterest(expressedUserInterest),
    completionFactor(completionFactor), reviewSentiment(reviewSentiment)
{
}
