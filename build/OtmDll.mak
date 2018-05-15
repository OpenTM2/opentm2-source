#------------------------------------------------------------------------------
# OTMDLL.MAK - Makefile for Startup code and resource DLL
# Copyright (c) 2016,2017 International Business Machines
# Corporation and others.  All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER                                                                  -
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------

build:    $(_DLL)\OTMDLL.DLL $(_DLL)\OTMMemory.DLL $(_DLL)\OTMUtilities.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------


# source\core\lists





$(_DLL)\OTMDLL.DLL:		\


#------------------------------------------------------------------------------
# Build OTMDLL.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMDLL.DLL:
    @echo ---- Linking $(_DLL)\OTMDLL.DLL
    @echo ---- Linking $(_DLL)\OTMDLL.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFDDISP.OBJ
						$(_OBJ)\EQFDOC00.OBJ
						$(_OBJ)\EQFDOC01.OBJ
						$(_OBJ)\EQFDOC02.OBJ
						$(_OBJ)\EQFLIST.OBJ
						$(_OBJ)\EQFLSTIE.OBJ
						$(_OBJ)\EQFLSTLP.OBJ
						$(_OBJ)\EQFLSTUT.OBJ
						$(_OBJ)\EQFMORPH.OBJ
						$(_OBJ)\EQFMORPW.OBJ
						$(_OBJ)\EQFXDOC.OBJ
						$(_OBJ)\EQFMT00.OBJ
						$(_OBJ)\EQFMT01.OBJ
						$(_OBJ)\EQFPAPI.OBJ
						$(_OBJ)\EQFUTMDI.OBJ
						$(_OBJ)\EQFUTDLG.OBJ
						$(_OBJ)\EQFPROPS.OBJ
						$(_OBJ)\EQFAPROF.OBJ
						$(_OBJ)\eqfutclb.OBJ
						$(_OBJ)\EQFNOISE.OBJ
						$(_OBJ)\EQFPROGR.OBJ
						$(_OBJ)\EQFSETUP.OBJ
						$(_OBJ)\SpellFactory.OBJ
						$(_OBJ)\MorphFactory.OBJ
						$(_OBJ)\MarkupPluginMapper.OBJ
            $(_OBJ)\EqfFilterNoMatchFile.OBJ
						$(_OBJ)\SpecialCharDlg.OBJ

/OUT:$(_DLL)\OTMDLL.DLL
/MAP:$(_MAP)\OTMDLL.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmAPI.lib $(_LIB)\PluginManager.lib $(_LIB)\PluginMgrAssist.lib $(_LIB)\OTMGLOBM.lib $(_LIBOTHER)\xerces-c_3.lib
<<
