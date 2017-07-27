/****************************************************************************/
/*  Copyright (C) 1998-2017, International Business Machines                */
/*         Corporation and others. All rights reserved                      */
/*                   IBM Internal Use Only                                  */
/*                                                                          */
/* ValDocDocx.cpp                                                           */
/*                                                                          */
/*FUNCTIONS:                                                                */
/* Parse                                                                    */
/*                                                                          */
/*CHANGES:                                                                  */
/*   When     Why    Who  What                                              */
/* -------- -------  ---  -------------------------                         */
/*  4/11/17           DAW  Created from IBMXMWRD                            */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*  <w:p> Paragraph element.                                                */
/*     Contains:                                                            */
/*        fldSimple    Simple Word field                                    */
/*        hlink        Hyperlink                                            */
/*        subDoc       Sub-document                                         */
/*                                                                          */
/*        rPr               Run properties for the paragraph mark           */
/*        rPr               Previous run properties for paragraph mark      */
/*            adjustRightInd    Auto adjust right index                     */
/*            autoSpaceDE       Auto adjust spacing of latin text           */
/*            autoSpaceDN       Auto adjust spacing of Eastern Asian text   */
/*            bidi              Right to left paragraph layout              */
/*            cnfStyle          Paragraph conditional formatting            */
/*            contextualSpacing Ignore spacing above and below              */
/*            divId             Associated HTML div ID                      */
/*            framePr           Text frame properties                       */
/*            ind               Paragraph indentation                       */
/*            jc                Paragraph alignment                         */
/*            keepLines         Keep all lines on one page                  */
/*            keepNext          Keep paragraph with next paragraph          */
/*            kinsoku           Use East Asian typography rules             */
/*            mirrorIndents     Use left/righ indents as inside/outside     */
/*            numPr             Numbering definition instance reference     */
/*            outlineLvl        Associated outline level                    */
/*            overflowPunct     Allow punctuation to extend past txt extents*/
/*            pageBreakBefore   Start paragraph on next page                */
/*            pBdr              Paragraph borders                           */
/*                bar           Paragraph border between facing pages       */
/*                between       Paragraph border between identical paragraph*/
/*                bottom        Paragraph border between identical parasraph*/
/*                left          Left paragraph border                       */
/*                right         Right paragraph border                      */
/*                top           Paragraph border above identical paragraphs */
/*            pPrChange         Revision information for para properties    */
/*            pStyle            Referenced paragraph style                  */
/*            rPr               Run properties for the paragraph mark       */
/*            rPr               Previous run properties for paragrpah mark  */
/*            sectPr            Section properties                          */
/*            shd               Paragraph shading                           */
/*            snapToGrid        Use grid for ingter-line paragraph spacing  */
/*            spacing           Spacing between lines                       */
/*            suppressAutoHyphens  Suppress hyphenation for paragraph       */
/*            suppressLineNumbers  Suppress line numbers for paragraph      */
/*            suppressOverlap   Prevent text frames from overlapping        */
/*            tabs              Set of custom tab stops                     */
/*                tab           Custom tab stop                             */
/*            textAlignment     Vertical character alignment                */
/*            textboxTightWrap  Tight wrap paragraphs to text box content   */
/*            textDirection     Paragraph text direction flow               */
/*            topLinePunct      Compress punctuation at start of a line     */
/*            windowControl     Allow first/last line on separate page      */
/*            wordWrap          Allow line break at character level         */
/*        bookmarkEnd       Bookmark end                                    */
/*        bookmarkStart     Bookmark start                                  */
/*        commentRangeEnd   Comment anchor range end                        */
/*        commentRangeStart  Comment anchor range start                     */
/*        customXml         Inline-level custom XML                         */
/*        customXmlDelRangeEnd        Custom XML deletion end               */
/*        customXmlDelRangeStart      Custom XML deletion start             */
/*        customXmlInsRangeEnd        Custom XML insertion end              */
/*        customXmlInsRangeStart      Custom XML insertion start            */
/*        customXmlMoveFromRangeEnd   Custom XML move source end            */
/*        customXmlMoveFromRangeStart  Custom XML move source start         */
/*        customXmlMoveToRangeEnd     Custom XML move destination end       */
/*        customXmlMoveToRangeStart   Custom XML move destination start     */
/*        del               Delete run content                              */
/*        fldSimple         Simple field                                    */
/*        hyperlink         Hyperlink                                       */
/*        ins               Inserted run content                            */
/*        moveFrom          Move source run content                         */
/*        moveFromRangeEnd  Move source location container - end            */
/*        moveFromRangeStart  Move source location container - start        */
/*        moveTo            Move destination run content                    */
/*        moveToRangeEnd    Move destination location container - end       */
/*        moveToRangeStart  Move destination location container - start     */
/*        oMath             Office math                                     */
/*        oMathPara         Math paragraph                                  */
/*        p                 Paragraph                                       */
/*        permEnd           Range permission end                            */
/*        permStart         Range permission start                          */
/*        pPr               Paragraph properties                            */
/*        proofErr          Proofing error anchor                           */
/*        r                 Text run                                        */
/*        sdt               Inline-level structured document tag            */
/*        smartTag          Inline-level smart tag                          */
/*        subDoc            Anchor for subdocument                          */
/*                                                                          */
/*  <w:r> Run of text element.                                              */
/*     Contains:                                                            */
/*        rPr               Run property                                    */
/*        rPr               Previous run properties                         */
/*            b             Bold                                            */
/*            bCs           Complex script bold                             */
/*            bdr           Text border                                     */
/*            caps          Display as capital letters                      */
/*            color         Content color                                   */
/*            cs            Complex script formatting                       */
/*            dstrike       Double strikethrough                            */
/*            eastAsianLayout  East Asian typography settings               */
/*            effect        Animated text effect                            */
/*            em            Emphasis mark                                   */
/*            emboss        Embossing                                       */
/*            fitText       Manual run width                                */
/*            highlight     Text highlighting                               */
/*            i             Italics                                         */
/*            iCs           Complex script italics                          */
/*            imprint       Imprinting                                      */
/*            kern          Font kerning                                    */
/*            lang          Language for run content                        */
/*            noProof       No spellor grammar check                        */
/*            oMath         Open Office XML Math                            */
/*            outline       Display character outline                       */
/*            position      Vertically raised/lowered text                  */
/*            rFonts        Run fonts                                       */
/*            rPrChange     Revision information                            */
/*            rStyle        Referenced character style                      */
/*            rtl           Right to left text                              */
/*            shadow        Shadowing                                       */
/*            shd           Run shading                                     */
/*            smallCaps     Small caps                                      */
/*            snapToGrid    Use document grid setting                       */
/*            spacing       Character spacing adjustment                    */
/*            specVanish    Paragraph mark is always hidden                 */
/*            strike        Single strikethrough                            */
/*            sz            Font size                                       */
/*            szCs          Complex script font size                        */
/*            u             Underline                                       */
/*            vanish        Hidden text                                     */
/*            vertAlign     Subscript/Superscript text                      */
/*            w             Expanded/Compressed text                        */
/*            webHidden     Web hidden text                                 */
/*        annotationRef     Comment information block                       */
/*        br                Break                                           */
/*        commentReference  Comment content reference mark                  */
/*        continuationSeparator  Continuation separator mark                */
/*        cr                Carriage return                                 */
/*        dayLong           Date block - long day format                    */
/*        dayShort          Date block - short day format                   */
/*        delInstrText      Deleted field code                              */
/*        delText           Deleted text                                    */
/*        drawing           DrawingML object                                */
/*        endnoteRef        Endnote reference mark                          */
/*        endnoteReference  Endnote reference                               */
/*        fldChar           Complex field character                         */
/*            instrText     Field code                                      */
/*        footnoteRef       Footnote reference mark                         */
/*        footnoteReference  Footnote reference                             */
/*        instrText         Regular text when not in a complex field        */
/*        lastRenderedPageBreak   Last calculated page break                */
/*        monthLong         Date block - long month format                  */
/*        monthShort        Date block - short month format                 */
/*        noBreakHyphen     Non-breaking hyphen character                   */
/*        object            Inline embedded object                          */
/*           control        Inline embedded control                         */
/*        pgNum             Page number block                               */
/*        pict              VML object                                      */
/*           control        Floating embedded control                       */
/*           movie          Embedded movie                                  */
/*        ptab              Absolute position tab character                 */
/*        ruby              Phonetic guide                                  */
/*           rt             Phonetic guide text                             */
/*           rubyAlign      Phonetic guide text alignment                   */
/*           rubyBase       Phonetic guide base text                        */
/*           rubyPr         Phonetic guide properties                       */
/*               dirty      Invalidated field cache                         */
/*               hps        Phonetic guide text font size                   */
/*               hpsBaseText  Phonetic guide base text font size            */
/*               hpsRaise   Distance between guide and guide base text      */
/*               lid        Language ID for phonetic guide                  */
/*        separator         Footnote/endnote separator mark                 */
/*        softHyphen        Optional soft hyphen character                  */
/*        sym               Symbol character                                */
/*        t                 Text                                            */
/*        tab               Tab character                                   */
/*        yearLong          Date block - long year format                   */
/*        yearShort         Date block - short year format                  */
/*                                                                          */
/*                                                                          */
/*  <w:t> Text element.                                                     */
/*        Contains:                                                         */
/*           Translatable text.                                             */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Note:  rsid___ attributes indicate runs of text which were changed      */
/*         during a specific editing session.  For translation purposes,    */
/*         these attributes should be ignored and runs of text should be    */
/*         merged together if this is the only difference.                  */
/*         rsidDel, rsidP, rsidR, rsidRDefauilt, rsidRPr                    */
/*                                                                          */
/****************************************************************************/


#include "EQF.H"
#include "ValDocDocx.h"
#include "ValDocDocxUtil.h"
#include "ValDocDocxParse.h"


using namespace mku ;                       

wifstream  *InputFile ;
wifstream  *InputFile2 ;
     BOOL  bUTF16 ;


       WCHAR     STYLE_TAG[3] = L"~+" ;
       WCHAR     STYLE_TAG_LONG[4] = L"~+:" ;
       WCHAR     STYLE_ENDTAG[4] = L"/~+" ;
       WCHAR     NEUTRAL_TAG[3] = L"~+" ;
       WCHAR     NEUTRAL_TAG_LONG[4] = L"~+:" ;
       WCHAR     ENDL[4] = L"\r\n";
       WCHAR     TWB_FILENAME[8] = L"TWBFILE";
       WCHAR     szPrevXMLInputText[MAX_XML_RCD_LENGTH] ;
       BOOL      bReadSource ;
       BOOL      bFileExport ;
       BOOL      bInGfxdata ;
       BOOL      bInLongAttr ;
       BOOL      bInDebugBinData ;
       BOOL      bSetCommonProperty ;
       BOOL      bStartTableRow ;

       WCHAR     FILE_NAME_SEPARATOR_START[10] = L"<!-- TWB " ;
       WCHAR     FILE_NAME_SEPARATOR_END[6] = L" -->\n" ;


extern   short   sTPVersion ;               /* From USRCALLS.C  */
extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */



typedef struct {
    WCHAR  Name[25] ;                   /* Name of neutral tag               */
    WCHAR  PseudoTag[8] ;               /* Name of internal tag              */
    USHORT TagAction ;                  /* Action to tag for this tag        */
    USHORT TagType ;                    /* Type of tag                       */
    USHORT TagId ;                      /* ID number of tag name             */
} NEUTRALTAG;

#define NUM_NEUTRAL_TAGS     35
static NEUTRALTAG      NEUTRAL_TAGS[ NUM_NEUTRAL_TAGS ] =  {


// ###############################################
//
//  STILL NEED TO HANDLE:
//
//         aml:annotation        w:type="Word.Insertion"
//         aml:annotation        w:type="Word.Deletion"
//         aml:annotation        w:type="Word.Comment"
//         aml:content           insertion, deletion, formatting change, comment, or bookmark
//         wx:t
//         w:tblCaption          w:val="translatable text"/>
//         w:tblDescription      w:val="translatable text"/>
//         w:placeholder         w:val="translatable text"/>
//         w:alias               w:val="translatable text"/>
//         w:docPartCategory     w:val="translatable text"/>
//
//         fldSimple
//         subDoc
//
/*        commentReference     Comment content reference mark                  */
/*        delInstrText         Deleted field code                              */
/*        delText              Deleted text                                    */
/*        endnoteRef           Endnote reference mark        append to next text */
/*        endnoteReference     Endnote reference             append to end of previous text, like footnote */
/*        footnoteReference    Footnote reference            append to end of previous text */
/*        ruby                 Phonetic guide                                  */
//
// ###############################################



   L"/br"                     , L"BRE"     , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_END      , TAG_ID_BR_END,          /* END   5-14-12 */
   L"/hlink"                  , L"HLE"     , /*TAG_ACTION_KEEP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_END      , TAG_ID_HLINK_END,
   L"/hyperlink"              , L"HPLE"    , /*TAG_ACTION_KEEP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_END      , TAG_ID_HYPERLINK_END,
   L"annotationRef"           , L"AR"      , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_ANNOTATEREF,
   L"br"                      , L"BR"      , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_BR,
   L"br"                      , L"BRS"     , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_BEGIN    , TAG_ID_BR_BEGIN,        /* BEGIN 5-14-12 */
   L"br"                      , L"BRE"     , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_END      , TAG_ID_BR_END,          /* END   5-14-12 */
   L"continuationSeparator"   , L"CS"      , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_CONTINUATIONSEPARATOR,
   L"cr"                      , L"CR"      , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_CR,
   L"dayLong"                 , L"DAYL"    , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_DAYLONG,
   L"dayShort"                , L"DAYS"    , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_DAYSHORT,
   L"drawing"                 , L"DRAW"    , /*TAG_ACTION_KEEP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_BEGIN    , TAG_ID_DRAWING, 
   L"endnote"                 , L"EN"      , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_ENDNOTE,
   L"endnoteRef"              , L"ENR"     , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_ENDNOTEREF,
   L"fldChar"                 , L"FD"      , /*TAG_ACTION_KEEP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_BEGIN    , TAG_ID_FLDCHAR_BEGIN,
   L"fldChar"                 , L"FDE"     , /*TAG_ACTION_KEEP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_END      , TAG_ID_FLDCHAR_END,  
   L"footnote"                , L"FN"      , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_FOOTNOTE,
   L"footnoteRef"             , L"FNR"     , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_FOOTNOTEREF,
   L"hlink"                   , L"HL"      , /*TAG_ACTION_KEEP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_BEGIN    , TAG_ID_HLINK_BEGIN,
   L"hyperlink"               , L"HPL"     , /*TAG_ACTION_KEEP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_BEGIN    , TAG_ID_HYPERLINK_BEGIN,
   L"instrText"               , L"ITXT"    , /*TAG_ACTION_KEEP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_BEGIN    , TAG_ID_INSTRTEXT,
   L"lastRenderedPageBreak"   , L"LRPB"    , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_LASTRENDEREDPAGEBREAK,
   L"monthLong"               , L"MONL"    , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_MONTHLONG,
   L"monthShort"              , L"MONS"    , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_MONTHSHORT,
   L"noBreakHyphen"           , L"BH"      , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_NOBREAKHYPHEN,
   L"object"                  , L"OBJ"     , /*TAG_ACTION_KEEP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_BEGIN    , TAG_ID_OBJECT,
   L"pgNum"                   , L"PGNUM"   , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_PGNUM,
   L"pict"                    , L"PICT"    , /*TAG_ACTION_KEEP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_BEGIN    , TAG_ID_PICT, 
   L"ptab"                    , L"PTAB"    , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_PTAB,
   L"separator"               , L"SEP"     , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_SEPARATOR,
   L"softHyphen"              , L"SH"      , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_SOFTHYPHEN,
   L"sym"                     , L"SYM"     , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_SYM,
   L"tab"                     , L"TAB"     , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_TAB,
   L"yearLong"                , L"YEARL"   , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_YEARLONG,
   L"yearShort"               , L"YEAYS"   , /*TAG_ACTION_SKIP  ,*/ TAG_ACTION_KEEP   , TAG_TYPE_EMPTY    , TAG_ID_YEARSHORT,
} ;


 CHAR  *TITLE_XMWRD_PARSING_ERROR             ="Parsing Error";
 
 CHAR  *MSG_XMWRD_PARSING_ERROR               ="File could not be parsed.\n\nProcessing is terminated.";



// ###############################################
//
//  STILL NEED TO HANDLE translatable attributes
//
//         hlink          screenTip
//         v:imagedata    o:title  
// 
// 
// 
// 
// 
// 
// ###############################################



    FILE      *fDebug ;
    CHAR      *szDebugFile = "C:\\ibmxmwrd.body.debug" ;
    BOOL      bDebugBody = FALSE ;
    BOOL      bDebugFree = FALSE ;
FILE *fdaw;




/*****************************************************************************/
/*  Parse                                                                    */
/*                                                                           */
/*  Function called by EQFPRESEG2.                                           */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be analyzed.                             */
/*****************************************************************************/

