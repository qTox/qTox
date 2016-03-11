/*
    Copyright © 2014 by The qTox Project

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
    along with qTox.  If not, see <http://www.gnu.org/licenses/>
*/


#include "widget.h"
#include "settings.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <memory>
#include <windows.h>

static std::unique_ptr<QTextStream> logFileStream {nullptr};
static std::unique_ptr<QFile> logFileFile {nullptr};
static QMutex mutex;

void logMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg)
{
    // Silence qWarning spam due to bug in QTextBrowser (trying to open a file for base64 images)
    if (ctxt.function == QString("virtual bool QFSFileEngine::open(QIODevice::OpenMode)")
            && msg == QString("QFSFileEngine::open: No file name specified"))
        return;

    QString LogMsg = QString("[%1] %2:%3 : ")
            .arg(QTime::currentTime().toString("HH:mm:ss.zzz")).arg(ctxt.file).arg(ctxt.line);
    switch (type)
    {
        case QtDebugMsg:
            LogMsg += "Debug";
            break;
        case QtWarningMsg:
            LogMsg += "Warning";
            break;
        case QtCriticalMsg:
            LogMsg += "Critical";
            break;
        case QtFatalMsg:
            LogMsg += "Fatal";
            break;
        default:
            break;
    }

    LogMsg += ": " + msg + "\n";

    QTextStream out(stderr, QIODevice::WriteOnly);
    out << LogMsg;

    if (!logFileStream)
        return;

    QMutexLocker locker(&mutex);
    *logFileStream << LogMsg;
    logFileStream->flush();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logMessageHandler);
    QApplication a(argc, argv);
    Settings s;

    logFileStream.reset(new QTextStream);
    logFileFile.reset(new QFile(s.getSettingsDirPath()+"qtox.log"));
    if (logFileFile->open(QIODevice::Append))
    {
        logFileStream->setDevice(logFileFile.get());
        *logFileStream << QDateTime::currentDateTime().toString("\nyyyy-MM-dd HH:mm:ss' Updater file logger starting\n'");
    }
    else
    {
        qWarning() << "Couldn't open log file!\n";
        logFileStream.release();
    }

    long unsigned int bufsize=100;
    char buf[100];
    GetUserNameA(buf, &bufsize);
    qDebug() << "Updater running as user" << buf;

    Widget w(s);
    w.show();

    return a.exec();
}
