/*! \file
	Description: this module will contain the keyboard mapping and keyboard processing procedures

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_DEV
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TM
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_FOLDER
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"                    // Translation Processor priv. include file
#include "EQFB.ID"                     // Translation Processor PM IDs

#include <eqfdoc00.h>             // document instance ida...
// Add for R012027 start
#include "SpecialChardlg.h"
// Add end
#include "ReImportDoc.h"

//extern ULONG NLSLanguage;
/* dalia (start) */
#ifndef _WINDOWS                            //DALIAMM
ULONG NlsLanguage;
#include "BIDI.H"
#else
  ULONG NLSLanguage;
#endif
/* dalia (end) */


static VOID EQFBWindowMenu (PTBDOCUMENT, HMENU);      // set document names
static VOID EQFBDocEnvMenu (PTBDOCUMENT, HMENU);      // set TEnv doc names
static VOID EQFBTPROWndNames(PTBDOCUMENT, HMENU);      // set TM/2 Wnd names

//
// table with names for virtual keys
//
#define MAX_VKEYNAME   21              // length of names for virtual keys

typedef struct _VKNAME
{
   CHAR   szName[MAX_VKEYNAME];        // name for key
   UCHAR  ucValue;                     // virtual key value
} VKNAME, *PVKNAME;

VKNAME VKTable[] =
{
   { "",               (UCHAR)-1 },
   { "",                 VK_SHIFT },
   { "",                  VK_CTRL },
   { "",                   VK_ALT },
   { "",                 VK_BREAK },
   { "",             VK_BACKSPACE },
   { "",                   VK_TAB },
   { "",               VK_BACKTAB },
   { "",               VK_NEWLINE },
   { "",               VK_ALTGRAF },
   { "",                 VK_PAUSE },
   { "",              VK_CAPSLOCK },
   { "",                   VK_ESC },
   { "",                 VK_SPACE },
   { "",                VK_PAGEUP },
   { "",              VK_PAGEDOWN },
   { "",                   VK_END },
   { "",                  VK_HOME },
   { "",                  VK_LEFT },
   { "",                    VK_UP },
   { "",                 VK_RIGHT },
   { "",                  VK_DOWN },
   { "",             VK_PRINTSCRN },
   { "",                VK_INSERT },
   { "",                VK_DELETE },
   { "",              VK_SCRLLOCK },
   { "",               VK_NUMLOCK },
   { "",                 VK_ENTER },
   { "",                 VK_SYSRQ },
   { "",                    VK_F1 },
   { "",                    VK_F2 },
   { "",                    VK_F3 },
   { "",                    VK_F4 },
   { "",                    VK_F5 },
   { "",                    VK_F6 },
   { "",                    VK_F7 },
   { "",                    VK_F8 },
   { "",                    VK_F9 },
   { "",                   VK_F10 },
   { "",                   VK_F11 },
   { "",                   VK_F12 },
   { "",                   VK_F13 },
   { "",                   VK_F14 },
   { "",                   VK_F15 },
   { "",                   VK_F16 },
   { "",                   VK_F17 },
   { "",                   VK_F18 },
   { "",                   VK_F19 },
   { "",                   VK_F20 },
   { "",                   VK_F21 },
   { "",                   VK_F22 },
   { "",                   VK_F23 },
   { "",                   VK_F24 },
   { "",                      0 }
};

// keyboard mapping table
//   first index is shift state
//   second index is key code
//   the table contains function IDs + 1 (to allow zero as not used indicator)
//
static USHORT ausMapKey[16][256] = { 0 };
static USHORT ausITMMapKey[16][256] = { 0 };

static CHAR szBuffer[256];             // multi-purpose buffer

/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBBuildKeyTable - EQF Translation Browser - Build keyboard mapping table
*/
// Description:
//    Build a keyboard mapping table (used for function EQFBMakKey) from
//    a key assignment list (as read from profile or processed by keys dialog).
//
// Flow:
//    xxxx
//
// Arguments:
//    PKEYPROFTABLE pKeyList              -- ptr to a key list table
//
// Returns:
//    BOOL fOK       TRUE           - function completed successful
//                   FALSE          - an error occured
//
// Prereqs:
//   None
//
// SideEffects:
//   ausMapKey will contain the new map table.
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
BOOL EQFBBuildKeyTable
(
   PKEYPROFTABLE pKeyList              // ptr to a key list table
)
{
   PRESKEYTABLE pResKey;               // ptr for processing of reserved keys
   USHORT       usState;               // index for key states
   USHORT       usCode;                // index for key codes

   //--- delete the mapping table ---
   memset( ausMapKey, 0, sizeof(ausMapKey) );

   //--- set character keys ---
   for ( usState = 0; usState <= 1; usState++)
   {
      for ( usCode = 0; usCode <= 255; usCode++ )
      {
         ausMapKey[usState][usCode] = CHARACTER_FUNC + 1;
      } /* endfor */
   } /* endfor */

   //--- set user assignable keys ---
   while ( pKeyList->Function != LAST_FUNC )
   {
      if ( pKeyList->ucCode || pKeyList->ucState )
      {
         ausMapKey[pKeyList->ucState][pKeyList->ucCode] =
            pKeyList->Function + 1;
      } /* endif */
      pKeyList++;
   } /* endwhile */

   //--- set reserved keys ---
   pResKey = get_ResKeyTab();
   while ( pResKey->ucCode || pResKey->ucState )
   {
     if (pResKey->usAssignStatus != ASSIGNED_TO_OTHER )
     {
       ausMapKey[pResKey->ucState][pResKey->ucCode] = pResKey->Function + 1;
     } /* endif */
      pResKey++;
   } /* endwhile */



   return( TRUE );
} /* EQFBBuildKeyTable */


VOID EQFBBuildITMKeyTable
(
   PKEYPROFTABLE pKeyList,             // ptr to a key list table
   USHORT        usLast
)
{
   USHORT       usState;               // index for key states
   USHORT       usCode;                // index for key codes

   //--- delete the mapping table ---
   memset( ausITMMapKey, 0, sizeof(ausITMMapKey) );

   //--- set character keys ---
   for ( usState = 0; usState <= 1; usState++)
   {
      for ( usCode = 0; usCode <= 255; usCode++ )
      {
         ausITMMapKey[usState][usCode] = CHARACTER_ITMFUNC + 1;
      } /* endfor */
   } /* endfor */

   //--- set user assignable keys ---
   while ( pKeyList->Function != usLast )
   {
      if ( pKeyList->ucCode || pKeyList->ucState )
      {
         ausITMMapKey[pKeyList->ucState][pKeyList->ucCode] =
            pKeyList->Function + 1;
      } /* endif */
      pKeyList++;
   } /* endwhile */


   return;
} /* EQFBBuildITMKeyTable */


VOID EQFBAdaptKeyTable
(
   PKEYPROFTABLE pKeyList,             // ptr to a key list table
   PTBDOCUMENT   pDoc
)
{
   USHORT       usState;               // index for key states
   USHORT       usCode;                // index for key codes
   BYTE         bEditor;
   PRESKEYTABLE pResKey;               // ptr for processing of reserved keys

   bEditor = (pDoc->hwndRichEdit) ? EDIT_RTF : EDIT_STANDARD;

   //--- delete the mapping table ---
   memset( ausMapKey, 0, sizeof(ausMapKey) );

   //--- set character keys ---
   for ( usState = 0; usState <= 1; usState++)
   {
      for ( usCode = 0; usCode <= 255; usCode++ )
      {
         ausMapKey[usState][usCode] = CHARACTER_FUNC + 1;
      } /* endfor */
   } /* endfor */

   /*******************************************************************/
   /* if bEditor of KEylist is EDIT_NONE, keysetting should not be    */
   /* used at all!                                                    */
   /* KEysetting should be used only if valid for current editor!     */
   /*******************************************************************/
   //--- set user assignable keys ---
   while ( pKeyList->Function != LAST_FUNC )
   {
      if ( (pKeyList->ucCode || pKeyList->ucState )
         && (bEditor & (pKeyList->bEditor)) )
      {
         ausMapKey[pKeyList->ucState][pKeyList->ucCode] =
            pKeyList->Function + 1;
      } /* endif */
      pKeyList++;
   } /* endwhile */

   //--- set reserved keys ---
   pResKey = get_ResKeyTab();
   while ( pResKey->ucCode || pResKey->ucState )
   {
     if (pResKey->usAssignStatus != ASSIGNED_TO_OTHER )
     {
       ausMapKey[pResKey->ucState][pResKey->ucCode] = pResKey->Function + 1;
     } /* endif */
      pResKey++;
   } /* endwhile */

   // Add for R012027 start
   SPECCHARKEYVEC* pSCKeyTable = GetSpecCharKeyVec();
   for (size_t iInx = 0; iInx < (*pSCKeyTable).size(); iInx++)
   {
       ausMapKey[(*pSCKeyTable)[iInx].ucState][(*pSCKeyTable)[iInx].ucCode] = LAST_FUNC + 1;
   }
   // Add end

   return;
} /* EQFBAdaptKeyTable */