BOOL Parse(PSZ in, PSZ out )
{
    wofstream OutFile ( out ) ;

    InputFile = new wifstream( in ) ;


    wifstream InFile ( in ) ;



    P_INFO     *ptrParaHead ;
    P_INFO     *ptrParaCur ;
    P_INFO     *ptrParaTemp ;

//  R_INFO     *ptrRunHead ;
    R_INFO     *ptrRunCur ;

//  T_INFO     *ptrTextHead ;
    T_INFO     *ptrTextCur ;

//  WCHAR      szErrTitle[128] ;
    WCHAR      szErrText[512] ;
//  WCHAR      szIn[MAX_XML_RCD_LENGTH2] ;
    WCHAR      szPreTag[2] ;
    WCHAR      szPostTag[2] ;
//  ULONG      ulFilePos = 0 ;
    ULONG      ulSeqNum ;
    ULONG      ulTextBlocks ;
    USHORT     i, j ;
    USHORT     rc ;
    BOOL       bConcatAllText = FALSE ;
    BOOL       bReturn = TRUE;

    i;


    if ( bDebugBody || bDebugFree ) {
       fDebug=fopen(szDebugFile,"ab");
    }


    bUTF16 = (*InputFile).IsUTF16();
    bReadSource = TRUE ;
    bInGfxdata = FALSE ;
    bInLongAttr = FALSE ;
    bSetCommonProperty = FALSE ;


    /*************************************************************************/
    /*  Create a linked list which contains all of the required              */
    /*  information necessary to segment this file.                          */
    /*************************************************************************/
    rc = fnCreateInputList( &ptrParaHead ) ;
    if ( rc > 0 ) {
       ShowIBMMessage( TITLE_XMWRD_PARSING_ERROR, MSG_XMWRD_PARSING_ERROR, FALSE, FALSE ) ;
       bReturn = FALSE ;
    }


    /*************************************************************************/
    /*  Create the output file for the translatable text.                    */
    /*************************************************************************/
    if ( bReturn ) {
       (*InputFile).fseekt(0, std::ios::beg) ;
       szPrevXMLInputText[0] = NULL ;
       OutFile.InsertBOM();

//     OutFile << L"\n" ;   
    
       /**********************************************************************/
       /*  Process each paragraph node.                                      */
       /**********************************************************************/
       ulTextBlocks = 0 ;
       for( ptrParaCur=ptrParaHead ; 
            ptrParaCur ; 
            ptrParaCur=(P_INFO*)ptrParaCur->ptrNext ) {

          ulTextBlocks += ptrParaCur->NumTextTags ;
    
          /*******************************************************************/
          /*  If paragraph has text associated with it, then write it out.   */
          /*******************************************************************/
          if ( ( ptrParaCur->NumTextTags > 0 ) &&   /* Paragraph contains text */
               ( ptrParaCur->NodeType != NODE_TYPE_FILE_NAME ) &&
               ( ptrParaCur->NodeType != NODE_TYPE_SHEET_REF ) &&
               ( ptrParaCur->NodeType != NODE_TYPE_TEXT_REF  ) ) {


             /*************************************************************/
             /*  Special case where paragraph consists of only text       */
             /*  which is concatenated because of same properties.        */
             /*  No need for leading/trailing inline tags.        4-24-12 */
             /*************************************************************/
             bConcatAllText = FALSE ;
             if ( ( ptrParaCur->ptrRunList ) &&
                  ( ptrParaCur->NumTextTags > 1     ) && 
                  ( ptrParaCur->NumNeutralTags == 0 ) ) {
                for( ptrRunCur=(R_INFO*)ptrParaCur->ptrRunList ; 
                     ptrRunCur ; 
                     ptrRunCur=(R_INFO*)ptrRunCur->ptrNext ) {
                   if ( ( ptrRunCur->ptrTextList ) ||
                        ( ptrRunCur->TagId != TAG_ID_TEXT ) ||
                        ( ( ptrRunCur != (R_INFO*)ptrParaCur->ptrRunList ) &&
                          ( ptrRunCur->Concat != CONCAT_TEXT_YES  ) ) )
                      break ;
                }
                if ( ! ptrRunCur ) 
                   bConcatAllText = TRUE ;
             }

             /*************************************************************/
             /*  Write out paragraph tag.                                 */
             /*************************************************************/
             if ( ! wcscmp( ptrParaCur->Tag, L"sheet" ) ) {
                if ( ( ptrParaCur->CommonProperty ) &&          /* 2-3-14 */
                     ( ! bConcatAllText ) )
                   swprintf( szErrText, L"<w:p n=\"%ld\" t=\"%s\" max=\"31\" cp=\"%d\">", 
                             ptrParaCur->SeqNum,ptrParaCur->Tag,ptrParaCur->CommonProperty ) ;
                else
                   swprintf( szErrText, L"<w:p n=\"%ld\" t=\"%s\" max=\"31\">", 
                             ptrParaCur->SeqNum,ptrParaCur->Tag ) ;
             } else {
                if ( ( ptrParaCur->CommonProperty ) &&          /* 2-3-14 */
                     ( ! bConcatAllText ) )
                   swprintf( szErrText, L"<w:p n=\"%ld\" t=\"%s\" cp=\"%d\">", 
                             ptrParaCur->SeqNum,ptrParaCur->Tag,ptrParaCur->CommonProperty ) ;
                else
                   swprintf( szErrText, L"<w:p n=\"%ld\" t=\"%s\" tr=\"%d\">", 
                             ptrParaCur->SeqNum,ptrParaCur->Tag,ptrParaCur->StartTableRow ) ;
             }
             OutFile << szErrText ;

             /****************************************************************/
             /*  If no run nodes, then this paragraph contains only 1 text   */
             /*  block.  Write out the translatable text for this            */
             /*  paragraph.                                                  */
             /****************************************************************/
             if ( ptrParaCur->ptrRunList == 0 ) {  
                fnWriteBlock( WRITE_SOURCE, 
                              ptrParaCur->BlockStartPos, 
                              ptrParaCur->BlockEndPos, 
                              &OutFile, FALSE ) ;
                szPrevXMLInputText[0] = NULL ;
             } else {

                /*************************************************************/
                /*  Process each run block in this paragraph.                */
                /*************************************************************/
                for( ptrRunCur=(R_INFO*)ptrParaCur->ptrRunList ; 
                     ptrRunCur ; 
                     ptrRunCur=(R_INFO*)ptrRunCur->ptrNext ) {

                   if ( ptrRunCur->TextIns ) {
                      swprintf( szErrText, L"<InS>" ) ;
                      OutFile << L"<InS>";
                   }
                   if ( ptrRunCur->TextDel ) {
                      swprintf( szErrText, L"<DeL>" ) ;
                      OutFile << L"<DeL>";
                   }

                   /**********************************************************/
                   /*  If no text nodes, then this run node consists of      */
                   /*  1 control, either text or neutral tags.               */
                   /**********************************************************/
                   if ( ptrRunCur->ptrTextList == 0 ) {

                      /*******************************************************/
                      /*  If this is a text node, write out the              */
                      /*  translatable text for this node.                   */
                      /*******************************************************/
                      if ( ptrRunCur->TagId == TAG_ID_TEXT ) {   

                         if ( ptrRunCur->BreakBeforeText ) {      /* 8-19-15 */
                            swprintf( szErrText, L"<Br>" ) ;
                            OutFile << L"<Br>";
                         }
                         if ( ptrParaCur->NumTextTags > 1 ) { /* If > 1 text */
                            if ( ( ptrRunCur->Concat != CONCAT_TEXT_YES  ) &&
                                 ( ! bConcatAllText ) &&                  /* 4-24-12 */
                                 ( ( ptrRunCur->CommonProperty == 0 ) ||   /* 2-3-14 */
                                   ( ptrRunCur->CommonProperty == COMMONPROPERTY_FORCE_OFF ) ) ) {
                               swprintf( szErrText, L"<%s%ld>", STYLE_TAG_LONG, ptrRunCur->SeqNum ) ;
                               OutFile << szErrText ;
                            }
                         }

                         fnWriteBlock( WRITE_SOURCE, 
                                       ptrRunCur->BlockStartPos, 
                                       ptrRunCur->BlockEndPos, 
                                       &OutFile, FALSE ) ;
                         szPrevXMLInputText[0] = NULL ;

                         if ( ptrParaCur->NumTextTags > 1 ) { /* If > 1 text */
                            if ( ( ( ptrRunCur->ptrNext == NULL ) ||
                                   ( ((R_INFO*)(ptrRunCur->ptrNext))->Concat != CONCAT_TEXT_YES ) ) &&
                                 ( ! bConcatAllText ) &&                 /* 4-24-12 */
                                 ( ( ptrRunCur->CommonProperty == 0 ) ||  /* 2-3-14 */
                                   ( ptrRunCur->CommonProperty == COMMONPROPERTY_FORCE_OFF ) ) ) {
                               swprintf( szErrText, L"</%s%ld>", STYLE_TAG_LONG, ptrRunCur->SeqNum ) ;
                               OutFile << szErrText ;
                            }
                         }
                      } else {

                         /****************************************************/
                         /*  This is a neutral tag node.  Write out the      */
                         /*  placeholder for this information.               */
                         /****************************************************/
                         for( j=0 ; 
                              j<NUM_NEUTRAL_TAGS && ptrRunCur->TagId!=NEUTRAL_TAGS[j].TagId ; 
                              ++j ) ;

                         if ( ( ptrRunCur->TagType == TAG_TYPE_END ) &&
                              ( ptrRunCur->BeginSeqNum > 0 ) ) 
                            ulSeqNum = ptrRunCur->BeginSeqNum ;
                         else
                            ulSeqNum = ptrRunCur->SeqNum ;

                         szPreTag[0] = NULL ;
                         szPostTag[0] = NULL ;
                         if ( ptrRunCur->TagType == TAG_TYPE_END ) 
                            wcscpy( szPreTag, L"/" ) ;
                         else
                            if ( ptrRunCur->TagType == TAG_TYPE_EMPTY ) 
                               wcscpy( szPostTag, L"/" ) ;

                         if ( j < NUM_NEUTRAL_TAGS ) {
                            swprintf( szErrText, L"<%s%s%ld_%s%s>", szPreTag, NEUTRAL_TAG_LONG, 
                                      ulSeqNum, NEUTRAL_TAGS[j].PseudoTag, szPostTag ) ;
                         } else {
                            swprintf( szErrText, L"<%s%s%ld_X%s>", szPreTag, NEUTRAL_TAG_LONG, 
                                      ulSeqNum, szPostTag ) ;
                         }

                         OutFile << szErrText ;
                      }
                   } else {

                      /*******************************************************/
                      /*  If multiple text nodes, then must surround this    */
                      /*  run block with neutral tag information.            */
                      /*******************************************************/
                      if ( ( ptrRunCur->NumTextTags > 1 ) && 
                           ( ((R_INFO*)(ptrParaCur->ptrRunList))->ptrNext != NULL ) ) {
                         swprintf( szErrText, L"<%s%ld_X>", STYLE_TAG_LONG, ptrRunCur->SeqNum ) ;
                         OutFile << szErrText ;
                      }


                      /*******************************************************/
                      /*  Process each text node in this run block.          */
                      /*******************************************************/
                      for( ptrTextCur=(T_INFO*)ptrRunCur->ptrTextList ; 
                           ptrTextCur ; 
                           ptrTextCur=(T_INFO*)ptrTextCur->ptrNext ) {

                         /****************************************************/
                         /*  If this is a text node, write out the           */
                         /*  translatable text for this node.                */
                         /****************************************************/
                         if ( ptrTextCur->TagId == TAG_ID_TEXT ) {

                            if ( ptrParaCur->NumTextTags > 1 ) { /* If > 1 text  */
                               swprintf( szErrText, L"<%s%ld>", STYLE_TAG_LONG, ptrTextCur->SeqNum ) ;
                               OutFile << szErrText ;
                            }


                            fnWriteBlock( WRITE_SOURCE, 
                                          ptrTextCur->BlockStartPos, 
                                          ptrTextCur->BlockEndPos, 
                                          &OutFile, FALSE ) ;

                            if ( ptrParaCur->NumTextTags > 1 ) { /* If > 1 text  */
                               swprintf( szErrText, L"</%s%ld>", STYLE_TAG_LONG, ptrTextCur->SeqNum ) ;
                               OutFile << szErrText ;
                            }
                         } else {

                            /*************************************************/
                            /*  This is a neutral tag node.  Write out the   */
                            /*  placeholder for this information.            */
                            /*************************************************/
                            for( j=0 ; 
                                 j<NUM_NEUTRAL_TAGS && ptrTextCur->TagId!=NEUTRAL_TAGS[j].TagId ; 
                                 ++j ) ;

                            if ( ( ptrTextCur->TagType == TAG_TYPE_END ) &&
                                 ( ptrTextCur->BeginSeqNum > 0 ) ) 
                               ulSeqNum = ptrTextCur->BeginSeqNum ;
                            else
                               ulSeqNum = ptrTextCur->SeqNum ;

                            szPreTag[0] = NULL ;
                            szPostTag[0] = NULL ;
                            if ( ptrTextCur->TagType == TAG_TYPE_END ) 
                               wcscpy( szPreTag, L"/" ) ;
                            else
                               if ( ptrTextCur->TagType == TAG_TYPE_EMPTY ) 
                                  wcscpy( szPostTag, L"/" ) ;

                            if ( j < NUM_NEUTRAL_TAGS ) {
                               swprintf( szErrText, L"<%s%s%ld_%s%s>", szPreTag, NEUTRAL_TAG_LONG, 
                                         ulSeqNum, NEUTRAL_TAGS[j].PseudoTag, szPostTag ) ;
                            } else {
                               swprintf( szErrText, L"<%s%s%ld_X%s>", szPreTag, NEUTRAL_TAG_LONG, 
                                         ulSeqNum, szPostTag ) ;
                            }

                            OutFile << szErrText ;
                         }

                      }

                      /*******************************************************/
                      /*  If multiple text nodes, then must surround this    */
                      /*  run block with neutral tag information.            */
                      /*******************************************************/
                      if ( ( ptrRunCur->NumTextTags > 1 ) &&
                           ( ((R_INFO*)(ptrParaCur->ptrRunList))->ptrNext != NULL ) ) {
                         swprintf( szErrText, L"</%s%ld_X>", STYLE_TAG_LONG, ptrRunCur->SeqNum ) ;
                         OutFile << szErrText ;
                      }

                   }
                   if ( ptrRunCur->TextIns ) {
                      swprintf( szErrText, L"</InS>" ) ;
                      OutFile << L"</InS>";
                   }
                   if ( ptrRunCur->TextDel ) {
                      swprintf( szErrText, L"</DeL>" ) ;
                      OutFile << L"</DeL>";
                   }
                }
             }
    
    
             /****************************************************************/
             /*  End this paragraph block.                                   */
             /****************************************************************/
             OutFile << L"</w:p>" << ENDL  ;
          } else {

             if ( ( ptrParaCur->NodeType == NODE_TYPE_FILE_NAME ) &&
                  ( ptrParaCur->ptrNext ) ) {
                for( ptrParaTemp=(P_INFO*)ptrParaCur->ptrNext ;
                     ptrParaTemp && ptrParaTemp->NodeType!=NODE_TYPE_FILE_NAME ; 
                     ptrParaTemp=(P_INFO*)ptrParaTemp->ptrNext ) {
                   if ( ( ptrParaTemp->NodeType != NODE_TYPE_SHEET_REF ) &&
                        ( ptrParaTemp->NodeType != NODE_TYPE_TEXT_REF  ) ) 
                      break ;
                }
                if ( ( ptrParaTemp ) &&
                     ( ptrParaTemp->NodeType != NODE_TYPE_FILE_NAME ) ) {
                   OutFile << L"<!--  " << ptrParaCur->Tag << L"  -->" << ENDL  ;
                }
             }
          }
       }

       /**********************************************************************/
       /*  If paragraph has NO text associated with it, write out dummy      */
       /*  paragraph so that the segmented file is not empty.                */
       /**********************************************************************/
       if ( ulTextBlocks == 0 ) { 
          swprintf( szErrText, L"<w:p n=\"0\"></w:p>" ) ;
          OutFile << szErrText ;
       }
    }


    /*************************************************************************/
    /*  Free linked list space.                                              */
    /*************************************************************************/
    fnFreeParaList( &ptrParaHead ) ;

    (*InputFile).freefile() ;

    if ( bDebugBody || bDebugFree ) {
       fclose( fDebug ) ;
    }

    return(bReturn);

} /* Parse */




/****************************************************************************/
/*                                                                          */
/* fnCreateInputList                                                        */
/*                                                                          */
/* Create a linked list containing the necessary information from the       */
/* input file.                                                              */
/*                                                                          */
/* Input:      xx                                                           */
/* Output:     ptrList      - Pointer to start of list.                     */
/* Return:     0            - List created successfully.                    */
/*             1            - Failed to create the list.                    */
/*                                                                          */
/****************************************************************************/

