@ECHO OFF
rem Copyright (c) 2013,2014 International Business Machines
rem Corporation and others.  All rights reserved.

call my-config.bat

rem Directory for release files
set RELEASE_DIR=%_DRIVE%\%_DEVDIR%\RELEASE

rem directory for OpenTM2 Scripter GUI
set SCRIPTERGUI_DIR=%_DRIVE%\%_DEVDIR%\openTM2ScripterGUI
rem folder containing the sample data
set SAMPLEDATADIR=%_DRIVE%\%_DEVDIR%\SAMPLEDATA

rem folder containing the EQF... markup tables and other table stuff
set EQFTABLES=%_DRIVE%\%_DEVDIR%\EQFTABLES

rem folders containing stuff from other sources (e.g. Xalan/Xerces)
set PACKAGESBIN=%_DRIVE%\%_DEVDIR%\PACKAGES\BIN
set PACKAGESLIB=%_DRIVE%\%_DEVDIR%\PACKAGES\LIB
set PACKAGESINCLUDE=%_DRIVE%\%_DEVDIR%\PACKAGES\INCLUDE

rem
rem variables used in the make files
rem    These settings normally don't require any adjustments
set _LANGUAGE=ENGLISH
set _BINWIN=%_DRIVE%\%_DEVDIR%\BIN
set _BINWINCV=%_DRIVE%\%_DEVDIR%\BIN-Debug
set _DLLWIN=%_DRIVE%\%_DEVDIR%\BIN
set _DLLWINCV=%_DRIVE%\%_DEVDIR%\BIN-Debug
set _DOC=%_DRIVE%\%_DEVDIR%\Doc
set _OBJWIN=%_DRIVE%\%_DEVDIR%\OBJ
set _OBJWINCV=%_DRIVE%\%_DEVDIR%\OBJ-Debug
set _OBJWINEXE=%_DRIVE%\%_DEVDIR%\OBJ-EXE
set _OBJWINEXECV=%_DRIVE%\%_DEVDIR%\OBJ-EXE-Debug
set _BLD=%_DRIVE%\%_DEVDIR%\BUILD
set _DEP=%_BLD%
set _MRI=%_DRIVE%\%_DEVDIR%\MRI
set _SRC=%_DRIVE%\%_DEVDIR%\SOURCE
set _INC=%_DRIVE%\%_DEVDIR%\INCLUDE
set _WININC=%PACKAGESINCLUDE%
set _ID=%_DRIVE%\%_DEVDIR%\MRI
set _RC=%_DRIVE%\%_DEVDIR%\MRI
set _RCMFC=%_DRIVE%\%_DEVDIR%\MRI
set _DLG=%_DRIVE%\%_DEVDIR%\MRI
set _STR=%_DRIVE%\%_DEVDIR%\MRI
set _MEN=%_DRIVE%\%_DEVDIR%\MRI
set _ACC=%_DRIVE%\%_DEVDIR%\MRI
set _ICO=%_DRIVE%\%_DEVDIR%\MRI
set _MSG=%_DRIVE%\%_DEVDIR%\MSG
set _LIBOTHER=%PACKAGESLIB%
set _RESWINCV=%_OBJWINCV%
set _RESWIN=%_OBJWIN%
set _RESTQM=%_OBJTQM%
set _DEF=%_BLD%
set _LIBWIN=%_DLLWIN%
set _LIBWINCV=%_DLLWINCV%
set _HLP=%_DRIVE%\%_DEVDIR%\HELPWIN
set _RTF=%_DRIVE%\%_DEVDIR%\HELPWIN
set _IPF=%_DRIVE%\%_DEVDIR%\HELP
set _TEMP=%_DRIVE%\%_DEVDIR%\TEMP
set _TMP=%_DRIVE%\%_DEVDIR%\TEMP
set _ERR=%_TEMP%\ERR
set _TMPERR=%_TEMP%\TMP_ERR
set _MAP=%_DRIVE%\%_DEVDIR%\MAPWIN
set DEPFILE=%_BLD%\EQFCDEP.MOZ
set MAKEDEP=N
set _TESTSUITE=%_DRIVE%\%_DEVDIR%\OpenTM2_TestSuite


set INCLUDE=%_INC%;%_MRI%;%_WININC%;%vcinclude%;%_SRC%;%_AXIS2C_INCL%;%_ICU_INCL%;%_HUNSPELL_INCL%
set LIB=%vclib%;%_LIB%;%ICULIB%;%HUNSPELLLIB%

rem compiler options

rem disable some compiler warnings:
rem 4201 (level 4) = nonstandard extension used : nameless struct/union
rem 4214 (level 4) = nonstandard extension used : bit field types other than int
rem 4238 (level 4) = nonstandard extension used : class rvalue used as lvalue
rem 4996 (level 3) = 'function': was declared deprecated
set _CL_WARNINGS=/W4 /wd4201 /wd4214 /wd4238 /wd4996
rem set _CL_WARNINGS=/W4 /wd4201 /wd4214 /wd4238

