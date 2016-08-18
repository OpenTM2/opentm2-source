# +----------------------------------------------------------------------------+
# |  EQFD.MAK    - Makefile for Startup code for MFC implementation            |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2012, International Business Machines              |
# |      Corporation and others. All rights reserved                           |
# +----------------------------------------------------------------------------+
# |  Description:                                                              |
# +----------------------------------------------------------------------------+
# | PVCS Section                                                               |
#
# $CMVC
# 
# $Revision: 1.1 $ ----------- 14 Dec 2009
#  -- New Release TM6.2.0!!
# 
# 
# $Revision: 1.1 $ ----------- 1 Oct 2009
#  -- New Release TM6.1.8!!
# 
# 
# $Revision: 1.1 $ ----------- 2 Jun 2009
#  -- New Release TM6.1.7!!
# 
# 
# $Revision: 1.1 $ ----------- 8 Dec 2008
#  -- New Release TM6.1.6!!
# 
# 
# $Revision: 1.1 $ ----------- 23 Sep 2008
#  -- New Release TM6.1.5!!
# 
# 
# $Revision: 1.1 $ ----------- 23 Apr 2008
#  -- New Release TM6.1.4!!
# 
# 
# $Revision: 1.1 $ ----------- 13 Dec 2007
#  -- New Release TM6.1.3!!
# 
# 
# $Revision: 1.1 $ ----------- 29 Aug 2007
#  -- New Release TM6.1.2!!
# 
# 
# $Revision: 1.1 $ ----------- 20 Apr 2007
#  -- New Release TM6.1.1!!
# 
# 
# $Revision: 1.1 $ ----------- 20 Dec 2006
#  -- New Release TM6.1.0!!
# 
# 
# $Revision: 1.1 $ ----------- 9 May 2006
#  -- New Release TM6.0.11!!
# 
# 
# $Revision: 1.1 $ ----------- 20 Dec 2005
#  -- New Release TM6.0.10!!
# 
# 
# $Revision: 1.1 $ ----------- 16 Sep 2005
#  -- New Release TM6.0.9!!
# 
# 
# $Revision: 1.1 $ ----------- 18 May 2005
#  -- New Release TM6.0.8!!
# 
# 
# $Revision: 1.1 $ ----------- 29 Nov 2004
#  -- New Release TM6.0.7!!
# 
# 
# $Revision: 1.1 $ ----------- 31 Aug 2004
#  -- New Release TM6.0.6!!
# 
# 
# $Revision: 1.1 $ ----------- 3 May 2004
#  -- New Release TM6.0.5!!
# 
# 
# $Revision: 1.1 $ ----------- 15 Dec 2003
#  -- New Release TM6.0.4!!
# 
# 
# $Revision: 1.1 $ ----------- 6 Oct 2003
#  -- New Release TM6.0.3!!
# 
# 
# $Revision: 1.1 $ ----------- 26 Jun 2003
#  -- New Release TM6.0.2!!
# 
# 
# $Revision: 1.2 $ ----------- 17 Mar 2003
# --RJ: removed compiler defines not needed any more and rework code to avoid warnings
# 
# 
# $Revision: 1.1 $ ----------- 20 Feb 2003
#  -- New Release TM6.0.1!!
# 
# 
# $Revision: 1.1 $ ----------- 25 Jul 2002
#  -- New Release TM6.0!!
# 
# 
# $Revision: 1.1 $ ----------- 20 Aug 2001
#  -- New Release TM2.7.2!!
# 
# 
# $Revision: 1.6 $ ----------- 25 Jun 2001
# GQ: Added EQFMUPRP to list of objects for EQFD.EXE
# 
#
# $Revision: 1.5 $ ----------- 16 Oct 2000
# -- add version.lib to check for version of riched20.dll
#
#
# $Revision: 1.4 $ ----------- 21 Jun 2000
# -- add eqffnt99.cpp
#
#
# $Revision: 1.3 $ ----------- 4 May 2000
# -- add eqffll99.cpp
#
#
#
# $Revision: 1.2 $ ----------- 6 Dec 1999
#  -- Initial Revision!!
#
#
#  $Header:   K:\DATA\EQFD.MVZ   1.2   11 Oct 1999 15:40:02   BUILD  $
#
#  $Log:   K:\DATA\EQFD.MVZ  $
#
#     Rev 1.2   11 Oct 1999 15:40:02   BUILD
#  added icn
#
#     Rev 1.1   07 Dec 1998 10:29:50   BUILD
#  -- use correct Icon
#
#     Rev 1.0   26 Oct 1998 19:09:44   BUILD
#  Initial revision.
#


!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:    $(_BIN)\EQFD.EXE

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------
OBJS= $(_OBJ)\EQFD.OBJ      \
      $(_OBJ)\eqfcfrm.OBJ  \
      $(_OBJ)\eqfmfrm.OBJ  \
      $(_OBJ)\eqfclbox.OBJ  \
      $(_OBJ)\eqfdoc99.OBJ  \
      $(_OBJ)\eqfprc99.OBJ  \
      $(_OBJ)\eqfrpt99.OBJ  \
      $(_OBJ)\eqftmm99.OBJ  \
      $(_OBJ)\eqfdic99.OBJ  \
      $(_OBJ)\eqffll99.OBJ  \
      $(_OBJ)\eqffnt99.OBJ  \
      $(_OBJ)\eqfweb.OBJ  \
      $(_OBJ)\eqfmuprp.OBJ  \
      $(_OBJ)\eqfgen99.OBJ  \
      $(_OBJ)\eqfmfc.OBJ

$(_BIN)\EQFD.EXE:      $(OBJS)
#------------------------------------------------------------------------------
# Build EQFD.EXE                                                          -
#------------------------------------------------------------------------------
$(_BIN)\EQFD.EXE:
    @echo ---- Linking $(_BIN)\EQFD.EXE
    @echo ---- Linking $(_BIN)\EQFD.EXE >>$(_ERR)
    RC /D_WINDOWS /Fo$(_OBJ)\EQFSTART.RES $(_RC)\EQFSTART.RC
    CVTRES /NOLOGO /OUT:$(_OBJ)\EQFD.RBJ $(_OBJ)\EQFSTART.RES
    $(_LINKER) @<<lnk.rsp >>$(_ERR)
$(OBJS) $(_OBJ)\EQFD.RBJ
$(_LINK_CPP_OPTIONS)
/OUT:$(_BIN)\EQFD.EXE /pdb:"$(_BIN)\EQFD.pdb"
 EQFDLL.LIB version.lib
<<
