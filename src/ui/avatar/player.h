#ifndef PLAYER_H
#define PLAYER_H

#include <QMediaPlayer>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoSurface>

class Player : public QMediaPlayer
{
  Q_OBJECT
  Q_PROPERTY(QAbstractVideoSurface* videoSurface READ getVideoSurface WRITE setVideoSurface )

public:
  Player(QObject * parent=0, Flags flags=0) : QMediaPlayer(parent, flags), m_surface(0) {}

public slots:
    void setVideoSurface(QAbstractVideoSurface *surface);
    QAbstractVideoSurface* getVideoSurface()
    {
    return m_surface;
    }

    void present(const QImage& image);

private:
  QAbstractVideoSurface* m_surface;
};

#endif // PLAYER_H
