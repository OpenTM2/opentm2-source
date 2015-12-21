//+----------------------------------------------------------------------------+
//|EQFBDLGS.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:   R.Jornitz                                                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:  Spellchecking dialog within Translation Processor             |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
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
// $Revision: 1.2 $ ----------- 25 Mar 2008
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
// $Revision: 1.2 $ ----------- 7 Sep 2004
// --RJ: Accessibility: SpellDlg: support Enter on Help PB
// -- Spell Dlg: delete unused #ifdef WINDOWS compiler define
// 
//
// $Revision: 1.1 $ ----------- 30 Aug 2004
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
// $Revision: 1.2 $ ----------- 9 Sep 2003
// --RJ: P016951: RTFEditor: EQFBMisspelled: force highlight misspelled word
//
//
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
//
//
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
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
// $Revision: 1.5 $ ----------- 15 Jul 2002
// --RJ: use strncpy instead of strcpy to assure that no overtyping happens
//
//
// $Revision: 1.4 $ ----------- 6 Feb 2002
// --RJ: KBT1179: re-position spellcheck window
//
//
// $Revision: 1.3 $ ----------- 26 Nov 2001
// --RJ: fix error in Spellaid: use InsertItemW, QUERYITEMTEXTW
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// -- RJ: Unicode enablement
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.5 $ ----------- 7 May 2001
// -- RJ: replace AnsiToOem by EQFAnsiToOem
//
//
// $Revision: 1.4 $ ----------- 14 Feb 2001
// -- Thai: use CONDOEMTOANSI in spellcheck dialog
//
//
// $Revision: 1.3 $ ----------- 25 Sep 2000
// -- add support for more than 64k segments
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFBDLGS.CV_   1.14   31 May 1999 10:48:10   BUILD  $
 *
 * $Log:   K:\DATA\EQFBDLGS.CV_  $
 *
 *    Rev 1.14   31 May 1999 10:48:10   BUILD
 * -- KBT0510: redraw screen and reset pusHLType after spellcheck on filedlg
 *
 *    Rev 1.13   11 May 1999 10:40:40   BUILD
 * -- update marking of misspelled words after change/ ignore / addenda
 *
 *    Rev 1.12   03 May 1999 09:14:24   BUILD
 * -- R004357_HLMISSPELLED: highlight misspelled words; make EQFBSpellIgnoreCheck
 *    accessible
 *
 *    Rev 1.11   15 Feb 1999 08:11:36   BUILD
 * -- KBT0422: avoid trap if Spellcheck temp add is full
 *
 *    Rev 1.10   08 Feb 1999 09:10:54   BUILD
 * -- KBT0446: Spellcheck should highlight word: set pBlock->usEndSegNum
 *
 *    Rev 1.9   12 Oct 1998 10:06:48   BUILD
 * - clear IME when EF looses input focus
 *
 *    Rev 1.8   18 May 1998 13:34:34   BUILD
 * -- KBT0306: Spellcheck highlighting: EQFBSpellMisspelled:
 *             call EQFBGotoSeg prior to setting pBlock, because EQFBGotoSeg
 *             resets the blockmarking to zero
 *
 *    Rev 1.7   08 May 1998 13:58:16   BUILD
 * - Win32: fixed 'double-click in listbox does nothing' problem
 *
 *    Rev 1.6   14 Jan 1998 16:11:46   BUILD
 * - migrated to Win32 environment
 *
 *    Rev 1.5   11 Nov 1996 09:46:48   BUILD
 * -- add additional checking for UtlDispatch()
 *
 *    Rev 1.4   01 Apr 1996 08:29:52   BUILD
 * -- force correct conversion for optimized version
 *
 *    Rev 1.3   04 Mar 1996 10:52:46   BUILD
 * -- KWT0407: allow for dynamic allocation of temp addenda...
 *
 *    Rev 1.2   26 Feb 1996 15:45:24   BUILD
 * -- KWT0087: check for starting segment in case of Spell-Check file, too
 *
 *    Rev 1.1   12 Feb 1996 10:44:04   BUILD
 * -- KWT0305: position spellcheck dialog at bottom of screen
 *
 *    Rev 1.0   09 Jan 1996 09:01:12   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_PRINT            // print functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file
#include "EQFB.ID"                // Translation Processor IDs
#include "EQFBDLG.ID"             // dialog control IDs

MRESULT EQFBSpellInit( HWND, WPARAM, LPARAM );
MRESULT EQFBSpellCommand( HWND, WPARAM, LPARAM );
MRESULT EQFBSpellControl( HWND, SHORT, SHORT );
MRESULT EQFBSpellClose( HWND, WPARAM, LPARAM );
MRESULT EQFBSpellMisspelled (HWND,WPARAM, LPARAM );

