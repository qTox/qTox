/*
    Copyright © 2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef GUI_H
#define GUI_H

#include <QObject>

class QWidget;

/// Abstracts the GUI from the target backend (DesktopGUI, ...)
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
    /// Optionally switches to a view of the qTox update being downloaded
    static void showUpdateDownloadProgress();
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
    void _showUpdateDownloadProgress();
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
