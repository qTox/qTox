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


#include "gui.h"
#include "widget.h"
#include "src/nexus.h"
#include <assert.h>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QThread>

/**
 * @class GUI
 * @brief Abstracts the GUI from the target backend (DesktopGUI, ...)
 *
 * All the functions exposed here are thread-safe.
 * Prefer calling this class to calling a GUI backend directly.
 *
 * @fn void GUI::resized()
 * @brief Emitted when the GUI is resized on supported platforms.
 */

GUI::GUI(QObject *parent) :
    QObject(parent)
{
    assert(QThread::currentThread() == qApp->thread());
    assert(Nexus::getDesktopGUI());
    connect(Nexus::getDesktopGUI(), &Widget::resized, this, &GUI::resized);
}

/**
 * @brief Returns the singleton instance.
 */
GUI& GUI::getInstance()
{
    static GUI gui;
    return gui;
}

// Implementation of the public clean interface

/**
 * @brief Clear the GUI's contact list.
 */
void GUI::clearContacts()
{
    if (QThread::currentThread() == qApp->thread())
        getInstance()._clearContacts();
    else
        QMetaObject::invokeMethod(&getInstance(), "_clearContacts", Qt::BlockingQueuedConnection);
}

/**
 * @brief Will enable or disable the GUI.
 * @note A disabled GUI can't be interacted with by the user.
 * @param state Enable/disable GUI.
 */
void GUI::setEnabled(bool state)
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._setEnabled(state);
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_setEnabled", Qt::BlockingQueuedConnection,
                                  Q_ARG(bool, state));
    }
}

/**
 * @brief Change the title of the main window.
 * @param title Titile to set.
 *
 * This is usually always visible to the user.
 */
void GUI::setWindowTitle(const QString& title)
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._setWindowTitle(title);
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_setWindowTitle", Qt::BlockingQueuedConnection,
                                  Q_ARG(const QString&, title));
    }
}

/**
 * @brief Reloads the application theme and redraw the window.
 */
void GUI::reloadTheme()
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._reloadTheme();
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_reloadTheme", Qt::BlockingQueuedConnection);
    }
}

/**
 * @brief Optionally switches to a view of the qTox update being downloaded.
 */
void GUI::showUpdateDownloadProgress()
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._showUpdateDownloadProgress();
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_showUpdateDownloadProgress", Qt::BlockingQueuedConnection);
    }
}

/**
 * @brief Show some text to the user.
 * @param title Title of information window.
 * @param msg Text in information window.
 */
void GUI::showInfo(const QString& title, const QString& msg)
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._showInfo(title, msg);
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_showInfo", Qt::BlockingQueuedConnection,
                        Q_ARG(const QString&, title), Q_ARG(const QString&, msg));
    }
}

/**
 * @brief Show a warning to the user
 * @param title Title of warning window.
 * @param msg Text in warning window.
 */
void GUI::showWarning(const QString& title, const QString& msg)
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._showWarning(title, msg);
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_showWarning", Qt::BlockingQueuedConnection,
                        Q_ARG(const QString&, title), Q_ARG(const QString&, msg));
    }
}

/**
 * @brief Show an error to the user.
 * @param title Title of error window.
 * @param msg Text in error window.
 */
void GUI::showError(const QString& title, const QString& msg)
{
    if (QThread::currentThread() == qApp->thread())
    {
        // If the GUI hasn't started yet and we're on the main thread,
        // we still want to be able to show error messages
        if (!Nexus::getDesktopGUI())
            QMessageBox::critical(nullptr, title, msg);
        else
            getInstance()._showError(title, msg);
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_showError", Qt::BlockingQueuedConnection,
                        Q_ARG(const QString&, title), Q_ARG(const QString&, msg));
    }
}

/**
 * @brief Asks the user a question with Ok/Cancel or Yes/No buttons.
 * @param title Title of question window.
 * @param msg Text in question window.
 * @param defaultAns If is true, default was positive answer. Negative otherwise.
 * @param warning If is true, we will use a special warning style.
 * @param yesno Show "Yes" and "No" buttons.
 * @return True if the answer is positive, false otherwise.
 */
bool GUI::askQuestion(const QString& title, const QString& msg,
                      bool defaultAns, bool warning,
                      bool yesno)
{
    if (QThread::currentThread() == qApp->thread())
    {
        return getInstance()._askQuestion(title, msg, defaultAns, warning, yesno);
    }
    else
    {
        bool ret;
        QMetaObject::invokeMethod(&getInstance(), "_askQuestion", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret),
                                  Q_ARG(const QString&, title), Q_ARG(const QString&, msg),
                                  Q_ARG(bool, defaultAns), Q_ARG(bool, warning),
                                  Q_ARG(bool, yesno));
        return ret;
    }
}