USHORT fnCreateInputList( P_INFO** ptrList )
{

    P_INFO    *ptrParaHead = 0 ;
    P_INFO    *ptrParaTail = 0 ;
    P_INFO    *ptrParaCur = 0 ;
  
    R_INFO    *ptrRunHead = 0 ;
    R_INFO    *ptrRunTail = 0 ;
    R_INFO    *ptrRunCur = 0 ;
    R_INFO    *ptrRunTemp = 0 ;
    R_INFO    *ptrRunTemp2 = 0 ;
    R_INFO    *ptrRunLastKeep = 0 ;
  
    T_INFO    *ptrTextHead = 0 ;
    T_INFO    *ptrTextTail = 0 ;
    T_INFO    *ptrTextCur = 0 ;
    T_INFO    *ptrTextTemp = 0 ;
    T_INFO    *ptrTextTemp2 = 0 ;
    T_INFO    *ptrTextLastKeep = 0 ;

    WCHAR     szIn[MAX_XML_RCD_LENGTH2*2] ;
    WCHAR     szTag[XML_TAG_LEN] ;
    WCHAR     szBaseTag[XML_TAG_LEN] ;
    WCHAR     szPrevTag[XML_TAG_LEN] ;
    WCHAR     szPrevBaseTag[XML_TAG_LEN] ;
    WCHAR     szTagText[MAX_XML_RCD_LENGTH2*2] ;
    WCHAR     szValue[MAX_XML_RCD_LENGTH] ;
//  WCHAR     szValue2[MAX_XML_RCD_LENGTH] ;
//  WCHAR     swErrMsg[1000];
//  WCHAR     swErrMsg2[1000];
    WCHAR     *ptrChar ; 
  
    ULONG     ulFilePos = 0 ;
    ULONG     ulTagStartPos = 0 ;
    ULONG     ulTagEndPos = 0 ;
    ULONG     ulParaSeqNum = 0 ;
    ULONG     ulRunSeqNum = 0 ;
    ULONG     ulTextSeqNum = 0 ;
    ULONG     ulPrevRunStartPos = 0 ;
    ULONG     ulTagAction ;
    ULONG     ulTagId ;
    ULONG     ulTagType ;
    ULONG     *ptrCheckCommonPropText ;
    ULONG     i, j ;
    USHORT    rc ;
    USHORT    usReturn = 0 ;
    USHORT    usFieldState = FIELD_STATE_NONE ;
    USHORT    usFieldStateNest[10] ;
    USHORT    usFieldStateNestLvl = 0 ;
    USHORT    usParaState = PARA_STATE_NONE ;
    USHORT    usRunState = RUN_STATE_NONE ;
    USHORT    usTextType = 0 ;   
    USHORT    usTextCount ;
//  USHORT    usParaNestLevel = 0 ;
    USHORT    usNestInfo[100] = {0} ; 
    USHORT    usNestIndex = 0 ;
    USHORT    usNestwpPr = 0 ;
    USHORT    usNestResetInfo = NEST_NONE ;
    USHORT    usTextWithNoRun = 0;
    USHORT    usRunMaxMatch ;
    USHORT    usRunMaxID ;
    USHORT    usRunCount ;
//  USHORT    usCommonPropertyInsert = 0 ;
    USHORT    usRunCommonPropID ;
//  USHORT    usTemp ;
  
    BOOL      bForever = TRUE ;
    BOOL      bNewNode = FALSE ;
    BOOL      bExport ;   
    BOOL      bAllocate ;
//  BOOL      bTextFound ;
//  BOOL      bRepeatTag = FALSE;
    BOOL      bInsertedTag = FALSE;
//  BOOL      bInTitlesOfParts = FALSE ;
//  BOOL      bInTx = FALSE ;
//  BOOL      bInCell = FALSE ;
    BOOL      bPartialTag = FALSE ;
    BOOL      bCheckCommonProp ;
    BOOL      bCheckCommonPropBold ;
    BOOL      bFoundText = FALSE ;
//  BOOL      bTemp ;


    szPrevXMLInputText[0] = NULL ;
  
    szIn[0] = NULL ;
    szTag[0] = NULL ;
    szBaseTag[0] = NULL ;
    i = 0 ;

    bExport = FALSE ;



    szPrevXMLInputText[0] = NULL ;


//if (! bReadSource) 
// bDebugBody = TRUE ;

    /***********************************************************************/
    /*  Read through the file to determine where the text is defined.      */
    /***********************************************************************/
    while ( bForever ) {

       /***********************************************************************/
       /*  Get next tag from this line.                                       */
       /***********************************************************************/
       if ( usNestResetInfo == NEST_NONE ) {
          if ( wcscmp( szTag, L"phoneticPr" ) ) {                 /* 12-10-14 */
             wcscpy( szPrevTag, szTag ) ;
             wcscpy( szPrevBaseTag, szBaseTag ) ;
          }

          if ( usTextWithNoRun ) {
             if ( usTextWithNoRun == 1 ) {
                wcscpy( szTagText, L"<t>" ) ;
                wcscpy( szTag, L"t" ) ;
                wcscpy( szBaseTag, L"t" ) ;
             } else {
                wcscpy( szTagText, L"</si>" ) ;
                wcscpy( szTag, L"/si" ) ;
                wcscpy( szBaseTag, L"/si" ) ;
             }
             usTextWithNoRun = 0 ;
          } else {
             {
                if ( bDebugBody ) {
                   if ( ! wcscmp( szPrevBaseTag, L"binData" ) ) 
                      bInDebugBinData = TRUE ; 
                   else
                      bInDebugBinData = FALSE ;
                }
                rc = fnGetXMLTag( szIn, &i, &ulFilePos, szTag, szTagText,
                                  &ulTagStartPos, &ulTagEndPos, &bPartialTag ) ;
             }

             if ( rc == 1 ) {
                ShowIBMMessage( TITLE_XMWRD_PARSING_ERROR, MSG_XMWRD_PARSING_ERROR, FALSE, FALSE ) ;
                usReturn = 1 ;
                break ;
             }
             if ( rc == 2 ) {    /* End of file */
                if ( ( ptrParaHead ) &&             /* Free unused node       */
                     ( ptrParaTail->NumTextTags == 0 ) ) {
                   if ( ptrParaTail->Tag ) 
                      free( ptrParaTail->Tag ) ;
                   ptrParaCur = (P_INFO*)ptrParaTail->ptrPrev ;
                   free( ptrParaTail ) ;
                   ptrParaTail = ptrParaCur ;
                   if ( ptrParaTail ) 
                      ptrParaTail->ptrNext = NULL ;
                   else
                      ptrParaHead = NULL ;
                }
                break ;
             }

             if ( ( ! wcscmp( szTag, L"t" )  ) &&
                  ( ! wcscmp( szPrevTag, L"si" ) ) ) {
                if ( fnIsEmptyTag( szTagText ) ) {  /* If empty tag             */
                   usTextWithNoRun = 0 ;
                   wcscpy( szTagText, L"<r/>" ) ;
                   wcscpy( szTag, L"r" ) ;
                   wcscpy( szBaseTag, L"r" ) ;
                } else {
                   usTextWithNoRun = 1 ;
                   wcscpy( szTagText, L"<r>" ) ;
                   wcscpy( szTag, L"r" ) ;
                   wcscpy( szBaseTag, L"r" ) ;
                }
             } else
             if ( ( ! wcscmp( szTag, L"/si" ) ) &&
                  ( ! wcscmp( szPrevTag, L"/t"  ) ) ) {
                usTextWithNoRun = 2 ;
                wcscpy( szTagText, L"</r>" ) ;
                wcscpy( szTag, L"/r" ) ;
                wcscpy( szBaseTag, L"/r" ) ;
             } 
             if ( ( ! wcscmp( szBaseTag, L"tr" ) ) &&
                  ( ptrParaTail ) ) {
                bStartTableRow = TRUE ;
             }
          }

          ptrChar = wcschr( szTag, L':' ) ;
          if ( ptrChar ) {
             if ( szTag[0] == L'/' ) {
                szBaseTag[0] = L'/' ;
                wcscpy( &szBaseTag[1], ptrChar+1 ) ;
             } else {
                wcscpy( szBaseTag, ptrChar+1 ) ;
             }
          } else {
             wcscpy( szBaseTag, szTag ) ;
          }

          if ( ! wcscmp( szBaseTag, L"/ins" ) ) {
             ptrRunTail->TextIns = TRUE ;
             if ( bDebugBody ) {
                fwprintf(fDebug,L">>  INSTEXT\n");
             }
          }


          if ( bDebugBody ) {
             fwprintf(fDebug,L"TAG:   %lx-%lx  %d  %s  %s\n",ulTagStartPos,ulTagEndPos,usNestIndex,szBaseTag,szTagText);
             fwprintf(fDebug,L"       Nest: %d  %d\n",usNestResetInfo,usNestIndex);
             fflush(fDebug);
          }

       }


       /*====================================================================*/
       /*                                                                    */
       /*  Handle nested paragraph and run tags.                             */
       /*                                                                    */
       /*====================================================================*/
       bInsertedTag = FALSE ;
       if ( usNestResetInfo != NEST_NONE  ) {
          if ( usNestResetInfo == NEST_RESET_END_DONE ) {
             wcscpy( szTag, L"w:p" ) ;       /* Restore original <w:p> tag    */
             wcscpy( szBaseTag, L"p" ) ; 
             usNestResetInfo = NEST_NONE ;
//fDebug=fopen("c:\\dd.dd","a");
//fprintf(fDebug,">> P    i=%d   v=%d   r=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo);
//fclose(fDebug);
          } else 
          if ( usNestResetInfo == NEST_RESET_END_PARA ) {
             bInsertedTag = TRUE ;
             wcscpy( szTag, L"/w:p" ) ;
             wcscpy( szBaseTag, L"/p" ) ;
             usNestResetInfo = NEST_RESET_END_DONE ;
//fDebug=fopen("c:\\dd.dd","a");
//fprintf(fDebug,">>-P    i=%d   v=%d   r=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo);
//fclose(fDebug);
          } else
          if ( usNestResetInfo == NEST_RESET_END_RUN ) {
             bInsertedTag = TRUE ;
             wcscpy( szTag, L"/w:r" ) ;
             wcscpy( szBaseTag, L"/r" ) ;
             usNestResetInfo = NEST_RESET_END_PARA ;
//fDebug=fopen("c:\\dd.dd","a");
//fprintf(fDebug,">>-R    i=%d   v=%d   r=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo);
//fclose(fDebug);
          } else
          if ( usNestResetInfo == NEST_RESET_PARA ) {
             bInsertedTag = TRUE ;
             wcscpy( szTag, L"w:p" ) ;
             wcscpy( szBaseTag, L"p" ) ;
             if ( usNestInfo[usNestIndex] == NEST_RUN ) {
                usNestResetInfo = NEST_RESET_RUN ;
             } else {
                usNestResetInfo = NEST_NONE ;
             }
//fDebug=fopen("c:\\dd.dd","a");
//fprintf(fDebug,">>+P    i=%d   v=%d   r=%d  PS=%d  RS=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo,usParaState,usRunState);
//fclose(fDebug);
          } else
          if ( usNestResetInfo == NEST_RESET_RUN ) {
             bInsertedTag = TRUE ;
             wcscpy( szTag, L"w:r" ) ;
             wcscpy( szBaseTag, L"r" ) ;
             usNestResetInfo = NEST_NONE ;
//fDebug=fopen("c:\\dd.dd","a");
//fprintf(fDebug,">>+R    i=%d   v=%d   r=%d  PS=%d  RS=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo,usParaState,usRunState);
//fclose(fDebug);
          }



       } else {
//        if ( ! wcscmp( szTag, L"w:p" ) ) {
          if ( ( ! wcscmp( szBaseTag, L"p"  ) ) ||
               ( ! wcscmp( szBaseTag, L"si" ) ) ) {
             usNestwpPr = 0 ;
             if ( ! fnIsEmptyTag( szTagText ) ) {  /* If not an end tag        */
                ++usNestIndex ;                    /* 1 nest level deeper      */
                usNestInfo[usNestIndex] = NEST_PARA ; /* Nest level is para    */
                if ( usNestIndex > 1 ) {              /* If true nesting       */
                   if ( usNestInfo[usNestIndex-1] == NEST_PARA ) 
                      usNestResetInfo = NEST_RESET_END_PARA ;
                   else 
                      usNestResetInfo = NEST_RESET_END_RUN ;
//fDebug=fopen(szDebugFile,"a");
//fprintf(fDebug,"+ xP    i=%d   v=%d   r=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo);
//fclose(fDebug);
                   continue;
                }
             }
//fDebug=fopen("c:\\dd.dd","a");
//fprintf(fDebug,"+  P    i=%d   v=%d   r=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo);
//fclose(fDebug);
          } else 
//        if ( ! wcscmp( szTag, L"/w:p" ) ) {
          if ( ( ! wcscmp( szBaseTag, L"/p"  ) ) ||
               ( ! wcscmp( szBaseTag, L"/si" ) ) ) {
             --usNestIndex ;                   /* End this para nest level     */
             if ( usNestIndex > 0 ) {          /* Still nest levels            */
                usNestResetInfo = NEST_RESET_PARA ; /* Indicate reset nested para */
//fDebug=fopen("c:\\dd.dd","a");
//fprintf(fDebug,"- xP    i=%d   v=%d   r=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo);
//fclose(fDebug);
             } else {
//fDebug=fopen("c:\\dd.dd","a");
//fprintf(fDebug,"-  P    i=%d   v=%d   r=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo);
//fclose(fDebug);
             }
          } else 
//        if ( ! wcscmp( szTag, L"w:r" ) ) {
          if ( ! wcscmp( szBaseTag, L"r" ) ) {
             if ( ! fnIsEmptyTag( szTagText ) ) {  /* If not an end tag        */
                ++usNestIndex ;                    /* 1 nest level deeper      */
                usNestInfo[usNestIndex] = NEST_RUN ; /* Nest level is run      */
             }
//fDebug=fopen("c:\\dd.dd","a");
//fprintf(fDebug,"  +R    i=%d   v=%d   r=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo);
//fclose(fDebug);
          } else 
//        if ( ! wcscmp( szTag, L"/w:r" ) ) {
          if ( ! wcscmp( szBaseTag, L"/r" ) ) {
             --usNestIndex ;                   /* End this run nest level      */
//fDebug=fopen("c:\\dd.dd","a");
//fprintf(fDebug,"  -R    i=%d   v=%d   r=%d\n",usNestIndex,usNestInfo[usNestIndex],usNestResetInfo);
//fclose(fDebug);
          }
       }







  
       /* ----------   DEBUG   --------------------------------------- */
///    if ( ! wcscmp( szTag, L"w:body" ) /*&& !bReadSource*/ ) 
///       bDebugBody = TRUE ;
///    if ( bDebugBody ) {
///       fDebug=fopen(szDebugFile,"ab");    
///       fwprintf(fDebug,L"Tag...........%s    %lx-%lx  P=%d  R=%d\n",szTag,ulTagStartPos,ulTagEndPos,usParaState,usRunState);
///       fclose(fDebug);
///    }

       /*====================================================================*/
       /*                                                                    */
       /*  Handle paragraph-level tags.                                      */
       /*                                                                    */
       /*====================================================================*/

       /**********************************************************************/
       /*  Handle start of paragraph <w:p>.                                  */
       /**********************************************************************/
       if ( usParaState == PARA_STATE_NONE ) {

//        if ( ! wcscmp( szTag, L"w:p" ) ) {
          if ( ( ! wcscmp( szBaseTag, L"p"       ) ) ||
               ( ! wcscmp( szBaseTag, L"si"      ) ) ||  
               ( ! wcscmp( szBaseTag, TWB_FILENAME ) ) ) {

             /* ----  Initialize info for a new paragraph.  ---------------- */
             ++ulParaSeqNum ;                      /* Increment sequence #   */
             ulRunSeqNum = 0 ;                     /* Reset run sequence #   */
             ulTextSeqNum = 0 ;                    /* Reset text sequence #  */
             if ( ( fnIsEmptyTag( szTagText ) ) && /* If an end tag           */
                  ( ! bInsertedTag ) ) 
                usParaState = PARA_STATE_NONE ;    /* In empty paragraph     */
             else
                usParaState = PARA_STATE_PARA ;    /* In paragraph block     */
             usRunState = RUN_STATE_NONE ;         /* Not in run block       */
             ulPrevRunStartPos = 0 ;               /* No previous run        */
             usFieldState = FIELD_STATE_NONE ;     /* Reset FLDCHAR state    */
             ptrRunHead = 0 ;
             ptrRunTail = 0 ;
             ptrRunLastKeep = 0 ;
             ptrTextHead = 0 ;
             ptrTextTail = 0 ;
             ptrTextLastKeep = 0 ;
         
             /* ----  Allocate a paragraph node.  -------------------------- */
             if ( ( ! ptrParaHead ) ||             /* Allocate new node ?    */
                  ( ptrParaTail->NumTextTags > 0  ) ||
                  ( ptrParaTail->NodeType == NODE_TYPE_FILE_NAME ) ) {
                bAllocate = TRUE ;
             } else {                              /* Reuse existing node    */
                bAllocate = FALSE ;                             
             }
             fnAllocateParaNode( &ptrParaHead, &ptrParaTail, szTag, bAllocate ) ;

             /* ----  Initialize the paragraph node.  ---------------------- */
             ptrParaTail->NodeType  = NODE_TYPE_NONE ;  
             ptrParaTail->SeqNum = ulParaSeqNum ;
             if ( bInsertedTag ) 
                ptrParaTail->StartPos = ulTagEndPos + 1*sizeof(WCHAR) ; 
             else 
                ptrParaTail->StartPos = ulTagStartPos ; /* Start paragraph block*/

             if ( ! wcscmp( szBaseTag, TWB_FILENAME ) ) {
                ptrParaTail->NodeType  = NODE_TYPE_FILE_NAME ;  
                usParaState = PARA_STATE_NONE ;
                ptrParaTail->SeqNum = 0 ;
                --ulParaSeqNum ;
                free( ptrParaTail->Tag ) ;
                ptrParaTail->Tag = (WCHAR*)malloc( (wcslen(szTagText)+1) * sizeof(WCHAR) ) ;
                wcscpy( ptrParaTail->Tag, szTagText ) ;
                ptrParaTail->EndPos = ulTagEndPos ;
             } 
             ptrParaTail->StartTableRow = bStartTableRow ;
             bStartTableRow = FALSE ;


             /* ----------   DEBUG   --------------------------------------- */
             if ( bDebugBody ) {
                fwprintf(fDebug,L"PARA...#%ld   N=%ld   %lx    %lx-%lx   %lx-%lx\n",
                   ptrParaTail->SeqNum,ptrParaTail->NodeType,ptrParaTail,
                   ptrParaTail->StartPos,ptrParaTail->BlockStartPos,ptrParaTail->BlockEndPos,ptrParaTail->EndPos);
                fflush(fDebug);
             }

          } 

          continue ;
       } 

       /***********************************************************************/
       /*  Ignore paragraph property tags and all tags within this block.     */
       /***********************************************************************/
       if ( ! wcscmp( szBaseTag, L"pPr" ) ) {
          if ( ! fnIsEmptyTag( szTagText ) ) { /* If not an empty tag       */
             ++usNestwpPr ;
             usParaState = PARA_STATE_PROPERTY ;
          }
          continue ;
       }
       if ( usParaState == PARA_STATE_PROPERTY ) {
          if ( ! wcscmp( szBaseTag, L"/pPr" ) ) {
             if ( usNestwpPr > 0 ) 
                --usNestwpPr ;
             if ( usNestwpPr == 0 ) 
                usParaState = PARA_STATE_PARA ;
          }

          continue ;
       }
       if ( ( bReadSource ) &&
            ( ! wcscmp( szBaseTag, L"/pPr" ) ) &&
            ( ptrParaHead ) &&
            ( ptrRunHead ) &&
            ( ptrParaTail->NumTextTags == 0 ) ) {
          ptrParaTail->ptrRunList = 0 ;
          fnFreeRunList( &ptrRunHead ) ;
          ptrRunHead = 0 ;
          ptrRunTail = 0 ;
      //  ulRunSeqNum = 0 ;
          ptrRunLastKeep = 0 ;
          ptrParaTail->NumNeutralTags = 0 ;
          continue ;
       }


       /**********************************************************************/
       /*  Handle end of paragraph </w:p>.                                   */
       /**********************************************************************/
       if ( ( ! wcscmp( szBaseTag, L"/p"  ) ) ||
            ( ! wcscmp( szBaseTag, L"/si" ) ) ) {
          usParaState = PARA_STATE_NONE ;          /* Reset paragraph state  */
          usRunState = RUN_STATE_NONE ;            /* Reset run state        */

          if ( ptrParaHead ) {

             /* ----  Text exists in this paragraph node  ------------------ */
             if ( ptrParaTail->NumTextTags > 0 ) { 
                if ( bInsertedTag ) 
                   ptrParaTail->EndPos = ulTagStartPos - 1*sizeof(WCHAR) ;
                else
                   ptrParaTail->EndPos = ulTagEndPos ;


                /*************************************************************/
                /*  Remove any trailing non-text run tags after the last     */
                /*  text unit.  Condenses trailing neutral tags into 1.      */
                /*************************************************************/
                if ( bReadSource ) {
                   if ( ( ptrRunTail ) &&                          /* 7-9-14 */
                        ( ptrRunTail->ptrPrev ) &&
                        ( wcsstr( ptrRunTail->Tag, L"endParaRPr" ) ) ) { 
                      ptrRunLastKeep = (R_INFO*)ptrRunTail->ptrPrev ;
                   }
                   if ( ( ptrRunLastKeep ) &&
                        ( ptrRunLastKeep != ptrRunTail ) ) {
                      ptrRunTemp = (R_INFO*)ptrRunLastKeep->ptrNext ;
                      fnFreeRunList( &ptrRunTemp ) ;
                      ptrRunTail = ptrRunLastKeep ;
                      ptrRunLastKeep = 0 ;
                      ptrRunTail->ptrNext = 0 ;
                   }

                   if ( ( bInsertedTag ) &&
                        ( ptrRunTail   ) &&
                        ( ptrRunTail != ptrRunHead ) &&
                        ( ptrRunTail->NumTextTags == 0 ) && 
                        ( ptrRunTail->TagId == TAG_ID_UNKNOWN ) ) {
                      for( ptrTextTemp=(T_INFO*)ptrRunTail->ptrTextList ; 
                           ptrTextTemp ; 
                           ptrTextTemp=(T_INFO*)ptrTextTemp->ptrNext ) {
                         if ( ( ptrTextTemp->TagId != TAG_ID_UNKNOWN   ) ||
                              ( ptrTextTemp->TagType != TAG_TYPE_BEGIN ) ) 
                            break ;
                      }
                      if ( ! ptrTextTemp ) {
                         ptrRunTemp = (R_INFO*)ptrRunTail->ptrPrev ;
                         fnFreeRunList( &ptrRunTail ) ;
                         ptrRunTail = ptrRunTemp ;
                         ptrRunTail->ptrNext = 0 ;
                      }
                   }

                   ptrParaTail->BlockStartPos = ptrRunHead->StartPos ;
                   ptrParaTail->BlockEndPos   = ptrRunTail->EndPos ;

                   /* ----  SOURCE: If only 1 text node, reduce to para node  ---- */
                   if ( ptrParaTail->NumTextTags == 1 ) {
//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"/PARA-1textnode...%lx-%lx\n",ptrParaTail->BlockStartPos,ptrParaTail->BlockEndPos);
//fclose(fDebug);
                      for( ptrRunCur=ptrRunHead; ptrRunCur ; ptrRunCur=(R_INFO*)ptrRunCur->ptrNext ) {
                         if ( ptrRunCur->NodeType == NODE_TYPE_TEXT ) {
                            ptrParaTail->BlockStartPos = ptrRunCur->BlockStartPos ;
                            ptrParaTail->BlockEndPos   = ptrRunCur->BlockEndPos ;
//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"/PARA-1textnode-text...%lx-%lx\n",ptrParaTail->BlockStartPos,ptrParaTail->BlockEndPos);
//fclose(fDebug);
                            ptrParaTail->ptrRunList = 0 ;

                            fnFreeRunList( &ptrRunHead ) ;
                            ptrRunHead = 0 ;
                            ptrRunTail = 0 ;
                            ptrRunLastKeep = 0 ;
                            break ;
                         }
                      }
                   }

                   /* ----  SOURCE: If only 1 run node, change para node  ---- */
/* ############################################################################################################################*/
                   if ( ( ptrRunHead ) &&
                        ( ptrRunHead->ptrNext == NULL ) && 
                        ( ptrRunHead->NodeType == NODE_TYPE_TEXT ) ) {
                      ptrParaTail->BlockStartPos = ptrRunHead->BlockStartPos ;
                      ptrParaTail->BlockEndPos   = ptrRunHead->BlockEndPos ;
                      ptrRunHead->StartPos = 0 ;
                      ptrRunHead->EndPos = 0 ;
                      ptrRunHead->BlockStartPos = 0 ;
                      ptrRunHead->BlockEndPos = 0 ;
                   }


                   /* ----  SOURCE: If > 1 text nodes, check to see if any text nodes  ---- */
                   /* ----          use the same properties as another node.           ---- */
                   /* ----          If so, use this as the common properties.   2-3-14 ---- */
                   if ( ( bSetCommonProperty              ) &&
                        ( ptrParaTail->NumTextTags    > 1 ) &&
                        ( ptrRunHead ) ) {
                      usRunMaxMatch = 0 ;
                      usRunMaxID = 0 ;
                      j = 0 ;
                      bCheckCommonProp = TRUE ;
                      bCheckCommonPropBold = FALSE ;
                      ptrCheckCommonPropText = 0 ;
                      usRunCommonPropID = 0 ;
                      for( ptrRunTemp=ptrRunHead ; ptrRunTemp ; ptrRunTemp=(R_INFO*)ptrRunTemp->ptrNext ) {
                         if ( ( ptrRunTemp->NumTextTags > 1 ) ||        /* If > 1 text nodes, or         */
                              ( ( ptrRunTemp->NumTextTags == 1 ) &&     /* If text and tags in same run, */
                                ( ptrRunTemp->NumNeutralTags >= 1 ) ) ) { /* Cannot simplify properties  */
                            usRunMaxID = 0 ;
                            break ;
                         }
                         usRunCount = 0 ;
                         ptrRunCur = 0 ;
                         ++j ;
                         if ( ( ptrRunTemp->BlockStartPos > ptrRunTemp->BlockEndPos ) || /* Empty text node   */
                              ( ptrRunTemp->CommonProperty == COMMONPROPERTY_FORCE_OFF ) ) /* Force break     */
                            continue ; 
                         if ( ( bCheckCommonProp ) &&
                              ( usRunMaxID == 0  ) &&
                              ( ptrRunTemp->NumTextTags == 1 ) &&  
                              ( ptrRunTemp->Concat != CONCAT_TEXT_YES ) ) { 
                            if ( fnCheckCommonProp( ptrRunTemp->Properties ) ) {
                               if ( bCheckCommonPropBold ) 
                                  bCheckCommonProp = FALSE ;
                               else
                                  bCheckCommonPropBold = TRUE ; 
                            } else {
                               if ( ptrCheckCommonPropText ) {
                                  bCheckCommonProp = FALSE ;
                               } else {
                                  ptrCheckCommonPropText = &(ptrRunTemp->CommonProperty) ;
                                  usRunCommonPropID = (USHORT)j ;     
                               }
                            }
                         }

                         for( ptrRunTemp2=ptrRunTemp ; ptrRunTemp2 ; ptrRunTemp2=(R_INFO*)ptrRunTemp2->ptrNext ) {
                            if ( ( ptrRunTemp2->NumTextTags == 1 ) &&    /* Only 1 text block in run   */
                                 ( ( ptrRunTemp2->CommonProperty == 0 ) ||
                                   ( ptrRunTemp2->CommonProperty == COMMONPROPERTY_FORCE_OFF ) ) ) {
                               if ( ptrRunTemp2->BlockStartPos > ptrRunTemp2->BlockEndPos ) { /* Empty text node   */
                                  if ( ( ptrRunTemp2->Concat == CONCAT_TEXT_YES ) &&          /* Concat text 4-14-14 */
                                       ( ptrRunTemp2->ptrPrev ) &&
                                       ( ((R_INFO*)(ptrRunTemp2->ptrPrev))->CommonProperty ) ) {
                                     ptrRunTemp2->CommonProperty = j ;
                                     ++usRunCount ; 
                                  }
                                  continue ; 
                               }

                               if ( ptrRunCur == 0 ) {              /* Identify base properties   */
                                  ptrRunCur = ptrRunTemp2 ; 
//
//       Too big of on an impact on memory reuse.
//                                ptrRunCur->CommonProperty = j ;   /* At least remove tagging    */
//                                usRunCount = 1 ;                  /*   around 1st text node.    */
//
                               } else {
                                  if ( ( ( ptrRunCur->Properties ) &&
                                         ( ptrRunTemp2->Properties  ) &&
                                         ( ! wcscmp( ptrRunCur->Properties, ptrRunTemp2->Properties ) ) ) ||
                                       ( ( ptrRunCur->Properties == NULL ) &&
                                         ( ptrRunTemp2->Properties == NULL  ) ) ) {
                                     ptrRunCur->CommonProperty = j ;  /* Run uses default properties */
                                     ptrRunTemp2->CommonProperty = j ;
                                     ++usRunCount ; 
                                     if ( bDebugBody ) {
                                        fwprintf(fDebug,L"CmnPR_RUN  %d  C=%d  %d",j,usRunCount,bCheckCommonProp);
                                        if ( ptrRunCur->Properties )
                                           fwprintf(fDebug,L"  %d=[%s]",ptrRunCur->SeqNum,ptrRunCur->Properties);
                                        if ( ptrRunTemp2->Properties )
                                           fwprintf(fDebug,L"  %d=[%s]",ptrRunTemp2->SeqNum,ptrRunTemp2->Properties);
                                        fwprintf(fDebug,L"\n");
                                        fflush(fDebug);
                                     }
                                  }
                               }
                            }
                         }
                         if ( usRunCount > usRunMaxMatch ) {
                            usRunMaxMatch = usRunCount ;
                            usRunMaxID = (USHORT)j ;
                         }
                      }
                      if ( bDebugBody ) {
                         fwprintf(fDebug,L"CmnPR_xxx  ID=%d  %d  %d\n",usRunMaxID,usRunMaxMatch,bCheckCommonProp);
                         fflush(fDebug);
                      }
                      if ( ( bCheckCommonProp ) &&
                           ( usRunMaxID == 0  ) &&
                           ( bCheckCommonPropBold ) &&
                           ( ptrCheckCommonPropText ) ) {
                         usRunMaxMatch = 1 ;
                         usRunMaxID = usRunCommonPropID ;
                         *ptrCheckCommonPropText = usRunCommonPropID ;
                      }


                      for( ptrRunCur=ptrRunHead ; ptrRunCur ; ptrRunCur=(R_INFO*)ptrRunCur->ptrNext ) {
                         if ( ptrRunCur->CommonProperty != usRunMaxID ) {
                            if ( ptrRunCur->CommonProperty != COMMONPROPERTY_FORCE_OFF ) 
                               ptrRunCur->CommonProperty = 0 ;
                         } else
                         if ( ( ptrRunCur->CommonProperty != 0 ) &&
                              ( ptrRunCur->CommonProperty != COMMONPROPERTY_FORCE_OFF ) &&
                              ( ptrParaTail->CommonProperty == 0 ) ) {
                            ptrParaTail->CommonProperty = ptrRunCur->SeqNum ; 

                            /* ----------   DEBUG   --------------------------------------- */
                            if ( bDebugBody ) {
                               fwprintf(fDebug,L"CmnPR_PARA=%d\n",ptrParaTail->CommonProperty);
                               fflush(fDebug);
                            }
                         }
                      }
                   }

                }
             }

             /* ----  Free storage used to compress paragraph revision text  ---- */
             for( ptrRunTemp=ptrRunHead ; ptrRunTemp!=0 ; ptrRunTemp=(R_INFO*)ptrRunTemp->ptrNext ) {
                if ( ptrRunTemp->Properties ) {
                   free( ptrRunTemp->Properties ) ;
                   ptrRunTemp->Properties = NULL ; 
                }
             }

             /* ----------   DEBUG   --------------------------------------- */
             if ( bDebugBody ) {
                fwprintf(fDebug,L"/PARA...#%ld   N=%ld   %lx-%lx   %lx-%lx\n",
                   ptrParaTail->SeqNum,ptrParaTail->NodeType,
                   ptrParaTail->StartPos,ptrParaTail->BlockStartPos,ptrParaTail->BlockEndPos,ptrParaTail->EndPos);
                fflush(fDebug);
             }
          }


          continue ;
       } 



       /*====================================================================*/
       /*                                                                    */
       /*  Handle run-level tags.                                            */
       /*                                                                    */
       /*====================================================================*/


       /**********************************************************************/
       /*  Handle start of run of text <w:r> or a neutral tag at run level.  */
       /**********************************************************************/
       if ( ( usRunState == RUN_STATE_NONE ) &&
            ( bReadSource ) ) {

          ++ulRunSeqNum ;                           /* Increment sequence #  */
          if ( ulPrevRunStartPos == 0 )             /* Save prev start pos   */
             ulPrevRunStartPos = ulTagStartPos ;

          /* ----------   DEBUG   --------------------------------------- */
          if ( bDebugBody ) {
             fwprintf(fDebug,L"NewRunSeq#=%ld\n",ulRunSeqNum);
             fflush(fDebug);
          }


          /*******************************************************************/
          /*  Save run element information.                                  */
          /*******************************************************************/
          if ( ! wcscmp( szBaseTag, L"r" ) ) {

     //      if ( fnIsEmptyTag( szTagText ) ) {     /* If an end tag         */
     //         continue;
     //      }


             usRunState = RUN_STATE_RUN ;           /* In run mode now       */

             if ( ( ptrRunTail ) &&                 /* Skip leading nodes    */
                  ( ptrRunTail->NumTextTags    == 0 ) &&
                  ( ptrRunTail->NumNeutralTags == 0 ) ) {
                ulRunSeqNum = ptrRunTail->SeqNum ;
                ptrRunTail->CommonProperty = COMMONPROPERTY_FORCE_OFF ; /* Force break */

             }

             if ( usFieldState == FIELD_STATE_BEGIN ) {
                --ulRunSeqNum ;
             } else {

                /* ----  Allocate a run node.  ----------------------------- */
                if ( ( ! ptrRunHead ) ||          
                     ( ptrRunTail->NumTextTags    > 0 ) ||
                     ( ptrRunTail->NumNeutralTags > 0 ) ) {
                   bAllocate = TRUE ;               /* Allocate new node     */
                } else {                           
                   bAllocate = FALSE ;              /* Reuse existing node   */
                   if ( ptrRunTail->ptrTextList ) 
                      --ulRunSeqNum ; 
                }
                fnAllocateRunNode( &ptrRunHead, &ptrRunTail, ptrParaTail, szTag, bAllocate ) ;

                /* ----  Initialize the run node.  ------------------------- */
                ptrRunTail->SeqNum = ulRunSeqNum ;
                ptrRunTail->TagAction = TAG_ACTION_NONE ;  
                ptrRunTail->TagType = TAG_TYPE_NONE ;  
                ptrRunTail->TagId = TAG_ID_NONE ;  
                fnSaveRunProperties( ptrRunTail, szTagText ) ;
                if ( ptrRunTail->ptrPrev ) 
                   ptrRunTail->StartPos = ((R_INFO*)(ptrRunTail->ptrPrev))->EndPos + 1*sizeof(WCHAR) ; 
                else
                   if ( bInsertedTag ) 
                      ptrRunTail->StartPos = ulTagEndPos + 1*sizeof(WCHAR) ;
                   else
                      ptrRunTail->StartPos = ulTagStartPos ; 
                ptrRunTail->EndPos = 0 ;
                ptrTextHead = 0 ;
                ptrTextTail = 0 ;
                ptrTextLastKeep = 0 ;
             }


             /* ----------   DEBUG   --------------------------------------- */
             if ( bDebugBody ) {
                fwprintf(fDebug,L"RUN...#%ld  NT=%ld  AT=%ld  TT=%ld  ID=%ld   %lx   %lx-%lx   %lx-%lx\n",
                      ptrRunTail->SeqNum,ptrRunTail->NodeType,ptrRunTail->TagAction,ptrRunTail->TagType,ptrRunTail->TagId,ptrRunTail,
                      ptrRunTail->StartPos,ptrRunTail->BlockStartPos,ptrRunTail->BlockEndPos,ptrRunTail->EndPos);
                fflush(fDebug);
             }
          } else {

             /***********************************************************************/
             /*  Save neutral element information at run level.                     */
             /***********************************************************************/
             for( j=0 ; 
                  j<NUM_NEUTRAL_TAGS && wcscmp(szBaseTag,NEUTRAL_TAGS[j].Name) ; 
                  ++j ) ;

             /* ----------   DEBUG   --------------------------------------- */
             if ( bDebugBody ) {
                fwprintf(fDebug,L"RUN_NN...J=%d  %s    B=%s\n",j,szTag,szBaseTag);
                fflush(fDebug);
             }

             /* ----  Determine if known neutral element or not.  ---------- */
             if ( j < NUM_NEUTRAL_TAGS ) {
                ulTagAction = NEUTRAL_TAGS[j].TagAction ;
                ulTagType = NEUTRAL_TAGS[j].TagType ;
                ulTagId = NEUTRAL_TAGS[j].TagId ;
                if ( ! wcscmp( szBaseTag, L"br" ) ) {             /* 5-14-12 */
                   if ( szTag[0] == L'/' ) {
                      ulTagType = TAG_TYPE_END ;      /* End tag of pair     */
                      ulTagId = TAG_ID_BR_BEGIN ;
                   } else
                   if ( fnIsEmptyTag( szTagText ) ) {
                      ulTagType = TAG_TYPE_EMPTY ;    /* Empty tag           */
                      ulTagId = TAG_ID_BR ;
                   } else {
                      ulTagType = TAG_TYPE_BEGIN ;    /* Begin tag of pair   */
                      ulTagId = TAG_ID_BR_BEGIN ;
                   }
                } else
                if ( ! wcscmp( szBaseTag, L"/br" ) ) {            /* 2-12-14 */
                   ulTagType = TAG_TYPE_END ;         /* End tag of pair     */
                   ulTagId = TAG_ID_BR_BEGIN ;
                }
             } else {
                ulTagAction = TAG_ACTION_UNKNOWN ;    /* Unknown neutral  tag*/
                if ( szTag[0] == L'/' ) 
                   ulTagType = TAG_TYPE_END ;         /* End tag of pair     */
                else
                   if ( fnIsEmptyTag( szTagText ) )   
                      ulTagType = TAG_TYPE_EMPTY ;    /* Empty tag           */
                   else
                      ulTagType = TAG_TYPE_BEGIN ;    /* Begin tag of pair   */
                ulTagId = TAG_ID_UNKNOWN ;
             }

             /* ----  Allocate a run node.  -------------------------------- */
             if ( ( ptrParaTail->NumTextTags == 0    ) &&
                  ( ptrParaTail->NumNeutralTags == 0 ) &&
                  ( ulTagAction == TAG_ACTION_SKIP   ) ) {
                bNewNode = FALSE ;                    /* Skip leading nodes  */
                --ulRunSeqNum ; 
             } else 
             if ( ( ulTagId == TAG_ID_UNKNOWN ) &&  /*Combine similar tags   */
                  ( ulTagType == TAG_TYPE_EMPTY ) &&
                  ( ptrRunHead ) &&
                  ( ptrRunTail->TagId == ulTagId ) &&
                  ( ptrRunTail->TagType == ulTagType ) &&
                  ( ! wcscmp( ptrRunTail->Tag, szTag ) ) ) {
                ptrRunTail->EndPos  = ulTagEndPos ;
                bNewNode = FALSE ;
                --ulRunSeqNum ; 
             } else {
                bNewNode = TRUE ;
                if ( ( ptrRunHead ) &&
                     ( ptrRunTail->NumTextTags == 0 ) &&
                     ( ptrRunTail->NumNeutralTags == 0 ) ) {
                   ++ptrParaTail->NumNeutralTags ;
                   ++ptrRunTail->NumNeutralTags ;
                   if ( ( ptrRunTail->TagType == TAG_TYPE_NONE ) ) {
                      ptrRunTail->TagType = TAG_TYPE_EMPTY ;
                   }
                }
             }

             if ( usFieldState == FIELD_STATE_BEGIN ) 
                bNewNode = FALSE ;

             if ( bNewNode ) {
                fnAllocateRunNode( &ptrRunHead, &ptrRunTail, ptrParaTail,szTag, TRUE ) ;

                /* ----  Initialize the run node.  ------------------------- */
                ++(ptrParaTail->NumNeutralTags) ; 
                ++(ptrRunTail->NumNeutralTags) ; 

                ptrRunTail->SeqNum = ptrParaTail->NumTextTags + ptrParaTail->NumNeutralTags ;
                if ( ptrRunTail->ptrPrev ) {
                   ptrRunTail->StartPos = ((R_INFO*)(ptrRunTail->ptrPrev))->EndPos + 1*sizeof(WCHAR) ; 
              //   if ( ((R_INFO*)(ptrRunTail->ptrPrev))->Properties  ) {
              //      free( ((R_INFO*)(ptrRunTail->ptrPrev))->Properties  ) ;
              //      ((R_INFO*)(ptrRunTail->ptrPrev))->Properties = NULL ;
              //   }
                } else {
                   ptrRunTail->StartPos  = ulPrevRunStartPos ; 
                }
                ptrRunTail->EndPos  = ulTagEndPos ; 
                ulPrevRunStartPos = 0 ;
                ptrRunTail->TagAction = ulTagAction ;
                ptrRunTail->TagType = ulTagType ;
                ptrRunTail->TagId = ulTagId ;
                ptrRunTail->NodeType = NODE_TYPE_NEUTRAL ;
                if ( ulTagAction != TAG_ACTION_SKIP ) {
                   if ( ( ! bSetCommonProperty ) ||                /* 2-12-14 */
                        ( ulTagAction != TAG_ACTION_UNKNOWN ) || 
                        ( ulTagType != TAG_TYPE_EMPTY ) ) 
                      ptrRunLastKeep = ptrRunTail ;
                }
                ptrRunTail->CommonProperty = COMMONPROPERTY_FORCE_OFF ; /* Force break */

                /* ----------   DEBUG   ------------------------------------ */
                if ( bDebugBody ) {
                   fwprintf(fDebug,L"NEUT-R...#%ld  N=%ld  A=%ld  T=%ld  I=%ld   %lx   %lx-%lx   %lx-%lx   Last=%lx\n",
                      ptrRunTail->SeqNum,ptrRunTail->NodeType,ptrRunTail->TagAction,ptrRunTail->TagType,ptrRunTail->TagId,ptrRunTail,
                      ptrRunTail->StartPos,ptrRunTail->BlockStartPos,ptrRunTail->BlockEndPos,ptrRunTail->EndPos,ptrRunLastKeep);
                   fflush(fDebug);
                }
             }

             /****************************************************************/
             /*  If last tag was the end tag of a pair, then find the begin  */
             /*  tag and link them together.                                 */
             /****************************************************************/
             if ( ( ptrRunTail ) &&
                  ( ptrRunTail->TagType == TAG_TYPE_END ) ) {

                for( ptrRunTemp=(R_INFO*)(ptrRunTail->ptrPrev), usTextCount=0 ;
                     ptrRunTemp ;
                     ptrRunTemp=(R_INFO*)(ptrRunTemp->ptrPrev) ) {

                   /* ----  Note if text between begin and end tag.  ------- */
                   if ( ptrRunTemp->NodeType == NODE_TYPE_TEXT ) {
                      ++usTextCount ;
                      ptrRunTemp2 = ptrRunTemp ;
                      continue ;
                   }

                   /* ----  Matching begin tag is found.  ------------------ */
                   if ( ( ptrRunTemp->BeginSeqNum == 0 ) &&
                        ( ptrRunTemp->TagType != TAG_TYPE_EMPTY ) &&   /* 10-1-08 */
                        ( ! wcscmp( ptrRunTemp->Tag, &(ptrRunTail->Tag[1]) ) ) ) {

                      /* ----  Text found between begin and end tag.  ------ */
                      if ( usTextCount > 0  ) {
                         ptrRunTemp->BeginSeqNum = ptrRunTail->SeqNum ;
                         ptrRunTail->BeginSeqNum = ptrRunTemp->SeqNum ;

                         /* ----  If only 1 text node, remove begin and end. */
                         if ( usTextCount == 1 ) {
                            ptrRunTemp2->SeqNum = ptrRunTemp->SeqNum ;
                            ptrRunTail->NumNeutralTags -= 2 ;
                            ptrParaTail->NumNeutralTags -= 2 ;

                            ptrRunTemp2->StartPos = ptrRunTemp->StartPos ;
                            ptrRunTemp2->ptrPrev = ptrRunTemp->ptrPrev ;
                            ptrRunTemp->ptrNext = NULL ;
                            fnFreeRunList( &ptrRunTemp ) ;

                            ptrRunTemp2->EndPos = ptrRunTail->EndPos ;
                            ptrRunTemp2->ptrNext = ptrRunTail->ptrNext ;
                            ptrRunTail->ptrNext = NULL ;
                            fnFreeRunList( &ptrRunTail ) ;


                            if ( ptrRunTemp2->ptrPrev != NULL ) {
                               ((R_INFO*)(ptrRunTemp2->ptrPrev))->ptrNext = ptrRunTemp2 ;
                            } else {
                               ptrRunHead = ptrRunTemp2 ;
                               ptrParaTail->ptrRunList = ptrRunHead ;
                            }
                            if ( ptrRunTemp2->ptrNext != NULL ) 
                               ((R_INFO*)(ptrRunTemp2->ptrNext))->ptrPrev = ptrRunTemp2 ;
                            else
                               ptrRunTail = ptrRunTemp2 ;
                            ptrRunLastKeep = ptrRunTail ;
                            ptrRunTail->CommonProperty = COMMONPROPERTY_FORCE_OFF ; /* Force break */


                            /* ----------   DEBUG   ------------------------ */
                            if ( bDebugBody ) {
                               fwprintf(fDebug,L"NEUT-R...#%ld  N=%ld  A=%ld  T=%ld  I=%ld   %lx   %lx-%lx   %lx-%lx  Prev=%lx\n",
                                  ptrRunTail->SeqNum,ptrRunTail->NodeType,ptrRunTail->TagAction,ptrRunTail->TagType,ptrRunTail->TagId,ptrRunTail,
                                  ptrRunTail->StartPos,ptrRunTail->BlockStartPos,ptrRunTail->BlockEndPos,ptrRunTail->EndPos,ptrRunTail->ptrPrev);
                               fflush(fDebug);
                            }
                         }
                      } else {

                         /* ----  No text between begin and end tag.  ------ */
                         for( ; ptrRunTail!=ptrRunTemp ; ptrRunTail=ptrRunTemp2 ) {
                            --(ptrParaTail->NumNeutralTags) ; 
                            --(ptrRunTail->NumNeutralTags) ; 
                            ptrRunTemp2 = (R_INFO*)(ptrRunTail->ptrPrev) ;
                            ptrRunTemp2->EndPos = ptrRunTail->EndPos ;
                            if ( ptrRunLastKeep == ptrRunTail )        /* 2-12-14 */
                               ptrRunLastKeep = ptrRunTemp2 ;
                            fnFreeRunList( &ptrRunTail ) ;
                            if ( ptrRunTemp2 )
                               ptrRunTemp2->ptrNext = 0 ;
                         }
                         ptrRunTail->ptrNext = 0 ;
                         ptrRunTail->TagType = TAG_TYPE_EMPTY ; /* Begin/End handle as empty tag */
                         ptrRunTail->CommonProperty = COMMONPROPERTY_FORCE_OFF ; /* Force break */
                      }
                      break ;
                   }
                }
             }
          }
          continue;
       } 

       /**********************************************************************/
       /*  Ignore run property tags and all tags within this block.          */
       /**********************************************************************/
       if ( ! wcscmp( szBaseTag, L"rPr" ) ) {
          if ( ! fnIsEmptyTag( szTagText ) ) {  /* If not an empty tag       */
             usRunState = RUN_STATE_PROPERTY ;
             if ( ! bSetCommonProperty ) 
                fnSaveRunProperties( ptrRunTail, szTagText ) ;
          }
          if ( bSetCommonProperty )        /* Always save properties  2-3-14 */
             fnSaveRunProperties( ptrRunTail, szTagText ) ; 
          continue ;
       }
       if ( usRunState == RUN_STATE_PROPERTY ) {
          if ( ! wcscmp( szBaseTag, L"/rPr" ) ) 
             usRunState = RUN_STATE_RUN ;
          fnSaveRunProperties( ptrRunTail, szTagText ) ;
          continue ;
       }


       /**********************************************************************/
       /*  Handle end of text run  </w:r>.                                   */
       /**********************************************************************/
       if ( ( ! wcscmp( szBaseTag, L"/r" ) ) &&
            ( usFieldState != FIELD_STATE_BEGIN ) ) {

          usRunState = RUN_STATE_NONE ;            /* Reset run state        */

          if ( ptrRunTail ) 
             if ( bInsertedTag ) 
                ptrRunTail->EndPos = ulTagStartPos - 1*sizeof(WCHAR) ;
             else
                ptrRunTail->EndPos = ulTagEndPos ;
          if ( ptrRunHead ) {

             /* ----  Text exists in this run node.  ----------------------- */
             if ( ptrRunTail->NumTextTags > 0 ) {
                if ( bInsertedTag ) 
                   ptrRunTail->EndPos = ulTagStartPos - 1*sizeof(WCHAR) ;
                else
                   ptrRunTail->EndPos = ulTagEndPos ;

                /*************************************************************/
                /*  Remove any trailing non-text run tags after the last     */
                /*  text unit.  Condenses trailing neutral tags into 1.      */
                /*************************************************************/
                if ( bReadSource ) {
                   if ( ( ptrTextLastKeep ) &&
                        ( ptrTextLastKeep != ptrTextTail ) ) {
                      for( ptrTextTemp=(T_INFO*)ptrTextLastKeep->ptrNext ;
                           ptrTextTemp ; 
                           ptrTextTemp=(T_INFO*)ptrTextTemp->ptrNext ) {
                         --(ptrParaTail->NumNeutralTags) ; 
                         --(ptrRunTail->NumNeutralTags) ; 
                      }
                      ptrTextTemp = (T_INFO*)ptrTextLastKeep->ptrNext ;
                      fnFreeTextList( &ptrTextTemp ) ;
                      ptrTextTail = ptrTextLastKeep ;
                      ptrTextLastKeep = 0 ;
                      ptrTextTail->ptrNext = 0 ;

                   }
                   ptrRunTail->BlockStartPos = ptrTextHead->StartPos ;
                   ptrRunTail->BlockEndPos   = ptrTextTail->EndPos ;

//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"/RUN-?textnode...%lx  %lx\n",ptrRunTail->NumTextTags,ptrRunTail->NumNeutralTags);
//if ( ptrRunHead != ptrRunTail ) 
//fwprintf(fDebug,L"/RUN-?textnode...%lx  \n",((R_INFO*)(ptrRunTail->ptrPrev))->SeqNum);
//fclose(fDebug);
                   if ( ( ptrRunTail->NumTextTags == 1 ) &&
                        ( ptrRunTail->NumNeutralTags > 0 ) ) {
//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"/RUN-1textnode...%lx-%lx\n",ptrRunTail->BlockStartPos,ptrRunTail->BlockEndPos);
//fclose(fDebug);
                      ptrParaTail->NumNeutralTags -= ptrRunTail->NumNeutralTags ; 
                      bFoundText = FALSE ;
                      for( ptrTextCur=ptrTextHead; ptrTextCur ; ptrTextCur=(T_INFO*)ptrTextCur->ptrNext ) {
                         if ( ptrTextCur->NodeType == NODE_TYPE_TEXT ) {
                            ptrRunTail->BlockStartPos = ptrTextCur->BlockStartPos ;
                            ptrRunTail->BlockEndPos   = ptrTextCur->BlockEndPos ;
                            bFoundText = TRUE ;
//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"/RUN-1textnode-text...%lx-%lx\n",ptrRunTail->BlockStartPos,ptrRunTail->BlockEndPos);
//fclose(fDebug);
                            if ( ptrRunHead == ptrRunTail ) 
                               ulRunSeqNum = 1 ;
                            else
                               ulRunSeqNum = ((R_INFO*)(ptrRunTail->ptrPrev))->SeqNum + 1 ;
                            ptrRunTail->SeqNum = ulRunSeqNum ;


                            /*************************************************************/
                            /*  Remove <lastRenderedPageBreak/> when first tag in a      */
                            /*  run block.                                       1-10-14 */
                            /*************************************************************/
                            ptrRunTemp = (R_INFO*)ptrRunTail->ptrPrev ;       
                            ptrTextTemp = (T_INFO*)ptrTextTail->ptrPrev ;
                            if ( ( ptrRunTail->NumNeutralTags == 1 ) &&
                                 ( ptrRunTemp  ) &&
                                 ( ptrTextTemp == ptrTextHead ) &&
                                 ( ptrRunTemp->NumTextTags == 1    ) &&  /* Only 1 text block and 0 neutral */
                                 ( ptrRunTemp->NumNeutralTags == 0 ) &&  /*  tags in previous run.          */
                                 ( bSetCommonProperty ) &&               /* 2-21-14 */
                                 ( ptrTextTemp->TagId == TAG_ID_LASTRENDEREDPAGEBREAK ) &&
                                 ( ptrRunTemp->Concat != CONCAT_TEXT_NO ) &&
                                 ( ptrRunTail->Concat != CONCAT_TEXT_NO ) &&
                                 ( ( ( ptrRunTemp->Properties ) &&       /* Same properties          5-9-14 */
                                     ( ptrRunTail->Properties  ) &&
                                     ( ! wcscmp( ptrRunTemp->Properties, ptrRunTail->Properties ) ) ) ||
                                   ( ( ptrRunTemp->Properties == NULL ) &&
                                     ( ptrRunTail->Properties == NULL  ) ) ) ) {
                               ptrRunTail->Concat = CONCAT_TEXT_YES ;
                               ptrRunTail->SeqNum = ptrRunTemp->SeqNum ; 
                               ulRunSeqNum = ptrRunTail->SeqNum ; 
                               --(ptrParaTail->NumNeutralTags) ; 
                               --(ptrRunTail->NumNeutralTags) ; 
                            }

                         
                            ptrRunTail->ptrTextList = 0 ;

                            fnFreeTextList( &ptrTextHead ) ;
                            ptrTextHead = 0 ;
                            ptrTextTail = 0 ;
                            ptrTextLastKeep = 0 ;
                            break;                        /* 1-21-15 */
                         } else 
                         if ( ( ! bFoundText ) &&         /* 8-19-15 */
                              ( ptrTextCur->TagId == TAG_ID_BR ) ) {
                            ptrRunTail->BreakBeforeText = TRUE ;
                         }
                      }
                   }
                }
             }

             if ( bReadSource ) {

                /* ----------   DEBUG   ------------------------------------ */
                if ( bDebugBody ) {
                   fwprintf(fDebug,L"RUN_/...T#=%d  N#=%d\n",ptrRunTail->NumTextTags,ptrRunTail->NumNeutralTags);
                   fflush(fDebug);
                }

                /* ----  Run consists of only 1 text node.  ---------------- */
                if ( ( ptrRunTail->NumTextTags == 1    ) &&  /* Only 1 text block in run block */
                     ( ptrRunTail->NumNeutralTags == 0 ) &&  /*  and 0 neutral tags in block   */
                     ( ptrRunTail->Concat != CONCAT_TEXT_YES ) ) { /* not concatenated  1-10-14*/
                   ptrRunTail->SeqNum = ptrTextHead->SeqNum ;
                   ptrRunTail->BlockStartPos = ptrTextHead->BlockStartPos ;
                   ptrRunTail->BlockEndPos   = ptrTextHead->BlockEndPos ;
                   ptrRunTail->ptrTextList = 0 ;

                   fnFreeTextList( &ptrTextHead ) ;
                   ptrTextHead = 0 ;
                   ptrTextTail = 0 ;
                   ptrTextLastKeep = 0 ;
                   ptrRunTemp = (R_INFO*)ptrRunTail->ptrPrev ;
//if ( ptrRunTemp ) {
//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"     ... CONCAT?  {%lx} {%lx} [%s]  [%s]  T=%d  N=%d\n",ptrRunTemp->Properties,ptrRunTail->Properties,ptrRunTemp->Properties,ptrRunTail->Properties,ptrRunTemp->NumTextTags,ptrRunTemp->NumNeutralTags);
//fwprintf(fDebug,L"                  %ld   %ld\n",ptrParaTail->SeqNum,ptrRunTail->SeqNum);
//fclose(fDebug);
//}
                   if ( ( ptrRunTemp ) &&
                        ( ptrRunTemp->Properties ) &&
                        ( ptrRunTail->Properties ) &&
                        ( ptrRunTemp->NumTextTags == 1    ) &&  /* Only 1 text block in run block */
                        ( ptrRunTemp->NumNeutralTags == 0 ) &&  /*  and 0 neutral tags in block   */
                        ( ptrRunTemp->Concat != CONCAT_TEXT_NO ) &&
                        ( ptrRunTail->Concat != CONCAT_TEXT_NO ) &&
                        ( ptrRunTemp->CommonProperty != COMMONPROPERTY_FORCE_OFF ) &&
                        ( ptrRunTail->CommonProperty != COMMONPROPERTY_FORCE_OFF ) &&
                        ( ( ! wcscmp( ptrRunTemp->Properties, ptrRunTail->Properties ) ) ||
                          ( wcsstr( ptrRunTemp->Properties, L"<w:hyphen" ) ) ||       /* 11-10-10 */
                          ( wcsstr( ptrRunTail->Properties, L"<w:hyphen" ) ) ) ) {
                      ptrRunTail->Concat = CONCAT_TEXT_YES ;
                      ptrRunTail->SeqNum = ptrRunTemp->SeqNum ; 
                      --ulRunSeqNum ;


//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"     ... CONCAT  {%s}   {%s}\n",ptrRunTemp->Properties,ptrRunTail->Properties);
//fclose(fDebug);
                   } 

                } else {

                   /* ----  Run consists of only 1 neutral node.  ------------- */
                   if ( ( ptrRunTail->NumTextTags == 0    ) &&  /* Only 1 neutral tag in run block*/
                        ( ptrRunTail->NumNeutralTags == 1 ) ) { /*  and 0 text tags in block      */

//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"NEUTRAL/RUN..RUN:...#%ld  %lx-%lx   %lx-%lx\n",
//                ptrRunTail->SeqNum,ptrRunTail->StartPos,ptrRunTail->BlockStartPos,ptrRunTail->BlockEndPos,ptrRunTail->EndPos);
//if ( ptrTextHead ) 
//fwprintf(fDebug,L"             TXT:...#%ld  %lx-%lx   %lx-%lx\n",
//                ptrTextHead->SeqNum,ptrTextHead->StartPos,ptrTextHead->BlockStartPos,ptrTextHead->BlockEndPos,ptrTextHead->EndPos);
//fclose(fDebug);

                      if ( ptrTextHead )                             /* 11-8-11 */
                         ptrRunTail->SeqNum = ptrTextHead->SeqNum ;
                      ptrRunTail->ptrTextList = 0 ;

                      fnFreeTextList( &ptrTextHead ) ;
                      ptrTextHead = 0 ;
                      ptrTextTail = 0 ;
                      ptrTextLastKeep = 0 ;
                   } else
                
                   /* ----  Run consists of only several neutral node.  -------4-22-15------ */
                   if ( ( ptrRunTail->NumTextTags == 0   ) &&  /* Only several neutral tag in run block*/
                        ( ptrRunTail->NumNeutralTags > 1 ) &&  /*  and 0 text tags in block            */
                        ( ptrParaTail->ptrRunList == ptrRunTail ) ) { /* at beginning of paragraph.    */
                      ptrParaTail->EndPos = ptrRunTail->BlockEndPos + 1*sizeof(WCHAR)  ;
                      ptrParaTail->ptrRunList = 0 ;
                
                      fnFreeRunList( &ptrRunHead ) ;
                      ptrRunHead = 0 ;
                      ptrRunTail = 0 ;
                      ptrRunLastKeep = 0 ;
                      ptrTextHead = 0 ;
                      ptrTextTail = 0 ;
                      ptrTextLastKeep = 0 ;
                   } else

                   /* ----  Run consists of 2 or more text/neutral nodes.  ---- */
                   if ( ptrRunTail->NumTextTags + ptrRunTail->NumNeutralTags > 1 ) {
                      ++(ptrParaTail->NumNeutralTags) ; 
                      ++(ptrRunTail->NumNeutralTags) ; 
                      ptrRunTail->SeqNum = ptrParaTail->NumTextTags + ptrParaTail->NumNeutralTags ;
                   } else {

                      /* ----  Run consists of 0 nodes.  ---------------------- */
                      // ????????????????????????????????????????????????????????????????????????
                   }
                }
             }
          }

          /* ----------   DEBUG   --------------------------------------- */
          if ( ( bDebugBody ) &&
               ( ptrRunHead ) ) {
             fwprintf(fDebug,L"/RUN...#%ld  N=%ld  A=%ld  T=%ld  I=%ld   %lx   %lx-%lx   %lx-%lx\n",
                ptrRunTail->SeqNum,ptrRunTail->NodeType,ptrRunTail->TagAction,ptrRunTail->TagType,ptrRunTail->TagId,ptrRunTail,
                ptrRunTail->StartPos,ptrRunTail->BlockStartPos,ptrRunTail->BlockEndPos,ptrRunTail->EndPos);
             fflush(fDebug);
          }

          continue;;
       }  
       if ( ( ! wcscmp( szBaseTag, L"/r" ) ) &&              /* 11-8-11 */
            ( usFieldState == FIELD_STATE_BEGIN ) ) {
          if ( ( bReadSource ) &&
               ( ptrRunTail  ) &&
               ( ptrTextHead ) ) {
             if ( ( ptrRunTail->NumTextTags == 0    ) &&  /* Only 1 neutral tag in run block*/
                  ( ptrRunTail->NumNeutralTags == 1 ) ) { /*  and 0 text tags in block      */
                if ( bDebugBody ) {
                   fwprintf(fDebug,L"NEUTRAL/run..RUN:...#%ld  %lx-%lx   %lx-%lx\n",
                                   ptrRunTail->SeqNum,ptrRunTail->StartPos,ptrRunTail->BlockStartPos,ptrRunTail->BlockEndPos,ptrRunTail->EndPos);

                   if ( ptrTextHead )
                      fwprintf(fDebug,L"             TXT:...#%ld  %lx-%lx   %lx-%lx\n",
                                      ptrTextHead->SeqNum,ptrTextHead->StartPos,ptrTextHead->BlockStartPos,ptrTextHead->BlockEndPos,ptrTextHead->EndPos);
                   fflush(fDebug);
                }
                if ( ptrRunTail->EndPos == 0 ) {
                   if ( ptrTextHead ) 
                      ptrRunTail->SeqNum = ptrTextHead->SeqNum ;
                   ptrRunTail->EndPos = ulTagEndPos ;
                   ptrRunTail->ptrTextList = 0 ;
                   fnFreeTextList( &ptrTextHead ) ;
                   ptrTextHead = 0 ;
                   ptrTextTail = 0 ;
                   ptrTextLastKeep = 0 ;
                   if ( bDebugBody ) {
                      fwprintf(fDebug,L"NEUTRAL/run..run:...#%ld  %lx-%lx   %lx-%lx\n",
                                      ptrRunTail->SeqNum,ptrRunTail->StartPos,ptrRunTail->BlockStartPos,ptrRunTail->BlockEndPos,ptrRunTail->EndPos);
                      fflush(fDebug);
                   }
                }
             }
          }
       }




       /*====================================================================*/
       /*                                                                    */
       /*  Handle text-level tags.                                           */
       /*                                                                    */
       /*====================================================================*/


       /**********************************************************************/
       /*  Handle start of text <w:t>.                                       */
       /**********************************************************************/
       if ( ( ! wcscmp( szBaseTag, L"t" ) ) ||
            ( ( usFieldState == FIELD_STATE_NONE ) &&
              ( ! wcscmp( szBaseTag, L"instrText" ) ) ) ||
            ( ( usFieldState == FIELD_STATE_NONE ) &&
              ( ! wcscmp( szBaseTag, L"delText" ) ) ) ) {

     //   if ( fnIsEmptyTag( szTagText ) ) {     /* If an end tag         */
     //      continue;
     //   }
          if ( usFieldState == FIELD_STATE_BEGIN ) 
             continue ;

          if ( ! wcscmp( szBaseTag, L"t" ) )
             usTextType = 1 ;
          else
          if ( ! wcscmp( szBaseTag, L"instrText" ) )
             usTextType = 2 ;
          else
             usTextType = 3 ;

          /* ----  Allocate a text node.  ---------------------------------- */
          fnAllocateTextNode( &ptrTextHead, &ptrTextTail, ptrRunTail, szTag, TRUE ) ;
          ptrTextCur = ptrTextTail ;
    
          /* ----  Initialize the text node.  ------------------------------ */
          ++(ptrParaTail->NumTextTags) ; 
          ptrParaTail->NodeType = NODE_TYPE_TEXT ;

          ++(ptrRunTail->NumTextTags) ; 
          ptrRunTail->NodeType = NODE_TYPE_TEXT ;
          ptrRunTail->TagAction = TAG_ACTION_TEXT ;
          ptrRunTail->TagType = TAG_TYPE_NONE ;
          ptrRunTail->TagId = TAG_ID_TEXT ;

          ptrTextTail->NodeType = NODE_TYPE_TEXT ;
          ptrTextTail->TagAction = TAG_ACTION_TEXT ;
          ptrTextTail->TagType = TAG_TYPE_NONE ;
          ptrTextTail->TagId = TAG_ID_TEXT ;
          ptrTextTail->SeqNum = ptrParaTail->NumTextTags + ptrParaTail->NumNeutralTags ;
          if ( ptrTextTail->ptrPrev ) 
             ptrTextTail->StartPos = ((T_INFO*)(ptrTextTail->ptrPrev))->EndPos + 1*sizeof(WCHAR) ; 
          else
             ptrTextTail->StartPos = ulTagStartPos ; 
          ptrTextTail->BlockStartPos = ulTagEndPos + 1*sizeof(WCHAR) ;

          if ( ( ! wcscmp( szBaseTag, L"t" ) ) &&
               ( fnIsEmptyTag( szTagText ) ) ) {  /* If an empty tag       */
             ptrTextTail->BlockStartPos = ulTagEndPos - 1*sizeof(WCHAR) ;
             ptrTextTail->BlockEndPos = ulTagEndPos - 2*sizeof(WCHAR) ;
             ptrTextTail->EndPos = ulTagEndPos ;
          }

          ulPrevRunStartPos = 0 ;
          ptrRunLastKeep = ptrRunTail ;
          ptrTextLastKeep = ptrTextTail ;
          if ( ! wcscmp( szBaseTag, L"delText" ) ) {
             ptrRunTail->TextDel = TRUE ;
             if ( bDebugBody ) {
                fwprintf(fDebug,L">>  DELTEXT\n");
             }
          }

          /* ----------   DEBUG   --------------------------------------- */
          if ( bDebugBody ) {
             fwprintf(fDebug,L"TEXT...#%ld  N=%ld  A=%ld  T=%ld  I=%ld   %lx   %lx-%lx   %lx-%lx\n",
                ptrTextTail->SeqNum,ptrTextTail->NodeType,ptrTextTail->TagAction,ptrTextTail->TagType,ptrTextTail->TagId,ptrTextTail,
                ptrTextTail->StartPos,ptrTextTail->BlockStartPos,ptrTextTail->BlockEndPos,ptrTextTail->EndPos);
             fflush(fDebug);
          }
          continue ;
       } 

       /***********************************************************************/
       /*  Handle end of text </w:t>.                                         */
       /***********************************************************************/
       if ( ( ! wcscmp( szBaseTag, L"/t" ) ) ||
            ( ( usTextType == 2 ) && 
              ( ! wcscmp( szBaseTag, L"/instrText" ) ) ) ||
            ( ( usTextType == 3 ) && 
              ( ! wcscmp( szBaseTag, L"/delText" ) ) ) ) {
          if ( ( ! wcscmp( szBaseTag, L"/instrText" ) ) ||
               ( ! wcscmp( szBaseTag, L"/delText"   ) ) ) {
             ptrRunTail->Concat = CONCAT_TEXT_NO ;
             if ( ! wcscmp( szBaseTag, L"/delText"   ) ) {
             ptrRunTail->CommonProperty = COMMONPROPERTY_FORCE_OFF ; /* Force break */
             ptrParaTail->CommonProperty = COMMONPROPERTY_FORCE_OFF ; /* Force break */
             }
          }
          if ( ptrTextHead ) {
             ptrTextTail->EndPos  = ulTagEndPos ;
             ptrTextTail->BlockEndPos = ulTagStartPos - 1*sizeof(WCHAR) ;

             /* ----------   DEBUG   --------------------------------------- */
             if ( bDebugBody ) {
                fwprintf(fDebug,L"/TEXT...#%ld  A=%ld  N=%ld  T=%ld  I=%ld   %lx   %lx-%lx   %lx-%lx\n",
                   ptrTextTail->SeqNum,ptrTextTail->NodeType,ptrTextTail->TagAction,ptrTextTail->TagType,ptrTextTail->TagId,ptrTextTail,
                   ptrTextTail->StartPos,ptrTextTail->BlockStartPos,ptrTextTail->BlockEndPos,ptrTextTail->EndPos);
                fflush(fDebug);
             }
          }
          continue ;
       } 
    
       /***********************************************************************/
       /*  Handle other items imbedded in a run of text.                      */
       /***********************************************************************/
       if ( ( usRunState == RUN_STATE_RUN ) &&
            ( bReadSource ) ) {
    
          /*******************************************************************/
          /*  Save neutral element information.                              */
          /*******************************************************************/
          for( j=0 ; 
               j<NUM_NEUTRAL_TAGS && wcscmp(szBaseTag,NEUTRAL_TAGS[j].Name) ; 
               ++j ) ;

          /* ----------   DEBUG   ------------------------------------------ */
          if ( bDebugBody ) {
             fwprintf(fDebug,L"RUN_NN...J=%d  %s    B=%s\n",j,szTag,szBaseTag);
             fflush(fDebug);
          }

          /* ----  Determine if known neutral element or not.  ------------- */
          if ( j < NUM_NEUTRAL_TAGS ) {
             ulTagAction = NEUTRAL_TAGS[j].TagAction ;
             ulTagType = NEUTRAL_TAGS[j].TagType ;
             ulTagId = NEUTRAL_TAGS[j].TagId ;
             if ( ! wcscmp( szBaseTag, L"br" ) ) {             /* 5-14-12 */
                if ( szTag[0] == L'/' ) {
                   ulTagType = TAG_TYPE_END ;      /* End tag of pair        */
                   ulTagId = TAG_ID_BR_BEGIN ;
                } else
                if ( fnIsEmptyTag( szTagText ) ) {
                   ulTagType = TAG_TYPE_EMPTY ;    /* Empty tag              */
                   ulTagId = TAG_ID_BR ;
                } else {
                   ulTagType = TAG_TYPE_BEGIN ;    /* Begin tag of pair      */
                   ulTagId = TAG_ID_BR_BEGIN ;
                }
             }
          } else {
             ulTagAction = TAG_ACTION_UNKNOWN ;    /* Unknown neutral  tag   */
             if ( szTag[0] == L'/' ) 
                ulTagType = TAG_TYPE_END ;         /* End tag of pair        */
             else
                if ( fnIsEmptyTag( szTagText ) )  
                   ulTagType = TAG_TYPE_EMPTY ;    /* Empty tag              */
                else
                   ulTagType = TAG_TYPE_BEGIN ;    /* Begin tag of pair      */
             ulTagId = TAG_ID_UNKNOWN ;
          }

          if ( ( ptrRunTail->NumTextTags == 0    ) &&
               ( ptrRunTail->NumNeutralTags == 0 ) &&
               ( ( ulTagAction == TAG_ACTION_SKIP ) || 
                 ( ulTagType   == TAG_TYPE_END    ) ) ) { /* End tag at run start */
             bNewNode = FALSE ;                    /* Skip leading nodes     */
          } else {
             bNewNode = TRUE ;
          }

          if ( ulPrevRunStartPos == 0 )            /* Save prev start pos    */
             ulPrevRunStartPos = ulTagStartPos ;
    
          if ( usFieldState == FIELD_STATE_BEGIN ) 
             bNewNode = FALSE ;                    /* No node in field begin */

      //  if ( ( ptrRunTail->ptrPrev ) &&
      //       ( ((R_INFO*)(ptrRunTail->ptrPrev))->Properties ) ) {
      //     free( ((R_INFO*)(ptrRunTail->ptrPrev))->Properties ) ;
      //     ((R_INFO*)(ptrRunTail->ptrPrev))->Properties = NULL ;
      //  }

          /*******************************************************************/
          /*  The <w:fldChar> element is handled uniquely, based on          */
          /*  fldCharType attribute value:                                   */
          /*     "begin"      Beginning of section.  No trans. text.         */
          /*     "separator"  Separator of section   Trans. text may follow. */
          /*     "end"        End of section.                                */
          /*******************************************************************/
          if ( ulTagId == TAG_ID_FLDCHAR_BEGIN ) {
             if ( fnGetAttributeValue( szTagText, L"fldCharType", szValue ) ) {
                if ( ! wcscmp( szValue, L"begin" ) ) {
                   usFieldStateNest[++usFieldStateNestLvl] = usFieldState ;
                   usFieldState = FIELD_STATE_BEGIN ;
                   ulTagId = TAG_ID_FLDCHAR_BEGIN ;
                } else
                if ( ! wcscmp( szValue, L"separate" ) ) {
                   usFieldState = FIELD_STATE_SEPARATE ;
                   bNewNode = FALSE ;
                   ulPrevRunStartPos = 0 ;
                } else
                if ( ! wcscmp( szValue, L"end" ) ) {
                   usFieldState = FIELD_STATE_END ;
                   ulTagId = TAG_ID_FLDCHAR_END ;
                }
             }
          }
    
          if ( bNewNode ) {
             fnAllocateTextNode( &ptrTextHead, &ptrTextTail, ptrRunTail, szTag, TRUE ) ;
             ptrTextCur = ptrTextTail ;
    
             /* ----  Initialize the run node.  ---------------------------- */
             ++(ptrParaTail->NumNeutralTags) ; 
             ++(ptrRunTail->NumNeutralTags) ; 
             ptrTextTail->NodeType = NODE_TYPE_NEUTRAL ;
             if ( ptrRunTail->TagAction == TAG_ACTION_NONE ) 
                ptrRunTail->TagAction = ulTagAction ;
             if ( ptrRunTail->TagType == TAG_TYPE_NONE ) 
                ptrRunTail->TagType = ulTagType ;
             if ( ptrRunTail->TagId == TAG_ID_NONE ) 
                ptrRunTail->TagId = ulTagId ;
    
             ptrTextTail->SeqNum = ptrParaTail->NumTextTags + ptrParaTail->NumNeutralTags ;
             if ( ptrTextTail->ptrPrev ) 
                ptrTextTail->StartPos = ((T_INFO*)(ptrTextTail->ptrPrev))->EndPos + 1*sizeof(WCHAR) ; 
             else
                ptrTextTail->StartPos = ulTagStartPos ; 
             ptrTextTail->EndPos  = ulTagEndPos ; 
             ulPrevRunStartPos = 0 ;
             ptrTextTail->TagAction = ulTagAction ;
             ptrTextTail->TagType = ulTagType ;
             ptrTextTail->TagId = ulTagId ;
             if ( ulTagAction != TAG_ACTION_SKIP ) {
                ptrTextLastKeep = ptrTextTail ;
                ptrRunLastKeep = ptrRunTail ;
             }

             /* ----------   DEBUG   --------------------------------------- */
             if ( bDebugBody ) {
                fwprintf(fDebug,L"NEUT-T...#%ld  N=%ld  A=%ld  T=%ld  I=%ld   %lx   %lx-%lx   %lx-%lx  Prev=%lx\n",
                   ptrTextTail->SeqNum,ptrTextTail->NodeType,ptrTextTail->TagAction,ptrTextTail->TagType,ptrTextTail->TagId,ptrTextTail,
                   ptrTextTail->StartPos,ptrTextTail->BlockStartPos,ptrTextTail->BlockEndPos,ptrTextTail->EndPos,ptrTextTail->ptrPrev);
                fflush(fDebug);
             }
          }


          /****************************************************************/
          /*  If last tag was the end tag of a pair, then find the begin  */
          /*  tag and link them together.                                 */
          /****************************************************************/
          if ( ( ptrTextTail ) &&
               ( ptrTextTail->TagType == TAG_TYPE_END ) ) {
             for( ptrTextTemp=(T_INFO*)(ptrTextTail->ptrPrev), usTextCount=0 ;
                  ptrTextTemp ;
                  ptrTextTemp=(T_INFO*)(ptrTextTemp->ptrPrev) ) {

                /* ----  Note if text between begin and end tag.  ------- */
                if ( ptrTextTemp->NodeType == NODE_TYPE_TEXT ) {
                   ++usTextCount ;
                   ptrTextTemp2 = ptrTextTemp ;
                   continue ;
                }

                /* ----  Matching begin tag is found.  ------------------ */
                if ( ( ptrTextTemp->BeginSeqNum == 0 ) &&
                     ( ! wcscmp( ptrTextTemp->Tag, &(ptrTextTail->Tag[1]) ) ) ) {

                   /* ----  Text found between begin and end tag.  ------ */
                   if ( usTextCount > 0  ) {
                      ptrTextTemp->BeginSeqNum = ptrTextTail->SeqNum ;
                      ptrTextTail->BeginSeqNum = ptrTextTemp->SeqNum ;

                      /* ----  If only 1 text node, remove begin and end. */
                      if ( usTextCount == 1 ) {
                         ptrTextTemp2->SeqNum = ptrTextTemp->SeqNum ;
                         ptrRunTail->NumNeutralTags -= 2 ;
                         ptrParaTail->NumNeutralTags -= 2 ;

                         ptrTextTemp2->StartPos = ptrTextTemp->StartPos ;
                         ptrTextTemp2->ptrPrev = ptrTextTemp->ptrPrev ;
                         ptrTextTemp->ptrNext = NULL ;
                         fnFreeTextList( &ptrTextTemp ) ;

                         ptrTextTemp2->EndPos = ptrTextTail->EndPos ;
                         ptrTextTemp2->ptrNext = ptrTextTail->ptrNext ;
                         ptrTextTail->ptrNext = NULL ;
                         fnFreeTextList( &ptrTextTail ) ;


                         if ( ptrTextTemp2->ptrPrev != NULL ) {
                            ((T_INFO*)(ptrTextTemp2->ptrPrev))->ptrNext = ptrTextTemp2 ;
                         } else {
                            ptrTextHead = ptrTextTemp2 ;
                            ptrRunTail->ptrTextList = ptrTextHead ;
                         }
                         if ( ptrTextTemp2->ptrNext != NULL ) 
                            ((T_INFO*)(ptrTextTemp2->ptrNext))->ptrPrev = ptrTextTemp2 ;
                         else
                            ptrTextTail = ptrTextTemp2 ;
                         ptrTextLastKeep = ptrTextTail ;


                         if ( bDebugBody ) {
                            fwprintf(fDebug,L"NEUT-T...#%ld  N=%ld  A=%ld  T=%ld  I=%ld   %lx   %lx-%lx   %lx-%lx  Prev=%lx\n",
                               ptrTextTail->SeqNum,ptrTextTail->NodeType,ptrTextTail->TagAction,ptrTextTail->TagType,ptrTextTail->TagId,ptrTextTail,
                               ptrTextTail->StartPos,ptrTextTail->BlockStartPos,ptrTextTail->BlockEndPos,ptrTextTail->EndPos,ptrTextTail->ptrPrev);
                            fflush(fDebug);
                         }
                      }
                   } else {

                      /* ----  No text between begin and end tag.  ------ */
                      for( ; ptrTextTail!=ptrTextTemp ; ptrTextTail=ptrTextTemp2 ) {
                         --(ptrParaTail->NumNeutralTags) ; 
                         --(ptrRunTail->NumNeutralTags) ; 
                         ptrTextTemp2 = (T_INFO*)(ptrTextTail->ptrPrev) ;
                         ptrTextTemp2->EndPos = ptrTextTail->EndPos ;
                         fnFreeTextList( &ptrTextTail ) ;
                         if ( ptrTextTemp2 )
                            ptrTextTemp2->ptrNext = 0 ;
                      }
                      ptrTextTail->ptrNext = 0 ;
                      ptrTextTail->TagType = TAG_TYPE_EMPTY ; /* Begin/End handle as empty tag */
                   }
                   break ;
                }
             }
          }

          if ( ulTagId == TAG_ID_FLDCHAR_END ) {
             if ( usFieldStateNestLvl > 0 ) {
                usFieldState = usFieldStateNest[usFieldStateNestLvl--] ;
                if ( usFieldState > 0 )
                   ulTagId = TAG_ID_FLDCHAR_BEGIN ;
             }
          }

          continue ;
    
       }
    
    }  /* End reading file for parsing */



    *ptrList = ptrParaHead ; 

    return( usReturn ) ;
}



