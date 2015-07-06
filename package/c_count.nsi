; $Id: c_count.nsi,v 1.4 2015/07/05 23:58:02 tom Exp $
; Script generated with the Venis Install Wizard

; Define your application name
!define APPNAME "c_count"
!define VERSION_MAJOR "7"
!define VERSION_MINOR "16"
!define VERSION "${VERSION_MAJOR}.${VERSION_MINOR}"
!define APPNAMEANDVERSION "${APPNAME} ${VERSION}"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "\mingw\bin"
InstallDirRegKey HKLM "Software\${APPNAME}" "NSIS install_dir"
OutFile "NSIS-Output\${APPNAME}-${VERSION}-setup.exe"

; Modern interface settings
!include "MUI.nsh"

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_RESERVEFILE_LANGDLL

InstType "Full"		; SectionIn 1
InstType "Typical"	; SectionIn 2

Section "C_Count" Section1

	SectionIn 1 2

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\"
	File ".\bin\c_count.exe"
	CreateDirectory "$SMPROGRAMS\C_Count"

SectionEnd

Section "Documentation" Section2

	SectionIn 1 2

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\"
	File ".\share\man\man1\c_count.1"

SectionEnd

Section -FinishSection

	WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Section1} "Program"
	!insertmacro MUI_DESCRIPTION_TEXT ${Section2} "Documentation"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall

	;Remove from registry...
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
	DeleteRegKey HKLM "SOFTWARE\${APPNAME}"

	; Delete self
	Delete "$INSTDIR\uninstall.exe"

	; Clean up program
	Delete "$INSTDIR\c_count.exe"
	Delete "$INSTDIR\c_count.txt"

	; Remove remaining directories
	RMDir "$SMPROGRAMS\CCount"
	RMDir "$INSTDIR\"

SectionEnd

; eof