VOID EQFBSpellCmdChange(HWND hwndDlg,PSPELLDATA pSpellData);
VOID EQFBSpellCmdIgnore(HWND hwndDlg,PSPELLDATA pSpellData);
static VOID EnableSpellControls ( HWND hwndDlg, BOOL fEnable );
HWND ReplaceWithUnicodeEditControl( HWND hwndDlg, int iID, HWND hwndInsertBehind );
HWND ReplaceWithUnicodeListboxControl( HWND hwndDlg, int iID, HWND hwndInsertBehind );

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBSpellDlgProc - dialog procedure for find dialog
*/
// Description:
//    spellcheck of translated segments
//
//   Flow (message driven):
//       case WM_INITDLG:
//         call EQFBSpellInit to initialize the dialog controls;
//       case WM_EQFMISSPELLED:
//         fill dialog with misspelled word and force
//         update of file and dialog  (call EQFBSpellMisspelled)
//       case WM_EQFPROOF:
//        proofread next segment
//       case WM_COMMAND
//         call EQFBSpellCommand to handle user commands;
//       case WM_CLOSE
//         call EQFBSpellClose to end the dialog;
//
// Arguments:
//       mp2 of WM_INITDLG msg = PSPELLDATA pSpellData ptr to data structure
//                 containing data areas for proofread
// Returns:
//
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK EQFBSPELLDLGPROC
(
   HWND hwndDlg,                       /* handle of dialog window             */
   WINMSG msg,
   WPARAM mp1,
   LPARAM mp2
)
{
   PTBSEGMENT  pTBSeg;                           //ptr to segment
   MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure
   USHORT usDataLen;                             // length of chProofData
   USHORT usRc;                                  // return code
   PSPELLDATA pSpellData;                        // struct. of Spellcheck data

   switch ( msg )
   {
      case WM_EQF_QUERYID: HANDLEQUERYID( ID_TB_SPELL_DLG, mp2 ); break;

      case WM_INITDLG:
         SETWINDOWID( hwndDlg, ID_TB_SPELL_DLG );
         EnableSpellControls( hwndDlg, FALSE );
         mResult = DIALOGINITRETURN( EQFBSpellInit( hwndDlg, mp1, mp2 ) );
         break;

      case WM_EQF_MISSPELLED:
         mResult = EQFBSpellMisspelled( hwndDlg, mp1, mp2 );

         break;
      case WM_EQF_PROOF:

         pSpellData = ACCESSDLGIDA(hwndDlg, PSPELLDATA);
         if (pSpellData)
         {
           PCHAR_W pData = pSpellData->chProofData;
           pTBSeg = EQFBGetVisSeg( pSpellData->pDoc,&(pSpellData->ulProofSeg));
           usDataLen = sizeof(pSpellData->chProofData);
           usRc = EQFPROOFW(pTBSeg->pDataW, pData, &usDataLen);
           if (!usRc)
           {
             UtlDispatch();
             /*********************************************************/
             /* highlight all misspelled words in current segment     */
             /*********************************************************/

             EQFBMisspelledHLType(pSpellData->pDoc,
                                  pTBSeg, pData);

             pSpellData = ACCESSDLGIDA(hwndDlg, PSPELLDATA);
             if ( pSpellData )
             {
               pSpellData->sStartOffset = 0;    // startpos for EQFBPosGotoSeg
                                      //pass begin of misspelled area
               pSpellData->pProofCurrent = pSpellData->chProofData;
               WinPostMsg(hwndDlg,WM_EQF_MISSPELLED,0L,pSpellData->pProofCurrent);
             }
             else
             {
               POSTEQFCLOSE( hwndDlg, FALSE );
             } /* endif */
           }
           else
           {
              //display error msg (area too small, no correct dict )
              UtlError( TB_ERRSPELLCHECK, MB_CANCEL, 0, NULL, EQF_ERROR );
              POSTEQFCLOSE( hwndDlg, FALSE );
           } /* endif */
         } /* endif */
         break;
      case WM_COMMAND:
         mResult = EQFBSpellCommand( hwndDlg, mp1, mp2 );
         break;

        case DM_GETDEFID:
          /************************************************************/
          /* check if user pressed the ENTER key, but wants only to   */
          /* select/deselect an item of the listbox via a simulated   */
          /* (keystroke) double click.                                */
          /************************************************************/
          if ((GetFocus() == GetDlgItem(hwndDlg, ID_TB_SPELL_PROOFAID_LB)) &&
               (GetKeyState(VK_RETURN) & 0x8000)  )
          {
            EQFBSpellControl( hwndDlg, ID_TB_SPELL_PROOFAID_LB, LN_ENTER );
            mResult = TRUE;
          }
          else
          {
            // pass info on to normal dialog proc...
            mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
          } /* endif */
          break;

      case WM_EQF_CLOSE:
         EnableSpellControls( hwndDlg, FALSE );
         mResult = EQFBSpellClose( hwndDlg, mp1, mp2 );
         break;

      default:
         mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
         break;
   } /* endswitch */

   return mResult;
} /* end of EQFBSpellDlgProc */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBSpellInit - initialization for spell dialog
*/
// Description:
//    Initialize all dialog controls and allocate required memory.
//
//   Flow:
//      - allocate and anchor dialog IDA;
//      - get ptr to spelldata structure
//      - init pSpellData structure fields
//      - if spellcheck file was selected and postedit was not already set...
//          set temporary post edit
//      - if ok so far:
//          if current segment is already translated, send msg to proofread
//             segment
//          else post WM_EQFMISSPELLED ( to find next translated segment)
//        else close spellcheck dialog
//
// Arguments:
//       mp2 of WM_INITDLG msg = PSPELLDATA pSpellData ptr to data structure
//                 containing data areas for proofread
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - ptr to IDA is stored in dialog word QWL_USER
//
// External references:
//
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBSpellInit
(
   HWND    hwndDlg,                    // handle of dialog window
   WPARAM  mp1,                        // first parameter of WM_INITDLG
   LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
   MRESULT     mResult = FALSE;        // result of message processing
   PTBSEGMENT pTBSeg;                  //ptr to segment struct
   PSPELLDATA pSpellData;              // Spell data structure
   BOOL fOK = TRUE;                    // FALSE if CANCEL from SaveSeg

   mp1 = mp1;                          //suppress 'unreferenced parameter' msg

   ANCHORDLGIDA( hwndDlg, mp2 );
   pSpellData = (PSPELLDATA) mp2;

   pSpellData->ulProofSeg = pSpellData->pDoc->TBCursor.ulSegNum;
   pSpellData->ulStartSeg = pSpellData->ulProofSeg;

   pSpellData->fPostEditOld = pSpellData->pDoc->EQFBFlags.PostEdit;
   pSpellData->ulActSeg = pSpellData->pDoc->tbActSeg.ulSegNum;
                                            // init all FindData fields
   pSpellData->FindData.usFirstCall = CH_CHNGEONLY;     // change without find
   pSpellData->FindData.fConfirmChanges = TRUE; //only 1 change
   pSpellData->FindData.usRange = CHANGE_SEGMENTONLY;
                                                 // only this segment..
   pSpellData->FindData.fForward = TRUE;
   pSpellData->FindData.fIgnoreCase = TRUE;

   /****************************************************************/
   /* store the position of an already marked block -- we have to  */
   /* use it temporarily to highlight items...                     */
   /****************************************************************/
   memcpy( &pSpellData->FindData.MarkedBlock,
           pSpellData->pDoc->pBlockMark,
           sizeof( pSpellData->FindData.MarkedBlock ));


   //if spellcheck file was selected and postedit was not already set...
   if (!pSpellData->fSegOnly && !pSpellData->fPostEditOld)
   {
     fOK = EQFBTempPostEdit( pSpellData->pDoc );
   } /* endif */
   if (fOK)
   {
     HWND hwndInsertBehind = HWND_TOP;
     BOOL isUnicode;

     // replace resource file created controls with Unicode enabled counterpart
     hwndInsertBehind = ReplaceWithUnicodeEditControl( hwndDlg, ID_TB_SPELL_WRONG_EF, hwndInsertBehind );
     hwndInsertBehind = ReplaceWithUnicodeEditControl( hwndDlg, ID_TB_SPELL_CORRECT_EF, hwndInsertBehind );
     //hwndInsertBehind = ReplaceWithUnicodeListboxControl( hwndDlg, ID_TB_SPELL_PROOFAID_LB, hwndInsertBehind );

     isUnicode = IsWindowUnicode( hwndInsertBehind );

     SETTEXTLIMIT( hwndDlg, ID_TB_SPELL_CORRECT_EF, MAX_FINDCHANGE_LEN );
     // set read only for field with misspelled word
     WinSendDlgItemMsg ( hwndDlg, ID_TB_SPELL_WRONG_EF, EM_SETREADONLY, 0 , 0L );

      // activate proofreading for 1st segment
      // if fSegOnly: error, else: set pWord=NULL
      // and   activate EQF_MISSPELLED

      pTBSeg = EQFBGetVisSeg(pSpellData->pDoc, &(pSpellData->ulProofSeg));
      if (pTBSeg &&
          ((pTBSeg->qStatus == QF_XLATED || pTBSeg->qStatus == QF_CURRENT) ||
          ((pSpellData->ulStartSeg == pSpellData->ulProofSeg) && (pTBSeg->qStatus == QF_TOBE) )))
      {
        WinPostMsg(hwndDlg, WM_EQF_PROOF, 0L, 0L);
      }
      else
      {
        *(pSpellData->chProofData) = EOS;
        WinPostMsg (hwndDlg, WM_EQF_MISSPELLED, 0L, pSpellData->chProofData);
      } /* endif */
      // position the dialog window
      WinSetWindowPos( hwndDlg, HWND_TOP, 100, 50, 0, 0, EQF_SWP_MOVE );
   }
   else                         //close in not OK
   {
     POSTEQFCLOSE( hwndDlg, FALSE );
   } /* endif */
   return ( mResult );
} /* end of EQFBSpellInit */


