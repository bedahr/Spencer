#include "player.h"

void Player::setVideoSurface(QAbstractVideoSurface *surface)
{
    qDebug() << "setting surface";
    if (m_surface && m_surface == surface)
        return;

    qDebug() << "setting new surface";
    //AvatarSurface *wrapper = new AvatarSurface(surface, this);

    if (m_surface && m_surface->isActive()) {
        m_surface->stop();
    }
    m_surface = surface;

    //m_surface->start(QVideoSurfaceFormat(frame.size(), frame.pixelFormat()));
    //m_surface->present(frame);
    //if (m_surface)
      //  m_surface->start(QVideoSurfaceFormat());
}

void Player::present(const QImage& image)
{
    QVideoFrame frame = QVideoFrame(image);

    if (!m_surface->isActive()) {
        m_surface->start(QVideoSurfaceFormat(frame.size(), frame.pixelFormat()));
    }

    m_surface->present(frame);
}
