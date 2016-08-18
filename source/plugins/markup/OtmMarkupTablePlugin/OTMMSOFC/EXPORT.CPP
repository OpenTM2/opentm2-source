/*
*
*  Copyright (C) 1998-2016, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/****************************************************************************/
/*                                                                          */
/* EXPORT.CPP                                                               */
/*                                                                          */
/*FUNCTIONS:                                                                */
/* PostExport                                                               */
/*                                                                          */
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 2/26/2016: IBM : Original Source                                         */
/*==========================================================================*/

#include "unicode.h"
#include "otmmsofc.h"



extern   wifstream  *InputFile ;
extern   wifstream  *InputFile2 ;

extern   WCHAR   szPrevXMLInputText[MAX_XML_RCD_LENGTH2-MAX_XML_RCD_LENGTH+1] ;
extern   BOOL    bBidi  ;
extern   BOOL    bDBCS  ;
extern   BOOL    bUTF16 ;
extern   BOOL    bReadSource ;
extern   BOOL    bFileExport ; 
extern   BOOL    bSetCommonProperty ;
extern   BOOL    bExportSSNTContent ;


extern   short   sTPVersion ;               /* From USRCALLS.C  */
extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */



#define    TITLE_MSOFC_PARSING_ERROR           "Parsing Error"
#define    TITLE_MSOFC_PARAGRAPH_MISMATCH      "Paragraph Mismatch"
#define    TITLE_MSOFC_TEXT_UNITS_MISMATCH     "Text Units Mismatch"

#define    MSG_MSOFC_PARSING_ERROR             "File could not be parsed.\n\nProcessing is terminated."
#define    MSG_MSOFC_PARAGRAPH_MISMATCH        "Target paragraph ID=%ld does not match source paragraph ID=%ld.\n\nProcessing is terminated."
#define    MSG_MSOFC_TEXT_UNITS_MISMATCH       "Target paragraph %ld contains %ld text units while source contains %ld.\n\nProcessing is terminated."


/*****************************************************************************/
/*                                                                           */
/*          ===>  PROBLEM WITH EXCEL 2007  <=======                          */
/*                                                                           */
/*     When the text is changed, any attribute which contains that same      */
/*     English text must also be changed.  The actual text is also used as   */
/*     a reference to that cell, column, row.                                */
/*                                                                           */
/*     If the text is never referenced by an attribute, then it can just     */
/*     be translated.                                                        */
/*                                                                           */
/*                                                                           */
/*     For example (inventory.xlsx):                                         */
/*          <si><t>Reorder Level</t></si>                                    */
/*          <tableColumn id="11" name="Reorder Level"/>                      */
/*                                                                           */
/*       If the translatable text is changed:                                */
/*          <si><t>Trans Reorder Level</t></si>                              */
/*                                                                           */
/*       Then the reference also has to be changed:                          */
/*          <tableColumn id="11" name="Trans Reorder Level"/>                */
/*                                                                           */
/*                                                                           */
/*     Sales.xlsx   same problem for <sheet name=""> element.                */
/*       workbook.xml                                                        */
/*          <sheet name="Sales By Region" sheetId="2" r:id="rId2"/>          */
/*       app.xml                                                             */
/*          <TitlesOfParts>                                                  */
/*          <vt:vector size="3" baseType="lpstr">                            */
/*          <vt:lpstr>Sales By Region</vt:lpstr>                             */
/*       pivotCacheDefinition1.xml                                           */
/*          <cacheField name="Quarter" numFmtId="0">                         */
/*       table1.xml                                                          */
/*          <tableColumn id="2" name="Quarter">                              */
/*       pivotTable2.xml                                                     */
/*          <pivotTableDefinition rowHeaderCaption="" colHeaderCaption=""    */
/*                                dataCaption="">                            */
/*                                                                           */
/*     Statistics.xlsx  (<c:f> is formula)                                   */
/*       workbook.xml                                                        */
/*          <sheet name="Skewness" sheetId="9" r:id="rId4"/>                 */
/*       chart.xml                                                           */
/*          <c:val><c:numRef><c:f>Skewness!$D$13:$D$21</c:f>                 */
/*          <c:val><c:numRef><c:f>'Normal Distribution'!$B$2:$B$802          */
/*          <definedName>Sheet1!xxxx</definedName>                           */
/*          <c:tx><c:v>Frequency</c:v></c:tx>                                */
/*                                                                           */
/*     Expenses.xlsx                                                         */
/*       sheet1.xml                                                          */
/*         <f>"The expense total for this quarter in 2007 is " &amp; F11</f> */
/*                                                                           */
/*                                                                           */
/*    Others:                                                                */
/*      <hyperlink display="text" tooltip="Search Page" />                   */
/*      <controlPr altText="alternate text" />                               */
/*      <objectPr  altText="alternate text" />                               */
/*      <dataValidation error="text" errorTitle="text"                       */
/*                      prompt="text" promptTitle="text">                    */
/*      <webPublishItem title="text" />                                      */
/*      <table comment="text" displayName="text?">                           */
/*                                                                           */
/*      <c><is><t>text</t></is></c>        (cell)                            */
/*      <nc><is><t>text</t></is></nc>      (new cell)                        */
/*                                                                           */
/*      <evenFooter>&amp;CCenter &amp;"-,Bold"Bold</evenFooter>              */
/*      <oddFooter>                                                          */
/*      <evenHeader>                                                         */
/*      <oddHeader>                                                          */
/*                                                                           */
/*      Formula:                                                             */
/*        String must be within double quotes ("text""more")                 */
/*                                                                           */
/*      Sheet name:                                                          */
/*        sheet_name!                                                        */
/*        sheet_name:sheet_name!                                             */
/*        [workbook_name]sheet_name:sheet_name!                              */
/*        'sheet_name'!                                                      */
/*        'sheet_name:sheet_name'!                                           */
/*        '[workbook_name]sheet_name:sheet_name'!                            */
/*                                                                           */
/*****************************************************************************/