/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBSpellMisspelled - handle misspelled word
*/
// Description:
//    handle misspelled word, search for next word in background
//
//   Flow:  if next word available in proofdata-list:
//             - call EQFBSpellIgnoreCheck
//                 ( check if word is in Ignore- list)
//               while next word available and word is in ignorelist:
//                       goto next word in proofdata list
//                       and check ignorelist for this word
//          if word is found which has to be displayed:
//             - fill fields of dialog
//             - get aid (service function EQFPROOFAID)
//             - fill listbox with aid
//             - position document and update screen
//          else
//             - if user wants to check only 1 segment:
//                  - display ending message and post WM_CLOSE
//               else
//                  - determine next visible segment which is
//                         is already translated
//                  - if full cycle is done already
//                       display ending message
//                    else
//                       proofread next segment
//                       post WM_EQF_PROOF message
//
// Arguments:
//       mp2 of WM_INITDLG msg = PSPELLDATA pSpellData ptr to data structure
//                 containing data areas for proofread
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - ptr to IDA is stored in dialog word QWL_USER
//
// External references:
//   UtlAlloc
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBSpellMisspelled
(
   HWND    hwndDlg,                    // handle of dialog window
   WPARAM  mp1,                        // first parameter
   LPARAM  mp2                         // second parameter (pointer to word)
)
{
   PSPELLDATA pSpellData;              // Spell data structure
   MRESULT    mResult = FALSE;         // result of message processing
   SHORT      sSegOffset;              // offset of pWord in segment
   PSZ_W      pWord;                   //current misspelled word
   PTBSEGMENT pTBSeg;                  //ptr to segment struct
   BOOL       fIgnore = TRUE;          // true if word in ignorelist
   USHORT     usRc;                    //return from EQFProofAid
   USHORT     usLength;                // length of chAidData
   PCHAR_W    pAid;                    // ptr to chAidData
   BOOL       fAnsiToOEM = TRUE;

//         mp2 contains ptr to word to be handled
//         fill dialog with misspelled word and force
//         update of file and dialog
     mp1;
     pSpellData = ACCESSDLGIDA(hwndDlg, PSPELLDATA);
     // find next word in proofdata list
     pWord = (PSZ_W) mp2;
     if (*pWord)
     {
        fIgnore = EQFBSpellIgnoreCheck(pSpellData);
     } /* endif */
     // check if word is already in ignorelist
     while (*pWord && fIgnore)
     {
       pSpellData->pProofCurrent += UTF16strlenCHAR(pSpellData->pProofCurrent) + 1;
       pWord = pSpellData->pProofCurrent;
       if (*pWord)
       {
         fIgnore = EQFBSpellIgnoreCheck(pSpellData);
       } /* endif */

     } /* endwhile */
     // check if word is available or valid
     fAnsiToOEM = CheckForAnsiConv(pSpellData->pDoc);
    if ( *pWord )                     // if not eq end of string
     {
        CHAR_W chTemp[ 256 ];
        USHORT  usTempLen = 0;
     // fill fields of dialog

        usTempLen = (USHORT)min((sizeof(chTemp)/sizeof(CHAR_W))-1, UTF16strlenCHAR(pWord));
        UTF16strncpy( chTemp, pWord, usTempLen);
        chTemp[usTempLen] = EOS;

        SETTEXTW( hwndDlg, ID_TB_SPELL_WRONG_EF, chTemp );
        SETTEXTW( hwndDlg, ID_TB_SPELL_CORRECT_EF, chTemp );
        SETFOCUS( hwndDlg, ID_TB_SPELL_CORRECT_EF );
     // mark field as selected , i.e. allow easy overtype
        SETEFSEL( hwndDlg, ID_TB_SPELL_CORRECT_EF, 0, -1 );
        /**************************************************************/
        /* delete old entries in AID listbox, init calling parameters */
        /* and call AID function                                      */
        /**************************************************************/
        DELETEALL( hwndDlg, ID_TB_SPELL_PROOFAID_LB );
        /**************************************************************/
        /* force display of window...                                 */
        /**************************************************************/
        UpdateWindow( hwndDlg);

        usLength = (MAX_SEGMENT_SIZE)/4;
        pSpellData->chAidData[0] = EOS;
        pAid = pSpellData->chAidData;
        usRc = EQFPROOFAIDW(pWord, pAid, &usLength);
        if (!usRc)
        {
           // as long as data available, insert them into listbox
           pAid = pSpellData->chAidData;
           while (*pAid)
           {
              SendDlgItemMessageW( hwndDlg, ID_TB_SPELL_PROOFAID_LB, LB_ADDSTRING, 0, (LPARAM)pAid );
              pAid += UTF16strlenCHAR(pAid) + 1;
           } /* endwhile */
        } /* endif */

     // position doc and update screen
        // determine position of misspelled word in segment
        sSegOffset = EQFBPosGotoSeg(pSpellData->pDoc,pSpellData->ulProofSeg,
                                       pWord,pSpellData->sStartOffset);
        if (sSegOffset != -1)          // if match found
        {
           pSpellData->sStartOffset = sSegOffset + (SHORT)UTF16strlenCHAR(pWord);
		   if ( pSpellData->pDoc->hwndRichEdit )
		   {
			 EQFBFindGotoSegRTF( pSpellData->pDoc, pSpellData->ulProofSeg, sSegOffset,
			                          pSpellData->sStartOffset - sSegOffset );
		   } /* endif */
		   else
		   { PEQFBBLOCK  pBlock = NULL;

             EQFBGotoSeg(pSpellData->pDoc,pSpellData->ulProofSeg, sSegOffset);

             /****************************************************************/
             /* we will mark match as selected, so that we can see it more   */
             /* visible                                                      */
             /****************************************************************/
             pBlock = (PEQFBBLOCK) pSpellData->pDoc->pBlockMark;
             pBlock->pDoc       = pSpellData->pDoc;
             pBlock->ulSegNum   = pSpellData->ulProofSeg;
             pBlock->usStart    = sSegOffset;
             pBlock->usEnd      = pSpellData->sStartOffset - 1;
             pBlock->ulEndSegNum = pSpellData->ulProofSeg;
	       }

           EnableSpellControls( hwndDlg, TRUE );
           SETCURSOR( SPTR_ARROW );
           EQFBScreenData(pSpellData->pDoc);            // display screen
           EQFBScreenCursor(pSpellData->pDoc);          // update cursor
        }
        else                            // if match is not found
        { // go on with next word in list(if avail) or next segment
          // actually skip current word
            pSpellData->pProofCurrent +=
                                  UTF16strlenCHAR(pSpellData->pProofCurrent) + 1;
            WinPostMsg (hwndDlg, WM_EQF_MISSPELLED, 0L,
                        pSpellData->pProofCurrent);
        } /* endif */
     }
     else
     {                                 // end of listing
                                       //next segment possible?
       if (pSpellData->fSegOnly)
       {
          //display ending message
         UtlError( TB_ENDSPELLCHECK, MB_CANCEL, 0, NULL, EQF_WARNING );

         POSTEQFCLOSE( hwndDlg, FALSE );
       }
       else
       {
          //determine next segment if available
          pSpellData->ulProofSeg++;
          pTBSeg = EQFBGetVisSeg(pSpellData->pDoc, &(pSpellData->ulProofSeg));  // point to start of table

          while (pTBSeg && pTBSeg->qStatus != QF_XLATED
                  && ( pTBSeg->ulSegNum != pSpellData->ulStartSeg))
          {
            pSpellData->ulProofSeg++;
            pTBSeg = EQFBGetVisSeg(pSpellData->pDoc, &(pSpellData->ulProofSeg));
          } /* endwhile */
          //if not start segment and if no transl.seg found
          //                 (i.e. end of file reached)
          if ((pSpellData->ulProofSeg != pSpellData->ulStartSeg)
              && ( !pTBSeg || pTBSeg->qStatus != QF_XLATED))
          {
             //wrap around
             pSpellData->ulProofSeg = 1;
            pTBSeg = EQFBGetVisSeg(pSpellData->pDoc, &(pSpellData->ulProofSeg));
            while (pTBSeg && pTBSeg->qStatus != QF_XLATED
                   && (pTBSeg->ulSegNum != pSpellData->ulStartSeg))
            {
               pSpellData->ulProofSeg++;
               pTBSeg = EQFBGetVisSeg(pSpellData->pDoc,
                                      &(pSpellData->ulProofSeg));
            } /* endwhile */
          } /* endif */
          pSpellData->ulProofSeg = (pTBSeg->ulSegNum);

          //determine if full cycle done already
          if (pSpellData->ulProofSeg == pSpellData->ulStartSeg)
          {
            /**********************************************************/
            /* position cursor at segment where we started...         */
            /**********************************************************/
            pSpellData->pDoc->TBCursor.ulSegNum = pSpellData->ulStartSeg;
             //display ending message
            UtlError( TB_ENDSPELLCHECK, MB_CANCEL, 0, NULL, EQF_WARNING );
            POSTEQFCLOSE( hwndDlg, FALSE );
          }
          else
          {   //proofread next segment
             sSegOffset = 0;               //start searching word at begin
                                           // of segment
                                           //(in next WM_EQF_MISSPELLED)
             WinPostMsg(hwndDlg, WM_EQF_PROOF, 0L, 0L);

          } /* endif */

       } /* endif */

     } /* endif */
   return ( mResult );
} /* end of EQFBSpellMisspelled */

