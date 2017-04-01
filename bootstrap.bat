@echo off

echo.
echo Note: This script assumes that you have installed MinGW and added its binary
echo       folder (where mingw-get can be found) to your PATH. 
echo       In case you have MSYS already installed, make sure it is not part of your
echo       PATH, as any   sh   or   bash   in your PATH will make later parts of the
echo       compile process fail.
echo.
echo       In case you need to edit your PATH to that effect, now's the time. :)
echo.
pause

echo.
echo.
echo   ****************************************************************************
echo   ***  Making sure the necessary MinGW packages are installed...           ***
echo   ****************************************************************************
echo.
echo This will try to install the necessary MinGW packages. Don't worry about package
echo already installed error messages, as they're expected if you have (some of) the
echo packages already present on your system.
echo.
mingw-get update
mingw-get install mingw-get
mingw-get install mingw-developer-toolkit mingw32-base mingw32-gcc-g++ msys-base mingw32-pthreads-w32
echo.
echo Done configuring MinGW.

REM We need the shell to run the shell script but we don’t want it in our path permanently because we’re going to use CMAKE later.
REM This assumes that MYSYS has been installed in its default location via mingw-get.
echo.
echo.
echo   ****************************************************************************
echo   ***  Finding MSYS and temporarily adding it to the PATH...               ***
echo   ****************************************************************************
echo.
REM Step 1: Find out where mingw-get is.
set MINGW_GET_PATH=
for /f "delims=" %%a in ('where mingw-get') do @set MINGW_GET_PATH=%%a

REM Step 2: Extract the folder from the information we got.
for %%a in (%MINGW_GET_PATH%) do (
    set MINGW_GET_FILE=%%~fa
    set MINGW_GET_FILEPATH=%%~dpa
    set MINGW_GET_FILENAME=%%~nxa
) 
REM echo This is how it works: 
REM echo %MINGW_GET_FILE% = %MINGW_GET_FILEPATH% + %MINGW_GET_FILENAME%

REM Step 3: Add MSYS to the PATH for this session (needed in shell script lateron)
set "PATH=%MINGW_GET_FILEPATH%..\msys\1.0\bin\;%PATH%"

echo Done.

echo.
echo.
echo   ****************************************************************************
echo   ***  Executing shell script for library and tool setup...                ***
echo   ****************************************************************************
echo.
REM Step 4: Use sh.exe directly from the MYSYS directory 
REM (cannot be started from PATH as the internal PATH variable of the cmd is not refreshed)
%MINGW_GET_FILEPATH%..\msys\1.0\bin\sh.exe bootstrap.sh

echo.
echo.
echo   ****************************************************************************
echo   ***  Finished setting up the libraries and environment variables.        ***
echo   ***                                                                      ***
echo   ***  Important:                                                          ***
echo   ***  Due to a bug in QtCreator, you will have to restart your PC for     ***
echo   ***  some of the changes to take effekt.                                 ***
echo   ***                                                                      ***
echo   ***  Afterwards, just open CMakeLists.txt with QtCreator.                ***
echo   ****************************************************************************
echo.
echo.

pause
