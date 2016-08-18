// Copyright (c) 1990-2012, International Business Machines
// Corporation and others.  All rights reserved.
// 
// $CMVC 
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 31 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
// 
// 
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
// 
// 
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
// 
// 
// 
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
// 
/**********************************************************************/
/*                                                                    */
/*              Licensed Materials - Property of IBM                  */
/*              20G0400 (C) Copyright IBM Corp. 1992.                 */
/*                    See Copyright Instructions.                     */
/*                                                                    */
/**********************************************************************/
/**********************************************************************/
/*                                                                    */
/*                           S L I D E R                              */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/* Slider Window Class                                                */
/**********************************************************************/

#define CCL_SLIDER     "CCL Slider"
#define CCL_SLIDER_DLL "FCLSLDW.DLL"

/**********************************************************************/
/* Ownerdraw id                                                       */
/**********************************************************************/
#define ODT_SLIDER      102              /* ownerdraw id for Slider   */

/**********************************************************************/
/* Define messages for the slider control                             */
/**********************************************************************/
#define SLM_ADDDETENT       (WM_USER+0x0369) /* Add detent niche      */
#define SLM_QUERYDETENTPOS  (WM_USER+0x036a) /* Query detent position */
#define SLM_QUERYSCALETEXT  (WM_USER+0x036b) /* Query tick text       */
#define SLM_QUERYSLIDERINFO (WM_USER+0x036c) /* Query slider info     */
#define SLM_QUERYTICKPOS    (WM_USER+0x036d) /* Query position of tick*/
#define SLM_QUERYTICKSIZE   (WM_USER+0x036e) /* Query size of tick    */
#define SLM_REMOVEDETENT    (WM_USER+0x036f) /* Remove detent niche   */
#define SLM_SETSCALETEXT    (WM_USER+0x0370) /* Set text above tick   */
#define SLM_SETSLIDERCOLORS (WM_USER+0x0371) /* Set colors of a slider*/
#define SLM_SETSLIDERINFO   (WM_USER+0x0372) /* Set slider parameters */
#define SLM_SETTICKSIZE     (WM_USER+0x0373) /* Set size of tick      */

/**********************************************************************/
/* Define notification messages for the slider control                */
/**********************************************************************/
#define SLN_CHANGE          1            /* Slider position changed   */
#define SLN_KILLFOCUS       2            /* Slider losing focus       */
#define SLN_SETFOCUS        3            /* Slider gaining focus      */
#define SLN_SLIDERTRACK     4            /* Slider dragged by user    */

/**********************************************************************/
/* Slider control data structure                                      */
/**********************************************************************/
typedef struct _SLDCDATA {
  DWORD   cbSize;                /* Size of control block             */
  WORD    usScale1Increments;    /* # of divisions on scale           */
  WORD    usScale1Spacing;       /* Space in pels between increments  */
  WORD    usScale2Increments;    /* # of divisions on scale           */
  WORD    usScale2Spacing;       /* Space in pels between increments  */
} SLDCDATA;

typedef SLDCDATA FAR *LPSLDCDATA;

/**********************************************************************/
/* Slider QueryScaleText                                              */
/**********************************************************************/
typedef struct _SLDQUERYSCALETEXT{
  WORD  wTickNum;                /* tick number to query text for     */
  WORD  wBufLen;                 /* length of text buffer             */
  LPSTR lpTickText;              /* pointer to text buffer            */
} SLDQUERYSCALETEXT;

typedef SLDQUERYSCALETEXT FAR *LPSLDQUERYSCALETEXT;

/**********************************************************************/
/* Slider info parameter structure                                    */
/**********************************************************************/
typedef struct _SLDSETSLIDERINFO {
  WORD  wInfoType;               /* id of component to be modified    */
  WORD  wArmPosType;             /* format of ulInfo                  */
  DWORD dwInfo;                  /* location or size of component     */
} SLDSETSLIDERINFO;

typedef SLDSETSLIDERINFO FAR *LPSLDSETSLIDERINFO;

