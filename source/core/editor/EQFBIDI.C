//+----------------------------------------------------------------------------+
//|EQFBIDI.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:            IBM Israel                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       This file contains all functions concerned with the      |
//|                   BIDI functionality under Windows                         |
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
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.3 $ ----------- 17 Mar 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
// 
//
// $Revision: 1.2 $ ----------- 26 Feb 2003
// --RJ: removed compiler defines not needed any more;
// -- do not remove warnings as code has been supplied by Israel--
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
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.3 $ ----------- 7 Jan 2000
// -- use correct size indication (size_t) for allocation
//    size_t is under VC6.0 bigger than char!!!!
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFBIDI.CV_   1.2   22 Nov 1999 18:09:02   BUILD  $
 *
 * $Log:   K:\DATA\EQFBIDI.CV_  $
 *
 *    Rev 1.2   22 Nov 1999 18:09:02   BUILD
 * -- add reordering option (necessary for mouse selection)
 *
 *    Rev 1.1   12 Jul 1999 10:03:52   BUILD
 * -- init buffer for reuse
 *
 *    Rev 1.0   21 Jun 1999 14:37:36   BUILD
 * Initial revision.
 *
*/
//+----------------------------------------------------------------------------+
/*
  Comments on this routine and comparisons with the x/Open standard:

    if inp_len = 0 does nothing

    if inp_len = -1, indicates null-terminated string

    if out_len < inp_len, only the first {out_len} bytes will be translated

    on exit out_len contains the length of the output string

    input and output code pages must be identical

  Return values: BDT_SUCCESS                -- successful completion
           BDT_BAD_OUTLEN             -- out_len = 0
           BDTERR_BAD_CCSID           -- a requested CCSID is not defined
           BDTERR_DIFFERENT_CODEPAGES -- input and output codepages were not identical
           BDTERR_NOMEM             -- required memory was unavailable
*/

#define INCL_EQF_TP               // public translation processor functions
#include "eqf.h"
#include "eqfbidi.h"

int BiDitransform
   ( unsigned char * inp_buff,
   size_t inp_len,
   unsigned short inp_ccsid,
   unsigned short inp_strtype,
   unsigned char * out_buff,
   size_t *out_len,
   unsigned short out_ccsid,
   unsigned short out_strtype,
   unsigned short cont_mode,
   PUINT    puOrder )
{
  int num_flag;
  size_t i, j, buflen=0;
  int       k;
  int      RC=0;
  static size_t   *OutToInp= NULL;
  static char     *store= NULL;
  static unsigned char *tempbuf= NULL;
  static size_t OutToInpSize=0;
  static size_t storeSize=0;
  static size_t tempbufSize=0;
  size_t BufferSize;
  CCSIDTable inp_table;
  CCSIDTable out_table;
  BidiAttrTable Values_inp;
  BidiAttrTable Values_out;
  int    sizeofMainTable=sizeof(MainTable)/sizeof(CCSIDTable);

  int strongChar;
  PARAMRec csRec;
  cont_mode;
  memset( &csRec, 0, sizeof(csRec));
  if ((out_len == NULL)
    || (*out_len == 0)) {
    *out_buff=0;
    return (BDTERR_BAD_OUTLEN);
  }

  if (inp_len == -1)
    inp_len=strlen((const char *)inp_buff);

  if (inp_len==0) {
    *out_len= 0;
    *out_buff=0;
    return(BDT_SUCCESS);
  }

  if (inp_len > *out_len) {
    inp_len = *out_len;
  }


  /* Determine code pages and string types */
  k=0;
  while(inp_ccsid != MainTable[k].ccsid && k<(sizeofMainTable))
    k++;
  if (k==sizeofMainTable) {
    *out_len= 0;
    *out_buff=0;
    return (BDTERR_BAD_CCSID);
  }
  else {
    inp_table = MainTable[k];
    if (inp_strtype == 0)
      inp_strtype = inp_table.string_type;
    Values_inp=stringtypeTable[inp_strtype];
  }


  k=0;
  while(out_ccsid != MainTable[k].ccsid && k<(sizeofMainTable))
    k++;
  if (k==sizeofMainTable) {
    *out_len= 0;
    *out_buff=0;
    return (BDTERR_BAD_CCSID);
  }
  else {
    out_table = MainTable[k];
    if (out_strtype == 0)
      out_strtype = out_table.string_type;
    Values_out=stringtypeTable[out_strtype];
  }
  if (inp_table.codepage != out_table.codepage) {
    *out_len= 0;
    *out_buff=0;
    return (BDTERR_DIFFERENT_CODEPAGES);
  }

  buflen = inp_len;

  /* Determine input orientation */
  strongChar = -1;

  if (Values_inp.orientation==OR_CONTEXT) { /* Contextual orientation -- determined by the */
                        /*  first strong character           */
    strongChar=StrongChar((char *)inp_buff, buflen, inp_table.codepage);
    switch (strongChar) {

      case 1: /* Means that the first strong char is LTR */
        Values_inp.orientation=OR_LTR;
        break;

      case 2: /* The first strong char is RTL */
        Values_inp.orientation=OR_RTL;
        break;

      default: /* There is no strong character, use the default in Values_inp.context */
        Values_inp.orientation = (Values_inp.context==CON_RTL) ? OR_RTL : OR_LTR;
    }
  }

  /* Determine output orientation */
  if (Values_out.orientation==OR_CONTEXT) { /* Contextual orientation -- determined by the */
                         /*  first strong character          */
    if(strongChar == -1)
      strongChar=StrongChar((char *)inp_buff, buflen, inp_table.codepage);

    switch (strongChar) {

      case 1: /* Means that the first strong char is LTR */
        Values_out.orientation=OR_LTR;
        break;

      case 2: /* The first strong char is RTL */
        Values_out.orientation=OR_RTL;
        break;

      default: /* There is no strong character, use the default in Values_inp.context */
        Values_out.orientation = (Values_out.context==CON_RTL) ? OR_RTL : OR_LTR;
    }
  }

  if (Values_inp.numerics!=Values_out.numerics)
    num_flag=Values_out.numerics;
  else
    num_flag=NUM_PASSTHRU;

  BufferSize=(inp_len+1)*4*sizeof(char);
  if (BufferSize > tempbufSize) {
    if (tempbuf) free (tempbuf);
    tempbuf=(unsigned char *)malloc(BufferSize);
    if(!tempbuf) return(BDTERR_NOMEM);
    tempbufSize=BufferSize;
  }

  BufferSize=inp_len*4*sizeof(size_t);
  if (BufferSize > storeSize) {
    if (store) free(store);
    store=(char *)calloc(inp_len*4,sizeof(size_t));
    if(!store) return(BDTERR_NOMEM);
    storeSize=BufferSize;
  }
  else
  {
    /* XJR 07/09/99: init string */
    memset( store, 0, storeSize );
  }

  BufferSize=inp_len*4*sizeof(size_t);
  if (BufferSize > OutToInpSize) {
    if (OutToInp) free(OutToInp);
    OutToInp=(size_t *)calloc(inp_len*4,sizeof(size_t));
    if(!OutToInp) return(BDTERR_NOMEM);
    OutToInpSize=BufferSize;
  }
  else
  {
    /* XJR 07/09/99: init string */
    memset( OutToInp, 0, OutToInpSize );
  }

  *out_len= 0;
  *out_buff=0;

/*-----------------------------------------------------------*/
  if  ((Values_inp.typeoftext==TT_IMPLICIT)||(Values_out.typeoftext==TT_IMPLICIT))
  {
    csRec.compc=FALSE;

    csRec.codepage = inp_table.codepage;

    csRec.orient_out = Values_out.orientation;
    csRec.orient_in  = Values_inp.orientation;

    csRec.Strongchar = strongChar - 1;

    if (Values_inp.shaping!=Values_out.shaping)
      csRec.txtShp_flag=Values_out.shaping;
    else
      csRec.txtShp_flag=FALSE;
    csRec.formShp = 0;
    csRec.boc_flag = 0;
    csRec.num_flag = num_flag;
    if (Values_inp.typeoftext!=Values_out.typeoftext)
      csRec.flip_flag=TRUE;
    else
      csRec.flip_flag=FALSE;
    csRec.symmetric=(Values_inp.swapping!=Values_out.swapping);
    csRec.size=buflen;
    csRec.buffer_in = (char *)inp_buff;
    csRec.buffer_out = (char *)out_buff;
    if (Values_inp.typeoftext==TT_VISUAL)
      csRec.texttype_in=TT_VISUAL;
    else
      csRec.texttype_in=TT_IMPLICIT;
    if (Values_out.typeoftext==TT_VISUAL)
      csRec.texttype_out=TT_VISUAL;
    else
      csRec.texttype_out=TT_IMPLICIT;

    csRec.A_level = (unsigned char *)store;
    csRec.TrgToSrcMap = (unsigned int *)OutToInp;

    RC=CB2VIS(&csRec, puOrder );

  }
  else

  {
    /* Visual to Visual:*/
    for (i=0; i< buflen;i++) {
      if(Values_out.orientation!= Values_inp.orientation)
        j=buflen-i-1;
      else
        j=i;
      if ((num_flag==NUM_ARABIC)&&((getchtype(inp_buff[j], inp_table.codepage) & CH_HDIGIT) == CH_HDIGIT))
        out_buff[i] = ArabicDigit(inp_buff[j], inp_table.codepage);
      else if (Values_inp.swapping!=Values_out.swapping)
        out_buff[i] = mirror(inp_buff[j], inp_table.codepage);
      else
        out_buff[i] = inp_buff[j];
    }
  }
/**********************************************************************/
  if (out_buff)
  {
    /*if(strcmp(uniData->InCharSet,"UCS-") != 0){*/ /*Simon*/
    if (inp_table.codepage<=CP_1046) {      /* The last Arabic codepage requiring shaping*/
/*#ifdef ERROR*/
   /* check if ASD required  */
      if ((Values_inp.shaping != Values_out.shaping)
        /*&& context->core.active_shape_editing */ /*Always TRUE for DB2*/
        && !((Values_inp.typeoftext==TT_IMPLICIT)||(Values_out.typeoftext==TT_IMPLICIT)))
      {
/* Haytham */

        /*BidiShape(out_buff,inp_len,arabicData->OneCell->out,
              arabicData->SpecialSh->out,1,
              Values_out.shaping);*/
        if (inp_table.codepage<CP_1046)
          MapTo1046(out_buff, tempbuf, (int)buflen, inp_table.codepage);
          else
            tempbuf=out_buff;

        if (Values_inp.shaping!=Values_out.shaping)
          csRec.txtShp_flag=Values_out.shaping;
        else
          csRec.txtShp_flag=FALSE;
        BidiShape(tempbuf, buflen, FALSE,
                  0,   /* OS_flag????, */
              Values_out.orientation, csRec.txtShp_flag);
        if (csRec.codepage<CP_1046)
          MapFrom1046(tempbuf, out_buff, (int)buflen, csRec.codepage);
      /* if ((arabicData->SpecialSh->out==TEXT_COMPOSED) &&
              (context->core.text_shaping->out==TEXT_SHAPED))
            BidiCompose(ShapingOrient,outpbuf,inp_len);
      */
      }
/*#endif */  /* ERROR */
    }
  }
/********************* End of String Conversion ***************************/
    *out_len=buflen ; /*Return size of output string (Simon)*/
  return(BDT_SUCCESS);      /* successfully performed */
}

