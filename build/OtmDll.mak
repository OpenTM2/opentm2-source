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

build:    $(_DLL)\OTMDLL.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\CXMLWriter.OBJ:	$(_SRC)\tools\CXMLWriter.CPP
$(_OBJ)\EQFTAML.OBJ:	$(_SRC)\core\analysis\EQFTAML.CPP
$(_OBJ)\EQFRPXML.OBJ:	$(_SRC)\core\counting\EQFRPXML.CPP

# source\core\memory

$(_OBJ)\EQFMECLM.OBJ:	$(_SRC)\core\memory\EQFMECLM.CPP
$(_OBJ)\EqfFilterNoMatchFile.OBJ:	$(_SRC)\core\memory\EqfFilterNoMatchFile.CPP
$(_OBJ)\EQFMEM00.OBJ:	$(_SRC)\core\memory\EQFMEM00.CPP
$(_OBJ)\EQFMEMCD.OBJ:	$(_SRC)\core\memory\EQFMEMCD.CPP
$(_OBJ)\EQFMEMED.OBJ:	$(_SRC)\core\memory\EQFMEMED.CPP
$(_OBJ)\EQFMEMEP.OBJ:	$(_SRC)\core\memory\EQFMEMEP.CPP
$(_OBJ)\EQFMEMIP.OBJ:	$(_SRC)\core\memory\EQFMEMIP.CPP
$(_OBJ)\EQFMEMLD.OBJ:	$(_SRC)\core\memory\EQFMEMLD.CPP
$(_OBJ)\EQFMEMLP.OBJ:	$(_SRC)\core\memory\EQFMEMLP.CPP
$(_OBJ)\EQFMEMMD.OBJ:	$(_SRC)\core\memory\EQFMEMMD.CPP
$(_OBJ)\EQFMEMMP.OBJ:	$(_SRC)\core\memory\EQFMEMMP.CPP
$(_OBJ)\EQFMEMRP.OBJ:	$(_SRC)\core\memory\EQFMEMRP.CPP
$(_OBJ)\EQFMEMAddMatchSegID :	$(_SRC)\core\memory\EQFMEMAddMatchSegID.CPP
$(_OBJ)\EQFTMM.OBJ:		$(_SRC)\core\memory\EQFTMM.CPP
$(_OBJ)\EQFTMMV.OBJ:	$(_SRC)\core\memory\EQFTMMV.CPP
$(_OBJ)\EQFTMFUN.OBJ:		$(_SRC)\core\memory\EQFTMFUN.CPP
$(_OBJ)\GenericTagReplace.C:	$(_SRC)\core\memory\GenericTagReplace.C
$(_OBJ)\TMPluginWrapper.OBJ:	$(_SRC)\core\memory\TMPluginWrapper.cpp
$(_OBJ)\MemoryFactory.OBJ:	$(_SRC)\core\memory\MemoryFactory.cpp

# source\core\spell

$(_OBJ)\SpellFactory.OBJ:	$(_SRC)\core\spell\SpellFactory.cpp

# source\core\morph

$(_OBJ)\MorphFactory.OBJ:	$(_SRC)\core\morph\MorphFactory.cpp

# source\core\utilities

$(_OBJ)\EQFUTMDI.OBJ:	$(_SRC)\core\utilities\EQFUTMDI.C
$(_OBJ)\EQFUTDLG.OBJ:	$(_SRC)\core\utilities\EQFUTDLG.CPP
$(_OBJ)\EQFPROPS.OBJ:	$(_SRC)\core\utilities\EQFPROPS.CPP
$(_OBJ)\EQFAPROF.OBJ:	$(_SRC)\core\utilities\EQFAPROF.C
$(_OBJ)\eqfutclb.OBJ:	$(_SRC)\core\utilities\EQFUTCLB.C
$(_OBJ)\EQFNOISE.OBJ:	$(_SRC)\core\utilities\EQFNOISE.C
$(_OBJ)\EQFPROGR.OBJ:	$(_SRC)\core\utilities\EQFPROGR.C
$(_OBJ)\EQFSETUP.OBJ:	$(_SRC)\core\utilities\EQFSETUP.C
$(_OBJ)\UtlDocInfo.OBJ:	$(_SRC)\core\utilities\UtlDocInfo.c
$(_OBJ)\EQFTADIT.OBJ:	$(_SRC)\core\utilities\EQFTADIT.cpp
$(_OBJ)\EQFSEGEXPORT.OBJ:	$(_SRC)\core\utilities\EQFSEGEXPORT.C

# source\api

#$(_OBJ)\eqfx_api.obj:	$(_SRC)\api\eqfx_api.c
#$(_OBJ)\eqfx1api.obj:	$(_SRC)\api\eqfx1api.c

