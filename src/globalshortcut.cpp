#include "src/globalshortcut.h"

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

#include <memory>

GlobalShortcut::GlobalShortcut(QObject *parent)
    : QObject(parent)
    , thread(new QThread())
    , hookManager{new HookManager{this}}
{
    connect(thread, &QThread::started, hookManager.get(), &HookManager::onStarted);
    hookManager->moveToThread(thread);
    thread->setObjectName("qTox global shortcut");
    thread->start();
}

void GlobalShortcut::onPttShortcutKeysChanged(QList<int> keys)
{
    QMutexLocker lock(&keysMutex);
    shortcutKeys = std::vector<int>(keys.begin(), keys.end());
}

std::unique_ptr<std::vector<int>> GlobalShortcut::getShortcutKeys()
{
    if (!keysMutex.tryLock()) {
        // we don't want to block the hook thread and cause system-wide stuttering.
        // failing to read and using a stale value will only happen when the shortcut was
        // just changed, in which case hook using the stale list for one extra key and
        // getting the new list on the next key stroke is fine
        return {};
    }

    auto ret = std::unique_ptr<std::vector<int>>{new std::vector<int>{shortcutKeys}};
    // Qt doesn't have a nice RAII wrapper like std::unique_lock, but std::mutex isn't working under mingw
    keysMutex.unlock();
    return ret;
}

GlobalShortcut::~GlobalShortcut()
{
    hookManager.reset(); // destroy hookmanager first to stop hook before stopping the thread
    thread->exit(0);
    thread->wait();
}