/**********************************************************************/
/* Slider control style flag definition                               */
/**********************************************************************/
#define SLS_HORIZONTAL       0x00000000L /* Orient slider horizontally*/
#define SLS_VERTICAL         0x00000001L /* Orient slider vertically  */
#define SLS_CENTER           0x00000000L /* Center shaft in window    */
#define SLS_BOTTOM           0x00000002L /* Offset shaft to bottom (H)*/
#define SLS_TOP              0x00000004L /* Offset shaft to top (H)   */
#define SLS_LEFT             0x00000002L /* Offset shaft to left (V)  */
#define SLS_RIGHT            0x00000004L /* Offset shaft to right (V) */
#define SLS_SNAPTOINCREMENT  0x00000008L /* Snap to nearest increment */
#define SLS_BUTTONSBOTTOM    0x00000010L /* Add buttons at shaft bot. */
#define SLS_BUTTONSTOP       0x00000020L /* Add buttons at shaft top  */
#define SLS_BUTTONSLEFT      0x00000010L /* Add buttons left of shaft */
#define SLS_BUTTONSRIGHT     0x00000020L /* Add buttons right of shaft*/
#define SLS_OWNERDRAW        0x00000040L /* Owner draw some fields    */
#define SLS_READONLY         0x00000080L /* Provide a read only slider*/
#define SLS_RIBBONSTRIP      0x00000100L /* Provide a ribbon strip    */
#define SLS_HOMEBOTTOM       0x00000000L /* Set home position at bot. */
#define SLS_HOMETOP          0x00000200L /* Set home position at top  */
#define SLS_HOMELEFT         0x00000000L /* Set home position at left */
#define SLS_HOMERIGHT        0x00000200L /* Set home position at right*/
#define SLS_PRIMARYSCALE1    0x00000000L /* Scale 1 is primary scale  */
#define SLS_PRIMARYSCALE2    0x00000400L /* Scale 2 is primary scale  */

/**********************************************************************/
/* Style aliases for Dialog Editor support.  The Dialog Editor        */
/* doesn't provide a buffer big enough to hold a complete style       */
/* string with full style names, so we return a shorter string made   */
/* up of these equivalent style names.                                */
/**********************************************************************/
#define SLS_HORZ             SLS_HORIZONTAL
#define SLS_VERT             SLS_VERTICAL
#define SLS_C                SLS_CENTER
#define SLS_B                SLS_BOTTOM
#define SLS_T                SLS_TOP
#define SLS_L                SLS_LEFT
#define SLS_R                SLS_RIGHT
#define SLS_SNAP             SLS_SNAPTOINCREMENT
#define SLS_BB               SLS_BUTTONSBOTTOM
#define SLS_BT               SLS_BUTTONSTOP
#define SLS_BL               SLS_BUTTONSLEFT
#define SLS_BR               SLS_BUTTONSRIGHT
#define SLS_RIBBON           SLS_RIBBONSTRIP
#define SLS_HB               SLS_HOMEBOTTOM
#define SLS_HT               SLS_HOMETOP
#define SLS_HL               SLS_HOMELEFT
#define SLS_HR               SLS_HOMERIGHT
#define SLS_SCALE1           SLS_PRIMARYSCALE1
#define SLS_SCALE2           SLS_PRIMARYSCALE2

/**********************************************************************/
/* Message attributes for setting and querying slider components      */
/**********************************************************************/
#define SMA_SCALE1               0x0001
#define SMA_SCALE2               0x0002
#define SMA_SHAFTDIMENSIONS      0x0000
#define SMA_SHAFTPOSITION        0x0001
#define SMA_SLIDERARMDIMENSIONS  0x0002
#define SMA_SLIDERARMPOSITION    0x0003
#define SMA_RANGEVALUE           0x0000
#define SMA_INCREMENTVALUE       0x0001
#define SMA_SETALLTICKS          0xFFFF

/**********************************************************************/
/* Ownerdraw flag definitions                                         */
/**********************************************************************/
#define SDA_RIBBONSTRIP          0x0001
#define SDA_SLIDERSHAFT          0x0002
#define SDA_BACKGROUND           0x0003
#define SDA_SLIDERARM            0x0004

/**********************************************************************/
/* Attributes for setting the colors of a slider                      */
/**********************************************************************/
#define SDA_FOREGROUNDCOLOR      0x0001
#define SDA_BACKGROUNDCOLOR      0x0002

/**********************************************************************/
/* Error return codes                                                 */
/**********************************************************************/
#define SLDERR_INVALID_PARAMETERS     -1