/****************************************************************************/
/*                                                                          */
/* fnGetXMLTag                                                              */
/*                                                                          */
/* Get the next XML tag from the input data and return it.                  */
/*                                                                          */
/* Input:      szInput     - Input line being processed.                    */
/*             usIndex     - Current index into input line.                 */
/*             usInputPos  - Current file position for current input line.  */
/* Output:     szTag       - Returned name of tag.                          */
/*             szTagText   - Returned tag text.                             */
/*             ulStartPos  - Returned starting file position of tag.        */
/*             ulEndPos    - Returned ending file position of tag.          */
/*             bPartialTag - TRUE=Partial tag retrieved.                    */
/* Return:     0           - Tag found and returned.                        */
/*             1           - Error, tag not returned.                       */
/*             2           - End of file, no tag returned.                  */
/*                                                                          */
/****************************************************************************/

USHORT fnGetXMLTag( WCHAR *szInput, ULONG *ulIndex,
                    ULONG *ulInputPos, WCHAR *szTag, WCHAR *szTagText, 
                    ULONG *ulStartPos, ULONG *ulEndPos, BOOL *bPartialTag ) 
{

   WCHAR     szTagName[XML_TAG_LEN] ;
   WCHAR     QuoteChar ;
   WCHAR     *ptr ;
   USHORT    usFileNameState = 0 ;
   USHORT    i, s, t  ;
   ULONG     ulBaseIndex ;
   BOOL      bTagFound = FALSE ;
   BOOL      bDebugTextFound = FALSE ;
   BOOL      bInComment = FALSE ;
   BOOL      bInProcInstr = FALSE ;
   BOOL      bInCdata = FALSE ;
   BOOL      bEndFile = FALSE ;
   BOOL      bQuoteEnd;
   BOOL      bForever = TRUE ;


   *ulStartPos = 0 ;
   *ulEndPos = 0 ;
   szTagText[0] = 0 ;
   i = (USHORT)*ulIndex ;
   ulBaseIndex = i ;
   if ( *bPartialTag ) {
      *bPartialTag = FALSE ;
      bTagFound = TRUE ;
      *ulStartPos = *ulInputPos + i*sizeof(WCHAR) ;
      wcscpy( szTagText, &szInput[i] ) ;
   } else {
      szTag[0] = 0 ;
   }
   --i ;
   t = 0 ;

   while ( bForever ) {
      ++i ;

      /***********************************************************************/
      /*  If end of this record, get next record to process.                 */
      /***********************************************************************/
      if ( szInput[i] == NULL ) {
         if ( fnGetXMLRcd( szInput, ulInputPos ) ) {
            if ( bTagFound ) {
               t = (USHORT)wcslen( szTagText ) ;
               wcscat( szTagText, szInput ) ;
            }
            ulBaseIndex = 0 ;
            i = 0 ;
         } else {
            bEndFile = TRUE ;
            break ;
         }
      }

      /***********************************************************************/
      /*  If comment text, ignore the text.                                  */
      /***********************************************************************/
      if ( bInComment ) {
         if ( ! wcsncmp( &szInput[i], L"-->", 3 ) ) {
            i += 2 ;
            bInComment = FALSE ;
         }
         if ( usFileNameState ) {       /* Looking for new file name  1-5-11 */
            if ( usFileNameState == 1 ) {      /* Looking for "TWB" ID       */
               if ( ! iswspace(szInput[i]) ) {
                  if ( ! wcsncmp( &szInput[i], L"TWB ", 4 ) ) {
                     usFileNameState = 2 ;
                     i += 3 ;
                  } else
                     usFileNameState = 0 ;
               }
            } else 
            if ( ( usFileNameState == 2 ) &&   /* Looking for file name      */
                 ( ! iswspace(szInput[i]) ) ) {
               usFileNameState = 0 ;
               bTagFound = TRUE ;
               ulBaseIndex = i ;
               *ulStartPos = *ulInputPos + i*sizeof(WCHAR) ;
               wcscpy( szTag, TWB_FILENAME ) ;
               wcscpy( szTagText, &szInput[i] ) ;
               ptr = wcsstr( szTagText, L" -->" ) ; 
               if ( ptr ) 
                  *ptr = NULL ;
               i += (USHORT)wcslen( &szInput[i] ) - 1 ;
               *ulEndPos = *ulInputPos + (i)*sizeof(WCHAR) ;
               break ;
            }
         }
         continue ;
      }

      /***********************************************************************/
      /*  If CDATA tag, ignore all text until CDATA end tag found.           */
      /***********************************************************************/
      if ( bInCdata ) {
         if ( ! wcsncmp( &szInput[i], L"]]>", 3 ) ) {
            bInCdata = FALSE ;
            i += 2 ;
         } else {
            continue ;
         }
      }

      /***********************************************************************/
      /*  If processing instruction tag (<?xxx?>), ignore all text until the */
      /*  end of this tag is found.                                          */
      /***********************************************************************/
      if ( bInProcInstr ) {
         if ( ! wcsncmp( &szInput[i], L"?>", 2 ) ) {
            bInProcInstr = FALSE ;
            ++i ;
         } else {
            continue ;
         }
      }


      /***********************************************************************/
      /*  If "<" found, either start a new tag or terminate current tag.     */
      /***********************************************************************/
      if ( ( szInput[i] == L'<' ) &&
           ( ! bInCdata ) ) {
         if ( bTagFound ) {                         /* Ignore invalid '<'    */
         }
         if ( ! wcsncmp( &szInput[i+1], L"!--", 3 ) ) { /* Found file name   */ 
            bInComment = TRUE ;
            usFileNameState = 1 ;             /* Look for start of file name */
            i += 3 ;
            continue ;
         }

         /* ----------   DEBUG   --------------------------------------- */
         if ( ( bDebugBody ) &&
              ( bDebugTextFound ) ) {
            fwprintf(fDebug,L"]\n");
            fflush(fDebug);

         }

         wcsncpy( szTagName, &szInput[i+1], sizeof(szTagName)/sizeof(WCHAR) ) ;
         szTagName[sizeof(szTagName)/sizeof(WCHAR)-1] = NULL ;
         bInCdata = FALSE ;
         if ( !wcsncmp( szTagName, L"![CDATA[", 8 ) ) {
            bInCdata = TRUE ;
            szTagName[7] = NULL ;
         } else {
            if ( ( iswalpha(szTagName[0])          ) ||
                 ( wcschr( L"._", szTagName[0] )   ) ||
                 ( ( wcschr( L"/?!", szTagName[0]  ) ) &&
                   ( ( iswalpha(szTagName[1])      ) ||
                     ( wcschr( L"._", szTagName[1] ) ) ) ) ) {
               for( s=1 ;
                    szTagName[s]!=NULL &&
                    ( ( iswalnum(szTagName[s])        ) ||
                      ( wcschr( L".-_:", szTagName[s] ) ) ) ;
                    ++s ) ;
               szTagName[s] = NULL ;
            } else {
               if ( ! wcsncmp( szTagName, L"~+:",  3 ) ) {
                  szTagName[2] = NULL ;
               } else {
                  if ( ! wcsncmp( szTagName, L"/~+:", 4 ) ) {
                     szTagName[3] = NULL ;
                  } else {
                     continue ;               /* '<' is not a tag */
                  }
               }
            }
         }

         bTagFound = TRUE ;
         *ulStartPos = *ulInputPos + i*sizeof(WCHAR) ;
         wcscpy( szTag, szTagName ) ;
         wcscpy( szTagText, &szInput[i] ) ;
//fDebug=fopen("c:\\daw.daw","ab");    
//fwprintf(fDebug,L"\nTag:  #%s#\n",szTag);
//fclose(fDebug);
         ulBaseIndex = i ;
         t = 0 ;
         if ( szTag[0] == L'?' ) {
            bInProcInstr = TRUE ;
         }
      }

      /***********************************************************************/
      /*  If no start of tag found yet, then keep looking.                   */
      /***********************************************************************/
      if ( ! bTagFound ) {

         /* ----------   DEBUG   --------------------------------------- */
         if ( bDebugBody ) {
            if ( ( ! iswspace(szInput[i]) ) ||
                 ( bDebugTextFound ) ) {
               if ( ! bDebugTextFound ) {
                  bDebugTextFound = TRUE ; 
                  fwprintf(fDebug,L"NON-TAG TEXT...[");
                  if ( bInDebugBinData ) 
                     fwprintf(fDebug,L"xxx");
                  fflush(fDebug);
               }
               if ( ! bInDebugBinData ) {
                  fwprintf(fDebug,L"%c",szInput[i]);
                  fflush(fDebug);
               }
            }
         }

         continue ;
      }

      /***********************************************************************/
      /*  If ending ">" was found, end this tag.                             */
      /***********************************************************************/
      if ( szInput[i] == L'>' ) {
         *ulEndPos = *ulInputPos + i*sizeof(WCHAR) ;
         break ;
      }

      /***********************************************************************/
      /*  If quote, accumulate this text and then continue.                  */
      /***********************************************************************/
      if ( wcschr( L"\"\'", szInput[i] ) ) {
         QuoteChar = szInput[i] ;
         ++i ;
         if ( ( i > 10 ) &&
              ( ! wcsncmp( &szInput[i-11], L"o:gfxdata=", 10 ) ) ) {
            bInGfxdata = TRUE ; 
         }
         bQuoteEnd = FALSE ;
         while ( ! bQuoteEnd ) {
            if ( szInput[i] == NULL ) {
               if ( fnGetXMLRcd( szInput, ulInputPos ) ) {
                  t = (USHORT)wcslen( szTagText ) ;
                  wcscat( szTagText, szInput ) ;
                  ulBaseIndex = 0 ;
                  i = 0 ;
               } else {
                  bEndFile = TRUE ;
                  break ;
               }
            }
            if ( szInput[i] == QuoteChar ) {
               bQuoteEnd = TRUE ;
               bInGfxdata = FALSE ; 
               bInLongAttr = FALSE ; 
            }

            /***********************************************************************/
            /*  If tag is too long, then truncate it and handle as partial tag.    */
            /***********************************************************************/
            if ( ( bInGfxdata ) &&
                 ( t + i - ulBaseIndex > 7500 ) ) {
               *bPartialTag = TRUE ; 
               szTagText[t+i-ulBaseIndex-1] = '/' ;
               szTagText[t+i-ulBaseIndex] = '>' ;
               *ulEndPos = *ulInputPos + i*sizeof(WCHAR) ;
               szInput[i+1] = QuoteChar ;
               break;
            }
            if ( t + i - ulBaseIndex > 7500 ) {
               bInLongAttr = TRUE ;
               *bPartialTag = TRUE ; 
               szTagText[t+i-ulBaseIndex-1] = '/' ;
               szTagText[t+i-ulBaseIndex] = '>' ;
               *ulEndPos = *ulInputPos + i*sizeof(WCHAR) ;
               szInput[i+1] = QuoteChar ;
               break;
            }
            ++i ;
         }
         if ( ( bInGfxdata  ) ||
              ( bInLongAttr ) ) 
            break ;
         --i ;
         continue ;
      }
   }

   t += (USHORT)(i - ulBaseIndex + 1);
   szTagText[t] = NULL ;
   if ( *ulEndPos > 0 ) {
      ++i ;
   } else {
      *ulEndPos = *ulStartPos + wcslen(szTagText)*sizeof(WCHAR) ;
   }
   *ulIndex = i ;


   if ( szTagText[0] != NULL )
      return 0 ;
   if ( bEndFile )
      return 2 ;
   return 1 ;
}

