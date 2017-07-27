@echo off
rem *******************************************************************************************************
rem ** Script name...: BuildTechnicalReference_OT.bat                                                    **
rem ** Version.......: 1.0                                                                               **
rem ** Creation Date.: June 9, 2017                                                                      **
rem ** Update........: June 9, 2017                                                                      **
rem ** ------------------------------------------------------------------------------------------------- **
rem ** Purpose.......: Create OpenTM2 Technical Reference using Dita Open Toolkit.                       **
rem ** Prerequisites.: none.                                                                             **
rem ** Developer.....: David Walters                                                                     **
rem *******************************************************************************************************
echo.
cls

echo.
echo *-----------------------------------------------------------------------*
echo *  Start output creation using DITA OpenToolkit.                        *
echo *-----------------------------------------------------------------------*
echo.

:GetUserInput1
echo.
echo.
:RepeatInput1
set OTMType=0
echo *-----------------------------------------------------------------------*
echo *  Enter "1" to create PDF.                                             * 
echo *  Enter "2" to create HTML 5.                                          * 
echo *  Enter "3" to create HTML Help.                                       * 
echo *  Enter "9" to CANCEL processing.                                      * 
echo *-----------------------------------------------------------------------*
echo.
set /P OTMType=    Your selection (either "1", "2", "3" or "9"):    
if   %OTMType% EQU 1 goto GetUserInput2
if   %OTMType% EQU 2 goto GetUserInput2
if   %OTMType% EQU 3 goto GetUserInput2
if   %OTMType% EQU 9 GOTO CANCEL    
 echo.
 echo *-----------------------------------------------------------------------*
 echo *  Wrong input. You need to select either "1", "2", "3" or "9".         *
 echo *-----------------------------------------------------------------------*
 GOTO RepeatInput1

:GetUserInput2
echo.
echo.
:RepeatInput2
set OTMnumber=0
echo *-----------------------------------------------------------------------*
echo *  Enter "1" to  CONVERT   IBM DITA to Dita OpenToolkit files.          * 
echo *  Enter "2" to  SKIP      conversion.                                  * 
echo *  Enter "9" to  CANCEL    processing.                                  * 
echo *-----------------------------------------------------------------------*
echo.
set /P OTMnumber=    Your selection (either "1", "2" or "9"):    
IF   %OTMnumber% EQU 1 GOTO CONVERT
IF   %OTMnumber% EQU 2 GOTO BUILD     
IF   %OTMnumber% EQU 9 GOTO CANCEL    
 echo.
 echo *-----------------------------------------------------------------------*
 echo *  Wrong input. You need to select either "1", "2" or "9".              *
 echo *-----------------------------------------------------------------------*
 GOTO GetUserInput2

:CONVERT    
perl ConvertDITA_IBM2OT.perl  ..\Technical_Reference  ..\build\OT_TechRef  1 

:BUILD      
echo.
echo *-----------------------------------------------------------------------*
echo *  Start building output.                                               *
echo *-----------------------------------------------------------------------*
if %OTMtype% EQU 1 (
  echo    Creating PDF...
  call dita.bat -i OT_TechRef/Opentm2TechnicalReference.ditamap   -format=pdf        -output=out/tech_PDF       -verbose   -logfile=logs\TechRef_PDF.log
)
if %OTMtype% EQU 2 (
  echo    Creating HTML 5...
  call dita.bat -i OT_TechRef/Opentm2TechnicalReference.ditamap   -format=html5      -output=out/tech_HTML5     -verbose   -logfile=logs\TechRef_HTML5.log
)
if %OTMtype% EQU 3 (
  echo    Creating HTML Help...
  call dita.bat -i OT_TechRef/Opentm2TechnicalReference.ditamap   -format=htmlhelp   -output=out/tech_HTMLHELP  -verbose   -logfile=logs\TechRef_HMLHELP.log
)
echo *-----------------------------------------------------------------------*



:DONE
echo.
echo *-----------------------------------------------------------------------*
echo *  Build completed.                                                     *
echo *-----------------------------------------------------------------------*
echo.
GOTO EXIT

:CANCEL
echo.
echo *-----------------------------------------------------------------------*
echo *  User cancelled processing.                                           *
echo *-----------------------------------------------------------------------*
echo.
GOTO EXIT

:EXIT
echo on