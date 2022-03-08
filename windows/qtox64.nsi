Unicode True
###################
#META
###################
!define APP_NAME "qTox"
!define COMP_NAME "Tox"
!define WEB_SITE "https://github.com/qTox/qTox"
!define VERSION "1.0.0.0"
!define DESCRIPTION "qTox Installer"
!define COPYRIGHT "The Tox Project"
!define INSTALLER_NAME "setup-qtox.exe"
!define MAIN_APP_EXE "bin\qtox.exe"
!define REG_ROOT "HKLM"
!define REG_APP_PATH "Software\Microsoft\Windows\CurrentVersion\App Paths\qtox.exe"
!define UNINSTALL_PATH "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
!define REG_START_MENU "Start Menu Folder"
var SM_Folder

Name "${APP_NAME}"
Caption "${APP_NAME}"
OutFile "${INSTALLER_NAME}"
BrandingText "${APP_NAME}"
InstallDir "$PROGRAMFILES64\${APP_NAME}"
SetCompressor /SOLID lzma

VIProductVersion "${VERSION}"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "CompanyName" "${COMP_NAME}"
VIAddVersionKey "LegalCopyright" "${COPYRIGHT}"
VIAddVersionKey "FileDescription" "${DESCRIPTION}"
VIAddVersionKey "FileVersion" "${VERSION}"

##############
#DEFINE MACROS
##############
;Set the name of the uninstall log
  !define UninstLog "uninstall.log"
  Var UninstLog

;AddItem macro
  !macro AddItem Path
    FileWriteUTF16LE $UninstLog "${Path}$\r$\n"
  !macroend
  !define AddItem "!insertmacro AddItem"

;File macro
  !macro File FileName
     IfFileExists "$OUTDIR\${FileName}" +2
     FileWriteUTF16LE $UninstLog "$OUTDIR\${FileName}$\r$\n"
     File "${FileName}"
  !macroend
  !define File "!insertmacro File"

;CreateShortcut macro
  !macro CreateShortcut FilePath FilePointer Pamameters Icon IconIndex
    FileWriteUTF16LE $UninstLog "${FilePath}$\r$\n"
    CreateShortcut "${FilePath}" "${FilePointer}" "${Pamameters}" "${Icon}" "${IconIndex}"
  !macroend
  !define CreateShortcut "!insertmacro CreateShortcut"

;Copy files macro
  !macro CopyFiles SourcePath DestPath
    IfFileExists "${DestPath}" +2
    FileWriteUTF16LE $UninstLog "${DestPath}$\r$\n"
    CopyFiles "${SourcePath}" "${DestPath}"
  !macroend
  !define CopyFiles "!insertmacro CopyFiles"

;Rename macro
  !macro Rename SourcePath DestPath
    IfFileExists "${DestPath}" +2
    FileWriteUTF16LE $UninstLog "${DestPath}$\r$\n"
    Rename "${SourcePath}" "${DestPath}"
  !macroend
  !define Rename "!insertmacro Rename"

;CreateDirectory macro
  !macro CreateDirectory Path
    CreateDirectory "${Path}"
    FileWriteUTF16LE $UninstLog "${Path}$\r$\n"
  !macroend
  !define CreateDirectory "!insertmacro CreateDirectory"

;SetOutPath macro
  !macro SetOutPath Path
    SetOutPath "${Path}"
    FileWriteUTF16LE $UninstLog "${Path}$\r$\n"
  !macroend
  !define SetOutPath "!insertmacro SetOutPath"

;WriteUninstaller macro
  !macro WriteUninstaller Path
    WriteUninstaller "${Path}"
    FileWriteUTF16LE $UninstLog "${Path}$\r$\n"
  !macroend
  !define WriteUninstaller "!insertmacro WriteUninstaller"

;WriteIniStr macro
  !macro WriteIniStr IniFile SectionName EntryName NewValue
     IfFileExists "${IniFile}" +2
     FileWriteUTF16LE $UninstLog "${IniFile}$\r$\n"
     WriteIniStr "${IniFile}" "${SectionName}" "${EntryName}" "${NewValue}"
  !macroend