/**********************************************************************/
/* enable or disable the controls on the spell dialog                 */
/**********************************************************************/
static VOID EnableSpellControls
(
  HWND hwndDlg,
  BOOL fEnable
)
{
  ENABLECTRL( hwndDlg, ID_TB_SPELL_CHANGE_PB, fEnable );
  ENABLECTRL( hwndDlg, ID_TB_SPELL_SKIP_PB,   fEnable );
  ENABLECTRL( hwndDlg, ID_TB_SPELL_ADDENDA_PB,fEnable );
  ENABLECTRL( hwndDlg, ID_TB_SPELL_TEMPADD_PB,fEnable );
  ENABLECTRL( hwndDlg, ID_TB_SPELL_CANCEL_PB, fEnable );
  return;
}
/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBSpellCommand - process WM_COMMAND messages of spell dialog
*/
// Description:
//    Handle WM_COMMAND messages (= pressing of pushbuttons) of
//    spell dialog panel.
//
//   Flow (message driven):
//      case 'Temp Add' pushbutton:
//         remember word in list of ignore-words
//      case 'Change' pushbutton:
//         change in given area the misspelled word
//         update translation memory, find next misspelled word
//           (using the change function from the FIND/CHANGE dialog)
//      case Skip pushbutton:
//         skip current misspelled word
//      case CANCEL pushbutton or DID_CANCEL (= ESCAPE key):
//         cancel dialog, go through 'change alll'list?
//      case ADDENDA pushbutton:
//        copy word to addendum
//        post WM_EQFMISSPELLED with next word
//
// Arguments:
//   SHORT1FROMMP(mp1) = ID of control sending the WM_COMMAND message
//
// Returns:
//  MRESULT(TRUE)  = command is processed
//
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBSpellCommand
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   PSPELLDATA pSpellData;                   // SpellData structure
   MRESULT mResult = MRFROMSHORT(TRUE);          // TRUE = command is processed

   mp2 = mp2;                                  // to avoid warning..

   pSpellData = ACCESSDLGIDA(hwndDlg, PSPELLDATA);

   switch ( WMCOMMANDID( mp1, mp2 ) )
   {
	  case ID_TB_SPELL_HELP_PB:
	    mResult = UtlInvokeHelp();
	     break;
      case ID_TB_SPELL_CHANGE_PB:          // change error to corrected word
         SETCURSOR( SPTR_WAIT );
         EnableSpellControls( hwndDlg, FALSE );
         EQFBSpellCmdChange(hwndDlg,pSpellData);
         break;
      case ID_TB_SPELL_SKIP_PB:          // skip word
         SETCURSOR( SPTR_WAIT );
         EnableSpellControls( hwndDlg, FALSE );
         pSpellData->pProofCurrent += UTF16strlenCHAR(pSpellData->pProofCurrent) + 1;
         WinPostMsg (hwndDlg, WM_EQF_MISSPELLED, 0L,
                     pSpellData->pProofCurrent);
         break;

      case ID_TB_SPELL_ADDENDA_PB:         // copy word to addenda
         /*************************************************************/
         /* put it into the addenda AND in the TempAddenda list to    */
         /* ensure proper misspelling of the same word in the same    */
         /* segment more than once.....                               */
         /*************************************************************/
         SETCURSOR( SPTR_WAIT );
         EnableSpellControls( hwndDlg, FALSE );
         EQFPROOFADDW(pSpellData->pProofCurrent);
         EQFBSpellCmdIgnore(hwndDlg,pSpellData);
         break;

      case ID_TB_SPELL_TEMPADD_PB:          // ignore misspelled word
         SETCURSOR( SPTR_WAIT );
         EnableSpellControls( hwndDlg, FALSE );
        EQFBSpellCmdIgnore(hwndDlg,pSpellData);
        break;
      case ID_TB_SPELL_CANCEL_PB:
      case DID_CANCEL:

         pSpellData->pDoc->Redraw = REDRAW_ALL;
         POSTEQFCLOSE( hwndDlg, FALSE );
         break;

      case ID_TB_SPELL_PROOFAID_LB:
          mResult = EQFBSpellControl( hwndDlg, WMCOMMANDID( mp1, mp2 ),
                                      WMCOMMANDCMD( mp1, mp2 ) );
          break;

       case ID_TB_SPELL_CORRECT_EF:
           if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
           {
             ClearIME( hwndDlg );
           } /* endif */
           break;

        default:
          mResult = WinDefDlgProc( hwndDlg, WM_COMMAND, mp1, mp2 );
          break;
     } /* endswitch */

   return( mResult );
} /* end of EQFBSpellCommand */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBSpellClose - process WM_CLOSE messages of spell dialog
*/
// Description:
//    Handle WM_CLOSE messages (= dialog termination requests) of
//    spell dialog panel.
//
//   Flow:
//       - if nec. reset temporary post edit mode
//       - dismiss dialog
//
// Arguments:
//   SHORT1FROMMP(mp1) = type to be returned using WinDismissDlg
//
// Returns:
//  MRESULT(FALSE)
//
// Prereqs:
//   None
//
// SideEffects:
//   - dialog is removed
//   - caller receives type specifed in mp1
//
// External references:
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBSpellClose
(
   HWND hwndDlg,
   WPARAM mp1,
   LPARAM mp2
)
{
   PSPELLDATA   pSpellData;                   // SpellData structure
   MRESULT      mResult = FALSE;
   USHORT       ulSegNum = 1;
   PTBDOCUMENT  pDoc;
   PTBSEGMENT   pSeg;

   pSpellData = ACCESSDLGIDA(hwndDlg, PSPELLDATA);

   mp1;                          // supress 'unreferenced parameter' msg
   mp2;                          // supress 'unreferenced parameter' msg

   /****************************************************************/
   /* restore the position of the block and force repaint of screen*/
   /* -- we might have to update some areas...                     */
   /****************************************************************/
   memcpy( pSpellData->pDoc->pBlockMark,
           &pSpellData->FindData.MarkedBlock,
           sizeof( pSpellData->FindData.MarkedBlock ));

   pDoc = pSpellData->pDoc;
   if (!pSpellData->fSegOnly && !pSpellData->fPostEditOld)
   {
      EQFBTempPostEdit( pSpellData->pDoc );
   } /* endif */

    ulSegNum = 1;
    pSeg = EQFBGetSegW(pDoc, ulSegNum);

    while (pSeg && (ulSegNum < pDoc->ulMaxSeg) )
    {
      if (pSeg->pusHLType )
      {
        UtlAlloc((PVOID *)&(pSeg->pusHLType) ,0L ,0L , NOMSG);
      } /* endif */
      pSeg->SegFlags.Spellchecked = FALSE;
      ulSegNum++;
      pSeg = EQFBGetSegW(pDoc, ulSegNum);
    } /* endwhile */
    pDoc->Redraw |= REDRAW_ALL;

   //--- get rid off dialog ---
   DISMISSDLG( hwndDlg, TRUE );

   return( mResult );
} /* end of EQFBSpellClose */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBSpellCmdChange - process change in file and translation memory
*/
// Description:
//    update the segment with the corrected word and the translation memory
//
//
//   Flow:
//         - get input from entry fields
//         - call EQFBCmdChange (change in file and in TM)
//         - post WM_EQFMISSPELLED with next word
//
// Arguments:
//   SHORT1FROMMP(mp1) = type to be returned using WinDismissDlg
//
// Returns:
//
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//
//////////////////////////////////////////////////////////////////////////////
VOID EQFBSpellCmdChange
(
HWND         hwndDlg,
PSPELLDATA   pSpellData
)
{
  // fill the SpellData structure
  QUERYTEXTW( hwndDlg, ID_TB_SPELL_CORRECT_EF, pSpellData->FindData.chReplace );
  QUERYTEXTW( hwndDlg, ID_TB_SPELL_WRONG_EF, pSpellData->FindData.chFind );

  UTF16strcpy( pSpellData->FindData.chFindTarget, pSpellData->FindData.chFind);
  //set begin SegNum and SegOffset inFindData structure
  pSpellData->FindData.ulSegNumBegin = pSpellData->pDoc->TBCursor.ulSegNum;
  pSpellData->FindData.ulStartSegNum = pSpellData->pDoc->TBCursor.ulSegNum;
  pSpellData->FindData.usSegOffsetBegin
                = pSpellData->pDoc->TBCursor.usSegOffset;

  // change it in file and automatically ( in EQFBWorkSegCheck) in TM
  EQFBCmdChange(pSpellData->pDoc,&(pSpellData->FindData));

  // goto next misspelled word
  // pass pointer to next word
  pSpellData->pProofCurrent += UTF16strlenCHAR(pSpellData->FindData.chFind) + 1;

  {   PTBSEGMENT  pTBSeg;
      pTBSeg = EQFBGetVisSeg( pSpellData->pDoc,&(pSpellData->ulProofSeg));
      EQFBMisspelledHLType(pSpellData->pDoc,
                           pTBSeg, pSpellData->pProofCurrent);
  }

  WinPostMsg (hwndDlg, WM_EQF_MISSPELLED, 0L, pSpellData->pProofCurrent);

return;
} /* end of EQFBSpellCmdChange */
/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBSpellCmdIgnore - ignore the misspelled word
*/
// Description:
//    add the misspelled word to the ignore word list
//   Flow:
//         - get word and pass it in ignorelist
//         - reset ptr to next free position in ignorelist
//         - post WM_EQFMISSPELLED with next word
//
//
// Arguments:
//   SHORT1FROMMP(mp1) = type to be returned using WinDismissDlg
//
// Returns:
//
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//
//////////////////////////////////////////////////////////////////////////////
VOID EQFBSpellCmdIgnore
(
HWND         hwndDlg,
PSPELLDATA   pSpellData
)
{
  ULONG      ulNextFreePos = 0L;

  if ( (pSpellData->pIgnoreNextFree + UTF16strlenCHAR(pSpellData->pProofCurrent)
         >= pSpellData->pIgnoreData+pSpellData->usIgnoreLen-2 ))
  {
    ulNextFreePos = pSpellData->pIgnoreNextFree - pSpellData->pIgnoreData;
    UtlAlloc( (PVOID *)&(pSpellData->pIgnoreData) ,
              (LONG) pSpellData->usIgnoreLen,
              (LONG) (pSpellData->usIgnoreLen+MAX_SEGMENT_SIZE) * sizeof(CHAR_W), ERROR_STORAGE );
    if ( pSpellData->pIgnoreData )
    {
      pSpellData->pIgnoreNextFree = ulNextFreePos + pSpellData->pIgnoreData;
      pSpellData->usIgnoreLen += MAX_SEGMENT_SIZE;
    } /* endif */
  } /* endif */
  // get word and pass it in ignorelist, reset pointer to next free
  // position in ignorelist
  if (pSpellData->pIgnoreNextFree + UTF16strlenCHAR(pSpellData->pProofCurrent)
      < pSpellData->pIgnoreData + pSpellData->usIgnoreLen - 2 )
  {
     UTF16strcpy(pSpellData->pIgnoreNextFree,pSpellData->pProofCurrent);
     pSpellData->pIgnoreNextFree += UTF16strlenCHAR(pSpellData->pProofCurrent);
     *(pSpellData->pIgnoreNextFree) = EOS;
     pSpellData->pIgnoreNextFree ++;
  }
  else
  { //return error message : ignore list is full, no further word
     // can be ignored during this session
    UtlError( TB_IGNOREERRSPELLCHECK, MB_CANCEL, 0, NULL, EQF_ERROR );

  } /* endif */

  // goto next misspelled word
  // pass pointer to next word
  pSpellData->pProofCurrent += UTF16strlenCHAR(pSpellData->pProofCurrent) + 1;

  {   PTBSEGMENT  pTBSeg;
      pTBSeg = EQFBGetVisSeg( pSpellData->pDoc,&(pSpellData->ulProofSeg));
      EQFBMisspelledHLType(pSpellData->pDoc,
                           pTBSeg, pSpellData->pProofCurrent);
  }
  WinPostMsg (hwndDlg, WM_EQF_MISSPELLED, 0L, pSpellData->pProofCurrent);
  return;
} /* end of EQFBSpellCmdIgnore */

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBSpellIgnoreCheck - check if word is in ignore list
*/
// Description:
//    check if the misspelled word is in the ignore word list
//   Flow:
//       while not end of ignore list and
//                no match found, compare words
//
//
// Arguments:
//   SHORT1FROMMP(mp1) = type to be returned using WinDismissDlg
//
// Returns:  TRUE if word is in ignorelist
//           FALSE if word is not in ignorelist
//
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//
//////////////////////////////////////////////////////////////////////////////
BOOL EQFBSpellIgnoreCheck
(
  PSPELLDATA   pSpellData
)
{
  BOOL fReturn = FALSE;             // true if word is in ignorelist
  PSZ_W pIgnoreWord;                // ptr in ignore list for while loop
  LONG  lComp = 1L;                  // return from strcmp

  /********************************************************************/
  /* check if returned word is punctuation, i.e. if it is of length 1 */
  /********************************************************************/
  if ( pSpellData->pProofCurrent[1] == EOS )
  {
    fReturn = TRUE;                    // ignore this word
  }
  else
  {
    /******************************************************************/
    /* while not end of ignore list and no match found, compare words */
    /******************************************************************/
    pIgnoreWord = pSpellData->pIgnoreData;

    while ((lComp != 0) && (pIgnoreWord != pSpellData->pIgnoreNextFree))
    {
       lComp = UTF16strcmp(pIgnoreWord,pSpellData->pProofCurrent);
       if (lComp == 0)
       {
          fReturn = TRUE;            // word found in ignorelist
       }
       else
       {                             // check next word in ignorelist
         pIgnoreWord += UTF16strlenCHAR(pIgnoreWord) + 1;
       } /* endif */
    } /* endwhile */
  } /* endif */

  return(fReturn);
}