int StrongChar(char *inputBuffer, size_t  length, CODEPAGE code_page)
{
  CHTYPE ch;
  size_t  i;

  for(i=0;i<length;i++){
    ch=getchtype(inputBuffer[i], code_page) & CH_BATYPES;
    if(ch == CH_L)
      return(1);
    else if(ch == CH_R)
      return(2);
  }
  return(0);
}

CHTYPE getchtype(unsigned char ch, CODEPAGE codepage)
{
  return chartype[codepage] [ch];
}

unsigned char mirror(unsigned char ch, CODEPAGE codepage)
{
  return symmpair[codepage] [ch];
}

unsigned char ArabicDigit(unsigned char ch, CODEPAGE codepage)
{
  switch (codepage) {
    case CP_420:
      switch (ch) {
        case 0xDF: return (0xf0); break; /*0*/
        case 0xEA: return (0xf1); break; /*1*/
        case 0xEB: return (0xf2); break; /*2*/
        case 0xED: return (0xf3); break; /*3*/
        case 0xEE: return (0xf4); break; /*4*/
        case 0xEF: return (0xf5); break; /*5*/
        case 0xFB: return (0xf6); break; /*6*/
        case 0xFC: return (0xf7); break; /*7*/
        case 0xFD: return (0xf8); break; /*8*/
        case 0xFE: return (0xf9); break; /*9*/
        default: return(ch);
      }
    case CP_864:
    case CP_1046: return (ch + 0x30 - 0xb0); break;
/*    case 424:*/
/*    case 856:*/
/*    case 862:*/
/*    case 916:*/
/*    case 1089:*/
/*    case 1255:*/
/*    case 1256:*/
    default:
      return (ch);
  }

}

int CB2VIS(PPARAMRec csRec, PUINT puOrder)
{
  int i=0, RC=0;
  CB2V cb;
  int saved_symmetric = 0;
  int special_symmetric=FALSE;
  static unsigned char   *tempbuf= NULL;
  static size_t tempbufSize=0;
  size_t BufferSize;

  /*unsigned short prevLink=0, lastLink=0, currLink;*/
  /*int iend, step;*/

  csRec->size_out=0;

   /*******************/
   /* Initializations */
   /*******************/
  cb.ix=0;
  cb.Compac=0;
  cb.Shaping=0;

  if(
    (csRec->texttype_in==TT_VISUAL) &&
    (csRec->texttype_out==TT_IMPLICIT) &&
    (csRec->orient_in!=csRec->orient_out)
    ){
    /* Inversion is required */                 /*Simon*/
    BufferSize=(csRec->size+1)*sizeof(char);
    if (BufferSize > tempbufSize) {
      if (tempbuf) free (tempbuf);
      tempbuf=(unsigned char *)malloc(BufferSize);
      if(!tempbuf) return(BDTERR_NOMEM);
      tempbufSize=BufferSize;
    }
    memcpy(tempbuf, csRec->buffer_in, csRec->size);
    inver((char *)tempbuf, csRec->size, csRec->codepage, FALSE);    /*Simon*/
    csRec->buffer_in=(char *)tempbuf;
    csRec->orient_in=csRec->orient_out;
  }

  if ((csRec->texttype_out==TT_VISUAL) &&
    (csRec->texttype_in==TT_IMPLICIT) &&
    (csRec->orient_in!=csRec->orient_out)){
      csRec->orient_out=csRec->orient_in;
      csRec->swapFlag=TRUE;
      saved_symmetric=!(csRec->symmetric);
      csRec->symmetric=TRUE;
    }
    else {
      csRec->swapFlag=FALSE;
    }

  if (csRec->texttype_out!=csRec->texttype_in) {
    special_symmetric=!csRec->symmetric;
    csRec->symmetric=TRUE;
  }



  if (csRec->orient_out == OR_LTR)
    cb.outLev = LTR;
  else
    cb.outLev = RTL;

  if (csRec->texttype_in!=csRec->texttype_out)
    csRec->order_flag=TRUE;
  else
    csRec->order_flag=FALSE;


 /*----------------------------------------*/
 /* Determination of the base level basLev */
 /*----------------------------------------*/


  BaseLvl(csRec, &cb);

  while(cb.ix < csRec->size) {
    pass1(csRec, &cb);
    cb.ix++;
  }


   /* do Implicit process for UBAT_B to resolve possible conditional string */
  cb.xType = CH_B;
  ucics(csRec, &cb);
  if(csRec->A_level)
  {
    for(i=0; i < csRec->size; i++)
      csRec->A_level[i]=(unsigned char)(csRec->TrgToSrcMap[i] & 0x00FF);
  }

  if(csRec->order_flag == TRUE)
  {

  /* Pass 2 :This pass must not be executed when there is no request
               for reordering */

    pass2(csRec, &cb);

    /******************************************************************/
    /* XJR: add reording optioin                                      */
    /******************************************************************/
    if ( puOrder )
    {
      for ( i=0; i<csRec->size; i++ )
      {
        puOrder[i] = csRec->size - 1 - csRec->TrgToSrcMap[i] & 0x1FFF;
      } /* endfor */
    } /* endif */

  /* Pass 3 :The logical to visual mapping must be converted to a visual-
               to-logical mapping. We use bit 14 in each position of the
               target area to mark positions which have been processed.  */
    pass3(csRec, &cb);
  } /* Simon*/
/*    for(i=0; i < csRec->size; i++)
      csRec->buffer_out[i]=csRec->TrgToSrcMap[i];*/


  /* Pass 4 :Replaces the content of the target area set by pass3 with
               the source charcters from the indicated position. European
               digits which need Arabic-Indic shapes are translated; Arabic
               presentation forms which need shaping are replaced by the
               corresponding nominal letters; symbols which need swapping
               are replaced by thier symmetric symbol; bidi special codes
               which need replacement are replaced. */

    pass4(csRec, &cb);



  /*}*/ /*Simon*/


 /* Pass 5 : Excuted only if Shaping is not zero and it replaces Arabic
             base letters by the appropriate presentation forms.  */

  if (csRec->codepage<=CP_1046) { /* The last Arabic codepage requiring shaping*/

    if((csRec->txtShp_flag != 0) && (csRec->txtShp_flag != SH_BASE))
    /*pass5(csRec, &cb);*/    /*Simon*/
    {
      if (csRec->codepage<CP_1046) {
        if (tempbuf) free(tempbuf);
        tempbuf=(unsigned char *)malloc((csRec->size+1)*sizeof(char));
        MapTo1046((unsigned char *)csRec->buffer_out, tempbuf, (int)csRec->size, csRec->codepage);
      }


    else
      tempbuf=(unsigned char *)csRec->buffer_out;
    BidiShape(tempbuf, csRec->size, FALSE, 0,
          csRec->orient_out, csRec->txtShp_flag);
    if (csRec->codepage<CP_1046)
      MapFrom1046(tempbuf, (unsigned char *)csRec->buffer_out, (int)csRec->size, csRec->codepage);
    }
  }

  if(csRec->size_out == 0 )
    csRec->size_out=csRec->size;

  if(csRec->swapFlag){
      /* Inversion is required */                   /*Simon*/
    inver(csRec->buffer_out, csRec->size, csRec->codepage, saved_symmetric);/*Simon*/
  }                               /*Simon*/
  if(special_symmetric)
    for(i=0; i < csRec->size; i++)
      csRec->buffer_out[i]=mirror(csRec->buffer_out[i], csRec->codepage);


  return(RC);
}