/**
 * @brief Asks the user a question.
 *
 * The text for the displayed buttons can be specified.
 * @param title Title of question window.
 * @param msg Text in question window.
 * @param button1 Text of positive button.
 * @param button2 Text of negative button.
 * @param defaultAns If is true, default was positive answer. Negative otherwise.
 * @param warning If is true, we will use a special warning style.
 * @return True if the answer is positive, false otherwise.
 */
bool GUI::askQuestion(const QString& title, const QString& msg,
                      const QString& button1, const QString& button2,
                      bool defaultAns, bool warning)
{
    if (QThread::currentThread() == qApp->thread())
    {
        return getInstance()._askQuestion(title, msg, button1, button2, defaultAns, warning);
    }
    else
    {
        bool ret;
        QMetaObject::invokeMethod(&getInstance(), "_askQuestion", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret),
                                  Q_ARG(const QString&, title), Q_ARG(const QString&, msg),
                                  Q_ARG(bool, defaultAns), Q_ARG(bool, warning));
        return ret;
    }
}

/**
 * @brief Asks the user to input text and returns the answer.
 *
 * The interface is equivalent to QInputDialog::getItem()
 * @param parent Is the dialog's parent widget
 * @param title Is the text which is displayed in the title bar of the dialog.
 * @param label Is the text which is shown to the user (it should say what should be entered).
 * @param items Is the string list which is inserted into the combobox.
 * @param current Is the number of the item which should be the current item.
 * @param editable If is true the user can enter their own text, otherwise the user may only select one of the existing items.
 * @param ok If is nonnull will be set to true if the user pressed OK and to false if the user pressed Cancel.
 * @param flags The dialog will uses to widget flags.
 * @param hints Is the input method hints that will be used if the combobox is editable and an input method is active.
 * @return This function returns the text of the current item, or if editable is true, the current text of the combobox.
 */
QString GUI::itemInputDialog(QWidget * parent, const QString & title,
                    const QString & label, const QStringList & items,
                    int current, bool editable, bool * ok,
                    Qt::WindowFlags flags,
                    Qt::InputMethodHints hints)
{
    if (QThread::currentThread() == qApp->thread())
    {
        return getInstance()._itemInputDialog(parent, title, label, items, current, editable, ok, flags, hints);
    }
    else
    {
        QString r;
        QMetaObject::invokeMethod(&getInstance(), "_itemInputDialog", Qt::BlockingQueuedConnection,
                    Q_RETURN_ARG(QString, r),
                    Q_ARG(QWidget*, parent), Q_ARG(const QString&, title),
                    Q_ARG(const QString&,label), Q_ARG(const QStringList&, items),
                    Q_ARG(int, current), Q_ARG(bool, editable), Q_ARG(bool*, ok),
                    Q_ARG(Qt::WindowFlags, flags), Q_ARG(Qt::InputMethodHints, hints));
        return r;
    }
}

/**
 * @brief Asks the user to answer a password.
 * @param cancel Is the text on the cancel button.
 * @param body Is descriptive text that will be shown to the user.
 * @return Entered password.
 */
QString GUI::passwordDialog(const QString& cancel, const QString& body)
{
    if (QThread::currentThread() == qApp->thread())
    {
        return getInstance()._passwordDialog(cancel, body);
    }
    else
    {
        QString r;
        QMetaObject::invokeMethod(&getInstance(), "_passwordDialog", Qt::BlockingQueuedConnection,
                        Q_RETURN_ARG(QString, r),
                        Q_ARG(const QString&, cancel), Q_ARG(const QString&, body));
        return r;
    }
}

// Private implementations

void GUI::_clearContacts()
{
    Nexus::getDesktopGUI()->clearContactsList();
}

void GUI::_setEnabled(bool state)
{
    Nexus::getDesktopGUI()->setEnabled(state);
}

void GUI::_setWindowTitle(const QString& title)
{
    if (title.isEmpty())
        getMainWidget()->setWindowTitle("qTox");
    else
        getMainWidget()->setWindowTitle("qTox - " + title);
}

void GUI::_reloadTheme()
{
    Nexus::getDesktopGUI()->reloadTheme();
}

void GUI::_showInfo(const QString& title, const QString& msg)
{
    QMessageBox messageBox(QMessageBox::Information, title, msg, QMessageBox::Ok, getMainWidget());
    messageBox.setButtonText(QMessageBox::Ok, QApplication::tr("Ok"));
    messageBox.exec();
}

