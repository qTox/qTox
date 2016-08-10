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

class GUI : public QObject
{
    Q_OBJECT
public:
    static GUI& getInstance();
    static QWidget* getMainWidget();
    static void clearContacts();
    static void setEnabled(bool state);
    static void setWindowTitle(const QString& title);
    static void reloadTheme();
    static void showUpdateDownloadProgress();
    static void showInfo(const QString& title, const QString& msg);
    static void showWarning(const QString& title, const QString& msg);
    static void showError(const QString& title, const QString& msg);
    static bool askQuestion(const QString& title, const QString& msg,
                            bool defaultAns = false, bool warning = true,
                            bool yesno = true);

    static bool askQuestion(const QString& title, const QString& msg,
                            const QString& button1, const QString& button2,
                            bool defaultAns = false, bool warning = true);

    static QString itemInputDialog(QWidget * parent, const QString & title,
                    const QString & label, const QStringList & items,
                    int current = 0, bool editable = true, bool * ok = 0,
                    Qt::WindowFlags flags = 0,
                    Qt::InputMethodHints hints = Qt::ImhNone);

    static QString passwordDialog(const QString& cancel, const QString& body);

signals:
    void resized();

private:
    explicit GUI(QObject *parent = 0);

private slots:
    // Private implementation, those must be called from the GUI thread
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
