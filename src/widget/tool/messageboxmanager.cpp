/*
    Copyright © 2022 by The qTox Project Contributors

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

#include "messageboxmanager.h"

#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QPushButton>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

MessageBoxManager::MessageBoxManager(QWidget* parent)
    : QWidget(parent)
{
}

/**
 * @brief Show some text to the user.
 * @param title Title of information window.
 * @param msg Text in information window.
 */
void MessageBoxManager::showInfo(const QString& title, const QString& msg)
{
    if (QThread::currentThread() == qApp->thread()) {
        _showInfo(title, msg);
    } else {
        QMetaObject::invokeMethod(this, "_showInfo", Qt::BlockingQueuedConnection,
                                  Q_ARG(const QString&, title), Q_ARG(const QString&, msg));
    }
}

/**
 * @brief Show a warning to the user
 * @param title Title of warning window.
 * @param msg Text in warning window.
 */
void MessageBoxManager::showWarning(const QString& title, const QString& msg)
{
    if (QThread::currentThread() == qApp->thread()) {
        _showWarning(title, msg);
    } else {
        QMetaObject::invokeMethod(this, "_showWarning", Qt::BlockingQueuedConnection,
                                  Q_ARG(const QString&, title), Q_ARG(const QString&, msg));
    }
}

/**
 * @brief Show an error to the user.
 * @param title Title of error window.
 * @param msg Text in error window.
 */
void MessageBoxManager::showError(const QString& title, const QString& msg)
{
    if (QThread::currentThread() == qApp->thread()) {
        _showError(title, msg);
    } else {
        QMetaObject::invokeMethod(this, "_showError", Qt::BlockingQueuedConnection,
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
bool MessageBoxManager::askQuestion(const QString& title, const QString& msg, bool defaultAns, bool warning, bool yesno)
{
    if (QThread::currentThread() == qApp->thread()) {
        return _askQuestion(title, msg, defaultAns, warning, yesno);
    } else {
        bool ret;
        QMetaObject::invokeMethod(this, "_askQuestion", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret), Q_ARG(const QString&, title),
                                  Q_ARG(const QString&, msg), Q_ARG(bool, defaultAns),
                                  Q_ARG(bool, warning), Q_ARG(bool, yesno));
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
bool MessageBoxManager::askQuestion(const QString& title, const QString& msg, const QString& button1,
                      const QString& button2, bool defaultAns, bool warning)
{
    if (QThread::currentThread() == qApp->thread()) {
        return _askQuestion(title, msg, button1, button2, defaultAns, warning);
    } else {
        bool ret;
        QMetaObject::invokeMethod(this, "_askQuestion", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret), Q_ARG(const QString&, title),
                                  Q_ARG(const QString&, msg), Q_ARG(bool, defaultAns),
                                  Q_ARG(bool, warning));
        return ret;
    }
}

void MessageBoxManager::confirmExecutableOpen(const QFileInfo& file)
{
    static const QStringList dangerousExtensions = {"app",  "bat",     "com",    "cpl",  "dmg",
                                                    "exe",  "hta",     "jar",    "js",   "jse",
                                                    "lnk",  "msc",     "msh",    "msh1", "msh1xml",
                                                    "msh2", "msh2xml", "mshxml", "msi",  "msp",
                                                    "pif",  "ps1",     "ps1xml", "ps2",  "ps2xml",
                                                    "psc1", "psc2",    "py",     "reg",  "scf",
                                                    "sh",   "src",     "vb",     "vbe",  "vbs",
                                                    "ws",   "wsc",     "wsf",    "wsh"};

    if (dangerousExtensions.contains(file.suffix())) {
        bool answer = askQuestion(tr("Executable file", "popup title"),
                                       tr("You have asked qTox to open an executable file. "
                                          "Executable files can potentially damage your computer. "
                                          "Are you sure want to open this file?",
                                          "popup text"),
                                       false, true);
        if (!answer) {
            return;
        }

        // The user wants to run this file, so make it executable and run it
        QFile(file.filePath())
            .setPermissions(file.permissions() | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup
                            | QFile::ExeOther);
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(file.filePath()));
}

// Private implementations
void MessageBoxManager::_showInfo(const QString& title, const QString& msg)
{
    QMessageBox messageBox(QMessageBox::Information, title, msg, QMessageBox::Ok, this);
    messageBox.setButtonText(QMessageBox::Ok, QApplication::tr("Ok"));
    messageBox.exec();
}

void MessageBoxManager::_showWarning(const QString& title, const QString& msg)
{
    QMessageBox messageBox(QMessageBox::Warning, title, msg, QMessageBox::Ok, this);
    messageBox.setButtonText(QMessageBox::Ok, QApplication::tr("Ok"));
    messageBox.exec();
}

void MessageBoxManager::_showError(const QString& title, const QString& msg)
{
    QMessageBox messageBox(QMessageBox::Critical, title, msg, QMessageBox::Ok, this);
    messageBox.setButtonText(QMessageBox::Ok, QApplication::tr("Ok"));
    messageBox.exec();
}


bool MessageBoxManager::_askQuestion(const QString& title, const QString& msg, bool defaultAns, bool warning,
                       bool yesno)
{
    QString positiveButton = yesno ? QApplication::tr("Yes") : QApplication::tr("Ok");
    QString negativeButton = yesno ? QApplication::tr("No") : QApplication::tr("Cancel");

    return _askQuestion(title, msg, positiveButton, negativeButton, defaultAns, warning);
}

bool MessageBoxManager::_askQuestion(const QString& title, const QString& msg, const QString& button1,
                       const QString& button2, bool defaultAns, bool warning)
{
    QMessageBox::Icon icon = warning ? QMessageBox::Warning : QMessageBox::Question;
    QMessageBox box(icon, title, msg, QMessageBox::NoButton, this);
    QPushButton* pushButton1 = box.addButton(button1, QMessageBox::AcceptRole);
    QPushButton* pushButton2 = box.addButton(button2, QMessageBox::RejectRole);
    box.setDefaultButton(defaultAns ? pushButton1 : pushButton2);
    box.setEscapeButton(pushButton2);

    box.exec();
    return box.clickedButton() == pushButton1;
}
