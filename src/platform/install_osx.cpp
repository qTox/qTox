#include "install_osx.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QProcess>
#include <QDir>

#include <unistd.h>

void osx::moveToAppFolder()
{
    if (qApp->applicationDirPath() != "/Applications/qtox.app/Contents/MacOS")
    {
        qDebug() << "OS X: Not in Applications folder";

        QMessageBox AskInstall;
        AskInstall.setIcon(QMessageBox::Question);
        AskInstall.setWindowModality(Qt::ApplicationModal);
        AskInstall.setText("Move to Applications folder?");
        AskInstall.setInformativeText("I can move myself to the Applications folder, keeping your downloads folder less cluttered.\r\n");
        AskInstall.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        AskInstall.setDefaultButton(QMessageBox::Yes);

        int AskInstallAttempt = AskInstall.exec(); //Actually ask the user

        if (AskInstallAttempt == QMessageBox::Yes)
        {
            QProcess *sudoprocess = new QProcess;
            QProcess *qtoxprocess = new QProcess;

            QString bindir = qApp->applicationDirPath();
            QString appdir = bindir;
            appdir.chop(15);
            QString sudo = bindir + "/qtox_sudo rsync -avzhpltK " + appdir + " /Applications";
            QString qtox = "open /Applications/qtox.app";

            QString appdir_noqtox = appdir;
            appdir_noqtox.chop(8);

            if ((appdir_noqtox + "qtox.app") != appdir) //quick safety check
            {
                qDebug() << "OS X: Attmepted to delete non qTox directory!";
                exit(EXIT_UPDATE_MACX_FAIL);
            }

            QDir old_app(appdir);

            sudoprocess->start(sudo); //Where the magic actually happens, safety checks ^
            sudoprocess->waitForFinished();

            if (old_app.removeRecursively()) //We've just deleted the running program
                qDebug() << "OS X: Cleaned up old directory";
            else
                qDebug() << "OS X: This should never happen, the directory failed to delete";

            if (fork() != 0) //Forking is required otherwise it won't actually cleanly launch
                exit(EXIT_UPDATE_MACX);

            qtoxprocess->start(qtox);

            exit(0); //Actually kills it
        }
    }
}
