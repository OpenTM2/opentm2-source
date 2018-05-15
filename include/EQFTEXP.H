// Copyright (c) 1996-2011, International Business Machines
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
// $Revision: 1.2 $ ----------- 26 Feb 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
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
/*
 * $Header:   J:\DATA\EQFTEXP.HV_   1.1   18 Nov 1997 10:45:56   BUILD  $
 *
 * $Log:   J:\DATA\EQFTEXP.HV_  $
 *
 *    Rev 1.1   18 Nov 1997 10:45:56   BUILD
 * -- A48: add pTagAddInfo
 *
 *    Rev 1.0   09 Jan 1996 09:47:46   BUILD
 * Initial revision.
*/

#define BUFFERSIZE         4000            // Length of the text buffer

typedef struct _TAGEXPORT
{
   IDA_HEAD      IdaHead;                      // common header of instance area
   BUFFER        stBuffer;                     // input file handle
   PTAG          pTag;
   PATTRIBUTE    pAttribute;
   CHAR          szSourceName[MAX_LONGPATH];   // MUT name
   CHAR          szTargetName[MAX_LONGPATH];   // newly specified MUT name
   HPROP         hTagListProp;            // handle of tag list properties
   BOOL          fAscii;                  // TRUE = external format
   HWND          hwndDirLb;               // handle to directories listbox
   HWND          hwndNewNameSle;          // handle to new MUT name sle
   CHAR          szDrive[MAX_DRIVE];      // drive letter with :
   CHAR          szPath[MAX_LONGPATH];    // directory incl. trailing
   CHAR          szPathContent[MAX_LONGPATH];   // directory incl. trailing
   CHAR          szName[MAX_FNAME];             // filename
   CHAR          szPatternName[MAX_FNAME];      // active pattern name
   CHAR          szPatternExt[MAX_FEXT];        // active pattern extension
   CHAR          szString[MAX_LONGPATH];        // string buffer
   CHAR          szDummy[MAX_LONGPATH];         // string buffer
   CONTROLSIDA   ControlsIda;                   // pointer to controls ida
   FILEFINDBUF   ResultBuf;                     // DOS file find struct
   CHAR          szTitle[MAX_LONGPATH];         // title of export window
   CHAR          szExportTagTxt[MAX_LONGPATH];
   HWND          hwndExportTagTxt;
   HWND          hwndSlider;
   USHORT        usAttrCount;          // number of attributes
   USHORT        usTagCount;           // number of tags
   USHORT        usCount;              // counter
   PSZ           pString;              // string buffer
   BOOL          fKill;
   BOOL          fStop;
   IMEXTASK      Task;
   PBYTE         pOffset;
   USHORT        usMarkupCount;
   PTAGTABLE     pTagTable;
   BOOL          fOK;
   PTAGADDINFO   pTagAddInfos;         // pointer to additional tag info
} TAGEXPORT, *PTAGEXPORT;

