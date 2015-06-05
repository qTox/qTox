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
    static QTranslator* translator;
    static QVector<QPair<void*, std::function<void()>>> callbacks;
    static QMutex lock;
};

#endif // TRANSLATOR_H
