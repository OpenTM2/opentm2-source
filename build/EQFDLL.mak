# +----------------------------------------------------------------------------+
# |  EQFDLL.MAK    - Makefile for Startup code and resource DLL                |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |          Copyright (C) 1990-2012, International Business Machines          |
# |          Corporation and others. All rights reserved                       |
# |                                                                            |
# |                                                                            |
# +----------------------------------------------------------------------------+
# |  Description:                                                              |
# +----------------------------------------------------------------------------+

#------------------------------------------------------------------------------
# eqfrules  from BUILDER                                                                  -
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------


build:    $(_DLL)\EQFDLL.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

# source\core\editor

$(_OBJ)\EQFBCLIP.OBJ:       $(_SRC)\core\editor\EQFBCLIP.C

$(_OBJ)\EQFBFILE.OBJ:       $(_SRC)\core\editor\EQFBFILE.C

$(_OBJ)\EQFBFUNC.OBJ:       $(_SRC)\core\editor\EQFBFUNC.C

$(_OBJ)\EQFBFUZZ.OBJ:       $(_SRC)\core\editor\EQFBFUZZ.C

$(_OBJ)\EQFBCFND.OBJ:       $(_SRC)\core\editor\EQFBCFND.C

$(_OBJ)\EQFBDLG.OBJ:       $(_SRC)\core\editor\EQFBDLG.C

$(_OBJ)\EQFBDLGF.OBJ:       $(_SRC)\core\editor\EQFBDLGF.C

$(_OBJ)\EQFBDLGP.OBJ:       $(_SRC)\core\editor\EQFBDLGP.C

$(_OBJ)\EQFBDLGS.OBJ:       $(_SRC)\core\editor\EQFBDLGS.C

$(_OBJ)\EQFBDOC.OBJ:       $(_SRC)\core\editor\EQFBDOC.C

$(_OBJ)\EQFBBIDI.OBJ:       $(_SRC)\core\editor\EQFBBIDI.C

$(_OBJ)\EQFBINIT.OBJ:       $(_SRC)\core\editor\EQFBINIT.C

$(_OBJ)\EQFBKEYB.OBJ:       $(_SRC)\core\editor\EQFBKEYB.C

$(_OBJ)\EQFBMAIN.OBJ:       $(_SRC)\core\editor\EQFBMAIN.C

$(_OBJ)\EQFBMARK.OBJ:       $(_SRC)\core\editor\EQFBMARK.C

$(_OBJ)\EQFBPM.OBJ:       $(_SRC)\core\editor\EQFBPM.C

$(_OBJ)\EQFBSCRN.OBJ:       $(_SRC)\core\editor\EQFBSCRN.C

$(_OBJ)\EQFBTRAN.OBJ:       $(_SRC)\core\editor\EQFBTRAN.C

$(_OBJ)\EQFBTRUT.OBJ:       $(_SRC)\core\editor\EQFBTRUT.C

$(_OBJ)\EQFBUTL.OBJ:       $(_SRC)\core\editor\EQFBUTL.C

$(_OBJ)\EQFBWORK.OBJ:       $(_SRC)\core\editor\EQFBWORK.C

$(_OBJ)\EQFBWPRC.OBJ:       $(_SRC)\core\editor\EQFBWPRC.C

$(_OBJ)EQFBIDI.OBJ:              $(_SRC)\core\editor\EQFBIDI.C

$(_OBJ)EQFBRTFF.OBJ:             $(_SRC)\core\editor\EQFBRTFF.C

$(_OBJ)EQFBRTF.OBJ:              $(_SRC)\core\editor\EQFBRTF.C

$(_OBJ)EQFSEGMD.OBJ:             $(_SRC)\core\editor\EQFSEGMD.C


# source\core\dictionary

$(_OBJ)EQFDAM.OBJ:               $(_SRC)\core\dictionary\EQFDAM.C

$(_OBJ)EQFDASD.OBJ:              $(_SRC)\core\dictionary\EQFDASD.C

$(_OBJ)EQFDASDM.OBJ:             $(_SRC)\core\dictionary\EQFDASDM.C

