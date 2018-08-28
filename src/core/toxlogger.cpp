#include "toxlogger.h"

#include <tox/tox.h>

#include <QDebug>
#include <QRegularExpression>
#include <QString>
#include <QStringBuilder>

namespace ToxLogger {
namespace {

QByteArray cleanPath(const char *file)
{
    // for privacy, make the path relative to the c-toxcore source directory
    const QRegularExpression pathCleaner(QLatin1Literal{"[\\s|\\S]*c-toxcore."});
    QByteArray cleanedPath = QString{file}.remove(pathCleaner).toUtf8();
    cleanedPath.append('\0');
    return cleanedPath;
}

}  // namespace

/**
 * @brief Log message handler for toxcore log messages
 * @note See tox.h for the parameter definitions
 */
void onLogMessage(Tox *tox, TOX_LOG_LEVEL level, const char *file, uint32_t line,
                  const char *func, const char *message, void *user_data)
{
    const QByteArray cleanedPath = cleanPath(file);

    switch (level) {
    case TOX_LOG_LEVEL_TRACE:
        return; // trace level generates too much noise to enable by default
    case TOX_LOG_LEVEL_DEBUG:
        QMessageLogger(cleanedPath.data(), line, func).debug() << message;
        break;
    case TOX_LOG_LEVEL_INFO:
        QMessageLogger(cleanedPath.data(), line, func).info() << message;
        break;
    case TOX_LOG_LEVEL_WARNING:
        QMessageLogger(cleanedPath.data(), line, func).warning() << message;
        break;
    case TOX_LOG_LEVEL_ERROR:
        QMessageLogger(cleanedPath.data(), line, func).critical() << message;
        break;
    }
}

}  // namespace ToxLogger