/*////////////////////////////////////////////////////////////////////////////
:h2.EQFBSpellControl - process WM_Control messages of spell dialog
*/
// Description:
//    Handle WM_CONTROL messages (= pressing of pushbuttons) of
//    spell dialog panel.
//
//   Flow (message driven):
//      case LN_ENTER from aid listbox:
//         make selected word the active one;
//         copy word in field for the corrected word
//
// Arguments:
//   sId               = ID of control sending the WM_COMMAND message
//   sNotification     = id of notification message
//
// Returns:
//  MRESULT(TRUE)  = command is processed
//
// Prereqs:
//   None
//
// SideEffects:
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
MRESULT EQFBSpellControl
(
   HWND hwndDlg,
   SHORT  sId,                             // id of field
   SHORT  sNotification                    // notification
)
{
   PSPELLDATA pSpellData;                   // SpellData structure
   MRESULT mResult = MRFROMSHORT(TRUE);     // TRUE = command is processed
   SHORT   sItem;                           // no.of selected word in listbox

   pSpellData = ACCESSDLGIDA(hwndDlg, PSPELLDATA);

   switch ( sId )
   {
      case ID_TB_SPELL_PROOFAID_LB:
        if ( sNotification == LN_ENTER )
        {
           sItem = QUERYSELECTION( hwndDlg, ID_TB_SPELL_PROOFAID_LB );
           if ( sItem != LIT_NONE )
           {
              QUERYITEMTEXTW(hwndDlg,ID_TB_SPELL_PROOFAID_LB,sItem,pSpellData->FindData.chReplace);

            // fill window with selected word
              SETTEXTW(hwndDlg,ID_TB_SPELL_CORRECT_EF,pSpellData->FindData.chReplace);
              SETFOCUS( hwndDlg, ID_TB_SPELL_CORRECT_EF );
           // mark field as selected , i.e. allow easy overtype
              SETEFSEL( hwndDlg, ID_TB_SPELL_CORRECT_EF, 0, -1 );
           } /* endif */
        } /* endif */
        break;
   } /* endswitch */

   return( mResult );
} /* end of EQFBSpellControl */

