//+----------------------------------------------------------------------------+
//|EQFLOGOI.H                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:      Gnther Jornitz                                                |
//+----------------------------------------------------------------------------+
//|Description: Internal functions and definitions for  TM/Logos I/F           |
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
// $Revision: 1.3 $ ----------- 30 Mar 2000
// Implemented HTML special char conversion for HTML preview
//
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFLOGOI.HV_   1.5   19 Aug 1999 14:37:28   BUILD  $
 *
 * $Log:   K:\DATA\EQFLOGOI.HV_  $
 *
 *    Rev 1.5   19 Aug 1999 14:37:28   BUILD
 * added TAG_CLS_HILIT
 * added TAG_CLS_HILIT1
 *
 *    Rev 1.4   30 Jun 1999 16:02:38   BUILD
 * Added paired attribute
 *
 *    Rev 1.3   16 Dec 1998 13:15:54   BUILD
 * changed szett 217->225
 * changed Buffer size
 *
 *    Rev 1.2   07 Dec 1998 11:39:32   BUILD
 * added eszett for logos
 *
 *    Rev 1.1   15 Oct 1998 19:52:14   BUILD
 * changed conversion table for HTML tags
 *
 *    Rev 1.0   25 Aug 1998 19:06:26   BUILD
 * Initial revision.
 *
*/
//+----------------------------------------------------------------------------+


#define TAG_START                '<'
#define MEM_SEG_BEGIN            "<Segment>"       // Memory DB segment begin tag
#define MEM_SEG_BEGIN_LEN        9
#define MEM_SEG_END              "</Segment>"      // Memory DB segment end tag
#define MEM_SEG_END_LEN          10
#define MEM_SOURCE_BEGIN         "<Source>"        // Memory DB source begin tag
#define MEM_SOURCE_BEGIN_LEN     8
#define MEM_SOURCE_END           "</Source>"       // Memory DB source end tag
#define MEM_SOURCE_END_LEN       9
#define MEM_TARGET_BEGIN         "<Target>"        // Memory DB Target begin tag
#define MEM_TARGET_BEGIN_LEN     8
#define MEM_TARGET_END           "</Target>"               // Memory DB Target end tag
#define MEM_TARGET_END_LEN       9
#define MEM_CONTROL_BEGIN        "<Control>"       // Memory DB Control begin tag
#define MEM_CONTROL_BEGIN_LEN    9
#define MEM_CONTROL_END          "</Control>"      // Memory DB Control end tag
#define MEM_CONTROL_END_LEN      10
#define NTM_BEGIN_TAG            "<NTMMemoryDb>"   // new Memory DB begin tag /*@NTM*/
#define NTM_BEGIN_TAG_LEN        13
#define NTM_END_TAG              "</NTMMemoryDb>"  // new Memory DB end tag   /*@NTM*/
#define NTM_END_TAG_LEN          14

#define LGS_EXT_START            "<LGS-EXT Seq=%d>\r\n"
#define LGS_EXT_START_LEN        13
#define LGS_EXT_END              "</LGS-EXT Seq=%d>\r\n"
#define LGS_EXT_END_LEN          14

/**********************************************************************/
/* The meaning of the Logos SGML language is as follows:              */
/* LGS-INTR    : associate tag with the right word       /LGS-INTR    */
/* LGS-INTL    : associate tag with the left  word       /LGS-INTL    */
/* LGS-INTW    : treat group of words as a single word   /LGS-INTW    */
/* LGS-INTH    : hide formatting information             /LGS-INTH    */
/* LGS-HEADER  : the next sentence is a header           /LGS-HEADER  */
/* LGS-FOOTER  : the next sentence is a footer           /LGS-FOOTER  */
/* LGS-TITLE   : the next sentence is a title            /LGS-TITLE   */
/*                                                                    */
/**********************************************************************/
#define TAG_CLS_DEFAULT          "<LGS-INTL Seq=%d>%s</LGS-INTL Seq=%d>"
#define TAG_CLS_HEAD             "<LGS-TITLE Seq=%d><LGS-INTR Seq=%d>%s</LGS-INTR Seq=%d>"
#define TAG_CLS_EHEAD            "<LGS-INTL Seq=%d>%s</LGS-INTL Seq=%d></LGS-TITLE Seq=%d>"
#define TAG_CLS_PARA             "<LGS-INTR Seq=%d>%s</LGS-INTR Seq=%d>"
#define TAG_CLS_EPARA            "<LGS-INTL Seq=%d>%s</LGS-INTL Seq=%d>"
#define TAG_CLS_XTRCT            "<LGS-INTR Seq=%d>%s</LGS-INTR Seq=%d>"
#define TAG_CLS_EXTRCT           "<LGS-INTL Seq=%d>%s</LGS-INTL Seq=%d>"
#define TAG_CLS_LIST             "<LGS-INTR Seq=%d>%s</LGS-INTR Seq=%d>"
#define TAG_CLS_ELIST            "<LGS-INTL Seq=%d>%s</LGS-INTL Seq=%d>"

