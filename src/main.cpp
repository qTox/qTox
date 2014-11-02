/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "widget/widget.h"
#include "misc/settings.h"
#include <QApplication>
#include <QFontDatabase>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>

#ifdef LOG_TO_FILE
static QtMessageHandler dflt;
static QTextStream* logFile {nullptr};

void myMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg)
{
    if (!logFile)
        return;

    dflt(type, ctxt, msg);
    *logFile << QTime::currentTime().toString("HH:mm:ss' '") << msg << '\n';
    logFile->flush();
}
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("qTox");
    a.setOrganizationName("Tox");

#ifdef LOG_TO_FILE
    logFile = new QTextStream;
    dflt = qInstallMessageHandler(nullptr);
    QFile logfile(QDir(Settings::getSettingsDirPath()).filePath("qtox.log"));
    logfile.open(QIODevice::Append);
    logFile->setDevice(&logfile);

    *logFile << QDateTime::currentDateTime().toString("yyyy-dd-MM HH:mm:ss' file logger starting\n'");
    qInstallMessageHandler(myMessageHandler);
#endif

    // Windows platform plugins DLL hell fix
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    a.addLibraryPath("platforms");
    
    qDebug() << "built on: " << __TIME__ << __DATE__;
    qDebug() << "commit: " << GIT_VERSION << "\n";

    // Install Unicode 6.1 supporting font
    QFontDatabase::addApplicationFont("://DejaVuSans.ttf");

    Widget* w = Widget::getInstance();

    int errorcode = a.exec();

    delete w;
#ifdef LOG_TO_FILE
    delete logFile;
    logFile = nullptr;
#endif

    return errorcode;
}
