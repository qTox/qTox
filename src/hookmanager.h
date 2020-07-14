#pragma once

#include <QObject>
#include <QLibrary>

class GlobalShortcut;

class HookManager : public QObject
{
Q_OBJECT
public:
    HookManager(GlobalShortcut* _globalShortcut, QObject* parent = nullptr);
    ~HookManager();
    static bool isPttSupported();
public slots:
    void onStarted();
public:
    // Needed definitions from uiohook.h, since we don't want to depent on uiohook directly
    /* Begin Error Codes */
    #define UIOHOOK_SUCCESS                          0x00
    #define UIOHOOK_FAILURE                          0x01

    // System level errors.
    #define UIOHOOK_ERROR_OUT_OF_MEMORY              0x02

    // Unix specific errors.
    #define UIOHOOK_ERROR_X_OPEN_DISPLAY             0x20
    #define UIOHOOK_ERROR_X_RECORD_NOT_FOUND         0x21
    #define UIOHOOK_ERROR_X_RECORD_ALLOC_RANGE       0x22
    #define UIOHOOK_ERROR_X_RECORD_CREATE_CONTEXT    0x23
    #define UIOHOOK_ERROR_X_RECORD_ENABLE_CONTEXT    0x24
    #define UIOHOOK_ERROR_X_RECORD_GET_CONTEXT       0x25

    // Windows specific errors.
    #define UIOHOOK_ERROR_SET_WINDOWS_HOOK_EX        0x30
    #define UIOHOOK_ERROR_GET_MODULE_HANDLE          0x31

    // Darwin specific errors.
    #define UIOHOOK_ERROR_AXAPI_DISABLED             0x40
    #define UIOHOOK_ERROR_CREATE_EVENT_PORT          0x41
    #define UIOHOOK_ERROR_CREATE_RUN_LOOP_SOURCE     0x42
    #define UIOHOOK_ERROR_GET_RUNLOOP                0x43
    #define UIOHOOK_ERROR_CREATE_OBSERVER            0x44

    typedef enum _log_level {
        LOG_LEVEL_DEBUG = 1,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR
    } log_level;

    typedef enum _event_type {
        EVENT_HOOK_ENABLED = 1,
        EVENT_HOOK_DISABLED,
        EVENT_KEY_TYPED,
        EVENT_KEY_PRESSED,
        EVENT_KEY_RELEASED,
        EVENT_MOUSE_CLICKED,
        EVENT_MOUSE_PRESSED,
        EVENT_MOUSE_RELEASED,
        EVENT_MOUSE_MOVED,
        EVENT_MOUSE_DRAGGED,
        EVENT_MOUSE_WHEEL
    } event_type;

    typedef struct _keyboard_event_data {
        uint16_t keycode;
        uint16_t rawcode;
        uint16_t keychar;
    } keyboard_event_data;

    typedef struct _mouse_event_data {
        uint16_t button;
        uint16_t clicks;
        int16_t x;
        int16_t y;
    } mouse_event_data;

    typedef struct _mouse_wheel_event_data {
        uint16_t clicks;
        int16_t x;
        int16_t y;
        uint8_t type;
        uint16_t amount;
        int16_t rotation;
        uint8_t direction;
    } mouse_wheel_event_data;

    typedef struct _uiohook_event {
        event_type type;
        uint64_t time;
        uint16_t mask;
        uint16_t reserved;
        union {
            keyboard_event_data keyboard;
            mouse_event_data mouse;
            mouse_wheel_event_data wheel;
        } data;
    } uiohook_event;

private:
    typedef void (*dispatcher_t)(uiohook_event *const);
    typedef bool (*logger_t)(unsigned int, const char *, ...);
    typedef int (*HookStopFunc)();
    typedef int (*HookRunFunc)();
    typedef void (*HookSetDispatchProcFunc)(dispatcher_t dispatch_proc);
    typedef void (*HookSetLoggerProcFunc)(logger_t dispatch_proc);
    void process();
    void loadHookSymbols();
    QLibrary hookLib;
    HookStopFunc hookStop = nullptr;
    HookRunFunc hookRun = nullptr;
    HookSetDispatchProcFunc hookSetDispatchProc = nullptr;
    HookSetLoggerProcFunc hookSetLoggerProc = nullptr;
    bool hookSymbolsLoaded = false;
};