#define TAG_CLS_HILIT1           "<LGS-INTR Seq=%d>%s</LGS-INTR Seq=%d>"
#define TAG_CLS_EHILIT1          "<LGS-INTL Seq=%d>%s</LGS-INTL Seq=%d>"


#define TAG_CLS_HILIT            "<LGS-INTR Seq=%d,Paired=%d>%s</LGS-INTR Seq=%d>"
#define TAG_CLS_EHILIT           "<LGS-INTL Seq=%d,Paired=%d>%s</LGS-INTL Seq=%d>"


#define TAG_CLS_PROTECT          "<LGS-INTR Seq=%d>%s</LGS-INTR Seq=%d>"
#define TAG_CLS_TREF             "<LGS-INTW Seq=%d>%s</LGS-INTW Seq=%d>"
#define TAG_CLS_LREF             "<LGS-INTW Seq=%d>%s</LGS-INTW Seq=%d>"
#define TAG_CLS_ADDINF           "<LGS-INTR Seq=%d>%s</LGS-INTR Seq=%d>"
#define TAG_CLS_EADDINF          "<LGS-INTL Seq=%d>%s</LGS-INTL Seq=%d>"
#define TAG_CLS_VAR              "<LGS-INTW Seq=%d>%s</LGS-INTW Seq=%d>"


#define EQFLOGOS_IBL             "EQFLOGOS"    // our Logos conversion table

#define BUF_SIZE                 16000

#define ERROR_TM_FORMAT          7000
#define ORIG_ID                  5001

#define LMTSUB_STR               "&logos"

typedef struct _LOGOSCONV        // possible start character of tag
{
  CHAR     chInBuf[BUF_SIZE];
  CHAR     chLine[MAX_SEGMENT_SIZE];
  USHORT   usInBufRead;
  USHORT   usInBufParsed;
  CHAR     chTagTable[ MAX_FILESPEC ];          // tag table for current segment
  CHAR     chActiveTagTable[ MAX_FILESPEC ];    // active tag table
  CHAR     chSourceLang[MAX_LANG_LENGTH];       // SOURCE LANGUAGE
  CHAR     chSegment[2*MAX_SEGMENT_SIZE];
  CHAR     chConvSrc[4*MAX_SEGMENT_SIZE];
  BOOL     fShowMsg;                            // show message??
  PVOID    pLoadedTable;                        // loaded tag table
  SHORT    sLangId;                             // language id
  BYTE     chTokBuf[4 * MAX_SEGMENT_SIZE ];     // token buffer
  HMODULE  hModule;                             // user exit module handle
  PFN      pfnUserExit;                         // ptr to user exit function
  PFN      pfnCheckSegExit;                     // ptr to segment check func
  PFN      pfnWYSIWYGExit;                      // ptr to show transl.  func
  CHAR     chTokBufStartStop[MAX_SEGMENT_SIZE]; // token buffer for start/stop
  CHAR     chControl[ 512 ];                    // control string
  CHAR     chSegStart[ 512 ];
  CHAR     chOutBuf[ 6*MAX_SEGMENT_SIZE ];
  CHAR     chSubstSrc[MAX_SEGMENT_SIZE];        // substitution string
  CHAR     chSourceSeg[MAX_SEGMENT_SIZE];       // source segment buffer
  CHAR     chSub[5][256];                       // substitution buffer
  PVOID    pfnTocGotoExit;                      // ptr to table of content func.
  USHORT   usSeqID;                             // sequence ID of Logos EXT seg.
  USHORT   usHeadID;                            // sequence ID of Logos Title seg.
} LOGOSCONV, * PLOGOSCONV;