/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBMapKey - EQF Translation Browser - Map WM_CHAR message to a function
*/
// Description:
//    Map a WM_CHAR message to a function ID.
//
// Flow:
//    xxxx
//
// Arguments:
//    MPARAM    mp1                 - first parameter of WM_CHAR message
//    MPARAM    mp2                 - second parameter of WM_CHAR message
//    PUSHORT   pusFunction         - ptr to variable receiving the function ID
//    PUCHAR    pucState            - status of keyboard flags
//
// Returns:
//    BOOL fMapped   TRUE           - key has been mapped to a function
//                   FALSE          - no function can be mapped to the key
//
// Prereqs:
//   ausMapKey must contain a keyboard mapping table
//
// SideEffects:
//   None.
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
   /*******************************************************************/
   /* Arabic currently contains OS/2 settings ....                    */
   /*******************************************************************/
   static UCHAR ArbHKMap[256] = {
   0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
   0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
   0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
   0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
   0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
   0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
   0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
   0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
   0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
   0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x47,0x00,0x9B,0x9C,0x62,0x00,0x00,
   0xA0,0xA1,0x00,0xA3,0xA4,0x00,0xA6,0xA7,0x00,0x00,0x00,0x00,0x4B,0x00,0x00,0x00,
   0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x00,0x50,0x00,0x00,0x00,0x3F,
   0xC0,0x78,0x4E,0x48,0x63,0x00,0x7A,0x68,0x66,0x6D,0x6A,0x65,0x5B,0x70,0x6F,0x5D,
   0x60,0x76,0x2E,0x73,0x61,0x77,0x71,0x27,0x2F,0x75,0x79,0x00,0x00,0x49,0x4F,0x00,
   0x5C,0x74,0x72,0x3B,0x67,0x6C,0x6B,0x69,0x2C,0x6E,0x64,0x00,0x00,0x00,0x00,0x00,
   0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x00,0x00,0x00,0x00,0xFE,0xFF
   };
   static UCHAR ArbUnicodeHKMap[256] = {
      0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
      0x20,0x78,0x22,0x23,0x63,0x25,0x7A,0x68,0x66,0x6D,0x6A,0x65,0x2C,0x70,0x6F,0x2F,
      0x30,0x76,0x32,0x73,0x61,0x77,0x71,0x37,0x38,0x75,0x79,0x3B,0x3C,0x3D,0x3E,0x3F,
      0x40,0x74,0x72,0x43,0x67,0x6C,0x6B,0x69,0x48,0x6E,0x64,0x4B,0x4C,0x4D,0x4E,0x4F,
      0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
      0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
      0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
      0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
      0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
      0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
      0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
      0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
      0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
      0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
      0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
   };
   /*******************************************************************/
   /* Hebrew adopted to Codepage 1255                                 */
   /*******************************************************************/
   static UCHAR HebHKMap[256] = {
   0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
   0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
   0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27/*0x77*/,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F/*0x71*/,
   0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
   0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
   0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
   0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
   0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
   0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
   0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
   0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
   0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
   0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
   0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
   0x74,0x63,0x64,0x73,0x76,0x75,0x7A,0x6A,0x79,0x68,0x6C,0x66,0x6B,0x6F,0x6E,0x69,
   0x62,0x78,0x67,0xF3,0x70,0xF5,0x6D,0x65,0x72,0x61,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
   };
   /*******************************************************************/
   /* Hebrew adopted to Codepage 1255 - for Unicode implementation    */
   /*******************************************************************/
   static UCHAR HebUnicodeHKMap[256] = {
   0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
   0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
   0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27/*0x77*/,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F/*0x71*/,
   0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
   0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
   0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
   0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
   0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
   0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
   0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
   0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
   0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
   0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
   0x74,0x63,0x64,0x73,0x76,0x75,0x7A,0x6A,0x79,0x68,0x6C,0x66,0x6B,0x6F,0x6E,0x69,
   0x62,0x78,0x67,0xE3,0x70,0xE5,0x6D,0x65,0x72,0x61,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
   0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
   };

BOOL EQFBMapKey
(
   WINMSG    usMsg,                    // message parameter
   WPARAM    mp1,                      // first parameter of WM_CHAR message
   LPARAM    mp2,                      // second parameter of WM_CHAR message
   PUSHORT   pusFunction,              // ptr to var receiving the function ID
   PUCHAR    pucState,                 // status of keyboard flags
   USHORT    usMapKeyID                // identifier for mapkeytable
)
{
   BOOL fMapped = FALSE;               // key has been mapped flag
   UCHAR ucCode = 0;                   // key code
   USHORT usCode;                      // key code
   UCHAR ucState = 0;                  // key shift state

   usMsg;                              // avoid compiler warnings

#ifdef _TRC_DEBUG
{
  FILE *fOut = fopen( "\\keyb.out", "a" );
  fprintf( fOut, " %4.4d %4.4d %5.5d %5.5d\n", usMsg, mp1, LOWORD(mp2), HIWORD(mp2));
  fprintf( fOut, " %d\n",  GetKBCodePage());
  fclose( fOut );

}
#endif

   /*******************************************************************/
   /* for windows we will only pass key down-messages to conv key...  */
   /*******************************************************************/
   switch ( usMsg )
   {
     case WM_KEYDOWN:
       /********************************************************************/
       /* ignore CTRL key if it comes from an extended key...              */
       /********************************************************************/
           //{
           //  HKL hkl = GetKeyboardLayout( 0 );
           //  BYTE kb[256];

           //  GetKeyboardState(kb);
           //  WCHAR uc[5] = {};

           //  int i = 0;
           //  int iLen = ToUnicodeEx( mp1, MapVirtualKey( mp1, MAPVK_VK_TO_VSC), kb, uc, 4, 0, hkl );
           //  if ( iLen == 1 )
           //  {
           //    WCHAR ucKey = uc[0];
           //  }
           //}

       if ( mp2 & 0x020000000 )
       {
         ucState = 0;
       }
       else
       {
         fMapped = EQFBKeyState( (USHORT)mp1, &ucCode, &ucState );
       } /* endif */
       break;
     case WM_SYSKEYDOWN:
       /***************************************************************/
       /* for some reasons F10 arrives as a SYSKEY                    */
       /***************************************************************/
       if (!(mp2 & 0x20000000) )
       {
         fMapped = EQFBKeyState( (USHORT)mp1, &ucCode, &ucState );
       }
       else
       {
         fMapped = EQFBKeyState( (USHORT)mp1, &ucCode, &ucState );
         ucState |= ST_ALT;
       } /* endif */
       break;
     case WM_CHAR:
       ucCode = (UCHAR) LOWORD( mp1 );
       usCode = LOWORD( mp1 );
       ucState = 0;
       fMapped = TRUE;
       if ( (usCode == VK_ENTER )  ||
            (usCode == VK_TAB )    ||
            (usCode == VK_ESC )    ||
            (usCode == VK_BACKSPACE) )
       {
         ucState |= ST_VK;
         if ( (ucCode == VK_TAB) && (GetKeyState(VK_SHIFT) & 0x8000) )
         {
           /**********************************************************/
           /* ignore shift status and set backtab                    */
           /**********************************************************/
           ucCode = VK_BACKTAB;
           ucState  &= ~ST_SHIFT;
         } /* endif */
       } /* endif*/
       /****************************************************************/
       /* refine for dictionary proposal copy...and ctrl-backspace     */
       /* make sure that key is not picked up twice, as character to be*/
       /* typed in also!!                                              */
       /****************************************************************/
       if ( GetKeyState (VK_CTRL) & 0x8000 )
       {
         if ( (usCode <= 'Z' - 'A'+ 1) || (usCode == 127)
               || (usCode == 28) || (usCode == 29) )
//         if ( (ucCode <= 'Z' - 'A'+ 1) || (ucCode == 127)
//              || (ucCode == 28) || (ucCode == 29) )
         {
           fMapped = FALSE;
         } /* endif */
       } /*endif*/
       break;

     case WM_SYSCHAR:
       if ( mp2 & 0x20000000 )
       {
         fMapped = TRUE;
         ucState = ST_ALT;
         ucCode = (UCHAR) LOWORD( mp1 );

         if ( (ucCode == VK_ENTER )  ||
              (ucCode == VK_TAB )    ||
              (ucCode == VK_ESC )    ||
              (ucCode == VK_BACKSPACE) )
         {
           ucState |= ST_VK;
           if ( (ucCode == VK_TAB) && (GetKeyState(VK_SHIFT) & 0x8000) )
           {
             /**********************************************************/
             /* ignore shift status and set backtab                    */
             /**********************************************************/
             ucCode = VK_BACKTAB;
             ucState  &= ~ST_SHIFT;
           } /* endif */
         } /* endif*/
       } /* endif */
       break;
     case WM_SYSDEADCHAR:
     case WM_DEADCHAR:
       ucState = ucCode = 0;
       break;
     default:
       MessageBeep( (UINT) -1 );
       ucState = ucCode = 0;
       break;
   } /* endswitch */
   /***************************************************************/
   /* add mapping from Hebrew/Arabic characters                   */
   /***************************************************************/
   if ( (ucState != 0) && (ucState & ~(ST_VK|ST_CTRL) ) )
   {
     if (NLSLanguage & NLS_PROCESS_ARABIC)
     {
        ucCode = ArbUnicodeHKMap[ucCode];
     }
     else if (NLSLanguage & NLS_PROCESS_HEBREW)
     {
       ucCode = HebUnicodeHKMap[ucCode];
     }
   } /* endif */


   if ( fMapped )
   {
     if ( usMapKeyID == ITM_MAPKEY )
     {
       if ( ausITMMapKey[ucState][ucCode] )
       {
          // get function ID (Note: function IDs are zero based!)
          *pusFunction = ausITMMapKey[ucState][ucCode] - 1;
          *pucState    = ucState;
       }
       else
       {
          fMapped = FALSE;
       } /* endif */
     }
     else
     {
       if ( ausMapKey[ucState][ucCode] )
       {
          // get function ID (Note: function IDs are zero based!)
          *pusFunction = ausMapKey[ucState][ucCode] - 1;
          *pucState    = ucState;
       }
       else
       {
          fMapped = FALSE;
       } /* endif */
     } /* endif */
   } /* endif */

   return( fMapped );
} /* EQFBMapKey */


