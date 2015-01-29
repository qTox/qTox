#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include "systemtrayicon_private.h"
#include <QObject>

class QSystemTrayIcon;
class QMenu;

class SystemTrayIcon : public QObject
{
    Q_OBJECT
public:
    SystemTrayIcon();
    void setContextMenu(QMenu* menu);
    void show();
    void hide();
    void setVisible(bool);
    void setIcon(QIcon&& icon);

signals:
    void activated(QSystemTrayIcon::ActivationReason);

private:
    SystrayBackendType backendType;
    QSystemTrayIcon* qtIcon;
#ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    AppIndicator *unityIndicator;
    GtkWidget *unityMenu;
#endif
};

#endif // SYSTEMTRAYICON_H