void GUI::_showWarning(const QString& title, const QString& msg)
{
    QMessageBox messageBox(QMessageBox::Warning, title, msg, QMessageBox::Ok, getMainWidget());
    messageBox.setButtonText(QMessageBox::Ok, QApplication::tr("Ok"));
    messageBox.exec();
}

void GUI::_showError(const QString& title, const QString& msg)
{
    QMessageBox messageBox(QMessageBox::Critical, title, msg, QMessageBox::Ok, getMainWidget());
    messageBox.setButtonText(QMessageBox::Ok, QApplication::tr("Ok"));
    messageBox.exec();
}

void GUI::_showUpdateDownloadProgress()
{
    Nexus::getDesktopGUI()->showUpdateDownloadProgress();
}

bool GUI::_askQuestion(const QString& title, const QString& msg,
                       bool defaultAns, bool warning,
                       bool yesno)
{
    QString positiveButton = yesno ? QApplication::tr("Yes") : QApplication::tr("Ok");
    QString negativeButton = yesno ? QApplication::tr("No") : QApplication::tr("Cancel");

    return _askQuestion(title, msg, positiveButton, negativeButton, defaultAns, warning);
}

bool GUI::_askQuestion(const QString& title, const QString& msg,
                       const QString& button1, const QString& button2,
                       bool defaultAns, bool warning)
{
    QMessageBox::Icon icon = warning ? QMessageBox::Warning : QMessageBox::Question;
    QMessageBox box(icon, title, msg, QMessageBox::NoButton, getMainWidget());
    QPushButton* pushButton1 = box.addButton(button1, QMessageBox::AcceptRole);
    QPushButton* pushButton2 = box.addButton(button2, QMessageBox::RejectRole);
    box.setDefaultButton(defaultAns ? pushButton1 : pushButton2);
    box.setEscapeButton(pushButton2);

    box.exec();
    return box.clickedButton() == pushButton1;
}

QString GUI::_itemInputDialog(QWidget * parent, const QString & title,
                              const QString & label, const QStringList & items,
                              int current, bool editable, bool * ok,
                              Qt::WindowFlags flags,
                              Qt::InputMethodHints hints)
{
    return QInputDialog::getItem(parent, title, label, items, current, editable, ok, flags, hints);
}

QString GUI::_passwordDialog(const QString& cancel, const QString& body)
{
    // we use a hack. It is considered that closing the dialog without explicitly clicking
    // disable history is confusing. But we can't distinguish between clicking the cancel
    // button and closing the dialog. So instead, we reverse the Ok and Cancel roles,
    // so that nothing but explicitly clicking disable history closes the dialog
    QString ret;
    QInputDialog dialog;
    dialog.setWindowTitle(tr("Enter your password"));
    dialog.setOkButtonText(cancel);
    dialog.setCancelButtonText(tr("Decrypt"));
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setTextEchoMode(QLineEdit::Password);
    dialog.setLabelText(body);

    // problem with previous hack: the default button is disable history, not decrypt.
    // use another hack to reverse the default buttons.
    // http://www.qtcentre.org/threads/49924-Change-property-of-QInputDialog-button
    QList<QDialogButtonBox*> l = dialog.findChildren<QDialogButtonBox*>();
    if (!l.isEmpty())
    {
        QPushButton* ok     = l.first()->button(QDialogButtonBox::Ok);
        QPushButton* cancel = l.first()->button(QDialogButtonBox::Cancel);
        if (ok && cancel)
        {
            ok->setAutoDefault(false);
            ok->setDefault(false);
            ok->setText(QApplication::tr("Ok"));
            cancel->setAutoDefault(true);
            cancel->setDefault(true);
            cancel->setText(QApplication::tr("Cancel"));
        }
        else
        {
            qWarning() << "PasswordDialog: Missing button!";
        }
    }
    else
    {
        qWarning() << "PasswordDialog: No QDialogButtonBox!";
    }

    // using similar code, set QLabels to wrap
    for (auto* label : dialog.findChildren<QLabel*>())
        label->setWordWrap(true);

    while (true)
    {
        int val = dialog.exec();
        if (val == QDialog::Accepted)
            return QString();

        ret = dialog.textValue();
        if (!ret.isEmpty())
            return ret;

        dialog.setTextValue("");
        dialog.setLabelText(body + "\n\n" + tr("You must enter a non-empty password:"));
    }
}

// Other

/**
 * @brief Get the main widget.
 * @return The main QWidget* of the application
 */
QWidget* GUI::getMainWidget()
{
    QWidget* maingui{nullptr};
    maingui = Nexus::getDesktopGUI();
    return maingui;
}
