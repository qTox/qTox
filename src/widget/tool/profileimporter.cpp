/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include "profileimporter.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include "src/core/core.h"
#include "src/persistence/settings.h"

/**
 * @class ProfileImporter
 * @brief This class provides the ability to import profile.
 * @note This class can be used before @a Nexus instance will be created,
 * consequently it can't use @a GUI class. Therefore it should use QMessageBox
 * to create dialog forms.
 */

ProfileImporter::ProfileImporter(Settings& settings_, QWidget* parent)
    : QWidget(parent)
    , settings{settings_}
{
}

/**
 * @brief Show a file dialog. Selected file will be imported as Tox profile.
 * @return True, if the import was succesful. False otherwise.
 */
bool ProfileImporter::importProfile()
{
    QString title = tr("Import profile", "import dialog title");
    QString filter = tr("Tox save file (*.tox)", "import dialog filter");
    QString dir = QDir::homePath();

    // TODO: Change all QFileDialog instances across project to use
    // this instead of Q_NULLPTR. Possibly requires >Qt 5.9 due to:
    // https://bugreports.qt.io/browse/QTBUG-59184
    QString path = QFileDialog::getOpenFileName(Q_NULLPTR, title, dir, filter);

    return importProfile(path);
}

/**
 * @brief Asks the user a question with Yes/No choices.
 * @param title Title of question window.
 * @param message Text in question window.
 * @return True if the answer is positive, false otherwise.
 */
bool ProfileImporter::askQuestion(QString title, QString message)
{
    QMessageBox::Icon icon = QMessageBox::Warning;
    QMessageBox box(icon, title, message, QMessageBox::NoButton, this);
    QPushButton* pushButton1 = box.addButton(QApplication::tr("Yes"), QMessageBox::AcceptRole);
    QPushButton* pushButton2 = box.addButton(QApplication::tr("No"), QMessageBox::RejectRole);
    box.setDefaultButton(pushButton2);
    box.setEscapeButton(pushButton2);

    box.exec();
    return box.clickedButton() == pushButton1;
}

/**
 * @brief Try to import Tox profile.
 * @param path Path to Tox profile.
 * @return True, if the import was succesful. False otherwise.
 */
bool ProfileImporter::importProfile(const QString& path)
{
    if (path.isEmpty())
        return false;

    QFileInfo info(path);
    if (!info.exists()) {
        QMessageBox::warning(this, tr("File doesn't exist"), tr("Profile doesn't exist"),
                             QMessageBox::Ok);
        return false;
    }

    QString profile = info.completeBaseName();

    if (info.suffix() != "tox") {
        QMessageBox::warning(this, tr("Ignoring non-Tox file", "popup title"),
                             tr("Warning: You have chosen a file that is not a "
                                "Tox save file; ignoring.",
                                "popup text"),
                             QMessageBox::Ok);
        return false; // ingore importing non-tox file
    }

    QString settingsPath = settings.getPaths().getSettingsDirPath();
    QString profilePath = QDir(settingsPath).filePath(profile + Core::TOX_EXT);

    if (QFileInfo(profilePath).exists()) {
        QString title = tr("Profile already exists", "import confirm title");
        QString message = tr("A profile named \"%1\" already exists. "
                             "Do you want to erase it?",
                             "import confirm text")
                              .arg(profile);
        bool erase = askQuestion(title, message);

        if (!erase)
            return false; // import canelled

        QFile(profilePath).remove();
    }

    QFile::copy(path, profilePath);
    // no good way to update the ui from here... maybe we need a Widget:refreshUi() function...
    // such a thing would simplify other code as well I believe
    QMessageBox::information(this, tr("Profile imported"),
                             tr("%1.tox was successfully imported").arg(profile), QMessageBox::Ok);

    return true; // import successfull
}
