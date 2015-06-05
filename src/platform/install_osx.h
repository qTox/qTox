#ifndef INSTALLOSX_H
#define INSTALLOSX_H

#ifndef Q_OS_MACX
#error "This file is only meant to be compiled for Mac OSX targets"
#endif

namespace osx
{
    static constexpr int EXIT_UPDATE_MACX = 218; // We track our state using unique exit codes when debugging
    static constexpr int EXIT_UPDATE_MACX_FAIL = 216;

    void moveToAppFolder();
}

#endif // INSTALLOSX_H