$(_OBJ)EQFDASDT.OBJ:             $(_SRC)\core\dictionary\EQFDASDT.C

$(_OBJ)EQFDORG.OBJ:              $(_SRC)\core\dictionary\EQFDORG.C

$(_OBJ)EQFFILTC.OBJ:             $(_SRC)\core\dictionary\EQFFILTC.C

$(_OBJ)EQFFILTD.OBJ:             $(_SRC)\core\dictionary\EQFFILTD.C

$(_OBJ)EQFFILTH.OBJ:             $(_SRC)\core\dictionary\EQFFILTH.C

$(_OBJ)EQFFILTU.OBJ:             $(_SRC)\core\dictionary\EQFFILTU.C

$(_OBJ)EQFFILTW.OBJ:             $(_SRC)\core\dictionary\EQFFILTW.C

$(_OBJ)EQFDDISP.OBJ:             $(_SRC)\core\dictionary\EQFDDISP.C

$(_OBJ)EQFDEDIT.OBJ:             $(_SRC)\core\dictionary\EQFDEDIT.C

$(_OBJ)EQFDEX.OBJ:               $(_SRC)\core\dictionary\EQFDEX.C

$(_OBJ)EQFDIC00.OBJ:             $(_SRC)\core\dictionary\EQFDIC00.C

$(_OBJ)EQFDIC01.OBJ:             $(_SRC)\core\dictionary\EQFDIC01.C

$(_OBJ)EQFDIC02.OBJ:             $(_SRC)\core\dictionary\EQFDIC02.C

$(_OBJ)EQFDIC03.OBJ:             $(_SRC)\core\dictionary\EQFDIC03.C

$(_OBJ)EQFDIMP.OBJ:              $(_SRC)\core\dictionary\EQFDIMP.C

$(_OBJ)EQFDLOOK.OBJ:             $(_SRC)\core\dictionary\EQFDLOOK.C

$(_OBJ)EQFDLUP.OBJ:              $(_SRC)\core\dictionary\EQFDLUP.C

$(_OBJ)EQFQDAM.OBJ:              $(_SRC)\core\dictionary\EQFQDAM.C

$(_OBJ)EQFQDAMI.OBJ:             $(_SRC)\core\dictionary\EQFQDAMI.C

$(_OBJ)EQFQDAMU.OBJ:             $(_SRC)\core\dictionary\EQFQDAMU.C

$(_OBJ)EQFQDAMW.OBJ:             $(_SRC)\core\dictionary\EQFQDAMW.C

$(_OBJ)EQFQDPR.OBJ:              $(_SRC)\core\dictionary\EQFQDPR.C

$(_OBJ)EQFQDPRA.OBJ:             $(_SRC)\core\dictionary\EQFQDPRA.C

$(_OBJ)EQFQDPRD.OBJ:             $(_SRC)\core\dictionary\EQFQDPRD.C

$(_OBJ)EQFQDPRP.OBJ:             $(_SRC)\core\dictionary\EQFQDPRP.C

$(_OBJ)EQFQDPRU.OBJ:             $(_SRC)\core\dictionary\EQFQDPRU.C

$(_OBJ)EQFQDSRV.OBJ:             $(_SRC)\core\dictionary\EQFQDSRV.C

$(_OBJ)EQFQLDB.OBJ:              $(_SRC)\core\dictionary\EQFQLDB.C

$(_OBJ)EQFQLDBI.OBJ:             $(_SRC)\core\dictionary\EQFQLDBI.C

$(_OBJ)EQFRDICS.OBJ:             $(_SRC)\core\dictionary\EQFRDICS.C

$(_OBJ)EQFQDPRU.OBJ:             $(_SRC)\core\dictionary\EQFQDPRU.C

$(_OBJ)EQFSDICS.OBJ:             $(_SRC)\core\dictionary\EQFSDICS.C


# source\core\analysis

$(_OBJ)EQFANA00.OBJ:             $(_SRC)\core\analysis\EQFANA00.C

$(_OBJ)EQFTAARC.OBJ:             $(_SRC)\core\analysis\EQFTAARC.C

