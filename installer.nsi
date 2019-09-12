; Windows Installer for co2amp
; Author: Mikhail Polyanskiy (polyanskiy@bnl.gov)
; Brookhaven National Laboratory, USA

!include "MUI.nsh"

;General
Name "co2amp"
OutFile "co2amp_v.20190911_setup.exe"

;Default install path
InstallDir "$PROGRAMFILES64\co2amp"          ;default
InstallDirRegKey HKLM "Software\co2amp" "" ;if previous installation exists (overrides default)


;-------------------------Interface Settings---------------------------

!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
!define CO2AMP_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\co2amp"
!define CO2AMP_ROOT_KEY "Applications\co2i.exe"

;Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
  
;Uninstaller pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;Languages
!insertmacro MUI_LANGUAGE "English"


;-------------------------Installer Sections---------------------------

Section "Section_01" Sec01

  ;Read previous installation path from registry
  ReadRegDWORD $0 HKLM "Software\co2amp" ""

  ;Remove previous installation
  SetShellVarContext all
  RMDir /r $0                                   
  RMDir /r "$SMPROGRAMS\co2amp"
  RMDir /r "$INSTDIR"

  ;Write files to installation directory
  SetOutPath "$INSTDIR\src\co2amp"
  File "co2amp\*.c"
  File "co2amp\*.h"
  File "co2amp\*.pro"
  SetOutPath "$INSTDIR\src\co2i"
  File "co2i\*.cpp"
  File "co2i\*.h"
  File "co2i\*.ui"
  File "co2i\*.rc"
  File "co2i\*.qrc"
  File "co2i\*.pro"
  SetOutPath "$INSTDIR\src\co2i\images"
  File "co2i\images\*"
  SetOutPath "$INSTDIR\src\images"
  File "images\*"
  SetOutPath "$INSTDIR\src"
  File "installer.nsi"
  SetOutPath "$INSTDIR\doc"
  File "doc\tex\co2amp.pdf"
  SetOutPath "$INSTDIR\doc\tex"
  File "doc\tex\*.tex"
  File "doc\tex\*.bib"
  SetOutPath "$INSTDIR"
  File "co2amp\release\co2amp.exe"
  File "co2i\release\co2i.exe"
  File "C:\Qt\Tools\mingw730_64\bin\libgcc_s_seh-1.dll"
  File "C:\Qt\Tools\mingw730_64\bin\libgomp-1.dll"
  File "C:\Qt\Tools\mingw730_64\bin\libstdc++-6.dll"
  File "C:\Qt\Tools\mingw730_64\bin\libwinpthread-1.dll"
  File "C:\Qt\5.12.3\mingw73_64\bin\Qt5Core.dll"
  File "C:\Qt\5.12.3\mingw73_64\bin\Qt5Gui.dll"
  File "C:\Qt\5.12.3\mingw73_64\bin\Qt5Widgets.dll"
  File "C:\Qt\5.12.3\mingw73_64\bin\Qt5Svg.dll"
  File "libyaml\libyaml-0-2.dll"
  SetOutPath "$INSTDIR\platforms"
  File "C:\Qt\5.12.3\mingw73_64\plugins\platforms\qwindows.dll"
  
  ;Write Start menu entries
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\co2amp"
  CreateShortCut "$SMPROGRAMS\co2amp\co2i.lnk" "$INSTDIR\co2i.exe"
  ;SetOutPath "$SMPROGRAMS\co2amp"
  ;SetOverwrite on
  ;File "co2amp homepage.url"
  CreateShortCut "$SMPROGRAMS\co2amp\Uninstall co2amp.lnk" "$INSTDIR\uninstall.exe" "" ""
  
  ;Create desktope shortcut
  CreateShortCut "$DESKTOP\co2i.lnk" "$INSTDIR\co2i.exe"

  ;Registry
  WriteRegStr HKLM "SOFTWARE\co2amp" "" $INSTDIR
  ;WriteRegStr HKCR "${CO2AMP_ROOT_KEY}\SupportedTypes" ".co2" ""
  ;WriteRegStr HKCR "${CO2AMP_ROOT_KEY}\shell\open" "FriendlyAppName" "co2amp"
  WriteRegStr HKCR "${CO2AMP_ROOT_KEY}\shell\open\command" "" '"$INSTDIR\co2i.exe" "%1$"'
  
  ;Register extensions
  WriteRegStr HKCR ".co2\OpenWithProgIds" "co2i.co2" ""
  ;WriteRegStr HKCR "co2i.co2\shell\open" "FriendlyAppName" "co2i";
  WriteRegStr HKCR "co2i.co2\shell\open\command" "" '"$INSTDIR\co2i.exe" "%1"'

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "${CO2AMP_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "${CO2AMP_UNINST_KEY}" "DisplayName" "co2amp"
  WriteRegStr HKLM "${CO2AMP_UNINST_KEY}" "DisplayIcon" "$INSTDIR\co2i.exe"

SectionEnd


;-------------------------Uninstaller Section---------------------------

Section "Uninstall"

  SetShellVarContext all
  RMDir /r "$SMPROGRAMS\co2amp"
  Delete "$DESKTOP\co2amp.lnk"

  Delete "$INSTDIR\uninstall.exe"
  RMDir /r "$INSTDIR"

  ;unregister extensions
  DeleteRegKey HKLM "Software\co2amp"
  DeleteRegKey HKLM "${CO2AMP_UNINST_KEY}"
  DeleteRegKey HKCR "${CO2AMP_ROOT_KEY}"
  DeleteRegValue HKCR ".co2\OpenWithProgIds" "co2amp.co2"
  DeleteRegKey HKCR "co2amp.co2"

SectionEnd