# source\core\editor

$(_OBJ)\EQFBCLIP.OBJ:	$(_SRC)\core\editor\EQFBCLIP.C
$(_OBJ)\EQFBFUNC.OBJ:	$(_SRC)\core\editor\EQFBFUNC.C
$(_OBJ)\EQFBFUZZ.OBJ:	$(_SRC)\core\editor\EQFBFUZZ.C
$(_OBJ)\EQFBCFND.OBJ:	$(_SRC)\core\editor\EQFBCFND.CPP
$(_OBJ)\EQFBDLG.OBJ:	$(_SRC)\core\editor\EQFBDLG.C
$(_OBJ)\EQFBDLGF.OBJ:	$(_SRC)\core\editor\EQFBDLGF.C
$(_OBJ)\EQFBDLGP.OBJ:	$(_SRC)\core\editor\EQFBDLGP.C
$(_OBJ)\EQFBDLGS.OBJ:	$(_SRC)\core\editor\EQFBDLGS.C
$(_OBJ)\EQFBBIDI.OBJ:	$(_SRC)\core\editor\EQFBBIDI.C
$(_OBJ)\EQFBINIT.OBJ:	$(_SRC)\core\editor\EQFBINIT.C
$(_OBJ)\EQFBKEYB.OBJ:	$(_SRC)\core\editor\EQFBKEYB.C
$(_OBJ)\EQFBMAIN.OBJ:	$(_SRC)\core\editor\EQFBMAIN.C
$(_OBJ)\EQFBMARK.OBJ:	$(_SRC)\core\editor\EQFBMARK.C
$(_OBJ)\EQFBPM.OBJ:		$(_SRC)\core\editor\EQFBPM.C
$(_OBJ)\EQFBSCRN.OBJ:	$(_SRC)\core\editor\EQFBSCRN.C
$(_OBJ)\EQFBTRAN.OBJ:	$(_SRC)\core\editor\EQFBTRAN.C
$(_OBJ)\EQFBTRUT.OBJ:	$(_SRC)\core\editor\EQFBTRUT.C
$(_OBJ)\EQFBUTL.OBJ:	$(_SRC)\core\editor\EQFBUTL.CPP
$(_OBJ)\EQFBWORK.OBJ:	$(_SRC)\core\editor\EQFBWORK.C
$(_OBJ)\EQFBWPRC.OBJ:	$(_SRC)\core\editor\EQFBWPRC.C
$(_OBJ)\EQFBIDI.OBJ:	$(_SRC)\core\editor\EQFBIDI.C
$(_OBJ)\EQFBRTFF.OBJ:	$(_SRC)\core\editor\EQFBRTFF.C
$(_OBJ)\EQFBRTF.OBJ:	$(_SRC)\core\editor\EQFBRTF.C
$(_OBJ)\EQFSEGMD.OBJ:	$(_SRC)\core\editor\EQFSEGMD.C
$(_OBJ)\DocumentPluginMapper.OBJ:	$(_SRC)\core\document\DocumentPluginMapper.cpp
$(_OBJ)\ReImportDoc.OBJ:	$(_SRC)\core\editor\ReImportDoc.CPP

# source\core\dictionary

