/*
 *   Copyright (C) 2011-2013 Peter Grasch <me@bedahr.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU `neral Public License version 2,
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

#include "simondconnector.h"
#include "spenceradapter.h"
#include "spencer.h"
#include "ui/qmlspencerview.h"
#include <QObject>
#include <QtDBus/QDBusConnection>
#include <QDebug>
#include <QGuiApplication>
#include <QLocale>

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath("app/native/plugins");
    QGuiApplication app(argc, argv);

    bool voiceControlled = true;
    SimondConnector *connector;
    if (voiceControlled)
        connector = new SimondConnector;
    else
        connector = 0;

    Spencer* spencer = new Spencer;
    SpencerAdapter *adapter = new SpencerAdapter(spencer);
    QDBusConnection::sessionBus().registerService("at.tugraz.Spencer");
    QDBusConnection::sessionBus().registerObject("/Spencer", adapter, QDBusConnection::ExportAllSlots);
    QMLSpencerView* view = new QMLSpencerView(spencer, voiceControlled);

    if (voiceControlled) {
        QObject::connect(view, SIGNAL(connectToServer()), connector, SLOT(connectToServer()));
        QObject::connect(view, SIGNAL(disconnectFromServer()), connector, SLOT(disconnectFromServer()));
        QObject::connect(view, SIGNAL(startRecording()), connector, SLOT(startRecording()));
        QObject::connect(view, SIGNAL(commitRecording()), connector, SLOT(commitRecording()));
        QObject::connect(view, SIGNAL(configurationChanged()), connector, SLOT(configurationChanged()));

        QObject::connect(connector, SIGNAL(connectionState(ConnectionState)), view, SLOT(displayConnectionState(ConnectionState)));
        QObject::connect(connector, SIGNAL(status(QString)), view, SLOT(displayStatus(QString)));
        QObject::connect(connector, SIGNAL(error(QString)), view, SLOT(displayError(QString)));
        QObject::connect(connector, SIGNAL(listening()), view, SLOT(displayListening()));
        QObject::connect(connector, SIGNAL(recognizing(qint64, qint64)), view, SLOT(displayRecognizing()));

        QObject::connect(connector, SIGNAL(listening()), spencer, SLOT(listening()));
        QObject::connect(connector, SIGNAL(recognizing(qint64, qint64)), spencer, SLOT(recognizing()));

        QObject::connect(connector, SIGNAL(microphoneLevel(int,int,int)), view, SLOT(displayMicrophoneLevel(int,int,int)));
        //QObject::connect(connector, SIGNAL(recognized(QString)), view, SLOT(displayExecutedAction(QString)));
    }

    QObject::connect(connector, SIGNAL(recognized(RecognitionResultList)), spencer, SLOT(userInput(RecognitionResultList)));
    QObject::connect(spencer, SIGNAL(elicit(AvatarTask, bool)), view, SLOT(actOut(AvatarTask, bool)));

    QObject::connect(spencer, SIGNAL(recommend(QString, double, double, QStringList,
                                               QList<RecommendationAttribute*>, SentimentMap, QString)),
                     view, SLOT(displayRecommendation(QString, double, double, QStringList,
                                                      QList<RecommendationAttribute*>, SentimentMap, QString)));

    view->show();
    if (voiceControlled)
        connector->init();
    if (!spencer->init()) {
        qWarning() << "Failed to initialize Spencer; Aborting";
        return -1;
    }

    int ret = app.exec();
    delete spencer;
    delete view;
    delete connector;
    return ret;
}