/****************************************************************************/
/*                                                                          */
/* fnGetAttributeValue                                                      */
/*                                                                          */
/* Find the text for the specified attribute.                               */
/*                                                                          */
/* Input:      szTagText   - The entire tag to process.                     */
/*             szAttr      - The attribute keyword to find.                 */
/* Output:     szAttrValue - The attribute's value to return.               */
/*                                                                          */
/****************************************************************************/

BOOL fnGetAttributeValue( WCHAR  *szTagText, WCHAR  *szAttr, WCHAR  *szAttrValue)
{
   WCHAR     szScanAttr[80] ;   
   WCHAR     AttrQuote ;
   WCHAR     *ptrAttr ;
   WCHAR     *ptr, *ptr2 ;
   BOOL      bReturn = 0 ;

   szAttrValue[0] = NULL ;
// szScanAttr[0] = L' ' ;
// wcscpy( &szScanAttr[1], szAttr ) ;
   wcscpy( szScanAttr, szAttr ) ;

   for( ptrAttr=wcsstr(szTagText,szScanAttr ) ;  
        ptrAttr ; 
        ptrAttr=wcsstr(ptrAttr+1,szScanAttr ) ) {
      if ( ( ptrAttr == szTagText ) ||
           ( ( *(ptrAttr-1) != L' ' ) &&
             ( *(ptrAttr-1) != L':' ) ) ) 
         continue;
      ptr = ptrAttr ;
      for( ptr+=wcslen(szScanAttr) ; *ptr && iswspace(*ptr) ; ++ptr ) ;
      if ( *ptr == L'=' ) {
         ++ptr ;
         for( ptr2=0 ; *ptr && iswspace(*ptr) ; ++ptr ) ;
         if ( ( *ptr == L'\'' ) ||
              ( *ptr == L'\"' ) ) {
            AttrQuote = *ptr ;
            for( ptr2=ptr+1 ; *ptr2 && (*ptr2!=AttrQuote) ; ++ptr2 ) ;
         } 
         if ( ptr2 ) {
            wcscpy( szAttrValue, (ptr+1) ) ;
            szAttrValue[ ptr2-ptr-1 ] = NULL ;
            bReturn = TRUE ;
         }
      }
   }
   return( bReturn ) ;
}

