Name "QUpgrade"

OutFile "qupgrade-installer-win32.exe"

InstallDir $PROGRAMFILES\qupgrade

Page license 
Page directory
Page components
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

LicenseData ..\..\license.txt

Section ""

  SetOutPath $INSTDIR
  File /r ..\..\release\*.*
  WriteUninstaller $INSTDIR\QUpgrade_uninstall.exe
SectionEnd 

Section "Uninstall"
  Delete $INSTDIR\QUpgrade_uninstall.exe
  Delete $INSTDIR\*.*
  RMDir $INSTDIR
  Delete "$SMPROGRAMS\QUpgrade\*.*"
  RMDir "$SMPROGRAMS\QUpgrade\"
SectionEnd

Section "create Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\QUpgrade"
  CreateShortCut "$SMPROGRAMS\QUpgrade\uninstall.lnk" "$INSTDIR\QUpgrade_uninstall.exe" "" "$INSTDIR\QUpgrade_uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\QUpgrade\QUpgrade.lnk" "$INSTDIR\qupgrade.exe" "" "$INSTDIR\qupgrade.exe" 0
SectionEnd