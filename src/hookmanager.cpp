/*
    Copyright © 2020 by The qTox Project Contributors

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

#include <algorithm>
#include <cstdarg>

#include <QLibrary>
#include <QDebug>

#include "src/globalshortcut.h"
#include "src/hookmanager.h"

namespace {
    GlobalShortcut* globalShortcut = nullptr;
#if defined(Q_OS_WIN)
    const char* hookLibName = "libuiohook-0";
#else
    const char* hookLibName = "libuiohook";
#endif
    void logError(int hookStatus)
    {
        const char* errorString = nullptr;
        switch (hookStatus)
        {
        case UIOHOOK_SUCCESS:
            return;
        
        // System level errors. 
        case UIOHOOK_ERROR_OUT_OF_MEMORY:
            errorString = "Failed to allocate memory.";
            break;

        // Unix specific errors.
        case UIOHOOK_ERROR_X_OPEN_DISPLAY:
            errorString = "Failed to open X11 display.";
            break;
        case UIOHOOK_ERROR_X_RECORD_NOT_FOUND:
            errorString = "Unable to locate XRecord extension.";
            break;
        case UIOHOOK_ERROR_X_RECORD_ALLOC_RANGE:
            errorString = "Unable to allocate XRecord range.";
            break;
        case UIOHOOK_ERROR_X_RECORD_CREATE_CONTEXT:
            errorString = "Unable to allocate XRecord context.";
            break;
        case UIOHOOK_ERROR_X_RECORD_ENABLE_CONTEXT:
            errorString = "Failed to enable XRecord context.";
            break;
        case UIOHOOK_ERROR_X_RECORD_GET_CONTEXT:
            errorString = "Failed to get XRecord context.";
            break;

        // Windows specific errors.
        case UIOHOOK_ERROR_SET_WINDOWS_HOOK_EX:
            errorString = "Failed to register low level windows hook.";
            break;

        // Windows specific errors.
        case UIOHOOK_ERROR_AXAPI_DISABLED:
            errorString = "Failed to enable access for assistive devices.";
            break;
        case UIOHOOK_ERROR_CREATE_EVENT_PORT:
            errorString = "Failed to create apple event port.";
            break;
        case UIOHOOK_ERROR_CREATE_RUN_LOOP_SOURCE:
            errorString = "Failed to create apple run loop source.";
            break;
        case UIOHOOK_ERROR_GET_RUNLOOP:
            errorString = "Failed to acquire apple run loop.";
            break;
        case UIOHOOK_ERROR_CREATE_OBSERVER:
            errorString = "Failed to create apple run loop observer.";
            break;

        // Generic
        case UIOHOOK_FAILURE: // fallthrough
        default:
            errorString = "An unknown hook error occurred.";
            break;
        }
        qCritical().nospace() << errorString << " (0x" << QString::number(hookStatus, 16) << ")";
    }

    bool onHookLog(unsigned int level, const char *format, ...)
    {
        // arbitrary size that _should_ be big enough for any logs
        // faster and easier than mallocing on each log
        const static int bufferSize = 200;
        char buffer[bufferSize]; 
        va_list args;
        switch (level) {
            case HookManager::LOG_LEVEL_WARN:
            case HookManager::LOG_LEVEL_ERROR:
                va_start(args, format);
                int length = vsnprintf(buffer, bufferSize, format, args);
                va_end(args);
                if (length > bufferSize) {
                    qWarning() << "Failed to handle hook log";
                    return -1;
                }
                if (length < 0) {
                    qWarning() << "Encoding error in hook log";
                    return length;
                }
                qWarning() << buffer;
                return false;
        }
        return false;
    }

    void getNewKeys(std::vector<int>& keys, std::vector<bool>& pressed)
    {
        const auto newKeys = globalShortcut->getShortcutKeys();
        if (!newKeys) {
            return;
        }
        if (*newKeys == keys) {
            return;
        }
        
        keys = *newKeys;
        pressed = std::vector<bool>(keys.size(), false);
    }

    bool setKeyPressed(std::vector<int>& keys, std::vector<bool>& pressed, int rawcode, bool newState)
    {
        auto it = std::find(keys.begin(), keys.end(), rawcode);
        if (it == keys.end()) {
            return false;
        }

        auto idx = std::distance(keys.begin(), it);
        pressed[idx] = newState;
        return true;
    }

    void blockKeyPropagation(HookManager::uiohook_event* const event)
    {
        // note: this does not work on X11. https://github.com/kwhat/libuiohook/issues/57#issuecomment-366757355
        event->reserved = 0x1;
    }

    void handleKeyChange(HookManager::uiohook_event* const event, bool keyState)
    {
        static bool active = false;
        static std::vector<int> keys;
        static std::vector<bool> pressed;

        getNewKeys(keys, pressed);
        if (!setKeyPressed(keys, pressed, event->data.keyboard.rawcode, keyState)) {
            return;
        }

        blockKeyPropagation(event);
        bool newActive = std::all_of(pressed.begin(), pressed.end(), [](bool val){return val == true;});
        if (active != newActive) {
            active = newActive;
            emit globalShortcut->toggled();
        }
    }

    // NOTE: The following callback executes on the same thread that hook_run() is called
    // from.  This is important because hook_run() attaches to the operating systems
    // event dispatcher and may delay event delivery to the target application.
    // Furthermore, some operating systems may choose to disable your hook if it
    // takes to long to process.  If you need to do any extended processing, please
    // do so by copying the event to your own queued dispatch thread.
    void onHookEvent(HookManager::uiohook_event* const event)
    {
        switch (event->type) {
            case HookManager::EVENT_KEY_PRESSED:
                handleKeyChange(event, true);
                break;
            case HookManager::EVENT_KEY_RELEASED:
                handleKeyChange(event, false);
                break;
            default:
                break;
        }
    }
}

bool HookManager::isPttSupported()
{
    QLibrary lib{hookLibName};
    return lib.load();
}

HookManager::HookManager(GlobalShortcut* _globalShortcut, QObject* parent)
    : QObject(parent)
{
    // Note: hook_run blocks. This class cannot receive any signals because of that.
    // Note: It's not really safe to call hook_stop off of hook's thread, but it's also not possible
    // to call if from hook's thread unless we call it from a key event callback, which leads
    // to us requiring mouse or keyboard movement to stop it.
    // It's still not _that_ unsafe because hook_run should be stuck in an Xlib call soon
    // after starting, not touching any lib state.
    globalShortcut = _globalShortcut;
    loadHookSymbols();
}

HookManager::~HookManager()
{
    if (!hookSymbolsLoaded) {
        return;
    }

    int status = hookStop();
    logError(status);
}

void HookManager::onStarted()
{
    if (!hookSymbolsLoaded) {
        return;
    }

    // Set the logger callback for library output.
    hookSetLoggerProc(&onHookLog);

    // Set the event callback for uiohook events.
    hookSetDispatchProc(&onHookEvent);

    process();
}

void HookManager::process()
{
    // Start the hook and block.
    int status = hookRun();
    logError(status);
};

void HookManager::loadHookSymbols()
{
    hookLib.setFileName(hookLibName);
    hookSetLoggerProc = reinterpret_cast<HookSetLoggerProcFunc>(hookLib.resolve("hook_set_logger_proc"));
    hookSetDispatchProc = reinterpret_cast<HookSetDispatchProcFunc>(hookLib.resolve("hook_set_dispatch_proc"));
    hookRun = reinterpret_cast<HookRunFunc>(hookLib.resolve("hook_run"));
    hookStop = reinterpret_cast<HookStopFunc>(hookLib.resolve("hook_stop"));
    if (hookSetLoggerProc &&
        hookSetDispatchProc &&
        hookRun &&
        hookStop) {
        hookSymbolsLoaded = true;
    } else {
        qCritical() << "Failed to load libuiohook symbols";
    }
}