$(_OBJ)EQFTAFUN.OBJ:             $(_SRC)\core\analysis\EQFTAFUN.C

$(_OBJ)EQFTAG00.OBJ:             $(_SRC)\core\analysis\EQFTAG00.C

$(_OBJ)EQFTALP0.OBJ:             $(_SRC)\core\analysis\EQFTALP0.C

$(_OBJ)EQFTALP1.OBJ:             $(_SRC)\core\analysis\EQFTALP1.C

$(_OBJ)EQFTALP2.OBJ:             $(_SRC)\core\analysis\EQFTALP2.C

$(_OBJ)EQFTAPH2.OBJ:             $(_SRC)\core\analysis\EQFTAPH2.C

$(_OBJ)EQFTATAG.OBJ:             $(_SRC)\core\analysis\EQFTATAG.C

$(_OBJ)EQFPRSNO.OBJ:             $(_SRC)\core\analysis\EQFPRSNO.C

$(_OBJ)EQFTSEGM.OBJ:             $(_SRC)\core\analysis\EQFTSEGM.C

$(_OBJ)EQFIANA1.OBJ:             $(_SRC)\core\analysis\EQFIANA1.C

$(_OBJ)EQFENTITY.OBJ:            $(_SRC)\core\analysis\EQFENTITY.C


# source\core\utilities

$(_OBJ)EQFUTCLB.OBJ:             $(_SRC)\core\utilities\EQFUTCLB.C

$(_OBJ)EQFUTDLG.OBJ:             $(_SRC)\core\utilities\EQFUTDLG.C

$(_OBJ)EQFUTDOS.OBJ:             $(_SRC)\core\utilities\EQFUTDOS.C

$(_OBJ)EQFUTERR.OBJ:             $(_SRC)\core\utilities\EQFUTERR.C

$(_OBJ)EQFUTFIL.OBJ:             $(_SRC)\core\utilities\EQFUTFIL.C

$(_OBJ)EQFUTILS.OBJ:             $(_SRC)\core\utilities\EQFUTILS.C

$(_OBJ)EQFUTLNG.OBJ:             $(_SRC)\core\utilities\EQFUTLNG.C

$(_OBJ)EQFUTPRT.OBJ:             $(_SRC)\core\utilities\EQFUTPRT.C

$(_OBJ)EQFUTMDI.OBJ:             $(_SRC)\core\utilities\EQFUTMDI.C

$(_OBJ)EQFPROPS.OBJ:             $(_SRC)\core\utilities\EQFPROPS.C

$(_OBJ)EQFOBJ00.OBJ:             $(_SRC)\core\utilities\EQFOBJ00.C

$(_OBJ)EQFPRO00.OBJ:             $(_SRC)\core\utilities\EQFPRO00.C

$(_OBJ)EQFHASH.OBJ:              $(_SRC)\core\utilities\EQFHASH.C

$(_OBJ)EQFCMPR.OBJ:              $(_SRC)\core\utilities\EQFCMPR.C

$(_OBJ)EQFPROGR.OBJ:             $(_SRC)\core\utilities\EQFPROGR.C

$(_OBJ)EQFAPROF.OBJ:             $(_SRC)\core\utilities\EQFAPROF.C

$(_OBJ)CSTUB.OBJ:                $(_SRC)\core\utilities\CSTUB.C

$(_OBJ)EQFCSTUB.OBJ:             $(_SRC)\core\utilities\EQFCSTUB.C

$(_OBJ)EQFOSWIN.OBJ:             $(_SRC)\core\utilities\EQFOSWIN.C

$(_OBJ)EQFNOISE.OBJ:             $(_SRC)\core\utilities\EQFNOISE.C

$(_OBJ)EQFSETUP.OBJ:             $(_SRC)\core\utilities\EQFSETUP.C


# source\core\counting

$(_OBJ)EQFCNT00.OBJ:             $(_SRC)\core\counting\EQFCNT00.C

$(_OBJ)EQFCNT01.OBJ:             $(_SRC)\core\counting\EQFCNT01.C