$(_OBJ)\EQFDORG.OBJ:	$(_SRC)\core\dictionary\EQFDORG.C
$(_OBJ)\EQFFILTC.OBJ:	$(_SRC)\core\dictionary\EQFFILTC.C
$(_OBJ)\EQFFILTD.OBJ:	$(_SRC)\core\dictionary\EQFFILTD.C
$(_OBJ)\EQFFILTH.OBJ:	$(_SRC)\core\dictionary\EQFFILTH.C
$(_OBJ)\EQFFILTU.OBJ:	$(_SRC)\core\dictionary\EQFFILTU.C
$(_OBJ)\EQFFILTW.OBJ:	$(_SRC)\core\dictionary\EQFFILTW.C
$(_OBJ)\EQFDDISP.OBJ:	$(_SRC)\core\dictionary\EQFDDISP.C
$(_OBJ)\EQFDEDIT.OBJ:	$(_SRC)\core\dictionary\EQFDEDIT.C
$(_OBJ)\EQFDEX.OBJ:		$(_SRC)\core\dictionary\EQFDEX.C
$(_OBJ)\EQFDIC00.OBJ:	$(_SRC)\core\dictionary\EQFDIC00.C
$(_OBJ)\EQFDIC01.OBJ:	$(_SRC)\core\dictionary\EQFDIC01.C
$(_OBJ)\EQFDIC02.OBJ:	$(_SRC)\core\dictionary\EQFDIC02.C
$(_OBJ)\EQFDIC03.OBJ:	$(_SRC)\core\dictionary\EQFDIC03.C
$(_OBJ)\EQFDIMP.OBJ:	$(_SRC)\core\dictionary\EQFDIMP.C
$(_OBJ)\EQFDLOOK.OBJ:	$(_SRC)\core\dictionary\EQFDLOOK.C
$(_OBJ)\EQFDLUP.OBJ:	$(_SRC)\core\dictionary\EQFDLUP.C
$(_OBJ)\EQFQDPR.OBJ:	$(_SRC)\core\dictionary\EQFQDPR.C
$(_OBJ)\EQFQDPRA.OBJ:	$(_SRC)\core\dictionary\EQFQDPRA.C
$(_OBJ)\EQFQDPRD.OBJ:	$(_SRC)\core\dictionary\EQFQDPRD.C
$(_OBJ)\EQFQDPRP.OBJ:	$(_SRC)\core\dictionary\EQFQDPRP.C
$(_OBJ)\EQFQDPRU.OBJ:	$(_SRC)\core\dictionary\EQFQDPRU.C
$(_OBJ)\EQFRDICS.OBJ:	$(_SRC)\core\dictionary\EQFRDICS.C
$(_OBJ)\EQFSDICS.OBJ:	$(_SRC)\core\dictionary\EQFSDICS.C
$(_OBJ)\EQFQLDB.OBJ:	$(_SRC)\core\dictionary\EQFQLDB.C
$(_OBJ)\EQFQLDBI.OBJ:	$(_SRC)\core\dictionary\EQFQLDBI.C
$(_OBJ)\EQFDICRC.OBJ:	$(_SRC)\core\dictionary\EQFDICRC.C
$(_OBJ)\DictPluginWrapper.OBJ:	$(_SRC)\core\dictionary\DictPluginWrapper.cpp

# source\core\analysis

$(_OBJ)\EQFANA00.OBJ:	$(_SRC)\core\analysis\EQFANA00.CPP
$(_OBJ)\EQFTAARC.OBJ:	$(_SRC)\core\analysis\EQFTAARC.CPP
$(_OBJ)\EQFTAFUN.OBJ:	$(_SRC)\core\analysis\EQFTAFUN.C
$(_OBJ)\EQFTAG00.OBJ:	$(_SRC)\core\analysis\EQFTAG00.C
$(_OBJ)\EQFTALP0.OBJ:	$(_SRC)\core\analysis\EQFTALP0.C
$(_OBJ)\EQFTALP1.OBJ:	$(_SRC)\core\analysis\EQFTALP1.C
$(_OBJ)\EQFTALP2.OBJ:	$(_SRC)\core\analysis\EQFTALP2.C
$(_OBJ)\EQFTAPH2.OBJ:	$(_SRC)\core\analysis\EQFTAPH2.CPP
$(_OBJ)\EQFTATAG.OBJ:	$(_SRC)\core\analysis\EQFTATAG.C
$(_OBJ)\EQFPRSNO.OBJ:	$(_SRC)\core\analysis\EQFPRSNO.CPP
$(_OBJ)\EQFTSEGM.OBJ:	$(_SRC)\core\analysis\EQFTSEGM.CPP
$(_OBJ)\EQFIANA1.OBJ:	$(_SRC)\core\analysis\EQFIANA1.C
$(_OBJ)\EQFENTITY.OBJ:	$(_SRC)\core\analysis\EQFENTITY.C

# source\core\counting

$(_OBJ)\EQFCNT00.OBJ:	$(_SRC)\core\counting\EQFCNT00.C
$(_OBJ)\EQFCNT01.OBJ:	$(_SRC)\core\counting\EQFCNT01.C
$(_OBJ)\EQFRPT.OBJ:		$(_SRC)\core\counting\EQFRPT.C
$(_OBJ)\EQFRPT00.OBJ:	$(_SRC)\core\counting\EQFRPT00.C
$(_OBJ)\EQFWCNT.OBJ: 	$(_SRC)\core\counting\EQFWCNT.CPP

# source\core\folder