/**********************************************************************/
/* determine the status of the keyboard flags..                       */
/**********************************************************************/
BOOL EQFBKeyState
(
  USHORT  usKey,
  PUCHAR  pucCode,
  PUCHAR  pucState
)
{
  BOOL fMapped = FALSE;
  UCHAR  ucCode = (UCHAR) LOWORD( usKey );
  UCHAR  ucState = 0;

  /********************************************************************/
  /* get keyboard flags..                                             */
  /********************************************************************/
  if ( GetKeyState(VK_SHIFT) & 0x8000 )
  {
    ucState |= ST_SHIFT;
  } /*endif*/

  if ( GetKeyState (VK_CTRL) & 0x8000 )
  {
    ucState |= ST_CTRL;
  } /*endif*/

  switch ( usKey )
  {
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
    case VK_F1:
    case VK_F2:
    case VK_F3:
    case VK_F4:
    case VK_F5:
    case VK_F6:
    case VK_F7:
    case VK_F8:
    case VK_F9:
    case VK_F10:
    case VK_F11:
    case VK_F12:
    case VK_INSERT:
    case VK_DELETE:
    case VK_SCROLL:
    case VK_PAUSE:
//  case VK_ENTER:
//  case VK_TAB:
//  case VK_BACKSPACE:
      fMapped = TRUE;
      ucState |= ST_VK;
      break;
    case VK_CTRL:
    case VK_SHIFT:
    case VK_ALT:
      fMapped = FALSE;
      break;
    case VK_NUMPAD0  :                                   /* @KWT0010A */
    case VK_NUMPAD1  :                                   /* @KWT0010A */
    case VK_NUMPAD2  :                                   /* @KWT0010A */
    case VK_NUMPAD3  :                                   /* @KWT0010A */
    case VK_NUMPAD4  :                                   /* @KWT0010A */
    case VK_NUMPAD5  :                                   /* @KWT0010A */
    case VK_NUMPAD6  :                                   /* @KWT0010A */
    case VK_NUMPAD7  :                                   /* @KWT0010A */
    case VK_NUMPAD8  :                                   /* @KWT0010A */
    case VK_NUMPAD9  :                                   /* @KWT0010A */
      if (ucState == ST_CTRL)                            /* @KWT0010A */
      {                                                  /* @KWT0010A */
        fMapped = TRUE;                                  /* @KWT0010A */
        //adjust virtual key codes                       /* @KWT0010A */
        ucCode = ucCode - VK_NUMPAD0 + '0';              /* @KWT0010A */
      } /* endif */                                      /* @KWT0010A */
      break;                                             /* @KWT0010A */
    default:
      if ( ucState & ST_CTRL)
      {
        HKL  hKL = GetKeyboardLayout(0);
        USHORT usLangId = LOWORD(hKL) & 0x00ff;
        UCHAR  ucCodeOrg = ucCode;

        fMapped = TRUE;
        ucCode = (UCHAR) MapVirtualKey((USHORT)ucCode, 2);

        /**************************************************************/
        /* if we have Russian, we have another arrangement for CTRL-0 */
        /* to CTRL-9 -- we map it to our defaults to allow for coping */
        /* with dictionary and translation memory                     */
        /**************************************************************/
        switch ( usLangId )
        {
          case LANG_RUSSIAN:
            switch ( ucCode )
            {
              case 41:            // Ctrl-0
                ucCode = 48;
                break;
              case 33:            // Ctrl-1
                ucCode = 49;
                break;
              case 34:            // Ctrl-2
                ucCode = 50;
                break;
              case 185:           // Ctrl-3
                ucCode = 51;
                break;
              default:
                break;
            } /* endswitch */
            break;
          case LANG_FRENCH:       // French Keyboard
            switch ( ucCodeOrg )  // Disable mapping via MapVirtualKey
            {
              case 48:            // Ctrl-0
              case 49:            // Ctrl-1
              case 50:            // Ctrl-2
              case 51:            // Ctrl-3
              case 52:            // Ctrl-4
              case 53:            // Ctrl-5
              case 54:            // Ctrl-6
              case 55:            // Ctrl-7
              case 56:            // Ctrl-8
              case 57:            // Ctrl-9
                ucCode = ucCodeOrg;
                break;
              default:
                break;
            } /* endswitch */
            break;

          default:
          break;
        }

        if ( (usKey == VK_ENTER )  ||
             (usKey == VK_TAB )    ||
             (usKey == VK_ESC )    ||
             (usKey == VK_BACKSPACE) )
        {
          fMapped = TRUE;
          ucState |= ST_VK;
          if ( (usKey == VK_TAB) && (ucState & ST_SHIFT) )
          {
            /**********************************************************/
            /* ignore shift status and set backtab                    */
            /**********************************************************/
            *pucCode = VK_BACKTAB;
            ucState  &= ~ST_SHIFT;
          } /* endif */
        } /* endif*/
      }
      /****************************************************************/
      /* refine for dictionary proposal copy...                       */
      /****************************************************************/
      if ((ucState == ST_CTRL) && ('A' <= ucCode )
           && (ucCode <= 'Z' ))
      {
        //adjust virtual key codes because lower a-z are in
        //reserved key list
        ucCode = ucCode + 'a' - 'A';
        fMapped = TRUE;
      } /* endif */
      if ((ucState == ST_CTRL) && ('0' <= ucCode ) && (ucCode <= '9' ))
      {
        fMapped = TRUE;
      } /* endif */
      break;
  } /* endswitch */
  /********************************************************************/
  /* set the key status flags                                         */
  /********************************************************************/
  *pucCode = ucCode;
  *pucState = ucState;
  return fMapped;
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBConvKey
//------------------------------------------------------------------------------
// Function call:     EQFBConvKey(wParam, lParam, &ucCode, &ucState)
//------------------------------------------------------------------------------
// Description:       converts wm_char and wm_keydown into key code & status
//------------------------------------------------------------------------------
// Parameters:        WORD       wParam      - 1st par. of WM_CHAR + KEYDOWN
//                    LONG       lParam      - 2nd par. of WM_CHAR + KEYDOWN
//                    PUCHAR     pucCode     - ptr to var receiving key code
//                    PUCHAR     pucState    - ptr to var receiving key state
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE - key has been converted
//                    FALSE - no conversion, message should fall through
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------
BOOL
EQFBConvKey
(
   WPARAM   wParam,                        // 1st par. of WM_CHAR
   PUCHAR   pucCode,                       // ptr to var receiving key code
   PUCHAR   pucState                       // ptr to var receiving key state
)

{
  BOOL fConverted = TRUE;

  *pucState = 0;
  *pucCode = (UCHAR) wParam;

  if ( GetKeyState(VK_SHIFT) & 0x8000 )
  {
    *pucState |= ST_SHIFT;
  } /*endif*/

  if ( GetKeyState (VK_ALT) & 0x8000 )
  {
    *pucState |= ST_ALT;
  } /*endif*/

  if ( GetKeyState (VK_CTRL) & 0x8000 )
  {
    *pucState |= ST_CTRL;
  } /*endif*/

  /********************************************************************/
  /* ignore Shift sstate for backtab                                  */
  /********************************************************************/
  if ((*pucCode == VK_TAB) && (*pucState == ST_SHIFT ))
  {
     *pucCode = VK_BACKTAB;
     // delete ST_SHIFT in pucState
     *pucState = 0;
  } /* endif */

  return (fConverted);
} /* end of function EQFBConvKey */

/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBKeyName - EQF Translation Browser - Build a key name
*/
// Description:
//    Convert a key state and key code pair into a key name.
//
// Flow:
//    xxxx
//
// Arguments:
//    PSZ       pszBuffer           - ptr to buffer for key name
//    UCHAR     ucCode              - character code
//    UCHAR     ucState             - character shift state
//
// Returns:
//    BOOL fOK        TRUE          - key name is in buffer
//                    FALSE         - key name cannot be generated
//
// Prereqs:
//   None.
//
// SideEffects:
//   It is assumed, that the first four keys are in the following order:
//          CHAR key,                -1       },
//          "SHIFT",                 VK_SHIFT },
//          "CTRL",                  VK_CTRL },
//          "ALT",                   VK_ALT },
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
BOOL EQFBKeyName
(
   PSZ       pszBuffer,                // ptr to buffer for key name
   UCHAR     ucCode,                   // character code
   UCHAR     ucState                   // character shift state
)
{
   PVKNAME  pVK;                       // ptr for virtual key table processing
   HAB      hab;
   PSZ      pTable[4];                 // table to hold pointers to strings
   ULONG    ulLen;
   CHAR     chChar[2];                 // character typed
   PSZ      *ppInsMsg;
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   *pszBuffer = EOS;                   // clear buffer
   hab = WinQueryAnchorBlock( QUERYACTIVEWINDOW() );


   // if resource not already loaded, do so.....
   pVK = VKTable;
   if ( ! *pVK->szName  )
   {
      ulLen = 0;
      while ( pVK->ucValue )
      {
          WinLoadString( hab, hResMod, IDS_TB_VK_CHAR+ ulLen,
                         MAX_VKEYNAME, pVK->szName );

          pVK++;
          ulLen ++;
      } /* endwhile */
   } /* endif */


   if ( ucCode || ucState )
   {
      for ( ulLen = 0 ; ulLen < 4 ; ulLen ++)
      {
        pTable[ ulLen ] = EMPTY_STRING;
      } /* endfor */
      ppInsMsg = &(pTable[0]);

      if ( ucState & ST_SHIFT )
      {
         pTable[0] = VKTable[1].szName;
      } /* endif */

      if ( ucState & ST_CTRL )
      {
         pTable[1] = VKTable[2].szName;
      } /* endif */

      if ( ucState & ST_ALT )
      {
         pTable[2] = VKTable[3].szName;
      } /* endif */

      // append name of key
      if ( ucState & ST_VK  )
      {
         // handle virtual keys
         pVK = VKTable;
         while ( pVK->ucValue &&
                 (pVK->ucValue != ucCode) )
         {
            pVK++;
         } /* endwhile */
         ppInsMsg = &(pTable[0]);

         DosInsMessage( &(pTable[0]), 3, pVK->szName,MAX_VKEYNAME,
                        pszBuffer, 2 * MAX_VKEYNAME , &ulLen );
         *(pszBuffer+ulLen ) = EOS;
      }
      else
      {
         // handle character keys
         chChar[0] = ucCode;
         chChar[1] = EOS;
         pTable[3] = chChar;

         DosInsMessage( &(pTable[0]), 4, VKTable[0].szName, MAX_VKEYNAME,
                        pszBuffer, 2 * MAX_VKEYNAME , &ulLen );
         *(pszBuffer+ulLen ) = EOS;
      } /* endif */
   }
   else
   {
      WinLoadString( hab, hResMod, IDS_TB_VK_NONE,
                     MAX_VKEYNAME, pszBuffer);
   } /* endif */
   return( TRUE );

} /* end of EQFBKeyName */

// Add for R012027 start
void EQFBKeyNameW
(
   wchar_t * pwszKeyName,                // ptr to buffer for key name
   UCHAR     ucCode,                     // character code
   UCHAR     ucState                     // character shift state
)
{
    PVKNAME  pVK;                       // ptr for virtual key table processing
    HAB      hab;
    PSZ      pTable[4];                 // table to hold pointers to strings
    ULONG    ulLen;
    CHAR     chChar[2];                 // character typed
    PSZ      *ppInsMsg;
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    char strKeyName[MAX_BUF_SIZE];
    memset(strKeyName, 0x00, sizeof(strKeyName));

    hab = WinQueryAnchorBlock(QUERYACTIVEWINDOW());

    // if resource not already loaded, do so.....
    pVK = VKTable;
    if (!*pVK->szName)
    {
        ulLen = 0;
        while ( pVK->ucValue )
        {
            WinLoadString(hab, hResMod, IDS_TB_VK_CHAR+ ulLen, MAX_VKEYNAME, pVK->szName);
            pVK++;
            ulLen ++;
        }
    }

    if (ucCode || ucState)
    {
        for (ulLen = 0 ; ulLen < 4 ; ulLen ++)
        {
            pTable[ ulLen ] = EMPTY_STRING;
        }
        ppInsMsg = &(pTable[0]);

        if (ucState & ST_SHIFT)
        {
            pTable[0] = VKTable[1].szName;
        }

        if (ucState & ST_CTRL)
        {
            pTable[1] = VKTable[2].szName;
        }

        if (ucState & ST_ALT)
        {
            pTable[2] = VKTable[3].szName;
        }

        // append name of key
        if (ucState & ST_VK)
        {
            // handle virtual keys
            pVK = VKTable;
            while (pVK->ucValue && (pVK->ucValue != ucCode))
            {
                pVK++;
            }
            ppInsMsg = &(pTable[0]);

            DosInsMessage( &(pTable[0]), 3, pVK->szName,MAX_VKEYNAME, strKeyName, 2 * MAX_VKEYNAME , &ulLen);
            *(strKeyName+ulLen ) = EOS;
        }
        else
        {
            // handle character keys
            chChar[0] = ucCode;
            chChar[1] = EOS;
            pTable[3] = chChar;

            DosInsMessage(&(pTable[0]), 4, VKTable[0].szName, MAX_VKEYNAME, strKeyName, 2 * MAX_VKEYNAME , &ulLen);
            *(strKeyName+ulLen ) = EOS;
        }
    }
    else
    {
        WinLoadString( hab, hResMod, IDS_TB_VK_NONE, MAX_VKEYNAME, strKeyName);
    }

    memset(pwszKeyName, 0x00, MAX_BUF_SIZE);
    int nLen = MultiByteToWideChar(CP_ACP, 0, strKeyName, strlen(strKeyName), pwszKeyName, MAX_BUF_SIZE);
    pwszKeyName[nLen] = L'\0';
}
// Add end

/*////////////////////////////////////////////////////////////////////////////
:h3.EQFBSetKeyName - EQF Translation Browser - Set key name of menu item
*/
// Description:
//    Set the name of the key associated to a specific function in the
//    menu item for this function.
//
// Flow:
//    xxxx
//
// Arguments:
//    HWND      hwndMenu            - handle of menu
//    SHORT     sItem               - ID of menu item
//    FUNCTION  Function            - ID of associated function
//
// Returns:
//    Nothing
//
// Prereqs:
//   None.
//
// SideEffects:
//   None.
//
// External references:
//   None
//
//////////////////////////////////////////////////////////////////////////////
VOID EQFBSetKeyName
(
   HMENU    hwndMenu,                  // handle of menu
   SHORT    sItem,                     // ID of menu item
   FUNCTION Function                   // id of function processing the item
)
{
   PKEYPROFTABLE pKeyEntry;            // pointer for key table processing
   BOOL          fKeyFound = FALSE;    // TRUE = key for function found
   PSZ           pszTemp;              // temporary pointer to locate tab char

   // get key assigned to function
   pKeyEntry = get_KeyTable();
   while ( (pKeyEntry->Function != LAST_FUNC) && !fKeyFound )
   {
      if ( pKeyEntry->Function == Function )
      {
         fKeyFound = TRUE;
      }
      else
      {
         pKeyEntry++;
      } /* endif */
   } /* endwhile */

   // get current menu item text
   if ( fKeyFound )
   {
     GETMENUTEXT( hwndMenu, sItem, szBuffer );
   } /* endif */

   // discard old key name
   if ( fKeyFound )
   {
      pszTemp = strchr( szBuffer, '\t' );
      if ( pszTemp )
      {
         *pszTemp = EOS;
      } /* endif */
   } /* endif */

   // append new key name
   if ( fKeyFound )
   {
      if ( pKeyEntry->ucCode || pKeyEntry->ucState )
      {
         strcat( szBuffer, "\t" );
         EQFBKeyName( szBuffer + strlen(szBuffer),
                      pKeyEntry->ucCode,
                      pKeyEntry->ucState );
      } /* endif */
   } /* endif */

   // set menu item text
   if ( fKeyFound )
   {
     SETMENUTEXT( hwndMenu, sItem, szBuffer );
   } /* endif */

} /* end of EQFBSetKeyName */


//------------------------------------------------------------------------------
// Function name: EQFBLoadResource
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBLoadResource()
//
//------------------------------------------------------------------------------
// Description:       preload the function key names
//
//------------------------------------------------------------------------------
// Parameter:         VOID
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Prerequesits:      _
//
//------------------------------------------------------------------------------
// Side effects:      VKTable will contain the loaded strings
//
//------------------------------------------------------------------------------
// Function flow:     get anchor block handle
//                    loop through list of all virtual keys and load the string
//                    get function key names for all functions
//------------------------------------------------------------------------------

VOID EQFBLoadResource ()
{
   PVKNAME  pVK;                       // ptr for virtual key table processing
   HAB      hab;
   USHORT   usLen;
   USHORT   usFuncId;                  // function ID
   PSZ         pszFuncName;            // pointer to function name
   PKEYPROFTABLE pKeyTemp;             // temp pointer
   USHORT        usCount = 0;
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   hab = WinQueryAnchorBlock( QUERYACTIVEWINDOW() );

   /*******************************************************************/
   /* load virtual key names if not already done                      */
   /*******************************************************************/
   pVK = VKTable;
   usLen = 0;

   if ( ! pVK->szName[0] )
   {
     while ( pVK->ucValue )
     {
         WinLoadString( hab, hResMod, IDS_TB_VK_CHAR+ usLen,
                        MAX_VKEYNAME, pVK->szName );
         pVK++;
         usLen ++;
     } /* endwhile */

     /*******************************************************************/
     /* load function key names                                         */
     /*******************************************************************/
     pKeyTemp = get_KeyTable();                // pointer to keytable
     usCount = 0;
     usFuncId = pKeyTemp->Function;
     // check if resource already loaded, if not load it
     // while not end of key list
     while ( (usFuncId <= LAST_FUNC) && (usCount <= LAST_FUNC + 1) )
     {
        pszFuncName = (get_FuncTab() + usFuncId)->szDescription;
        // load name for function types
        WinLoadString( hab, hResMod, ID_TB_KEYS_DLG+usFuncId,
                       MAX_FUNCDESCRIPTION, pszFuncName);
        usCount ++;
        usFuncId = (++pKeyTemp)->Function;
     }
   } /* endif */
} /* end of function EQFBLoadResource */



/*//////////////////////////////////////////////////////////////
:h3.EQFBInitMenu ( PTBDOCUMENT,HWND,USHORT)
*///////////////////////////////////////////////////////////////
// Description:
// Flow:
//    - get current segment
//    - if automatic translation is processing stop it
//    - set pulldown items according to document type and current status
//    - include the current keynames in the pulldown items.
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
//  USHORT        - message parameter 1 to get selected id
//
////////////////////////////////////////////////////////////////
VOID EQFBInitMenu
(
  PTBDOCUMENT pDoc,                                // pointer to doc ida
  USHORT      usmp1                                // message parameter
)
{
   HMENU hwndMenu;                                 // handle of aab in windows
   PTBSEGMENT  pSeg;                               // pointer to active seg
   BOOL        fActive;                            // action active
   ULONG       ulSegNum;                           // segment number
   EQFBBLOCK* pEQFBBlockMark = get_EQFBBlockMark();
   PTBDOCUMENT pDocTarget;                         // pointer to target doc
   /*******************************************************************/
   /* Attention: AAB now anchored at TWB main window....              */
   /*******************************************************************/

   hwndMenu = GetMenu( (HWND) UtlQueryULong( QL_TWBFRAME ));
   /*******************************************************************/
   /* Attention: If the window is maximized, the zero based position  */
   /* in the action bar is one place off (the system menu is counted).*/
   /* Therefore: adjust it if necessary....                           */
   /*******************************************************************/
   if ( IsZoomed( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS) )
   {
     usmp1--;
   } /* endif */

   pSeg =  EQFBGetSegW( pDoc,
                       pDoc->TBCursor.ulSegNum );  // get current segment

   if ( pDoc->EQFBFlags.AutoMode )
   {
      EQFBFuncAutoStop( pDoc );                    // reset automatic mode
   } /* endif */

   switch (usmp1)
   {
      case IDM_TRANS_MENU:
        // correct key names displayed in menu
        EQFBSetKeyName( hwndMenu, IDM_TRANSSEG, TSEG_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_TRANSNEW, TSEGNEXT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_TRANSNEW_EXACT,  TSEGNEXT_EXACT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_TRANSNEW_FUZZY,  TSEGNEXT_FUZZY_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_TRANSNEW_NONE,   TSEGNEXT_NONE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_TRANSNEW_MT,     TSEGNEXT_MT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_TRANSNEW_GLOBAL, TSEGNEXT_GLOBAL_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_DICTLOOK, DICTLOOK_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_EDITTERM, EDITTERM_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_ADDABBREV,ADDABBREV_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_GOTO, GOTOSEG_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_MARKSEG, MARKSEG_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_GOTOMARK, GOTOMARK_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_CLEARMARK, CLEARSEGMARK_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_JOINSEG, JOINSEG_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_SPLITSEG, SPLITSEG_FUNC  );
        EQFBSetKeyName( hwndMenu, IDM_POSTEDIT, POSTEDIT_FUNC  );
        EQFBSetKeyName( hwndMenu, IDM_AUTOTRANS, AUTOTRANS_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_SHOWTRANS, SHOWTRANS_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_UNTRANS, UNTRANS_FUNC );
        /**************************************************************/
        /* enable/disable depending on available user exit            */
        /**************************************************************/
        SETAABITEM( hwndMenu, IDM_SHOWTRANS, pDoc->pfnShowTrans );

        switch (pDoc->docType)
        {
           case STARGET_DOC:
             {
               PLOADEDTABLE pTable = (PLOADEDTABLE)pDoc->pDocTagTable;
               BOOL fXLIFF = (strcmp( pTable->szName, "EQFXLIFF" ) == 0 );

              SETAABITEM( hwndMenu, IDM_TRANSNEW, ! pDoc->fXlated);
              SETAABITEM( hwndMenu, IDM_TRANSNEW_EXACT,  ! pDoc->fXlated);
              SETAABITEM( hwndMenu, IDM_TRANSNEW_FUZZY,  ! pDoc->fXlated);
              SETAABITEM( hwndMenu, IDM_TRANSNEW_NONE,   ! pDoc->fXlated);
              SETAABITEM( hwndMenu, IDM_TRANSNEW_MT,     ! pDoc->fXlated);
              SETAABITEM( hwndMenu, IDM_TRANSNEW_GLOBAL, ! pDoc->fXlated);

                                              // Disable Goto Mark if no mark availab.

              SETAABITEM( hwndMenu, IDM_GOTOMARK, pDoc->EQFBFlags.MarkedSeg );
                                              // disable clear mark if no mark is ava
              SETAABITEM( hwndMenu, IDM_CLEARMARK, pSeg->SegFlags.Marked );
                                              // disable split if not joined
              fActive = ((! pDoc->EQFBFlags.PostEdit)
                            && pSeg->SegFlags.JoinStart );
              SETAABITEM( hwndMenu, IDM_SPLITSEG, fActive  );
              // disable join if at end of file (no next segment)
              ulSegNum = pSeg->ulSegNum + 1;
              fActive = ( EQFBGetVisSeg( pDoc->twin, &ulSegNum )
                          && (pDoc->TBCursor.ulSegNum == pDoc->ulWorkSeg)
                          && pDoc->EQFBFlags.Reflow
                          && ! pDoc->EQFBFlags.PostEdit
                          && !fXLIFF  ) ;
              SETAABITEM( hwndMenu, IDM_JOINSEG, fActive );
                  // enable post edit if not automode and vice versa
              SETAABITEM( hwndMenu, IDM_POSTEDIT, ! pDoc->EQFBFlags.PostEdit );
              SETAABITEM( hwndMenu, IDM_AUTOTRANS, ! pDoc->EQFBFlags.PostEdit);
                // goto active segment only if not post edit or automatic mode
              SETAABITEM( hwndMenu, IDM_GOTO, ! pDoc->EQFBFlags.PostEdit);
              // allow untranslate segment only if in active segment and no auto
              fActive = (pDoc->TBCursor.ulSegNum == pDoc->ulWorkSeg) &&
                          ! pDoc->EQFBFlags.PostEdit &&
                          (pDoc->tbActSeg.qStatus == QF_XLATED);
              SETAABITEM( hwndMenu, IDM_UNTRANS, fActive );
             }
             break;
           case SSOURCE_DOC:
                        // Disable all AAB items except GOTO_Act Segment
                        // and dictionary lookup
             // goto active segment only if not post edit or automatic mode
             pDocTarget = pDoc;
             while ( pDocTarget->docType != STARGET_DOC )
             {
                pDocTarget = pDocTarget->next;
             } /* endwhile */
             SETAABITEM( hwndMenu, IDM_GOTO, ! pDocTarget->EQFBFlags.PostEdit);
             SETAABITEM( hwndMenu, IDM_TRANSSEG, FALSE );
             SETAABITEM( hwndMenu, IDM_TRANSNEW, FALSE );
             SETAABITEM( hwndMenu, IDM_TRANSNEW_EXACT, FALSE );
             SETAABITEM( hwndMenu, IDM_TRANSNEW_FUZZY, FALSE );
             SETAABITEM( hwndMenu, IDM_TRANSNEW_NONE, FALSE );
             SETAABITEM( hwndMenu, IDM_MARKSEG, FALSE );
             SETAABITEM( hwndMenu, IDM_GOTOMARK, FALSE );
             SETAABITEM( hwndMenu, IDM_CLEARMARK, FALSE );
             SETAABITEM( hwndMenu, IDM_JOINSEG, FALSE );
             SETAABITEM( hwndMenu, IDM_SPLITSEG, FALSE  );
             SETAABITEM( hwndMenu, IDM_POSTEDIT, FALSE  );
             SETAABITEM( hwndMenu, IDM_AUTOTRANS, FALSE );
             SETAABITEM( hwndMenu, IDM_UNTRANS, FALSE );
             break;
           case OTHER_DOC:
           case TRNOTE_DOC:
                        // Disable all AAB items except
                        // and dictionary lookup
             SETAABITEM( hwndMenu, IDM_GOTO, FALSE );
             SETAABITEM( hwndMenu, IDM_TRANSSEG, FALSE );
             SETAABITEM( hwndMenu, IDM_TRANSNEW, FALSE );
             SETAABITEM( hwndMenu, IDM_TRANSNEW_EXACT, FALSE );
             SETAABITEM( hwndMenu, IDM_TRANSNEW_FUZZY, FALSE );
             SETAABITEM( hwndMenu, IDM_TRANSNEW_NONE, FALSE );
             SETAABITEM( hwndMenu, IDM_MARKSEG, FALSE );
             SETAABITEM( hwndMenu, IDM_GOTOMARK, FALSE );
             SETAABITEM( hwndMenu, IDM_CLEARMARK, FALSE );
             SETAABITEM( hwndMenu, IDM_JOINSEG, FALSE );
             SETAABITEM( hwndMenu, IDM_SPLITSEG, FALSE  );
             SETAABITEM( hwndMenu, IDM_POSTEDIT, FALSE  );
             SETAABITEM( hwndMenu, IDM_AUTOTRANS, FALSE );
             SETAABITEM( hwndMenu, IDM_SHOWTRANS, FALSE );
             SETAABITEM( hwndMenu, IDM_UNTRANS, FALSE );
             break;
        } /* endswitch */

        /**************************************************************/
        /* activate dictionary lookup only if dictionary is attached  */
        /* -- this has to be changed if we support the system dicts.. */
        /**************************************************************/
        SETAABITEM( hwndMenu, IDM_DICTLOOK, ! EQFGETDICT(EQF_IS_AVAIL,NULL));
        SETAABITEM( hwndMenu, IDM_EDITTERM, ! EQFGETDICT(EQF_IS_AVAIL,NULL));

        break;
      case IDM_WINDOW_MENU:
        break;
      case IDM_FILE_MENU:
          EQFBTPROWndNames( pDoc, hwndMenu );
          EQFBWindowMenu( pDoc, hwndMenu );
          EQFBSetKeyName( hwndMenu, IDM_CYCLE, NEXTDOC_FUNC );
          EQFBDocEnvMenu( pDoc, hwndMenu );
        EQFBSetKeyName( hwndMenu, IDM_OPEN, OPEN_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_SAVE, SAVE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_SAVEAS, SAVEAS_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_QUIT, QUIT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_FILE, FILE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_PRINT, PRINT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_REIMPORT, REIMPORT_FUNC );

        SETAABITEM( hwndMenu, IDM_CYCLE, TRUE );
        SETAABITEM( hwndMenu, IDM_OPEN, TRUE );
        if ( pDoc->docType == STARGET_DOC )
        {
           SETAABITEM( hwndMenu, IDM_SAVE, TRUE );
           SETAABITEM( hwndMenu, IDM_SAVEAS, TRUE );
           SETAABITEM( hwndMenu, IDM_FILE, TRUE );
           SETAABITEM( hwndMenu, IDM_QUIT,
                       !(pDoc->flags.changed || pDoc->EQFBFlags.workchng ||
                         pDoc->fFuzzyCopied ||
                        (pDoc->pTBSeg && pDoc->pTBSeg->SegFlags.UnTrans) ) );
           SETAABITEM( hwndMenu, IDM_REIMPORT, ReImport_IsAvailable( pDoc->szDocName ) );
        }
        else
        {
           SETAABITEM( hwndMenu, IDM_SAVE, FALSE );
           SETAABITEM( hwndMenu, IDM_SAVEAS, FALSE );
           SETAABITEM( hwndMenu, IDM_FILE, FALSE );
           SETAABITEM( hwndMenu, IDM_QUIT, TRUE );
           SETAABITEM( hwndMenu, IDM_REIMPORT, FALSE );
        } /* endif */
        break;

      case IDM_EDIT:
        EQFBSetKeyName( hwndMenu, IDM_FIND, FIND_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_CFIND, CFIND_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_RFIND, RFIND_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_CUT, CUT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_COPY, COPY_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_PASTE, PASTE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_CLEAR, MARKDELETE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_UNDO, UNDO_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_REDO, REDO_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_UNMARK, MARKCLEAR_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_SPLIT, SPLITLINE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_JOIN, JOINLINE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_REFLOW, LINEWRAP_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_WRAP, MARGINACT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_COPYPROPBLOCK, PROPMARKCOPY_FUNC );
        {
          PSTEQFGEN pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;
          PDOCUMENT_IDA pDocIda = (PDOCUMENT_IDA)pstEQFGen->pDoc;
          PTBDOCUMENT pTBDocProp = &(pDocIda->tbDevProposal.tbDoc);
          SETAABITEM( hwndMenu, IDM_COPYPROPBLOCK,
                      ((PEQFBBLOCK)pTBDocProp->pBlockMark)->pDoc );
        }

        SETAABITEM( hwndMenu, IDM_FIND, TRUE );
        SETAABITEM( hwndMenu, IDM_CFIND, TRUE );
        SETAABITEM( hwndMenu, IDM_RFIND, TRUE );
        SETAABITEM( hwndMenu, IDM_COPY, (pEQFBBlockMark->pDoc == pDoc) );
        SETAABITEM( hwndMenu, IDM_UNMARK, (BOOL) (pEQFBBlockMark->pDoc != NULL) );

        if ( pDoc->docType == STARGET_DOC )
        {
           DOSVALUE usInfo;            // info about clipboard
           if (pEQFBBlockMark->pDoc == pDoc )
           {
             if (pDoc->EQFBFlags.PostEdit)
             {
               if (pSeg->qStatus == QF_XLATED )
               {
                fActive = TRUE;
               }
               else
               {
                fActive = FALSE;
               }
             }
             else
             {
               if (( pSeg->qStatus == QF_CURRENT )
                    && (pEQFBBlockMark->ulSegNum == pSeg->ulSegNum)
                    && (pEQFBBlockMark->ulEndSegNum == pSeg->ulSegNum) )
               {
                 fActive = TRUE;
               }
               else
               {
                 fActive = FALSE;
               } /* endif */
             } /* endif */
           }
           else
           {
             fActive = FALSE;
           }
           SETAABITEM( hwndMenu, IDM_CUT, fActive);
                                         // paste only if clipdata avail
           usInfo = 0;
           SETAABITEM( hwndMenu, IDM_PASTE,
                       (ISCLIPBOARDFORMATAVAILABLE( CF_TEXT, &usInfo )|| ISCLIPBOARDFORMATAVAILABLE( CF_UNICODETEXT, &usInfo )) );

           SETAABITEM( hwndMenu, IDM_CLEAR, (BOOL) (pEQFBBlockMark->pDoc != NULL) );
//           fActive = (pDoc->TBCursor.ulSegNum == pDoc->ulWorkSeg);
           fActive=((pDoc->EQFBFlags.PostEdit && pSeg->qStatus == QF_XLATED)
              || (!pDoc->EQFBFlags.PostEdit && pSeg->qStatus == QF_CURRENT));

           if ( pDoc->hwndRichEdit )
           {
             BOOL fUndo = SendMessage( pDoc->hwndRichEdit, EM_CANUNDO, 0, 0L );
             SETAABITEM( hwndMenu, IDM_UNDO, fUndo );
           }
           else
           {
             SETAABITEM( hwndMenu, IDM_UNDO, fActive );
           } /* endif */

           SETAABITEM( hwndMenu, IDM_SPLIT, fActive && pDoc->EQFBFlags.Reflow);
           SETAABITEM( hwndMenu, IDM_JOIN, fActive && pDoc->EQFBFlags.Reflow);
           if (pDoc->fAutoLineWrap)
           {
			   SETAABITEM( hwndMenu, IDM_REFLOW, FALSE);
	       }
	       else
	       {
             SETAABITEM( hwndMenu, IDM_REFLOW, fActive && pDoc->EQFBFlags.Reflow);
	       }
	       {
			   BOOL fReflowAllowed = FALSE;
			   if (pDoc->EQFBFlags.Reflow || (!pDoc->EQFBFlags.Reflow && pDoc->fAutoLineWrap))
			   {
			     fReflowAllowed = TRUE;
		       }
               SETAABITEM( hwndMenu, IDM_WRAP, fReflowAllowed);
               SETAABITEMCHECK( hwndMenu, IDM_WRAP,
                            fReflowAllowed && pDoc->fLineWrap);
	       }
           if ( pDoc->hwndRichEdit )
           {
             BOOL fActive = SendMessage( pDoc->hwndRichEdit, EM_CANREDO, 0, 0L );
             SETAABITEM( hwndMenu, IDM_REDO, fActive );
           } /* endif */
        }
        else
        {
           SETAABITEM( hwndMenu, IDM_CUT, FALSE );
           SETAABITEM( hwndMenu, IDM_PASTE, FALSE );
           SETAABITEM( hwndMenu, IDM_CLEAR, FALSE );
           SETAABITEM( hwndMenu, IDM_UNDO, FALSE );
           SETAABITEM( hwndMenu, IDM_REDO, FALSE );
           SETAABITEM( hwndMenu, IDM_SPLIT, FALSE );
           SETAABITEM( hwndMenu, IDM_JOIN, FALSE );
           SETAABITEM( hwndMenu, IDM_REFLOW, FALSE );
           SETAABITEM( hwndMenu, IDM_WRAP, FALSE );
           SETAABITEMCHECK( hwndMenu, IDM_WRAP, FALSE );
        } /* endif */
        break;

      case IDM_OPTIONS_MENU:
        EQFBSetKeyName( hwndMenu, IDM_FONTCOL, FONTS_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_KEYS, KEYS_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_ENTRY, ENTRYSEN_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_COMMAND, COMMAND_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_FONTSIZE,FONTSIZE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_SETTINGS, SETTINGS_FUNC );

        if ( pDoc->docType == STARGET_DOC )
        {
           // Disable entry option if no dictionary or transl. mem windw
           fActive = (pDoc->EQFBFlags.PostEdit | pDoc->EQFBFlags.AutoMode);
           SETAABITEM( hwndMenu, IDM_ENTRY, ! fActive);
        }
        else
        {
           SETAABITEM( hwndMenu, IDM_ENTRY, FALSE );
        } /* endif */

        if ( hwndMenu )
        {
          ULONG ulCurStyle = ((PSTEQFGEN)pDoc->pstEQFGen)->flEditTgtStyle;
          SETAABITEMCHECK( hwndMenu, IDM_SC_HORZ,
                           (ulCurStyle & FCF_HORZSCROLL) );
          SETAABITEMCHECK( hwndMenu, IDM_SC_VERT,
                           (ulCurStyle & FCF_VERTSCROLL) );
          SETAABITEMCHECK( hwndMenu, IDM_SC_TITLE,
                           (ulCurStyle & FCF_TITLEBAR) );
          SETAABITEMCHECK( hwndMenu, IDM_SC_STATUS,
                           WinIsWindowVisible( pDoc->hStatusBarWnd ) );
          SETAABITEMCHECK( hwndMenu, IDM_SC_RULER,
                           WinIsWindowVisible( pDoc->hRulerWnd ) );
        }
        break;

      case IDM_CURSOR:
        EQFBSetKeyName(hwndMenu, IDM_GOUPDSEGMENT, GOUPDSEGMENT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_FINDMARK, MARKFIND_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_GOTOLINE, GOTO_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_TOCGOTO, TOCGOTO_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_QUERYLINE, QUERYLINE_FUNC );/* @KIT1341A*/
        EQFBSetKeyName( hwndMenu, IDM_TOP, TOPDOC_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_BOTTOM, BOTTOMDOC_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_SOL, STARTLINE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_EOL, ENDLINE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_SOS, STARTSEG_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_EOS, ENDSEG_FUNC );
        switch ( pDoc->docType )
        {
           case STARGET_DOC:
              SETAABITEM( hwndMenu, IDM_SOS, TRUE );
              SETAABITEM( hwndMenu, IDM_EOS, TRUE );
              break;
           default:
              SETAABITEM( hwndMenu, IDM_SOS, FALSE );
              SETAABITEM( hwndMenu, IDM_EOS, FALSE );
              break;
        } /* endswitch */
        break;

      case IDM_STYLE_MENU:
        // set key name
        EQFBSetKeyName( hwndMenu, IDM_PROTECTED, PROTECT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_UNPROTECTED, UNPROTECT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_HIDE, HIDE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_SHRINK, SHRINK_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_COMPACT, COMPACT_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_SHORTEN, SHORTEN_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_WYSIWYG, WYSIWYG_FUNC );

        switch ( pDoc->docType )
        {
           case SSOURCE_DOC:
              SETAABITEM( hwndMenu, IDM_UNPROTECTED, FALSE );
              break;
           case OTHER_DOC:
              SETAABITEM( hwndMenu, IDM_UNPROTECTED, FALSE );
              SETAABITEM( hwndMenu, IDM_HIDE, FALSE );
              SETAABITEM( hwndMenu, IDM_SHRINK, FALSE );
              SETAABITEM( hwndMenu, IDM_COMPACT, FALSE );
              SETAABITEM( hwndMenu, IDM_SHORTEN, FALSE );
              SETAABITEM( hwndMenu, IDM_WYSIWYG, FALSE );
              break;
           default:
              break;
        } /* endswitch */

        // set check mark
        SETAABITEMCHECK( hwndMenu, IDM_UNPROTECTED,
                         (pDoc->DispStyle == DISP_UNPROTECTED));
        SETAABITEMCHECK( hwndMenu, IDM_PROTECTED,
                         (pDoc->DispStyle == DISP_PROTECTED) );
        SETAABITEMCHECK( hwndMenu, IDM_HIDE,
                         (pDoc->DispStyle == DISP_HIDE) );
        SETAABITEMCHECK( hwndMenu, IDM_SHRINK,
                         (pDoc->DispStyle == DISP_SHRINK) );
        SETAABITEMCHECK( hwndMenu, IDM_COMPACT,
                         (pDoc->DispStyle == DISP_COMPACT) );
        SETAABITEMCHECK( hwndMenu, IDM_SHORTEN,
                         (pDoc->DispStyle == DISP_SHORTEN) );
        SETAABITEMCHECK( hwndMenu, IDM_WYSIWYG,
                         (pDoc->DispStyle == DISP_WYSIWYG) );
        break;
     case IDM_SPELL_MENU:
        // set key name
        EQFBSetKeyName( hwndMenu, IDM_PROOFSEG, SPELLSEG_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_PROOFALL, SPELLFILE_FUNC );
        EQFBSetKeyName( hwndMenu, IDM_EDITADD, EDITADD_FUNC );

        switch ( pDoc->docType )
        {
           case STARGET_DOC:
              // Disable spellcheck segment dependent of postedit on/off
              SETAABITEM( hwndMenu, IDM_AUTOSPELL, pDoc->fSpellCheck);
              SETAABITEM( hwndMenu, IDM_NEXTMISSPELL, pDoc->fSpellCheck);
              SETAABITEMCHECK( hwndMenu, IDM_AUTOSPELL,
                               pDoc->fAutoSpellCheck);

             fActive=((pDoc->EQFBFlags.PostEdit && pSeg->qStatus == QF_XLATED)
              || (!pDoc->EQFBFlags.PostEdit && pSeg->qStatus == QF_CURRENT));

              SETAABITEM( hwndMenu, IDM_PROOFSEG, fActive && pDoc->fSpellCheck);
              SETAABITEM( hwndMenu, IDM_PROOFALL, pDoc->fSpellCheck);
              SETAABITEM( hwndMenu, IDM_SPELL_MENU, pDoc->fSpellCheck);
              SETAABITEM( hwndMenu, IDM_EDITADD, pDoc->fSpellCheck);
              break;
           case SSOURCE_DOC:
           case OTHER_DOC:
              SETAABITEM( hwndMenu, IDM_PROOFSEG, FALSE);
              SETAABITEM( hwndMenu, IDM_PROOFALL, FALSE);
              SETAABITEM( hwndMenu, IDM_SPELL_MENU, FALSE);
              SETAABITEM( hwndMenu, IDM_EDITADD, FALSE);
              // Disable spellcheck segment dependent of postedit on/off
              SETAABITEM( hwndMenu, IDM_AUTOSPELL, FALSE);
              SETAABITEM( hwndMenu, IDM_NEXTMISSPELL, FALSE);
              break;
           default:
              break;
        } /* endswitch */
        break;

   } /* endswitch */
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBEnableToolbarItem
//------------------------------------------------------------------------------
// Function call:     EQFBEnableToolbarItem( hwnd, usID )
//------------------------------------------------------------------------------
// Description:       enable/disable the specific toolbar item
//------------------------------------------------------------------------------
// Parameters:        HWND        hwnd,        handle of window
//                    USHORT      usID         menu command id
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Function flow:     do checking if toolbar item has to be enabled
//                    take care of different document types active
//                    return
//------------------------------------------------------------------------------
BOOL EQFBEnableToolbarItem
(
  HWND        hwnd,
  USHORT      usID                 // message parameter
)
{
  BOOL          fEnabled = TRUE;   // item always enabled
  PTBDOCUMENT   pDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );
  PTBSEGMENT    pSeg = NULL;
  USHORT        usInfo;
  EQFBBLOCK* pEQFBBlockMark = get_EQFBBlockMark();

  if ( pDoc )
  {
    pSeg =  EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
  } /* endif */

  if ( pSeg )
  {
    /********************************************************************/
    /* take care of differences between source and target documents     */
    /********************************************************************/
    if ( pDoc->docType == STARGET_DOC )
    {
      switch ( usID )
      {
        case IDM_SHOWTRANS:
          fEnabled = (pDoc->pfnShowTrans != NULL );
          break;
        case IDM_TRANSNEW:
        case IDM_TRANSNEW_EXACT:
        case IDM_TRANSNEW_FUZZY:
        case IDM_TRANSNEW_NONE:
          fEnabled = !pDoc->fXlated;
          break;
        case IDM_GOTOMARK:
          fEnabled = pDoc->EQFBFlags.MarkedSeg;
          break;
        case IDM_CLEARMARK:
          fEnabled = pSeg->SegFlags.Marked;
          break;
        case IDM_SPLITSEG:
          fEnabled = ((! pDoc->EQFBFlags.PostEdit) && pSeg->SegFlags.JoinStart );
          break;
        case IDM_JOINSEG:
          {
            PLOADEDTABLE pTable = (PLOADEDTABLE)pDoc->pDocTagTable;
            BOOL fXLIFF = (strcmp( pTable->szName, "EQFXLIFF" ) == 0 );
            ULONG ulSegNum = pSeg->ulSegNum + 1;
            fEnabled = ( EQFBGetVisSeg( pDoc->twin, &ulSegNum )
                           && pDoc->EQFBFlags.Reflow
                           && ! pDoc->EQFBFlags.PostEdit && !fXLIFF ) ;
          }
          break;
        case IDM_POSTEDIT:
        case IDM_AUTOTRANS:
        case IDM_GOTO:
          fEnabled = !pDoc->EQFBFlags.PostEdit;
          break;
        case IDM_UNTRANS:
          fEnabled = (pDoc->TBCursor.ulSegNum == pDoc->ulWorkSeg) &&
                            ! pDoc->EQFBFlags.PostEdit &&
                            (pDoc->tbActSeg.qStatus == QF_XLATED);
          break;
        case IDM_DICTLOOK:
        case IDM_EDITTERM:
          fEnabled = ! EQFGETDICT(EQF_IS_AVAIL,NULL);
          break;
        case IDM_QUIT:
          fEnabled = !(pDoc->flags.changed || pDoc->EQFBFlags.workchng ||
                         pDoc->fFuzzyCopied ||
                         (pDoc->pTBSeg && pDoc->pTBSeg->SegFlags.UnTrans) );
          break;
        case IDM_REIMPORT:
          fEnabled = (pDoc->docType == STARGET_DOC) && ReImport_IsAvailable( pDoc->szDocName );
          break;
        case IDM_COPYPROPBLOCK:
          {
            PSTEQFGEN pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;
            PDOCUMENT_IDA pDocIda = (PDOCUMENT_IDA)pstEQFGen->pDoc;
            PTBDOCUMENT pTBDocProp = &(pDocIda->tbDevProposal.tbDoc);
            fEnabled = ((PEQFBBLOCK)pTBDocProp->pBlockMark)->pDoc != NULL;
          }
          break;
        case IDM_COPY:
          fEnabled = (pEQFBBlockMark->pDoc == pDoc);
          break;
        case IDM_UNMARK:
        case IDM_CLEAR:
          fEnabled = (pEQFBBlockMark->pDoc != NULL);
          break;
        case IDM_CUT:
          fEnabled = (pEQFBBlockMark->pDoc == pDoc);
          break;
        case IDM_PASTE:
          usInfo   = 0;
          fEnabled = (ISCLIPBOARDFORMATAVAILABLE( CF_TEXT, &usInfo )||ISCLIPBOARDFORMATAVAILABLE( CF_UNICODETEXT, &usInfo ));
          break;
        case IDM_UNDO:
          fEnabled = ((pDoc->EQFBFlags.PostEdit && pSeg->qStatus == QF_XLATED)
                       || (!pDoc->EQFBFlags.PostEdit && pSeg->qStatus == QF_CURRENT));
          break;
        case IDM_SPLIT:
        case IDM_JOIN:
        case IDM_REFLOW:
          fEnabled = (((pDoc->EQFBFlags.PostEdit && pSeg->qStatus == QF_XLATED)
                       || (!pDoc->EQFBFlags.PostEdit && pSeg->qStatus == QF_CURRENT))
                       && pDoc->EQFBFlags.Reflow);
          break;
        case IDM_WRAP:
         	  fEnabled = FALSE;
			  if (pDoc->EQFBFlags.Reflow ||
			      (!pDoc->EQFBFlags.Reflow && pDoc->fAutoLineWrap))
			  {
				  fEnabled = TRUE;
		      }
		      break;
        case IDM_ENTRY:
          fEnabled = ! (pDoc->EQFBFlags.PostEdit | pDoc->EQFBFlags.AutoMode);
          break;
        case IDM_SOS:
        case IDM_EOS:
          fEnabled = TRUE;
          break;
        case IDM_PROOFSEG:
          fEnabled = ((pDoc->EQFBFlags.PostEdit && pSeg->qStatus == QF_XLATED)
                       || (!pDoc->EQFBFlags.PostEdit && pSeg->qStatus == QF_CURRENT))
                       && pDoc->fSpellCheck;
          break;
        case IDM_PROOFALL:
        case IDM_SPELL_MENU:
        case IDM_EDITADD:
          fEnabled = pDoc->fSpellCheck;
          break;
      } /* endswitch */
    }
    else
    {
      switch ( usID )
      {
        case IDM_FILE_MENU:
        case IDM_TRANSSEG:
        case IDM_TRANSNEW:
        case IDM_TRANSNEW_EXACT:
        case IDM_TRANSNEW_FUZZY:
        case IDM_TRANSNEW_NONE:
        case IDM_MARKSEG:
        case IDM_GOTOMARK:
        case IDM_CLEARMARK:
        case IDM_JOINSEG:
        case IDM_SPLITSEG:
        case IDM_POSTEDIT:
        case IDM_AUTOTRANS:
        case IDM_UNTRANS:
        case IDM_SAVE:
        case IDM_SAVEAS:
        case IDM_FILE:
        case IDM_CUT:
        case IDM_PASTE:
        case IDM_CLEAR:
        case IDM_UNDO:
        case IDM_REDO:
        case IDM_SPLIT:
        case IDM_JOIN:
        case IDM_REFLOW:
        case IDM_WRAP:
        case IDM_ENTRY:
        case IDM_SOS:
        case IDM_EOS:
        case IDM_PROOFSEG:
        case IDM_PROOFALL:
        case IDM_SPELL_MENU:
        case IDM_EDITADD:
        case IDM_UNPROTECTED:
          fEnabled =FALSE;
          break;
        case IDM_GOTO:
          {
            // goto active segment only if not post edit or automatic mode
            PTBDOCUMENT pDocTarget = pDoc;
            while ( pDocTarget && pDocTarget->docType != STARGET_DOC )
            {
               pDocTarget = pDocTarget->next;
            } /* endwhile */
            if ( pDocTarget )
            {
               fEnabled = ! pDocTarget->EQFBFlags.PostEdit;
            }
            else
            {
              fEnabled = FALSE;
            }
          }
          break;
        case IDM_DICTLOOK:
        case IDM_EDITTERM:
          fEnabled = ! EQFGETDICT(EQF_IS_AVAIL,NULL);
          break;
        case IDM_COPY:
//          fEnabled = (pEQFBBlockMark->pDoc == pDoc);
          if (pDoc && pDoc->pBlockMark)
          {
            fEnabled = ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc == pDoc;
          }
          break;
        case IDM_UNMARK:
          if (pDoc && pDoc->pBlockMark)
          {
            fEnabled = ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc != NULL;
          }
          break;
        case IDM_HIDE:
        case IDM_SHRINK:
        case IDM_COMPACT:
        case IDM_SHORTEN:
        case IDM_WYSIWYG:
          fEnabled = ( pDoc->docType == SSOURCE_DOC );
          break;
        case IDM_SHOWTRANS:
          fEnabled = ( pDoc->docType == SSOURCE_DOC );
          break;
      } /* endswitch */
    } /* endif */
  } /* endif */
  return fEnabled;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBTPROWndNames
//------------------------------------------------------------------------------
// Function call:     EQFBWIndowMenu(pDoc, hwnd)
//------------------------------------------------------------------------------
// Description:       update TM Window names in pulldown
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc,        pointer to document ida
//                    HMENU       hwndMenu     handle of aab in windows
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     set actionbar names
//                    find target document pointer
//                    determine if we are in post-edit mode
//                    set action-bar items correspondingly
//                    return
//------------------------------------------------------------------------------
VOID EQFBTPROWndNames
(
  PTBDOCUMENT pDoc,
  HMENU hwndMenu                     // handle of aab in windows
)
{
  PTBDOCUMENT pDocTarget;
  BOOL        fActive;                 // post editing active


  // correct key names displayed in menu
  EQFBSetKeyName( hwndMenu, IDM_TRANSWND, ACTTRANS_FUNC );
  EQFBSetKeyName( hwndMenu, IDM_DICTWND, ACTDIC_FUNC );
  EQFBSetKeyName( hwndMenu, IDM_TMWND, ACTPROP_FUNC );
  EQFBSetKeyName( hwndMenu, IDM_DISPORG, DISPORG_FUNC );
  EQFBSetKeyName( hwndMenu, IDM_SRCPROPWND, SRCPROP_FUNC );
  EQFBSetKeyName( hwndMenu, IDM_SEGPROPWND, SEGPROP_FUNC );
  /**************************************************************/
  /* get ptr to target document                                 */
  /**************************************************************/
  pDocTarget = pDoc;
  while ( pDocTarget->docType != STARGET_DOC )
  {
     pDocTarget = pDocTarget->next;
  } /* endwhile */
  fActive = ! pDocTarget->EQFBFlags.PostEdit;
  if ( fActive &&
       (EQFGETDICT( EQF_IS_AVAIL, NULL ) != EQFRS_NOT_AVAILABLE))
  {
    SETAABITEM( hwndMenu, IDM_DICTWND, TRUE );
  }
  else
  {
    SETAABITEM( hwndMenu, IDM_DICTWND, FALSE );
  } /* endif */
  if ( fActive && pDocTarget->pstEQFGen )
  {
    fActive = WinIsWindowVisible( ((PDOCUMENT_IDA)((PSTEQFGEN)pDocTarget->pstEQFGen)->pDoc)->hwndProposals );
  } /* endif */
  SETAABITEM( hwndMenu, IDM_TMWND, fActive );
  SETAABITEM( hwndMenu, IDM_SRCPROPWND, fActive );

  if ( pDoc->pstEQFGen )
  {
    //PDOCUMENT_IDA pIdaDoc  = (PDOCUMENT_IDA) ((PSTEQFGEN)(pDoc->pstEQFGen))->pDoc;
    //if ( pIdaDoc )
    //{
    //  if ( strcmp( pIdaDoc->szDocFormat, "EQFXLIFF" ) == 0 )
    //  {
    //    SETAABITEM( hwndMenu, IDM_SEGPROPWND, TRUE );
    //  }
    //  else
    //  {
    //    SETAABITEM( hwndMenu, IDM_SEGPROPWND, FALSE );
    //  } /* endif */
    //}
    //else
    //{
    //  SETAABITEM( hwndMenu, IDM_SEGPROPWND, FALSE );
    //} /* endif */
    SETAABITEM( hwndMenu, IDM_SEGPROPWND, TRUE );
  }
  else
  {
    SETAABITEM( hwndMenu, IDM_SEGPROPWND, FALSE );
  } /* endif */

  switch (pDoc->docType)
  {
     case STARGET_DOC:
       SETAABITEMCHECK( hwndMenu, IDM_TRANSWND, TRUE );
       break;
     case SSOURCE_DOC:
       SETAABITEMCHECK( hwndMenu, IDM_DISPORG, TRUE );
       break;
     case SERVPROP_DOC:                // proposal window
       SETAABITEMCHECK( hwndMenu, IDM_TMWND, TRUE );
       break;
     case SERVDICT_DOC:                // dicitonary window
       SETAABITEMCHECK( hwndMenu, IDM_DICTWND, TRUE );
       break;
     case SERVSOURCE_DOC:              // source window for proposals
       SETAABITEMCHECK( hwndMenu, IDM_SRCPROPWND, TRUE );
       break;
     case OTHER_DOC:
     case TRNOTE_DOC:
     default:
       break;
  } /* endswitch */
  return;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBWindowMenu
//------------------------------------------------------------------------------
// Function call:     EQFBWIndowMenu(pDoc, hwnd)
//------------------------------------------------------------------------------
// Description:       update document names in windows pulldown
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   _
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------
static
VOID EQFBWindowMenu
(
  PTBDOCUMENT pDoc,                              // pointer to document ida
  HMENU       hwndMenu
)
{
  HMENU  hwndSubMenu;
  USHORT usI, usId;
  USHORT usWindow;
  PTBDOCUMENT pDocTarget;                         // pointer to target doc
  PTBDOCUMENT pDocStart;                          // to get end of loop
  BOOL  fOK = TRUE;
  PSZ         pFilename;                          //filename w/o path
  CHAR       szMenuName[144];                     // name to insert into menu
  USHORT      usItems;                            // number of items in menu
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  {
    USHORT usIdPropSrc = IDM_FILE_MENU;

     /*****************************************************************/
     /* delete all document window names ....                         */
     /*****************************************************************/
     if ( IsZoomed( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS) )
     {
       usIdPropSrc ++;
     } /* endif */
     hwndSubMenu = GetSubMenu(hwndMenu, usIdPropSrc);
     usItems = (USHORT) GetMenuItemCount(hwndSubMenu);
     for ( usIdPropSrc = 0; usIdPropSrc < usItems; usIdPropSrc++ )
     {
       usI = (USHORT) GetMenuItemID( hwndSubMenu, usIdPropSrc );
       if ( usI == IDM_SEGPROPWND )
       {
         break;
       } /* endif */
     } /* endfor */
     usIdPropSrc++;
     for ( usI= usIdPropSrc; usI <= usItems ;usI++ )
     {
       DeleteMenu(hwndSubMenu, usIdPropSrc, MF_BYPOSITION);
     } /* endfor */
  }
     if ( fOK )
     {
        CHAR chDesc[ MAX_DESCRIPTION ];
        usWindow = 7;
        usId = IDM_OTHERWND;
        pDocTarget = pDocStart = pDoc;           //starting document
        do
        {
          if ((pDoc->docType == OTHER_DOC) || (pDoc->docType == TRNOTE_DOC))
          {
            /*****************************************************/
            /* add documentname in windows pulldown              */
            /* eliminate path in document name                   */
            /*****************************************************/
            pFilename = UtlGetFnameFromPath( pDoc->szDocName );
            if (!pFilename || (pDoc->docType == TRNOTE_DOC))
            {
              WinLoadString( (HAB) UtlQueryULong( QL_HAB ), hResMod, SID_TRNOTE_TITLE,
                             sizeof(chDesc) - 1, chDesc );
              pFilename = chDesc;
            } /* endif */
            if ( usWindow <= 9 )
            {
              sprintf( szMenuName, "&%d) %s", usWindow++, pFilename );
            }
            else
            {
               strcpy( szMenuName, pFilename );
            } /* endif */
            AppendMenu(hwndSubMenu,MF_ENABLED, usId, szMenuName);
            pDoc->usWndId = usId;
            usId ++;
          } /* endif */
          pDoc = pDoc->next;
                                                           /* enddo */
        } while (pDoc != pDocTarget && (usId <= IDM_LASTOTHER) );

        pDoc = pDocStart;                             //starting document
        /**************************************************************/
        /* grey name of current doc if it is other-doc                */
        /**************************************************************/
        if ( pDoc->docType == OTHER_DOC )
        {
          SETAABITEM( hwndSubMenu, pDoc->usWndId, FALSE );
        } /* endif */
     } /* endif */
} /* end of function EQFBWindowMenu */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBDocEnvMenu
//------------------------------------------------------------------------------
// Function call:     EQFBDocEnvMenu(pDoc, hwnd)
//------------------------------------------------------------------------------
// Description:       update document names in environment pulldown
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    -- ptr to document ida
//                    HWND           -- handle of menu
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     delete all old documents listed in the DocEnvMenu
//                    iterate through all 'loaded' documents and list them
//                    return
//------------------------------------------------------------------------------
static
VOID EQFBDocEnvMenu
(
  PTBDOCUMENT pDoc,                              // pointer to document ida
  HMENU       hwndMenu
)
{
  HMENU  hwndSubMenu;
  USHORT usId;
  USHORT usWindow;
  PSZ         pFilename = NULL;                  //filename w/o path
  PSZ pInDocObjName=NULL;
  PSZ pOutDocObjName, pMenuName, pOutDocLongName;
  USHORT usRc;
  {
    USHORT usIdPropSrc = IDM_FILE_MENU;
    SHORT  sItems;
    SHORT  sI;
    /*****************************************************************/
    /* delete all document window names ....                         */
    /*****************************************************************/
    if ( IsZoomed( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS) )
    {
      usIdPropSrc ++;
    } /* endif */
    hwndSubMenu = GetSubMenu(hwndMenu, usIdPropSrc);
    usIdPropSrc = IDM_CYCLE;
    /*****************************************************************/
    /* delete all document names                                     */
    /* Assumption: it is first entry in file pulldown.               */
    /*****************************************************************/
    hwndSubMenu = GetSubMenu(hwndSubMenu, 0);
    sItems = (SHORT)GetMenuItemCount(hwndSubMenu);

    for ( sI= 0; sI < sItems ;sI++ )
    {
      DeleteMenu(hwndSubMenu, 0, MF_BYPOSITION);
    } /* endfor */
  }
  if ( UtlAlloc( (PVOID *)&pInDocObjName, 0L, 4L*MAX_LONGPATH, NOMSG ))
  {
    pOutDocObjName = pInDocObjName+MAX_LONGPATH;
    pMenuName      = pInDocObjName+2*MAX_LONGPATH;
    pOutDocLongName= pInDocObjName+3*MAX_LONGPATH;

    usWindow = 1;
    usId  = IDM_DOC_ENVIRONMENT;

    do
    {
       usRc = EQF_XDOCNEXT( (PSTEQFGEN)pDoc->pstEQFGen, pInDocObjName, pOutDocObjName );
      if ( !usRc && *pOutDocObjName )
      {
        /**************************************************************/
        /* try to find long name -- in case of error use mangled name */
        /**************************************************************/
        if ( DocQueryInfo2( pOutDocObjName,        // document object name
                            NULL,                  // document translation memory
                            NULL,                  // format of document
                            NULL,                  // document source language
                            NULL,                  // document target language
                            pOutDocLongName,       // long name of document
                            NULL,                  // document alias name
                            NULL,                  // document editor
                            TRUE ) == NO_ERROR )           // handle errors in function
        {
          if ( *pOutDocLongName )
          {
             pFilename = pOutDocLongName;
          }
          else
          {
            pFilename = UtlGetFnameFromPath( pOutDocObjName );
          }
        } /* endif */
        if ( usWindow <= 9 )
        {
           sprintf( pMenuName, "&%d) %s", usWindow++, pFilename );
        }
        else
        {
          strcpy( pMenuName, pFilename );
        } /* endif */
        AppendMenu(hwndSubMenu,MF_ENABLED, usId, pMenuName);
        usId ++;
        strcpy( pInDocObjName, pOutDocObjName );
      } /* endif */
    } while ( !usRc && *pOutDocObjName );
    UtlAlloc( (PVOID *)&pInDocObjName, 0L, 0L, NOMSG );
  } /* endif */
} /* end of function EQFBDocEnvMenu */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBBuildPrefixes
//------------------------------------------------------------------------------
// Function call:     EQFBBuildPrevixes(pArray )
//------------------------------------------------------------------------------
// Description:       build array with dictionary prefixes
//------------------------------------------------------------------------------
// Parameters:        PCHAR          -- ptr to array of prefixes
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID EQFBBuildPrefixes
(
  PCHAR_W     pDictPrefix                        // pointer to document ida
)
{
  PRESKEYTABLE  pResKey;
  PCHAR_W       pNextPrefix;
  USHORT        usI = 0;


  pNextPrefix = pDictPrefix;
  pResKey = get_ResKeyTab();
  while (pNextPrefix && (pResKey->ucCode || pResKey->ucState )
         && (usI <= ALL_LETTERS)  )
  {
    if ((pResKey->Function == GETDICTMATCH_FUNC)
         && (pResKey->usAssignStatus != ASSIGNED_TO_OTHER ) )
    {
       *pNextPrefix = pResKey->ucCode;
       pNextPrefix ++;
       usI++;
    } /* endif */
    pResKey++;
  } /* endwhile */
  /********************************************************************/
  /* fill up with '*' the array of dict-prefixes                      */
  /********************************************************************/
  while (usI <= ALL_LETTERS )
  {
    *pNextPrefix = '*';
    usI++;
    pNextPrefix ++;
  } /* endwhile */

  return;
}

/**********************************************************************/
/* Switch the keyboard to support the language we are translating     */
/* This has to be done for BIDI and THAI languages                    */
/**********************************************************************/
HKL SwitchKeyboardForSpecLangs( PDOCUMENT_IDA pIda )
{
  HKL  BuffKL[21];
  ULONG uli ;
  BOOL fFound;
  WORD wLangID;
  HKL  hOldKL = GetKeyboardLayout(0);
  HKL  hKLReturn = NULL;

  wLangID = GetLangID( pIda );

  if ( wLangID != 0 )
  {
    if (LOWORD(hOldKL) != wLangID )
    {
      ULONG ulMax = GetKeyboardLayoutList( 20, &BuffKL[0] );
      uli = 0;
      fFound = FALSE;
      while (!fFound && (uli < ulMax))
      {
        if (LOWORD(BuffKL[uli] ) == wLangID)
        {
          /************************************************************/
          /* activate corresponding keyboard                          */
          /************************************************************/
          hKLReturn = hOldKL;
          ActivateKeyboardLayout( BuffKL[uli], 0 /*KLF_SETFORPROCESS*/);
          fFound = TRUE;
        }
        else
        {
          uli ++;
        } /* endif */
      } /* endwhile */
    } /* endif */
  } /* endif */
  return hKLReturn;
}