// replace a resource file created edit control with its Unicode enabled counterpart
HWND ReplaceWithUnicodeEditControl( HWND hwndDlg, int iID, HWND hwndInsertBehind )
{
  WINDOWPLACEMENT Placement;
  HWND hwndEF   = GetDlgItem( hwndDlg, iID );
  HFONT   hFont;

  GetWindowPlacement( hwndEF, &Placement );
  hFont = (HFONT)SendMessage( hwndEF, WM_GETFONT, 0, 0L );
  DestroyWindow( hwndEF );
  hwndEF = CreateWindowExW( 0, L"Edit", L"", 
                            ES_LEFT | WS_BORDER | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL| WS_CHILD,
                            Placement.rcNormalPosition.left, Placement.rcNormalPosition.top, 
                            Placement.rcNormalPosition.right - Placement.rcNormalPosition.left, 
                            Placement.rcNormalPosition.bottom - Placement.rcNormalPosition.top, 
                            hwndDlg, (HMENU)iID, (HINSTANCE)(HAB)UtlQueryULong( QL_HAB ), 0 );
  SetWindowLong( hwndEF, GWL_ID, (LONG)iID );
  SetWindowPos( hwndEF, hwndInsertBehind, 0, 0, Placement.rcNormalPosition.right - Placement.rcNormalPosition.left, 
                            Placement.rcNormalPosition.bottom - Placement.rcNormalPosition.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW );
  if ( hFont != NULL ) SendMessage( hwndEF, WM_SETFONT, (WPARAM)hFont, 0L ); 
  return( hwndEF );
}