$(_OBJ)\EQFFOL00.OBJ:	$(_SRC)\core\folder\EQFFOL00.C
$(_OBJ)\EQFFOL01.OBJ:	$(_SRC)\core\folder\EQFFOL01.C
$(_OBJ)\EQFFOL02.OBJ:	$(_SRC)\core\folder\EQFFOL02.CPP
$(_OBJ)\EQFFOL03.OBJ:	$(_SRC)\core\folder\EQFFOL03.CPP
$(_OBJ)\EQFFOL04.OBJ:	$(_SRC)\core\folder\EQFFOL04.CPP $(_SRC)\core\folder\EQFGFR.H 
$(_OBJ)\EQFGFRBATCH.OBJ: $(_SRC)\core\folder\EQFGFRBATCH.CPP $(_SRC)\core\folder\EQFGFR.H
$(_OBJ)\EQFGFRPAINT.OBJ: $(_SRC)\core\folder\EQFGFRPAINT.CPP $(_SRC)\core\folder\EQFGFR.H
$(_OBJ)\EQFFOL05.OBJ:	$(_SRC)\core\folder\EQFFOL05.C
$(_OBJ)\EQFFOL06.OBJ:	$(_SRC)\core\folder\EQFFOL06.CPP
$(_OBJ)\EQFFSRCH.OBJ:	$(_SRC)\core\folder\EQFFSRCH.CPP

# source\core\workbench

$(_OBJ)\EQFFLL00.OBJ:	$(_SRC)\core\workbench\EQFFLL00.CPP
$(_OBJ)\EQFPLGMG.OBJ:	$(_SRC)\core\workbench\EQFPLGMG.CPP
$(_OBJ)\EQFDRVEX.OBJ:	$(_SRC)\core\workbench\EQFDRVEX.CPP
$(_OBJ)\EQFCOPYR.OBJ:	$(_SRC)\core\workbench\EQFCOPYR.C
$(_OBJ)\EQFLOGO.OBJ:	$(_SRC)\core\workbench\EQFLOGO.C
$(_OBJ)\EQFFOLLI.OBJ:	$(_SRC)\core\workbench\EQFFOLLI.C
$(_OBJ)\EQFHNDLR.OBJ:	$(_SRC)\core\workbench\EQFHNDLR.C
$(_OBJ)\EQFLNG00.OBJ:	$(_SRC)\core\workbench\EQFLNG00.C
$(_OBJ)\EQFMAIN.OBJ:	$(_SRC)\core\workbench\EQFMAIN.C

# source\core\services

$(_OBJ)\EQF_DA.OBJ:     $(_SRC)\core\services\EQF_DA.C
$(_OBJ)\EQF_MT.OBJ:	$(_SRC)\core\services\EQF_MT.C
$(_OBJ)\EQF_TM.OBJ:	$(_SRC)\core\services\EQF_TM.CPP
$(_OBJ)\EQF_TWBS.OBJ:	$(_SRC)\core\services\EQF_TWBS.C
$(_OBJ)\EQFXDOC.OBJ:	$(_SRC)\core\services\EQFXDOC.C
$(_OBJ)\EQFDDE00.OBJ:	$(_SRC)\core\services\EQFDDE00.C
$(_OBJ)\EQFDDED.OBJ:	$(_SRC)\core\services\EQFDDED.C

# source\core\tagtable

$(_OBJ)\EQFTEXP.OBJ:	$(_SRC)\core\tagtable\EQFTEXP.C
$(_OBJ)\EQFTIMP.OBJ:	$(_SRC)\core\tagtable\EQFTIMP.C
$(_OBJ)\EQFPAPI.OBJ:	$(_SRC)\core\tagtable\EQFPAPI.C
$(_OBJ)\EQFPARSE.OBJ:	$(_SRC)\core\tagtable\EQFPARSE.C
$(_OBJ)\EQFTOKEN.OBJ:	$(_SRC)\core\tagtable\EQFTOKEN.C
$(_OBJ)\MarkupPluginMapper.OBJ:	$(_SRC)\core\tagtable\MarkupPluginMapper.CPP

# source\core\lists

$(_OBJ)\EQFLIST.OBJ:	$(_SRC)\core\lists\EQFLIST.C
$(_OBJ)\EQFLSTIE.OBJ:	$(_SRC)\core\lists\EQFLSTIE.C
$(_OBJ)\EQFLSTLP.OBJ:	$(_SRC)\core\lists\EQFLSTLP.C
$(_OBJ)\EQFLSTUT.OBJ:	$(_SRC)\core\lists\EQFLSTUT.C

# source\core\linguistic

$(_OBJ)\EQFMORPH.OBJ:	$(_SRC)\core\linguistic\EQFMORPH.CPP
$(_OBJ)\EQFMORPW.OBJ:	$(_SRC)\core\linguistic\EQFMORPW.C

# Add for R012027
$(_OBJ)\SpecialCharDlg.OBJ:	$(_SRC)\core\editor\SpecialCharDlg.cpp
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /D UNICODE /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\editor\SpecialCharDlg.cpp
# Add end

# source\core\mt

$(_OBJ)\EQFMT00.OBJ:	$(_SRC)\core\mt\EQFMT00.C
$(_OBJ)\EQFMT01.OBJ:	$(_SRC)\core\mt\EQFMT01.C