typedef struct  _CONVERTTAGS {
  char def[8];
  int  code;
} CONVERTTAGS,*PCONVERTTAGS;



typedef struct  _CONVERTTAGS1 {
  BYTE  code;
  char def[10];
} CONVERTTAGS1,*PCONVERTTAGS1;


// -------------------
// Convert Tags table
// -------------------

#define LOGOS_NUM_TAGS   63
CONVERTTAGS CONVERT[LOGOS_NUM_TAGS] =
                                    {
                                     {"AElig",146  },
                                     {"Aacute",199 },
                                     {"Acirc",200  },
                                     {"Agrave",182 },
                                     {"Aring",143  },
                                     {"Atilde",183 },
                                     {"Auml",142   },
                                     {"Ccedil",128 },
                                     {"Eacute",144 },
                                     {"Ecirc",202  },
                                     {"Egrave",201 },
                                     {"Euml",203   },
                                     {"Iacute",205 },
                                     {"Icirc",206  },
                                     {"Igrave",204 },
                                     {"Iuml",207   },
                                     {"Ntilde",165 },
                                     {"Oacute",209 },
                                     {"Ocirc",210  },
                                     {"Ograve",208 },
                                     {"Oslash",178 },
                                     {"Otilde",184 },
                                     {"Ouml",153   },
                                     {"Uacute",214 },
                                     {"Ucirc",215  },
                                     {"Ugrave",213 },
                                     {"Uuml",154   },
                                     {"aacute",160 },
                                     {"acirc",131  },
                                     {"aellig",145 },
                                     {"aelig",145 },
                                     {"agrave",133 },
                                     {"aring",134  },
                                     {"atilde",176 },
                                     {"auml",132   },
                                     {"ccedil",135 },
                                     {"copy",189  },
                                     {"eacute",130 },
                                     {"ecirc",136  },
                                     {"egrave",138 },
                                     {"eszett",225 },
                                     {"euml",137   },
                                     {"gt",62      },
                                     {"iacute",161 },
                                     {"icirc",140  },
                                     {"igrave",141 },
                                     {"iuml",139   },
                                     {"lt",60      },
                                     {"ntilde",164 },
                                     {"oacute",162 },
                                     {"ocirc",147 },
                                     {"ograve",149 },
                                     {"oslash",179 },
                                     {"otilde",177 },
                                     {"ouml",148  },
                                     {"reg",190    },
                                     {"slash",47  },
                                     {"szlig",225 },
                                     {"uacute",163 },
                                     {"ucirc",150 },
                                     {"ugrave",151 },
                                     {"uuml",129  },
                                     {"yuml",152  }
                                                         };




#define LOGOS_NUM_TAGS1  60
CONVERTTAGS1 CONVERT1[LOGOS_NUM_TAGS1] =
                                    {
                  {128,"<Ccedil>"    },
                  {129,"<uuml>"       },
                  {130,"<eacute>"    },
                  {131,"<acirc>"      },
                  {132,"<auml>"        },
                  {133,"<agrave>"    },
                  {134,"<aring>"      },
                  {135,"<ccedil>"    },
                  {136,"<ecirc>"      },
                  {137,"<euml>"        },
                  {138,"<egrave>"    },
                  {139,"<iuml>"        },
                  {140,"<icirc>"      },
                  {141,"<igrave>"    },
                  {142,"<Auml>"        },
                  {143,"<Aring>"      },
                  {144,"<Eacute>"    },
                  {145,"<aelig>"     },
                  {146,"<AElig>"      },
                  {147,"<ocirc>"     },
                  {148,"<ouml>"       },
                  {149,"<ograve>"    },
                  {150,"<ucirc>"     },
                  {151,"<ugrave>"    },
                  {152,"<yuml>"       },
                  {153,"<Ouml>"        },
                  {154,"<Uuml>"        },
                  {160,"<aacute>"    },
                  {161,"<iacute>"    },
                  {162,"<oacute>"    },
                  {163,"<uacute>"    },
                  {164,"<ntilde>"    },
                  {165,"<Ntilde>"    },
                  {176,"<atilde>"    },
                  {177,"<otilde>"    },
                  {178,"<Oslash>"    },
                  {179,"<oslash>"    },
                  {182,"<Agrave>"    },
                  {183,"<Atilde>"    },
                  {184,"<Otilde>"    },
                  {189,"<copy>"       },
                  {190,"<reg>"          },
                  {199,"<Aacute>"    },
                  {200,"<Acirc>"      },
                  {201,"<Egrave>"    },
                  {202,"<Ecirc>"      },
                  {203,"<Euml>"        },
                  {204,"<Igrave>"    },
                  {205,"<Iacute>"    },
                  {206,"<Icirc>"      },
                  {207,"<Iuml>"        },
                  {208,"<Ograve>"    },
                  {209,"<Oacute>"    },
                  {210,"<Ocirc>"      },
                  {213,"<Ugrave>"    },
                  {214,"<Uacute>"    },
                  {215,"<Ucirc>"      },
                  {225,"<szlig>"     },

                                                         };