$(_OBJ)EQFRPT.OBJ:               $(_SRC)\core\counting\EQFRPT.C

$(_OBJ)EQFRPT00.OBJ:             $(_SRC)\core\counting\EQFRPT00.C

$(_OBJ)EQFRPT01.OBJ:             $(_SRC)\core\counting\EQFRPT01.C

$(_OBJ)EQFWCNT.OBJ:              $(_SRC)\core\counting\EQFWCNT.C


# source\core\folder

$(_OBJ)EQFFOL00.OBJ:             $(_SRC)\core\folder\EQFFOL00.C

$(_OBJ)EQFFOL01.OBJ:             $(_SRC)\core\folder\EQFFOL01.C

$(_OBJ)EQFFOL02.OBJ:             $(_SRC)\core\folder\EQFFOL02.C

$(_OBJ)EQFFOL03.OBJ:             $(_SRC)\core\folder\EQFFOL03.C

$(_OBJ)EQFFOL04.OBJ:             $(_SRC)\core\folder\EQFFOL04.C

$(_OBJ)EQFFOL05.OBJ:             $(_SRC)\core\folder\EQFFOL05.C

$(_OBJ)EQFFOL06.OBJ:             $(_SRC)\core\folder\EQFFOL06.C

$(_OBJ)EQFFSRCH.OBJ:             $(_SRC)\core\folder\EQFFSRCH.C


# source\core\document

$(_OBJ)EQFDOC00.OBJ:             $(_SRC)\core\document\EQFDOC00.C

$(_OBJ)EQFDOC01.OBJ:             $(_SRC)\core\document\EQFDOC01.C

$(_OBJ)EQFDOC02.OBJ:             $(_SRC)\core\document\EQFDOC02.C



# source\core\workbench

$(_OBJ)EQFFLL00.OBJ:             $(_SRC)\core\workbench\EQFFLL00.C

$(_OBJ)EQFDRVEX.OBJ:             $(_SRC)\core\workbench\EQFDRVEX.C

$(_OBJ)EQFCOPYR.OBJ:             $(_SRC)\core\workbench\EQFCOPYR.C

$(_OBJ)EQFLOGO.OBJ:              $(_SRC)\core\workbench\EQFLOGO.C

$(_OBJ)EQFFOLLI.OBJ:             $(_SRC)\core\workbench\EQFFOLLI.C

$(_OBJ)EQFHNDLR.OBJ:             $(_SRC)\core\workbench\EQFHNDLR.C

$(_OBJ)EQFLNG00.OBJ:             $(_SRC)\core\workbench\EQFLNG00.C

$(_OBJ)EQFMAIN.OBJ:              $(_SRC)\core\workbench\EQFMAIN.C



# source\core\services

$(_OBJ)EQF_DA.OBJ:               $(_SRC)\core\services\EQF_DA.C

$(_OBJ)EQF_MT.OBJ:               $(_SRC)\core\services\EQF_MT.C

$(_OBJ)EQF_TM.OBJ:               $(_SRC)\core\services\EQF_TM.C

$(_OBJ)EQF_TWBS.OBJ:             $(_SRC)\core\services\EQF_TWBS.C

$(_OBJ)EQFXDOC.OBJ:              $(_SRC)\core\services\EQFXDOC.C

$(_OBJ)EQFDDE00.OBJ:             $(_SRC)\core\services\EQFDDE00.C

$(_OBJ)EQFDDED.OBJ:              $(_SRC)\core\services\EQFDDED.C


# source\core\tagtable

$(_OBJ)EQFTEXP.OBJ:              $(_SRC)\core\tagtable\EQFTEXP.C

$(_OBJ)EQFTIMP.OBJ:              $(_SRC)\core\tagtable\EQFTIMP.C

$(_OBJ)EQFPAPI.OBJ:              $(_SRC)\core\tagtable\EQFPAPI.C

$(_OBJ)EQFPARSE.OBJ:             $(_SRC)\core\tagtable\EQFPARSE.C

$(_OBJ)EQFTOKEN.OBJ:             $(_SRC)\core\tagtable\EQFTOKEN.C


# source\core\lists

