; ShotCap Installer Script
; Created for nstechbytes

; Define constants
!define PRODUCT_NAME "ShotCap"
!define PRODUCT_VERSION "1.3"
!define PRODUCT_PUBLISHER "nstechbytes"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Include Modern UI and LogicLib for PATH manipulation
!include "MUI2.nsh"
!include "LogicLib.nsh"

; Set image definitions for wizard and header images
!define MUI_WELCOMEFINISHPAGE_BITMAP "wizard.bmp"

; Set compression
SetCompressor lzma

; General attributes
Name "${PRODUCT_NAME}"
OutFile "ShotCap_Setup_v${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
InstallDirRegKey HKLM "Software\${PRODUCT_NAME}" "Install_Dir"
RequestExecutionLevel admin

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "installer-icon.ico"
!define MUI_UNICON "uninstaller-icon.ico"

; Language Selection Dialog Settings
!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

; Wizard Pages
!insertmacro MUI_PAGE_WELCOME
!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller Pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Language file
!insertmacro MUI_LANGUAGE "English"

; PATH manipulation functions
!define Environ 'HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"'

Function AddToPath
  Exch $0
  Push $1
  Push $2
  Push $3
  Push $4
  
  ; Read current PATH
  ReadRegStr $1 ${Environ} "PATH"
  Push "$1;"
  Push "$0;"
  Call StrStr
  Pop $2
  ${If} $2 == ""
    ; Not found in PATH, add it
    StrCpy $2 "$1;$0"
    WriteRegExpandStr ${Environ} "PATH" $2
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
  ${EndIf}
  
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
FunctionEnd

Function un.RemoveFromPath
  Exch $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $5
  Push $6
  
  ReadRegStr $1 ${Environ} "PATH"
  StrCpy $5 $1 1 -1 ; Get last char
  ${If} $5 != ";" ; Add trailing semicolon if missing
    StrCpy $1 "$1;"
  ${EndIf}
  
  Push $1
  Push "$0;"
  Call un.StrStr
  Pop $2 ; Position of our dir in PATH
  ${If} $2 != ""
    StrLen $3 "$0;"
    StrLen $4 $1
    StrCpy $5 $1 $2 ; Part before our dir
    IntOp $2 $2 + $3 ; Position after our dir
    StrCpy $6 $1 "" $2 ; Part after our dir
    StrCpy $1 $5$6
    
    ; Remove any double semicolons
    Push $1
    Push ";;"
    Push ";"
    Call un.StrReplace
    Pop $1
    
    ; Remove trailing semicolon if present
    StrCpy $5 $1 1 -1
    ${If} $5 == ";"
      StrLen $5 $1
      IntOp $5 $5 - 1
      StrCpy $1 $1 $5
    ${EndIf}
    
    WriteRegExpandStr ${Environ} "PATH" $1
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
  ${EndIf}
  
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
FunctionEnd

Function StrStr
  Exch $R1 ; String to search for
  Exch
  Exch $R2 ; String to search in
  Push $R3
  Push $R4
  Push $R5
  StrCpy $R3 0
  StrLen $R4 $R1
  StrLen $R5 $R2
  ${If} $R5 >= $R4
    loop:
      StrCpy $R1 $R2 $R4 $R3
      StrCmp $R1 $R1 found
      IntOp $R3 $R3 + 1
      IntCmp $R3 $R5 done loop
    
    found:
      StrCpy $R1 $R2 "" $R3
    done:
  ${EndIf}
  
  Pop $R5
  Pop $R4
  Pop $R3
  Exch $R1
  Exch
  Exch $R2
FunctionEnd

Function un.StrStr
  Exch $R1 ; String to search for
  Exch
  Exch $R2 ; String to search in
  Push $R3
  Push $R4
  Push $R5
  StrCpy $R3 0
  StrLen $R4 $R1
  StrLen $R5 $R2
  ${If} $R5 >= $R4
    loop:
      StrCpy $R1 $R2 $R4 $R3
      StrCmp $R1 $R1 found
      IntOp $R3 $R3 + 1
      IntCmp $R3 $R5 done loop
    
    found:
      StrCpy $R1 $R3
      Goto done
    done:
  ${Else}
    StrCpy $R1 ""
  ${EndIf}
  
  Pop $R5
  Pop $R4
  Pop $R3
  Exch $R1
  Exch
  Exch $R2
FunctionEnd

Function un.StrReplace
  Exch $R2 ; Replacement string
  Exch 1
  Exch $R1 ; String to replace
  Exch 2
  Exch $R0 ; String to search in
  Push $R3
  Push $R4
  Push $R5
  Push $R6
  Push $R7
  
  StrCpy $R3 0
  StrLen $R4 $R1
  StrLen $R6 $R0
  StrLen $R7 $R2
  loop:
    StrCpy $R5 $R0 $R4 $R3
    StrCmp $R5 $R1 replace
    StrCmp $R3 $R6 done
    IntOp $R3 $R3 + 1
    Goto loop
  
  replace:
    StrCpy $R5 $R0 $R3
    IntOp $R3 $R3 + $R4
    StrCpy $R0 $R5$R2$R0 "" $R3
    IntOp $R3 $R3 - $R4
    IntOp $R3 $R3 + $R7
    IntOp $R6 $R6 - $R4
    IntOp $R6 $R6 + $R7
    Goto loop
  
  done:
    Pop $R7
    Pop $R6
    Pop $R5
    Pop $R4
    Pop $R3
    Pop $R1
    Exch $R0
    Exch
    Pop $R2
FunctionEnd

Section "ShotCap" SEC01
  SetOutPath "$INSTDIR"
  
  ; Add the ShotCap executable
  File "..\release\ShotCap.exe"
  
  ; Store installation folder
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "Install_Dir" $INSTDIR
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
  ; Write uninstall information to the registry
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\ShotCap.exe"

  ; Add to PATH
  Push "$INSTDIR"
  Call AddToPath
SectionEnd

Section "Uninstall"
  ; Remove application files
  Delete "$INSTDIR\ShotCap.exe"
  Delete "$INSTDIR\uninstall.exe"
  
  ; Remove from PATH
  Push "$INSTDIR"
  Call un.RemoveFromPath
  
  ; Remove registry entries
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "Software\${PRODUCT_NAME}"
  
  ; Remove installation directory if empty
  RMDir "$INSTDIR"
SectionEnd
