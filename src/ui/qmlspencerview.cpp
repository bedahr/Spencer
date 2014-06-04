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

#include "qmlspencerview.h"
#include "settings.h"
#include "spencer.h"
#include "recommender/offer.h"
#include "ui/avatar/avatar.h"
#include "recommender/attributemodel.h"
#include <QMediaPlayer>
#include <QMediaObject>
#include <QQuickImageProvider>
#include <QQuickView>
#include <QQuickItem>
#include <QStringList>
#include <QMediaObject>
#include <QQmlContext>
#include <QVariant>
#include <QVariantMap>
#include <QDebug>
#include <QUuid>

class QMLSpencerImageProvider: public QQuickImageProvider
{
public:
    QMLSpencerImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Pixmap)
    {}

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
    {
        QPixmap out = m_pixmapCache.value(id);
        if (size)
            *size = out.size();

        if (requestedSize.isValid())
            out = out.scaled(requestedSize,Qt::IgnoreAspectRatio);
        return out;
    }

    QString add(const QPixmap& pixmap) {
        QString id = QUuid::createUuid().toString().mid(1, 36);
        qDebug() << "Adding image to provider: " + id;
        m_pixmapCache.insert(id, pixmap);
        return id;
    }

    void remove(const QString& id) {
        m_pixmapCache.remove(id);
    }

private:
    QHash<QString, QPixmap> m_pixmapCache;
};


QMLSpencerView::QMLSpencerView(Spencer *spencer, bool voiceControlled, QObject *parent) :
    SpencerView(voiceControlled, parent),
    viewer(new QQuickView()),
    viewerImageCache(new QMLSpencerImageProvider),
    attributeModel(new AttributeModel(spencer->getAttributeFactory(), this)),
    state(Unconnected),
    skipNonEssentialUIUpdates(false)
{
    avatar = new Avatar();
    //viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer->rootContext()->setContextProperty("spencerView", this);
    viewer->rootContext()->setContextProperty("spencer", spencer);
    viewer->rootContext()->setContextProperty("attributeModel", attributeModel);
    viewer->rootContext()->setContextProperty("avatarPlayer", avatar->getPlayer());
    viewer->setSource(QUrl::fromLocalFile("app/native/res/qml/spencer/main.qml"));
    viewer->engine()->addImageProvider(QLatin1String("SpencerImages"), viewerImageCache);

    connect(connectButton(), SIGNAL(clicked()),
                                 this, SLOT(connectClicked()));
    connect(disconnectButton(), SIGNAL(clicked()),
                                 this, SIGNAL(disconnectFromServer()));

    QObject *avatarQMLProxy = viewer->rootObject()->findChild<QObject*>("avatar");
    avatar->setUI(avatarQMLProxy);
    restoreConfiguration();
}

QMLSpencerView::~QMLSpencerView()
{
    delete viewer;
    delete attributeModel;
    //delete viewerImageCache; deleted by engine
}

void QMLSpencerView::pauseUpdates()
{
    qDebug() << "Pause";
    skipNonEssentialUIUpdates = true;
}

void QMLSpencerView::resumeUpdates()
{
    qDebug() << "Resume";
    skipNonEssentialUIUpdates = false;
}

void QMLSpencerView::connectClicked()
{
    storeConfiguration();
    emit connectToServer();
}

void QMLSpencerView::startRecordingRequested()
{
    if (state != Active)
        return;
    emit startRecording();
}

void QMLSpencerView::commitRecordingRequested()
{
    if (state != Active)
        return;
    emit commitRecording();
}

void QMLSpencerView::displayStatus(const QString& status)
{
    qDebug() << "Display status" << status;
    //QObject *lbStatus = viewer->rootObject()->findChild<QObject*>("lbStatus");
    //lbStatus->setProperty("text", status);
}

void QMLSpencerView::displayError(const QString& error)
{
    //qDebug() << "Error: " << error;
    //QMetaObject::invokeMethod(viewer->rootObject()->findChild<QObject*>("errorDialog"),
    //                          "show", Q_ARG(QVariant, error));
}

void QMLSpencerView::displayExecutedAction(const QString& action)
{
    //QMetaObject::invokeMethod(viewer->rootObject()->findChild<QObject*>("recognitionResultBanner"),
    //                          "recognized", Q_ARG(QVariant, action));
}

void QMLSpencerView::displayConnectionState(ConnectionState state_)
{
    state = state_;
    //qDebug() << "State changed: " << state;

    QObject *cb = connectButton();
    switch (state) {
    case Unconnected:
        cb->setProperty("text", tr("Connect"));
        break;
    case Connecting:
        cb->setProperty("text", tr("Connecting..."));
        break;
    default:
        cb->setProperty("text", tr("Connected"));
    }


    QObject *db = disconnectButton();
    switch (state) {
    case ConnectedWaiting:
        db->setProperty("text", tr("Disconnect"));
        break;
    case Connected:
    case Active:
        db->setProperty("text", tr("Disconnect"));
        break;
    case Disconnecting:
        db->setProperty("text", tr("Disconnecting..."));
        break;
    default:
        db->setProperty("text", tr("Disconnected"));
        break;
    }

    viewer->rootObject()->setProperty("state",
        ((state == Unconnected) || (state == Connecting)) ? "disconnected" :
                        ((state == Active) ? "activated" : "connected"));
}

