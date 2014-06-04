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

#ifndef SPENCERVIEW_H
#define SPENCERVIEW_H

#include "avatar/avatartask.h"
#include "simone.h"
#include "recognitionresult.h"
#include <QObject>

class Offer;

class SpencerView : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool voiceControlled READ getVoiceControlled
               WRITE setVoiceControlled NOTIFY voiceControlledChanged)

signals:
    void connectToServer();
    void disconnectFromServer();

    void startRecording();
    void commitRecording();

    void configurationChanged();

    void voiceControlledChanged();

public:
    explicit SpencerView(bool voiceControlled, QObject *parent = 0);
    virtual ~SpencerView();

    bool getVoiceControlled() const { return m_voiceControlled; }
    void setVoiceControlled(bool voiceControlled);

public slots:
    virtual void show()=0;

    virtual void displayStatus(const QString& status)=0;
    virtual void displayError(const QString& error)=0;
    virtual void displayConnectionState(ConnectionState state)=0;

    virtual void displayExecutedAction(const QString& action)=0;

    virtual void displayMicrophoneLevel(int level, int min, int max)=0;
    virtual void displayListening()=0;
    virtual void displayRecognizing()=0;

    virtual void displayRecommendation(const Offer* offer, const QString &explanation)=0;
    virtual void displayNoMatch(const QString& description)=0;

    virtual void actOut(const AvatarTask& avatarTask, bool immediately)=0;

private:
    bool m_voiceControlled;
};

#endif // SPENCERVIEW_H