/****************************************************************************/
/*                                                                          */
/* fnGetNextAttributeValue                                                  */
/*                                                                          */
/* Find the next attribute and its associated value.                        */
/*                                                                          */
/* Input:      szTagText    - The entire tag to process.                     */
/*             usAttrNum    - Attribute number to retrieve.                  */
/*             ulSize       - Size of output attribute strings.              */
/* Output:     szAttr       - The attribute keyword to find.                 */
/*             szAttrValue  - The attribute's value to return.               */
/*             ptrAttrStart - Pointer to start of attribute value.           */
/*                                                                          */
/****************************************************************************/

BOOL fnGetNextAttributeValue( WCHAR  *szTagText, USHORT usAttrNum, 
                              WCHAR  *szAttr, WCHAR  *szAttrValue,
                              WCHAR  **ptrValueStart, ULONG ulSize )
{
   WCHAR     szTempAttr[256] ;  
   WCHAR     AttrQuote ;
   WCHAR     *ptr, *ptr2 ;
   USHORT    i, j ;
   BOOL      bReturn = 0 ;

   szAttr[0] = NULL ;
   szAttrValue[0] = NULL ;
   for( ptr=szTagText ; *ptr && !iswspace(*ptr) ; ++ptr ) ;

   for( i=1 ; i<=usAttrNum ; ++i ) {
      for( ; *ptr && iswspace(*ptr) ; ++ptr ) ;
      for( j=0 ; 
          *ptr && !iswspace(*ptr) && *ptr!=L'=' ; 
          szTempAttr[j++]=*(ptr++) ) {
         if ( szTempAttr[j-1] == L':' ) 
            j = 0 ;
      }
      szTempAttr[j] = NULL ;
      for( ; *ptr && iswspace(*ptr) ; ++ptr ) ;
      if ( *ptr == L'=' ) {
         for( ++ptr, ptr2=0 ; *ptr && iswspace(*ptr) ; ++ptr ) ;
         if ( ( *ptr == L'\'' ) ||
              ( *ptr == L'\"' ) ) {
            AttrQuote = *ptr ;
            ptr2 = ptr + 1;
            *ptrValueStart = ptr2 ;
            for( ; *ptr2 && (*ptr2!=AttrQuote) ; ++ptr2 ) ;
         }
         if ( ptr2 && *ptr2 ) {
            if ( i == usAttrNum ) {
               wcsncpy( szAttr, szTempAttr, ulSize ) ;
               szAttr[ulSize-1] = NULL ;
               wcsncpy( szAttrValue, (ptr+1), ulSize ) ;
               szAttrValue[ulSize-1] = NULL ;
               if ( (ULONG)(ptr2 - ptr) < ulSize ) 
                  szAttrValue[ ptr2-ptr-1 ] = NULL ;
               bReturn = TRUE ;
            } else {
               ptr = ptr2 + 1 ;
            }
         }
      }
   }
   return( bReturn ) ;
}


