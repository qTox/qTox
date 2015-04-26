#ifndef NEXUS_H
#define NEXUS_H

#include <QObject>

class QThread;
class Core;
class Widget;
class AndroidGUI;

/// This class is in charge of connecting various systems together
/// and forwarding signals appropriately to the right objects
/// It is in charge of starting the GUI and the Core
class Nexus : public QObject
{
    Q_OBJECT
public:
    void start(); ///< Will initialise the systems (GUI, Core, ...)

    static Nexus& getInstance();
    static void destroyInstance();
    static Core* getCore(); ///< Will return 0 if not started
    static AndroidGUI* getAndroidGUI(); ///< Will return 0 if not started
    static Widget* getDesktopGUI(); ///< Will return 0 if not started
    static QString getSupportedImageFilter();
    static bool tryRemoveFile(const QString& filepath); ///< Dangerous way to find out if a path is writable

private:
    explicit Nexus(QObject *parent = 0);
    ~Nexus();

private:
    Core* core;
    QThread* coreThread;
    Widget* widget;
    AndroidGUI* androidgui;
    bool started;
};

#endif // NEXUS_H