/*****************************************************************************/
/*  PostExport                                                               */
/*                                                                           */
/*  Function called by EQFPOSTUNSEG2.                                        */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL PostExport( PSZ in, PSZ out, PSZ source, USHORT *usFileFormat )
{

    wofstream OutFile ( out ) ;
    InputFile  = new wifstream( source ) ;
    InputFile2 = new wifstream( in ) ;
 
    P_INFO     *ptrSrcParaHead ;
    P_INFO     *ptrTgtParaHead ;
    P_INFO     *ptrSrcParaCur ;
    P_INFO     *ptrTgtParaCur ;
 
    R_INFO     *ptrSrcRunHead ;
    R_INFO     *ptrTgtRunHead ;
    R_INFO     *ptrSrcRunCur ;
    R_INFO     *ptrTgtRunCur ;
    R_INFO     *ptrRunTemp ;
 
    T_INFO     *ptrSrcTextCur ;
    R_INFO     *ptrTextTemp ;

    SSNT_INFO  *ptrSrcSSNTHead ;
    SSNT_INFO  *ptrTgtSSNTHead ;
    SSNT_INFO  *ptrSSNTCur ;

    DBCS_INFO  *ptrDbcsHead ;
    DBCS_INFO  *ptrDbcsCur ;

    WCHAR      szTemp[MAX_XML_RCD_LENGTH2*2] ;
    WCHAR      szValue[MAX_XML_RCD_LENGTH] ;
    WCHAR      szValue2[MAX_XML_RCD_LENGTH] ;
    WCHAR      szValue3[MAX_XML_RCD_LENGTH] ;
    WCHAR      szErrTitle[128] ;
    WCHAR      szErrText[512] ;
    char       szTempC1[512] ;
    char       szTempC2[512] ;
    ULONG      ulFromSrcFilePos ;
    ULONG      ulToSrcFilePos ;
    ULONG      ulFromTgtFilePos ;
    ULONG      ulToTgtFilePos ;
    ULONG      ulSeqNum ;
    USHORT     sRunConcat ;
    USHORT     sRunSrcNodes ;
    USHORT     sRunTgtNodes ;
    USHORT     usFixLevel = 0 ;
    USHORT     rc ;
    BOOL       bConcat ;
    BOOL       bPreserveBlanks ;
    BOOL       bReturn = TRUE;


    bFileExport = TRUE ;
    bSetCommonProperty = FALSE ;
    bExportSSNTContent = FALSE ;

 
    if ( ( ! stricmp( szDocTargetLanguage, "ARABIC" ) ) ||
         ( ! stricmp( szDocTargetLanguage, "HEBREW" ) ) ||
         ( ! stricmp( szDocSourceLanguage, "ARABIC" ) ) ||
         ( ! stricmp( szDocSourceLanguage, "HEBREW" ) ) )
       bBidi = TRUE ;
    else
       bBidi = FALSE ;

    if ( ( ! stricmp( szDocTargetLanguage, "CHINESE(SIMPL.)" ) ) ||
         ( ! stricmp( szDocTargetLanguage, "CHINESE(TRAD.)"  ) ) ||
         ( ! stricmp( szDocTargetLanguage, "JAPANESE"        ) ) ||
         ( ! stricmp( szDocTargetLanguage, "KOREAN"          ) ) ) {
       bDBCS = TRUE ;
    }
 
    /*************************************************************************/
    /*  Create a linked list which contains all of the required              */
    /*  information necessary for the translated file.                       */
    /*************************************************************************/
    if ( bReturn ) {
       bReadSource = FALSE ;
       bUTF16 = (*InputFile2).IsUTF16();
       rc = fnCreateInputList( &ptrTgtParaHead, &ptrTgtSSNTHead, 0, NULL, usFileFormat, &usFixLevel ) ;
       if ( rc > 0 ) {
          MessageBoxA(HWND_DESKTOP, MSG_MSOFC_PARSING_ERROR, TITLE_MSOFC_PARSING_ERROR, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
          bReturn = FALSE ;
       }
    }
    /*************************************************************************/
    /*  Create a linked list which contains all of the required              */
    /*  information necessary for the source file.                           */
    /*************************************************************************/
    if ( bReturn ) {
       bReadSource = TRUE ;
       bUTF16 = (*InputFile).IsUTF16();
       rc = fnCreateInputList( &ptrSrcParaHead, &ptrSrcSSNTHead, 0, NULL, NULL, &usFixLevel ) ;
       if ( rc > 0 ) {
          MessageBoxA(HWND_DESKTOP, MSG_MSOFC_PARSING_ERROR, TITLE_MSOFC_PARSING_ERROR, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
          bReturn = FALSE ;
       }
    }
 
 
    /*************************************************************************/
    /*  Create the exported file.                                            */
    /*************************************************************************/
    if ( bReturn ) {
       ulFromSrcFilePos = 0 ;

       /**********************************************************************/
       /*  If file contains no translatable text, then simply copy entire    */
       /*  source file to export file.                                       */
       /**********************************************************************/
       if ( ( ptrTgtParaHead ) &&
            ( ptrTgtParaHead->SeqNum == 0 ) &&
            ( ptrTgtParaHead->ptrNext == 0 ) ) {
          ulFromSrcFilePos = 0 ;
          ulToSrcFilePos = 0 ;
          fnWriteBlock( WRITE_SOURCE, 
                        ulFromSrcFilePos, 
                        ulToSrcFilePos, 
                        &OutFile, FALSE ) ;
       }
       else {

       /**********************************************************************/
       /*                                                                    */
       /*  Process each paragraph node.                                      */
       /*                                                                    */
       /**********************************************************************/
       for( ptrSrcParaCur=ptrSrcParaHead, ptrTgtParaCur=ptrTgtParaHead ; 
            ptrSrcParaCur || ptrTgtParaCur ; 
            ptrSrcParaCur=(P_INFO*)ptrSrcParaCur->ptrNext,
               ptrTgtParaCur=(P_INFO*)ptrTgtParaCur->ptrNext ) {

          /*******************************************************************/
          /*  Handle reference nodes by finding the references source text   */
          /*  and writing out the associated target text.                    */
          /*******************************************************************/
          if ( ( ptrSrcParaCur->NodeType == NODE_TYPE_SHEET_REF ) ||   
               ( ptrSrcParaCur->NodeType == NODE_TYPE_TEXT_REF  ) ) {   
             for( ; ptrSrcParaCur && ( ptrSrcParaCur->NodeType==NODE_TYPE_SHEET_REF ||
                    ptrSrcParaCur->NodeType==NODE_TYPE_TEXT_REF ) ;
                  ptrSrcParaCur=(P_INFO*)ptrSrcParaCur->ptrNext ) {
                /*******************************************************************/
                /*  Copy end of previous paragraph block to the start of this      */
                /*  paragraph block.                                 (SOURCE COPY) */
                /*******************************************************************/
                ulToSrcFilePos = (ptrSrcParaCur->BlockStartPos) - 1*sizeof(WCHAR) ;
                if ( ptrTgtParaCur ) 
                   bPreserveBlanks = ptrTgtParaCur->PreserveBlanks ;
                else
                   bPreserveBlanks = FALSE ;
                fnWriteBlock( WRITE_SOURCE, 
                              ulFromSrcFilePos, 
                              ulToSrcFilePos, 
                              &OutFile,
                              bPreserveBlanks ) ;
                ulFromSrcFilePos = (ptrSrcParaCur->BlockEndPos) + 1*sizeof(WCHAR) ;
                fnWriteReferenceText( ptrSrcParaCur, ptrSrcParaHead, ptrTgtParaHead, &OutFile ) ;
             }
          }
          if ( ( ! ptrSrcParaCur ) ||
               ( ! ptrTgtParaCur ) ) {
             break ;
          }

          if ( ptrSrcParaCur->SeqNum != ptrTgtParaCur->SeqNum ) {
             strcpy( szTempC1, MSG_MSOFC_PARAGRAPH_MISMATCH ) ;
             sprintf( szTempC2, szTempC1, ptrTgtParaCur->SeqNum, ptrSrcParaCur->SeqNum ); 
             MessageBoxA(HWND_DESKTOP, szTempC2, TITLE_MSOFC_PARAGRAPH_MISMATCH, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
             bReturn = FALSE ;
             break ;
          }
          if ( ptrSrcParaCur->NumTextTags != ptrTgtParaCur->NumTextTags ) {
             if ( bSetCommonProperty ) {                        
                sRunSrcNodes = 0 ;
                if ( ptrSrcParaCur->CommonProperty ) 
                   ++sRunSrcNodes ;
                for( ptrSrcRunCur=(R_INFO*)ptrSrcParaCur->ptrRunList ; 
                     ptrSrcRunCur ; 
                     ptrSrcRunCur=(R_INFO*)ptrSrcRunCur->ptrNext ) {
                   if ( ( ptrSrcRunCur->Concat != CONCAT_TEXT_YES ) &&
                        ( ( ptrSrcRunCur->CommonProperty == 0 ) ||
                          ( ptrSrcRunCur->CommonProperty == COMMONPROPERTY_FORCE_OFF  ) ) ) {
                      if ( ( ptrSrcRunCur->NumTextTags == 0   ) &&
                           ( ptrSrcRunCur->NumNeutralTags > 1 ) )
                         sRunSrcNodes += ptrSrcRunCur->NumNeutralTags-1 ;
                      else
                         ++sRunSrcNodes ;
                   }
                }
                if ( ( ptrTgtParaCur->CommonProperty ) ||
                     ( ptrTgtParaCur->ptrRunList == 0 ) )
                   sRunTgtNodes = 1 ;
                else
                   sRunTgtNodes = 0 ;
                for( ptrTgtRunCur=(R_INFO*)ptrTgtParaCur->ptrRunList ; 
                     ptrTgtRunCur ; 
                     ptrTgtRunCur=(R_INFO*)ptrTgtRunCur->ptrNext ) {
                   if ( ( ptrTgtRunCur->Concat != CONCAT_TEXT_YES ) &&
                        ( ptrTgtRunCur->CommonProperty == 0 ) ) 
                      ++sRunTgtNodes ;
                }
                if ( sRunSrcNodes != sRunTgtNodes ) {
                   strcpy( szTempC1, MSG_MSOFC_TEXT_UNITS_MISMATCH ) ;
                   sprintf( szTempC2, szTempC1,ptrTgtParaCur->SeqNum, sRunTgtNodes, sRunSrcNodes ); 
                   MessageBoxA(HWND_DESKTOP, szTempC2, TITLE_MSOFC_TEXT_UNITS_MISMATCH, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
                   bReturn = FALSE ;
                   break ;
                }
             } else {
                for( ptrSrcRunCur=(R_INFO*)ptrSrcParaCur->ptrRunList, sRunConcat=0 ; 
                     ptrSrcRunCur ; 
                     ptrSrcRunCur=(R_INFO*)ptrSrcRunCur->ptrNext ) {
                   if ( ptrSrcRunCur->Concat == CONCAT_TEXT_YES ) 
                      ++sRunConcat ;
                }
                if ( ptrSrcParaCur->NumTextTags - sRunConcat != ptrTgtParaCur->NumTextTags ) {
                   strcpy( szTempC1, MSG_MSOFC_TEXT_UNITS_MISMATCH ) ;
                   sprintf( szTempC2, szTempC1,ptrTgtParaCur->SeqNum, ptrTgtParaCur->NumTextTags, ptrSrcParaCur->NumTextTags ); 
                   MessageBoxA(HWND_DESKTOP, szTempC2, TITLE_MSOFC_TEXT_UNITS_MISMATCH, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
                   bReturn = FALSE ;
                   break ;
                }
             }
          }

          /*******************************************************************/
          /*  If paragraph contains no translatable text, then skip it.      */
          /*******************************************************************/
          if ( ptrSrcParaCur->NumTextTags == 0 ) 
             continue ;


          /*******************************************************************/
          /*  Copy end of previous paragraph block to the start of this      */
          /*  paragraph block.                                 (SOURCE COPY) */
          /*******************************************************************/
          ulToSrcFilePos = (ptrSrcParaCur->BlockStartPos) - 1*sizeof(WCHAR) ;
          if ( ptrTgtParaCur ) 
             bPreserveBlanks = ptrTgtParaCur->PreserveBlanks ;
          else
             bPreserveBlanks = FALSE ;
          fnWriteBlock( WRITE_SOURCE, 
                        ulFromSrcFilePos, 
                        ulToSrcFilePos, 
                        &OutFile, 
                        bPreserveBlanks ) ;

  
          /*******************************************************************/
          /*  If this paragraph has no run nodes, then the text block is     */
          /*  defined in this paragraph node.                  (TARGET COPY) */
          /*******************************************************************/
          if ( ptrSrcParaCur->ptrRunList == 0 ) {
             ulFromTgtFilePos = ptrTgtParaCur->BlockStartPos  ;
             ulToTgtFilePos = ptrTgtParaCur->BlockEndPos ;
             fnWriteBlock( WRITE_TARGET, 
                           ulFromTgtFilePos, 
                           ulToTgtFilePos, 
                           &OutFile, FALSE ) ;
             ulFromSrcFilePos = ptrSrcParaCur->BlockEndPos + 1*sizeof(WCHAR) ;

  
             continue ;
          }
  
  
          /*****************************************************************/
          /*  Handle special case where paragraph is only text nodes       */
          /*  which are all concatenated together.                         */
          /*****************************************************************/
          if ( ( ptrSrcParaCur->NumTextTags > 1     ) && 
               ( ptrSrcParaCur->NumNeutralTags == 0 ) &&
               ( ptrTgtParaCur->NumTextTags == 1    ) &&
               ( ! ptrTgtParaCur->ptrRunList        ) ) {
             for( ptrSrcRunCur=(R_INFO*)ptrSrcParaCur->ptrRunList ; 
                  ptrSrcRunCur ; 
                  ptrSrcRunCur=(R_INFO*)ptrSrcRunCur->ptrNext ) {
                if ( ( ptrSrcRunCur->ptrTextList ) ||
                     ( ptrSrcRunCur->TagId != TAG_ID_TEXT ) ||
                     ( ( ptrSrcRunCur != (R_INFO*)ptrSrcParaCur->ptrRunList ) &&
                       ( ptrSrcRunCur->Concat != CONCAT_TEXT_YES  ) ) )
                   break ;
             }
             if ( ! ptrSrcRunCur ) {                                
                ptrSrcRunCur=(R_INFO*)ptrSrcParaCur->ptrRunList ;  
                ulFromSrcFilePos = ptrSrcRunCur->StartPos  ;
                ulToSrcFilePos = ptrSrcRunCur->BlockStartPos - 1*sizeof(WCHAR) ;
                if ( ptrTgtParaCur ) 
                   bPreserveBlanks = ptrTgtParaCur->PreserveBlanks ;
                else
                   bPreserveBlanks = FALSE ;
                fnWriteBlock( WRITE_SOURCE, 
                              ulFromSrcFilePos, 
                              ulToSrcFilePos, 
                              &OutFile, 
                              bPreserveBlanks ) ;
  
                ulFromTgtFilePos = ptrTgtParaCur->BlockStartPos  ;
                ulToTgtFilePos = ptrTgtParaCur->BlockEndPos ;
                fnWriteBlock( WRITE_TARGET, 
                              ulFromTgtFilePos, 
                              ulToTgtFilePos, 
                              &OutFile, FALSE ) ;
                for( ; ptrSrcRunCur ; 
                     ptrRunTemp=ptrSrcRunCur, ptrSrcRunCur=(R_INFO*)ptrSrcRunCur->ptrNext ) ;
                ulFromSrcFilePos = ptrRunTemp->BlockEndPos + 1*sizeof(WCHAR) ;
                ulToSrcFilePos = ptrRunTemp->EndPos ;
                fnWriteBlock( WRITE_SOURCE, 
                              ulFromSrcFilePos, 
                              ulToSrcFilePos, 
                              &OutFile, FALSE ) ; 
  
                ulFromSrcFilePos = ptrSrcParaCur->BlockEndPos + 1*sizeof(WCHAR) ;
                continue ;
             }
          }
  
  
          /*****************************************************************/
          /*                                                               */
          /*  Process each run node within this paragraph.                 */
          /*                                                               */
          /*  For each target run node, find the corresponding source run  */
          /*  or text node.                                                */
          /*****************************************************************/
          for( ptrTgtRunCur=(R_INFO*)ptrTgtParaCur->ptrRunList ; 
               ptrTgtRunCur ; 
               ptrTgtRunCur=(R_INFO*)ptrTgtRunCur->ptrNext ) {
  
             /**************************************************************/
             /*  Loop through each source run node to find a match for     */
             /*  the current target run node.                              */
             /**************************************************************/
             for( ptrSrcRunCur=(R_INFO*)ptrSrcParaCur->ptrRunList ; 
                  ptrSrcRunCur ; 
                  ptrSrcRunCur=(R_INFO*)ptrSrcRunCur->ptrNext ) {
  
                if ( ( ptrTgtRunCur->TagType == TAG_TYPE_END ) &&
                     ( ptrSrcRunCur->BeginSeqNum > 0 ) )
                   ulSeqNum = ptrSrcRunCur->BeginSeqNum ;
                else 
                   ulSeqNum = ptrSrcRunCur->SeqNum ;
  
  
                /***********************************************************/
                /*  If this source run node has no text nodes, then see if */
                /*  this node matches target node.                         */
                /***********************************************************/
                if ( ptrSrcRunCur->ptrTextList == 0 ) {
  
                   if ( ulSeqNum != ptrTgtRunCur->SeqNum ) 
                      continue;
                   
                   /********************************************************/
                   /*  If run node defines text, then write the pre-source */
                   /*  text, target text, post-source text.                */
                   /********************************************************/
                   if ( ptrSrcRunCur->TagId == TAG_ID_TEXT ) {
                      ulFromSrcFilePos = ptrSrcRunCur->StartPos ;
                      ulToSrcFilePos = ptrSrcRunCur->BlockStartPos - 1*sizeof(WCHAR) ;
                      if ( ptrTgtRunCur ) 
                         bPreserveBlanks = ptrTgtRunCur->PreserveBlanks ;
                      else
                         bPreserveBlanks = FALSE ;
                      fnWriteBlock( WRITE_SOURCE, 
                                    ulFromSrcFilePos, 
                                    ulToSrcFilePos, 
                                    &OutFile,
                                    bPreserveBlanks ) ;

                      ulFromTgtFilePos = ptrTgtRunCur->BlockStartPos  ;
                      ulToTgtFilePos = ptrTgtRunCur->BlockEndPos ;
                      fnWriteBlock( WRITE_TARGET, 
                                    ulFromTgtFilePos, 
                                    ulToTgtFilePos, 
                                    &OutFile, FALSE ) ;

                      if ( sRunConcat ) {
                         for( ptrRunTemp=(R_INFO*)ptrSrcRunCur->ptrNext,bConcat=FALSE ;
                              ptrRunTemp && ptrRunTemp->Concat==CONCAT_TEXT_YES; 
                              ptrSrcRunCur=ptrRunTemp, ptrRunTemp=(R_INFO*)ptrRunTemp->ptrNext, bConcat=TRUE ) ;

                         /* Concat text may end with empty <w:t/> tag.  Adjust output  */
                         if ( ( bConcat ) &&
                              ( ptrSrcRunCur->BlockEndPos < ptrSrcRunCur->BlockStartPos ) ) { 
                            ulFromSrcFilePos = ptrSrcRunCur->BlockStartPos ;
                            ulToSrcFilePos = ulFromSrcFilePos + 1*sizeof(WCHAR);
                            fnReadBlock( WRITE_SOURCE, 
                                        ulFromSrcFilePos, ulToSrcFilePos, 
                                        szValue, sizeof(szValue) ) ;
                            if ( ! wcscmp( szValue, L"/>" ) ) {
                               ulFromSrcFilePos -= 4*sizeof(WCHAR) ;
                               fnReadBlock( WRITE_SOURCE, 
                                           ulFromSrcFilePos, ulToSrcFilePos, 
                                           szValue, sizeof(szValue) ) ;
                               if ( szValue[0] == L'<' ) {
                                  ptrSrcRunCur->BlockEndPos += 2*sizeof(WCHAR) ;
                                  wmemmove( &szValue[4], &szValue[5], wcslen(&szValue[5])+1 ) ;
                                  wmemmove( &szValue[2], &szValue[1], wcslen(&szValue[1])+1 ) ;
                                  szValue[1] = L'/' ;
                                  OutFile << szValue ;
                               }
                            }
                         }
                      }
                      ulFromSrcFilePos = ptrSrcRunCur->BlockEndPos + 1*sizeof(WCHAR) ;
                      ulToSrcFilePos = ptrSrcRunCur->EndPos ;
                      fnWriteBlock( WRITE_SOURCE, 
                                    ulFromSrcFilePos, 
                                    ulToSrcFilePos, 
                                    &OutFile, FALSE ) ;
                   } else {
  
                      /*****************************************************/
                      /*  If run node defines neutral tag, then write the  */
                      /*  source text.                                     */
                      /*****************************************************/
                      ulFromSrcFilePos = ptrSrcRunCur->StartPos ;
                      ulToSrcFilePos = ptrSrcRunCur->EndPos ;
                      fnWriteBlock( WRITE_SOURCE, 
                                    ulFromSrcFilePos, 
                                    ulToSrcFilePos, 
                                    &OutFile, FALSE ) ;
                      ulFromSrcFilePos = ptrSrcRunCur->EndPos + 1*sizeof(WCHAR) ;
  

                   }
  
                   break ;
                }            /* End no source text nodes */
  

                if ( ulSeqNum == ptrTgtRunCur->SeqNum ) {
                   if ( ptrTgtRunCur->TagType == TAG_TYPE_END ) {
                      ulFromSrcFilePos = ptrSrcRunCur->BlockEndPos + 1*sizeof(WCHAR) ;
                      ulToSrcFilePos = ptrSrcRunCur->EndPos ;
                   } else {
                      ulFromSrcFilePos = ptrSrcRunCur->StartPos ;
                      ulToSrcFilePos = ptrSrcRunCur->BlockStartPos - 1*sizeof(WCHAR) ;
                   }
                   fnWriteBlock( WRITE_SOURCE, 
                                 ulFromSrcFilePos, 
                                 ulToSrcFilePos, 
                                 &OutFile, FALSE ) ;
                   break ;
  
                }
  
                for( ptrSrcTextCur=(T_INFO*)ptrSrcRunCur->ptrTextList ; 
                     ptrSrcTextCur ; 
                     ptrSrcTextCur=(T_INFO*)ptrSrcTextCur->ptrNext ) {

                   if ( ( ptrTgtRunCur->TagType == TAG_TYPE_END ) &&
                        ( ptrSrcTextCur->BeginSeqNum > 0 ) )
                      ulSeqNum = ptrSrcTextCur->BeginSeqNum ;
                   else 
                      ulSeqNum = ptrSrcTextCur->SeqNum ;



                   if ( ulSeqNum == ptrTgtRunCur->SeqNum ) {

                      /*********************************************************/
                      /*  If text node defines text, then write the pre-source */
                      /*  text, target text, post-source text.                 */
                      /*********************************************************/
                      if ( ptrSrcTextCur->TagId == TAG_ID_TEXT ) {
                         ulFromSrcFilePos = ptrSrcTextCur->StartPos ;
                         ulToSrcFilePos = ptrSrcTextCur->BlockStartPos - 1*sizeof(WCHAR) ;
                         if ( ptrTgtRunCur ) 
                            bPreserveBlanks = ptrTgtRunCur->PreserveBlanks ;
                         else
                            bPreserveBlanks = FALSE ;
                         fnWriteBlock( WRITE_SOURCE, 
                                       ulFromSrcFilePos, 
                                       ulToSrcFilePos, 
                                       &OutFile,
                                       bPreserveBlanks ) ;

                         ulFromTgtFilePos = ptrTgtRunCur->BlockStartPos  ;
                         ulToTgtFilePos = ptrTgtRunCur->BlockEndPos ;
                         fnWriteBlock( WRITE_TARGET, 
                                       ulFromTgtFilePos, 
                                       ulToTgtFilePos, 
                                       &OutFile, FALSE ) ;

                         ulFromSrcFilePos = ptrSrcTextCur->BlockEndPos + 1*sizeof(WCHAR) ;
                         ulToSrcFilePos = ptrSrcTextCur->EndPos ;
                         fnWriteBlock( WRITE_SOURCE, 
                                       ulFromSrcFilePos, 
                                       ulToSrcFilePos, 
                                       &OutFile, FALSE ) ;

                      } else {
  
                         if ( ( ptrSrcRunCur->ptrTextList == ptrSrcTextCur ) &&         /* 9-22-08 */
                              ( ptrSrcRunCur->StartPos > 0 ) &&
                              ( ptrSrcRunCur->BlockStartPos == 0 ) &&
                              ( ptrSrcRunCur->StartPos < ptrSrcTextCur->StartPos ) ) {
                            ulFromSrcFilePos = ptrSrcRunCur->StartPos ;
                            ulToSrcFilePos = ptrSrcTextCur->StartPos-1*sizeof(WCHAR) ;
                            fnWriteBlock( WRITE_SOURCE, 
                                          ulFromSrcFilePos, 
                                          ulToSrcFilePos, 
                                          &OutFile, FALSE ) ;
                         }
  
                         ulFromSrcFilePos = ptrSrcTextCur->StartPos ;
                         ulToSrcFilePos = ptrSrcTextCur->EndPos ;
                         fnWriteBlock( WRITE_SOURCE, 
                                       ulFromSrcFilePos, 
                                       ulToSrcFilePos, 
                                       &OutFile, FALSE ) ;

                         if ( ( ptrSrcTextCur->ptrNext == NULL ) &&        
                              ( ptrSrcRunCur->EndPos > 0 ) &&
                              ( ptrSrcRunCur->BlockEndPos == 0 ) &&
                              ( ptrSrcRunCur->EndPos > ptrSrcTextCur->EndPos ) ) {
                            ulFromSrcFilePos = ptrSrcTextCur->EndPos + 1*sizeof(WCHAR) ;
                            ulToSrcFilePos = ptrSrcRunCur->EndPos ;
                            fnWriteBlock( WRITE_SOURCE, 
                                          ulFromSrcFilePos, 
                                          ulToSrcFilePos, 
                                          &OutFile, FALSE ) ;
                            ulFromSrcFilePos = ptrSrcRunCur->EndPos + 1*sizeof(WCHAR) ;
                         } else {
                            ulFromSrcFilePos = ptrSrcTextCur->EndPos + 1*sizeof(WCHAR) ;
                         }
      
                      }
                      break ;
                   } 
                }         /* End source text node loop */
             }            /* End run node loop */

             if ( ( ! ptrSrcRunCur ) &&                 
                  ( ptrTgtRunCur->Tag ) &&
                  ( ! wcscmp( ptrTgtRunCur->Tag, L"INSERT" ) ) ) {
                OutFile << L"<w:r><w:t>" ;   
                ulFromTgtFilePos = ptrTgtRunCur->StartPos  ;
                ulToTgtFilePos = ptrTgtRunCur->EndPos ;
                fnWriteBlock( WRITE_TARGET, 
                              ulFromTgtFilePos, 
                              ulToTgtFilePos, 
                              &OutFile, FALSE ) ;
                OutFile << L"</w:t></w:r>" ;   

             }
          }
          ulFromSrcFilePos = ptrSrcParaCur->BlockEndPos + 1*sizeof(WCHAR) ;
       }
  
       /*****************************************************************/
       /*  Copy from end of last paragraph block to the end of the      */
       /*  file.                                          (SOURCE COPY) */
       /*****************************************************************/
       ulToSrcFilePos = 0 ;                 /* Indicate to end of file  */
       fnWriteBlock( WRITE_SOURCE, 
                     ulFromSrcFilePos, 
                     ulToSrcFilePos, 
                     &OutFile, FALSE ) ;
       }
    }
 
 
    /***********************************************************************/
    /*  Free linked list space.                                            */
    /***********************************************************************/
    fnFreeParaList( &ptrSrcParaHead ) ;
    fnFreeParaList( &ptrTgtParaHead ) ;
 
    (*InputFile).freefile() ;
    (*InputFile2).freefile() ;
    OutFile.close() ;
 
 
    /***********************************************************************/
    /*  SpreadSheets.  Determine if there are shared strings which         */
    /*  are defined as both translatable and non-translatable.  If so,     */
    /*  then modify non-translatable instances to be hard-coded text       */
    /*  rather than using the shared string.                               */
    /***********************************************************************/
    if ( ptrSrcSSNTHead ) {
       fnUpdateSpreadsheetNonTrans( out, in, ptrSrcSSNTHead ) ;
    }
    fnFreeSSNTList( &ptrSrcSSNTHead ) ;
    fnFreeSSNTList( &ptrTgtSSNTHead ) ;

    return(bReturn);

}/* PostExport */









/****************************************************************************/
/*                                                                          */
/* fnCreateDbcsList                                                         */
/*                                                                          */
/* Create a linked list containing the necessary information for exporting  */
/* this DBCS file.                                                          */
/*                                                                          */
/* Input:      None.                                                        */
/* Output:     ptrList    - Pointer to start of list.                       */
/* Return:     0          - List created successfully.                      */
/*             1          - Failed to create the list.                      */
/*                                                                          */
/****************************************************************************/

USHORT fnCreateDbcsList( DBCS_INFO** ptrList ) 
{

    DBCS_INFO    *ptrDbcsHead = 0 ;
    DBCS_INFO    *ptrDbcsTail = 0 ;
  
    WCHAR     szIn[MAX_XML_RCD_LENGTH2*2] ;
    WCHAR     szTag[XML_TAG_LEN] ;
    WCHAR     szTagText[MAX_XML_RCD_LENGTH2*2] ;
    WCHAR     *ptrChar ; 
  
    ULONG     ulFilePos = 0 ;
    ULONG     ulTagStartPos = 0 ;
    ULONG     ulTagEndPos = 0 ;
    ULONG     i, j ;
    USHORT    rc ;
    USHORT    usReturn = 0 ;
    USHORT    usDbcsState = DBCS_STATE_NONE ;
  
    BOOL      bForever = TRUE ;
    BOOL      bNewNode = FALSE ;
    BOOL      bPartialTag = FALSE ;

  
    szIn[0] = NULL ;
    szTag[0] = NULL ;
    i = 0 ;


    /***********************************************************************/
    /*  Read through the file to determine where the text is defined.      */
    /***********************************************************************/
    while ( bForever ) {


       /***********************************************************************/
       /*  Get next tag from this line.                                       */
       /***********************************************************************/
       rc = fnGetXMLTag( szIn, &i, &ulFilePos, szTag, szTagText,
                         &ulTagStartPos, &ulTagEndPos, &bPartialTag ) ;
       if ( rc == 1 ) {
          MessageBoxA(HWND_DESKTOP, MSG_MSOFC_PARSING_ERROR, TITLE_MSOFC_PARSING_ERROR, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
          usReturn = 1 ;
          break ;
       }
       if ( rc == 2 ) {    /* End of file */
          break ;
       }

       /*====================================================================*/
       /*                                                                    */
       /*  Handle start of DBCS-related sections.                            */
       /*                                                                    */
       /*====================================================================*/
       if ( usDbcsState == DBCS_STATE_NONE ) {

          /*******************************************************************/
          /*  Handle start of document fonts <w:fonts>                       */
          /*******************************************************************/
          if ( ! wcscmp( szTag, L"w:fonts" ) ) {

             /* ----  Allocate a DBCS node.  ------------------------------- */
             fnAllocateDbcsNode( &ptrDbcsHead, &ptrDbcsTail, szTag ) ;

             /* ----  Initialize the DBCS node.  --------------------------- */
             usDbcsState = DBCS_STATE_FONTS ;             /* Set DBCS state  */
             ptrDbcsTail->NodeType  = DBCS_STATE_FONTS ; 
             ptrDbcsTail->StartPos = ulTagEndPos + 1*sizeof(WCHAR); 
         
             continue ;
          } 

          /*******************************************************************/
          /*  Handle start of run block fonts <w:rFonts>                     */
          /*******************************************************************/
          if ( ! wcscmp( szTag, L"w:rFonts" ) ) {

             /* ----  Allocate a DBCS node.  ------------------------------- */
             fnAllocateDbcsNode( &ptrDbcsHead, &ptrDbcsTail, szTag ) ;

             /* ----  Initialize the DBCS node.  --------------------------- */
             ptrDbcsTail->NodeType  = DBCS_STATE_RFONTS ; 
             ptrDbcsTail->StartPos = ulTagStartPos ;

             if ( wcschr( szTagText, L'/' ) ) {
                usDbcsState = DBCS_STATE_NONE ;    /* In empty tag           */
                ptrDbcsTail->EndPos = ulTagEndPos ;
             } else {
                usDbcsState = DBCS_STATE_RFONTS ;            /* Set DBCS state  */
             }
         
             continue ;
          } 

          /*******************************************************************/
          /*  Handle start of run block fonts <wx:font>                      */
          /*******************************************************************/
          if ( ! wcscmp( szTag, L"wx:font" ) ) {

             /* ----  Allocate a DBCS node.  ------------------------------- */
             fnAllocateDbcsNode( &ptrDbcsHead, &ptrDbcsTail, szTag ) ;

             /* ----  Initialize the DBCS node.  --------------------------- */
             ptrDbcsTail->NodeType  = DBCS_STATE_XFONT ; 
             ptrDbcsTail->StartPos = ulTagStartPos ;

             if ( wcschr( szTagText, L'/' ) ) {
                usDbcsState = DBCS_STATE_NONE ;    /* In empty tag           */
                ptrDbcsTail->EndPos = ulTagEndPos ;
             } else {
                usDbcsState = DBCS_STATE_XFONT ;             /* Set DBCS state  */
             }
         
             continue ;
          } 

          continue ;
       } 

       /**********************************************************************/
       /*  Handle end of document fonts <w:fonts>                            */
       /**********************************************************************/
       if ( usDbcsState == DBCS_STATE_FONTS ) {

          if ( ! wcscmp( szTag, L"/w:fonts" ) ) {

             /* ----  Initialize the DBCS node.  --------------------------- */
             usDbcsState = DBCS_STATE_NONE ;          /* Reset DBCS state    */
             ptrDbcsTail->EndPos = ulTagStartPos - 1*sizeof(WCHAR) ;
          } 

          continue ;
       } 

       /**********************************************************************/
       /*  Handle end of run fonts <w:rFonts>                                */
       /**********************************************************************/
       if ( usDbcsState == DBCS_STATE_RFONTS ) {

          if ( ! wcscmp( szTag, L"/w:rFonts" ) ) {

             /* ----  Initialize the DBCS node.  --------------------------- */
             usDbcsState = DBCS_STATE_NONE ;          /* Reset DBCS state    */
             ptrDbcsTail->EndPos = ulTagEndPos ;
         
          } 

          continue ;
       } 

       /**********************************************************************/
       /*  Handle end of run fonts <wx:font>                                 */
       /**********************************************************************/
       if ( usDbcsState == DBCS_STATE_XFONT ) {

          if ( ! wcscmp( szTag, L"/w:rFonts" ) ) {

             /* ----  Initialize the DBCS node.  --------------------------- */
             usDbcsState = DBCS_STATE_NONE ;          /* Reset DBCS state    */
             ptrDbcsTail->EndPos = ulTagEndPos ;
         
          } 

          continue ;
       } 


    }  /* End reading file for parsing */

    *ptrList = ptrDbcsHead ; 

    return( usReturn ) ;
}





/****************************************************************************/
/*                                                                          */
/* fnAllocateDbcsNode                                                       */
/*                                                                          */
/* Allocate storage for a new DBCS node.                                    */
/*                                                                          */
/* Input:      Tag       - Tag defined by this node.                        */
/* In/Out:     ptrHead   - The first node in the linked list.               */
/*             ptrTail   - The last node in the linked list.                */
/* Return:     TRUE      - Node allocated.                                  */
/*             FALSE     - Allocation failure                               */
/*                                                                          */
/****************************************************************************/

BOOL fnAllocateDbcsNode( DBCS_INFO** ptrHead, DBCS_INFO** ptrTail, 
                         WCHAR *Tag ) 
{
   DBCS_INFO    *ptrNew ;
   BOOL         bReturn = FALSE ;

   ptrNew = (DBCS_INFO*)calloc( sizeof(DBCS_INFO), 1 ) ;
   if ( ptrNew ) {
      bReturn = TRUE ;
      if ( *ptrHead ) {
         (*ptrTail)->ptrNext = ptrNew ;
         ptrNew->ptrPrev = *ptrTail ;
      } else {
         *ptrHead = ptrNew ;
      }
      *ptrTail = ptrNew ; 
   }

   if ( Tag[0] != NULL ) {
      (*ptrTail)->Tag = (WCHAR*)malloc( (wcslen(Tag)+1) * sizeof(WCHAR) ) ;
      wcscpy( (*ptrTail)->Tag, Tag ) ;
   }

   return( bReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnFreeDbcsList                                                           */
/*                                                                          */
/* Free all of the storage used in the DBCS list.                           */
/*                                                                          */
/****************************************************************************/

VOID fnFreeDbcsList( DBCS_INFO** ptrList ) 
{
   DBCS_INFO    *ptrDbcsHead ;
   DBCS_INFO    *ptrDbcsCur ;



    /***********************************************************************/
    /*  Free linked list space.                                            */
    /***********************************************************************/
    ptrDbcsHead = *ptrList ; 

    while ( ptrDbcsHead ) {

       if ( ptrDbcsHead->Tag ) 
          free( ptrDbcsHead->Tag ) ;

       ptrDbcsCur = (DBCS_INFO*)ptrDbcsHead->ptrNext ;
       free( ptrDbcsHead ) ;
       ptrDbcsHead = ptrDbcsCur ;
    }

    *ptrList = 0; 
}




/****************************************************************************/
/*                                                                          */
/* fnGetDbcsBlock                                                           */
/*                                                                          */
/* Get a blockof DBCS text to process.                                      */
/*                                                                          */
/* Input:      StartPos  - Starting position to read from.                  */
/*             EndPos    - Ending position to read from.                    */
/* Output:     Block     - DBCS text.                                       */
/* Return:     0         - Block read OK.                                   */
/*             1         - Failure.                                         */
/*                                                                          */
/****************************************************************************/

USHORT fnGetDbcsBlock( ULONG StartPos, ULONG EndPos, WCHAR *szBlock )
{
   WCHAR     szIn[MAX_XML_RCD_LENGTH2*2] ;
   ULONG     ulBlockLen ;
   ULONG     ulReadLen ;
   ULONG     ulFilePos ;
   ULONG     i ;
   USHORT    usReturn = 0 ;
   BOOL      bReadEOF = FALSE ;

   szBlock[0] = NULL ;
   if ( EndPos == 0 ) {
      bReadEOF = TRUE ;
   } else {
      if ( StartPos > EndPos ) {
         usReturn = 1 ;
         ulBlockLen = 0 ;
      } else {
         ulBlockLen = ( EndPos - StartPos ) / sizeof(WCHAR) + 1 ;
      }
   }
   (*InputFile).fseekt(StartPos, std::ios::beg) ;

   ulReadLen = ulBlockLen ;
   szPrevXMLInputText[0] = NULL ;

   while( ( usReturn == 0  ) &&
          ( ( ulReadLen > 0 ) ||
            ( bReadEOF       ) ) ) {
      if ( fnGetXMLRcd( szIn, &ulFilePos ) ) {
         wcscat( szBlock, szIn ) ;
         i = wcslen( szBlock ) ;
         if ( ( i > ulReadLen ) &&
              ( ! bReadEOF     ) ) {
            i = ulReadLen ;
            szBlock[i] = NULL ;
         }
         ulReadLen -= i ; 
      } else {
         usReturn = 1 ;
      }
   }

   return( usReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnAddTagAttr                                                             */
/*                                                                          */
/* Add/replace a tag attribute.                                             */
/*                                                                          */
/* Input:      szTag     - Tag text to be changed.                          */
/*             szAttr    - Attribute value to be added or changed.          */
/*             szValue   - Attribute value to be set.                       */
/*             usCond    - 0= Add attr or change existing value.            */
/*                         1= Only change existing value.                   */
/*                         2= Only add attr if it does not already exist.   */
/* Output:     szTag     - Updated tag text.                                */
/* Return:     n/a                                                          */
/*                                                                          */
/****************************************************************************/

VOID fnAddTagAttr( WCHAR *szTag, WCHAR *szAttr, WCHAR *szValue, USHORT usCond )
{
   WCHAR     szScanAttr[80] ;   
   WCHAR     AttrQuote ;
   WCHAR     *ptrAttr ;
   WCHAR     *ptr, *ptr2 ;
   USHORT    usValueLen ; 
   BOOL      bDone = 0 ;

   szScanAttr[0] = L' ' ;
   wcscpy( &szScanAttr[1], szAttr ) ;
   usValueLen = wcslen( szValue ) ;

   /***********************************************************************/
   /*  Update attribute value if it already exists.                      */
   /***********************************************************************/
   for( ptrAttr=wcsstr(szTag,szScanAttr ) ;  
        ptrAttr ; 
        ptrAttr=wcsstr(ptrAttr+1,szScanAttr ) ) {
      ptr = ptrAttr ;
      for( ptr+=wcslen(szScanAttr) ; *ptr && iswspace(*ptr) ; ++ptr ) ;
      if ( *ptr == L'=' ) {
         ++ptr ;
         for( ptr2=0 ; *ptr && iswspace(*ptr) ; ++ptr ) ;
         if ( ( *ptr == L'\'' ) ||
              ( *ptr == L'\"' ) ) {
            AttrQuote = *ptr ;
            ptr2 = ptr + 1;
            for( ; *ptr2 && (*ptr2!=AttrQuote) ; ++ptr2 ) ;
         } 
         if ( ( ptr2 ) &&
              ( ( usCond == COND_ADDATTR_ALWAYS  ) ||
                ( usCond == COND_ADDATTR_CHGONLY ) ) ) {
            if ( ( ptr2-ptr-1) != usValueLen ) 
               wmemmove( ptr+usValueLen+1, ptr2 , wcslen(ptr2)+1 ) ;
            wcsncpy( ptr+1, szValue, usValueLen ) ;
            bDone = TRUE ;
         }
      }
   }

   /***********************************************************************/
   /*  Add attribute when it does not already exist.                      */
   /***********************************************************************/
   if ( ( ! bDone ) &&
        ( ( usCond == COND_ADDATTR_ALWAYS  ) ||
          ( usCond == COND_ADDATTR_NEWONLY ) ) ) {
      ptr = szTag + wcslen(szTag)- 1 ;
      if ( *(ptr-1) == L'/' ) 
         --ptr ;
      wmemmove( ptr+wcslen(szAttr)+usValueLen+4, ptr, wcslen(ptr)+1 ) ;
      *(ptr++) = L' ' ;
      wcsncpy( ptr, szAttr, wcslen(szAttr) ) ;
      ptr += wcslen(szAttr) ; 
      *(ptr++) = L'=' ;
      *(ptr++) = L'\"' ;
      wcsncpy( ptr, szValue, usValueLen ) ;
      ptr += wcslen(szValue) ;
      *(ptr) = L'\"' ;
   }
   return ;
}




/****************************************************************************/
/*                                                                          */
/* fnRemoveTagAttr                                                          */
/*                                                                          */
/* Remove a tag attribute.                                                  */
/*                                                                          */
/* Input:      szTag     - Tag text to be changed.                          */
/*             szAttr    - Attribute value to be added or changed.          */
/* Output:     szTag     - Updated tag text.                                */
/* Return:     n/a                                                          */
/*                                                                          */
/****************************************************************************/

VOID fnRemoveTagAttr( WCHAR *szTag, WCHAR *szAttr )
{
   WCHAR     szScanAttr[80] ;   
   WCHAR     AttrQuote ;
   WCHAR     *ptrAttr ;
   WCHAR     *ptr, *ptr2 ;

   szScanAttr[0] = L' ' ;
   wcscpy( &szScanAttr[1], szAttr ) ;

   /***********************************************************************/
   /*  Remove attribute, if it already exists.                            */
   /***********************************************************************/
   for( ptrAttr=wcsstr(szTag,szScanAttr ) ;  
        ptrAttr ; 
        ptrAttr=wcsstr(ptrAttr+1,szScanAttr ) ) {
      ptr = ptrAttr ;
      for( ptr+=wcslen(szScanAttr) ; *ptr && iswspace(*ptr) ; ++ptr ) ;
      if ( *ptr == L'=' ) {
         ++ptr ;
         for( ptr2=0 ; *ptr && iswspace(*ptr) ; ++ptr ) ;
         if ( ( *ptr == L'\'' ) ||
              ( *ptr == L'\"' ) ) {
            AttrQuote = *ptr ;
            ptr2 = ptr + 1;
            for( ; *ptr2 && (*ptr2!=AttrQuote) ; ++ptr2 ) ;
         } 
         if ( ptr2 ) {
            for( ptr=ptrAttr-1 ; ptr>szTag && iswspace(*ptr) ; --ptr ) ;
            wmemmove( ptr+1, ptr2+1, wcslen(ptr2+1)+1 ) ;
         }
      }
   }
   return ;
}

/*****************************************************************************/
/*                                                                          */
/*  fnWriteReferenceText                                                     */
/*                                                                           */
/*  Find and write translated reference text.                                */
/*                                                                           */
/* Input:      ptrRefCur   - Ptr to current source reference node.           */
/*             ptrSrcHead  - Head of source paragraph linked list.           */
/*             ptrTgtHead  - Head of target paragraph linked list.           */
/* Output:                 - Write target reference text.                    */
/* Return:     n/a                                                           */
/*****************************************************************************/

BOOL fnWriteReferenceText( P_INFO *ptrRefCur, P_INFO *ptrSrcHead,
                           P_INFO *ptrTgtHead, wofstream *OutFile )
{

    P_INFO     *ptrSrcCur ;
    P_INFO     *ptrTgtCur ;

    WCHAR      szRefValue[MAX_XML_RCD_LENGTH] ;
    WCHAR      szValue[MAX_XML_RCD_LENGTH] ;
    ULONG      ulFromFilePos ;
    ULONG      ulToFilePos ;
    ULONG      ulRefLen ;
    ULONG      ulRefBytes ;
    ULONG      ulRefId ;
    USHORT     rc ;
    BOOL       bSheetReference = FALSE ;
    BOOL       bMatch = FALSE ;
    BOOL       bQuoted = FALSE ;
    BOOL       bReturn = TRUE;


    ulFromFilePos = ptrRefCur->BlockStartPos ;
    ulToFilePos = ptrRefCur->BlockEndPos ;
    fnReadBlock( WRITE_SOURCE, 
                 ulFromFilePos, ulToFilePos+1*sizeof(WCHAR), 
                 szRefValue, sizeof(szRefValue) ) ;
    ulRefLen = wcslen(szRefValue) ;
    if ( ulRefLen ) {
       if ( szRefValue[ulRefLen-1] == L'!' ) 
          bSheetReference = TRUE ;
       szRefValue[ulRefLen-1] = NULL ;
       ulRefLen = wcslen(szRefValue) ;
    }
    if ( ( ulRefLen > 2 ) &&
         ( szRefValue[0] == L'\'' ) &&
         ( szRefValue[ulRefLen-1] == L'\'' ) ) {
       bQuoted = TRUE ;
       szRefValue[ulRefLen-1] = NULL ;
       wmemmove( szRefValue, &szRefValue[1], ulRefLen ) ;
    }
    ulRefLen = wcslen(szRefValue) ;
    ulRefBytes = ulRefLen * sizeof(WCHAR) ;
    szValue[0] = NULL ;
  
    /**********************************************************************/
    /*                                                                    */
    /*  Process each paragraph node to find matching text.                */
    /*                                                                    */
    /**********************************************************************/
    for( ptrSrcCur=ptrSrcHead ;
         ptrSrcCur ; 
         ptrSrcCur=(P_INFO*)ptrSrcCur->ptrNext ) {
       if ( ( ptrSrcCur->NodeType == NODE_TYPE_FILE_NAME ) ||
            ( ptrSrcCur->NodeType == NODE_TYPE_SHEET_REF ) ||
            ( ptrSrcCur->NodeType == NODE_TYPE_TEXT_REF  ) ) 
          continue ;
       if ( ( ptrRefCur->NodeType == NODE_TYPE_SHEET_REF ) &&
            ( wcscmp( ptrSrcCur->Tag, L"sheet" ) ) )
          continue ;
       if ( ptrSrcCur->BlockEndPos-ptrSrcCur->BlockStartPos+1*sizeof(WCHAR) == ulRefBytes ) {
          ulFromFilePos = ptrSrcCur->BlockStartPos ;
          ulToFilePos = ptrSrcCur->BlockEndPos ;
          fnReadBlock( WRITE_SOURCE, 
                      ulFromFilePos, ulToFilePos, 
                      szValue, sizeof(szValue) ) ;
          if ( ! wcscmp( szValue, szRefValue ) ) {
             bMatch = TRUE ;
             break ;
          }
          szValue[0] = NULL ;
       }
    }
    if ( ptrSrcCur ) {
       ulRefId = ptrSrcCur->SeqNum ; 
       for( ptrTgtCur=ptrTgtHead ; 
            ptrTgtCur && ptrTgtCur->SeqNum != ulRefId ; 
            ptrTgtCur=(P_INFO*)ptrTgtCur->ptrNext ) ;
       if ( ptrTgtCur ) {
          ulFromFilePos = ptrTgtCur->BlockStartPos ;
          ulToFilePos = ptrTgtCur->BlockEndPos ;
          fnReadBlock( WRITE_TARGET, 
                      ulFromFilePos, ulToFilePos, 
                      szValue, sizeof(szValue) ) ;
       }
    }
  
    if ( wcslen(szValue) >= 1 ) 
       wcscpy( szRefValue, szValue ) ;
  
    if ( ( ( !wcscmp( ptrRefCur->Tag, L"c:f"         ) ) ||
           ( !wcscmp( ptrRefCur->Tag, L"f"           ) ) ||
           ( !wcscmp( ptrRefCur->Tag, L"definedName" ) ) ) &&
         ( bSheetReference ) &&
         ( bMatch || bQuoted ) ) {
       wmemmove( &szRefValue[1], szRefValue, wcslen(szRefValue)+1 ) ;
       szRefValue[0] = L'\'' ;
       wcscat( szRefValue, L"\'" ) ;
    }

    *OutFile << szRefValue ;   

    return(bReturn);

}





/****************************************************************************/
/*                                                                          */
/* fnUpdateSpreadsheetNonTrans                                              */
/*                                                                          */
/* If there is a shared string which is defined both as translatable and    */
/* non-translatable, then change the non-translatable references so that    */
/* the text is defined directly in the worksheet rather than using the      */
/* shared string value.                                                     */
/*                                                                          */
/* For example, if the following cell references the shared string "option" */
/* and it can be both translatable and non-translatable:                    */
/*    <c r="A1" s="1" t="s">                                                */
/*      <v>1261</v>                                                         */
/*    </c>                                                                  */
/* Then if this is translatable cell, then change nothing.                  */
/* Then if this is a non-translatable cell, then change this to:            */
/*    <c r="A1" s="1">                                                      */
/*      <is><t>option</t></is>                                              */
/*    </c>                                                                  */
/*                                                                          */
/****************************************************************************/

BOOL fnUpdateSpreadsheetNonTrans( PSZ In, PSZ Work, SSNT_INFO* ptrCells ) 
{
#define SSNT_STATE_NONE           0
#define SSNT_STATE_WORKSHEET      1
#define SSNT_STATE_COLS           2
#define SSNT_STATE_ROW            3
#define SSNT_STATE_CELL           4
#define MAX_COLS                200

   SSNT_INFO *ptrStylesHead = 0 ;
   SSNT_INFO *ptrStylesCur = 0 ;

   SSNT_INFO *ptrStyleXfsHead = 0 ;
   SSNT_INFO *ptrStyleXfsCur = 0 ;

   SSNT_INFO *ptrNTCellHead = 0 ;
   SSNT_INFO *ptrNTCellPrev = 0 ;
   SSNT_INFO *ptrTRCellHead = 0 ;
   SSNT_INFO *ptrTRCellPrev = 0 ;
   SSNT_INFO *ptrCellHead = 0 ;
   SSNT_INFO *ptrCellPrev = 0 ;
   SSNT_INFO *ptrCellCur  = 0 ;
   SSNT_INFO *ptrCellNew  = 0 ;

   wofstream TempFile ( Work ) ;
//   wofstream  *TempFile ;

   WCHAR     szIn[MAX_XML_RCD_LENGTH2*2] ;
   WCHAR     szTag[XML_TAG_LEN] ;
   WCHAR     szTagText[MAX_XML_RCD_LENGTH2*2] ;
   WCHAR     szValue[MAX_XML_RCD_LENGTH] ;
   WCHAR     *ptrChar, *ptrChar2 ;
   ULONG     ulTagStartPos = 0 ;
   ULONG     ulTagEndPos = 0 ;
   ULONG     ulFilePos = 0 ;
   ULONG     ulColMin, ulColMax ;
   ULONG     ulTemp ;
   ULONG     i, k ;
   LONG      lStyleCount ;
   USHORT    usColsNonTrans[MAX_COLS] = { 0 } ;
   USHORT    usState ;
   USHORT    usCol ;
   USHORT    rc ;
   BOOL      bPartialTag = FALSE ;
   BOOL      bInStyles = FALSE ;
   BOOL      bInXfs = FALSE ;
   BOOL      bInWorksheet = FALSE ;
   BOOL      bInCols = FALSE ;
   BOOL      bInRow = FALSE ;
   BOOL      bInCell = FALSE ;
   BOOL      bTransCell = FALSE ;
   BOOL      bSkipSSRef = FALSE ;
   BOOL      bForever = TRUE ;
   BOOL      bReturn = TRUE ;

    ptrNTCellHead = ptrCells ; 

    /***********************************************************************/
    /*  Determine if there are any strings defined as both translatable    */
    /*  and non-translatable.  If not, do nothing.                         */
    /***********************************************************************/
    for( ptrCellCur=ptrNTCellHead ; ptrCellCur ; ptrCellCur=(SSNT_INFO*)ptrCellCur->ptrNext ) {
       if ( ( ptrCellCur->bTrans   ) && 
            ( ptrCellCur->szString ) )
          break;
    }

    if ( ! ptrCellCur ) {
       return( 0 ) ;
    }

    InputFile = new wifstream( In ) ;


   /***********************************************************************/
   /*  Find the <cellStyle> element(s) which define "CN INTERNAL" styles. */
   /*                                                                     */
   /*  <cellStyles count="nn">                                            */
   /*    <cellStyle name="CN Internal Note Compact" xfId="6"/>            */
   /*  </cellStyles>                                                      */
   /*                                                                     */
   /***********************************************************************/
   bInStyles = FALSE ;
   szIn[0] = NULL ;
   szTag[0] = NULL ;
   szPrevXMLInputText[0] = NULL ;                     
   ulFilePos = 0 ;
   i = 0 ;
   while ( bReturn ) {
      rc = fnGetXMLTag( szIn, &i, &ulFilePos, szTag, szTagText,
                        &ulTagStartPos, &ulTagEndPos, &bPartialTag ) ;
      if ( rc == 1 ) {
         bReturn = FALSE ;
      }
      if ( rc == 2 ) {    /* End of file */
         break;
      }
       
      if ( ! wcscmp( szTag, L"cellStyles" ) ) {
         bInStyles = TRUE ;
      }

      if ( bInStyles ) {
         if ( ( ! wcscmp( szTag, L"cellStyle" ) ) &&
              ( ( wcsstr( szTagText, L"CNInternal"  ) ) ||
                ( wcsstr( szTagText, L"CN Internal" ) ) ||
                ( wcsstr( szTagText, L"DNT"         ) ) ||
                ( wcsstr( szTagText, L"dnt"         ) ) ) ) {
            if ( ( fnGetAttributeValue( szTagText, L"xfId", szValue ) ) &&
                 ( iswdigit( szValue[0] ) ) ) {
               ptrStylesCur = (SSNT_INFO*)calloc( sizeof(SSNT_INFO), 1 ) ;
               ptrStylesCur->ptrNext = ptrStylesHead ;
               ptrStylesCur->ulData = _wtoi( szValue ) ;
               ptrStylesHead = ptrStylesCur ; 
            }
         }
         if ( ! wcscmp( szTag, L"/cellStyles" ) ) {
            bInStyles = FALSE ;
            if ( ! ptrStylesHead ) 
               bReturn = FALSE ;
            break ;
         }
      }
   }

    (*InputFile).fseekt(0, std::ios::beg) ;


    /***********************************************************************/
    /*  Using the "xfId"s from the previously found <cellStyles> elements, */
    /*  find the <cellXfs> element(s) which have the same "xfID"s.         */
    /*  The zero-based index into <cellXfs> is the style number which is   */
    /*  referenced in the "s" attribute of the <c> element.                */
    /*                                                                     */
    /*  <cellXfs count="nn">                                               */
    /*    <xf numFmtId="0" fontId="618" fillId="0" borderId="0" xfId="6"/> */
    /*  </cellXfs>                                                         */
    /*                                                                     */
    /***********************************************************************/
    bInXfs = FALSE ;
    bPartialTag = FALSE ;
    szIn[0] = NULL ;
    szTag[0] = NULL ;
    szPrevXMLInputText[0] = NULL ;                       
    ulFilePos = 0 ;
    lStyleCount = 0 ;
    i = 0 ;
    while ( bReturn ) {
       rc = fnGetXMLTag( szIn, &i, &ulFilePos, szTag, szTagText,
                         &ulTagStartPos, &ulTagEndPos, &bPartialTag ) ;
       if ( rc == 1 ) {
          bReturn = FALSE ;
       }
       if ( rc == 2 ) {    /* End of file */
          break;
       }

       if ( ! wcscmp( szTag, L"cellXfs" ) ) {
          bInXfs = TRUE ;
          lStyleCount = -1 ;                  /* Zero-based index        */
       }

       if ( bInXfs ) {
          if ( ! wcscmp( szTag, L"xf" ) ) {
             ++lStyleCount ;
             if ( ( fnGetAttributeValue( szTagText, L"xfId", szValue ) ) &&
                  ( iswdigit( szValue[0] ) ) ) {
                ulTemp = _wtoi( szValue );
                for( ptrStylesCur=ptrStylesHead ; 
                     ptrStylesCur && ( ulTemp != ptrStylesCur->ulData ); 
                     ptrStylesCur=(SSNT_INFO*)ptrStylesCur->ptrNext ) {
                }
                if ( ptrStylesCur ) {
                   ptrStyleXfsCur = (SSNT_INFO*)calloc( sizeof(SSNT_INFO), 1 ) ;
                   ptrStyleXfsCur->ptrNext = ptrStyleXfsHead ;
                   ptrStyleXfsCur->ulData = lStyleCount ;
                   ptrStyleXfsHead = ptrStyleXfsCur ; 
                }
             }
          }
          if ( ! wcscmp( szTag, L"/cellXfs" ) ) {
             bInXfs = FALSE ;
             if ( ! ptrStyleXfsHead ) 
                bReturn = FALSE ;
             break ;
          }
       }
    }

     (*InputFile).fseekt(0, std::ios::beg) ;


      /***********************************************************************/
      /*  Find individual cells which use the non-translatable styles.       */
      /*  Save a list of SharedString IDs for later processing.              */
      /*                                                                     */
      /*  <cols>                                                             */
      /*    <col min="1" max="1" width="14" style="628"/>                    */
      /*  </cols>                                                            */
      /*  <row r="1">                                                        */
      /*    <c r="A1" s="629" t="s">                                         */
      /*      <v>27</v>                                                      */
      /*    </c>                                                             */
      /*  </row>                                                             */
      /***********************************************************************/

      usState = SSNT_STATE_NONE ;
      bExportSSNTContent = TRUE ;
      bSkipSSRef = FALSE ;
      bPartialTag = FALSE ;
      szIn[0] = NULL ;
      szTag[0] = NULL ;
      szPrevXMLInputText[0] = NULL ;                
      ulFilePos = 0 ;
      i = 0 ;
      while ( bReturn ) {
         if ( i ) {
            wcsncpy( szTagText, szIn, i ) ; 
            szTagText[i] = NULL ;
            TempFile << szTagText ;
            wmemmove( szIn, &szIn[i], wcslen(&szIn[i])+1 ) ;
            i = 0 ;
         }
         rc = fnGetXMLTag( szIn, &i, &ulFilePos, szTag, szTagText,
                           &ulTagStartPos, &ulTagEndPos, &bPartialTag ) ;
         if ( rc == 1 ) {
            bReturn = FALSE ;
         }
         if ( rc == 2 ) {    /* End of file */
            if ( i ) {
               wcsncpy( szTagText, szIn, i ) ; 
               szTagText[i] = NULL ;
               TempFile << szTagText ;
            }
            break;
         }
         if ( bSkipSSRef ) {
            wmemmove( szIn, &szIn[i], wcslen(&szIn[i])+1 ) ;
            i = 0 ;
            if ( ! wcscmp( szTag, L"/v" ) ) 
               bSkipSSRef = FALSE ;
            continue ;

         }

         if ( ! wcscmp( szTag, L"worksheet" ) ) {
            usState = SSNT_STATE_WORKSHEET ;
            usCol = 0 ;
            memset( usColsNonTrans, 0, sizeof(usColsNonTrans) ) ;
         }

         if ( usState == SSNT_STATE_WORKSHEET ) {
            if ( ! wcscmp( szTag, L"/worksheet" ) ) {
               usState = SSNT_STATE_NONE ;
            }

            if ( ( ! wcscmp( szTag, L"cols" )  ) &&
                 ( ! fnIsEmptyTag( szTagText ) ) ) {  /* Not an empty tag */
               usState = SSNT_STATE_COLS ;
            }

            if ( ( ! wcscmp( szTag, L"row" )   ) &&
                 ( ! fnIsEmptyTag( szTagText ) ) ) {  /* Not an empty tag */
               usState = SSNT_STATE_ROW ;
               usCol = 0 ;
            }
         }

         if ( usState == SSNT_STATE_COLS ) {
            if ( ! wcscmp( szTag, L"/cols" ) ) {
               usState = SSNT_STATE_WORKSHEET ;
            }

            if ( ! wcscmp( szTag, L"col" ) ) {
               ++usCol ;
               usColsNonTrans[usCol] = 0 ;
               ulColMin = 0 ;
               ulColMax = 0 ;
               if ( ( fnGetAttributeValue( szTagText, L"min", szValue ) ) &&
                    ( iswdigit( szValue[0] ) ) ) {
                  ulColMin = _wtoi( szValue );
                  if ( ( fnGetAttributeValue( szTagText, L"max", szValue ) ) &&
                       ( iswdigit( szValue[0] ) ) ) {
                     ulColMax = _wtoi( szValue );
                     if ( ulColMax >= MAX_COLS ) 
                        ulColMax = MAX_COLS - 1 ;
                  }
                  if ( ( ulColMin > 0 ) &&
                       ( ulColMin < ulColMax ) ) {
                     for( k=ulColMax-ulColMin ; k>0 ; usColsNonTrans[usCol+k]=0, --k ) ;
                  }
               }
               if ( ( fnGetAttributeValue( szTagText, L"style", szValue ) ) &&
                    ( iswdigit( szValue[0] ) ) ) {
                  ulTemp = _wtoi( szValue );
                  for( ptrStyleXfsCur=ptrStyleXfsHead ; 
                       ptrStyleXfsCur && ( ulTemp != ptrStyleXfsCur->ulData ); 
                       ptrStyleXfsCur=(SSNT_INFO*)ptrStyleXfsCur->ptrNext ) {
                  }
                  if ( ptrStyleXfsCur ) {
                     usColsNonTrans[usCol] = 1 ;
                     if ( ( ulColMin > 0 ) &&
                          ( ulColMin < ulColMax ) ) {
                        for( k=ulColMax-ulColMin ; k>0 ; usColsNonTrans[usCol+k]=1, --k ) ;
                        usCol = ulColMax ; 
                     }
                  }
               }
               if ( ulColMax > usCol ) 
                  usCol = ulColMax ; 
            }
         }

         if ( usState == SSNT_STATE_ROW ) {
            if ( ! wcscmp( szTag, L"/row" ) ) {
               usState = SSNT_STATE_WORKSHEET ;
            }

            if ( ( ! wcscmp( szTag, L"c" )     ) &&
                 ( ! fnIsEmptyTag( szTagText ) ) ) {  /* Not an empty tag */
               usState = SSNT_STATE_CELL ;   
               bTransCell = TRUE ;
               ++usCol ;
//         Column setting is not used as default for its cells.
//         Each cell has its own style.  If no "s" attribute, then default s="0".
               if ( ( fnGetAttributeValue( szTagText, L"t", szValue ) ) &&
                    ( ! wcscmp( szValue,L"s" )                        ) ) {
                  if ( ( fnGetAttributeValue( szTagText, L"s", szValue ) ) &&
                       ( iswdigit( szValue[0] )                          ) ) 
                     ulTemp = _wtoi( szValue );
                  else 
                     ulTemp = 0 ;                          
                  for( ptrStyleXfsCur=ptrStyleXfsHead ; 
                       ptrStyleXfsCur && ( ulTemp != ptrStyleXfsCur->ulData ); 
                       ptrStyleXfsCur=(SSNT_INFO*)ptrStyleXfsCur->ptrNext ) {
                  }
                  if ( ptrStyleXfsCur ) {
                     bTransCell = FALSE ;         /* Non-trans cell      */
                  }
               } else {
                  if ( usColsNonTrans[usCol] ) {  /* Column is non-trans */
                     bTransCell = FALSE ;         /* Non-trans cell      */
                  } 
               }
               if ( ! bTransCell ) {                /* Look ahead for <v> value */
                  fnGetCompleteString( &szIn[i], L"/", szValue, sizeof(szValue)/sizeof(WCHAR) ) ;
                  ptrChar = wcsstr( szValue, L"<v>" ) ;
                  if ( ptrChar ) {
                     ptrChar += 3 ;
                     ptrChar2 = wcsstr( ptrChar, L"<" ) ;
                     if ( ptrChar2 ) {
                        *ptrChar2 = NULL ;
                        for( ptrChar2=ptrChar ; *ptrChar2 && iswdigit(*ptrChar2) ; ++ptrChar2 ) ;
                        *ptrChar2 = NULL ;
                        ulTemp = _wtoi( ptrChar );
                        ptrCellHead = ptrNTCellHead ;
                        for( ptrCellCur=ptrCellHead, ptrCellPrev=0  ; 
                             ptrCellCur && ( ulTemp > ptrCellCur->ulData ); 
                             ptrCellPrev=ptrCellCur, ptrCellCur=(SSNT_INFO*)ptrCellCur->ptrNext ) {
                        }
                        if ( ( ptrCellCur ) &&
                             ( ulTemp == ptrCellCur->ulData ) &&
                             ( ptrCellCur->szString ) ) {
                           for( ptrChar=&szIn[i-1] ; ptrChar>szIn && wcsncmp( ptrChar, L" t=", 3 ) ; --ptrChar ) ;
                           if ( ! wcsncmp( ptrChar, L" t=\"", 4 ) ) {
                              bSkipSSRef = TRUE ;
                              ptrChar += 4 ;
                              for( ptrChar2=ptrChar ; *ptrChar2 && *ptrChar2!=L'\"' ; ptrChar2++ );
                              wmemmove( ptrChar+9, ptrChar2, wcslen(ptrChar2)+1 ) ;
                              wcsncpy( ptrChar, L"inlineStr", 9 ) ;
                              i += 9 - (ptrChar2 - ptrChar) ;
                              wcsncpy( szTagText, szIn, i ) ; 
                              szTagText[i] = NULL ;
                              TempFile << szTagText ;
                              TempFile << L"<is><t>" ;
                              TempFile << ptrCellCur->szString ;
                              TempFile << L"</t></is>" ;
                              wmemmove( szIn, &szIn[i], wcslen(&szIn[i])+1 ) ;
                              i = 0 ;
                           }

                        }
                     }
                  }
               }
            }
         }

         if ( usState == SSNT_STATE_CELL ) {
            if ( ! wcscmp( szTag, L"/c" ) ) {
               usState = SSNT_STATE_ROW ;
            }
         }
      }


      bExportSSNTContent = FALSE ;

      (*InputFile).fseekt(0, std::ios::beg) ;


   /***********************************************************************/
   /*  Determine which strings are defined as both translatable and       */
   /*  non-translatable.                                                  */
   /***********************************************************************/
   ptrCellCur = ptrTRCellHead ;
   while ( ptrCellCur ) {
      for( ptrCellPrev = ptrNTCellHead ;
           ptrCellPrev && (ptrCellPrev->ulData < ptrCellCur->ulData) ; 
           ptrCellPrev = (SSNT_INFO*)ptrCellPrev->ptrNext ) ;
      if ( ( ptrCellPrev ) &&
           ( ptrCellPrev->ulData == ptrCellCur->ulData ) )
         ptrCellPrev->bTrans = TRUE ;
      ptrCellCur = (SSNT_INFO*)ptrCellCur->ptrNext ;
   }


   /***********************************************************************/
   /*  Free allocated storage.                                            */
   /***********************************************************************/

   fnFreeSSNTList( &ptrStylesHead ) ;
   fnFreeSSNTList( &ptrStyleXfsHead ) ;
   fnFreeSSNTList( &ptrTRCellHead ) ;



   (*InputFile).freefile() ;
   TempFile.close() ;
   CopyFileA(Work, In, FALSE);

   return( bReturn ) ;
}
