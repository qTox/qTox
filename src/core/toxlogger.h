#ifndef TOXLOGGER_H
#define TOXLOGGER_H

#include <tox/tox.h>

#include <cstdint>

namespace ToxLogger {
    void onLogMessage(Tox *tox, TOX_LOG_LEVEL level, const char *file, uint32_t line,
                      const char *func, const char *message, void *user_data);
}

#endif // TOXLOGGER_H
