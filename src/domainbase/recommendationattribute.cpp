#include "recommendationattribute.h"

RecommendationAttribute::RecommendationAttribute(const QString& name, const QVariant& value, float expressedUserInterest,
                                                 float reviewSentiment) :
    name(name), value(value), expressedUserInterest(expressedUserInterest), reviewSentiment(reviewSentiment)
{
}