/************************************************************************/
void BaseLvl(PPARAMRec csRec,
       PCB2V cb)
{
  CHTYPE chtype;

  cb->basLev = 0;

  if (csRec->orient_in == OR_RTL)       /* 0 = LTR, 1 = RTL*/
    cb->basLev = 1;
  else if (csRec->orient_in == OR_LTR)
    ;
  else
  {
    while(cb->ix < csRec->size) {
      chtype = getchtype(csRec->buffer_in[cb->ix], csRec->codepage) & CH_BATYPES;
      if ( chtype == CH_R) { cb->basLev=1; break; }
      if( chtype == CH_L) break;
      cb->ix++;
    }

  }


  cb->curLev = cb->basLev;

  cb->impLev = 0;
  if (cb->basLev == 0)
    cb->impSta = 0;
  else
    cb->impSta = 9;

  cb->condPos = -1;
  cb->linsepPos = -1;

  if ( (cb->basLev == 0) && (cb->outLev == RTL) )
    cb->addLev=2;
  else
    cb->addLev=0;

}

void ucics(PPARAMRec csRec,
       PCB2V cb)
{

  int i=0;
  unsigned short newIS, Special, newIL, sCond;


  newIS   = impTab[cb->impSta][cb->xType];
  Special = newIS >> 5;
  newIS   = newIS & 0x1F;
  newIL   = impTab[newIS][ITIL];

  if (Special > 0)
    switch(Special)
    {
      case 1:    /* note a: set characters to level 0 from condPos until last */
        for(i=cb->condPos; i < cb->ix; i++)
        {
          csRec->TrgToSrcMap[i] = csRec->TrgToSrcMap[i] & 0xC000;
          csRec->TrgToSrcMap[i] = csRec->TrgToSrcMap[i] |(cb->curLev);
        }
        cb->condPos = -1;               /* Haytham 19/4/98*/
        break;

      case 2:     /* note b: set characters to level 1 from condPos until last */
        for(i=cb->condPos; i < cb->ix; i++)
        {
          csRec->TrgToSrcMap[i] = csRec->TrgToSrcMap[i] & 0xC000;
          csRec->TrgToSrcMap[i] = csRec->TrgToSrcMap[i] |(cb->curLev+1);
        }
        cb->condPos = -1;               /* Haytham 19/4/98*/
        break;

      case 3:    /* note c: set characters to level 1 from condPos until next to last
                    and set last character to level 2 */
        for(i=cb->condPos; i < cb->ix; i++)
        {
          csRec->TrgToSrcMap[i] = csRec->TrgToSrcMap[i] & 0xC000;
          csRec->TrgToSrcMap[i] = csRec->TrgToSrcMap[i] |(cb->curLev+1);
        }
        /*csRec->TrgToSrcMap[i]++;*/          /* Haytham 19/4/98*/
        csRec->TrgToSrcMap[i]+=cb->curLev+cb->addLev+2; /* Haytham 19/4/98*/
        cb->condPos = -1;               /* Haytham 19/4/98*/
        break;

      case 4:    /* note d: set condPos at the current character position  */
        cb->condPos = cb->ix;
        break;

      case 5:    /* note e: mark that there is no conditional string  */
        cb->condPos = -1;
        break;

    }

  sCond = impTab[newIS][ITCOND];
  if (sCond == 0) {
    if (cb->condPos > -1) {
      for(i=cb->condPos; i < cb->ix; i++) {
        csRec->TrgToSrcMap[i] = csRec->TrgToSrcMap[i] & 0xC000;
        csRec->TrgToSrcMap[i] = csRec->TrgToSrcMap[i] |
                      (cb->curLev+newIL);
        if ( ((cb->curLev+newIL) % 2 == 0)
/*           &&  ( (csRec->buffer_in[i] != 0x206C) || */
/*             (csRec->buffer_in[i] != 0x206F)  )*/
             )
             /* clear bit 15 */
          csRec->TrgToSrcMap[i] = csRec->TrgToSrcMap[i] & 0x7FFF;
      }

      cb->condPos = -1;
      if (cb->linsepPos >= 0) {
        csRec->TrgToSrcMap[cb->linsepPos]=0;
        cb->linsepPos = -1;
      }

    }
  }
  else if (cb->condPos == -1)
    cb->condPos = cb->ix;


  cb->impLev = newIL;
  cb->impSta = newIS;
  cb->wTarget = (unsigned short)(cb->wTarget | (cb->curLev+cb->impLev));

}
void  pass1(PPARAMRec csRec,
      PCB2V cb)
{

  CHTYPE cType;
  static CHTYPE pType; /*Haytham 22/4/98*/
  int savIL;

  /*if(cb->ix >=1)
    pType=getchtype(csRec->buffer_in[cb->ix-1], csRec->codepage) & CH_BATYPES;
  else
    pType=0;*/  /*removed Haytham 22/4/98*/
  if (cb->ix ==0) /*Haytham 22/4/98*/
    pType=0;    /*Haytham 22/4/98*/

  cb->wTarget=0;

  cType = getchtype(csRec->buffer_in[cb->ix], csRec->codepage) & CH_BATYPES;
  switch(cType){

   /* Block Separator */
/*    case CH_B:*/
          /* do implicit process for CH_B */

/*      cb->wTarget = 0;*/

          /* redo Determination of the base level basLev */
/*      BaseLvl(csRec, cb);*/
/*      break;*/

   /* Segment Separator */
    case CH_S:
          /* do implicit process for CH_S */

      cb->wTarget = (unsigned short)cb->basLev;
      break;

    default:

      if((cType==CH_AN)
         &&((csRec->num_flag == NUM_ARABIC ) ))
         /*) && (cb->araLet == 1) )*/
      {
        cb->wTarget = cb->wTarget | 0x8000;
        cType = CH_EN;
      }
      if (cType == CH_L)
        cb->araLet = 0;

      if (cType == CH_R) {
        if /* ((csRec->buffer_in[cb->ix] >= 0x0600) &&
          (csRec->buffer_in[cb->ix] <= 0x06EF)*/ /*Simon*/
           ((getchtype(csRec->buffer_in[cb->ix], csRec->codepage) & CH_ARSIMLET) == CH_ARSIMLET) { /*Simon*/
          cb->araLet = 1;
          if (csRec->txtShp_flag != FALSE && csRec->txtShp_flag != SH_BASE)
            cb->Shaping = 1;
        }
        /* if ((csRec->buffer_in[cb->ix] >= 0xFB50) &&*/  /*Simon */
        /*  (csRec->buffer_in[cb->ix] <= 0xFEFC) */    /*Simon*/
        if ((getchtype(csRec->buffer_in[cb->ix], csRec->codepage) & CH_ARPRSLET) == CH_ARPRSLET) { /*Simon*/

          cb->araLet = 1;
          if ((csRec->txtShp_flag != FALSE && csRec->txtShp_flag != SH_BASE)  &&
            (csRec->formShp == 1)) {
            cb->Shaping = 1;
            cb->wTarget = cb->wTarget | 0x8000;
          }
        }
      }
      savIL = cb->impLev;
      /* Added by Haytham 19/4/98 */
      if (cType == CH_ET ){
        int iFlag=0, i2=0;
        if(pType == CH_EN )
          cType = CH_EN;
        while(iFlag == 0){
          if (cb->ix+i2 < csRec->size)
            pType = getchtype(csRec->buffer_in[cb->ix+i2+1], csRec->codepage) & CH_BATYPES;
          if(pType == CH_EN ){
            cType = CH_EN;
            iFlag=1;
          }
          if(pType == CH_ET){
            i2++;
            continue;
          }
          else
            iFlag=1;
        }
      }
      /* end of Added by Haytham 19/4/98 */

          /* do implicit Process for cType */
      cb->xType = cType;
      ucics(csRec, cb);

          /* Find if character is spacing */
      /*if ( UCQSPAC(cType) == FALSE)*/ /*Simon*/
      if ((getchtype(csRec->buffer_in[cb->ix], csRec->codepage)& CH_NSPACE) == CH_NSPACE)

        cb->wTarget = (unsigned short)((cb->curLev + savIL) | 0x4000);

      if ( (cType == CH_N) &&
         (csRec->symmetric == TRUE) &&
         ( (cb->condPos > -1)  ||
           ((cb->curLev+cb->impLev) % 2 != 0) ))
        cb->wTarget = cb->wTarget | 0x8000;

      break;   /* Case End */

  }

  csRec->TrgToSrcMap[cb->ix]=cb->wTarget; /* put wTarget in target area at
                                             position ix  */
  pType=cType; /*Haytham 22/4/98*/
}
void  pass2(PPARAMRec csRec,
       PCB2V cb)

