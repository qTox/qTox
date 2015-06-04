#ifndef GUI_H
#define GUI_H

#include <QObject>

class QWidget;

/// Abstracts the GUI from the target backend (AndroidGUI, DesktopGUI, ...)
/// All the functions exposed here are thread-safe
/// Prefer calling this class to calling a GUI backend directly
class GUI : public QObject
{
    Q_OBJECT
public:
    static GUI& getInstance();
    /// Returns the main QWidget* of the application
    static QWidget* getMainWidget();
    /// Clear the GUI's contact list
    static void clearContacts();
    /// Will enable or disable the GUI.
    /// A disabled GUI can't be interacted with by the user
    static void setEnabled(bool state);
    /// Change the title of the main window
    /// This is usually always visible to the user
    static void setWindowTitle(const QString& title);
    /// Reloads the application theme and redraw the window
    static void reloadTheme();
    /// Show some text to the user, for example in a message box
    static void showInfo(const QString& title, const QString& msg);
    /// Show a warning to the user, for example in a message box
    static void showWarning(const QString& title, const QString& msg);
    /// Show an error to the user, for example in a message box
    static void showError(const QString& title, const QString& msg);
    /// Asks the user a question, for example in a message box.
    /// If warning is true, we will use a special warning style.
    /// Returns the answer.
    static bool askQuestion(const QString& title, const QString& msg,
                            bool defaultAns = false, bool warning = true,
                            bool yesno = true);
    /// Asks the user a question, for example in a message box.
    /// The text for the displayed buttons can be specified.
    /// If warning is true, we will use a special warning style.
    /// Returns the answer.
    static bool askQuestion(const QString& title, const QString& msg,
                            const QString& button1, const QString& button2,
                            bool defaultAns = false, bool warning = true);
    /// Asks the user to input text and returns the answer.
    /// The interface is equivalent to QInputDialog::getItem()
    static QString itemInputDialog(QWidget * parent, const QString & title,
                    const QString & label, const QStringList & items,
                    int current = 0, bool editable = true, bool * ok = 0,
                    Qt::WindowFlags flags = 0,
                    Qt::InputMethodHints hints = Qt::ImhNone);
    /// Asks the user to answer a password
    /// cancel is the text on the cancel button and body
    /// is descriptive text that will be shown to the user
    static QString passwordDialog(const QString& cancel, const QString& body);

signals:
    /// Emitted when the GUI is resized on supported platforms
    /// Guaranteed to work on desktop platforms
    void resized();

private:
    explicit GUI(QObject *parent = 0);

    // Private implementation, those must be called from the GUI thread
private slots:
    void _clearContacts();
    void _setEnabled(bool state);
    void _setWindowTitle(const QString& title);
    void _reloadTheme();
    void _showInfo(const QString& title, const QString& msg);
    void _showWarning(const QString& title, const QString& msg);
    void _showError(const QString& title, const QString& msg);
    bool _askQuestion(const QString& title, const QString& msg,
                      bool defaultAns = false, bool warning = true,
                      bool yesno = true);
    bool _askQuestion(const QString& title, const QString& msg,
                      const QString& button1, const QString& button2,
                      bool defaultAns = false, bool warning = true);
    QString _itemInputDialog(QWidget * parent, const QString & title,
                        const QString & label, const QStringList & items,
                        int current = 0, bool editable = true, bool * ok = 0,
                        Qt::WindowFlags flags = 0,
                        Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
    QString _passwordDialog(const QString& cancel, const QString& body);
};

#endif // GUI_H
