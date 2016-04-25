/*
    Copyright © 2015-2016 by The qTox Project

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
#include <QString>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include "src/persistence/settings.h"
#include "src/core/core.h"
#include "src/widget/gui.h"
#include <QDebug>
#include <QMessageBox>

ProfileImporter::ProfileImporter(QWidget *parent) : QWidget(parent)
{

}

bool ProfileImporter::importProfile()
{
    QString path = QFileDialog::getOpenFileName( this,
                                                tr("Import profile", "import dialog title"),
                                                QDir::homePath(),
                                                tr("Tox save file (*.tox)", "import dialog filter") );

    if (path.isEmpty())
         return false;

     QFileInfo info(path);
     QString profile = info.completeBaseName();

     if (info.suffix() != "tox")
     {
         QMessageBox::warning( this,
                              tr("Ignoring non-Tox file", "popup title"),
                              tr("Warning: You have chosen a file that is not a Tox save file; ignoring.", "popup text"),
                              QMessageBox::Ok);
         return false; //ingore importing non-tox file
     }

     QString profilePath = QDir(Settings::getInstance().getSettingsDirPath()).filePath(profile + Core::TOX_EXT);

     if (QFileInfo(profilePath).exists())
     {
         QMessageBox::StandardButton reply;
         reply = QMessageBox::warning( this,
                                      tr("Profile already exists", "import confirm title"),
                                      tr("A profile named \"%1\" already exists. Do you want to erase it?", "import confirm text").arg(profile),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            QFile::copy(path, profilePath);
            return true; //import successfull
        }
        else
        {
            return false; //import canelled
        }
     }
     else
     {
         QFile::copy(path, profilePath);
         return true; //import successfull
     }

}