/****************************************************************************/
/*                                                                          */
/* fnGetCompleteString                                                      */
/*                                                                          */
/* Get the complete string ending with a specified set of characters.       */
/*                                                                          */
/* Input:      szText      - The input text to be parsed.                   */
/*             szEndChars  - The set of ending characters for the string.   */
/*             lValueSize  - Maximum size of output value.                  */
/* Output:     szValue     - The string's text to return.                   */
/*                                                                          */
/****************************************************************************/

BOOL fnGetCompleteString( WCHAR  *szText, WCHAR  *szEndChars, WCHAR  *szValue,
                          LONG   lValueSize ) 
{
   WCHAR           szTemp[MAX_XML_RCD_LENGTH] ;
   WCHAR           *ptrChar, *ptrChar2 ;
   long            save_pos ;
   long            lSize ;
// short           i ;
   BOOL            bFound = FALSE ;

   wcsncpy( szValue, szText, lValueSize ) ;
   szValue[lValueSize-1] = NULL ;

   for( ptrChar=szValue ; *ptrChar ; ++ptrChar ) {
      if ( wcschr( szEndChars, *ptrChar ) ) {
         *ptrChar = NULL ;
         bFound = TRUE ;
         break ;
      }
   }
   if ( ! bFound ) {

      /******************************************************************/
      /*  Add remainder of previously read string.                      */
      /******************************************************************/
      lSize = lValueSize - wcslen(szValue) ;
      if ( ( lSize > 0 ) &&
           ( szPrevXMLInputText[0] != NULL  ) ) {
         wcsncpy( ptrChar, szPrevXMLInputText, lSize ) ;
         szValue[lValueSize-1] = NULL ;
         lSize = lValueSize - wcslen(szValue) ;
      }
      if ( lSize > sizeof(szTemp)/sizeof(WCHAR) ) 
         lSize = ( sizeof(szTemp)/sizeof(WCHAR) ) - 1 ;

      /******************************************************************/
      /*  Read ahead to find end of this string                         */
      /******************************************************************/
      if ( lSize > 0 ) {
         if ( bReadSource )                        /* Save file position   */
            save_pos = (*InputFile).ftellt() ;
         else
            save_pos = (*InputFile2).ftellt() ;
         while( ( ( bReadSource ) &&              /* Read until find end char 2-6-17 */
                  ( (*InputFile).get((WCHAR*)szTemp, lSize, bUTF16, TRUE) != NULL ) ) ||
                ( ( ! bReadSource ) &&
                  ( (*InputFile2).get((WCHAR*)szTemp, lSize, bUTF16, TRUE) != NULL ) ) ) {
            if ( (LONG)(wcslen(szValue) + wcslen(szTemp)) < lValueSize ) {
               wcscat( szValue, szTemp ) ;
               for( ptrChar2=szTemp ; *ptrChar2 ; ++ptrChar2 ) {
                  if ( wcschr( szEndChars, *ptrChar2 ) ) 
                     break ;
               }
               if ( ( *ptrChar2 ) && 
                    ( wcschr( szEndChars, *ptrChar2 ) ) ) 
                  break ;
            } else {
               break ;
            }
         }
         if ( bReadSource )                        /* Save file position   */
            (*InputFile).fseekt( save_pos, SEEK_SET ) ;  /* Reset for next read */
         else
            (*InputFile2).fseekt( save_pos, SEEK_SET ) ;  /* Reset for next read */
      }

      for( ; *ptrChar ; ++ptrChar ) {
         if ( wcschr( szEndChars, *ptrChar ) ) {
            *ptrChar = NULL ;
            bFound = TRUE ;
            break ;
         }
      }
   }

   if ( ! bFound ) 
      szValue[0] = NULL ;

   return( bFound ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnGetXMLRcd                                                              */
/*                                                                          */
/* Get the next XML record from the file, handling too-long records.        */
/*                                                                          */
/* Input:      None                                                         */
/* Output:     szInput   - Input line being processed.                      */
/* Return:     0         - End-of-file.                                     */
/*             1         - Record returned.                                 */
/*                                                                          */
/****************************************************************************/

USHORT fnGetXMLRcd( WCHAR  *szInput, ULONG * ulFilePos )
{
   USHORT    i, j, k ;
   USHORT    usReturn = 1 ;
   WCHAR     *ptr ;

   i;

   wcscpy( szInput, szPrevXMLInputText ) ;
   j = (USHORT)wcslen( szInput ) ;
   szPrevXMLInputText[0] = NULL ;
   if ( bReadSource )
      *ulFilePos = (*InputFile).ftellt() ;
   else
      *ulFilePos = (*InputFile2).ftellt() ;
   if ( ( ( bReadSource ) &&
          ( (*InputFile).get((WCHAR*)(szInput+j), MAX_XML_RCD_LENGTH, bUTF16, TRUE) != NULL ) ) ||
        ( ( ! bReadSource ) &&
          ( (*InputFile2).get((WCHAR*)(szInput+j), MAX_XML_RCD_LENGTH, bUTF16, TRUE) != NULL ) ) ) {
      *ulFilePos -= j * sizeof(WCHAR) ; 
      k = (USHORT)wcslen( &szInput[j] ) ;
      if ( k == MAX_XML_RCD_LENGTH - 1 ) {
//       for( ptr=&szInput[j+k-1] ; ptr>szInput && !iswspace(*ptr) ; --ptr ) ;
//       if ( ptr == szInput ) {          /* No blanks in text. Look for "><"   5-6-15 */
//          for( ptr=&szInput[j+k-1] ; ptr>szInput && ((*ptr!=L'>') || (*(ptr+1)!=L'<') ) ; --ptr ) ;
//       }
         for( ptr=&szInput[j+k-1] ; 
              ptr>szInput && !iswspace(*ptr) && ((*ptr!=L'>') || (*(ptr+1)!=L'<') ) ;  /* 6-16-16 */
              --ptr ) ;
         if ( ( ptr > szInput ) &&
              ( wcslen(ptr+1) < sizeof(szPrevXMLInputText)-1 ) ) {
            ++ptr ;
            wcscpy( szPrevXMLInputText, ptr ) ;
            *(ptr) = NULL ;
         }
      }

   } else {
      if ( j == 0 ) 
         usReturn = 0 ;
   }

   return( usReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnWriteBlock                                                             */
/*                                                                          */
/* Write a block of text from the input file.                               */
/*                                                                          */
/* Input:      Type      - Read from source or target file.                 */
/*             StartPos  - Starting position to read from.                  */
/*             EndPos    - Ending position to read from.                    */
/*             OutFile   - File to write to.                                */
/*             PreserveBlanks - TRUE=add xml:space="preserve" to <w:t>.     */
/* Output:     None.                                                        */
/* Return:     0         - Block written                                    */
/*             1         - Failure.                                         */
/*                                                                          */
/****************************************************************************/

USHORT fnWriteBlock( USHORT Type, ULONG StartPos, ULONG EndPos,
                     wofstream *OutFile, BOOL bPreserveBlanks )
{
   WCHAR     szIn[MAX_XML_RCD_LENGTH2*2] ;
   ULONG     ulBlockLen = 0 ;
   ULONG     ulWriteLen = 0 ;
   ULONG     ulFilePos = 0 ;
   ULONG     i ;
   USHORT    usReturn = 0 ;
   BOOL      bReadEOF = FALSE ;

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

   if ( Type == WRITE_SOURCE ) {
      bReadSource = TRUE ;
      (*InputFile).fseekt(StartPos, std::ios::beg) ;
   } else {
      bReadSource = FALSE ;
      (*InputFile2).fseekt(StartPos, std::ios::beg) ;
   }

   ulWriteLen = ulBlockLen ;
   szPrevXMLInputText[0] = NULL ;

//fDebug=fopen("C:\\daw.daw","ab");
//if ( Type == WRITE_SOURCE ) 
//   fwprintf(fDebug,L"    S-WRITE    %lx-%lx     Length=%lx   PB=%d\n",StartPos,EndPos,ulWriteLen,bPreserveBlanks);
//else
//   fwprintf(fDebug,L"    T-WRITE    %lx-%lx     Length=%lx   PB=%d\n",StartPos,EndPos,ulWriteLen,bPreserveBlanks);
//fclose(fDebug);
  
   while( ( usReturn == 0  ) &&
          ( ( ulWriteLen > 0 ) ||
            ( bReadEOF       ) ) ) {
      if ( fnGetXMLRcd( szIn, &ulFilePos ) ) {
         i = wcslen( szIn ) ;
//fDebug=fopen("C:\\daw.daw","ab");
//fwprintf(fDebug,L"1.    L=%lx      ##%s##\n",i, szIn);
//fclose(fDebug);
        if ( ( i > ulWriteLen ) &&
             ( ! bReadEOF     ) ) {
           i = ulWriteLen ;
           szIn[i] = NULL ;
           if ( ( bPreserveBlanks ) &&              /* 7-18-13 */
                ( i > 5 ) ) {
              if ( ! wcsncmp( &szIn[i-5], L"<w:t>", 5 ) ) {
                 wcscpy( &szIn[i-1], L" xml:space=\"preserve\">" ) ;
              } else
              if ( ! wcsncmp( &szIn[i-3], L"<t>", 3 ) ) {   /* 2-17-17 */
                 wcscpy( &szIn[i-1], L" xml:space=\"preserve\">" ) ;
              }
           }
        }
//fDebug=fopen("C:\\daw.daw","ab");
//fwprintf(fDebug,L"2.    L=%lx      ##%s##\n",i, szIn);
//fclose(fDebug);
         *OutFile << szIn ;   
         ulWriteLen -= i ; 
      } else {
         usReturn = 1 ;
      }
   }

//////*OutFile << L"XX\n" ;    //DEBUG      DEBUG      DEBUG   #######################################################################

   return( usReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnReadBlock                                                              */
/*                                                                          */
/* Read a block of text from the input file.                                */
/*                                                                          */
/* Input:      Type      - Read from source or target file.                 */
/*             StartPos  - Starting position to read from.                  */
/*             EndPos    - Ending position to read from.                    */
/* OutPut:     Value     - Retrieved text.                                  */
/* Return:     0         - Block read                                       */
/*             1         - Failure.                                         */
/*                                                                          */
/****************************************************************************/

USHORT fnReadBlock( USHORT Type, ULONG StartPos, ULONG EndPos,
                    WCHAR *Value, ULONG ulValueSize )
{
   WCHAR     szIn[MAX_XML_RCD_LENGTH2*2] ;
   ULONG     ulBlockLen = 0 ;
   ULONG     ulWriteLen = 0 ;
   ULONG     ulFilePos = 0 ;
   ULONG     i, j ;
   USHORT    usReturn = 0 ;
   BOOL      bReadEOF = FALSE ;

   Value[0] = NULL ;

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

   if ( Type == WRITE_SOURCE ) {
      bReadSource = TRUE ;
      (*InputFile).fseekt(StartPos, std::ios::beg) ;
   } else {
      bReadSource = FALSE ;
      (*InputFile2).fseekt(StartPos, std::ios::beg) ;
   }

   ulWriteLen = ulBlockLen ;
   szPrevXMLInputText[0] = NULL ;

   while( ( usReturn == 0  ) &&
          ( ( ulWriteLen > 0 ) ||
            ( bReadEOF       ) ) ) {
      if ( fnGetXMLRcd( szIn, &ulFilePos ) ) {
         i = wcslen( szIn ) ;
        if ( ( i > ulWriteLen ) &&
             ( ! bReadEOF     ) ) {
           i = ulWriteLen ;
           szIn[i] = NULL ;
        }
        if ( wcslen(Value)+i > ulValueSize/sizeof(WCHAR) ) {
           j = (ulValueSize-1)/sizeof(WCHAR) - wcslen(Value) ;
           szIn[j] = NULL ;
        }
        wcscat( Value, szIn ) ;
        ulWriteLen -= i ; 
      } else {
        usReturn = 1 ;
      }
   }

   return( usReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnAllocateParaNode                                                       */
/*                                                                          */
/* Allocate storage for a new paragraph node.                               */
/*                                                                          */
/* Input:      Tag       - Tag defined by this node.                        */
/*             bAllocate - TRUE=Allocate new node, FALSE=Reuse existing     */
/* In/Out:     ptrHead   - The first node in the linked list.               */
/*             ptrTail   - The last node in the linked list.                */
/* Return:     TRUE      - Node allocated.                                  */
/*             FALSE     - Allocation failure                               */
/*                                                                          */
/****************************************************************************/

BOOL fnAllocateParaNode( P_INFO** ptrHead, P_INFO** ptrTail, 
                         WCHAR *Tag, BOOL bAllocate  ) 
{
   P_INFO    *ptrNew ;
   R_INFO    *ptrRun ;
   BOOL      bReturn = FALSE ;

   if ( ( bAllocate  ) ||
        ( ! *ptrHead ) ) {                         /* Allocate new node      */
      ptrNew = (P_INFO*)calloc( sizeof(P_INFO), 1 ) ;
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
   } else {
      if ( (*ptrTail)->Tag ) 
         free( (*ptrTail)->Tag ) ;
      if ( (*ptrTail)->ptrRunList ) {
         ptrRun = (R_INFO*)((*ptrTail)->ptrRunList) ;
         fnFreeRunList( &ptrRun ) ;
      }
      ptrNew = (P_INFO*)((*ptrTail)->ptrPrev) ;
      memset( *ptrTail, 0, sizeof(P_INFO) );  /* Clear storage.*/
      ((P_INFO*)(*ptrTail))->ptrPrev = ptrNew ;
   }

   if ( Tag[0] != NULL ) {
      (*ptrTail)->Tag = (WCHAR*)malloc( (wcslen(Tag)+1) * sizeof(WCHAR) ) ;
      wcscpy( (*ptrTail)->Tag, Tag ) ;
   }

   return( bReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnAllocateRunNode                                                        */
/*                                                                          */
/* Allocate storage for a new run node.                                     */
/*                                                                          */
/* Input:      ptrPTail  - The last node in the paragraph linked list.      */
/*             Tag       - Tag defined by this node.                        */
/*             bAllocate - TRUE=Allocate new node, FALSE=Reuse existing     */
/* In/Out:     ptrHead   - The first node in the linked list.               */
/*             ptrTail   - The last node in the linked list.                */
/* Return:     TRUE      - Node allocated.                                  */
/*             FALSE     - Allocation failure                               */
/*                                                                          */
/****************************************************************************/

BOOL fnAllocateRunNode( R_INFO** ptrHead, R_INFO** ptrTail, P_INFO* ptrPTail, 
                        WCHAR *Tag, BOOL bAllocate  ) 
{
   R_INFO    *ptrNew ;
   T_INFO    *ptrText ;
   BOOL      bReturn = FALSE ;

   if ( ( bAllocate ) ||
        ( ! *ptrHead ) ) {                         /* Allocate new node      */
      ptrNew = (R_INFO*)calloc( sizeof(R_INFO), 1 ) ;
      if ( ptrNew ) {
         bReturn = TRUE ;
         if ( *ptrHead ) {
            (*ptrTail)->ptrNext = ptrNew ;
            ptrNew->ptrPrev = *ptrTail ;
         } else {
            *ptrHead = ptrNew ;
            ptrPTail->ptrRunList = ptrNew ;
         }
         *ptrTail = ptrNew ; 
      }
   } else {
      if ( (*ptrTail)->Tag ) 
         free( (*ptrTail)->Tag ) ;
      if ( (*ptrTail)->Properties ) 
         free( (*ptrTail)->Properties ) ;
      if ( (*ptrTail)->ptrTextList ) {
         ptrText = (T_INFO*)((*ptrTail)->ptrTextList) ;
         fnFreeTextList( &ptrText ) ;
      }
      ptrNew = (R_INFO*)((*ptrTail)->ptrPrev) ;
      memset( *ptrTail, 0, sizeof(R_INFO) );  /* Clear storage.*/
      ((R_INFO*)(*ptrTail))->ptrPrev = ptrNew ;
   }

   (*ptrTail)->Concat = CONCAT_TEXT_CHECK ;
   if ( Tag[0] != NULL ) {
      (*ptrTail)->Tag = (WCHAR*)malloc( (wcslen(Tag)+1) * sizeof(WCHAR) ) ;
      wcscpy( (*ptrTail)->Tag, Tag ) ;
   }

   return( bReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnAllocateTextNode                                                       */
/*                                                                          */
/* Allocate storage for a new text node.                                    */
/*                                                                          */
/* Input:      ptrRTail  - The last node in the run linked list.            */
/*             Tag       - Tag defined by this node.                        */
/*             bAllocate - TRUE=Allocate new node, FALSE=Reuse existing     */
/* In/Out:     ptrHead   - The first node in the linked list.               */
/*             ptrTail   - The last node in the linked list.                */
/* Return:     TRUE      - Node allocated.                                  */
/*             FALSE     - Allocation failure                               */
/*                                                                          */
/****************************************************************************/

BOOL fnAllocateTextNode( T_INFO** ptrHead, T_INFO** ptrTail, R_INFO* ptrRTail,
                         WCHAR *Tag, BOOL bAllocate  ) 
{
   T_INFO    *ptrNew ;
   BOOL      bReturn = FALSE ;

   if ( ( bAllocate ) ||
        ( ! *ptrHead ) ) {                         /* Allocate new node      */
      ptrNew = (T_INFO*)calloc( sizeof(T_INFO), 1 ) ;
      if ( ptrNew ) {
         bReturn = TRUE ;
         if ( *ptrHead ) {
            (*ptrTail)->ptrNext = ptrNew ;
            ptrNew->ptrPrev = *ptrTail ;
         } else {
            *ptrHead = ptrNew ;
            ptrRTail->ptrTextList = ptrNew ;
         }
         *ptrTail = ptrNew ; 
      }
   } else {
      if ( (*ptrTail)->Tag ) 
         free( (*ptrTail)->Tag ) ;

      ptrNew = (T_INFO*)((*ptrTail)->ptrPrev) ;
      memset( *ptrTail, 0, sizeof(T_INFO) );  /* Clear storage.*/
      ((T_INFO*)(*ptrTail))->ptrPrev = ptrNew ;
   }

   if ( Tag[0] != NULL ) {
      (*ptrTail)->Tag = (WCHAR*)malloc( (wcslen(Tag)+1) * sizeof(WCHAR) ) ;
      wcscpy( (*ptrTail)->Tag, Tag ) ;
   }

   return( bReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnSaveRunProperties                                                      */
/*                                                                          */
/* Save the property information for this run of text.  Certain runs have   */
/* been split to identify text which was changed during the same editing    */
/* session.  These runs should be merged back together for translation.     */
/*                                                                          */
/* Input:      ptrCur    - Current node to add text to.                     */
/*             TagText   - Text of tag to add.                              */
/* Return:     TRUE      - Text was added.                                  */
/*             FALSE     - Allocation error.                                */
/*                                                                          */
/****************************************************************************/

BOOL fnSaveRunProperties( R_INFO* ptrCur, WCHAR *TagText  ) 
{
   WCHAR     szTemp[MAX_XML_RCD_LENGTH] ;
   WCHAR     *ptrSave = NULL ;
   WCHAR     *ptrChar, *ptrChar2, *ptrChar3 ;
   ULONG     ulSize ;
   BOOL      bAdd = TRUE ;
   BOOL      bAppend = FALSE ;
   BOOL      bReturn = FALSE ;

   if ( ptrCur ) {
      ulSize = wcslen( TagText ) ;
      if ( ptrCur->Properties != NULL ) {
         bAppend = TRUE ;
         ulSize += wcslen( ptrCur->Properties ) ;
         ptrSave = ptrCur->Properties ; 
      }

      /***********************************************************************/
      /*  Word 2003.                                                         */
      /*  Remove the attributes which indicate that this run was changed     */
      /*  for a particular editing session.                                  */
      /*     wsp:rsidP  wsp:rsidR  wsp:rsidRPr  wsp:rsidDel  wsp:rsidDefault */
      /***********************************************************************/
//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"SaveRunProp {%s}\n",TagText);
//fclose(fDebug);
      wcscpy( szTemp, TagText ) ;
      for( ptrChar=wcsstr(szTemp,L"wsp:rsid") ;
           ptrChar ; ptrChar=wcsstr(ptrChar+1,L"wsp:rsid") ) {
         if ( ( ( wcschr( L"PR", (WCHAR)*(ptrChar+8) ) ) &&
                ( wcschr( L" \n\r\t=", (WCHAR)*(ptrChar+9) ) ) ) ||
              ( ( ( ! wcsncmp( ptrChar+8, L"RPr", 3 ) ) ||
                  ( ! wcsncmp( ptrChar+8, L"Del", 3 ) ) ) &&
                ( wcschr( L" \n\r\t=", (WCHAR)*(ptrChar+11) ) ) ) ||
              ( ( ! wcsncmp( ptrChar+8, L"Default", 7 ) ) &&
                ( wcschr( L" \n\r\t=", (WCHAR)*(ptrChar+15) ) ) ) ) {
            ptrChar2 = wcschr( ptrChar, L'=' ) ;
            if ( ptrChar2 ) {                   
               for( ++ptrChar2 ; *ptrChar2 && iswspace(*ptrChar2) ; ++ptrChar2 ) ;
               if ( ( *ptrChar2 == L'\"' ) ||
                    ( *ptrChar2 == L'\'' ) ) {
                  ptrChar3 = wcschr( ptrChar2+1, *ptrChar2 ) ;
                  if ( ptrChar3 ) {
                     for( --ptrChar ; ptrChar>szTemp && iswspace(*ptrChar) ; --ptrChar ) ;
//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"RUN BEFORE: {%s}\n",szTemp);
                     wmemmove( ptrChar+1, ptrChar3+1, wcslen(ptrChar3+1)+1 ) ;
//fwprintf(fDebug,L"RUN AFTER:  {%s}\n",szTemp);
//fclose(fDebug);
                  }
               }
            }
         }
      }

      /***********************************************************************/
      /*  Word 2007.                                                         */
      /*  Remove the attributes which indicate that this run was changed     */
      /*  for a particular editing session.                                  */
      /*     w:rsidP  w:rsidR  w:rsidRPr  w:rsidDel  w:rsidRDefault          */
      /***********************************************************************/
//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"SaveRunProp {%s}\n",TagText);
//fclose(fDebug);
      for( ptrChar=wcsstr(szTemp,L"w:rsid") ;
           ptrChar ; ptrChar=wcsstr(ptrChar+1,L"w:rsid") ) {
         if ( ( ( wcschr( L"PR", (WCHAR)*(ptrChar+6) ) ) &&
                ( wcschr( L" \n\r\t=", (WCHAR)*(ptrChar+7) ) ) ) ||
              ( ( ( ! wcsncmp( ptrChar+6, L"RPr", 3 ) ) ||
                  ( ! wcsncmp( ptrChar+6, L"Del", 3 ) ) ) &&
                ( wcschr( L" \n\r\t=", (WCHAR)*(ptrChar+9) ) ) ) ||
              ( ( ! wcsncmp( ptrChar+6, L"RDefault", 8 ) ) &&
                ( wcschr( L" \n\r\t=", (WCHAR)*(ptrChar+14) ) ) ) ) {
            ptrChar2 = wcschr( ptrChar, L'=' ) ;
            if ( ptrChar2 ) {                   
               for( ++ptrChar2 ; *ptrChar2 && iswspace(*ptrChar2) ; ++ptrChar2 ) ;
               if ( ( *ptrChar2 == L'\"' ) ||
                    ( *ptrChar2 == L'\'' ) ) {
                  ptrChar3 = wcschr( ptrChar2+1, *ptrChar2 ) ;
                  if ( ptrChar3 ) {
                     for( --ptrChar ; ptrChar>szTemp && iswspace(*ptrChar) ; --ptrChar ) ;
//fDebug=fopen(szDebugFile,"ab");    
//fwprintf(fDebug,L"RUN BEFORE: {%s}\n",szTemp);
                     wmemmove( ptrChar+1, ptrChar3+1, wcslen(ptrChar3+1)+1 ) ;
//fwprintf(fDebug,L"RUN AFTER:  {%s}\n",szTemp);
//fclose(fDebug);
                  }
               }
            }
         }
      }

      /***********************************************************************/
      /*  PowerPoint.                                                        */
      /*  Remove the "lang" and "altLang" attribute values to improve        */
      /*  concatenating text for DrawingML.                         10-25-13 */
      /***********************************************************************/
      ptrChar = wcsstr( szTemp, L"a:rPr" ) ;
      if ( ptrChar ) {
         ptrChar2 = wcsstr( ptrChar, L" lang=\"" ) ;
         ptrChar3 = wcsstr( ptrChar, L" altLang=\"" ) ;
         if ( ( ptrChar2 ) &&
              ( *(ptrChar2+12) == L'\"' ) &&
              ( ptrChar3 ) &&
              ( *(ptrChar3+15) == L'\"' ) ) {
            wcsncpy( ptrChar2+7,  L"xx-xx", 5 ) ;
            wcsncpy( ptrChar3+10, L"xx-xx", 5 ) ;
         }
         ptrChar2 = wcsstr( ptrChar, L" smtClean=\"0\"" ) ;       /* 1-14-14 */
         if ( ptrChar2 ) {
            wmemmove( ptrChar2, ptrChar2+13, wcslen(ptrChar2+13)+1 ) ;
         }
         ptrChar2 = wcsstr( ptrChar, L" dirty=\"0\"" ) ;          /* 1-14-14 */
         if ( ptrChar2 ) {
            wmemmove( ptrChar2, ptrChar2+10, wcslen(ptrChar2+10)+1 ) ;
         }
      }

      /***********************************************************************/
      /*  PowerPoint.                                                        */
      /*  If run already has <a:latin>, then no need to add <a:ae> element   */
      /*  as split text parts do not have this.                     11-13-13 */
      /***********************************************************************/
      if ( ptrCur->Properties != NULL ) {
         ptrChar = wcsstr( ptrCur->Properties, L"<a:latin " ) ;
         if ( ( ptrChar ) && 
              ( ! wcsncmp( szTemp, L"<a:ea ", 6 ) ) ) {
            bAdd = FALSE ;
         }
      }

      /***********************************************************************/
      /*  Ignore differences in proof reading and grammer checking tags.     */
      /***********************************************************************/
      if ( ( ! wcsncmp( szTemp, L"<w:noProof", 10 ) ) ||          /* 2-15-13 */
           ( ! wcsncmp( szTemp, L"<w:lang ",    8 ) ) ) {
         bAdd = FALSE ;
      }

      /***********************************************************************/
      /*  Ignore font hints for "cs", "eastAsia", "farEast".         10-6-14 */
      /***********************************************************************/
      if ( ( ! wcsncmp( szTemp, L"<w:rFonts w:hint=\"", 18 ) ) &&     
           ( ( ! wcsncmp( szTemp+18, L"eastAsia\"/>", 11 ) ) ||         
             ( ! wcsncmp( szTemp+18, L"eastasia\"/>", 11 ) ) ||         
             ( ! wcsncmp( szTemp+18, L"farEast\"/>",  10 ) ) ||   /* 3-27-15 */
             ( ! wcsncmp( szTemp+18, L"fareast\"/>",  10 ) ) ||   /* 3-27-15 */
             ( ! wcsncmp( szTemp+18, L"cs\"/>",        5 ) ) ) ) {
         bAdd = FALSE ;
      }

      /***********************************************************************/
      /*  Add this tag to the run properties.                                */
      /***********************************************************************/
      if ( bAdd ) {
         ptrCur->Properties = (WCHAR*)malloc( (ulSize+1) * sizeof(WCHAR) ) ;
         if ( ptrCur->Properties ) {
            bReturn = TRUE ;
            if ( bAppend ) {
               wcscpy( ptrCur->Properties, ptrSave ) ;
               free( ptrSave ) ;
            } else {
               ptrCur->Properties[0] = NULL ;
            }
            wcscat( ptrCur->Properties, szTemp ) ;
         }
      }
   }
   return( bReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnIsEmptyTag                                                             */
/*                                                                          */
/* Determine if this is an empty tag or not.                                */
/*                                                                          */
/* Input:      TagText   - Text of tag.                                     */
/* Return:     TRUE      - Tag is an empty tag (<xx />)                     */
/*             FALSE     - Tag is not an empty tag.                         */
/*                                                                          */
/****************************************************************************/

BOOL fnIsEmptyTag( WCHAR *TagText  ) 
{
   USHORT    usTagLen ;
   USHORT    i ;
   BOOL      bEmpty = FALSE ;

   usTagLen = (USHORT)wcslen( TagText ) ;
   if ( ( TagText[0] == L'<' ) &&
        ( TagText[usTagLen-1] == L'>' ) ) {
      for( i=usTagLen-2 ; i>0 && iswspace(TagText[i]) ; --i ) ;
      if ( TagText[i] == L'/' ) 
         bEmpty = TRUE ;
   }

   return( bEmpty ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnCheckCommonProp                                                        */
/*                                                                          */
/* Determine if run property contains attributes to define bold, italics,   */
/* or underline.                                                            */
/*                                                                          */
/* Input:      Properties  - Property tags.                                 */
/* Return:     TRUE        - Property contains special attributes.          */
/*             FALSE       - Property has no special attributes.            */
/*                                                                          */
/****************************************************************************/

BOOL fnCheckCommonProp( WCHAR *Properties  ) 
{
   BOOL      bReturn = FALSE ;

   if ( Properties ) {
      if ( ( wcsstr( Properties, L" b=\"" ) ) ||
           ( wcsstr( Properties, L" i=\"" ) ) ||
           ( wcsstr( Properties, L" u=\"" ) ) ||
           ( wcsstr( Properties, L"<b " ) ) ||
           ( wcsstr( Properties, L"<i " ) ) ||
           ( wcsstr( Properties, L"<u " ) ) ||
           ( wcsstr( Properties, L"<w:b " ) ) ||
           ( wcsstr( Properties, L"<w:i " ) ) ||
           ( wcsstr( Properties, L"<w:u " ) ) ||
           ( wcsstr( Properties, L"<w:b/" ) ) ||
           ( wcsstr( Properties, L"<w:i/" ) ) ||
           ( wcsstr( Properties, L"<w:u/" ) ) ||
           ( wcsstr( Properties, L"<w:bCs " ) ) ||                /* 5-9-14 */
           ( wcsstr( Properties, L"<w:iCs " ) ) ||
           ( wcsstr( Properties, L"<w:bCs/" ) ) ||
           ( wcsstr( Properties, L"<w:iCs/" ) ) ) {
         bReturn = TRUE ; 
      }
   }

   return( bReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* fnFreeParaList                                                           */
/*                                                                          */
/* Free all of the storage used in the paragraph linked list.               */
/*                                                                          */
/****************************************************************************/

VOID fnFreeParaList( P_INFO** ptrList ) 
{
   P_INFO    *ptrParaHead ;
   P_INFO    *ptrParaCur ;

   R_INFO    *ptrRunCur ;


    /***********************************************************************/
    /*  Free linked list space.                                            */
    /***********************************************************************/
    ptrParaHead = *ptrList ; 

    if ( bDebugBody || bDebugFree ) {
       fwprintf(fDebug,L"\nFreeing....  %lx\n",ptrParaHead);
       fflush(fDebug);
    }
  
    while ( ptrParaHead ) {
  
       if ( bDebugBody || bDebugFree ) {
          fwprintf(fDebug,L"PARA_F...#%ld   NT=%ld   %lx-%lx   %lx-%lx       <",
              ptrParaHead->SeqNum,ptrParaHead->NodeType,ptrParaHead->StartPos,ptrParaHead->BlockStartPos,ptrParaHead->BlockEndPos,ptrParaHead->EndPos);
          fflush(fDebug);
          if ( ptrParaHead->Tag ) 
             fwprintf(fDebug,L"%s",ptrParaHead->Tag);
          fwprintf(fDebug,L">   T#=%ld  N#=%ld   RUN=%lx",
              ptrParaHead->NumTextTags,ptrParaHead->NumNeutralTags,ptrParaHead->ptrRunList);
          fflush(fDebug);
          if ( ptrParaHead->PreserveBlanks ) 
             fwprintf(fDebug,L"   PB=%d",ptrParaHead->PreserveBlanks);
          if ( ptrParaHead->CommonProperty ) 
             fwprintf(fDebug,L"   CP=%d",ptrParaHead->CommonProperty);
          if ( ptrParaHead->StartTableRow ) 
             fwprintf(fDebug,L"   StartRow");
          fputws(L"\n",fDebug);
          fflush(fDebug);
       }

  
       if ( ptrParaHead->Tag ) 
          free( ptrParaHead->Tag ) ;
  
       ptrRunCur = (R_INFO*)ptrParaHead->ptrRunList ; 
       fnFreeRunList( &ptrRunCur ) ;
  
       ptrParaCur = (P_INFO*)ptrParaHead->ptrNext ;
       free( ptrParaHead ) ;
       ptrParaHead = ptrParaCur ;
    }
  
    if ( bDebugBody || bDebugFree ) {
       fwprintf(fDebug,L"End freeing....\n");
       fflush(fDebug);
    }

    *ptrList = 0; 
}




/****************************************************************************/
/*                                                                          */
/* fnFreeRunList                                                            */
/*                                                                          */
/* Free all of the storage used in the run linked list.                     */
/*                                                                          */
/****************************************************************************/

VOID fnFreeRunList( R_INFO** ptrList ) 
{
   R_INFO    *ptrRunHead ;
   R_INFO    *ptrRunCur ;

   T_INFO    *ptrTextCur ;


    /***********************************************************************/
    /*  Free linked list space.                                            */
    /***********************************************************************/
    ptrRunHead = *ptrList ; 

    while ( ptrRunHead ) {

       if ( bDebugBody || bDebugFree ) {
          fwprintf(fDebug,L"   RUN_F...#%ld (%ld)   NT=%ld   %lx-%lx   %lx-%lx       <",
              ptrRunHead->SeqNum,ptrRunHead->BeginSeqNum,ptrRunHead->NodeType,ptrRunHead->StartPos,ptrRunHead->BlockStartPos,ptrRunHead->BlockEndPos,ptrRunHead->EndPos);
          if ( ptrRunHead->Tag ) 
             fwprintf(fDebug,L"%s",ptrRunHead->Tag);
          fwprintf(fDebug,L">     T#=%ld  N#=%ld   TA=%ld  TT=%ld  TID=%ld  TEXT=%lx",
              ptrRunHead->NumTextTags,ptrRunHead->NumNeutralTags,ptrRunHead->TagAction,ptrRunHead->TagType,ptrRunHead->TagId,ptrRunHead->ptrTextList);
          if ( ptrRunHead->Concat )
             fwprintf(fDebug,L"  CONCAT=%d ",ptrRunHead->Concat);
          if ( ptrRunHead->PreserveBlanks ) 
             fwprintf(fDebug,L"   PB=%d",ptrRunHead->PreserveBlanks);
          if ( ptrRunHead->CommonProperty ) 
             fwprintf(fDebug,L"   CP=%d",ptrRunHead->CommonProperty);
          if ( ptrRunHead->Properties ) 
             fwprintf(fDebug,L"   Prop={%s}",ptrRunHead->Properties);
          if ( ptrRunHead->BreakBeforeText ) 
             fwprintf(fDebug,L"   BR=%d",ptrRunHead->BreakBeforeText);
          if ( ptrRunHead->TextIns ) 
             fwprintf(fDebug,L"   INSERT");
          if ( ptrRunHead->TextDel ) 
             fwprintf(fDebug,L"   DELETE");
          fputws(L"\n",fDebug);
          fflush(fDebug);
       }

       if ( ptrRunHead->Tag ) 
          free( ptrRunHead->Tag ) ;
       if ( ptrRunHead->Properties ) 
          free( ptrRunHead->Properties ) ;

       ptrTextCur = (T_INFO*)ptrRunHead->ptrTextList ; 
       fnFreeTextList( &ptrTextCur ) ;

       ptrRunCur = (R_INFO*)ptrRunHead->ptrNext ;
       free( ptrRunHead ) ;
       ptrRunHead = ptrRunCur ;
    }

    *ptrList = 0; 
}




/****************************************************************************/
/*                                                                          */
/* fnFreeTextList                                                           */
/*                                                                          */
/* Free all of the storage used in the text linked list.                    */
/*                                                                          */
/****************************************************************************/

VOID fnFreeTextList( T_INFO** ptrList ) 
{

   T_INFO    *ptrTextHead ;
   T_INFO    *ptrTextCur ;


    /***********************************************************************/
    /*  Free linked list space.                                            */
    /***********************************************************************/
    ptrTextHead = *ptrList ; 

    while ( ptrTextHead ) {

       if ( bDebugBody || bDebugFree ) {
          fwprintf(fDebug,L"      TEXT_F...#%ld (%ld)  %lx  NT=%ld   %lx-%lx   %lx-%lx       <",
              ptrTextHead->SeqNum,ptrTextHead->BeginSeqNum,ptrTextHead,ptrTextHead->NodeType,ptrTextHead->StartPos,ptrTextHead->BlockStartPos,ptrTextHead->BlockEndPos,ptrTextHead->EndPos);
          if ( ptrTextHead->Tag ) 
             fwprintf(fDebug,L"%s",ptrTextHead->Tag);
          fwprintf(fDebug,L">   TA=%ld  TT=%ld  TID=%ld\n",
              ptrTextHead->TagAction,ptrTextHead->TagType,ptrTextHead->TagId);
          fflush(fDebug);
       }

       if ( ptrTextHead->Tag ) 
          free( ptrTextHead->Tag ) ;

       ptrTextCur = (T_INFO*)ptrTextHead->ptrNext ;
       free( ptrTextHead ) ;
       ptrTextHead = ptrTextCur ;
    }

    *ptrList = 0; 
}

/*****************************************************************************/
/*                                                                           */
/* ShowIBMMessage                                                            */
/*                                                                           */
/* Inputs:   szTitle      - Title text.                                      */
/*           szMsgText    - Message text.                                    */
/*           bOKCancel    - TRUE,  show OK and CANCEL buttons.               */
/*                          FALSE, show OK button only.                      */
/*           bWarning     - TRUE,  show warning message.                     */
/*                          FALSE, show error message.                       */
/* Returns:  TRUE         - OK button pressed.                               */
/*           FALSE        - CANCEL button pressed.                           */
/*                                                                           */
/*****************************************************************************/

BOOL  ShowIBMMessage( char *szTitle, char *szMsg,
                      BOOL bOKCancel, BOOL bWarning ) {

   char    szTitleText[256] ;
   char    szMsgText[4096] ;
   int     rc ;
   BOOL    bReturn = TRUE ;


   strcpy( szTitleText, szTitle ) ;
   strcpy( szMsgText, szMsg ) ;

   if ( bOKCancel ) {
      if ( bWarning )
         rc = MessageBoxA( HWND_DESKTOP, szMsgText, szTitleText,
                           MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING | MB_SYSTEMMODAL ) ;
      else
         rc = MessageBoxA( HWND_DESKTOP, szMsgText, szTitleText,
                           MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONSTOP | MB_SYSTEMMODAL ) ;
      if ( rc != IDOK )
         bReturn = FALSE ;

   } else {
      if ( bWarning )
         rc = MessageBoxA( HWND_DESKTOP, szMsgText, szTitleText,
                           MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL ) ;
      else
         rc = MessageBoxA( HWND_DESKTOP, szMsgText, szTitleText,
                           MB_OK | MB_DEFBUTTON1 | MB_ICONSTOP | MB_SYSTEMMODAL ) ;
   }
   return( bReturn ) ;
}  
