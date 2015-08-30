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
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Import profile", "import dialog title"),
                                                QDir::homePath(),
                                                tr("Tox save file (*.tox)", "import dialog filter"));

    if (path.isEmpty())
         return false; // pfff

     QFileInfo info(path);
     QString profile = info.completeBaseName();

     if (info.suffix() != "tox")
     {
         QMessageBox::warning(this,
                              tr("Ignoring non-Tox file", "popup title"),
                              tr("Warning: You have chosen a file that is not a Tox save file; ignoring.", "popup text"),
                              QMessageBox::Ok);
         return false; //ingore importing non-tox file
     }

     QString profilePath = QDir(Settings::getInstance().getSettingsDirPath()).filePath(profile + Core::TOX_EXT);

     if (QFileInfo(profilePath).exists())
     {
         QMessageBox::StandardButton reply;
         reply = QMessageBox::warning(this,
                                       tr("Profile already exists", "import confirm title"),
                                       tr("A profile named \"%1\" already exists. Do you want to erase it?", "import confirm text").arg(profile),
                                       QMessageBox::Yes);

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
         return false; //path doesnt exists
     }

}
