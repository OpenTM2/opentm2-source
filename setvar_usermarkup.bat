@ECHO OFF
rem Copyright (c) 2013, International Business Machines
rem Corporation and others.  All rights reserved.

echo Prepare UserMarkupTablePlugin items.

set _USERMARKUP_SRC=%_SRC%\plugins\markup\UserMarkupTablePlugin
set _USERMARKUP_MAP=%_MAP%\plugins\markup\UserMarkupTablePlugin
set _USERMARKUP_RELEASE_DIR=%RELEASE_DIR%\otm\plugins\UserMarkupTablePlugin

set _USERMARKUP_DLL_NODEBUG=%_DLLWIN%\plugins\markup\UserMarkupTablePlugin
set _USERMARKUP_DLL_DEBUG=%_DLLWINCV%\plugins\markup\UserMarkupTablePlugin
set _USERMARKUP_OBJ_NODEBUG=%_OBJWIN%\plugins\markup\UserMarkupTablePlugin
set _USERMARKUP_OBJ_DEBUG=%_OBJWINCV%\plugins\markup\UserMarkupTablePlugin
set _USERMARKUP_CFLAGS_NODEBUG=/nologo /c /W2 /MT /EHsc /TC /I%_USERMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS
set _USERMARKUP_CFLAGS_DEBUG=/nologo /c /W2 /MT /EHsc /TC /I%_USERMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /Zi
set _USERMARKUP_CUFLAGS_NODEBUG=/nologo /c /W2 /MT /EHsc /TC /I%_USERMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /DUNICODE /D_UNICODE
set _USERMARKUP_CUFLAGS_DEBUG=/nologo /c /W2 /MT /EHsc /TC /I%_USERMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /DUNICODE /D_UNICODE /Zi
set _USERMARKUP_CPPFLAGS_NODEBUG=/nologo /c /W2 /MT /EHsc /TP /I%_USERMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /DUNICODE /D_UNICODE
set _USERMARKUP_CPPFLAGS_DEBUG=/nologo /c /W2 /MT /EHsc /TP /I%_USERMARKUP_SRC%\common /DWIN32BIT /D_WINDOWS /DUNICODE /D_UNICODE /Zi
set _USERMARKUP_LINKFLAGS_NODEBUG=/nologo /MACHINE:IX86 /ALIGN:0X1000 /DRIVER /DLL
set _USERMARKUP_LINKFLAGS_DEBUG=/nologo /MACHINE:IX86 /ALIGN:0X1000 /DRIVER /DLL /DEBUG

if not exist %_USERMARKUP_DLL_NODEBUG% mkdir %_USERMARKUP_DLL_NODEBUG%
if not exist %_USERMARKUP_DLL_DEBUG% mkdir %_USERMARKUP_DLL_DEBUG%
if not exist %_USERMARKUP_OBJ_NODEBUG% mkdir %_USERMARKUP_OBJ_NODEBUG%
if not exist %_USERMARKUP_OBJ_DEBUG% mkdir %_USERMARKUP_OBJ_DEBUG%
if not exist %_USERMARKUP_MAP% mkdir %_USERMARKUP_MAP%
if not exist %_USERMARKUP_RELEASE_DIR% mkdir %_USERMARKUP_RELEASE_DIR%
if not exist %_USERMARKUP_RELEASE_DIR%\BIN mkdir %_USERMARKUP_RELEASE_DIR%\BIN
if not exist %_USERMARKUP_RELEASE_DIR%\TABLE mkdir %_USERMARKUP_RELEASE_DIR%\TABLE