{

  int lolim[19];
  int hilim[19];
  int splolim=0;
  int sphilim=-1;
  int x;
  int i, xLev, lastLev=0;
  int iy;
  int wLev;

  for (x=0; x < 19; x++)
  {
    lolim[x] = 0;
    hilim[x] = csRec->size-1;
  }

  for(cb->ix=0; cb->ix < csRec->size; cb->ix++)
  {
    /* retain only the level */
    xLev = csRec->TrgToSrcMap[cb->ix] & 0x1FFF;
     /* set limits of inverse for not-spacing if needed */
    if ((xLev & 1) != cb->outLev)
    {
      if (cb->ix <= sphilim )  /* ix belongs to defined run */
       /* reverse not-spacing */
        x = splolim + sphilim - cb->ix;
      else
      {
    /* According to the pseudo-code there is a scan process missing
       in this stage of the program  to get sphilim */
        if( ((csRec->TrgToSrcMap[cb->ix] & 0x4000) == 0) &&
          ((csRec->TrgToSrcMap[cb->ix+1] & 0x4000) != 0) )
          x = splolim + sphilim - cb->ix;
        else
          x = cb->ix;
      }
    }
    else
      x = cb->ix;
       /* check the limits : if the last character had a level (lastLev)
              greater than or equal to xLev, its segment is contained in the
              segment of ix, and all boundaries are already known; if xLev
              is greater than lastLev, this is the start of a new segment,
              lolim and hilim must be updated  */
    if (xLev > lastLev)
    {
      wLev = xLev;
      iy = cb->ix+1;
      while(wLev > lastLev)
      {
        lolim[wLev] = cb->ix;
        while(iy < csRec->size)
        {
          if ((csRec->TrgToSrcMap[iy] & 0x3FFF) < (unsigned int)wLev)
            break;
          iy++;
        }

        hilim[wLev] = iy-1;
        wLev--;

      }
    }

     /* Reverse according to levels */
    for(i= xLev; i >= (cb->outLev+1); i--)
      x = lolim[i]+hilim[i] -x;

      /* Update target area */
    csRec->TrgToSrcMap[cb->ix] = csRec->TrgToSrcMap[cb->ix] & 0xa000;
    csRec->TrgToSrcMap[cb->ix] = csRec->TrgToSrcMap[cb->ix] + x;
    lastLev = xLev;
  }
}
/**************************************************************************/
void  pass3(PPARAMRec csRec,
      PCB2V cb)
{
  unsigned int logPos, logPtr, visPos, wPtr;

  for(cb->ix = 0; cb->ix < csRec->size; cb->ix++)
  {
    logPos = cb->ix;
    logPtr = csRec->TrgToSrcMap[cb->ix];
    while( (logPtr & 0x4000) == 0)
    {
      visPos = logPtr & 0x1FFF;
      wPtr = csRec->TrgToSrcMap[visPos];
      if( (wPtr & 0x4000) > 0) break;
      csRec->TrgToSrcMap[visPos] = logPos+(logPtr & 0xa000) + 0x4000;
      logPos = visPos;
      logPtr = wPtr;
    }
  }
}
/**************************************************************************/
void  pass4(PPARAMRec csRec,
      PCB2V cb)
{
  int i;
  unsigned int xTran;
  unsigned char  xchar;
  CHTYPE test;
  cb;
  for (i=0; i < csRec->size; i++) {
    xTran = csRec->TrgToSrcMap[i] & 0x8000;
    if(csRec->order_flag == TRUE) {
      xchar = csRec->buffer_in[csRec->TrgToSrcMap[i] & 0x1FFF];
    }
    else
      xchar = csRec->buffer_in[i];
    test=getchtype(xchar, csRec->codepage);
    if (xTran > 0) {
      if ((test & CH_BIDICTRL) == CH_BIDICTRL) /*Simon*/
        xchar = (unsigned char)csRec->boc_flag;
      else if  ((test & CH_HDIGIT) == CH_HDIGIT)
        xchar=ArabicDigit(xchar, csRec->codepage);  /*Simon*/
      else if ((test & CH_EDIGIT) == CH_EDIGIT)
        xchar = xchar ;

      else
        if ((test & CH_ARPRSLET) == CH_ARPRSLET) /*Simon*/

         /* xchar = comp2nom(xchar - 0xFE80);*/
       /* There is an array called ComtoNom in uc_tables.h which contatin the
      corresponding equivalent nominal character */
          ;
      else {
        if((csRec->symmetric !=0)) /*Haytham 19/4/98*/
          xchar = mirror(xchar, csRec->codepage);
        if((csRec->orient_in != csRec->orient_out))
        {

          int i1, j;
          CHTYPE cType;
          i1=csRec->TrgToSrcMap[i]& 0x1FFF;
          j=i;
          do{
            cType=getchtype(csRec->buffer_in[i1], csRec->codepage) & CH_BATYPES;
            if(cType==CH_R)
              break;
            else if(cType==CH_L || cType==CH_EN){
              xchar = mirror(xchar, csRec->codepage);
              break;
            }
            else {
              j++;
              i1=csRec->TrgToSrcMap[j]& 0x1FFF;
            }
          } while(j<=csRec->size);
        }

      }

    }

    csRec->buffer_out[i]=xchar;

  }

}
/**************************************************************************/
int inver (char *buffer,size_t num_of_elements,CODEPAGE codepage,int symm_flag)
/* inver a stream of char */
{
  char temp;
  int i,j;

  for (i=0, j=num_of_elements-1; i<j; i++, j--)
  {
    if (symm_flag==TRUE) {
      temp=mirror (buffer[i], codepage);
      buffer[i]=mirror(buffer[j], codepage);
      buffer[j]=temp;
    }
    else {
      temp=buffer[i];
      buffer[i]=buffer[j];
      buffer[j]=temp;
    }
  }
  return 0;
}

