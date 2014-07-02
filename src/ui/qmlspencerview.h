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

#ifndef QMLSPENCERVIEW_H
#define QMLSPENCERVIEW_H

#include "spencerview.h"
#include "domainbase/offer.h"
#include <QVariant>
#include <QStringList>

class QQuickView;
class Spencer;
class QMLSpencerImageProvider;
class Avatar;

class QMLSpencerView : public SpencerView
{
    Q_OBJECT

signals:
    void publicCommandsChanged();

public:
    QMLSpencerView(Spencer *spencer, bool voiceControlled, QObject *parent=0);
    ~QMLSpencerView();

public slots:
    void show();

    void displayStatus(const QString& status);
    void displayError(const QString& error);
    void displayConnectionState(ConnectionState state);

    void storeConfiguration();
    void restoreConfiguration();

    void startRecordingRequested();
    void commitRecordingRequested();

    void displayExecutedAction(const QString& action);

    void displayMicrophoneLevel(int level, int min, int max);
    void displayListening();
    void displayRecognizing();

    void pauseUpdates();
    void resumeUpdates();

    void displayRecommendationPrivate(const QString& offerName, double price, double rating, const QStringList& images,
                               const QList<RecommendationAttribute *> &offer, SentimentMap userSentiment,
                               const QString &explanation);

    void actOut(const AvatarTask& avatarTask, bool immediately);

private slots:
    void avatarPresents(const QString& description);

private:
    QQuickView *viewer;
    QMLSpencerImageProvider *viewerImageCache;
    ConnectionState state;
    bool skipNonEssentialUIUpdates;

    Avatar *avatar;

    QObject* connectButton();
    QObject* disconnectButton();
    QObject* speakButton();
    QObject* speakLabel();

private slots:
    void connectClicked();
};

#endif // QMLSPENCERVIEW_H
