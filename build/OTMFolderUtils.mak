#------------------------------------------------------------------------------
# OTMFolderUtils.MAK - Makefile for the OTMFolderUtils DLL
# Copyright (c) 2018 International Business Machines
# Corporation and others.  All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER                                                                  -
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------

build:    $(_DLL)\OTMFolderUtils.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\FolderUtils.OBJ:	$(_SRC)\core\folder\FolderUtils.C


$(_DLL)\OTMFolderUtils.DLL: \
						$(_OBJ)\FolderUtils.OBJ 

#------------------------------------------------------------------------------
# Build OTMFolder.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMFolderUtils.DLL:
    @echo ---- Linking $(_DLL)\OTMFolderUtils.DLL
    @echo ---- Linking $(_DLL)\OTMFolderUtils.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\FolderUtils.OBJ
/OUT:$(_DLL)\OTMFolderUtils.DLL
/MAP:$(_MAP)\OTMFolderUtils.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
            $(_LINK_LIB_CRT)
            $(_LIB)\OtmBase.lib 
<<
