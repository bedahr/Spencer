/*
 *   Copyright (C) 2011-2013 Peter Grasch <me@bedahr.org>
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

#include "simondconnector.h"
#include "spencer.h"
#include "ui/qmlspencerview.h"
#include "dialogmanager/dialogmanager.h"
#include <QObject>
#include <QDebug>
#include <QGuiApplication>

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

    Spencer spencer;
    DialogManager dialogManager;
    QMLSpencerView view(&spencer, voiceControlled);

    if (voiceControlled) {
        QObject::connect(&view, SIGNAL(connectToServer()), connector, SLOT(connectToServer()));
        QObject::connect(&view, SIGNAL(disconnectFromServer()), connector, SLOT(disconnectFromServer()));
        QObject::connect(&view, SIGNAL(startRecording()), connector, SLOT(startRecording()));
        QObject::connect(&view, SIGNAL(commitRecording()), connector, SLOT(commitRecording()));
        QObject::connect(&view, SIGNAL(configurationChanged()), connector, SLOT(configurationChanged()));

        QObject::connect(connector, SIGNAL(connectionState(ConnectionState)), &view, SLOT(displayConnectionState(ConnectionState)));
        QObject::connect(connector, SIGNAL(status(QString)), &view, SLOT(displayStatus(QString)));
        QObject::connect(connector, SIGNAL(error(QString)), &view, SLOT(displayError(QString)));
        QObject::connect(connector, SIGNAL(listening()), &view, SLOT(displayListening()));
        QObject::connect(connector, SIGNAL(recognizing()), &view, SLOT(displayRecognizing()));
        QObject::connect(connector, SIGNAL(microphoneLevel(int,int,int)), &view, SLOT(displayMicrophoneLevel(int,int,int)));
        //QObject::connect(connector, SIGNAL(recognized(QString)), &view, SLOT(displayExecutedAction(QString)));
    }

    QObject::connect(connector, SIGNAL(recognized(QString)), &dialogManager, SLOT(userInput(QString)));
    QObject::connect(&dialogManager, SIGNAL(elicit(AvatarTask, bool)), &view, SLOT(actOut(AvatarTask, bool)));

    //QObject::connect(connector, SIGNAL(recognized(QString)), &spencer, SLOT(critique(QString)));

    QObject::connect(&spencer, SIGNAL(recommend(const Offer*, QString)), &view, SLOT(displayRecommendation(const Offer*, QString)));
    QObject::connect(&spencer, SIGNAL(noMatchFor(QString)), &view, SLOT(displayNoMatch(QString)));

    view.show();
    if (voiceControlled)
        connector->init();
    if (!spencer.init()) {
        qWarning() << "Failed to initialize Spencer; Aborting";
        return -1;
    }
    dialogManager.init();

    int ret = app.exec();
    delete connector;
    return ret;
}
