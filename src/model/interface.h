#ifndef INTERFACE_H
#define INTERFACE_H

#include <functional>

/**
 * @file interface.h
 *
 * Qt doesn't support QObject multiple inheritance. But for declaring signals
 * in class, it should be inherit QObject. To avoid this issue, interface can
 * provide some pure virtual methods, which allow to connect to some signal.
 *
 * This file provides macros to make signals declaring easly. With this macros
 * signal-like method will be declared and implemented in one line each.
 *
 * @example
 * class IExample {
 * public:
 *     // Like signal: void valueChanged(int value) const;
 *     // Declare `connectTo_valueChanged` method.
 *     DECLARE_SIGNAL(valueChanged, int value);
 * };
 *
 * class Example : public QObject, public IExample {
 * public:
 *     // Declare real signal and implement `connectTo_valueChanged`
 *     SIGNAL_IMPL(Example, valueChanged, int value);
 * };
 */

/**
 * @def DECLARE_SIGNAL
 * @brief Decalre signal-like method. Should be used in interface
 */
#define DECLARE_SIGNAL(name, ...) \
    using Slot_##name = std::function<void (__VA_ARGS__)>; \
    virtual void connectTo_##name(Slot_##name slot) const = 0

/**
 * @def SIGNAL_IMPL
 * @brief Declare signal and implement signal-like method.
 */
#define SIGNAL_IMPL(classname, name, ...) \
    using Slot_##name = std::function<void (__VA_ARGS__)>; \
    Q_SIGNAL void name(__VA_ARGS__); \
    void connectTo_##name(Slot_##name slot) const override { \
        connect(this, &classname::name, slot); \
    }

#endif // INTERFACE_H
