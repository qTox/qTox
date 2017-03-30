@echo off

echo.
echo Note: This script assumes that you have installed both MinGW and MSYS via
echo       mingw-get and that the mingw-get directory has been added to the PATH.
echo.

REM We need the shell to run the shell script but we don’t want it in our path permanently because we’re going to use CMAKE later.
REM This assumes that MYSYS has been installed in its default location via mingw-get.

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
