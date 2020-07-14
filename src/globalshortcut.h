#pragma once

#include <memory>

#include <QObject>
#include <QMutex>

#include "src/hookmanager.h"

class QThread;

class GlobalShortcut : public QObject
{
Q_OBJECT
public:
    GlobalShortcut(QObject *parent = 0);
    ~GlobalShortcut();
    std::unique_ptr<std::vector<int>> getShortcutKeys();

public slots:
    void onPttShortcutKeysChanged(QList<int> keys);

signals:
    void toggled();

private:
    std::unique_ptr<HookManager> hookManager;
    QThread* thread;
    std::vector<int> shortcutKeys;
    QMutex keysMutex;
};
