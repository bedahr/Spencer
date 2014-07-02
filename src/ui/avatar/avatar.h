#ifndef AVATAR_H
#define AVATAR_H

#include <QString>
#include <QList>
#include <QTimer>
#include <QCache>
#include <QHash>
#include <QMediaPlayer>
#include "avatartask.h"

class Player;
class QObject;

class Avatar : public QObject
{
Q_OBJECT
signals:
    void presenting(const QString& description);

public:
    Avatar(QObject *parent=0);
    ~Avatar();

    void interrupt();

    void queue(const AvatarTask& task);

    void setUI(QObject *ui);
    QMediaObject *getPlayer() const;

private:
    QCache<QString, QImage> imageCache;
    QObject *ui;
    QString currentTaskDirectory;

    QList<AvatarTask> taskQueue;
    Player *mediaPlayer;
    QMediaPlayer *audioPlayer;
    QTimer avatarUpdateTimer;

    QHash<QString, QString> visemes;

    void reset();
    void processQueue();
    QString getTaskDirectory(const AvatarTask& task);
    QString getMaryRequestUrl(const QString& text, bool wav) const;

    QByteArray doDownload(const QUrl &url);

private slots:
    void updateAvatar();
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void maximizeAvatar();
    void minimizeAvatar();
    void introCompleted();

};

#endif // AVATAR_H
