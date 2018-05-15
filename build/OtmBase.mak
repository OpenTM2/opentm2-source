#-------------------------------------------------------------------------------
# OTMBase.mak - Makefile for OTMBase DLL
# Copyright (c) 2014, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:    $(_DLL)\OTMBase.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------
$(_OBJ)\UtlAlloc.OBJ:	$(_SRC)\core\utilities\UtlAlloc.c $(_SRC)\core\utilities\Utility.h
$(_OBJ)\UtlMATVal.OBJ:	$(_SRC)\core\utilities\UtlMATVal.c
$(_OBJ)\UtlLangCP.OBJ:	$(_SRC)\core\utilities\UtlLangCP.c
$(_OBJ)\UtlString.OBJ:	$(_SRC)\core\utilities\UtlString.c
$(_OBJ)\eqfcmpr.OBJ:	$(_SRC)\core\utilities\eqfcmpr.c
$(_OBJ)\eqfpro00.OBJ:	$(_SRC)\core\utilities\eqfpro00.c
$(_OBJ)\eqfutdos.OBJ:	$(_SRC)\core\utilities\eqfutdos.c
$(_OBJ)\eqfutlng.OBJ:	$(_SRC)\core\utilities\eqfutlng.cpp
$(_OBJ)\eqfutfil.OBJ:	$(_SRC)\core\utilities\eqfutfil.c
$(_OBJ)\eqfuterr.OBJ:	$(_SRC)\core\utilities\eqfuterr.c
$(_OBJ)\eqfutprt.OBJ:	$(_SRC)\core\utilities\eqfutprt.c
$(_OBJ)\eqfobj00.OBJ:	$(_SRC)\core\utilities\eqfobj00.c
$(_OBJ)\eqfoswin.OBJ:	$(_SRC)\core\utilities\eqfoswin.c
$(_OBJ)\eqfhash.OBJ:	$(_SRC)\core\utilities\eqfhash.c
$(_OBJ)\UtlMisc.OBJ:	$(_SRC)\core\utilities\UtlMisc.cpp
$(_OBJ)\UtlRegistry.OBJ:	$(_SRC)\core\utilities\UtlRegistry.c
$(_OBJ)\OtmProposal.OBJ:	$(_SRC)\core\utilities\OtmProposal.cpp
$(_OBJ)\LogWriter.OBJ:	$(_SRC)\core\utilities\LogWriter.cpp
$(_OBJ)\LanguageFactory.OBJ:	$(_SRC)\core\utilities\LanguageFactory.cpp
$(_OBJ)\zip.OBJ:	$(_SRC)\core\utilities\zip.cpp
$(_OBJ)\unzip.OBJ:	$(_SRC)\core\utilities\unzip.cpp
$(_OBJ)\CXMLWriter.OBJ:	$(_SRC)\tools\CXMLWriter.CPP
$(_OBJ)\UtlDocInfo.OBJ:	$(_SRC)\core\utilities\UtlDocInfo.c
$(_OBJ)\OTMSegFile.OBJ:	$(_SRC)\core\editor\OTMSegFile.c
$(_OBJ)\EQFUTMDI.OBJ:	$(_SRC)\core\utilities\EQFUTMDI.C
$(_OBJ)\OptionsDialog.OBJ:	$(_SRC)\core\utilities\OptionsDialog.CPP

$(_DLL)\OTMBase.DLL:	$(_OBJ)\UtlMATVal.OBJ \
						$(_OBJ)\UtlLangCP.OBJ \
						$(_OBJ)\UtlString.OBJ \
						$(_OBJ)\eqfcmpr.OBJ \
						$(_OBJ)\eqfpro00.OBJ \
						$(_OBJ)\eqfutdos.OBJ \
						$(_OBJ)\eqfutlng.OBJ \
						$(_OBJ)\eqfutfil.OBJ \
						$(_OBJ)\eqfuterr.OBJ \
						$(_OBJ)\eqfutprt.OBJ \
						$(_OBJ)\eqfobj00.OBJ \
						$(_OBJ)\eqfoswin.OBJ \
						$(_OBJ)\eqfhash.OBJ \
						$(_OBJ)\UtlMisc.OBJ \
						$(_OBJ)\UtlRegistry.OBJ \
            $(_OBJ)\LanguageFactory.OBJ \
						$(_OBJ)\OtmProposal.OBJ \
						$(_OBJ)\zip.OBJ \
						$(_OBJ)\unzip.OBJ \
						$(_OBJ)\LogWriter.OBJ \
            $(_OBJ)\UtlAlloc.OBJ \
            $(_OBJ)\UtlDocInfo.OBJ \
            $(_OBJ)\OTMSegFile.OBJ \
            $(_OBJ)\EQFUTMDI.OBJ \
            $(_OBJ)\OptionsDialog.OBJ \
 						$(_OBJ)\CXMLWriter.OBJ 


#------------------------------------------------------------------------------
# Build OTMBase.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMBase.DLL:
    @echo ---- Linking $(_DLL)\OTMBase.DLL
    @echo ---- Linking $(_DLL)\OTMBase.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
					$(_OBJ)\UtlMATVal.OBJ
					$(_OBJ)\UtlLangCP.OBJ
					$(_OBJ)\UtlString.OBJ
					$(_OBJ)\eqfcmpr.OBJ
					$(_OBJ)\eqfpro00.OBJ
					$(_OBJ)\eqfutdos.OBJ
					$(_OBJ)\eqfutlng.OBJ
					$(_OBJ)\eqfutfil.OBJ
					$(_OBJ)\eqfuterr.OBJ
					$(_OBJ)\eqfutprt.OBJ
					$(_OBJ)\eqfobj00.OBJ
					$(_OBJ)\eqfoswin.OBJ
					$(_OBJ)\eqfhash.OBJ
					$(_OBJ)\UtlMisc.OBJ
					$(_OBJ)\UtlRegistry.OBJ
          $(_OBJ)\LanguageFactory.OBJ
					$(_OBJ)\OtmProposal.OBJ
  				$(_OBJ)\zip.OBJ 
					$(_OBJ)\unzip.OBJ 
					$(_OBJ)\LogWriter.OBJ
          $(_OBJ)\UtlAlloc.OBJ
          $(_OBJ)\UtlDocInfo.OBJ
          $(_OBJ)\OTMSegFile.OBJ
          $(_OBJ)\EQFUTMDI.OBJ 
          $(_OBJ)\OptionsDialog.OBJ
          $(_OBJ)\CXMLWriter.OBJ
/OUT:$(_DLL)\OTMBase.DLL
/MAP:$(_MAP)\OTMBase.MAP $(_LINK_OPTIONS) /MAPINFO:EXPORTS /DLL
$(_LINK_LIB_CRT) imm32.lib $(_LIBOTHER)\xerces-c_3.lib
<<