void QMLSpencerView::displayMicrophoneLevel(int level, int min, int max)
{
    if (skipNonEssentialUIUpdates)
        return;

    /*
    QObject *pbVUMeter = viewer->rootObject()->findChild<QObject*>("pbVUMeter");
    if (min != -1)
        pbVUMeter->setProperty("minimumValue", min);
    if (max != -1)
        pbVUMeter->setProperty("maximumValue", max);
    pbVUMeter->setProperty("value", level);
    */
}

void QMLSpencerView::displayListening()
{
    //speakLabel()->setProperty("text", tr("Listening..."));

}

void QMLSpencerView::displayRecognizing()
{
    /*
    QMetaObject::invokeMethod(viewer->rootObject()->findChild<QObject*>("currentRecommendation"),
                                      "displayRecognizing");
    speakLabel()->setProperty("text", tr("Please speak"));
    */
}

void QMLSpencerView::restoreConfiguration()
{
    qDebug() << "Restoring configuration";
    QObject *rootObject = viewer->rootObject();
    rootObject->findChild<QObject*>("cbAutoConnect")->setProperty("checked", Settings::autoConnect());
    rootObject->findChild<QObject*>("teHost")->setProperty("text", Settings::host());
    rootObject->findChild<QObject*>("tePort")->setProperty("text", QString::number(Settings::port()));
    rootObject->findChild<QObject*>("teUserName")->setProperty("text", Settings::user());
    rootObject->findChild<QObject*>("tePassword")->setProperty("text", Settings::password());
    rootObject->findChild<QObject*>("cbPushToTalk")->setProperty("checked", !Settings::voiceActivityDetection());

    qDebug() << "Restoring configuration: Done";
}

void QMLSpencerView::storeConfiguration()
{
    qDebug() << "Storing configuration";
    QObject *rootObject = viewer->rootObject();
    Settings::setAutoConnect(rootObject->findChild<QObject*>("cbAutoConnect")->property("checked").toBool());
    Settings::setHost(rootObject->findChild<QObject*>("teHost")->property("text").toString());
    Settings::setPort(rootObject->findChild<QObject*>("tePort")->property("text").toInt());
    Settings::setUser(rootObject->findChild<QObject*>("teUserName")->property("text").toString());
    Settings::setPassword(rootObject->findChild<QObject*>("tePassword")->property("text").toString());
    Settings::setVoiceActivityDetection(!rootObject->findChild<QObject*>("cbPushToTalk")->property("checked").toBool());
    Settings::store();

    emit configurationChanged();
}

void QMLSpencerView::show()
{
#ifndef Q_OS_BLACKBERRY
    viewer->showNormal();
#else
    viewer->showFullScreen();
#endif
}

QObject* QMLSpencerView::connectButton()
{
    return viewer->rootObject()->findChild<QObject*>("btConnect");
}

QObject* QMLSpencerView::disconnectButton()
{
    return viewer->rootObject()->findChild<QObject*>("btDisconnect");
}

QObject* QMLSpencerView::speakButton()
{
    return viewer->rootObject()->findChild<QObject*>("btSpeak");
}

QObject* QMLSpencerView::speakLabel()
{
    return viewer->rootObject()->findChild<QObject*>("lbSpeak");
}

void QMLSpencerView::displayRecommendation(const Offer* offer, const QString& explanation)
{
    //qDebug() << "Displaying recommendation";
    static QString oldId;
    if (!oldId.isNull()) {
        //qDebug() << "Removing: " << oldId;
        viewerImageCache->remove(oldId);
    }

    oldId = viewerImageCache->add(offer->getImage());
    qDebug() << oldId;
    attributeModel->setAttributes(offer->getAttributeNames(), offer->getAttributes());
    //QVariantMap attributes;
    //foreach (const QString& key, offer.getAttributes().keys())
      //  attributes.insert(key, offer.getAttribute(key)->toString());
    /*
    QMetaObject::invokeMethod(viewer->rootObject()->findChild<QObject*>("currentRecommendation"),
                              "recommend",
                              Q_ARG(QVariant, QVariant::fromValue(offer->getName())),
                              Q_ARG(QVariant,
                                    QVariant::fromValue(QLatin1String("image://SpencerImages/")+oldId)),
                              Q_ARG(QVariant, QVariant::fromValue(attributeModel))
                              );
                              */
    displayExecutedAction(explanation);
}

void QMLSpencerView::actOut(const AvatarTask& avatarTask, bool immediately)
{
    if (immediately)
        avatar->interrupt();
    avatar->queue(avatarTask);
}

void QMLSpencerView::displayNoMatch(const QString& description)
{
    //QMetaObject::invokeMethod(viewer->rootObject()->findChild<QObject*>("noMatchDialog"),
    //                          "show", Q_ARG(QVariant, description));
}