$(_OBJ)EQFLIST.OBJ:              $(_SRC)\core\lists\EQFLIST.C

$(_OBJ)EQFLSTIE.OBJ:             $(_SRC)\core\lists\EQFLSTIE.C

$(_OBJ)EQFLSTLP.OBJ:             $(_SRC)\core\lists\EQFLSTLP.C

$(_OBJ)EQFLSTUT.OBJ:             $(_SRC)\core\lists\EQFLSTUT.C


# source\core\linguistic

$(_OBJ)EQFMORPH.OBJ:             $(_SRC)\core\linguistic\EQFMORPH.C

$(_OBJ)EQFMORPW.OBJ:             $(_SRC)\core\linguistic\EQFMORPW.C



# source\core\mt

$(_OBJ)EQFMT00.OBJ:              $(_SRC)\core\mt\EQFMT00.C

$(_OBJ)EQFMT01.OBJ:              $(_SRC)\core\mt\EQFMT01.C


$(_DLL)\EQFDLL.DLL:       $(_DEF)\EQFDLL.$(_DEFEXT) \
                          $(_OBJ)\EQFCOPYR.OBJ \
                          $(_OBJ)\EQFLOGO.OBJ  \
                          $(_OBJ)\EQFDRVEX.OBJ \
                          $(_OBJ)\CSTUB.OBJ    \
                          $(_OBJ)\EQFANA00.OBJ \
                          $(_OBJ)\EQFBCLIP.OBJ \
                          $(_OBJ)\EQFBDLG.OBJ  \
                          $(_OBJ)\EQFBDLGF.OBJ \
                          $(_OBJ)\EQFBDLGP.OBJ \
                          $(_OBJ)\EQFBDLGS.OBJ \
                          $(_OBJ)\EQFBDOC.OBJ  \
                          $(_OBJ)\EQFBFILE.OBJ \
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
                          $(_OBJ)\EQFBWPRC.OBJ \
                          $(_OBJ)\EQFCMPR.OBJ  \
                          $(_OBJ)\EQFCNT00.OBJ \
                          $(_OBJ)\EQFCNT01.OBJ \
                          $(_OBJ)\EQFCSTUB.OBJ \
                          $(_OBJ)\EQFDAM.OBJ   \
                          $(_OBJ)\EQFDASD.OBJ  \
                          $(_OBJ)\EQFDASDM.OBJ \
                          $(_OBJ)\EQFDASDT.OBJ \
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
                          $(_OBJ)\EQFPROPS.OBJ \
                          $(_OBJ)\EQFFOL00.OBJ \
                          $(_OBJ)\EQFFOL01.OBJ \
                          $(_OBJ)\EQFFOL02.OBJ \
                          $(_OBJ)\EQFFOL03.OBJ \
                          $(_OBJ)\EQFFOL04.OBJ \
                          $(_OBJ)\EQFFOL05.OBJ \
                          $(_OBJ)\EQFFOL06.OBJ \
                          $(_OBJ)\EQFFOLLI.OBJ \
                          $(_OBJ)\EQFHASH.OBJ  \
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
                          $(_OBJ)\EQFNOISE.OBJ \
                          $(_OBJ)\EQFOBJ00.OBJ \
                          $(_OBJ)\EQFPARSE.OBJ \
                          $(_OBJ)\EQFPRO00.OBJ \
                          $(_OBJ)\EQFQDAM.OBJ  \
                          $(_OBJ)\EQFQDAMI.OBJ \
                          $(_OBJ)\EQFQDAMU.OBJ \
                          $(_OBJ)\EQFQDAMW.OBJ \
                          $(_OBJ)\EQFQDPR.OBJ  \
                          $(_OBJ)\EQFQDPRA.OBJ \
                          $(_OBJ)\EQFQDPRD.OBJ \
                          $(_OBJ)\EQFQDPRP.OBJ \
                          $(_OBJ)\EQFQDPRU.OBJ \
                          $(_OBJ)\EQFQDSRV.OBJ \
                          $(_OBJ)\EQFQLDB.OBJ  \
                          $(_OBJ)\EQFQLDBI.OBJ \
                          $(_OBJ)\EQFRDICS.OBJ \
                          $(_OBJ)\EQFRPT.OBJ   \
                          $(_OBJ)\EQFRPT00.OBJ \
                          $(_OBJ)\EQFRPT01.OBJ \
                          $(_OBJ)\EQFSDICS.OBJ \
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
                          $(_OBJ)\EQFUTCLB.OBJ \
                          $(_OBJ)\EQFUTDLG.OBJ \
                          $(_OBJ)\EQFUTDOS.OBJ \
                          $(_OBJ)\EQFUTERR.OBJ \
                          $(_OBJ)\EQFUTFIL.OBJ \
                          $(_OBJ)\EQFUTILS.OBJ \
                          $(_OBJ)\EQFUTLNG.OBJ \
                          $(_OBJ)\EQFUTPRT.OBJ \
                          $(_OBJ)\EQFUTMDI.OBJ \
                          $(_OBJ)\EQFWCNT.OBJ  \
                          $(_OBJ)\EQFXDOC.OBJ  \
                          $(_OBJ)\EQF_DA.OBJ   \
                          $(_OBJ)\EQF_MT.OBJ   \
                          $(_OBJ)\EQF_TM.OBJ   \
                          $(_OBJ)\EQF_TWBS.OBJ \
                          $(_OBJ)\EQFOSWIN.OBJ \
                          $(_OBJ)\EQFQDPRU.OBJ \
                          $(_OBJ)\EQFPROGR.OBJ \
                          $(_OBJ)\EQFMAIN.OBJ  \
                          $(_OBJ)\EQFMT00.OBJ  \
                          $(_OBJ)\EQFMT01.OBJ  \
                          $(_OBJ)\EQFBIDI.OBJ  \
                          $(_OBJ)\EQFBRTFF.OBJ \
                          $(_OBJ)\EQFBRTF.OBJ  \
                          $(_OBJ)\EQFPAPI.OBJ  \
                          $(_OBJ)\EQFSEGMD.OBJ \
                          $(_OBJ)\EQFAPROF.OBJ \
                          $(_OBJ)\EQFENTITY.OBJ \
                          $(_OBJ)\EQFFSRCH.OBJ \
                          $(_OBJ)\EQFSETUP.OBJ