int  BidiShape (unsigned char *Buffer,
             size_t length_t,
             int Onecell,
           int OS_flag,
             int orient,
             int csd)
{
 char state=InitialState;
                     /*this variable holds the state of the state machine */
 unsigned int counter  /* loop counter */
               ;
 unsigned char *current,     /* to hold characters for shaping */
               *first_prec,
               *sec_prec,
               *third_prec,
               *next,
                blank=0x20,
                symbol=0x21;  /* ! */
/********************** start of processing ***********************************/

   first_prec = NULL;
   sec_prec = NULL;
   switch (csd)
   {
     case SH_SHAPED : /* auto shaping */
       if (orient==OR_LTR)  /* LTR */
         {
           /* loop on whole string to be shaped */
           for (counter=length_t - 1;(int)counter>=0;counter--)
            {
              current=&Buffer[counter];
              /* check if we have a first preceding charcater */
              if (counter<(length_t-1))
                  first_prec=&(Buffer[counter+1]);
              else
                  first_prec=&blank;
              /* check if we have a second preceding charcater */
              if (counter<(length_t-2))
                 sec_prec=&(Buffer[counter+2]);
              else
                sec_prec=&blank;
              /* check if we have a third preceding charcater */
              if (counter<(length_t-3))
                 third_prec=&(Buffer[counter+3]);
              else  third_prec=&blank;
              /* now shape current and the preceding chars */
              csd_engine(current,first_prec,sec_prec,third_prec,
                         &state,
             OS_flag,
             Onecell);
            }
            /* now assume we have a symbol after the string and reshape
               last three chars. We do not assume a blank, because
               in case last char is a seen, we do not want it to
               overwrite the blank with a tail.  */
            third_prec=sec_prec;
            sec_prec=first_prec;
            first_prec=&(Buffer[0]); /* this is the last char in the string */
            current=&symbol;         /* this is a hypothetical symbol */
            csd_engine(current,first_prec,sec_prec,third_prec,
                       &state,
             OS_flag,
             Onecell);
         }
       else /* RTL */
         {
           /* loop on whole string to be shaped */
           for (counter=0;counter<length_t;counter++)
            {
              current=&Buffer[counter];
              /* check if we have a firct preceding charcater */
              if (counter>0)
                 first_prec=&(Buffer[counter-1]);
              else  first_prec=&blank;
              /* check if we have a second preceding charcater */
              if (counter>1)
                 sec_prec=&(Buffer[counter-2]);
              else  sec_prec=&blank;
              /* check if we have a third preceding charcater */
              if (counter>2)
                 third_prec=&(Buffer[counter-3]);
              else  third_prec=&blank;
              /* now shape current and the preceding chars */
               csd_engine(current,first_prec,sec_prec,third_prec,
                          &state,
              OS_flag,
              Onecell);
            }
            /* now assume we have a symbol after the string and reshape
               last three chars. We do not assume a blank, because
               in case last char is a seen, we do not want it to
               overwrite the blank with a tail.  */
            third_prec=sec_prec;
            sec_prec=first_prec;
            first_prec=&(Buffer[length_t-1]);
            current=&symbol;         /* this is a hypothetical symbol */
            csd_engine(current,first_prec,sec_prec,third_prec,
                       &state,
             OS_flag,
             Onecell);
         }
       break;
     default   : /* all special shapes */
       if (orient==OR_LTR)  /* LTR */
         {
           next=&(Buffer[length_t-1]); /* initial value */
           /* loop on whole string to be shaped */
           for (counter=length_t-1;(int)counter>=0;counter--)
            {
              current=next;
              /* check if we have a next charcater */
              if (counter>0)
                  next=&(Buffer[counter-1]);
              else
                  next=&symbol;
              csd_special(csd,current,next);
            }
         }
       else /* RTL */
         {
           next=&(Buffer[0]); /* initial value */
           /* loop on whole string to be shaped */
           for (counter=0;counter<length_t;counter++)
            {
              current=next;
              /* check if we have a next charcater */
              if (counter<(length_t-1))
                  next=&(Buffer[counter+1]);
              else
                  next=&symbol;
              csd_special(csd,current,next);
            }
         }
         break;
   } /* end switch */
 return (0);
}
unsigned char Lamalef(unsigned char x, CODEPAGE codepage)
{
  switch (codepage) {
    case CP_420:
      switch (x) {
        case 0x47: return (0xb2); break; /*with madda isolate*/
        case 0x48: return (0xb3); break; /*with madda final*/
        case 0x49: return (0xb4); break; /*with hamza isolate*/
        case 0x51: return (0xb5); break; /*with hamza final*/
        case 0x56: return (0xb8); break; /*plain isolate*/
        case 0x57: return (0xb9); break; /*plain final*/
        default: return (0);
      }
      break;
    case CP_864:
      switch (x) {
        case 0xa2: return (0xfa); break;/* with madda final*/
        case 0xa5: return (0x9a); break;/* with hamza final*/
        case 0xa8: return (0x9e); break;/* plain final*/
        case 0xc2: return (0xf9); break;/* with madda isolate*/
        case 0xc3: return (0x99); break;/* with hamza isolate*/
        case 0xc7: return (0x9d); break;/* plain isolate*/
        default:   return (0);
      }
      break;
    case CP_1046:
      switch (x){
        case 0xc5: return (0x9e); break;/* hamza under*/
        case 0xc2: return (0x9c); break;/* madda*/
        case 0xc3: return (0x9d); break;/* hamza*/
        case 0xc7: return (0x9f); break;/* plain*/
        case 0xdc: return (0xa1); break;/* special madda*/
        case 0xdd: return (0xa2); break;/* special hamza*/
        case 0x80: return (0xa3); break;/* special hamza under*/
        case 0xde: return (0xa5); break;/* special plain*/
        default: return (0);
      }
      break;
    default: return (0);
  }

}

static char sccsid[] = "@(#)39  1.2  src/bos/usr/lib/nls/loc/methods/shared.bidi/csd.c, cfgnls, bos410, 9428A410 9/10/93 11:10:14";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: ConnLamAlef
 *    Group
 *    InitMidShape
 *    InitialShape
 *    IsoFinalShape
 *    IsoLamAlef
 *    SpecialAlef
 *    ThreeQuarterSeen
 *    Vowel
 *    YehFinal
 *    csd_engine
 *    csd_special
 *    reset_alefs
 *    reset_tail
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/