// replace a resource file created listbox control with its Unicode enabled counterpart
HWND ReplaceWithUnicodeListboxControl( HWND hwndDlg, int iID, HWND hwndInsertBehind )
{
  WINDOWPLACEMENT Placement;
  HWND hwndLB   = GetDlgItem( hwndDlg, iID );
  HFONT   hFont;

  GetWindowPlacement( hwndLB, &Placement );
  hFont = (HFONT)SendMessage( hwndLB, WM_GETFONT, 0, 0L );
  DestroyWindow( hwndLB );
  hwndLB = CreateWindowExW( 0, L"LISTBOX", L"", 
                            LBS_STANDARD | LBS_NOTIFY | WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                            Placement.rcNormalPosition.left, Placement.rcNormalPosition.top, 
                            Placement.rcNormalPosition.right - Placement.rcNormalPosition.left, 
                            Placement.rcNormalPosition.bottom - Placement.rcNormalPosition.top, 
                            hwndDlg, (HMENU)iID, (HINSTANCE)(HAB)UtlQueryULong( QL_HAB ), 0 );
  SetWindowLong( hwndLB, GWL_ID, (LONG)iID );
  SetWindowPos( hwndLB, hwndInsertBehind, 0, 0, Placement.rcNormalPosition.right - Placement.rcNormalPosition.left, 
                            Placement.rcNormalPosition.bottom - Placement.rcNormalPosition.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW );
  if ( hFont != NULL ) SendMessage( hwndLB, WM_SETFONT, (WPARAM)hFont, 0L ); 
  return( hwndLB );
}