rem base compiler options used for all compiles
set _CL_OPT_BASE=/nologo /c /Zp1 /EHsc %_CL_WARNINGS% /D_WINDOWS /DWIN32BIT /D_WIN32 /D_LNG_JAP  /D_MT /D_WINDLL /DKOREA /D_CRT_SECURE_NO_WARNINGS /D_USE_32BIT_TIME_T /D ZLIB_WINAPI
set _CL_OPT_BASE=%_CL_OPT_BASE% /TP /DCPPTEST

rem compile options C files for executables - debug/nodebug
set _CL_OPTIONS_EXE_DEBUG=%_CL_OPT_BASE% /Zi /Od /Ob2 /FR /GA -Od -Zi /D_DEBUG /DDEBUGLOG /DSHOWDEBUGTITLEBARTEXT
set _CL_OPTIONS_EXE_NODEBUG=%_CL_OPT_BASE% /Zi /Od /Ob2 /FR /GA -Od -Gs /DNDEBUG

rem compile options C files for DLLs - debug/nodebug
set _CL_OPTIONS_DLL_DEBUG=%_CL_OPT_BASE% -Od -Zi /D_DEBUG /DDEBUGLOG /DSHOWDEBUGTITLEBARTEXT
set _CL_OPTIONS_DLL_NODEBUG=%_CL_OPT_BASE% -Od -Gs /DNDEBUG

rem compile options CPP files for executables - debug/nodebug
set _CL_CPP_OPTIONS_EXE_DEBUG=%_CL_OPT_BASE% -Od -Zi /D_DEBUG /DDEBUGLOG /D_EQF_MFC /FD /MTd
set _CL_CPP_OPTIONS_EXE_NODEBUG=%_CL_OPT_BASE% /D_EQF_MFC /FD /MT /O2 -Gs /D_EQF_MFC /DNDEBUG

rem compile options CPP files for DLLs - debug/nodebug
set _CL_CPP_OPTIONS_DLL_DEBUG=%_CL_OPT_BASE% /Gm  /Zi /Od /D_DEBUG /DDEBUGLOG /FD /MDd
set _CL_CPP_OPTIONS_DLL_NODEBUG=%_CL_OPT_BASE% /D_EQF_MFC /FD /MD /O2 -Gs /DNDEBUG

rem resource compiler options
set _RC_OPT=-D_WINDOWS -D_ALLTOP97 -DWIN32BIT -DOEM_MT -D_EQF_MFC

rem linker options
set _LINK_LIB_CRT_DEBUG= /DLL oldnames.lib msvcrtd.lib msvcprtd.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib
set _LINK_LIB_CRT_NODEBUG= /DLL oldnames.lib msvcrt.lib msvcprt.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib
set _LINK_LIB_EXE_DEBUG= oldnames.lib msvcrtd.lib msvcprtd.lib libcmt.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib
set _LINK_LIB_EXE_NODEBUG= oldnames.lib msvcrt.lib msvcprt.lib libcmt.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib
set _LINK_LIB_DOS= libcmt.lib
set _LINK_OPT_BASE= /MACHINE:IX86 /NOLOGO /NOD /ALIGN:0x1000 /DRIVER
set _LINK_OPT_DEBUG= /DEBUG
set _LINK_OPT_NODEBUG=
set _LINK_OPT_EXE= /MACHINE:IX86 /NOLOGO /NOD /ALIGN:0x1000 /DRIVER
set _LINK_CPP_OPTIONS=/nologo /subsystem:windows /incremental:no /machine:I386 /DEBUG
set _LINK_DLL_OPTIONS=/nologo /subsystem:windows /dll /incremental:no /machine:i386 /debug
set _CL_OPTIONS_DLL=%_CL_OPTIONS_DLL_DEBUG%
set _CL_OPTIONS_EXE=%_CL_OPTIONS_EXE_DEBUG%
set _CL_CPP_OPTIONS_EXE=%_CL_CPP_OPTIONS_EXE_DEBUG%
set _CL_CPP_OPTIONS_DLL=%_CL_CPP_OPTIONS_DLL_DEBUG%
set _LINK_OPTIONS=%_LINK_OPT_BASE% %_LINK_OPT_DEBUG%
set LIB=%vclib%;%_LIB%
set _BIN=%_BINWINCV%
set _OBJ=%_OBJWINCV%
set _OBJEXE=%_OBJWINEXECV%
set _DLL=%_DLLWINCV%
set _RC_COMPILER=rc
set _COMPILER=cl
set _LINKER=link
set _DEFEXT=DEF
set _IMPLIBER=LIB
set MAK=MAK

set _IPFCPREPFLAGS=-D _WINDOWS
set _IPF_CP=850
set _IPF_LNG=ENU
set _IPF_CNTRY=001
set _IPF_COMPILER=IPFC
set _MSG_MAKER=MKMSGF
SET PL=W
SET _NLSCHAR=E
SET _IPFNLS=%_IPF%\IPFENU.NLS
SET _TOCNAME=Contents
SET _IPFDBCS=N

call setvar_otmmarkup.bat

call setvar_usermarkup.bat

PROMPT [OpenTM2]$p$g