/****************************************************************************/
/****************************************************************************/
/* reset tail character to space */
void reset_tail(unsigned char *ch)
{
  if (*ch==A_TAIL)
     *ch=SPACE;
}
/****************************************************************************/
/* reset special alefs to normal alefs */
void reset_alefs(unsigned char *ch)
{
     if (*ch==A_ALEF_HAMZA_SPECIAL)
        *ch=A_ALEF_HAMZA_ISOLATED;
     else if (*ch==A_ALEF_MADDA_SPECIAL)
        *ch=A_ALEF_MADDA_ISOLATED;
     else if (*ch==A_ALEF_SPECIAL)
        *ch=A_ALEF_ISOLATED;
     else if (*ch==A_ALEF_HAMZA_UNDER_SPECIAL)
        *ch=A_ALEF_HAMZA_UNDER_ISOLATED;
}
/****************************************************************************/
/* checks if a character is a vowel */
int Vowel(unsigned char ch)
{
  return ((Group(ch)==Vowels_Group) || (Group(ch)==Iso_Vowels_Group));
}
/****************************************************************************/
/* Returns the group of the given characters, all characters under 0x80,
   except space, are considered symbols. For characters over 0x80, the
   lookup table CHRGRP is referenced.
   */
int Group (unsigned char ch)
{
  if (ch<START_NATIONAL)
     if (ch==SPACE)
          return (Spaces_Group);
     else return (Symbols_Group); /* Latin char */
  else return ((int)CHRGRP[ch-128][4]);
}
/****************************************************************************/
/* If the prev character connects to the left, final is returned, otherwise
   isolated is returned. OSflag is considred in cases of seen and yehhamza */
unsigned char IsoFinalShape (unsigned char ch, unsigned char prev, int OSflag)
{
  unsigned char newch;
  if (ch<START_NATIONAL) return (ch);  /* Latin ch */
  if (((Group(prev)>=Normal_Group)
     && (Group(prev)!=Vowels_Group)) ||
                                   /* i.e. normal, seen, yehhamza, lam */
     ((Group(prev)==Vowels_Group) && (prev==CHRGRP[prev-128][MID])))
                                   /* i.e. a connected vowel */
     newch=CHRGRP[ch-128][FIN];  /* return final */
  else newch=CHRGRP[ch-128][ISO];  /* return isolated */
  if (OSflag==TEXT_SPECIAL)
   {
                                               /* onecell final seen chars */
     if ((newch>=A_ONECELL_SEEN) && (newch<=A_ONECELL_DAD))
        newch=newch-0x50;   /* threequarter seen chars */
     if ((newch==A_YEH_HAMZA_FINAL)
         || (newch==A_YEH_HAMZA_ISOLATED))
        newch=A_YEH_HAMZA_INITIAL;
   }
 return(newch);
}
/****************************************************************************/
/* given a yehhamza, it returns a yeh, final or isolated depending
   on the previous char */
unsigned char YehFinal (unsigned char ch, unsigned char prev)
{
  ch;
  if ((Group(prev)>=Normal_Group) ||
                                       /* i.e. normal, seen, yehhamza, lam */
     ((Group(prev)==Vowels_Group) && (prev==CHRGRP[prev-128][MID])))
                                   /* i.e. a connected vowel */
    return(A_YEH_FINAL);  /* return final yeh */

  else return(A_YEH_ISOLATED);   /* return isolated yeh */
}
/****************************************************************************/
/* returns the middle or initial state of the current char dependign on */
/* whther the character before it is connected to it or not */
unsigned char InitMidShape(unsigned char ch, unsigned char prev)

{
  if (ch<START_NATIONAL)  return (ch);  /* Latin ch */
  if (Group(prev)<5)  /* no connection or R_connect character */
      ch=CHRGRP[ch-128][INIT];  /* return initial */
  else if (Group(prev)<9) /* R+L connector */
      ch=CHRGRP[ch-128][MID];  /* return middle */
  else /* it must be a vowel */
    if (prev==CHRGRP[prev-128][MID]) /* connected vowel */
        ch=CHRGRP[ch-128][MID];  /* return middle */
      else ch=CHRGRP[ch-128][INIT];  /* return initial */
  return(ch);
}
/****************************************************************************/
/* returns the middle or initial state of the current char dependign on state */
unsigned char InitialShape(unsigned char ch, char state)
{
  if (ch<START_NATIONAL) return (ch);  /* Latin ch */
  if (state==InitialState)
    return (CHRGRP[ch-128][INIT]);
  else return (CHRGRP[ch-128][MID]);
}
/****************************************************************************/
/* input is a seen character, output is the threequarter shape of that ch */
unsigned char ThreeQuarterSeen(unsigned char ch)
{
  switch(ch)
  {
   case A_INITIAL_SEEN      :
   case A_ONECELL_SEEN      :
   case A_THREEQUARTER_SEEN :
     return (A_THREEQUARTER_SEEN);
   case A_INITIAL_SHEEN      :
   case A_ONECELL_SHEEN      :
   case A_THREEQUARTER_SHEEN :
     return (A_THREEQUARTER_SHEEN);
   case A_INITIAL_SAD      :
   case A_ONECELL_SAD      :
   case A_THREEQUARTER_SAD :
     return (A_THREEQUARTER_SAD);
   case A_INITIAL_DAD      :
   case A_ONECELL_DAD      :
   case A_THREEQUARTER_DAD :
     return (A_THREEQUARTER_DAD);
  }
  return 0;
}
/****************************************************************************/
/* returns the special (AIX lam_alef) shape of the given alef */
unsigned char SpecialAlef(unsigned char alef)
/* char alef;*/
{
  switch (alef)
  {
    case A_ALEF_FINAL    :
    case A_ALEF_ISOLATED :
    case A_ALEF_SPECIAL  :
         return (A_ALEF_SPECIAL);
    case A_ALEF_HAMZA_FINAL    :
    case A_ALEF_HAMZA_ISOLATED :
    case A_ALEF_HAMZA_SPECIAL  :
         return (A_ALEF_HAMZA_SPECIAL);
    case A_ALEF_MADDA_FINAL    :
    case A_ALEF_MADDA_ISOLATED :
    case A_ALEF_MADDA_SPECIAL  :
         return (A_ALEF_MADDA_SPECIAL);
    case A_ALEF_HAMZA_UNDER_FINAL:
    case A_ALEF_HAMZA_UNDER_ISOLATED:
    case A_ALEF_HAMZA_UNDER_SPECIAL:
         return (A_ALEF_HAMZA_UNDER_SPECIAL);
  }
  return 0;
}
/****************************************************************************/
/* given an alef, it returns the isolated lamalef */
unsigned char IsoLamAlef(unsigned char alef)
/*char alef;*/
{
  switch (alef)
  {
    case A_ALEF_ISOLATED :
    case A_ALEF_FINAL    :
    case A_ALEF_SPECIAL  :
         return (A_LAM_ALEF_ISOLATED);
    case A_ALEF_HAMZA_ISOLATED :
    case A_ALEF_HAMZA_FINAL    :
    case A_ALEF_HAMZA_SPECIAL  :
         return (A_LAM_ALEF_HAMZA_ISOLATED);
    case A_ALEF_MADDA_ISOLATED :
    case A_ALEF_MADDA_FINAL    :
    case A_ALEF_MADDA_SPECIAL  :
         return (A_LAM_ALEF_MADDA_ISOLATED);
    case A_ALEF_HAMZA_UNDER_ISOLATED :
    case A_ALEF_HAMZA_UNDER_FINAL    :
    case A_ALEF_HAMZA_UNDER_SPECIAL  :
         return (A_LAM_ALEF_HAMZA_UNDER_ISOLATED);
  }
  return 0;
}
/****************************************************************************/
/* given an alef, it returns the connected lamalef */
unsigned char ConnLamAlef(unsigned char alef)
/*char alef;*/
{
  switch (alef)
  {
    case A_ALEF_ISOLATED :
    case A_ALEF_FINAL    :
    case A_ALEF_SPECIAL  :
         return (A_LAM_ALEF_CONNECTED);
    case A_ALEF_HAMZA_ISOLATED :
    case A_ALEF_HAMZA_FINAL    :
    case A_ALEF_HAMZA_SPECIAL  :
         return (A_LAM_ALEF_HAMZA_CONNECTED);
    case A_ALEF_MADDA_ISOLATED :
    case A_ALEF_MADDA_FINAL    :
    case A_ALEF_MADDA_SPECIAL  :
         return (A_LAM_ALEF_MADDA_CONNECTED);
    case A_ALEF_HAMZA_UNDER_ISOLATED :
    case A_ALEF_HAMZA_UNDER_FINAL    :
    case A_ALEF_HAMZA_UNDER_SPECIAL  :
         return (A_LAM_ALEF_HAMZA_UNDER_CONNECTED);
  }
  return 0;
}
/****************************************************************************/
/* To handle special shapes : base, initial, middle, final, isolated. */
/* We need the next char for lamalef cases in base shape */
void csd_special (int csd, unsigned char *current, unsigned char *next)
{
	csd;
     if((*current>=START_NATIONAL))     /* not Latin character */
     {
        /* handle all special cases */
        switch (*current)
        {
          /* switch all special alefs to normal alefs */
          case A_ALEF_MADDA_SPECIAL :
                    *current=A_ALEF_MADDA_ISOLATED; break;
          case A_ALEF_SPECIAL :
                    *current=A_ALEF_ISOLATED; break;
          case A_ALEF_HAMZA_SPECIAL :
                    *current=A_ALEF_HAMZA_ISOLATED; break;
          case A_ALEF_HAMZA_UNDER_SPECIAL :
                    *current=A_ALEF_HAMZA_UNDER_ISOLATED; break;
          /* replace tail with blank */
          case A_TAIL : /* tail */
                    *current=A_RSP; break;
        }
        /* set character to desired special shape */
       /* switch(csd)*/
       /* {*/
       /*   case SH_BASE :*/
                            /* in Arabic base shapes are all isolated */
                              *current=CHRGRP[*current-128][ISO];
                            /* handle special cases */
                            switch (*current)
                            {
                            case A_HEH_ISOLATED : /* heh isolated */
                                      *current=A_HEH_INITIAL; /* base heh */
                                      break;
                            /* switch all seen threequarter to normal seen */
                            case A_THREEQUARTER_SEEN :
                                         *current=A_ONECELL_SEEN; break;
                            case A_THREEQUARTER_SHEEN :
                                         *current=A_ONECELL_SHEEN; break;
                            case A_THREEQUARTER_SAD :
                                         *current=A_ONECELL_SAD; break;
                            case A_THREEQUARTER_DAD :
                                         *current=A_ONECELL_DAD; break;
                            /* handle lam alef cases */
                            case A_LAM_ALEF_CONNECTED: case A_LAM_ALEF_ISOLATED:
                                     if ((*next==SPACE) || (*next==A_RSP))
                                     {
                                       *current=A_LAM_ISOLATED;
                                       *next=A_ALEF_ISOLATED;
                                     }
                            case A_LAM_ALEF_MADDA_CONNECTED:
                            case A_LAM_ALEF_MADDA_ISOLATED:
                                     if ((*next==SPACE) || (*next==A_RSP))
                                     {
                                       *current=A_LAM_ISOLATED;
                                       *next=A_ALEF_MADDA_ISOLATED;
                                     }
                            case A_LAM_ALEF_HAMZA_CONNECTED:
                            case A_LAM_ALEF_HAMZA_ISOLATED:
                                     if ((*next==SPACE) || (*next==A_RSP))
                                     {
                                       *current=A_LAM_ISOLATED;
                                       *next=A_ALEF_HAMZA_ISOLATED;
                                     }
                            case A_LAM_ALEF_HAMZA_UNDER_CONNECTED:
                            case A_LAM_ALEF_HAMZA_UNDER_ISOLATED:
                                     if ((*next==SPACE) || (*next==A_RSP))
                                     {
                                       *current=A_LAM_ISOLATED;
                                       *next=A_ALEF_HAMZA_UNDER_ISOLATED;
                                     }
                            }
/*                        break;*/
/*          case TEXT_SHFORM1 :*/
/*                            *current=CHRGRP[*current-128][INIT];*/
/*                            break;*/
/*          case TEXT_SHFORM2 :*/
/*                            *current=CHRGRP[*current-128][MID];*/
/*                            break;*/
/*          case TEXT_SHFORM3 :*/
/*                            *current=CHRGRP[*current-128][FIN];*/
/*                            break;*/
/*          case TEXT_SHFORM4 :*/
/*                            *current=CHRGRP[*current-128][ISO];*/
/*                            break;*/
/*        }*/ /* end switch */
     } /* if Arabic character */
}