$(_DLL)\OTMDLL.DLL:		$(_OBJ)\EQFCOPYR.OBJ \
						$(_OBJ)\EQFLOGO.OBJ  \
						$(_OBJ)\EQFPLGMG.OBJ \
						$(_OBJ)\EQFDRVEX.OBJ \
						$(_OBJ)\EQFANA00.OBJ \
						$(_OBJ)\EQFBCLIP.OBJ \
						$(_OBJ)\EQFBDLG.OBJ  \
						$(_OBJ)\EQFBDLGF.OBJ \
						$(_OBJ)\EQFBDLGP.OBJ \
						$(_OBJ)\EQFBDLGS.OBJ \
						$(_OBJ)\DocumentPluginMapper.OBJ \
						$(_OBJ)\EQFBFIND.OBJ \
						$(_OBJ)\EQFBCFND.OBJ \
						$(_OBJ)\EQFBFUNC.OBJ \
						$(_OBJ)\EQFBFUZZ.OBJ \
						$(_OBJ)\EQFBINIT.OBJ \
						$(_OBJ)\EQFBKEYB.OBJ \
						$(_OBJ)\EQFBMAIN.OBJ \
						$(_OBJ)\EQFBMARK.OBJ \
						$(_OBJ)\EQFBPM.OBJ   \
						$(_OBJ)\EQFBSCRN.OBJ \
						$(_OBJ)\EQFBTRAN.OBJ \
						$(_OBJ)\EQFBTRUT.OBJ \
						$(_OBJ)\EQFBUTL.OBJ  \
						$(_OBJ)\EQFBWORK.OBJ \
            $(_OBJ)\ReImportDoc.OBJ \
						$(_OBJ)\EQFBWPRC.OBJ \
						$(_OBJ)\EQFCNT00.OBJ \
						$(_OBJ)\EQFCNT01.OBJ \
						$(_OBJ)\EQFDDE00.OBJ \
						$(_OBJ)\EQFDDED.OBJ  \
						$(_OBJ)\EQFDDISP.OBJ \
						$(_OBJ)\EQFDEDIT.OBJ \
						$(_OBJ)\EQFDEX.OBJ   \
						$(_OBJ)\EQFDIC00.OBJ \
						$(_OBJ)\EQFDIC01.OBJ \
						$(_OBJ)\EQFDIC02.OBJ \
						$(_OBJ)\EQFDIC03.OBJ \
						$(_OBJ)\EQFDIMP.OBJ  \
						$(_OBJ)\EQFDLOOK.OBJ \
						$(_OBJ)\EQFDLUP.OBJ  \
						$(_OBJ)\EQFDOC00.OBJ \
						$(_OBJ)\EQFDOC01.OBJ \
						$(_OBJ)\EQFDOC02.OBJ \
						$(_OBJ)\EQFDORG.OBJ  \
						$(_OBJ)\EQFFILTC.OBJ \
						$(_OBJ)\EQFFILTD.OBJ \
						$(_OBJ)\EQFFILTH.OBJ \
						$(_OBJ)\EQFFILTU.OBJ \
						$(_OBJ)\EQFFILTW.OBJ \
						$(_OBJ)\EQFFLL00.OBJ \
						$(_OBJ)\EQFFOL00.OBJ \
						$(_OBJ)\EQFFOL01.OBJ \
						$(_OBJ)\EQFFOL02.OBJ \
						$(_OBJ)\EQFFOL03.OBJ \
						$(_OBJ)\EQFFOL04.OBJ \
						$(_OBJ)\EQFGFRBATCH.OBJ \
						$(_OBJ)\EQFGFRPAINT.OBJ \
						$(_OBJ)\EQFFOL05.OBJ \
						$(_OBJ)\EQFFOL06.OBJ \
						$(_OBJ)\EQFFOLLI.OBJ \
						$(_OBJ)\EQFHNDLR.OBJ \
						$(_OBJ)\EQFIANA1.OBJ \
						$(_OBJ)\EQFLIST.OBJ  \
						$(_OBJ)\EQFLNG00.OBJ \
						$(_OBJ)\EQFLSTIE.OBJ \
						$(_OBJ)\EQFLSTLP.OBJ \
						$(_OBJ)\EQFLSTUT.OBJ \
						$(_OBJ)\EQFPRSNO.OBJ \
						$(_OBJ)\EQFMORPH.OBJ \
						$(_OBJ)\EQFMORPW.OBJ \
						$(_OBJ)\EQFPARSE.OBJ \
						$(_OBJ)\EQFQDPR.OBJ  \
						$(_OBJ)\EQFQDPRA.OBJ \
						$(_OBJ)\EQFQDPRD.OBJ \
						$(_OBJ)\EQFQDPRP.OBJ \
						$(_OBJ)\EQFQDPRU.OBJ \
						$(_OBJ)\EQFRPT.OBJ   \
						$(_OBJ)\EQFRPT00.OBJ \
						$(_OBJ)\EQFRDICS.OBJ \
						$(_OBJ)\EQFSDICS.OBJ \
						$(_OBJ)\EQFQLDB.OBJ \
						$(_OBJ)\EQFQLDBI.OBJ \
						$(_OBJ)\EQFDICRC.OBJ \
						$(_OBJ)\DictPluginWrapper.OBJ \
                                                $(_OBJ)\EqfFilterNoMatchFile.OBJ \
						$(_OBJ)\EQFTAARC.OBJ \
						$(_OBJ)\EQFTAFUN.OBJ \
						$(_OBJ)\EQFTAG00.OBJ \
						$(_OBJ)\EQFTALP0.OBJ \
						$(_OBJ)\EQFTALP1.OBJ \
						$(_OBJ)\EQFTALP2.OBJ \
						$(_OBJ)\EQFTAPH2.OBJ \
						$(_OBJ)\EQFTATAG.OBJ \
						$(_OBJ)\EQFTEXP.OBJ  \
						$(_OBJ)\EQFTIMP.OBJ  \
						$(_OBJ)\EQFTOKEN.OBJ \
						$(_OBJ)\EQFTSEGM.OBJ \
						$(_OBJ)\EQFWCNT.OBJ  \
						$(_OBJ)\EQFXDOC.OBJ  \
						$(_OBJ)\EQF_DA.OBJ   \
						$(_OBJ)\EQF_MT.OBJ   \
						$(_OBJ)\EQF_TM.OBJ   \
						$(_OBJ)\EQF_TWBS.OBJ \
						$(_OBJ)\EQFMAIN.OBJ  \
						$(_OBJ)\EQFMT00.OBJ  \
						$(_OBJ)\EQFMT01.OBJ  \
						$(_OBJ)\EQFBIDI.OBJ  \
						$(_OBJ)\EQFBRTFF.OBJ \
						$(_OBJ)\EQFBRTF.OBJ  \
						$(_OBJ)\EQFPAPI.OBJ  \
						$(_OBJ)\EQFSEGMD.OBJ \
						$(_OBJ)\EQFENTITY.OBJ \
						$(_OBJ)\EQFFSRCH.OBJ \
						$(_OBJ)\EQFUTMDI.OBJ \
						$(_OBJ)\EQFUTDLG.OBJ \
						$(_OBJ)\EQFPROPS.OBJ \
						$(_OBJ)\EQFAPROF.OBJ \
						$(_OBJ)\eqfutclb.OBJ \
						$(_OBJ)\EQFNOISE.OBJ \
						$(_OBJ)\EQFPROGR.OBJ \
						$(_OBJ)\EQFSETUP.OBJ \
						$(_OBJ)\UtlDocInfo.OBJ \
						$(_OBJ)\TMPluginWrapper.OBJ \
						$(_OBJ)\MemoryFactory.OBJ \
						$(_OBJ)\SpellFactory.OBJ \
						$(_OBJ)\MorphFactory.OBJ \
						$(_OBJ)\EQFTADIT.OBJ \
						$(_OBJ)\CXMLWriter.OBJ \
						$(_OBJ)\EQFTAML.OBJ \
						$(_OBJ)\EQFRPXML.OBJ \
						$(_OBJ)\GenericTagReplace.OBJ \
						$(_OBJ)\EQFMECLM.OBJ \
						$(_OBJ)\EQFMEM00.OBJ \
						$(_OBJ)\EQFMEMCD.OBJ \
						$(_OBJ)\EQFMEMED.OBJ \
						$(_OBJ)\EQFMEMEP.OBJ \
						$(_OBJ)\EQFMEMIP.OBJ \
						$(_OBJ)\EQFMEMLD.OBJ \
						$(_OBJ)\EQFMEMLP.OBJ \
						$(_OBJ)\EQFMEMMD.OBJ \
						$(_OBJ)\EQFMEMMP.OBJ \
						$(_OBJ)\EQFMEMAddMatchSegID.OBJ \
						$(_OBJ)\EQFMEMRP.OBJ \
						$(_OBJ)\EQFTMM.OBJ \
						$(_OBJ)\EQFTMFUN.OBJ \
						$(_OBJ)\EQFTMMV.OBJ \
						$(_OBJ)\EQFSEGEXPORT.OBJ \
						$(_OBJ)\MarkupPluginMapper.OBJ \
						$(_OBJ)\SpecialCharDlg.OBJ 
