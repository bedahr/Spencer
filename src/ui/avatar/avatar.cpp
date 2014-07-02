#include "avatar.h"
#include "player.h"
#include <cstdlib>
#include <QObject>
#include <QMediaPlayer>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkAccessManager>

//set at compile time
static const QString mediaPath = "/home/bedahr/ownCloud/Daten/TU/Master/avatar/videos/";
static const QString maryUrl = "http://localhost:59125";
static int fps = 25;
static int updateInterval = (1000 / fps);


Avatar::Avatar(QObject *parent) : QObject(parent), imageCache(500), ui(0),
    mediaPlayer(new Player()),
    audioPlayer(new QMediaPlayer())
{
    connect(&avatarUpdateTimer, SIGNAL(timeout()), this, SLOT(updateAvatar()));
    connect(audioPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));
    avatarUpdateTimer.start(updateInterval);


    //setting up visemes
    visemes.insert("_", (mediaPath+"visemes/Rest.png"));
    visemes.insert("?", (mediaPath+"visemes/Rest.png"));
    visemes.insert("@", (mediaPath+"visemes/E.png"));
    visemes.insert("e:", (mediaPath+"visemes/E.png"));
    visemes.insert("e", (mediaPath+"visemes/E.png"));
    visemes.insert("E", (mediaPath+"visemes/E.png"));
    visemes.insert("E:", (mediaPath+"visemes/E.png"));
    visemes.insert("6", (mediaPath+"visemes/A.png"));
    visemes.insert("a", (mediaPath+"visemes/A.png"));
    visemes.insert("a:", (mediaPath+"visemes/A.png"));
    visemes.insert("aI", (mediaPath+"visemes/A.png"));
    visemes.insert("aU", (mediaPath+"visemes/U.png"));
    visemes.insert("b", (mediaPath+"visemes/M_B_P.png"));
    visemes.insert("C", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    visemes.insert("d", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    visemes.insert("f", (mediaPath+"visemes/F_V.png"));
    visemes.insert("g", (mediaPath+"visemes/G_K.png"));
    visemes.insert("h", (mediaPath+"visemes/A.png"));
    visemes.insert("i", (mediaPath+"visemes/I.png"));
    visemes.insert("I", (mediaPath+"visemes/I.png"));
    visemes.insert("i:", (mediaPath+"visemes/I.png"));
    visemes.insert("j", (mediaPath+"visemes/I.png"));
    visemes.insert("k", (mediaPath+"visemes/G_K.png"));
    visemes.insert("l", (mediaPath+"visemes/L.png"));
    visemes.insert("m", (mediaPath+"visemes/M_B_P.png"));
    visemes.insert("n", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    visemes.insert("N", (mediaPath+"visemes/G_K.png"));
    visemes.insert("o:", (mediaPath+"visemes/O.png"));
    visemes.insert("o", (mediaPath+"visemes/O.png"));
    visemes.insert("9", (mediaPath+"visemes/O.png"));
    visemes.insert("O", (mediaPath+"visemes/O.png"));
    visemes.insert("OY", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    visemes.insert("p", (mediaPath+"visemes/M_B_P.png"));
    visemes.insert("R", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    visemes.insert("S", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    visemes.insert("s", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    visemes.insert("t", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    visemes.insert("t", (mediaPath+"visemes/L.png"));
    visemes.insert("ts", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    visemes.insert("u:", (mediaPath+"visemes/U.png"));
    visemes.insert("U", (mediaPath+"visemes/U.png"));
    visemes.insert("v", (mediaPath+"visemes/F_V.png"));
    visemes.insert("x", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    visemes.insert("Y", (mediaPath+"visemes/I.png"));
    visemes.insert("y:", (mediaPath+"visemes/I.png"));
    visemes.insert("z", (mediaPath+"visemes/C_D_N_R_S_Z.png"));
    //end setting up visemes

    processQueue();
}

Avatar::~Avatar()
{
    delete audioPlayer;
    delete mediaPlayer;
}

void Avatar::interrupt()
{
    taskQueue.clear();
    reset();
}

void Avatar::reset()
{
    QMetaObject::invokeMethod(ui, "reset");
}

void Avatar::setUI(QObject *ui) {
    this->ui = ui;
    reset();
}


QMediaObject* Avatar::getPlayer() const
{
    return mediaPlayer;
}

void Avatar::queue(const AvatarTask& task)
{
    taskQueue.append(task);
    processQueue();
}

QString Avatar::getMaryRequestUrl(const QString& text, bool wav) const
{
    return QString("%1/process?INPUT_TYPE=TEXT&OUTPUT_TYPE=%2&INPUT_TEXT=%3&OUTPUT_TEXT=&effect_Volume_selected=on"
                                 "&effect_Volume_parameters=amount:3;&effect_Volume_default=Default&effect_Volume_help=Help&"
                                 "effect_TractScaler_selected=on&effect_TractScaler_parameters=amount:1.5;&"
                                 "effect_TractScaler_default=Default&effect_TractScaler_help=Help&effect_F0Scale_selected=&"
                                 "effect_F0Scale_parameters=f0Scale:2.0;&effect_F0Scale_default=Default&effect_F0Scale_help=Help&"
                                 "effect_F0Add_selected=on&effect_F0Add_parameters=f0Add:30.0;&VOICE_SELECTIONS=dfki-pavoque-"
                                 "neutral-hsmm de male hmm&AUDIO_OUT=WAVE_FILE&LOCALE=de&VOICE=dfki-pavoque-neutral-hsmm&AUDIO=WAVE_FILE")
            .arg(maryUrl).arg(wav ? "AUDIO" : "PRAAT_TEXTGRID").arg(text);
}



QByteArray Avatar::doDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    qDebug() << "Downloading:";
    qDebug() << url.toEncoded();
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    return reply->readAll();
}


QString Avatar::getTaskDirectory(const AvatarTask& task) {
    switch(task.expression()) {
    case AvatarTask::Idle:
        return mediaPath+"/idle";
    case AvatarTask::Intro:
        return mediaPath+"/intro";
        break;
    case AvatarTask::AskUseCase:
        return mediaPath+"/initiator_1";
        break;
    case AvatarTask::AskMostImportantAttribute:
        return mediaPath+"/initiator_2";
        break;
    case AvatarTask::AskPerformanceImportant:
        return mediaPath+"/initiator_3";
        break;
    case AvatarTask::AskEasyTransportImportant:
        return mediaPath+"/initiator_4";
        break;
    case AvatarTask::AskPriceImporant:
        return mediaPath+"/initiator_5";
        break;
    case AvatarTask::PresentItem:
        return mediaPath+"/present_" + QString::number((rand() % 4) + 1);
        break;
    case AvatarTask::Misunderstood:
        return mediaPath+"/huh_" + QString::number((rand() % 3) + 1);
        break;
    case AvatarTask::Custom:
        // create new directory if it doesn't yet exist based on text's hash
        QString workDir = "/tmp/spencer/" + QString::number(qHash(task.text())) + '/';

        if (!QDir(workDir).exists()) {
            if (!QDir().mkpath(workDir)) {
                qWarning() << "Failed to make working directory: " << workDir;
                return QString();
            }


            // Design query for openmary
            QString wavQuery = getMaryRequestUrl(task.text(), true);

            // download audio
            QFile audio(workDir+"audio.wav");
            if (!audio.open(QIODevice::WriteOnly)) {
                qWarning() << "Failed to write audio file";
                return QString();
            }
            audio.write(doDownload(QUrl(wavQuery)));
            audio.close();

            // design query for praat textgrid
            QString textgridQuery = getMaryRequestUrl(task.text(), false);

            // parse praat textgrid
            QList<QByteArray> textGrid = doDownload(QUrl(textgridQuery)).split('\n');
            for (int i = 0; i < 13; ++i)
                textGrid.removeFirst(); //remove preamble

            qint64 currentTime = 0;
            qint64 thisStart = 0;
            qint64 thisEnd = 0;

            foreach (QString line, textGrid) {
                if (line.startsWith("xmin = ")) {
                    thisStart = qRound(line.mid(7).toFloat() * 1000.0f);
                } else if (line.startsWith("xmax = ")) {
                    thisEnd = qRound(line.mid(7).toFloat() * 1000.0f);
                } else if (line.startsWith("text = ")) {
                    QString text = line.mid(8);
                    text.chop(2);
                    while (currentTime < thisStart) {
                        // add "rest's"
                        QFile(visemes.value("_")).link(workDir+QString("/images%1.png").arg(currentTime / updateInterval, 5, 10, QLatin1Char('0')));
                        currentTime += updateInterval;
                    }
                    QFile viseme(visemes.value(text));
                    while (currentTime < thisEnd) {
                        viseme.link(workDir+QString("/images%1.png").arg(currentTime / updateInterval, 5, 10, QLatin1Char('0')));
                        currentTime += updateInterval;
                    }
                }
            }
        }
        return workDir;
        break;
    }
    return QString();
}

void Avatar::processQueue() {
    audioPlayer->stop();
    if (taskQueue.isEmpty()) {
        currentTaskDirectory = getTaskDirectory(AvatarTask(AvatarTask::Idle));
    } else {
        AvatarTask task = taskQueue.takeFirst();
        if (task.expression() == AvatarTask::Intro) {
            QTimer::singleShot(25500, Qt::CoarseTimer, this, SLOT(introCompleted()));
        }
        emit presenting(task.description());
        currentTaskDirectory = getTaskDirectory(task);
    }
    audioPlayer->setMedia(QUrl::fromLocalFile(currentTaskDirectory+"/audio.wav"));
    audioPlayer->play();
}

void Avatar::updateAvatar()
{
    if (audioPlayer->state() != QMediaPlayer::PlayingState)
        return;

    qint64 audioPlayerPosition = audioPlayer->position();
    int frame = audioPlayerPosition / updateInterval + 1;

    QString imageSrc = currentTaskDirectory+QString("/images%1.png").arg(frame, 5, 10, QLatin1Char('0'));
    if (imageCache.contains(imageSrc))
        mediaPlayer->present(* imageCache.object(imageSrc));
    else {
        QImage* img = new QImage(imageSrc);
        imageCache.insert(imageSrc, img);
        mediaPlayer->present(*img);
    }
}

void Avatar::maximizeAvatar()
{
    QMetaObject::invokeMethod(ui, "maximize");
}

void Avatar::introCompleted()
{
    minimizeAvatar();
    emit presenting(tr("Bitte beschreiben Sie Ihren Wunschlaptop."));
}

void Avatar::minimizeAvatar()
{
    QMetaObject::invokeMethod(ui, "minimize");
}

void Avatar::mediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia)
        processQueue();
}