/****************************************************************************/
void csd_engine(unsigned char *current,  /* current new character */
        unsigned char *prec1,    /* first preceding character */
        unsigned char *prec2,    /* second preceding character */
        unsigned char *prec3,    /* third preceding character */
        char *state,       /* the current state */
        int OSflag,              /* in Arabic, AIX or HOST shaping mode*/
        int Onecell)             /* in AIX, seen on onecell or two cells */
{/* start engine */
   char temp;    /* to hold temporary characters */

   reset_alefs(current); /* set special alefs to normal */
   reset_tail(current); /* set tail to space */
   switch (Group(*current))
   {
     case Symbols_Group :
            /* in all cases, set the previous char to final,
               if the previous char is a vowel, set also
                the second previous to final */
            *prec1=IsoFinalShape(*prec1,*prec2,OSflag);
            if (Vowel(*prec1))
                 *prec2=IsoFinalShape(*prec2,*prec3,OSflag);
            *state=InitialState; /* set to intial, because symbols
                                    do not connect */
            break;
           /* end case Symbols */
     case Spaces_Group :
            switch (*state)
            {
              case MiddleState  :
              case LamIsoState  :  /* all the normal characters */
              case LamConnState :
              case InitialLamConnState :
              case InitialLamIsoState :
                            /* set the previous char to final,
                               if the previous char is a vowel, set also
                               the second previous to final */
                           *prec1=IsoFinalShape(*prec1,*prec2,
                                                OSflag);
                           if (Vowel(*prec1))
                               *prec2=IsoFinalShape(*prec2,*prec3,
                                                    OSflag);
                           break;
              case InitialSeenState :
              case SeenState :  /* previous char was a seen,
                                   so consider a tail */
                          if (!Onecell)
                            if (Vowel(*prec1))
                                                   /* a vowel after the seen*/
                                 {
                                  /* put the vowel in current, and set prec2
                                     and prec1 to threequarterseen and to tail
                                     respectively */
                                  *prec2=ThreeQuarterSeen(*prec2);
                                  *current=IsoFinalShape(*prec1,*prec2,
                                                    OSflag);
                                  *prec1=A_TAIL;
                                 }
                            else /* no vowel after seen */
                                 {/* set prec1 and current to threequarterseen
                                     and tail respectively */
                                  *prec1=ThreeQuarterSeen(*prec1);
                                  *current=A_TAIL;
                                 }
                          else  /* onecell seen, even if there is space */
                          {
                           *prec1=IsoFinalShape(*prec1,*prec2,
                                                OSflag);
                           if (Vowel(*prec1))
                              *prec2=IsoFinalShape(*prec2,*prec3,
                                                   OSflag);
                          }
                          break;
              case YehHamzaState : /*previous char was a yehhamza */
                            if ((OSflag==TEXT_STANDARD)
                             || (OSflag==TEXT_COMPOSED))
                                { /* in AIX, yehhamza is handled like a
                                     normal char */
                                  *prec1=IsoFinalShape(*prec1,*prec2,
                                                       OSflag);
                                  if (Vowel(*prec1))
                                     *prec2=IsoFinalShape(*prec2,*prec3,
                                                    OSflag);
                                }
                            else /* HOST mode */
                                 /* in HOST, yehhamza is split in two chars,
                                     if followed by a space */
                                  if (Vowel(*prec1))
                                       /* we have a vowel after the yehhamza */
                                   {
                                   /* put the vowel in current and set
                                   prec2 and prec1 to yeh and hamza
                                   respectively */
                                   *prec2=YehFinal(*prec2,*prec3);
                                   *current=IsoFinalShape(*prec1,*prec2,
                                                   OSflag);
                                   *prec1=A_HAMZA;
                                   }
                                  else /* novowel after the yehhamza */
                                   {
                                    *prec1=YehFinal(*prec1,*prec2);
                                    *current=A_HAMZA;
                                   }
                            break;
            }/* end switch state */
            *state=InitialState; /* set to intial, because spaces
                                    do not connect */
            break;
     case Alef_Group :
              switch (*state)
              {
                case InitialState         :
                case MiddleState          :
                case SeenState            :
                case InitialSeenState     :
                case YehHamzaState        :
                            if (Vowel(*prec1))
                             {
                                 *prec1=InitialShape(*prec1,*state);
                                 *prec2=InitMidShape(*prec2,*prec3);
                             }
                             else *prec1=InitMidShape(*prec1,*prec2);
                            *current=IsoFinalShape(*current,*prec1,
                                              OSflag);
                            break;
                case LamConnState  :
                case InitialLamConnState  :
                            if ((OSflag==TEXT_STANDARD)
                             || (OSflag==TEXT_COMPOSED))
                             {
                               if (Vowel(*prec1))
                               {
                                /* set lam to middle */
                                *prec2=(CHRGRP[(*prec2)-128][MID]);
                                /* switch places of alef and vowel */
                                temp=*current;
                                *current=IsoFinalShape(*prec1,temp,
                                                       OSflag);
                                *prec1=SpecialAlef(temp);
                               }
                               else  /* no vowel, normal AIX lamalef case */
                               {
                                /* set lam to middle */
                                *prec1=(CHRGRP[(*prec1)-128][MID]);
                                *current=SpecialAlef(*current);
                               }
                             }
                            else /* Arabic HOST mode */
                               if (Vowel(*prec1)
                                                ) /*put lamalef, vowel, space */
                               {
                                  *prec2=ConnLamAlef(*current);
                                  *prec1=IsoFinalShape(*prec1,*prec2,
                                                   OSflag);
                                  *current=A_RSP;
                               }
                               else /* put lamalef , space */
                               {
                                  *prec1=ConnLamAlef(*current);
                                  *current=A_RSP;
                               }
                            break;
                case LamIsoState   :
                case InitialLamIsoState   :
                            if ((OSflag==TEXT_STANDARD)
                              || (OSflag==TEXT_COMPOSED))
                             {
                               if (Vowel(*prec1))
                               {
                                /* set lam to middle */
                                *prec2=(CHRGRP[(*prec2)-128][MID]);
                                /* switch places of alef and vowel */
                                temp=*current;
                                *current=IsoFinalShape(*prec1,*prec2,
                                                     OSflag);
                                *prec1=SpecialAlef(temp);
                               }
                               else  /* no vowel, normal AIX lamalef case */
                               {
                                /* set lam to middle */
                                *prec1=(CHRGRP[(*prec1)-128][MID]);
                                *current=SpecialAlef(*current);
                               }
                             }
                            else /* HOST mode */
                               if (Vowel(*prec1))
                                                  /*put lamalef, vowel, space */
                               {
                                  *prec2=IsoLamAlef(*current);
                                  *prec1=IsoFinalShape(*prec1,*prec2,
                                                   OSflag);

                                  *current=A_RSP;
                               }
                               else /* put lamalef , space */
                               {
                                  *prec1=IsoLamAlef(*current);
                                  *current=A_RSP;
                               }
                            break;
              }
            *state=InitialState; /* set to intial, because alefs
                                    do not connect to the left */
            break;
     case R_Conn_Group :
            if (Vowel(*prec1))
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=IsoFinalShape(*current,*prec1,OSflag);
            *state=InitialState; /* set to intial, because Rconnectors
                                    do not connect to the left */
            break;
     case Normal_Group :
            if (Vowel(*prec1))
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=InitialShape(*current,*state);
            *state=MiddleState; /* normal chars connect left and right */
            break;
     case Seen_Group :
            if (Vowel(*prec1))
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=InitialShape(*current,*state);
            *state=SeenState;
            break;
     case Yeh_Hamza_Group :
            if (Vowel(*prec1))
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=InitialShape(*current,*state);
            *state=YehHamzaState;
            break;
     case Lam_Group :
            if (Vowel(*prec1))
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=InitialShape(*current,*state);
            if (*state==InitialState)
               *state=LamIsoState;
            else
               *state=LamConnState;
            break;
     case Vowels_Group :
            if (Vowel(*prec1))
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=InitialShape(*current,*state);
           /* If we have more than one vowel in a sequence,
              LamIso and LamConn and Seen states are all reset to Middle,
              because if we have more than one vowel in a sequence, we do
              not handle these special cases any more. Otherwise the state
              is not affected by a vowel. */
            if (Vowel(*prec1))
              if (*state!=InitialState) *state=MiddleState;
            break;
     case Iso_Vowels_Group :
            /* in all cases, set the previous char to final,
               if the previous char is a vowel, set also
                the second previous to final */
            *prec1=IsoFinalShape(*prec1,*prec2,OSflag);
            if (Vowel(*prec1))
                 *prec2=IsoFinalShape(*prec2,*prec3,OSflag);
            /* if more than one vowel in a sequence,
               set state to initial, because these vowels do not connect.
               If only one vowel, set state to initial, but preserve the
               data of the seen and lam, because we can handle them after
               one vowel only. */
            if (Vowel(*prec1))
               *state=InitialState;
            else
              switch (*state)
              {
                case SeenState    : *state=InitialSeenState; break;
                case LamIsoState  : *state=InitialLamIsoState; break;
                case LamConnState : *state=InitialLamConnState; break;
                default           : *state=InitialState; break;
              }
            break;
   } /* end switch on group of current char */
}/* end engine */

