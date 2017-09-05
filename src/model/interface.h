#ifndef INTERFACE_H
#define INTERFACE_H

#include <functional>

#define CHANGED_SIGNAL(type, name) \
    using Slot_##name = std::function<void (const type& val)>; \
    virtual void connectTo_##name##Changed(Slot_##name slot) = 0; \
    virtual void connectTo_##name##Changed(QObject* handler, Slot_##name slot) = 0

#define CHANGED_SIGNAL_IMPL(type, classname, name) \
    void connectTo_##name##Changed(Slot_##name slot) override { \
            connect(this, &classname::name##Changed, slot); \
    } \
    void connectTo_##name##Changed(QObject* handler, Slot_##name slot) override { \
        connect(this, &classname::name##Changed, handler, slot); \
    }

#endif // INTERFACE_H
