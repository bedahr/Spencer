#ifndef LOGGER_H
#define LOGGER_H

class QTextStream;
class QFile;
class QString;

#include <cassert>

class Logger
{
public:
    static bool init();
    static void log(const QString& message);

private:
    static Logger* i;
    static QTextStream *logFile;
    static QFile *logF;
    Logger();
    bool init(const QString& path);
    static Logger* getInstance() {
        assert(i);
        return i;
    }

};

#endif // LOGGER_H
