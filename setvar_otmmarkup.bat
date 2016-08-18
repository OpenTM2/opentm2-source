@ECHO OFF
rem Copyright (c) 2013, International Business Machines
rem Corporation and others.  All rights reserved.

echo Prepare OtmMarkupTablePlugin items.

set _OTMMARKUP_SRC=%_SRC%\plugins\markup\OtmMarkupTablePlugin
set _OTMMARKUP_MAP=%_MAP%\plugins\markup\OtmMarkupTablePlugin
set _OTMMARKUP_RELEASE_DIR=%RELEASE_DIR%\otm\plugins\OtmMarkupTablePlugin

set _OTMMARKUP_DLL_NODEBUG=%_DLLWIN%\plugins\markup\OtmMarkupTablePlugin
set _OTMMARKUP_DLL_DEBUG=%_DLLWINCV%\plugins\markup\OtmMarkupTablePlugin
set _OTMMARKUP_OBJ_NODEBUG=%_OBJWIN%\plugins\markup\OtmMarkupTablePlugin
set _OTMMARKUP_OBJ_DEBUG=%_OBJWINCV%\plugins\markup\OtmMarkupTablePlugin
set _OTMMARKUP_CFLAGS_NODEBUG=/nologo /c /W2 /MT /EHsc /TC /I%_OTMMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /wd4229
set _OTMMARKUP_CFLAGS_DEBUG=/nologo /c /W2 /MT /EHsc /TC /I%_OTMMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /Zi /wd4229
set _OTMMARKUP_CUFLAGS_NODEBUG=/nologo /c /W2 /MT /EHsc /TC /I%_OTMMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /DUNICODE /D_UNICODE /wd4229
set _OTMMARKUP_CUFLAGS_DEBUG=/nologo /c /W2 /MT /EHsc /TC /I%_OTMMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /DUNICODE /D_UNICODE /Zi /wd4229
set _OTMMARKUP_CPPFLAGS_NODEBUG=/nologo /c /W2 /MT /EHsc /TP /I%_OTMMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /DUNICODE /D_UNICODE /wd4229
set _OTMMARKUP_CPPFLAGS_DEBUG=/nologo /c /W2 /MT /EHsc /TP /I%_OTMMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /DUNICODE /D_UNICODE /Zi /wd4229
set _OTMMARKUP_LINKFLAGS_NODEBUG=/nologo /MACHINE:IX86 /ALIGN:0X1000 /DRIVER /DLL
set _OTMMARKUP_LINKFLAGS_DEBUG=/nologo /MACHINE:IX86 /ALIGN:0X1000 /DRIVER /DLL /DEBUG

if not exist %_OTMMARKUP_DLL_NODEBUG% mkdir %_OTMMARKUP_DLL_NODEBUG%
if not exist %_OTMMARKUP_DLL_DEBUG% mkdir %_OTMMARKUP_DLL_DEBUG%
if not exist %_OTMMARKUP_OBJ_NODEBUG% mkdir %_OTMMARKUP_OBJ_NODEBUG%
if not exist %_OTMMARKUP_OBJ_DEBUG% mkdir %_OTMMARKUP_OBJ_DEBUG%
if not exist %_OTMMARKUP_MAP% mkdir %_OTMMARKUP_MAP%
if not exist %_OTMMARKUP_RELEASE_DIR% mkdir %_OTMMARKUP_RELEASE_DIR%
if not exist %_OTMMARKUP_RELEASE_DIR%\BIN mkdir %_OTMMARKUP_RELEASE_DIR%\BIN
if not exist %_OTMMARKUP_RELEASE_DIR%\TABLE mkdir %_OTMMARKUP_RELEASE_DIR%\TABLE