;WriteRegStr macro
  !macro WriteRegStr RegRoot UnInstallPath Key Value
     FileWriteUTF16LE $UninstLog "${RegRoot} ${UnInstallPath}$\r$\n"
     WriteRegStr "${RegRoot}" "${UnInstallPath}" "${Key}" "${Value}"
  !macroend
  !define WriteRegStr "!insertmacro WriteRegStr"

;WriteRegDWORD macro
  !macro WriteRegDWORD RegRoot UnInstallPath Key Value
     FileWriteUTF16LE $UninstLog "${RegRoot} ${UnInstallPath}$\r$\n"
     WriteRegDWORD "${RegRoot}" "${UnInstallPath}" "${Key}" "${Value}"
  !macroend
  !define WriteRegDWORD "!insertmacro WriteRegDWORD"

;BackupFile macro
  !macro BackupFile FILE_DIR FILE BACKUP_TO
   IfFileExists "${BACKUP_TO}\*.*" +2
    CreateDirectory "${BACKUP_TO}"
   IfFileExists "${FILE_DIR}\${FILE}" 0 +2
    Rename "${FILE_DIR}\${FILE}" "${BACKUP_TO}\${FILE}"
  !macroend
  !define BackupFile "!insertmacro BackupFile"

;RestoreFile macro
  !macro RestoreFile BUP_DIR FILE RESTORE_TO
   IfFileExists "${BUP_DIR}\${FILE}" 0 +2
    Rename "${BUP_DIR}\${FILE}" "${RESTORE_TO}\${FILE}"
  !macroend
  !define RestoreFile "!insertmacro RestoreFile"

;BackupFiles macro
  !macro BackupFiles FILE_DIR FILE BACKUP_TO
   IfFileExists "${BACKUP_TO}\*.*" +2
    CreateDirectory "22222"
   IfFileExists "${FILE_DIR}\${FILE}" 0 +7
    FileWriteUTF16LE $UninstLog "${FILE_DIR}\${FILE}$\r$\n"
    FileWriteUTF16LE $UninstLog "${BACKUP_TO}\${FILE}$\r$\n"
    FileWriteUTF16LE $UninstLog "FileBackup$\r$\n"
    Rename "${FILE_DIR}\${FILE}" "${BACKUP_TO}\${FILE}"
    SetOutPath "${FILE_DIR}"
    File "${FILE}" #After the Original file is backed up write the new file.
  !macroend
  !define BackupFiles "!insertmacro BackupFiles"

;RestoreFiles macro
  !macro RestoreFiles BUP_FILE RESTORE_FILE
   IfFileExists "${BUP_FILE}" 0 +2
    CopyFiles "${BUP_FILE}" "${RESTORE_FILE}"
  !macroend
  !define RestoreFiles "!insertmacro RestoreFiles"

##############
#MODERN UI
##############
!include "MUI.nsh"

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_NOAUTOCLOSE
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!ifdef REG_START_MENU
  !define MUI_STARTMENUPAGE_NODISABLE
  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "qTox"
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "${REG_ROOT}"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "${UNINSTALL_PATH}"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${REG_START_MENU}"
  !insertmacro MUI_PAGE_STARTMENU Application $SM_Folder
!endif
!insertmacro MUI_PAGE_INSTFILES

Function finishpageaction
${CreateShortcut} "$DESKTOP\qTox.lnk" "$INSTDIR\${MAIN_APP_EXE}" "" "" ""
FunctionEnd

!define MUI_FINISHPAGE_SHOWREADME ""
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create Desktop Shortcut"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION finishpageaction

!define MUI_FINISHPAGE_RUN_FUNCTION Launch_qTox_without_Admin
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_LINK "Find qTox on GitHub"
!define MUI_FINISHPAGE_LINK_LOCATION "https://github.com/qTox/qTox"
!insertmacro MUI_PAGE_FINISH

