#ifndef INTERFACE_H
#define INTERFACE_H

#include <functional>

#define DECLARE_SIGNAL(name, ...) \
    using Slot_##name = std::function<void (__VA_ARGS__)>; \
    virtual void connectTo_##name(Slot_##name slot) const = 0

#define SIGNAL_IMPL(classname, name, ...) \
    using Slot_##name = std::function<void (__VA_ARGS__)>; \
    Q_SIGNAL void name(__VA_ARGS__); \
    void connectTo_##name(Slot_##name slot) const override { \
        connect(this, &classname::name, slot); \
    }

#define CHANGED_SIGNAL(name, ...) \
    DECLARE_SIGNAL(name##Changed, __VA_ARGS__)

#define CHANGED_SIGNAL_IMPL(classname, name, ...) \
    SIGNAL_IMPL(classname, name##Changed, __VA_ARGS__)

#endif // INTERFACE_H
