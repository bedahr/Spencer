#include "logger.h"
#include <QStandardPaths>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>

Logger* Logger::i = 0;
QTextStream * Logger::logFile = 0;
QFile * Logger::logF = 0;

Logger::Logger()
{
}

bool Logger::init()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir directory(path);
    if (!directory.exists() && !directory.mkpath(path)) {
        qWarning() << "Failed to create logging path: " << path;
        return false;
    }
    QString fileName = QDateTime::currentDateTime().toString(Qt::ISODate);
    Logger::i = new Logger();
    return Logger::i->init(path + QDir::separator() + fileName);
}

bool Logger::init(const QString& path)
{
    delete logFile;
    delete logF;

    logF = new QFile(path);
    if (!logF->open(QIODevice::WriteOnly|QIODevice::Append)) {
        qWarning() << "Couldn't open log file at " << path;
        return false;
    }
    logFile = new QTextStream(logF);
    return true;
}
void Logger::log(const QString& message)
{
    *(getInstance()->logFile) << QDateTime::currentDateTime().toString(Qt::ISODate) << ' ' << message << '\n';
    getInstance()->logFile->flush();
}