Function Launch_qTox_without_Admin
   SetOutPath $INSTDIR
   ShellExecAsUser::ShellExecAsUser "" "$INSTDIR\${MAIN_APP_EXE}" ""
FunctionEnd

!define MUI_UNABORTWARNING
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

###################
#PREPARE UNINST LOG
###################
  ;Uninstall log file missing.
    LangString UninstLogMissing ${LANG_ENGLISH} "${UninstLog} not found!$\r$\nUninstallation cannot proceed!"

  Section "Create install directory"
    CreateDirectory "$INSTDIR"
    nsExec::ExecToStack 'icacls "$PROGRAMFILES64" /save "$TEMP\program-files-permissions.txt"'
    Pop $0 # return value/error/timeout
    Pop $1 # printed text, up to ${NSIS_MAX_STRLEN}
    FileOpen $0 "$TEMP\program-files-permissions.txt" r
      FileReadUTF16LE $0 $1 1024
      FileReadUTF16LE $0 $2 1024
    FileClose $0
    FileOpen $0 "$TEMP\qTox-install-file-permissions.txt" w
      FileWriteUTF16LE  $0 "$INSTDIR"
      FileWriteUTF16LE  $0 "$\r$\n"
      DetailPrint "Writing to file: $2"
      FileWriteUTF16LE  $0 "$2"
    FileClose $0
    nsExec::Exec 'icacls "" /restore "$TEMP\qTox-install-file-permissions.txt"'
  SectionEnd

  Section -openlogfile
    CreateDirectory "$INSTDIR"
    IfFileExists "$INSTDIR\${UninstLog}" +3
      FileOpen $UninstLog "$INSTDIR\${UninstLog}" w
    Goto +4
      SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
      FileOpen $UninstLog "$INSTDIR\${UninstLog}" a
      FileSeek $UninstLog 0 END
  SectionEnd

#################
#INSTALL
#################
Section "Install"
  SetShellVarContext all
  # Install files
  ${SetOutPath} "$INSTDIR"
  ${WriteUninstaller} "uninstall.exe"

  ${CreateDirectory} "$INSTDIR\bin"
  ${SetOutPath} "$INSTDIR\bin"
  ${File} "qtox\*.*"

  ${CreateDirectory} "$INSTDIR\bin\imageformats"
  ${SetOutPath} "$INSTDIR\bin\imageformats"
  File /nonfatal "qtox\imageformats\*.*"
  ${SetOutPath} "$INSTDIR\bin"

  ${CreateDirectory} "$INSTDIR\bin\iconengines"
  ${SetOutPath} "$INSTDIR\bin\iconengines"
  File /nonfatal "qtox\iconengines\*.*"
  ${SetOutPath} "$INSTDIR\bin"

  ${CreateDirectory} "$INSTDIR\bin\platforms"
  ${SetOutPath} "$INSTDIR\bin\platforms"
  File /nonfatal "qtox\platforms\*.*"
  ${SetOutPath} "$INSTDIR\bin"

  ${CreateDirectory} "$INSTDIR\bin\libsnore-qt5"
  ${SetOutPath} "$INSTDIR\bin\libsnore-qt5"
  File /nonfatal "qtox\libsnore-qt5\*.*"
  ${SetOutPath} "$INSTDIR\bin"

  # Create shortcuts
  ${CreateDirectory} "$SMPROGRAMS\qTox"
  ${CreateShortCut} "$SMPROGRAMS\qTox\qTox.lnk" "$INSTDIR\${MAIN_APP_EXE}" "" "" ""
  ${CreateShortCut} "$SMPROGRAMS\qTox\Uninstall qTox.lnk" "$INSTDIR\uninstall.exe" "" "" ""

  # Write setup/app info into the registry
  SetRegView 64
  ${WriteRegStr} "${REG_ROOT}" "${REG_APP_PATH}" "" "$INSTDIR\${MAIN_APP_EXE}"
  ${WriteRegStr} "${REG_ROOT}" "${REG_APP_PATH}" "Path" "$INSTDIR\bin\"
  ${WriteRegStr} ${REG_ROOT} "${UNINSTALL_PATH}" "DisplayName" "qTox"
  ${WriteRegStr} ${REG_ROOT} "${UNINSTALL_PATH}" "DisplayVersion" "1.17.5"
  ${WriteRegStr} ${REG_ROOT} "${UNINSTALL_PATH}" "Publisher" "The qTox Project"
  ${WriteRegStr} ${REG_ROOT} "${UNINSTALL_PATH}" "UninstallString" "$INSTDIR\uninstall.exe"
  ${WriteRegStr} ${REG_ROOT} "${UNINSTALL_PATH}" "URLInfoAbout" "https://qtox.github.io"

  # Register the tox: protocol
  ${WriteRegStr} HKCR "tox" "" "URL:tox Protocol"
  ${WriteRegStr} HKCR "tox" "URL Protocol" ""
  ${WriteRegStr} HKCR "tox\shell\open\command" "" "$INSTDIR\${MAIN_APP_EXE} %1"

  # Register the .tox file associations
  ${WriteRegStr} "HKCR" "Applications\qtox.exe\SupportedTypes" ".tox" ""
  ${WriteRegStr} HKCR ".tox" "" "toxsave"
  ${WriteRegStr} HKCR "toxsave" "" "Tox save file"
  ${WriteRegStr} HKCR "toxsave\DefaultIcon" "" "$INSTDIR\${MAIN_APP_EXE}"
  ${WriteRegStr} HKCR "toxsave\shell\open\command" "" "$INSTDIR\${MAIN_APP_EXE} %1"
