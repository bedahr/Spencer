/*
 *   Copyright (C) 2011 Peter Grasch <grasch@simon-listens.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "spencerview.h"

SpencerView::SpencerView(bool voiceControlled, QObject *parent) :
    QObject(parent),
    m_voiceControlled(voiceControlled)
{
}

SpencerView::~SpencerView()
{
}

void SpencerView::setVoiceControlled(bool voiceControlled)
{
    m_voiceControlled = voiceControlled;
    emit voiceControlledChanged();
}

void SpencerView::displayRecommendation(const QString& offerName, double price, double rating, const QStringList& images,
                                           const QList<RecommendationAttribute*> &offer, SentimentMap userSentiment,
                                           const QString& explanation)
{
    QList<RecommendationAttribute*> sanitizedOffer = offer;

    // Rewriting:
    //Prozessor: <processorType> <processorName> (<procesorFrequency>)
    //Display: <screenSize> <screenHResolution>x<screenHResolution>
    //Grafikkarte: <graphicsType> <graphicsModel> <dedicatedGraphicsMemoryCapacity>
    RecommendationAttribute *processorTypeAttribute = 0;
    RecommendationAttribute *processorNameAttribute = 0;
    RecommendationAttribute *processorFrequencyAttribute = 0;
    RecommendationAttribute *screenSizeAttribute = 0;
    RecommendationAttribute *screenSizeHResolutionAttribute = 0;
    RecommendationAttribute *screenSizeVResolutionAttribute = 0;
    RecommendationAttribute *graphicsTypeAttribute = 0;
    RecommendationAttribute *graphicsModelAttribute = 0;
    RecommendationAttribute *graphicsMemoryAttribute = 0;
    foreach (RecommendationAttribute* a, offer) {
        if (a->getName() == QObject::tr("Prozessortyp"))
            processorTypeAttribute = a;
        else if (a->getName() == QObject::tr("Prozessor"))
            processorNameAttribute = a;
        else if (a->getName() == QObject::tr("Prozessortakt"))
            processorFrequencyAttribute = a;
        else if (a->getName() == QObject::tr("Bildschirmdiagonale"))
            screenSizeAttribute = a;
        else if (a->getName() == QObject::tr("Horizontale Auflösung"))
            screenSizeHResolutionAttribute = a;
        else if (a->getName() == QObject::tr("Vertikale Auflösung"))
            screenSizeVResolutionAttribute = a;
        else if (a->getName() == QObject::tr("Grafikkartentyp"))
            graphicsTypeAttribute = a;
        else if (a->getName() == QObject::tr("Grafikkarte"))
            graphicsModelAttribute = a;
        else if (a->getName() == QObject::tr("Dedizierter Grafikkartenspeicher"))
            graphicsMemoryAttribute = a;
    }
    if (processorTypeAttribute && processorNameAttribute && processorFrequencyAttribute) {
        sanitizedOffer.removeAll(processorTypeAttribute);
        sanitizedOffer.removeAll(processorNameAttribute);
        sanitizedOffer.removeAll(processorFrequencyAttribute);

        float expressedUserInterest = qMax(qMax(processorTypeAttribute->getExpressedUserInterest(),
                                                processorNameAttribute->getExpressedUserInterest()),
                                           processorFrequencyAttribute->getExpressedUserInterest());
        float reviewSentiment =  (processorTypeAttribute->getReviewSentiment() +
                                  processorNameAttribute->getReviewSentiment()+
                                  processorFrequencyAttribute->getReviewSentiment()) / 3;

        sanitizedOffer.append(new RecommendationAttribute(QObject::tr("Prozessor"),
                                                          QString("%1 %2 (%3)").arg(processorTypeAttribute->getValue().toString())
                                                                               .arg(processorNameAttribute->getValue().toString())
                                                                               .arg(processorFrequencyAttribute->getValue().toString()),
                                                          expressedUserInterest,
                                                          reviewSentiment));
        delete processorTypeAttribute;
        delete processorNameAttribute;
        delete processorFrequencyAttribute;
    }
    if (screenSizeAttribute && screenSizeHResolutionAttribute && screenSizeVResolutionAttribute) {
        sanitizedOffer.removeAll(screenSizeAttribute);
        sanitizedOffer.removeAll(screenSizeHResolutionAttribute);
        sanitizedOffer.removeAll(screenSizeVResolutionAttribute);

        float expressedUserInterest = qMax(qMax(screenSizeAttribute->getExpressedUserInterest(),
                                                screenSizeHResolutionAttribute->getExpressedUserInterest()),
                                           screenSizeVResolutionAttribute->getExpressedUserInterest());
        float reviewSentiment =  (screenSizeAttribute->getReviewSentiment() +
                                  screenSizeHResolutionAttribute->getReviewSentiment()+
                                  screenSizeVResolutionAttribute->getReviewSentiment()) / 3;

        sanitizedOffer.append(new RecommendationAttribute(QObject::tr("Bildschirm"),
                                                          QString("%1 %2x%3)").arg(screenSizeAttribute->getValue().toString())
                                                                               .arg(screenSizeHResolutionAttribute->getValue().toString())
                                                                               .arg(screenSizeVResolutionAttribute->getValue().toString()),
                                                          expressedUserInterest,
                                                          reviewSentiment));
        delete screenSizeAttribute;
        delete screenSizeHResolutionAttribute;
        delete screenSizeVResolutionAttribute;
    }
    if (graphicsTypeAttribute && graphicsModelAttribute) {
        sanitizedOffer.removeAll(graphicsTypeAttribute);
        sanitizedOffer.removeAll(graphicsModelAttribute);
        sanitizedOffer.removeAll(graphicsMemoryAttribute);

        float expressedUserInterest = qMax(qMax(graphicsTypeAttribute->getExpressedUserInterest(),
                                                graphicsModelAttribute->getExpressedUserInterest()),
                                           graphicsMemoryAttribute ? graphicsMemoryAttribute->getExpressedUserInterest() : 0);
        float reviewSentiment =  (graphicsTypeAttribute->getReviewSentiment() +
                                  graphicsModelAttribute->getReviewSentiment()) / 2;

        QString value = graphicsMemoryAttribute ? "%1 %2 (%3)" : "%1 %2";
        value = value.arg(graphicsTypeAttribute->getValue().toString())
                .arg(graphicsModelAttribute->getValue().toString());

        if (graphicsMemoryAttribute) {
            value = value.arg(graphicsMemoryAttribute->getValue().toString());
        }

        sanitizedOffer.append(new RecommendationAttribute(QObject::tr("Grafikkarte"),
                                                         value, expressedUserInterest, reviewSentiment));
        delete graphicsTypeAttribute;
        delete graphicsModelAttribute;
        delete graphicsMemoryAttribute;
    }

    displayRecommendationPrivate(offerName, price, rating, images, sanitizedOffer, userSentiment, explanation);
    qDeleteAll(sanitizedOffer);
}