#						$(_OBJ)\eqfx_api.obj \
#						$(_OBJ)\eqfx1api.obj \

#------------------------------------------------------------------------------
# Build OTMDLL.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMDLL.DLL:
    @echo ---- Linking $(_DLL)\OTMDLL.DLL
    @echo ---- Linking $(_DLL)\OTMDLL.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFCOPYR.OBJ
						$(_OBJ)\EQFLOGO.OBJ
						$(_OBJ)\EQFPLGMG.OBJ
						$(_OBJ)\EQFDRVEX.OBJ
						$(_OBJ)\EQFANA00.OBJ
						$(_OBJ)\EQFBCLIP.OBJ
						$(_OBJ)\EQFBDLG.OBJ
						$(_OBJ)\EQFBDLGF.OBJ
						$(_OBJ)\EQFBDLGP.OBJ
						$(_OBJ)\EQFBDLGS.OBJ
						$(_OBJ)\DocumentPluginMapper.OBJ
						$(_OBJ)\EQFBFIND.OBJ
						$(_OBJ)\EQFBCFND.OBJ
						$(_OBJ)\EQFBFUNC.OBJ
						$(_OBJ)\EQFBFUZZ.OBJ
						$(_OBJ)\EQFBINIT.OBJ
						$(_OBJ)\EQFBKEYB.OBJ
						$(_OBJ)\EQFBMAIN.OBJ
						$(_OBJ)\EQFBMARK.OBJ
						$(_OBJ)\EQFBPM.OBJ
						$(_OBJ)\EQFBSCRN.OBJ
						$(_OBJ)\EQFBTRAN.OBJ
						$(_OBJ)\EQFBTRUT.OBJ
						$(_OBJ)\EQFBUTL.OBJ
						$(_OBJ)\EQFBWORK.OBJ
            $(_OBJ)\ReImportDoc.OBJ
						$(_OBJ)\EQFBWPRC.OBJ
						$(_OBJ)\EQFCNT00.OBJ
						$(_OBJ)\EQFCNT01.OBJ
						$(_OBJ)\EQFDDE00.OBJ
						$(_OBJ)\EQFDDED.OBJ
						$(_OBJ)\EQFDDISP.OBJ
						$(_OBJ)\EQFDEDIT.OBJ
						$(_OBJ)\EQFDEX.OBJ
						$(_OBJ)\EQFDIC00.OBJ
						$(_OBJ)\EQFDIC01.OBJ
						$(_OBJ)\EQFDIC02.OBJ
						$(_OBJ)\EQFDIC03.OBJ
						$(_OBJ)\EQFDIMP.OBJ
						$(_OBJ)\EQFDLOOK.OBJ
						$(_OBJ)\EQFDLUP.OBJ
						$(_OBJ)\EQFDOC00.OBJ
						$(_OBJ)\EQFDOC01.OBJ
						$(_OBJ)\EQFDOC02.OBJ
						$(_OBJ)\EQFDORG.OBJ
						$(_OBJ)\EQFFILTC.OBJ
						$(_OBJ)\EQFFILTD.OBJ
						$(_OBJ)\EQFFILTH.OBJ
						$(_OBJ)\EQFFILTU.OBJ
						$(_OBJ)\EQFFILTW.OBJ
						$(_OBJ)\EQFFLL00.OBJ
						$(_OBJ)\EQFFOL00.OBJ
						$(_OBJ)\EQFFOL01.OBJ
						$(_OBJ)\EQFFOL02.OBJ
						$(_OBJ)\EQFFOL03.OBJ
						$(_OBJ)\EQFFOL04.OBJ
						$(_OBJ)\EQFGFRBATCH.OBJ
						$(_OBJ)\EQFGFRPAINT.OBJ
						$(_OBJ)\EQFFOL05.OBJ
						$(_OBJ)\EQFFOL06.OBJ
						$(_OBJ)\EQFFOLLI.OBJ
						$(_OBJ)\EQFHNDLR.OBJ
						$(_OBJ)\EQFIANA1.OBJ
						$(_OBJ)\EQFLIST.OBJ
						$(_OBJ)\EQFLNG00.OBJ
						$(_OBJ)\EQFLSTIE.OBJ
						$(_OBJ)\EQFLSTLP.OBJ
						$(_OBJ)\EQFLSTUT.OBJ
						$(_OBJ)\EQFPRSNO.OBJ
						$(_OBJ)\EQFMORPH.OBJ
						$(_OBJ)\EQFMORPW.OBJ
						$(_OBJ)\EQFPARSE.OBJ
						$(_OBJ)\EQFQDPR.OBJ
						$(_OBJ)\EQFQDPRA.OBJ
						$(_OBJ)\EQFQDPRD.OBJ
						$(_OBJ)\EQFQDPRP.OBJ
						$(_OBJ)\EQFQDPRU.OBJ
						$(_OBJ)\EQFRPT.OBJ
						$(_OBJ)\EQFRPT00.OBJ
						$(_OBJ)\EQFRDICS.OBJ
						$(_OBJ)\EQFSDICS.OBJ
						$(_OBJ)\EQFQLDB.OBJ
						$(_OBJ)\EQFQLDBI.OBJ
						$(_OBJ)\EQFDICRC.OBJ
						$(_OBJ)\DictPluginWrapper.OBJ
						$(_OBJ)\EQFTADIT.OBJ 
						$(_OBJ)\EQFTAARC.OBJ
						$(_OBJ)\EQFTAFUN.OBJ
						$(_OBJ)\EQFTAG00.OBJ
						$(_OBJ)\EQFTALP0.OBJ
						$(_OBJ)\EQFTALP1.OBJ
						$(_OBJ)\EQFTALP2.OBJ
						$(_OBJ)\EQFTAPH2.OBJ
						$(_OBJ)\EQFTATAG.OBJ
						$(_OBJ)\EQFTEXP.OBJ
						$(_OBJ)\EQFTIMP.OBJ
						$(_OBJ)\EQFTOKEN.OBJ
						$(_OBJ)\EQFTSEGM.OBJ
						$(_OBJ)\EQFWCNT.OBJ
						$(_OBJ)\EQFXDOC.OBJ
						$(_OBJ)\EQF_DA.OBJ
						$(_OBJ)\EQF_MT.OBJ
						$(_OBJ)\EQF_TM.OBJ
						$(_OBJ)\EQF_TWBS.OBJ
						$(_OBJ)\EQFMAIN.OBJ
						$(_OBJ)\EQFMT00.OBJ
						$(_OBJ)\EQFMT01.OBJ
						$(_OBJ)\EQFBIDI.OBJ
						$(_OBJ)\EQFBRTFF.OBJ
						$(_OBJ)\EQFBRTF.OBJ
						$(_OBJ)\EQFPAPI.OBJ
						$(_OBJ)\EQFSEGMD.OBJ
						$(_OBJ)\EQFENTITY.OBJ
						$(_OBJ)\EQFFSRCH.OBJ
						$(_OBJ)\EQFUTMDI.OBJ
						$(_OBJ)\EQFUTDLG.OBJ
						$(_OBJ)\EQFPROPS.OBJ
						$(_OBJ)\EQFAPROF.OBJ
						$(_OBJ)\eqfutclb.OBJ
						$(_OBJ)\EQFNOISE.OBJ
						$(_OBJ)\EQFPROGR.OBJ
						$(_OBJ)\EQFSETUP.OBJ
						$(_OBJ)\UtlDocInfo.OBJ
						$(_OBJ)\TMPluginWrapper.OBJ
						$(_OBJ)\MemoryFactory.OBJ
						$(_OBJ)\SpellFactory.OBJ
						$(_OBJ)\MorphFactory.OBJ
						$(_OBJ)\CXMLWriter.OBJ
						$(_OBJ)\EQFTAML.OBJ
						$(_OBJ)\EQFRPXML.OBJ
						$(_OBJ)\EQFMECLM.OBJ
						$(_OBJ)\EQFMEM00.OBJ
						$(_OBJ)\EQFMEMCD.OBJ
						$(_OBJ)\EQFMEMED.OBJ
						$(_OBJ)\EQFMEMEP.OBJ
						$(_OBJ)\EQFMEMIP.OBJ
						$(_OBJ)\EQFMEMLD.OBJ
						$(_OBJ)\EQFMEMLP.OBJ
						$(_OBJ)\EQFMEMMD.OBJ
						$(_OBJ)\EQFMEMMP.OBJ
						$(_OBJ)\EQFMEMRP.OBJ
                                                $(_OBJ)\EQFMEMAddMatchSegID.OBJ
						$(_OBJ)\EQFTMM.OBJ
						$(_OBJ)\EQFTMMV.OBJ
						$(_OBJ)\GenericTagReplace.OBJ 
						$(_OBJ)\EQFTMFUN.OBJ 
						$(_OBJ)\EQFSEGEXPORT.OBJ 
						$(_OBJ)\MarkupPluginMapper.OBJ
                                                $(_OBJ)\EqfFilterNoMatchFile.OBJ
						$(_OBJ)\SpecialCharDlg.OBJ

/OUT:$(_DLL)\OTMDLL.DLL
/MAP:$(_MAP)\OTMDLL.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmAPI.lib $(_LIB)\PluginManager.lib $(_LIB)\PluginMgrAssist.lib $(_LIB)\OTMGLOBM.lib $(_LIBOTHER)\xerces-c_3.lib
<<