SectionEnd


################
#UNINSTALL
################
Section Uninstall
  SetShellVarContext all
  ;If there's no uninstall log, we'll try anyway to clean what we can
  IfFileExists "$INSTDIR\${UninstLog}" +3
    Goto noLog

  Push $R0
  Push $R1
  Push $R2
  SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" r
  StrCpy $R1 -1

  GetLineCount:
    ClearErrors
    FileRead $UninstLog $R0
    IntOp $R1 $R1 + 1
    StrCpy $R0 $R0 -2
    Push $R0
    IfErrors 0 GetLineCount

  Pop $R0

  LoopRead:
    StrCmp $R1 0 LoopDone
    Pop $R0

    IfFileExists "$R0\*.*" 0 +3
      RMDir $R0  #is dir
    Goto +9
    IfFileExists $R0 0 +3
      Delete $R0 #is file
    Goto +6
    StrCmp $R0 "${REG_ROOT} ${REG_APP_PATH}" 0 +3
      DeleteRegKey ${REG_ROOT} "${REG_APP_PATH}" #is Reg Element
    Goto +3
    StrCmp $R0 "${REG_ROOT} ${UNINSTALL_PATH}" 0 +2
      DeleteRegKey ${REG_ROOT} "${UNINSTALL_PATH}" #is Reg Element

    IntOp $R1 $R1 - 1
    Goto LoopRead
  LoopDone:
  FileClose $UninstLog
  Delete "$INSTDIR\${UninstLog}"
  noLog:
  Delete /REBOOTOK "$INSTDIR\uninstall.exe"
  RMDir /r /REBOOTOK "$INSTDIR\bin"
  RMDir /REBOOTOK "$INSTDIR"
  Pop $R2
  Pop $R1
  Pop $R0

  ;Remove start menu entries
  RMDir /r /REBOOTOK "$SMPROGRAMS\qTox"

  ;Remove registry keys
  SetRegView 64
  DeleteRegKey ${REG_ROOT} "${REG_APP_PATH}"
  DeleteRegKey ${REG_ROOT} "${UNINSTALL_PATH}"
  DeleteRegKey HKCR "Applications\qtox.exe"
  DeleteRegKey HKCR ".tox"
  DeleteRegKey HKCR "tox"
  DeleteRegKey HKCR "toxsave"
SectionEnd
