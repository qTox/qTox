/*
    Copyright © 2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    const QRegularExpression pathCleaner(QLatin1String{"[\\s|\\S]*c-toxcore."});
    QByteArray cleanedPath = QString::fromUtf8(file).remove(pathCleaner).toUtf8();
    cleanedPath.append('\0');
    return cleanedPath;
}

}  // namespace

/**
 * @brief Log message handler for toxcore log messages
 * @note See tox.h for the parameter definitions
 */
void onLogMessage(Tox *tox, Tox_Log_Level level, const char *file, uint32_t line,
                  const char *func, const char *message, void *user_data)
{
    std::ignore = tox;
    std::ignore = user_data;
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
