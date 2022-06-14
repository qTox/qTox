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

#pragma once

#include <QObject>

#include <memory>

class MessageBoxManager;
class Settings;
class IPC;
class QApplication;
class ToxURIDialog;
class Nexus;
class CameraSource;

class AppManager : public QObject
{
    Q_OBJECT

public:
    AppManager(int& argc, char** argv);
    ~AppManager();
    int run();

private slots:
    void cleanup();
private:
    void preConstructionInitialization();
    std::unique_ptr<QApplication> qapp;
    std::unique_ptr<MessageBoxManager> messageBoxManager;
    std::unique_ptr<Settings> settings;
    std::unique_ptr<IPC> ipc;
    std::unique_ptr<ToxURIDialog> uriDialog;
    std::unique_ptr<CameraSource> cameraSource;
    std::unique_ptr<Nexus> nexus;
};