/*******************************************************************\
 *                                                                 *
 *   This file implements functions to convert an input string     *
 *   from 1046 codepage to 420 or 864 codepage and vise versa      *
 *                                                                 *
 *   Functions: MapFrom1046()                                      *
 *              MapTo1046()                                        *
 *                                                                 *
\*******************************************************************/


/*******************************************************************\
 *                                                                 *
 *  Function Name : MapFrom1046                                    *
 *                                                                 *
 *  Parameters : fromBuffer - Buffer needed to be mapped           *
 *               toBuffer   - Buffer holding the mapping data      *
 *               size       - number of characters to be mapped    *
 *                            if -1 then the string length will    *
 *                            be accounted for.                    *
 *               codepage   - target codepage (from the table in   *
 *                            transform.h: 0=420, 1=864)           *
 *                                                                 *
 *   Description : maps fromBuffer from 1046cp. to 420cp. or 864cp *
 *                                                                 *
 *  Return : void                                                  *
 *                                                                 *
\*******************************************************************/
void MapFrom1046(unsigned char* fromBuffer, unsigned char* toBuffer,int size, CODEPAGE codepage)
{
int length=0,
    i=0;

  if (size==-1)
     length = strlen((char *)fromBuffer);
  else
     length = size;

  for(i=0 ; i<(length - 1) ;i++)
  {
    toBuffer[i] = From1046[codepage] [fromBuffer[i]];
  }

}

/*******************************************************************\
 *                                                                 *
 *  Function Name : MapTo1046                                      *
 *                                                                 *
 *  Parameters : fromBuffer - Buffer needed to be mapped           *
 *               toBuffer   - Buffer holding the mapping data      *
 *               size       - number of characters to be mapped    *
 *                            if -1 then the string length will    *
 *                            be accounted for.                    *
 *               codepage   - source codepage (from the table in   *
 *                            transform.h: 0=420, 1=864)           *
 *                                                                 *
 *   Description : maps fromBuffer from 420cp. or 864cp. to 1046cp.*
 *                                                                 *
 *  Return : void                                                  *
 *                                                                 *
\*******************************************************************/
void MapTo1046(unsigned char* fromBuffer,unsigned char* toBuffer,int size, CODEPAGE codepage)
{
int length=0,
    i=0;

  if (size==-1)
     length = strlen((char *)fromBuffer);
  else
     length = size;

  for(i=0 ; i<(length - 1) ;i++)
  {
    toBuffer[i] = To1046[codepage] [fromBuffer[i]];
  }
}

