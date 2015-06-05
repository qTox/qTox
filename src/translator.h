#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QVector>
#include <QPair>
#include <QMutex>
#include <functional>

class QTranslator;

class Translator
{
public:
    /// Loads the translations according to the settings or locale
    static void translate();
    /// Register a function to be called when the UI needs to be retranslated
    static void registerHandler(std::function<void()>, void* owner);
    /// Unregisters all handlers of an owner
    static void unregister(void* owner);

private:
    using Callback = QPair<void*, std::function<void()>>;
    static QVector<Callback> callbacks;
    static QMutex lock;
    static QTranslator* translator;
};

#endif // TRANSLATOR_H
