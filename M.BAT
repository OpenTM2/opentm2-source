@echo off
rem Copyright (c) 2013, International Business Machines
rem Corporation and others.  All rights reserved.

if %1.==. goto help
goto next

:help
echo.
echo Syntax: m makefile [deb] [all] [continue]
echo.
echo makefile = name of the .MAK-file to use
echo deb      = build debug version
echo all      = rebuild all
echo continue = don't stop on errors
goto exit

:next
set _MAKEOPT=
set _MAKEDEBUG=
if  %2.==all. set _MAKEOPT=/A
if  %3.==all. set _MAKEOPT=/A
if  %4.==all. set _MAKEOPT=/A

if  %2.==continue. set _MAKEOPT=/I
if  %3.==continue. set _MAKEOPT=/I
if  %4.==continue. set _MAKEOPT=/I

if %2.==deb. set _MAKEDEBUG=1
if %3.==deb. set _MAKEDEBUG=1
if %4.==deb. set _MAKEDEBUG=1
if NOT "%_MAKEDEBUG%"=="" goto debug


rem =========== set nodebug directories =================
:nodebug
echo Building non-debug version
set _CL_OPTIONS_DLL=%_CL_OPTIONS_DLL_NODEBUG%
set _CL_OPTIONS_EXE=%_CL_OPTIONS_EXE_NODEBUG%
set _CL_CPP_OPTIONS_EXE=%_CL_CPP_OPTIONS_EXE_NODEBUG%
set _CL_CPP_OPTIONS_DLL=%_CL_CPP_OPTIONS_DLL_NODEBUG%
set _LINK_OPTIONS=%_LINK_OPT_BASE% %_LINK_OPT_NODEBUG%
set _LINK_OPTIONS_EXE=%_LINK_OPT_EXE% %_LINK_OPT_NODEBUG%
set _LINK_LIB_CRT=%_LINK_LIB_CRT_NODEBUG%
set _LINK_LIB_EXE=%_LINK_LIB_EXE_NODEBUG%
set _RES=%_RESWIN%
set _OBJ=%_OBJWIN%
set _OBJEXE=%_OBJWINEXE%
set _LIB=%_DLLWIN%
set _DLL=%_DLLWIN%
set _BIN=%_BINWIN%
set LIB=%vclib%;%_LIBWIN%
set HUNSPELLLIB=libhunspell.lib
goto makeit


:debug
rem =========== set debug directories =================
echo Building debug version
set _CL_OPTIONS_DLL=%_CL_OPTIONS_DLL_DEBUG%
set _CL_OPTIONS_EXE=%_CL_OPTIONS_EXE_DEBUG%
set _CL_CPP_OPTIONS_EXE=%_CL_CPP_OPTIONS_EXE_DEBUG%
set _CL_CPP_OPTIONS_DLL=%_CL_CPP_OPTIONS_DLL_DEBUG%
set _LINK_OPTIONS=%_LINK_OPT_BASE% %_LINK_OPT_DEBUG%
set _LINK_OPTIONS_EXE=%_LINK_OPT_EXE% %_LINK_OPT_DEBUG%
set _LINK_LIB_CRT=%_LINK_LIB_CRT_DEBUG%
set _LINK_LIB_EXE=%_LINK_LIB_EXE_DEBUG%
set _RES=%_RESWINCV%
set _OBJ=%_OBJWINCV%
set _OBJEXE=%_OBJWINEXECV%
set _DLL=%_DLLWINCV%
set _LIB=%_DLLWINCV%
set _BIN=%_BINWINCV%
set _DEBUGOPT=YES
set LIB=%vclib%;%_LIBWINCV%
set HUNSPELLLIB=libhunspelld.lib
goto makeit


:makeit

rem create output directories when necessary
echo Creating output directories
if not exist %_TEMP% mkdir %_TEMP%
if not exist %_OBJ% mkdir %_OBJ%
if not exist %_OBJEXE% mkdir %_OBJEXE%
if not exist %_DLL% mkdir %_DLL%
if not exist %_BIN% mkdir %_BIN%
if not exist %_RES% mkdir %_RES%
if not exist %_MAP% mkdir %_MAP%
if not exist %_IPF% mkdir %_IPF%
if not exist %_HLP% mkdir %_HLP%


echo Preparing OTM Markup Table Plugin
call m_otmmarkup.bat
echo Preparing User Markup Table Plugin
call m_usermarkup.bat

set BUILD_FAIL=0

echo Erasing %_ERR%
if exist %_ERR% @erase %_ERR%
echo cd temp
cd temp
echo Making %_BLD%\%1.%MAK%
nmake %_MAKEOPT% /s /c /f %_BLD%\%1.%MAK%
if ERRORLEVEL 1 set BUILD_FAIL=1

if %BUILD_FAIL% == 1  echo ---- ERROR: Makefile %_BLD%\%1.%MAK% not completed !!!! >>%_ERR%
if %BUILD_FAIL% == 1  echo ---- ERROR: Makefile %_BLD%\%1.%MAK% not completed !!!!

if exist %_TMPERR% copy %_ERR%+%_TMPERR% %_ERR% >NUL
if exist %_TMPERR% del %_TMPERR% >NUL
if exist %_ERR% type %_ERR%
cd %_DRIVE%\%_DEVDIR%

if %BUILD_FAIL% == 1 exit /b 1

goto exit

:exit
exit /b 0 
