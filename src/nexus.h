#ifndef NEXUS_H
#define NEXUS_H

#include <QObject>

class Widget;
class AndroidGUI;
class Profile;
class LoginScreen;
class Core;

/// This class is in charge of connecting various systems together
/// and forwarding signals appropriately to the right objects
/// It is in charge of starting the GUI and the Core
class Nexus : public QObject
{
    Q_OBJECT
public:
    void start(); ///< Sets up invariants and calls showLogin
    void showLogin(); ///< Hides the man GUI, delete the profile, and shows the login screen
    /// Hides the login screen and shows the GUI for the given profile.
    /// Will delete the current GUI, if it exists.
    void showMainGUI();

    static Nexus& getInstance();
    static void destroyInstance();
    static Core* getCore(); ///< Will return 0 if not started
    static Profile* getProfile(); ///< Will return 0 if not started
    static void setProfile(Profile* profile); ///< Delete the current profile, if any, and replaces it
    static AndroidGUI* getAndroidGUI(); ///< Will return 0 if not started
    static Widget* getDesktopGUI(); ///< Will return 0 if not started
    static QString getSupportedImageFilter();
    static bool tryRemoveFile(const QString& filepath); ///< Dangerous way to find out if a path is writable

private:
    explicit Nexus(QObject *parent = 0);
    ~Nexus();

private:
    Profile* profile;
    Widget* widget;
    AndroidGUI* androidgui;
    LoginScreen* loginScreen;
};

#endif // NEXUS_H