#------------------------------------------------------------------------------
# Build EQFDLL.DLL                                                          -
#------------------------------------------------------------------------------


$(_DLL)\EQFDLL.DLL:
    @echo ---- Linking $(_DLL)\EQFDLL.DLL
    @echo ---- Linking $(_DLL)\EQFDLL.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
                          $(_OBJ)\EQFCOPYR.OBJ
                          $(_OBJ)\EQFLOGO.OBJ
                          $(_OBJ)\EQFDRVEX.OBJ
                          $(_OBJ)\CSTUB.OBJ
                          $(_OBJ)\EQFANA00.OBJ
                          $(_OBJ)\EQFBCLIP.OBJ
                          $(_OBJ)\EQFBDLG.OBJ
                          $(_OBJ)\EQFBDLGF.OBJ
                          $(_OBJ)\EQFBDLGP.OBJ
                          $(_OBJ)\EQFBDLGS.OBJ
                          $(_OBJ)\EQFBDOC.OBJ
                          $(_OBJ)\EQFBFILE.OBJ
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
                          $(_OBJ)\EQFBWPRC.OBJ
                          $(_OBJ)\EQFCMPR.OBJ
                          $(_OBJ)\EQFCNT00.OBJ
                          $(_OBJ)\EQFCNT01.OBJ
                          $(_OBJ)\EQFCSTUB.OBJ
                          $(_OBJ)\EQFDAM.OBJ
                          $(_OBJ)\EQFDASD.OBJ
                          $(_OBJ)\EQFDASDM.OBJ
                          $(_OBJ)\EQFDASDT.OBJ
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
                          $(_OBJ)\EQFPROPS.OBJ
                          $(_OBJ)\EQFFOL00.OBJ
                          $(_OBJ)\EQFFOL01.OBJ
                          $(_OBJ)\EQFFOL02.OBJ
                          $(_OBJ)\EQFFOL03.OBJ
                          $(_OBJ)\EQFFOL04.OBJ
                          $(_OBJ)\EQFFOL05.OBJ
                          $(_OBJ)\EQFFOL06.OBJ
                          $(_OBJ)\EQFFOLLI.OBJ
                          $(_OBJ)\EQFHASH.OBJ
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
                          $(_OBJ)\EQFNOISE.OBJ
                          $(_OBJ)\EQFOBJ00.OBJ
                          $(_OBJ)\EQFPARSE.OBJ
                          $(_OBJ)\EQFPRO00.OBJ
                          $(_OBJ)\EQFQDAM.OBJ
                          $(_OBJ)\EQFQDAMI.OBJ
                          $(_OBJ)\EQFQDAMU.OBJ
                          $(_OBJ)\EQFQDAMW.OBJ
                          $(_OBJ)\EQFQDPR.OBJ
                          $(_OBJ)\EQFQDPRA.OBJ
                          $(_OBJ)\EQFQDPRD.OBJ
                          $(_OBJ)\EQFQDPRP.OBJ
                          $(_OBJ)\EQFQDPRU.OBJ
                          $(_OBJ)\EQFQDSRV.OBJ
                          $(_OBJ)\EQFQLDB.OBJ
                          $(_OBJ)\EQFQLDBI.OBJ
                          $(_OBJ)\EQFRDICS.OBJ
                          $(_OBJ)\EQFRPT.OBJ
                          $(_OBJ)\EQFRPT00.OBJ
                          $(_OBJ)\EQFRPT01.OBJ
                          $(_OBJ)\EQFSDICS.OBJ
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
                          $(_OBJ)\EQFUTCLB.OBJ
                          $(_OBJ)\EQFUTDLG.OBJ
                          $(_OBJ)\EQFUTDOS.OBJ
                          $(_OBJ)\EQFUTERR.OBJ
                          $(_OBJ)\EQFUTFIL.OBJ
                          $(_OBJ)\EQFUTILS.OBJ
                          $(_OBJ)\EQFUTLNG.OBJ
                          $(_OBJ)\EQFUTPRT.OBJ
                          $(_OBJ)\EQFUTMDI.OBJ
                          $(_OBJ)\EQFWCNT.OBJ
                          $(_OBJ)\EQFXDOC.OBJ
                          $(_OBJ)\EQF_DA.OBJ
                          $(_OBJ)\EQF_MT.OBJ
                          $(_OBJ)\EQF_TM.OBJ
                          $(_OBJ)\EQF_TWBS.OBJ
                          $(_OBJ)\EQFOSWIN.OBJ
                          $(_OBJ)\EQFQDPRU.OBJ
                          $(_OBJ)\EQFPROGR.OBJ
                          $(_OBJ)\EQFMAIN.OBJ
                          $(_OBJ)\EQFMT00.OBJ
                          $(_OBJ)\EQFMT01.OBJ
                          $(_OBJ)\EQFBIDI.OBJ
                          $(_OBJ)\EQFBRTFF.OBJ
                          $(_OBJ)\EQFBRTF.OBJ
                          $(_OBJ)\EQFPAPI.OBJ
                          $(_OBJ)\EQFSEGMD.OBJ
                          $(_OBJ)\EQFAPROF.OBJ
                          $(_OBJ)\EQFENTITY.OBJ
                          $(_OBJ)\EQFFSRCH.OBJ
                          $(_OBJ)\EQFSETUP.OBJ
/OUT:$(_DLL)\EQFDLL.DLL
/MAP:$(_MAP)\EQFDLL.MAP $(_LINK_OPTIONS) /DLL
$(_LINK_LIB_CRT) $(_LIB)\EQF_API.LIB imm32.lib SHELL32.LIB COMCTL32.LIB $(_LIB)\EQFRPXML.LIB $(_LIB)\EQFTAML.LIB $(_LIB)\EQFNTM.LIB
/DEF:$(_DEF)\EQFDLL.$(_DEFEXT)
<<
