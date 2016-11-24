#-------------------------------------------------------------------------------
# OtmCleanupPlugin.MAK  - Makefile for cleanup tool plugin DLL
# Copyright (c) 2016, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER                                                                  -
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# Variable Defination                                                                 -
#------------------------------------------------------------------------------
CPP_COMP=/D "_UNICODE" /D "UNICODE" /Zc:wchar_t
#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------

build:    $(_OBJEXE)\S2TConv.exe

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------
$(_OBJEXE)\stdafx.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\stdafx.cpp \
							$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\stdafx.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\CellButton.obj:  $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\CellButton.cpp \
							$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\CellButton.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\CellCtrl.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\CellCtrl.cpp \
							$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\CellCtrl.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\CellPushButton.obj:   $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\CellPushButton.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\CellPushButton.h 
   @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\CellRadioButton.obj: $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\CellRadioButton.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\CellRadioButton.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\ConfigHeaderCtrl.obj: $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\ConfigHeaderCtrl.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\ConfigHeaderCtrl.h 
   @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\ConfigListCtrl.obj:  $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\ConfigListCtrl.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\ConfigListCtrl.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\listctrlcellwnd.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\listctrlcellwnd.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\listctrlcellwnd.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2T.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2T.cpp \
						$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2T.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TConv.obj:  $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\\S2TConv.cpp \
						$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TConv.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TConvDlg.obj:  $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TConvDlg.cpp \
							$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TConvDlg.h 
   @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TExpPrfDlg.obj:   $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TExpPrfDlg.cpp \
							   $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TExpPrfDlg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TNewPrfDlg.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TNewPrfDlg.cpp \
								 $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TNewPrfDlg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TEditPrjPrfDlg.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TEditPrjPrfDlg.cpp \
								    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TEditPrjPrfDlg.h 
     @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TEditTermDlg.obj:  $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TEditTermDlg.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TEditTermDlg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TEditNewTermDlg.obj:  $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TEditNewTermDlg.cpp \
								   $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TEditNewTermDlg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TTermTable.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TTermTable.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TTermTable.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TUpdateTermDlg.obj: $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TUpdateTermDlg.cpp \
								 $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TUpdateTermDlg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\StringUtil.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\StringUtil.cpp \
							  $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\StringUtil.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\Utilities.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\StringUtil.cpp \
							 $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\StringUtil.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TConv.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TConv.cpp \
						   $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TConv.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2T\$(*B).cpp

$(_OBJEXE)\S2TBuildTblDlg.obj:  $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TBuildTblDlg.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TBuildTblDlg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TNewTermDlg.obj:   $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TNewTermDlg.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TNewTermDlg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TDupTermDlg.obj:   $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TDupTermDlg.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TDupTermDlg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TImportDlg.obj:    $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TImportDlg.cpp \
								$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TImportDlg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) $(CPP_COMP) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\$(*B).cpp

$(_OBJEXE)\S2TConv.RES:	$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TConv.rc \
						$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\res\buttonEdit.bmp \
						$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\res\buttonDel.bmp \
						$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\res\buttonRadion.bmp \
						$(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\resource.h
    @$(_RC_COMPILER)  /n /Fo $(_OBJEXE)\S2TConv.RES  $(_SRC)\plugins\tools\OtmS2TPlugin\mfcbased\S2TConv.rc						
	

S2TCONVOBJS = $(_OBJEXE)\stdafx.obj \
	          $(_OBJEXE)\CellButton.obj \
	          $(_OBJEXE)\CellCtrl.obj \
	          $(_OBJEXE)\CellPushButton.obj \
	          $(_OBJEXE)\CellRadioButton.obj \
	          $(_OBJEXE)\ConfigHeaderCtrl.obj \
	          $(_OBJEXE)\ConfigListCtrl.obj \
	          $(_OBJEXE)\listctrlcellwnd.obj \
	          $(_OBJEXE)\S2T.obj \
	          $(_OBJEXE)\S2TConvDlg.obj \
	          $(_OBJEXE)\S2TExpPrfDlg.obj \
	          $(_OBJEXE)\S2TNewPrfDlg.obj \
	          $(_OBJEXE)\S2TEditPrjPrfDlg.obj \
	          $(_OBJEXE)\S2TEditTermDlg.obj \
	          $(_OBJEXE)\S2TEditNewTermDlg.obj \
	          $(_OBJEXE)\S2TTermTable.obj \
	          $(_OBJEXE)\S2TUpdateTermDlg.obj \
	          $(_OBJEXE)\StringUtil.obj \
	          $(_OBJEXE)\Utilities.obj \
	          $(_OBJEXE)\S2TConv.obj \
	          $(_OBJEXE)\S2TBuildTblDlg.obj \
	          $(_OBJEXE)\S2TNewTermDlg.obj \
	          $(_OBJEXE)\S2TDupTermDlg.obj \
	          $(_OBJEXE)\S2TImportDlg.obj \
	          $(_OBJEXE)\S2TConv.RES 

$(_OBJEXE)\S2TConv.exe:	$(S2TCONVOBJS)


#------------------------------------------------------------------------------
# Build S2TConv.exe
#------------------------------------------------------------------------------
$(_OBJEXE)\S2TConv.exe:
    @echo ---- Linking $(_OBJEXE)\S2TConv.exe
    @echo ---- Linking $(_OBJEXE)\S2TConv.exe >>$(_ERR)
    @echo ---- _LINK_OPTIONS_EXE=$(_LINK_CPP_OPTIONS)
    @echo --- _MAKEDEBUG=$(_MAKEDEBUG)
    @echo --- DEBUG_OPT=$(DEBUG_OPT)
    $(_LINKER) @<<lnk.rsp >>$(_ERR)
    $(S2TCONVOBJS) 
	$(_LINK_CPP_OPTIONS) "UxTheme.lib" /entry:wWinMainCRTStartup
/OUT:$(_OBJEXE)\S2TConv.exe
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmS2TPlugin md $(RELEASE_DIR)\OTM\Plugins\OtmS2TPlugin
    @copy $(_OBJEXE)\S2TConv.EXE $(RELEASE_DIR)\OTM\Plugins\OtmS2TPlugin /Y>$(_ERR)
	