#define LOGOS_NUM_TAGS2  60
CONVERTTAGS1 CONVERT2[LOGOS_NUM_TAGS2] =
                                    {
                  {128,"&Ccedil;"    },
                  {129,"&uuml;"       },
                  {130,"&eacute;"    },
                  {131,"&acirc;"      },
                  {132,"&auml;"        },
                  {133,"&agrave;"    },
                  {134,"&aring;"      },
                  {135,"&ccedil;"    },
                  {136,"&ecirc;"      },
                  {137,"&euml;"        },
                  {138,"&egrave;"    },
                  {139,"&iuml;"        },
                  {140,"&icirc;"      },
                  {141,"&igrave;"    },
                  {142,"&Auml;"        },
                  {143,"&Aring;"      },
                  {144,"&Eacute;"    },
                  {145,"&aelig;"     },
                  {146,"&AElig;"      },
                  {147,"&ocirc;"     },
                  {148,"&ouml;"       },
                  {149,"&ograve;"    },
                  {150,"&ucirc;"     },
                  {151,"&ugrave;"    },
                  {152,"&yuml;"       },
                  {153,"&Ouml;"        },
                  {154,"&Uuml;"        },
                  {160,"&aacute;"    },
                  {161,"&iacute;"    },
                  {162,"&oacute;"    },
                  {163,"&uacute;"    },
                  {164,"&ntilde;"    },
                  {165,"&Ntilde;"    },
                  {176,"&atilde;"    },
                  {177,"&otilde;"    },
                  {178,"&Oslash;"    },
                  {179,"&oslash;"    },
                  {182,"&Agrave;"    },
                  {183,"&Atilde;"    },
                  {184,"&Otilde;"    },
                  {189,"&copy;"       },
                  {190,"&reg;"          },
                  {199,"&Aacute;"    },
                  {200,"&Acirc;"      },
                  {201,"&Egrave;"    },
                  {202,"&Ecirc;"      },
                  {203,"&Euml;"        },
                  {204,"&Igrave;"    },
                  {205,"&Iacute;"    },
                  {206,"&Icirc;"      },
                  {207,"&Iuml;"        },
                  {208,"&Ograve;"    },
                  {209,"&Oacute;"    },
                  {210,"&Ocirc;"      },
                  {213,"&Ugrave;"    },
                  {214,"&Uacute;"    },
                  {215,"&Ucirc;"      },
                  {225,"&szlig;"     },

                                                         };




USHORT CreateGeneric( HFILE, HFILE, BOOL, BOOL );
USHORT CreateTM( HFILE, HFILE, EQF_BOOL );
USHORT CheckUnsegFormat( PSZ );
USHORT FillBuffer( HFILE, PLOGOSCONV );
USHORT ReadLine( HFILE, PLOGOSCONV );
USHORT WriteLine( HFILE, PLOGOSCONV, BOOL );
USHORT ConvertSourceSeg( HFILE, HFILE, PLOGOSCONV );
USHORT ConvertSegBack( HFILE, HFILE, PLOGOSCONV );
