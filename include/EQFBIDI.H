//+----------------------------------------------------------------------------+
//|  EQFBIDI.H                                                                 |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Description: Include file for BIDI support                                |
//+----------------------------------------------------------------------------+

/* @(#)40 1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/csd.h, cfgnls, bos410, 9428A410 8/30/93 15:01:55 */
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 */

#define False 0
#define True  1
/************************************************************************/
/************************************************************************/
/* A state indicates how the previous character is to change the following
   character, i.e. Initial means that the following character must
   be in initial shape because the preceding character does not connect to
   the left. There are two basic states Initial and Middle. All the
   other states affect the normal characters as Middle. They have special
   meaning only in certain cases.
   There are 6 state :*/

#define InitialState        'I'  /* next char is initial */
#define MiddleState         'M'  /* next char is middle */
#define LamIsoState         'L'  /* next char is middle, unless it is an alef,
                                    then it becomes an isolated lam-alef */
#define InitialLamIsoState  'O'  /* next char is initial, unless it is an alef,
                                    then it becomes an isolated lam-alef */
#define LamConnState        'C'  /* next char is middle, unless it is an alef,
                                    then it becomes a connected lam-alef */
#define InitialLamConnState 'N'  /* next char is initial, unless it is an alef,
                                    then it becomes a connected lam-alef */
#define SeenState           'S'  /* next char is middle, unless it is a
                                    SpaceGroup char, it becomes a Tail */
#define InitialSeenState    'E'  /* next char is initial, unless it is a
                                    SpaceGroup char, it becomes a Tail */
#define YehHamzaState       'Y'  /* next char is middle, unless it is a
                                    SpaceGroup char, it becomes a Hamza */

/************************************************************************/
/* All the characters are devided into groups. The group of the character
   indicates whether the character is right connected only, or connects
   on both sides or doesn't connect at all. This determines the state at
   which the character that follows it will be in. The groups also indicates
   the characters that require any sort of special handling.
   There are 8 groups that are characterized as follows :
   0 : Symbols --> These characters are neither left nor right connectors.
                   These are all the Latin characters, box characters and the
                   punctuation.
   1 : Isolated Vowels --> In Arabic, this group includes the dammatan and
             the kasratan. These are vowels that need special handling,
                   but they neither connect to the left nor to the right.
   2 : Spaces  --> These are neither left nor right connectors, but they may
                   be overridden if something needs to be expanded.
                   In Arabic : These are the space, RSP and tail.
   3 : ALefs   --> Alefs are right connectors only and need special handling
                   following a lam. This group contains all the shapes of all
                   the alefs.
   4 : R_connectors --> These are all the characters that are right connectors
                   only.
                   In Arabic: It contains characters like the reh, dal, waw,
                   all the lamalefs, and the alef maksoura.
   5 : Normal  --> This group contains all the normal characters that connect
                   both to the left and to the right. They need no special
                   handling.
   6 : Seen    --> In Arabic : This group contains all the shapes of the seen,
                   sheen, sad and dad. They connect both to the left and to
                   the right, but they need specail handling if a Space
                   character follows.
   7 : YehHamza -> This group contains all the shapes of the yeh hamza. They
                   connect both to the left and to the right, but they need
                   specail handling if a Space character follows in
                   in HOST shaping mode.
  8 : Lam      --> This group contains the lam characters. They connect to the
                   left and to the right, but they affect a following alef
                   in a special way.
  9 : Vowel    --> This group contains all the connectable vowels.
                   Vowels do not influence connectivity, but they need
                   special consideration.
  */
#define Symbols_Group    0
#define Iso_Vowels_Group 1
#define Spaces_Group     2
#define Alef_Group       3
#define R_Conn_Group     4
#define Normal_Group     5
#define Seen_Group       6
#define Yeh_Hamza_Group  7
#define Lam_Group        8
#define Vowels_Group     9

#define ISO  0
#define INIT 1
#define MID  2
#define FIN  3
/************************************************************************/
typedef unsigned char PARABCHAR [5];
/************************************************************************/
/* Codepage : IBM 1046  */
/************************************************************************/
/* This array contains the four different shapes of every character,
   namely isolated, initial, middle and final, and the group of the
   character. */
static PARABCHAR CHRGRP [128] = {

/*  ISO  INIT MID  FIN  GROUP   */
   {0xC5,0xC5,0x80,0x80,3},       /* char 128 --> hamza under alef */
   {0x81,0x81,0x81,0x81,0},       /* char 129 --> times */
   {0x82,0x82,0x82,0x82,0},       /* char 130 --> divided */
   {0x83,0xAF,0xAF,0x83,6},       /* char 131 --> seen */
   {0x84,0xBA,0xBA,0x84,6},       /* char 132 --> sheen */
   {0x85,0xBC,0xBC,0x85,6},       /* char 133 --> sad */
   {0x86,0xBD,0xBD,0x86,6},       /* char 134 --> dad */
   {0xEB,0xEB,0x87,0xEB,9},       /* char 135 --> fathatan */
   {0x88,0x88,0x88,0x88,0},       /* char 136 --> blank */
   {0x89,0x89,0x89,0x89,0},       /* char 137 --> box character */
   {0x8A,0x8A,0x8A,0x8A,0},       /* char 138 --> box character */
   {0x8B,0x8B,0x8B,0x8B,0},       /* char 139 --> box character */
   {0x8C,0x8C,0x8C,0x8C,0},       /* char 140 --> box character */
   {0x8D,0x8D,0x8D,0x8D,0},       /* char 141 --> box character */
   {0x8E,0x8E,0x8E,0x8E,0},       /* char 142 --> box character */
   {0x8F,0x8F,0x8F,0x8F,0},       /* char 143 --> box character */
   {0xEF,0xEF,0x90,0xEF,9},       /* char 144 --> damma */
   {0xF0,0xF0,0x91,0xF0,9},       /* char 145 --> kasra */
   {0xF1,0xF1,0x92,0xF1,9},       /* char 146 --> shadda */
   {0xF2,0xF2,0x93,0xF2,9},       /* char 147 --> soukoun */
   {0xEE,0xEE,0x94,0xEE,9},       /* char 148 --> fatha */
   {0xC6,0xA6,0xA6,0x95,7},       /* char 149 --> yeh hamza */
   {0xE9,0xE9,0x96,0x96,4},       /* char 150 --> yeh */
   {0xEA,0x97,0x97,0x98,5},       /* char 151 --> yeh nokteten */
   {0xEA,0x97,0x97,0x98,5},       /* char 152 --> yeh nokteten */
   {0xDA,0x9A,0x9B,0x99,5},       /* char 153 --> gheen */
   {0xDA,0x9A,0x9B,0x99,5},       /* char 154 --> gheen */
   {0xDA,0x9A,0x9B,0x99,5},       /* char 155 --> gheen */
   {0xF7,0xF7,0x9C,0x9C,4},       /* char 156 --> lam alef madda */
   {0xF8,0xF8,0x9D,0x9D,4},       /* char 157 --> lam alef hamza */
   {0xF9,0xF9,0x9E,0x9E,4},       /* char 158 --> lam alef hamza under */
   {0xFA,0xFA,0x9F,0x9F,4},       /* char 159 --> lam alef */
   {0xA0,0xA0,0xA0,0xA0,2},       /* char 160 --> RSP */
   {0xA1,0xA1,0xA1,0xA1,3},       /* char 161 --> special alef madda */
   {0xA2,0xA2,0xA2,0xA2,3},       /* char 162 --> special alef hamza */
   {0xA3,0xA3,0xA3,0xA3,3},       /* char 163 --> special alef hamza under */
   {0xA4,0xA4,0xA4,0xA4,0},       /* char 164 --> sign */
   {0xA5,0xA5,0xA5,0xA5,3},       /* char 165 --> special alef (of lamalef) */
   {0xC6,0xA6,0xA6,0x95,7} ,      /* char 166 --> yeh hamza */
   {0xC8,0xA7,0xA7,0xC8,5} ,      /* char 167 --> beh */
   {0xCA,0xA8,0xA8,0xCA,5},       /* char 168 --> teh */
   {0xCB,0xA9,0xA9,0xCB,5} ,      /* char 169 --> theh */
   {0xCC,0xAA,0xAA,0xCC,5} ,      /* char 170 --> geem */
   {0xCD,0xAB,0xAB,0xCD,5} ,      /* char 171 --> hah */
   {0xAC,0xAC,0xAC,0xAC,0},       /* char 172 --> comma */
   {0xAD,0xAD,0xAD,0xAD,5},       /* char 173 --> wasla */
   {0xCE,0xAE,0xAE,0xCE,5} ,      /* char 174 --> khah */
   {0xD3,0xAF,0xAF,0xD3,6} ,      /* char 175 --> seen */
   {0xB0,0xB0,0xB0,0xB0,0},       /* char 176 --> zero */
   {0xB1,0xB1,0xB1,0xB1,0},       /* char 177 --> one */
   {0xB2,0xB2,0xB2,0xB2,0},       /* char 178 --> two */
   {0xB3,0xB3,0xB3,0xB3,0},       /* char 179 --> three */
   {0xB4,0xB4,0xB4,0xB4,0},       /* char 180 --> four */
   {0xB5,0xB5,0xB5,0xB5,0},       /* char 181 --> five */
   {0xB6,0xB6,0xB6,0xB6,0},       /* char 182 --> six */
   {0xB7,0xB7,0xB7,0xB7,0},       /* char 183 --> seven */
   {0xB8,0xB8,0xB8,0xB8,0},       /* char 184 --> eight */
   {0xB9,0xB9,0xB9,0xB9,0},       /* char 185 --> nine */
   {0xD4,0xBA,0xBA,0xD4,6},       /* char 186 --> sheen */
   {0xBB,0xBB,0xBB,0xBB,0},       /* char 187 --> semi colon */
   {0xD5,0xBC,0xBC,0xD5,6},       /* char 188 --> sad */
   {0xD6,0xBD,0xBD,0xD6,6},       /* char 189 --> dad */
   {0xD9,0xC0,0xDB,0xBE,5},       /* char 190 --> ein */
   {0xBF,0xBF,0xBF,0xBF,0},       /* char 191 --> question mark */
   {0xD9,0xC0,0xDB,0xBE,5},       /* char 192 --> ein */
   {0xC1,0xC1,0xC1,0xC1,0},       /* char 193 --> hamza */
   {0xC2,0xC2,0xDC,0xDC,3},       /* char 194 --> alef madda */
   {0xC3,0xC3,0xDD,0xDD,3},       /* char 195 --> alef hamza */
   {0xC4,0xC4,0xC4,0xC4,4},       /* char 196 --> waw hamza */
   {0xC5,0xC5,0x80,0x80,3},       /* char 197 --> alef hamza under */
   {0xC6,0xA6,0xA6,0x95,7},       /* char 198 --> yeh hamza */
   {0xC7,0xC7,0xDE,0xDE,3},       /* char 199 --> alef */
   {0xC8,0xA7,0xA7,0xC8,5},       /* char 200 --> beh */
   {0xC9,0xC9,0xC9,0xC9,4} ,      /* char 201 --> teh marbouta */
   {0xCA,0xA8,0xA8,0xCA,5},       /* char 202 --> teh */
   {0xCB,0xA9,0xA9,0xCB,5},       /* char 203 --> theh */
   {0xCC,0xAA,0xAA,0xCC,5},       /* char 204 --> geem */
   {0xCD,0xAB,0xAB,0xCD,5} ,      /* char 205 --> hah */
   {0xCE,0xAE,0xAE,0xCE,5},       /* char 206 --> khah */
   {0xCF,0xCF,0xCF,0xCF,4},       /* char 207 --> dal */
   {0xD0,0xD0,0xD0,0xD0,4},       /* char 208 --> thal */
   {0xD1,0xD1,0xD1,0xD1,4},       /* char 209 --> reh */
   {0xD2,0xD2,0xD2,0xD2,4},       /* char 210 --> zeen */
   {0xD3,0xAF,0xAF,0xD3,6},       /* char 211 --> seen */
   {0xD4,0xBA,0xBA,0xD4,6},       /* char 212 --> sheen */
   {0xD5,0xBC,0xBC,0xD5,6},       /* char 213 --> sad */
   {0xD6,0xBD,0xBD,0xD6,6},       /* char 214 --> dad */
   {0xD7,0xD7,0xD7,0xD7,5},       /* char 215 --> tah */
   {0xD8,0xD8,0xD8,0xD8,5},       /* char 216 --> thah */
   {0xD9,0xC0,0xDB,0xBE,5},       /* char 217 --> ein */
   {0xDA,0x9A,0x9B,0x99,5},       /* char 218 --> ghein */
   {0xD9,0xC0,0xDB,0xBE,5},       /* char 219 --> ein */
   {0xC2,0xC2,0xDC,0xDC,3},       /* char 220 --> alef madda */
   {0xC3,0xC3,0xDD,0xDD,3},       /* char 221 --> alef hamza */
   {0xC7,0xC7,0xDE,0xDE,3},       /* char 222 --> alef */
   {0xE1,0xDF,0xDF,0xE1,5},       /* char 223 --> feh */
   {0xE0,0xE0,0xE0,0xE0,5},       /* char 224 --> wasla */
   {0xE1,0xDF,0xDF,0xE1,5},       /* char 225 --> feh */
   {0xE2,0xF3,0xF3,0xE2,5},       /* char 226 --> quaf */
   {0xE3,0xF4,0xF4,0xE3,5},       /* char 227 --> kaf */
   {0xE4,0xF5,0xF5,0xE4,8},       /* char 228 --> lam */
   {0xE5,0xFB,0xFB,0xE5,5},       /* char 229 --> meem */
   {0xE6,0xFC,0xFC,0xE6,5},       /* char 230 --> noun */
   {0xFE,0xE7,0xFD,0xFE,5},       /* char 231 --> heh */
   {0xE8,0xE8,0xE8,0xE8,4},       /* char 232 --> waw */
   {0xE9,0xE9,0x96,0x96,4},       /* char 233 --> yeh */
   {0xEA,0x97,0x97,0x98,5},       /* char 234 --> yeh nokteten */
   {0xEB,0xEB,0x87,0xEB,9},       /* char 235 --> fathatan */
   {0xEC,0xEC,0xEC,0xEC,1},       /* char 236 --> damattan */
   {0xED,0xED,0xED,0xED,1},       /* char 237 --> kassratan */
   {0xEE,0xEE,0x94,0xEE,9},       /* char 238 --> fatha */
   {0xEF,0xEF,0x90,0xEF,9},       /* char 239 --> damma */
   {0xF0,0xF0,0x91,0xF0,9},       /* char 240 --> kasra */
   {0xF1,0xF1,0x92,0xF1,9},       /* char 241 --> shadda */
   {0xF2,0xF2,0x93,0xF2,9},       /* char 242 --> soukoun */
   {0xE2,0xF3,0xF3,0xE2,5},       /* char 243 --> quaf */
   {0xE3,0xF4,0xF4,0xE3,5},       /* char 244 --> kaf */
   {0xE4,0xF5,0xF5,0xE4,8},       /* char 245 --> lam */
   {0xF6,0xF6,0xF6,0xF6,2},       /* char 246 --> tail */
   {0xF7,0xF7,0x9C,0x9C,3},       /* char 247 --> lam alef madda */
   {0xF8,0xF8,0x9D,0x9D,3},       /* char 248 --> lam alef hamza */
   {0xF9,0xF9,0x9E,0x9E,3},       /* char 249 --> lam alef hamza under */
   {0xFA,0xFA,0x9F,0x9F,3},       /* char 250 --> lam alef */
   {0xE5,0xFB,0xFB,0xE5,5},       /* char 251 --> meem */
   {0xE6,0xFC,0xFC,0xE6,5},       /* char 252 --> noun */
   {0xFE,0xE7,0xFD,0xFE,5},       /* char 253 --> heh */
   {0xFE,0xE7,0xFD,0xFE,5},       /* char 254 --> heh */
   {0xFF,0xFF,0xFF,0xFF,0}        /* char 255 --> blank */
};
/************************************************************************/
/* some special characters */
#define   START_NATIONAL       0x80
//#define   SPACE                0x20

/* Arabic codepoints */
#define   A_RSP                  0xa0
#define   A_TAIL                 0xF6
#define   A_YEH_FINAL            0x96
#define   A_YEH_ISOLATED         0xE9
#define   A_HAMZA                0xC1
#define   A_YEH_HAMZA_FINAL      0x95
#define   A_YEH_HAMZA_ISOLATED   0xC6
#define   A_YEH_HAMZA_INITIAL    0xA6
#define   A_HEH_INITIAL          0xE7
#define   A_HEH_ISOLATED         0xFE
#define   A_INITIAL_SEEN         0xAF
#define   A_INITIAL_SHEEN        0xBA
#define   A_INITIAL_SAD          0xBC
#define   A_INITIAL_DAD          0xBD
#define   A_THREEQUARTER_SEEN    0x83
#define   A_THREEQUARTER_SHEEN   0x84
#define   A_THREEQUARTER_SAD     0x85
#define   A_THREEQUARTER_DAD     0x86
#define   A_ONECELL_SEEN         0xD3
#define   A_ONECELL_SHEEN        0xD4
#define   A_ONECELL_SAD          0xD5
#define   A_ONECELL_DAD          0xD6
#define   A_ALEF_FINAL           0xDE
#define   A_ALEF_ISOLATED        0xC7
#define   A_ALEF_SPECIAL         0xA5
#define   A_ALEF_HAMZA_FINAL     0xDD
#define   A_ALEF_HAMZA_ISOLATED  0xC3
#define   A_ALEF_HAMZA_SPECIAL   0xA2
#define   A_ALEF_MADDA_FINAL     0xDC
#define   A_ALEF_MADDA_ISOLATED  0xC2
#define   A_ALEF_MADDA_SPECIAL   0xA1
#define   A_ALEF_HAMZA_UNDER_FINAL 0x80
#define   A_ALEF_HAMZA_UNDER_ISOLATED 0xC5
#define   A_ALEF_HAMZA_UNDER_SPECIAL 0xA3
#define   A_LAM_ALEF_ISOLATED    0xFA
#define   A_LAM_ALEF_CONNECTED   0x9F
#define   A_LAM_ALEF_HAMZA_ISOLATED 0xF8
#define   A_LAM_ALEF_HAMZA_CONNECTED 0x9D
#define   A_LAM_ALEF_MADDA_ISOLATED  0xF7
#define   A_LAM_ALEF_MADDA_CONNECTED 0x9C
#define   A_LAM_ALEF_HAMZA_UNDER_ISOLATED 0xF9
#define   A_LAM_ALEF_HAMZA_UNDER_CONNECTED 0x9E
#define   A_LAM_ISOLATED         0xE4
#define   A_LAM_CONNECTED        0xF5

/************************************************************************/
/* function prototypes */
int Group(unsigned char ch);
unsigned char IsoFinalShape(unsigned char ch, unsigned char prev, int OSflag);
unsigned char YehFinal(unsigned char ch, unsigned char prev);
unsigned char InitialShape(unsigned char ch, char state);
unsigned char InitMidShape(unsigned char ch, unsigned char prev);
unsigned char ThreeQuarterSeen(unsigned char ch);
unsigned char SpecialAlef(unsigned char alef);
unsigned char IsoLamAlef(unsigned char alef);
unsigned char ConnLamAlef(unsigned char alef);
void reset_alefs(unsigned char *ch);
void reset_tail(unsigned char *ch);
int Vowel(unsigned char ch);
void csd_special (int csd, unsigned char *current, unsigned char *next);
void csd_engine(unsigned char *current,  /* current new character */
        unsigned char *prec1,    /* first preceding character */
        unsigned char *prec2,    /* second preceding character */
        unsigned char *prec3,    /* third preceding character */
        char *state,       /* the current state */
        int OSflag,              /* in Arabic, AIX or HOST shaping mode*/
        int Onecell);             /* in AIX, seen on onecell or two cells */


#define TRUE 1
#define FALSE 0
#define LTR 0
#define RTL 1
#define ITIL    11
#define ITCOND  12
#define LINKR    1
#define LINKL    2
#define IRRELEVANT 4
#define LAMTYPE   16
#define ALEFTYPE  32
#define  THAI    1
#define  VN        2
#define  BI      3

enum DEFINED_CODEPAGES{CP_420,  /* MF/AS 400 Arabic*/
             CP_864,  /* OS/2 Arabic*/
             CP_1046, /* AIX Arabic*/
             CP_1089, /* AIX Arabic = ISO 8859-6*/
             CP_1256, /* Windows Arabic*/
             CP_424,  /* MF/AS 400 Hebrew*/
             CP_862,  /* OS/2 Hebrew*/
             CP_856,  /* AIX Hebrew*/
             CP_916,  /* AIX Hebrew = ISO 8859-8*/
             CP_1255  /* Windows Hebrew*/
};

typedef enum DEFINED_CODEPAGES CODEPAGE;
/*typedef unsigned short UNI_CHAR;*/
/*typedef UNI_CHAR *     PUNI_CHAR;*/
typedef unsigned short CHTYPE;

typedef struct
{
  int orient_in;       /* hazem */
  int orient_out;      /* hazem */
  int order_flag;
  int num_flag;
  int compc;
  int txtShp_flag;
  int formShp;         /* hazem */
  int boc_flag;        /* hazem */
  int size;
  int size_out;
  int flip_flag;
  int swapFlag;
  int symmetric;
  unsigned short texttype_in;
  unsigned short texttype_out;
  char * buffer_in;       /* hazem */
  char * buffer_out;      /* hazem */
  unsigned char *A_level;    /* hazem */
  unsigned int *SrcToTrgMap;
  unsigned int *TrgToSrcMap;
  CODEPAGE codepage;
  int Strongchar;
} PARAMRec;
typedef PARAMRec *PPARAMRec;

typedef struct
{
  int ix;
  int basLev;
  int outLev;
  int curLev;
  int addLev;
  int impLev;
  int impSta;
  int araLet;
  int Shaping;
  int Compac;
  int linsepPos;
  int condPos;
  unsigned short wTarget;
  CHTYPE xType;
} CB2V;
typedef CB2V *PCB2V;

#define CH_B    0x00    /* Block separator*/
#define CH_S    0x01    /* Segment separator*/
#define CH_L    0x02    /* Left to right*/
#define CH_R    0x03    /* Right to left*/
#define CH_EN   0x04    /* European numeric*/
#define CH_AN   0x05    /* Arabic (Hindi) numeric*/
#define CH_ET   0x06    /* European digit terminator*/
#define CH_ES   0x07    /* European digit separator*/
#define CH_CS   0x08    /* Common digit separator*/
#define CH_W    0x09    /* Whitespace*/
#define CH_N    0x0a    /* Neutral*/
#define CH_BIDICTRL 0x0b    /* Bidi control character*/
#define CH_BATYPES  0x0f    /* All the Bidi attributes*/

#define CH_ARSIMLET   0x10    /* Simple Arabic letter form*/
#define CH_ARPRSLET   0x20    /* Arabic presentation letter form (B)*/
#define CH_ARLET    CH_ARSIMLET | CH_ARPRSLET
#define CH_HEBLET   0x40    /* Hebrew letter form*/
#define CH_EDIGIT   0x50    /* European digit 0-9*/
#define CH_HDIGIT   0x60    /* Hindi digit 0-9*/
#define CH_NSPACE   0x80    /* Nonspacing character*/


typedef struct {
  unsigned short  ccsid;
  CODEPAGE    codepage;
  unsigned short  string_type;
} CCSIDTable;

typedef struct {
  int typeoftext;
  int numerics;
  int orientation;
  int context;
  int shaping;
  int swapping;
} BidiAttrTable;


#define TT_VISUAL 0
#define TT_IMPLICIT 1
#define NUM_PASSTHRU 0
#define NUM_ARABIC 1
#define OR_LTR 0
#define OR_RTL 1
#define OR_CONTEXT 2
#define CON_LTR 1
#define CON_RTL 2
#define SH_BASE 1
#define SH_SHAPED 2
#define SS_OFF 0
#define SS_ON 1

CCSIDTable MainTable[] = {
/*  0x01A4, CP_420,   4,
  0x01A8, CP_424,   4,
  0x21A4, CP_420,   5,
  0x21A8, CP_424,   5,
  0x31A4, CP_420,   7,
  0xF300, CP_856,   4,
  0xF301, CP_862,   4,
  0xF302, CP_916,   4,
  0xF303, CP_424,   5,
  0xF304, CP_856,   5,
  0xF305, CP_862,   5,
  0xF306, CP_916,   5,
  0xF307, CP_1255,  5,
  0xF308, CP_1046,  5,
  0xF309, CP_864,   5,
  0xF30A, CP_1089,  5,
  0xF30B, CP_1256,  5,
  0xF30C, CP_856,   6,
  0xF30D, CP_862,   6,
  0xF30E, CP_916,   6,
  0xF30F, CP_1255,  6,
  0xF310, CP_420,   6,
  0xF311, CP_864,   6,
  0xF312, CP_1046,  6,
  0xF313, CP_1089,  6,
  0xF314, CP_1256,  6,
  0xF315, CP_424,   8,
  0xF316, CP_856,   8,
  0xF317, CP_862,   8,
  0xF318, CP_916,   8,
  0xF319, CP_420,   8,
  0xF31A, CP_420,   9,
  0xF31B, CP_424,  10,
  0xF31C, CP_856,  10,
  0xF31D, CP_862,  10,
  0xF31E, CP_916,  10,
  0xF31F, CP_1255, 10,
  0xF320, CP_424,  11,
  0xF321, CP_856,  11,
  0xF322, CP_862,  11,
  0xF323, CP_916,  11,
  0xF324, CP_1255, 11

  Revised table 29/4/98 according to revisions to ACR162:

  N.B. The table is in sequential order of CCSID  */

  0x01A4, CP_420,   4,  /*420*/
  0x01A8, CP_424,   4,  /*424*/
  0x0358, CP_856,   5,  /*856*/
  0x035E, CP_862,   10, /*862*/
  0x0360, CP_864,   5,  /*864*/
  0x0394, CP_916,   5,  /*916*/
  0x0416, CP_1046,  5,  /*1046*/
  0x0441, CP_1089,  5,  /*1089*/
  0x04E7, CP_1255,  5,  /*1255*/
  0x04E8, CP_1256,  5,  /*1256*/
  0x21A4, CP_420,   5,  /*8612*/
  0x21A8, CP_424,   6,  /*8616*/
  0x31A4, CP_420,   7,  /*12708*/
  0xF300, CP_856,   4,
  0xF301, CP_862,   4,
  0xF302, CP_916,   4,
  0xF303, CP_424,   5,
  0xF305, CP_862,   5,
  0xF30C, CP_856,   6,
  0xF30D, CP_862,   6,
  0xF30E, CP_916,   6,
  0xF30F, CP_1255,  6,
  0xF310, CP_420,   6,
  0xF311, CP_864,   6,
  0xF312, CP_1046,  6,
  0xF313, CP_1089,  6,
  0xF314, CP_1256,  6,
  0xF315, CP_424,   8,
  0xF316, CP_856,   8,
  0xF317, CP_862,   8,
  0xF318, CP_916,   8,
  0xF319, CP_420,   8,
  0xF31A, CP_420,   9,
  0xF31B, CP_424,   10,
  0xF31C, CP_856,   10,
  0xF31E, CP_916,   10,
  0xF31F, CP_1255,  10,
  0xF320, CP_424,   11,
  0xF321, CP_856,   11,
  0xF322, CP_862,   11,
  0xF323, CP_916,   11,
  0xF324, CP_1255,  11
};

BidiAttrTable stringtypeTable[12] = {
  0,  0,  0,  0,  0,  0,  /* 0*/
  0,  0,  0,  0,  0,  0,  /* 1*/
  0,  0,  0,  0,  0,  0,  /* 2*/
  0,  0,  0,  0,  0,  0,  /* 3*/
  TT_VISUAL,    NUM_ARABIC,   OR_LTR,   0,      SH_SHAPED,  SS_OFF, /* 4*/
  TT_IMPLICIT,  NUM_ARABIC,   OR_LTR,   0,      SH_BASE,  SS_ON,  /* 5*/
  TT_IMPLICIT,  NUM_ARABIC,   OR_RTL,   0,      SH_BASE,  SS_ON,  /* 6*/
  TT_VISUAL,    NUM_ARABIC,   OR_CONTEXT, 0,      SH_BASE,  SS_OFF, /* 7*/
  TT_VISUAL,    NUM_ARABIC,   OR_RTL,   0,      SH_SHAPED,  SS_OFF, /* 8*/
  TT_VISUAL,    NUM_PASSTHRU, OR_RTL,   0,      SH_SHAPED,  SS_ON,  /* 9*/
  TT_IMPLICIT,  NUM_PASSTHRU, OR_CONTEXT, CON_LTR,  SH_BASE,  SS_ON,  /* 10*/
  TT_IMPLICIT,  NUM_PASSTHRU, OR_CONTEXT, CON_RTL,  SH_BASE,  SS_ON /* 11*/
};

static unsigned short impTab[19][13] =
{
           /* B, S,  L,  R,  EN,  AN, ET, ES, CS,  W,  N, IL, Cond */
/* 0 L0 text      */  { 0, 0,  0,  3,   0,   1,  0,  0,  0,  0,  0,  0,  0},

/* 1 L0+AN        { 0, 0,  0,  3,   0,   1,  4,  2,  2,  4,  4,  2,  0},*/
/* 1 L0+AN        */  { 0, 0,  0,  3,   0,   1,  4,  4,  2,  4,  4,  2,  0},
/* 2 L0+AN+cont   */  { 0, 0,  0,  3,   0,   1,  4,  4,  4,  4,  4,  2,  1},

/* 3 L1 text      */  { 0, 0,  0,  3,   6,   1,  5,  4,  4,  4,  4,  1,  0},
/* 4 L1 cont      */ /* { 0, 0,  0,  3,0xA6,0xA1,  5,  4,  4,  4,  4,  1,  1},Haytham 19/4/98*/
            { 0, 0,  0,  3,0x66,0x61,  5,  4,  4,  4,  4,  1,  1}, /*Haytham 19/4/98*/

/* 5 L1+prefix    */  { 0, 0,  0,  3,0x66,0x41,  5,  4,  4,  4,  4,  1,  1},

/* 6 L1+EN        */  { 0, 0,  0,  3,   6,   1,  8,  7,  7,  4,  4,  2,  0},

/* 7 L1+EN+cont   */  { 0, 0,  0,  3,   6,0x41,  5,  4,  4,  4,  4,  2,  1},

/* 8 L1+EN+suffix */  { 0, 0,  0,  3,   6,   1,  5,  4,  4,  4,  4,  2,  0},
/* 9 L0 text      */  { 9, 9, 16,  9,  11,  14, 10,  9,  9,  9,  9,  0,  0},
/* 10 L0+prefix   */  { 9, 9,0xB0, 9,  11,0xAE,0x8A, 9,  9,  9,  9,  0,  1},
/* 11 L0+EN       */  { 9, 9, 16,  9,  11,  14, 13, 12, 12,  9,  9,  1,  0},
/* 12 L0+EN+cont  */  { 9, 9,0x30, 9,  11,0x2E,0x2A, 9,  9,  9,  9,  1,  1},
/* 13 L0+EN+suffix*/  { 9, 9, 16,  9,  11,  14, 10,  9,  9,  9,  9,  1,  0},

/* 14 L0+AN       */  { 9, 9, 16,  9,  11,  14, 10,  9, 15,  9,  9,  1,  0},

/* 15 L0+AN+cont  */  { 9, 9,0x30, 9,0x2B,  14,0x2A, 9,  9,  9,  9,  1,  1},
/* 16 L1 text     */  { 9, 9, 16,  9,  18,  14, 17, 17, 17, 17, 17,  1,  0},
/* 17 L1+cont     */  { 9, 9, 16,  9,  18,0x2E, 17, 17, 17, 17, 17,  1,  1},
/* 18 L1+EN       */  { 9, 9, 16,  9,  18,  14, 16, 17, 17, 17, 17,  1,  0}
};


int StrongChar(char *inputBuffer, size_t  length, CODEPAGE code_page);

CHTYPE getchtype(unsigned char ch, CODEPAGE codepage);
unsigned char mirror(unsigned char ch, CODEPAGE codepage);
unsigned char ArabicDigit(unsigned char ch, CODEPAGE codepage);
unsigned char Lamalef(unsigned char x, CODEPAGE codepage);

int CB2VIS(PPARAMRec, PUINT);
void BaseLvl(PPARAMRec, PCB2V);
void ucics(PPARAMRec, PCB2V);
void  pass1(PPARAMRec, PCB2V);
void  pass2(PPARAMRec, PCB2V);
void  pass3(PPARAMRec, PCB2V);
void  pass4(PPARAMRec, PCB2V);
void  pass5(PPARAMRec, PCB2V);
unsigned short uba_getLink(unsigned char, CODEPAGE);
int inver (char *buffer, size_t num_of_elements, CODEPAGE codepage, int symm_flag);
int  BidiShape (unsigned char *Buffer,
             size_t length_t,
             int Onecell,
             int OS_flag,
             int orient,
             int csd);
void csd_engine(unsigned char*,unsigned char*,unsigned char*,unsigned char*, char*,
    int OSflag, int);
void csd_special (int csd,unsigned char *current, unsigned char *next);
void MapFrom1046(unsigned char* fromBuffer, unsigned char* toBuffer,int size, CODEPAGE codepage);
void MapTo1046(unsigned char* fromBuffer, unsigned char* toBuffer,int size, CODEPAGE codepage);

CHTYPE chartype [10] [256]={
  /* 420 MF/AS 400 Arabic:*/
  {
/* 00 */  0,      /*<U0000>       NULL*/
/* 01 */  0,      /*<U0001>       START OF HEADING*/
/* 02 */  0,      /*<U0002>       START OF TEXT*/
/* 03 */  0,      /*<U0003>       END OF TEXT*/
/* 04 */  0,      /*<U009C> */
/* 05 */  CH_S,   /*<U0009>       HORIZONTAL TABULATION*/
/* 06 */  0,      /*<U0086> */
/* 07 */  0,      /*<U007F>       DELETE*/
/* 08 */  0,      /*<U0097> */
/* 09 */  0,      /*<U008D> */
/* 0A */  0,      /*<U008E> */
/* 0B */  0,      /*<U000B>       VERTICAL TABULATION*/
/* 0C */  0,      /*<U000C>       FORM FEED*/
/* 0D */  0,      /*<U000D>       CARRIAGE RETURN*/
/* 0E */  0,      /*<U000E>       SHIFT OUT*/
/* 0F */  0,      /*<U000F>       SHIFT IN*/
/* 10 */  0,      /*<U0010>       DATA LINK ESCAPE*/
/* 11 */  0,      /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */  0,      /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */  0,      /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */  0,      /*<U009D> */
/* 15 */  0,      /*<U0085> */
/* 16 */  0,      /*<U0008>       BACKSPACE*/
/* 17 */  0,      /*<U0087> */
/* 18 */  0,      /*<U0018>       CANCEL*/
/* 19 */  0,      /*<U0019>       END OF MEDIUM*/
/* 1A */  0,      /*<U0092> */
/* 1B */  0,      /*<U008F> */
/* 1C */  0,      /*<U001C>       FILE SEPARATOR*/
/* 1D */  0,      /*<U001D>       GROUP SEPARATOR*/
/* 1E */  0,      /*<U001E>       RECORD SEPARATOR*/
/* 1F */  0,      /*<U001F>       UNIT SEPARATOR*/
/* 20 */  0,      /*<U0080> */
/* 21 */  0,      /*<U0081> */
/* 22 */  0,      /*<U0082> */
/* 23 */  0,      /*<U0083> */
/* 24 */  0,      /*<U0084> */
/* 25 */  0,      /*<U000A>       LINE FEED*/
/* 26 */  0,      /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 27 */  0,      /*<U001B>       ESCAPE*/
/* 28 */  0,      /*<U0088> */
/* 29 */  0,      /*<U0089> */
/* 2A */  0,      /*<U008A> */
/* 2B */  0,      /*<U008B> */
/* 2C */  0,      /*<U008C> */
/* 2D */  0,      /*<U0005>       ENQUIRY*/
/* 2E */  0,      /*<U0006>       ACKNOWLEDGE*/
/* 2F */  0,      /*<U0007>       BELL*/
/* 30 */  0,      /*<U0090> */
/* 31 */  0,      /*<U0091> */
/* 32 */  0,      /*<U0016>       SYNCHRONOUS IDLE*/
/* 33 */  0,      /*<U0093> */
/* 34 */  0,      /*<U0094> */
/* 35 */  0,      /*<U0095> */
/* 36 */  0,      /*<U0096> */
/* 37 */  0,      /*<U0004>       END OF TRANSMISSION*/
/* 38 */  0,      /*<U0098> */
/* 39 */  0,      /*<U0099> */
/* 3A */  0,      /*<U009A> */
/* 3B */  0,      /*<U009B> */
/* 3C */  0,      /*<U0014>       DEVICE CONTROL FOUR*/
/* 3D */  0,      /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 3E */  0,      /*<U009E> */
/* 3F */  0,      /*<U001A>       SUBSTITUTE*/
/* 40 */  CH_W,   /*<U0020>   SPACE*/
/* 41 */  CH_W,   /*<U00A0>       NON-BREAKING SPACE*/
/* 42 */  CH_R | CH_ARPRSLET,   /*<UFE7C>       ARABIC SPACING SHADDAH*/
/* 43 */  CH_R | CH_ARPRSLET,   /*<UFE7D>       ARABIC SHADDAH ON TATWEEL*/
/* 44 */  CH_R | CH_ARSIMLET,   /*<U0640>       ARABIC TATWEEL*/
/* 45 */  CH_R,   /*<tail_c       kasseh (Arabic tail)*/
/* 46 */  CH_R | CH_ARPRSLET,   /*<UFE80>       GLYPH FOR ISOLATE ARABIC HAMZAH*/
/* 47 */  CH_R | CH_ARPRSLET,   /*<UFE81>       GLYPH FOR ISOLATE ARABIC MADDAH ON ALEF*/
/* 48 */  CH_R | CH_ARPRSLET,   /*<UFE82>       GLYPH FOR FINAL ARABIC MADDAH ON ALEF*/
/* 49 */  CH_R | CH_ARPRSLET,   /*<UFE83>       GLYPH FOR ISOLATE ARABIC HAMZAH ON ALEF*/
/* 4A */  CH_ET,    /*<U00A2>       CENT SIGN*/
/* 4B */  CH_ES,    /*<U002E>       PERIOD*/
/* 4C */  CH_N,   /*<U003C>       LESS-THAN SIGN*/
/* 4D */  CH_N,   /*<U0028>       OPENING PARENTHESIS*/
/* 4E */  CH_ET,    /*<U002B>       PLUS SIGN*/
/* 4F */  CH_N,   /*<U007C>       VERTICAL BAR*/
/* 50 */  CH_N,   /*<U0026>       AMPERSAND*/
/* 51 */  CH_R | CH_ARPRSLET,   /*<UFE84>       GLYPH FOR FINAL ARABIC HAMZAH ON ALEF*/
/* 52 */  CH_R | CH_ARPRSLET,   /*<UFE86>       GLYPH FOR FINAL ARABIC HAMZAH ON WAW*/
/* 53 */  CH_N,   /* *** UNASSIGNED*/
/* 54 */  CH_N,   /* *** UNASSIGNED*/
/* 55 */  CH_R | CH_ARPRSLET,   /*<UFE8A>       GLYPH FOR FINAL ARABIC HAMZAH ON YA*/
/* 56 */  CH_R | CH_ARPRSLET,   /*<UFE8D>       GLYPH FOR ISOLATE ARABIC ALEF*/
/* 57 */  CH_R | CH_ARPRSLET,   /*<UFE8E>       GLYPH FOR FINAL ARABIC ALEF*/
/* 58 */  CH_R | CH_ARPRSLET,   /*<UFE90>       GLYPH FOR FINAL ARABIC BAA*/
/* 59 */  CH_R | CH_ARPRSLET,   /*<UFE92>       GLYPH FOR MEDIAL ARABIC BAA*/
/* 5A */  CH_N,   /*<U0021>       EXCLAMATION MARK*/
/* 5B */  CH_ET,    /*<U0024>       DOLLAR SIGN*/
/* 5C */  CH_N,   /*<U002A>       ASTERISK*/
/* 5D */  CH_N,   /*<U0029>       CLOSING PARENTHESIS*/
/* 5E */  CH_N,   /*<U003B>       SEMICOLON*/
/* 5F */  CH_N,   /*<U00AC>       NOT SIGN*/
/* 60 */  CH_ET,    /*<U002D>       HYPHEN-MINUS*/
/* 61 */  CH_ES,    /*<U002F>       SLASH*/
/* 62 */  CH_R | CH_ARPRSLET,   /*<UFE94>       GLYPH FOR FINAL ARABIC TAA MARBUTAH*/
/* 63 */  CH_R | CH_ARPRSLET,   /*<UFE96>       GLYPH FOR FINAL ARABIC TAA*/
/* 64 */  CH_R | CH_ARPRSLET,   /*<UFE98>       GLYPH FOR MEDIAL ARABIC TAA*/
/* 65 */  CH_R | CH_ARPRSLET,   /*<UFE9A>       GLYPH FOR FINAL ARABIC THAA*/
/* 66 */  CH_R | CH_ARPRSLET,   /*<UFE9C>       GLYPH FOR MEDIAL ARABIC THAA*/
/* 67 */  CH_R | CH_ARPRSLET,   /*<UFE9E>       GLYPH FOR FINAL ARABIC JEEM*/
/* 68 */  CH_R | CH_ARPRSLET,   /*<UFEA0>       GLYPH FOR MEDIAL ARABIC JEEM*/
/* 69 */  CH_R | CH_ARPRSLET,   /*<UFEA2>       GLYPH FOR FINAL ARABIC HAA*/
/* 6A */  CH_N,   /*<U00A6>       BROKEN VERTICAL BAR*/
/* 6B */  CH_CS,    /*<U002C>       COMMA*/
/* 6C */  CH_ET,    /*<U066A>       ARABIC PERCENT SIGN*/
/* 6D */  CH_N,   /*<U005F>       SPACING UNDERSCORE*/
/* 6E */  CH_N,   /*<U003E>       GREATER-THAN SIGN*/
/* 6F */  CH_N,   /*<U003F>       QUESTION MARK*/
/* 70 */  CH_R | CH_ARPRSLET,   /*<UFEA4>       GLYPH FOR MEDIAL ARABIC HAA*/
/* 71 */  CH_R | CH_ARPRSLET,   /*<UFEA6>       GLYPH FOR FINAL ARABIC KHAA*/
/* 72 */  CH_R | CH_ARPRSLET,   /*<UFEA8>       GLYPH FOR MEDIAL ARABIC KHAA*/
/* 73 */  CH_R | CH_ARPRSLET,   /*<UFEAA>       GLYPH FOR FINAL ARABIC DAL*/
/* 74 */  CH_R | CH_ARPRSLET,   /*<UFEAC>       GLYPH FOR FINAL ARABIC THAL*/
/* 75 */  CH_R | CH_ARPRSLET,   /*<UFEAE>       GLYPH FOR FINAL ARABIC RA*/
/* 76 */  CH_R | CH_ARPRSLET,   /*<UFEB0>       GLYPH FOR FINAL ARABIC ZAIN*/
/* 77 */  CH_R | CH_ARPRSLET,   /*<UFEB2>       GLYPH FOR FINAL ARABIC SEEN*/
/* 78 */  CH_R | CH_ARPRSLET,   /*<UFEB4>       GLYPH FOR MEDIAL ARABIC SEEN*/
/* 79 */  CH_R,   /*<U060C>       ARABIC COMMA*/
/* 7A */  CH_CS,    /*<U003A>       COLON*/
/* 7B */  CH_ET,    /*<U0023>       NUMBER SIGN*/
/* 7C */  CH_N,   /*<U0040>       COMMERCIAL AT*/
/* 7D */  CH_N,   /*<U0027>       APOSTROPHE-QUOTE*/
/* 7E */  CH_N,   /*<U003D>       EQUALS SIGN*/
/* 7F */  CH_N,   /*<U0022>       QUOTATION MARK*/
/* 80 */  CH_R | CH_ARPRSLET,   /*<UFEB6>       GLYPH FOR FINAL ARABIC SHEEN*/
/* 81 */  CH_L,   /*<U0061>       LATIN SMALL LETTER A*/
/* 82 */  CH_L,   /*<U0062>       LATIN SMALL LETTER B*/
/* 83 */  CH_L,   /*<U0063>       LATIN SMALL LETTER C*/
/* 84 */  CH_L,   /*<U0064>       LATIN SMALL LETTER D*/
/* 85 */  CH_L,   /*<U0065>       LATIN SMALL LETTER E*/
/* 86 */  CH_L,   /*<U0066>       LATIN SMALL LETTER F*/
/* 87 */  CH_L,   /*<U0067>       LATIN SMALL LETTER G*/
/* 88 */  CH_L,   /*<U0068>       LATIN SMALL LETTER H*/
/* 89 */  CH_L,   /*<U0069>       LATIN SMALL LETTER I*/
/* 8A */  CH_R | CH_ARPRSLET,   /*<UFEB8>       GLYPH FOR MEDIAL ARABIC SHEEN*/
/* 8B */  CH_R | CH_ARPRSLET,   /*<UFEBA>       GLYPH FOR FINAL ARABIC SAD*/
/* 8C */  CH_R | CH_ARPRSLET,   /*<UFEBC>       GLYPH FOR MEDIAL ARABIC SAD*/
/* 8D */  CH_R | CH_ARPRSLET,   /*<UFEBE>       GLYPH FOR FINAL ARABIC DAD*/
/* 8E */  CH_R | CH_ARPRSLET,   /*<UFEC0>       GLYPH FOR MEDIAL ARABIC DAD*/
/* 8F */  CH_R | CH_ARPRSLET,   /*<UFEC2>       GLYPH FOR FINAL ARABIC TAH*/
/* 90 */  CH_R | CH_ARPRSLET,   /*<UFEC6>       GLYPH FOR FINAL ARABIC DHAH*/
/* 91 */  CH_L,   /*<U006A>       LATIN SMALL LETTER J*/
/* 92 */  CH_L,   /*<U006B>       LATIN SMALL LETTER K*/
/* 93 */  CH_L,   /*<U006C>       LATIN SMALL LETTER L*/
/* 94 */  CH_L,   /*<U006D>       LATIN SMALL LETTER M*/
/* 95 */  CH_L,   /*<U006E>       LATIN SMALL LETTER N*/
/* 96 */  CH_L,   /*<U006F>       LATIN SMALL LETTER O*/
/* 97 */  CH_L,   /*<U0070>       LATIN SMALL LETTER P*/
/* 98 */  CH_L,   /*<U0071>       LATIN SMALL LETTER Q*/
/* 99 */  CH_L,   /*<U0072>       LATIN SMALL LETTER R*/
/* 9A */  CH_R | CH_ARPRSLET,   /*<UFEC9>       GLYPH FOR ISOLATE ARABIC AIN*/
/* 9B */  CH_R | CH_ARPRSLET,   /*<UFECA>       GLYPH FOR FINAL ARABIC AIN*/
/* 9C */  CH_R | CH_ARPRSLET,   /*<UFECB>       GLYPH FOR INITIAL ARABIC AIN*/
/* 9D */  CH_R | CH_ARPRSLET,   /*<UFECC>       GLYPH FOR MEDIAL ARABIC AIN*/
/* 9E */  CH_R | CH_ARPRSLET,   /*<UFECD>       GLYPH FOR ISOLATE ARABIC GHAIN*/
/* 9F */  CH_R | CH_ARPRSLET,   /*<UFECE>       GLYPH FOR FINAL ARABIC GHAIN*/
/* A0 */  CH_R | CH_ARPRSLET,   /*<UFECF>       GLYPH FOR INITIAL ARABIC GHAIN*/
/* A1 */  CH_N,   /*<U00F7>       DIVISION SIGN*/
/* A2 */  CH_L,   /*<U0073>       LATIN SMALL LETTER S*/
/* A3 */  CH_L,   /*<U0074>       LATIN SMALL LETTER T*/
/* A4 */  CH_L,   /*<U0075>       LATIN SMALL LETTER U*/
/* A5 */  CH_L,   /*<U0076>       LATIN SMALL LETTER V*/
/* A6 */  CH_L,   /*<U0077>       LATIN SMALL LETTER W*/
/* A7 */  CH_L,   /*<U0078>       LATIN SMALL LETTER X*/
/* A8 */  CH_L,   /*<U0079>       LATIN SMALL LETTER Y*/
/* A9 */  CH_L,   /*<U007A>       LATIN SMALL LETTER Z*/
/* AA */  CH_R | CH_ARPRSLET,   /*<UFED0>       GLYPH FOR MEDIAL ARABIC GHAIN*/
/* AB */  CH_R | CH_ARPRSLET,   /*<UFED2>       GLYPH FOR FINAL ARABIC FA*/
/* AC */  CH_R | CH_ARPRSLET,   /*<UFED4>       GLYPH FOR MEDIAL ARABIC FA*/
/* AD */  CH_R | CH_ARPRSLET,   /*<UFED6>       GLYPH FOR FINAL ARABIC QAF*/
/* AE */  CH_R | CH_ARPRSLET,   /*<UFED7>       GLYPH FOR MEDIAL ARABIC QAF*/
/* AF */  CH_R | CH_ARPRSLET,   /*<UFEDA>       GLYPH FOR FINAL ARABIC CAF*/
/* B0 */  CH_R | CH_ARPRSLET,   /*<UFEDC>       GLYPH FOR MEDIAL ARABIC CAF*/
/* B1 */  CH_R | CH_ARPRSLET,   /*<UFEDE>       GLYPH FOR FINAL ARABIC LAM*/
/* B2 */  CH_R | CH_ARPRSLET,   /*<UFEF5>       GLYPH FOR ISOLATE ARABIC MADDAH ON LIGATURE LAM ALEF*/
/* B3 */  CH_R | CH_ARPRSLET,   /*<UFEF6>       GLYPH FOR FINAL ARABIC MADDAH ON LIGATURE LAM ALEF*/
/* B4 */  CH_R | CH_ARPRSLET,   /*<UFEF7>       GLYPH FOR ISOLATE ARABIC HAMZAH ON LIGATURE LAM ALEF*/
/* B5 */  CH_R | CH_ARPRSLET,   /*<UFEF8>       GLYPH FOR FINAL ARABIC HAMZAH ON LIGATURE LAM ALEF*/
/* B6 */  CH_N,   /* *** UNASSIGNED*/
/* B7 */  CH_N,   /* *** UNASSIGNED*/
/* B8 */  CH_R | CH_ARPRSLET,   /*<UFEFB>       GLYPH FOR ISOLATE ARABIC LIGATURE LAM ALEF*/
/* B9 */  CH_R | CH_ARPRSLET,   /*<UFEFC>       GLYPH FOR FINAL ARABIC LIGATURE LAM ALEF*/
/* BA */  CH_R | CH_ARPRSLET,   /*<UFEE0>       GLYPH FOR MEDIAL ARABIC LAM*/
/* BB */  CH_R | CH_ARPRSLET,   /*<UFEE2>       GLYPH FOR FINAL ARABIC MEEM*/
/* BC */  CH_R | CH_ARPRSLET,   /*<UFEE4>       GLYPH FOR MEDIAL ARABIC MEEM*/
/* BD */  CH_R | CH_ARPRSLET,   /*<UFEE6>       GLYPH FOR FINAL ARABIC NOON*/
/* BE */  CH_R | CH_ARPRSLET,   /*<UFEE8>       GLYPH FOR MEDIAL ARABIC NOON*/
/* BF */  CH_R | CH_ARPRSLET,   /*<UFEEA>       GLYPH FOR FINAL ARABIC HA*/
/* C0 */  CH_R | CH_ARSIMLET,     /*<U061B>       ARABIC SEMICOLON*/
/* C1 */  CH_L,   /*<U0041>       LATIN CAPITAL LETTER A*/
/* C2 */  CH_L,   /*<U0042>       LATIN CAPITAL LETTER B*/
/* C3 */  CH_L,   /*<U0043>       LATIN CAPITAL LETTER C*/
/* C4 */  CH_L,   /*<U0044>       LATIN CAPITAL LETTER D*/
/* C5 */  CH_L,   /*<U0045>       LATIN CAPITAL LETTER E*/
/* C6 */  CH_L,   /*<U0046>       LATIN CAPITAL LETTER F*/
/* C7 */  CH_L,   /*<U0047>       LATIN CAPITAL LETTER G*/
/* C8 */  CH_L,   /*<U0048>       LATIN CAPITAL LETTER H*/
/* C9 */  CH_L,   /*<U0049>       LATIN CAPITAL LETTER I*/
/* CA */  CH_N,   /*<U00AD>       SOFT HYPHEN*/
/* CB */  CH_R | CH_ARPRSLET,   /*<UFEEB>       GLYPH FOR INITIAL ARABIC HA*/
/* CC */  CH_N,   /* *** UNASSIGNED*/
/* CD */  CH_R | CH_ARPRSLET,   /*<UFEEC>       GLYPH FOR MEDIAL ARABIC HA*/
/* CE */  CH_N,   /* *** UNASSIGNED*/
/* CF */  CH_R | CH_ARPRSLET,   /*<UFEEE>       GLYPH FOR FINAL ARABIC WAW*/
/* D0 */  CH_R,   /*<U061F>       ARABIC QUESTION MARK*/
/* D1 */  CH_L,   /*<U004A>       LATIN CAPITAL LETTER J*/
/* D2 */  CH_L,   /*<U004B>       LATIN CAPITAL LETTER K*/
/* D3 */  CH_L,   /*<U004C>       LATIN CAPITAL LETTER L*/
/* D4 */  CH_L,   /*<U004D>       LATIN CAPITAL LETTER M*/
/* D5 */  CH_L,   /*<U004E>       LATIN CAPITAL LETTER N*/
/* D6 */  CH_L,   /*<U004F>       LATIN CAPITAL LETTER O*/
/* D7 */  CH_L,   /*<U0050>       LATIN CAPITAL LETTER P*/
/* D8 */  CH_L,   /*<U0051>       LATIN CAPITAL LETTER Q*/
/* D9 */  CH_L,   /*<U0052>       LATIN CAPITAL LETTER R*/
/* DA */  CH_R | CH_ARPRSLET,   /*<UFEEF>       GLYPH FOR ISOLATE ARABIC ALEF MAQSURAH*/
/* DB */  CH_R | CH_ARPRSLET,   /*<UFEF0>       GLYPH FOR FINAL ARABIC ALEF MAQSURAH*/
/* DC */  CH_R | CH_ARPRSLET,   /*<UFEF1>       GLYPH FOR ISOLATE ARABIC YA*/
/* DD */  CH_R | CH_ARPRSLET,   /*<UFEF2>       GLYPH FOR FINAL ARABIC YA*/
/* DE */  CH_R | CH_ARPRSLET,   /*<UFEF4>       GLYPH FOR MEDIAL ARABIC YA*/
/* DF */  CH_AN | CH_HDIGIT,    /*<U0660>       ARABIC-INDIC DIGIT ZERO*/
/* E0 */  CH_N,   /*<U00D7>       MULTIPLICATION SIGN*/
/* E1 */  CH_ES,    /*<U2007>       FIGURE SPACE*/
/* E2 */  CH_L,   /*<U0053>       LATIN CAPITAL LETTER S*/
/* E3 */  CH_L,   /*<U0054>       LATIN CAPITAL LETTER T*/
/* E4 */  CH_L,   /*<U0055>       LATIN CAPITAL LETTER U*/
/* E5 */  CH_L,   /*<U0056>       LATIN CAPITAL LETTER V*/
/* E6 */  CH_L,   /*<U0057>       LATIN CAPITAL LETTER W*/
/* E7 */  CH_L,   /*<U0058>       LATIN CAPITAL LETTER X*/
/* E8 */  CH_L,   /*<U0059>       LATIN CAPITAL LETTER Y*/
/* E9 */  CH_L,   /*<U005A>       LATIN CAPITAL LETTER Z*/
/* EA */  CH_AN | CH_HDIGIT,    /*<U0661>       ARABIC-INDIC DIGIT ONE*/
/* EB */  CH_AN | CH_HDIGIT,    /*<U0662>       ARABIC-INDIC DIGIT TWO*/
/* EC */  CH_N,     /* *** UNASSIGNED*/
/* ED */  CH_AN | CH_HDIGIT,    /*<U0663>       ARABIC-INDIC DIGIT THREE*/
/* EE */  CH_AN | CH_HDIGIT,    /*<U0664>       ARABIC-INDIC DIGIT FOUR*/
/* EF */  CH_AN | CH_HDIGIT,    /*<U0665>       ARABIC-INDIC DIGIT FIVE*/
/* F0 */  CH_EN | CH_EDIGIT,    /*<U0030>       DIGIT ZERO*/
/* F1 */  CH_EN | CH_EDIGIT,    /*<U0031>       DIGIT ONE*/
/* F2 */  CH_EN | CH_EDIGIT,    /*<U0032>       DIGIT TWO*/
/* F3 */  CH_EN | CH_EDIGIT,    /*<U0033>       DIGIT THREE*/
/* F4 */  CH_EN | CH_EDIGIT,    /*<U0034>       DIGIT FOUR*/
/* F5 */  CH_EN | CH_EDIGIT,    /*<U0035>       DIGIT FIVE*/
/* F6 */  CH_EN | CH_EDIGIT,    /*<U0036>       DIGIT SIX*/
/* F7 */  CH_EN | CH_EDIGIT,    /*<U0037>       DIGIT SEVEN*/
/* F8 */  CH_EN | CH_EDIGIT,    /*<U0038>       DIGIT EIGHT*/
/* F9 */  CH_EN | CH_EDIGIT,    /*<U0039>       DIGIT NINE*/
/* FA */  CH_N,   /* *** UNASSIGNED*/
/* FB */  CH_AN | CH_HDIGIT,    /*<U0666>       ARABIC-INDIC DIGIT SIX*/
/* FC */  CH_AN | CH_HDIGIT,    /*<U0667>       ARABIC-INDIC DIGIT SEVEN*/
/* FD */  CH_AN | CH_HDIGIT,    /*<U0668>       ARABIC-INDIC DIGIT EIGHT*/
/* FE */  CH_AN | CH_HDIGIT,    /*<U0669>       ARABIC-INDIC DIGIT NINE*/
/* FF */  CH_N    /*<U009F>       EIGHT ONES*/
  },
   /* 864 OS/2 Arabic*/
  {
/* 00 */  CH_N,   /*<U0000>       NULL*/
/* 01 */  CH_N,   /*<U0001>       START OF HEADING*/
/* 02 */  CH_N,   /*<U0002>       START OF TEXT*/
/* 03 */  CH_N,   /*<U0003>       END OF TEXT*/
/* 04 */  CH_N,   /*<U0004>       END OF TRANSMISSION*/
/* 05 */  CH_N,   /*<U0005>       ENQUIRY*/
/* 06 */  CH_N,   /*<U0006>       ACKNOWLEDGE*/
/* 07 */  CH_N,   /*<U0007>       BELL*/
/* 08 */  CH_N,   /*<U0008>       BACKSPACE*/
/* 09 */  CH_S,   /*<U0009>       HORIZONTAL TABULATION*/
/* 0A */  CH_N,   /*<U000A>       LINE FEED*/
/* 0B */  CH_N,   /*<U000B>       VERTICAL TABULATION*/
/* 0C */  CH_N,   /*<U000C>       FORM FEED*/
/* 0D */  CH_N,   /*<U000D>       CARRIAGE RETURN*/
/* 0E */  CH_N,   /*<U000E>       SHIFT OUT*/
/* 0F */  CH_N,   /*<U000F>       SHIFT IN*/
/* 10 */  CH_N,   /*<U0010>       DATA LINK ESCAPE*/
/* 11 */  CH_N,   /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */  CH_N,   /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */  CH_N,   /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */  CH_N,   /*<U0014>       DEVICE CONTROL FOUR*/
/* 15 */  CH_N,   /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 16 */  CH_N,   /*<U0016>       SYNCHRONOUS IDLE*/
/* 17 */  CH_N,   /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 18 */  CH_N,   /*<U0018>       CANCEL*/
/* 19 */  CH_N,   /*<U0019>       END OF MEDIUM*/
/* 1A */  CH_N,   /*<U001C>       FILE SEPARATOR*/
/* 1B */  CH_N,   /*<U001B>       ESCAPE*/
/* 1C */  CH_N,   /*<U007F>       DELETE*/
/* 1D */  CH_N,   /*<U001D>       GROUP SEPARATOR*/
/* 1E */  CH_N,   /*<U001E>       RECORD SEPARATOR*/
/* 1F */  CH_N,   /*<U001F>       UNIT SEPARATOR*/
/* 20 */  CH_W,   /*<U0020>              SPACE*/
/* 21 */  CH_N,   /*<U0021>              EXCLAMATION MARK*/
/* 22 */  CH_N,   /*<U0022>              QUOTATION MARK*/
/* 23 */  CH_ET,    /*<U0023>              NUMBER SIGN*/
/* 24 */  CH_ET,    /*<U0024>              DOLLAR SIGN*/
/* 25 */  CH_ET,    /*<U066A>              ARABIC PERCENT SIGN*/
/* 26 */  CH_N,   /*<U0026>              AMPERSAND*/
/* 27 */  CH_N,   /*<U0027>              APOSTROPHE*/
/* 28 */  CH_N,   /*<U0028>              LEFT PARENTHESIS*/
/* 29 */  CH_N,   /*<U0029>              RIGHT PARENTHESIS*/
/* 2A */  CH_N,   /*<U002A>    SM040000  ASTERISK*/
/* 2B */  CH_ET,    /*<U002B>              PLUS SIGN*/
/* 2C */  CH_CS,    /*<U002C>              COMMA*/
/* 2D */  CH_ET,    /*<U002D>              HYPHEN-MINUS*/
/* 2E */  CH_ES,    /*<U002E>              FULL STOP*/
/* 2F */  CH_ES,    /*<U002F>              SOLIDUS*/
/* 30 */  CH_EN | CH_EDIGIT,    /*<U0030>              DIGIT ZERO*/
/* 31 */  CH_EN | CH_EDIGIT,    /*<U0031>              DIGIT ONE*/
/* 32 */  CH_EN | CH_EDIGIT,    /*<U0032>              DIGIT TWO*/
/* 33 */  CH_EN | CH_EDIGIT,    /*<U0033>              DIGIT THREE*/
/* 34 */  CH_EN | CH_EDIGIT,    /*<U0034>              DIGIT FOUR*/
/* 35 */  CH_EN | CH_EDIGIT,    /*<U0035>              DIGIT FIVE*/
/* 36 */  CH_EN | CH_EDIGIT,    /*<U0036>              DIGIT SIX*/
/* 37 */  CH_EN | CH_EDIGIT,    /*<U0037>              DIGIT SEVEN*/
/* 38 */  CH_EN | CH_EDIGIT,    /*<U0038>              DIGIT EIGHT*/
/* 39 */  CH_EN | CH_EDIGIT,    /*<U0039>              DIGIT NINE*/
/* 3A */  CH_CS,    /*<U003A>              COLON*/
/* 3B */  CH_N,   /*<U003B>              SEMICOLON*/
/* 3C */  CH_N,   /*<U003C>              LESS-THAN SIGN*/
/* 3D */  CH_N,   /*<U003D>              EQUALS SIGN*/
/* 3E */  CH_N,   /*<U003E>              GREATER-THAN SIGN*/
/* 3F */  CH_N,   /*<U003F>              QUESTION MARK*/
/* 40 */  CH_N,   /*<U0040>              COMMERCIAL AT*/
/* 41 */  CH_L,   /*<U0041>              LATIN CAPITAL LETTER A*/
/* 42 */  CH_L,   /*<U0042>              LATIN CAPITAL LETTER B*/
/* 43 */  CH_L,   /*<U0043>              LATIN CAPITAL LETTER C*/
/* 44 */  CH_L,   /*<U0044>              LATIN CAPITAL LETTER D*/
/* 45 */  CH_L,   /*<U0045>              LATIN CAPITAL LETTER E*/
/* 46 */  CH_L,   /*<U0046>              LATIN CAPITAL LETTER F*/
/* 47 */  CH_L,   /*<U0047>              LATIN CAPITAL LETTER G*/
/* 48 */  CH_L,   /*<U0048>              LATIN CAPITAL LETTER H*/
/* 49 */  CH_L,   /*<U0049>              LATIN CAPITAL LETTER I*/
/* 4A */  CH_L,   /*<U004A>              LATIN CAPITAL LETTER J*/
/* 4B */  CH_L,   /*<U004B>              LATIN CAPITAL LETTER K*/
/* 4C */  CH_L,   /*<U004C>              LATIN CAPITAL LETTER L*/
/* 4D */  CH_L,   /*<U004D>              LATIN CAPITAL LETTER M*/
/* 4E */  CH_L,   /*<U004E>              LATIN CAPITAL LETTER N*/
/* 4F */  CH_L,   /*<U004F>              LATIN CAPITAL LETTER O*/
/* 50 */  CH_L,   /*<U0050>              LATIN CAPITAL LETTER P*/
/* 51 */  CH_L,   /*<U0051>              LATIN CAPITAL LETTER Q*/
/* 52 */  CH_L,   /*<U0052>              LATIN CAPITAL LETTER R*/
/* 53 */  CH_L,   /*<U0053>              LATIN CAPITAL LETTER S*/
/* 54 */  CH_L,   /*<U0054>              LATIN CAPITAL LETTER T*/
/* 55 */  CH_L,   /*<U0055>              LATIN CAPITAL LETTER U*/
/* 56 */  CH_L,   /*<U0056>              LATIN CAPITAL LETTER V*/
/* 57 */  CH_L,   /*<U0057>              LATIN CAPITAL LETTER W*/
/* 58 */  CH_L,   /*<U0058>              LATIN CAPITAL LETTER X*/
/* 59 */  CH_L,   /*<U0059>              LATIN CAPITAL LETTER Y*/
/* 5A */  CH_L,   /*<U005A>              LATIN CAPITAL LETTER Z*/
/* 5B */  CH_N,   /*<U005B>              LEFT SQUARE BRACKET*/
/* 5C */  CH_N,   /*<U005C>              REVERSE SOLIDUS*/
/* 5D */  CH_N,   /*<U005D>              RIGHT SQUARE BRACKET*/
/* 5E */  CH_N,   /*<U005E>              CIRCUMFLEX ACCENT*/
/* 5F */  CH_N,   /*<U005F>              LOW LINE*/
/* 60 */  CH_N,   /*<U0060>              GRAVE ACCENT*/
/* 61 */  CH_L,   /*<U0061>              LATIN SMALL LETTER A*/
/* 62 */  CH_L,   /*<U0062>              LATIN SMALL LETTER B*/
/* 63 */  CH_L,   /*<U0063>              LATIN SMALL LETTER C*/
/* 64 */  CH_L,   /*<U0064>              LATIN SMALL LETTER D*/
/* 65 */  CH_L,   /*<U0065>              LATIN SMALL LETTER E*/
/* 66 */  CH_L,   /*<U0066>              LATIN SMALL LETTER F*/
/* 67 */  CH_L,   /*<U0067>              LATIN SMALL LETTER G*/
/* 68 */  CH_L,   /*<U0068>              LATIN SMALL LETTER H*/
/* 69 */  CH_L,   /*<U0069>              LATIN SMALL LETTER I*/
/* 6A */  CH_L,   /*<U006A>              LATIN SMALL LETTER J*/
/* 6B */  CH_L,   /*<U006B>              LATIN SMALL LETTER K*/
/* 6C */  CH_L,   /*<U006C>              LATIN SMALL LETTER L*/
/* 6D */  CH_L,   /*<U006D>              LATIN SMALL LETTER M*/
/* 6E */  CH_L,   /*<U006E>              LATIN SMALL LETTER N*/
/* 6F */  CH_L,   /*<U006F>              LATIN SMALL LETTER O*/
/* 70 */  CH_L,   /*<U0070>              LATIN SMALL LETTER P*/
/* 71 */  CH_L,   /*<U0071>              LATIN SMALL LETTER Q*/
/* 72 */  CH_L,   /*<U0072>              LATIN SMALL LETTER R*/
/* 73 */  CH_L,   /*<U0073>              LATIN SMALL LETTER S*/
/* 74 */  CH_L,   /*<U0074>              LATIN SMALL LETTER T*/
/* 75 */  CH_L,   /*<U0075>              LATIN SMALL LETTER U*/
/* 76 */  CH_L,   /*<U0076>              LATIN SMALL LETTER V*/
/* 77 */  CH_L,   /*<U0077>              LATIN SMALL LETTER W*/
/* 78 */  CH_L,   /*<U0078>              LATIN SMALL LETTER X*/
/* 79 */  CH_L,   /*<U0079>              LATIN SMALL LETTER Y*/
/* 7A */  CH_L,   /*<U007A>              LATIN SMALL LETTER Z*/
/* 7B */  CH_N,   /*<U007B>              LEFT CURLY BRACKET*/
/* 7C */  CH_N,   /*<U007C>              VERTICAL LINE*/
/* 7D */  CH_N,   /*<U007D>              RIGHT CURLY BRACKET*/
/* 7E */  CH_N,   /*<U007E>              TILDE*/
/* 7F */  CH_N,   /*<U001A>              ===*/
/* 80 */  CH_ET,    /*<U00B0>              DEGREE SIGN*/
/* 81 */  CH_N,   /*<U00B7>              MIDDLE DOT*/
/* 82 */  CH_N,   /*<U2219>              BULLET OPERATOR*/
/* 83 */  CH_N,   /*<U221A>              SQUARE ROOT*/
/* 84 */  CH_N,   /*<U2592>              MEDIUM SHADE*/
/* 85 */  CH_N,   /*<U2500>              BOX DRAWINGS LIGHT HORIZONTAL*/
/* 86 */  CH_N,   /*<U2502>              BOX DRAWINGS LIGHT VERTICAL*/
/* 87 */  CH_N,   /*<U253C>              BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL*/
/* 88 */  CH_N,   /*<U2524>              BOX DRAWINGS LIGHT VERTICAL AND LEFT*/
/* 89 */  CH_N,   /*<U252C>              BOX DRAWINGS LIGHT DOWN AND HORIZONTAL*/
/* 8A */  CH_N,   /*<U251C>              BOX DRAWINGS LIGHT VERTICAL AND RIGHT*/
/* 8B */  CH_N,   /*<U2534>              BOX DRAWINGS LIGHT UP AND HORIZONTAL*/
/* 8C */  CH_N,   /*<U2510>              BOX DRAWINGS LIGHT DOWN AND LEFT*/
/* 8D */  CH_N,   /*<U250C>              BOX DRAWINGS LIGHT DOWN AND RIGHT*/
/* 8E */  CH_N,   /*<U2514>              BOX DRAWINGS LIGHT UP AND RIGHT*/
/* 8F */  CH_N,   /*<U2518>              BOX DRAWINGS LIGHT UP AND LEFT*/
/* 90 */  CH_L,   /*<U03B2>              GREEK SMALL LETTER BETA*/
/* 91 */  CH_N,   /*<U221E>              INFINITY*/
/* 92 */  CH_L,   /*<U03C6>              GREEK SMALL LETTER PHI*/
/* 93 */  CH_ET,    /*<U00B1>              PLUS-MINUS SIGN*/
/* 94 */  CH_N,   /*<U00BD>              VULGAR FRACTION ONE HALF*/
/* 95 */  CH_N,   /*<U00BC>              VULGAR FRACTION ONE QUARTER*/
/* 96 */  CH_N,   /*<U2248>              ALMOST EQUAL TO*/
/* 97 */  CH_N,   /*<U00AB>              LEFT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* 98 */  CH_N,   /*<U00BB>              RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* 99 */  CH_R | CH_ARPRSLET,   /*<UFEF7>              ARABIC LIGATURE LAM WITH ALEF WITH HAMZA ABOVE ISOLATED FORM*/
/* 9A */  CH_R | CH_ARPRSLET,   /*<UFEF8>              ARABIC LIGATURE LAM WITH ALEF WITH HAMZA ABOVE FINAL FORM*/
/* 9B */  CH_N,   /* *** UNASSIGNED*/
/* 9C */  CH_N,   /* *** UNASSIGNED*/
/* 9D */  CH_R | CH_ARPRSLET,   /*<UFEFB>              ARABIC LIGATURE LAM WITH ALEF ISOLATED FORM*/
/* 9E */  CH_R | CH_ARPRSLET,   /*<UFEFC>              ARABIC LIGATURE LAM WITH ALEF FINAL FORM*/
/* 9F */  CH_R,   /*<tail_c              kasseh (Arabic tail)*/
/* A0 */  CH_W,   /*<U00A0>              NO-BREAK SPACE*/
/* A1 */  CH_N,   /*<U00AD>              SOFT HYPHEN*/
/* A2 */  CH_R | CH_ARPRSLET,   /*<UFE82>    FINAL     ARABIC LETTER ALEF WITH MADDA ABOVE FINAL FORM*/
/* A3 */  CH_ET,    /*<U00A3>              POUND SIGN*/
/* A4 */  CH_ET,    /*<U00A4>              CURRENCY SIGN*/
/* A5 */  CH_R | CH_ARPRSLET,   /*<UFE84>    FINAL     ARABIC LETTER ALEF WITH HAMZA ABOVE FINAL FORM*/
/* A6 */  CH_N,   /* *** UNASSIGNED*/
/* A7 */  CH_N,   /* *** UNASSIGNED*/
/* A8 */  CH_R | CH_ARPRSLET,   /*<UFE8E>    FINAL     ARABIC LETTER ALEF FINAL FORM*/
/* A9 */  CH_R | CH_ARSIMLET,   /*<U0628>    *NOMINAL  ARABIC LETTER BEH*/
/* AA */  CH_R | CH_ARSIMLET,   /*<U062A>    *NOMINAL  ARABIC LETTER TEH*/
/* AB */  CH_R | CH_ARSIMLET,   /*<U062B>    *NOMINAL  ARABIC LETTER THEH*/
/* AC */  CH_R | CH_ARSIMLET,   /*<U060C>              ARABIC COMMA*/
/* AD */  CH_R | CH_ARSIMLET,   /*<U062C>    *NOMINAL  ARABIC LETTER JEEM*/
/* AE */  CH_R | CH_ARSIMLET,   /*<U062D>    *NOMINAL  ARABIC LETTER HAH*/
/* AF */  CH_R | CH_ARSIMLET,   /*<U062E>    *NOMINAL  ARABIC LETTER KHAH*/
/* B0 */  CH_AN | CH_HDIGIT,    /*<U0660>              ARABIC-INDIC DIGIT ZERO*/
/* B1 */  CH_AN | CH_HDIGIT,    /*<U0661>              ARABIC-INDIC DIGIT ONE*/
/* B2 */  CH_AN | CH_HDIGIT,    /*<U0662>              ARABIC-INDIC DIGIT TWO*/
/* B3 */  CH_AN | CH_HDIGIT,    /*<U0663>              ARABIC-INDIC DIGIT THREE*/
/* B4 */  CH_AN | CH_HDIGIT,    /*<U0664>              ARABIC-INDIC DIGIT FOUR*/
/* B5 */  CH_AN | CH_HDIGIT,    /*<U0665>              ARABIC-INDIC DIGIT FIVE*/
/* B6 */  CH_AN | CH_HDIGIT,    /*<U0666>              ARABIC-INDIC DIGIT SIX*/
/* B7 */  CH_AN | CH_HDIGIT,    /*<U0667>              ARABIC-INDIC DIGIT SEVEN*/
/* B8 */  CH_AN | CH_HDIGIT,    /*<U0668>              ARABIC-INDIC DIGIT EIGHT*/
/* B9 */  CH_AN | CH_HDIGIT,    /*<U0669>              ARABIC-INDIC DIGIT NINE*/
/* BA */  CH_R | CH_ARPRSLET,   /*<UFED1>    ISOLATED  ARABIC LETTER FEH ISOLATED FORM*/
/* BB */  CH_R | CH_ARSIMLET,   /*<U061B>              ARABIC SEMICOLON*/
/* BC */  CH_R | CH_ARSIMLET,   /*<U0633>    *NOMINAL  ARABIC LETTER SEEN*/
/* BD */  CH_R | CH_ARSIMLET,   /*<U0634>    *NOMINAL  ARABIC LETTER SHEEN*/
/* BE */  CH_R | CH_ARSIMLET,   /*<U0635>    *NOMINAL  ARABIC LETTER SAD*/
/* BF */  CH_R | CH_ARSIMLET,   /*<U061F>              ARABIC QUESTION MARK*/
/* C0 */  CH_ET,    /*<U00A2>              CENT SIGN*/
/* C1 */  CH_R | CH_ARPRSLET,   /*<UFE80>              ARABIC LETTER HAMZA ISOLATED FORM*/
/* C2 */  CH_R | CH_ARSIMLET,   /*<U0622>    *NOMINAL  ARABIC LETTER ALEF WITH MADDA ABOVE*/
/* C3 */  CH_R | CH_ARSIMLET,   /*<U0623>    *NOMINAL  ARABIC LETTER ALEF WITH HAMZA ABOVE*/
/* C4 */  CH_R | CH_ARSIMLET,   /*<U0624>    *NOMINAL  ARABIC LETTER WAW WITH HAMZA ABOVE*/
/* C5 */  CH_R | CH_ARPRSLET,   /*<UFECA>    FINAL     ARABIC LETTER AIN FINAL FORM*/
/* C6 */  CH_R | CH_ARSIMLET,   /*<U0626>    NOMINAL   ARABIC LETTER YEH WITH HAMZA ABOVE*/
/* C7 */  CH_R | CH_ARSIMLET,   /*<U0627>    *NOMINAL  ARABIC LETTER ALEF*/
/* C8 */  CH_R | CH_ARSIMLET,   /*<U0628>    *NOMINAL  ARABIC LETTER BEH*/
/* C9 */  CH_R | CH_ARSIMLET,   /*<U0629>    *NOMINAL  ARABIC LETTER TEH MARBUTA*/
/* CA */  CH_R | CH_ARSIMLET,   /*<U062A>    *NOMINAL  ARABIC LETTER TEH*/
/* CB */  CH_R | CH_ARSIMLET,   /*<U062B>    *NOMINAL  ARABIC LETTER THEH*/
/* CC */  CH_R | CH_ARSIMLET,   /*<U062C>    *NOMINAL  ARABIC LETTER JEEM*/
/* CD */  CH_R | CH_ARSIMLET,   /*<U062D>    *NOMINAL  ARABIC LETTER HAH*/
/* CE */  CH_R | CH_ARSIMLET,   /*<U062E>    *NOMINAL  ARABIC LETTER KHAH*/
/* CF */  CH_R | CH_ARSIMLET,   /*<U062F>    *NOMINAL  ARABIC LETTER DAL*/
/* D0 */  CH_R | CH_ARSIMLET,   /*<U0630>    *NOMINAL  ARABIC LETTER THAL*/
/* D1 */  CH_R | CH_ARSIMLET,   /*<U0631>    *NOMINAL  ARABIC LETTER REH*/
/* D2 */  CH_R | CH_ARSIMLET,   /*<U0632>    *NOMINAL  ARABIC LETTER ZAIN*/
/* D3 */  CH_R | CH_ARSIMLET,   /*<U0633>    *NOMINAL  ARABIC LETTER SEEN*/
/* D4 */  CH_R | CH_ARSIMLET,   /*<U0634>    *NOMINAL  ARABIC LETTER SHEEN*/
/* D5 */  CH_R | CH_ARSIMLET,   /*<U0635>    *NOMINAL  ARABIC LETTER SAD*/
/* D6 */  CH_R | CH_ARSIMLET,   /*<U0636>    *NOMINAL  ARABIC LETTER DAD*/
/* D7 */  CH_R | CH_ARSIMLET,   /*<U0637>    *NOMINAL  ARABIC LETTER TAH*/
/* D8 */  CH_R | CH_ARSIMLET,   /*<U0638>    *NOMINAL  ARABIC LETTER ZAH*/
/* D9 */  CH_R | CH_ARSIMLET,   /*<U0639>    *NOMINAL  ARABIC LETTER AIN*/
/* DA */  CH_R | CH_ARSIMLET,   /*<U063A>    *NOMINAL  ARABIC LETTER GHAIN*/
/* DB */  CH_N,   /*<U00A6>              BROKEN BAR*/
/* DC */  CH_N,   /*<U00AC>              NOT SIGN*/
/* DD */  CH_N,   /*<U00F7>              DIVISION SIGN*/
/* DE */  CH_N,   /*<U00D7>              MULTIPLICATION SIGN*/
/* DF */  CH_R | CH_ARSIMLET,   /*<U0639>    *NOMINAL  ARABIC LETTER AIN*/
/* E0 */  CH_R | CH_ARSIMLET,   /*<U0640>              ARABIC TATWEEL*/
/* E1 */  CH_R | CH_ARPRSLET,   /*<UFED3>    INITIAL   ARABIC LETTER FEH INITIAL FORM*/
/* E2 */  CH_R | CH_ARSIMLET,   /*<U0642>    *NOMINAL  ARABIC LETTER QAF*/
/* E3 */  CH_R | CH_ARSIMLET,   /*<U0643>    *NOMINAL  ARABIC LETTER KAF*/
/* E4 */  CH_R | CH_ARSIMLET,   /*<U0644>    *NOMINAL  ARABIC LETTER LAM*/
/* E5 */  CH_R | CH_ARSIMLET,   /*<U0645>    *NOMINAL  ARABIC LETTER MEEM*/
/* E6 */  CH_R | CH_ARSIMLET,   /*<U0646>    *NOMINAL  ARABIC LETTER NOON*/
/* E7 */  CH_R | CH_ARSIMLET,   /*<U0647>    *NOMINAL  ARABIC LETTER HEH*/
/* E8 */  CH_R | CH_ARSIMLET,   /*<U0648>    *NOMINAL  ARABIC LETTER WAW*/
/* E9 */  CH_R | CH_ARSIMLET,   /*<U0649>    *NOMINAL  ARABIC LETTER ALEF MAKSURA*/
/* EA */  CH_R | CH_ARSIMLET,   /*<U064A>    *NOMINAL  ARABIC LETTER YEH*/
/* EB */  CH_R | CH_ARSIMLET,   /*<U0636>    *NOMINAL  ARABIC LETTER DAD*/
/* EC */  CH_R | CH_ARPRSLET,   /*<UFECC>    MEDIAL    ARABIC LETTER AIN MEDIAL FORM*/
/* ED */  CH_R | CH_ARPRSLET,   /*<UFECE>    FINAL     ARABIC LETTER GHAIN FINAL FORM*/
/* EE */  CH_R | CH_ARSIMLET,   /*<U063A>    *NOMINAL  ARABIC LETTER GHAIN*/
/* EF */  CH_R | CH_ARSIMLET,   /*<U0645>    *NOMINAL  ARABIC LETTER MEEM*/
/* F0 */  CH_R | CH_ARPRSLET,   /*<UFE7D>              ARABIC SHADDA MEDIAL FORM*/
/* F1 */  CH_R | CH_NSPACE,   /*<U0651>              ARABIC SHADDA*/
/* F2 */  CH_R | CH_ARSIMLET,   /*<U0646>    *NOMINAL  ARABIC LETTER NOON*/
/* F3 */  CH_R | CH_ARSIMLET,   /*<U0647>    *NOMINAL  ARABIC LETTER HEH*/
/* F4 */  CH_R | CH_ARPRSLET,   /*<UFEEC>    MEDIAL    ARABIC LETTER HEH MEDIAL FORM*/
/* F5 */  CH_R | CH_ARPRSLET,   /*<UFEF0>    FINAL     ARABIC LETTER ALEF MAKSURA FINAL FORM*/
/* F6 */  CH_R | CH_ARPRSLET,   /*<UFEF2>    FINAL     ARABIC LETTER YEH FINAL FORM*/
/* F7 */  CH_R | CH_ARPRSLET,   /*<UFECF>              ARABIC LETTER GHAIN INITIAL FORM*/
/* F8 */  CH_R | CH_ARSIMLET,   /*<U0642>    *NOMINAL  ARABIC LETTER QAF*/
/* F9 */  CH_R | CH_ARPRSLET,   /*<UFEF5>              ARABIC LIGATURE LAM WITH ALEF WITH MADDA ABOVE ISOLATED FORM*/
/* FA */  CH_R | CH_ARPRSLET,   /*<UFEF6>              ARABIC LIGATURE LAM WITH ALEF WITH MADDA ABOVE FINAL FORM*/
/* FB */  CH_R | CH_ARSIMLET,   /*<U0644>    *NOMINAL  ARABIC LETTER LAM*/
/* FC */  CH_R | CH_ARSIMLET,   /*<U0643>    *NOMINAL  ARABIC LETTER KAF*/
/* FD */  CH_R | CH_ARSIMLET,   /*<U064A>    *NOMINAL  ARABIC LETTER YEH*/
/* FE */  CH_N | CH_ARPRSLET,   /*<UFEF1>    ISOLATED  ARABIC LETTER YEH ISOLATED FORM*/
/* FF */  CH_N    /* *** UNASSIGNED*/
  },
  /*1046 AIX Arabic*/
  {
/* 00 */  CH_N,   /*<U0000>       NULL*/
/* 01 */  CH_N,   /*<U0001>       START OF HEADING*/
/* 02 */  CH_N,   /*<U0002>       START OF TEXT*/
/* 03 */  CH_N,   /*<U0003>       END OF TEXT*/
/* 04 */  CH_N,   /*<U0004>       END OF TRANSMISSION*/
/* 05 */  CH_N,   /*<U0005>       ENQUIRY*/
/* 06 */  CH_N,   /*<U0006>       ACKNOWLEDGE*/
/* 07 */  CH_N,   /*<U0007>       BELL*/
/* 08 */  CH_N,   /*<U0008>       BACKSPACE*/
/* 09 */  CH_S,   /*<U0009>       HORIZONTAL TABULATION*/
/* 0A */  CH_N,   /*<U000A>       LINE FEED*/
/* 0B */  CH_N,   /*<U000B>       VERTICAL TABULATION*/
/* 0C */  CH_N,   /*<U000C>       FORM FEED*/
/* 0D */  CH_N,   /*<U000D>       CARRIAGE RETURN*/
/* 0E */  CH_N,   /*<U000E>       SHIFT OUT*/
/* 0F */  CH_N,   /*<U000F>       SHIFT IN*/
/* 10 */  CH_N,   /*<U0010>       DATA LINK ESCAPE*/
/* 11 */  CH_N,   /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */  CH_N,   /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */  CH_N,   /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */  CH_N,   /*<U0014>       DEVICE CONTROL FOUR*/
/* 15 */  CH_N,   /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 16 */  CH_N,   /*<U0016>       SYNCHRONOUS IDLE*/
/* 17 */  CH_N,   /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 18 */  CH_N,   /*<U0018>       CANCEL*/
/* 19 */  CH_N,   /*<U0019>       END OF MEDIUM*/
/* 1A */  CH_N,   /*<U001C>       FILE SEPARATOR*/
/* 1B */  CH_N,   /*<U001B>       ESCAPE*/
/* 1C */  CH_N,   /*<U007F>       DELETE*/
/* 1D */  CH_N,   /*<U001D>       GROUP SEPARATOR*/
/* 1E */  CH_N,   /*<U001E>       RECORD SEPARATOR*/
/* 1F */  CH_N,   /*<U001F>       UNIT SEPARATOR*/
/* 20 */  CH_W,   /*<U0020>       SPACE*/
/* 21 */  CH_N,   /*<U0021>       EXCLAMATION MARK*/
/* 22 */  CH_N,   /*<U0022>       QUOTATION MARK*/
/* 23 */  CH_ET,    /*<U0023>       NUMBER SIGN*/
/* 24 */  CH_ET,    /*<U0024>       DOLLAR SIGN*/
/* 25 */  CH_ET,    /*<U066A>   ARABIC PERCENT SIGN*/
/* 26 */  CH_N,   /*<U0026>       AMPERSAND*/
/* 27 */  CH_N,   /*<U0027>       APOSTROPHE*/
/* 28 */  CH_N,   /*<U0028>       LEFT PARENTHESIS*/
/* 29 */  CH_N,   /*<U0029>       RIGHT PARENTHESIS*/
/* 2A */  CH_N,   /*<U002A>       ASTERISK*/
/* 2B */  CH_ET,    /*<U002B>       PLUS SIGN*/
/* 2C */  CH_CS,    /*<U002C>       COMMA*/
/* 2D */  CH_ET,    /*<U002D>       HYPHEN-MINUS*/
/* 2E */  CH_ES,    /*<U002E>       FULL STOP*/
/* 2F */  CH_ES,    /*<U002F>       SOLIDUS*/
/* 30 */  CH_EN | CH_EDIGIT,    /*<U0030>       DIGIT ZERO*/
/* 31 */  CH_EN | CH_EDIGIT,    /*<U0031>       DIGIT ONE*/
/* 32 */  CH_EN | CH_EDIGIT,    /*<U0032>       DIGIT TWO*/
/* 33 */  CH_EN | CH_EDIGIT,    /*<U0033>       DIGIT THREE*/
/* 34 */  CH_EN | CH_EDIGIT,    /*<U0034>       DIGIT FOUR*/
/* 35 */  CH_EN | CH_EDIGIT,    /*<U0035>       DIGIT FIVE*/
/* 36 */  CH_EN | CH_EDIGIT,    /*<U0036>       DIGIT SIX*/
/* 37 */  CH_EN | CH_EDIGIT,    /*<U0037>       DIGIT SEVEN*/
/* 38 */  CH_EN | CH_EDIGIT,    /*<U0038>       DIGIT EIGHT*/
/* 39 */  CH_EN | CH_EDIGIT,    /*<U0039>       DIGIT NINE*/
/* 3A */  CH_CS,    /*<U003A>       COLON*/
/* 3B */  CH_N,   /*<U003B>       SEMICOLON*/
/* 3C */  CH_N,   /*<U003C>       LESS-THAN SIGN*/
/* 3D */  CH_N,   /*<U003D>       EQUALS SIGN*/
/* 3E */  CH_N,   /*<U003E>       GREATER-THAN SIGN*/
/* 3F */  CH_N,   /*<U003F>       QUESTION MARK*/
/* 40 */  CH_N,   /*<U0040>       COMMERCIAL AT*/
/* 41 */  CH_L,   /*<U0041>       LATIN CAPITAL LETTER A*/
/* 42 */  CH_L,   /*<U0042>       LATIN CAPITAL LETTER B*/
/* 43 */  CH_L,   /*<U0043>       LATIN CAPITAL LETTER C*/
/* 44 */  CH_L,   /*<U0044>       LATIN CAPITAL LETTER D*/
/* 45 */  CH_L,   /*<U0045>       LATIN CAPITAL LETTER E*/
/* 46 */  CH_L,   /*<U0046>       LATIN CAPITAL LETTER F*/
/* 47 */  CH_L,   /*<U0047>       LATIN CAPITAL LETTER G*/
/* 48 */  CH_L,   /*<U0048>       LATIN CAPITAL LETTER H*/
/* 49 */  CH_L,   /*<U0049>       LATIN CAPITAL LETTER I*/
/* 4A */  CH_L,   /*<U004A>       LATIN CAPITAL LETTER J*/
/* 4B */  CH_L,   /*<U004B>       LATIN CAPITAL LETTER K*/
/* 4C */  CH_L,   /*<U004C>       LATIN CAPITAL LETTER L*/
/* 4D */  CH_L,   /*<U004D>       LATIN CAPITAL LETTER M*/
/* 4E */  CH_L,   /*<U004E>       LATIN CAPITAL LETTER N*/
/* 4F */  CH_L,   /*<U004F>       LATIN CAPITAL LETTER O*/
/* 50 */  CH_L,   /*<U0050>       LATIN CAPITAL LETTER P*/
/* 51 */  CH_L,   /*<U0051>       LATIN CAPITAL LETTER Q*/
/* 52 */  CH_L,   /*<U0052>       LATIN CAPITAL LETTER R*/
/* 53 */  CH_L,   /*<U0053>       LATIN CAPITAL LETTER S*/
/* 54 */  CH_L,   /*<U0054>       LATIN CAPITAL LETTER T*/
/* 55 */  CH_L,   /*<U0055>       LATIN CAPITAL LETTER U*/
/* 56 */  CH_L,   /*<U0056>       LATIN CAPITAL LETTER V*/
/* 57 */  CH_L,   /*<U0057>       LATIN CAPITAL LETTER W*/
/* 58 */  CH_L,   /*<U0058>       LATIN CAPITAL LETTER X*/
/* 59 */  CH_L,   /*<U0059>       LATIN CAPITAL LETTER Y*/
/* 5A */  CH_L,   /*<U005A>       LATIN CAPITAL LETTER Z*/
/* 5B */  CH_N,   /*<U005B>       LEFT SQUARE BRACKET*/
/* 5C */  CH_N,   /*<U005C>       REVERSE SOLIDUS*/
/* 5D */  CH_N,   /*<U005D>       RIGHT SQUARE BRACKET*/
/* 5E */  CH_N,   /*<U005E>       CIRCUMFLEX ACCENT*/
/* 5F */  CH_N,   /*<U005F>       LOW LINE*/
/* 60 */  CH_N,   /*<U0060>       GRAVE ACCENT*/
/* 61 */  CH_L,   /*<U0061>       LATIN SMALL LETTER A*/
/* 62 */  CH_L,   /*<U0062>       LATIN SMALL LETTER B*/
/* 63 */  CH_L,   /*<U0063>       LATIN SMALL LETTER C*/
/* 64 */  CH_L,   /*<U0064>       LATIN SMALL LETTER D*/
/* 65 */  CH_L,   /*<U0065>       LATIN SMALL LETTER E*/
/* 66 */  CH_L,   /*<U0066>       LATIN SMALL LETTER F*/
/* 67 */  CH_L,   /*<U0067>       LATIN SMALL LETTER G*/
/* 68 */  CH_L,   /*<U0068>       LATIN SMALL LETTER H*/
/* 69 */  CH_L,   /*<U0069>       LATIN SMALL LETTER I*/
/* 6A */  CH_L,   /*<U006A>       LATIN SMALL LETTER J*/
/* 6B */  CH_L,   /*<U006B>       LATIN SMALL LETTER K*/
/* 6C */  CH_L,   /*<U006C>       LATIN SMALL LETTER L*/
/* 6D */  CH_L,   /*<U006D>       LATIN SMALL LETTER M*/
/* 6E */  CH_L,   /*<U006E>       LATIN SMALL LETTER N*/
/* 6F */  CH_L,   /*<U006F>       LATIN SMALL LETTER O*/
/* 70 */  CH_L,   /*<U0070>       LATIN SMALL LETTER P*/
/* 71 */  CH_L,   /*<U0071>       LATIN SMALL LETTER Q*/
/* 72 */  CH_L,   /*<U0072>       LATIN SMALL LETTER R*/
/* 73 */  CH_L,   /*<U0073>       LATIN SMALL LETTER S*/
/* 74 */  CH_L,   /*<U0074>       LATIN SMALL LETTER T*/
/* 75 */  CH_L,   /*<U0075>       LATIN SMALL LETTER U*/
/* 76 */  CH_L,   /*<U0076>       LATIN SMALL LETTER V*/
/* 77 */  CH_L,   /*<U0077>       LATIN SMALL LETTER W*/
/* 78 */  CH_L,   /*<U0078>       LATIN SMALL LETTER X*/
/* 79 */  CH_L,   /*<U0079>       LATIN SMALL LETTER Y*/
/* 7A */  CH_L,   /*<U007A>       LATIN SMALL LETTER Z*/
/* 7B */  CH_N,   /*<U007B>       LEFT CURLY BRACKET*/
/* 7C */  CH_N,   /*<U007C>       VERTICAL LINE*/
/* 7D */  CH_N,   /*<U007D>       RIGHT CURLY BRACKET*/
/* 7E */  CH_N,   /*<U007E>       TILDE*/
/* 7F */  CH_N,   /*<U001A>       ===*/
/* 80 */  CH_R | CH_ARSIMLET,   /*hamza under alef*/
/* 81 */  CH_N,   /*times*/
/* 82 */  CH_N,   /*divided*/
/* 83 */  CH_R | CH_ARSIMLET,   /*seen*/
/* 84 */  CH_R | CH_ARSIMLET,   /*sheen*/
/* 85 */  CH_R | CH_ARSIMLET,   /*sad*/
/* 86 */  CH_R | CH_ARSIMLET,   /*dad*/
/* 87 */  CH_R | CH_NSPACE,   /*fathatan*/
/* 88 */  CH_N,   /*blank*/
/* 89 */  CH_N,   /*box character*/
/* 8A */  CH_N,   /*box character*/
/* 8B */  CH_N,   /*box character*/
/* 8C */  CH_N,   /*box character*/
/* 8D */  CH_N,   /*box character*/
/* 8E */  CH_N,   /*box character*/
/* 8F */  CH_N,   /*box character*/
/* 90 */  CH_R | CH_NSPACE,   /*damma*/
/* 91 */  CH_R | CH_NSPACE,   /*kasra*/
/* 92 */  CH_R | CH_NSPACE,   /*shadda*/
/* 93 */  CH_R | CH_ARSIMLET | CH_NSPACE,   /*soukoun*/
/* 94 */  CH_R | CH_NSPACE,   /*fatha*/
/* 95 */  CH_R | CH_ARSIMLET,   /*yeh hamza*/
/* 96 */  CH_R | CH_ARSIMLET,   /*yeh*/
/* 97 */  CH_R | CH_ARSIMLET,   /*yeh nokteten*/
/* 98 */  CH_R | CH_ARSIMLET,   /*yeh nokteten*/
/* 99 */  CH_R | CH_ARSIMLET,   /*gheen*/
/* 9A */  CH_R | CH_ARSIMLET,   /*gheen*/
/* 9B */  CH_R | CH_ARSIMLET,   /*gheen*/
/* 9C */  CH_R | CH_ARSIMLET,   /*lam alef madda*/
/* 9D */  CH_R | CH_ARSIMLET,   /*lam alef hamza*/
/* 9E */  CH_R | CH_ARSIMLET,   /*lam alef hamza under*/
/* 9F */  CH_R | CH_ARSIMLET,   /*lam alef*/
/* A0 */  CH_W,   /*RSP*/
/* A1 */  CH_R | CH_ARSIMLET,   /*special alef madda*/
/* A2 */  CH_R | CH_ARSIMLET,   /*special alef hamza*/
/* A3 */  CH_R | CH_ARSIMLET,   /*special alef hamza under*/
/* A4 */  CH_ET,          /*sign*/
/* A5 */  CH_R | CH_ARSIMLET,   /*special alef (of lamalef)*/
/* A6 */  CH_R | CH_ARSIMLET,   /*yeh hamza*/
/* A7 */  CH_R | CH_ARSIMLET,   /*beh*/
/* A8 */  CH_R | CH_ARSIMLET,   /*teh*/
/* A9 */  CH_R | CH_ARSIMLET,   /*theh*/
/* AA */  CH_R | CH_ARSIMLET,   /*geem*/
/* AB */  CH_R | CH_ARSIMLET,   /*hah*/
/* AC */  CH_R,   /*comma*/
/* AD */  CH_N /* | CH_ARSIMLET */,   /*wasla*/
/* AE */  CH_R | CH_ARSIMLET,   /*khah*/
/* AF */  CH_R | CH_ARSIMLET,   /*seen*/
/* B0 */  CH_AN | CH_HDIGIT,    /*zero*/
/* B1 */  CH_AN | CH_HDIGIT,    /*one*/
/* B2 */  CH_AN | CH_HDIGIT,    /*two*/
/* B3 */  CH_AN | CH_HDIGIT,    /*three*/
/* B4 */  CH_AN | CH_HDIGIT,    /*four*/
/* B5 */  CH_AN | CH_HDIGIT,    /*five*/
/* B6 */  CH_AN | CH_HDIGIT,    /*six*/
/* B7 */  CH_AN | CH_HDIGIT,    /*seven*/
/* B8 */  CH_AN | CH_HDIGIT,    /*eight*/
/* B9 */  CH_AN | CH_HDIGIT,    /*nine*/
/* BA */  CH_R | CH_ARSIMLET,   /*sheen*/
/* BB */  CH_R,   /*semi colon*/
/* BC */  CH_R | CH_ARSIMLET,   /*sad*/
/* BD */  CH_R | CH_ARSIMLET,   /*dad*/
/* BE */  CH_R | CH_ARSIMLET,   /*ein*/
/* BF */  CH_R,   /*question mark*/
/* C0 */  CH_R | CH_ARSIMLET,   /*ein*/
/* C1 */  CH_R | CH_ARSIMLET,   /*hamza*/
/* C2 */  CH_R | CH_ARSIMLET,   /*alef madda*/
/* C3 */  CH_R | CH_ARSIMLET,   /*alef hamza*/
/* C4 */  CH_R | CH_ARSIMLET,   /*waw hamza*/
/* C5 */  CH_R | CH_ARSIMLET,   /*alef hamza under*/
/* C6 */  CH_R | CH_ARSIMLET,   /*yeh hamza*/
/* C7 */  CH_R | CH_ARSIMLET,   /*alef*/
/* C8 */  CH_R | CH_ARSIMLET,   /*beh*/
/* C9 */  CH_R | CH_ARSIMLET,   /*teh marbouta*/
/* CA */  CH_R | CH_ARSIMLET,   /*teh*/
/* CB */  CH_R | CH_ARSIMLET,   /*theh*/
/* CC */  CH_R | CH_ARSIMLET,   /*geem*/
/* CD */  CH_R | CH_ARSIMLET,   /*hah*/
/* CE */  CH_R | CH_ARSIMLET,   /*khah*/
/* CF */  CH_R | CH_ARSIMLET,   /*dal*/
/* D0 */  CH_R | CH_ARSIMLET,   /*thal*/
/* D1 */  CH_R | CH_ARSIMLET,   /*reh*/
/* D2 */  CH_R | CH_ARSIMLET,   /*zeen*/
/* D3 */  CH_R | CH_ARSIMLET,   /*seen*/
/* D4 */  CH_R | CH_ARSIMLET,   /*sheen*/
/* D5 */  CH_R | CH_ARSIMLET,   /*sad*/
/* D6 */  CH_R | CH_ARSIMLET,   /*dad*/
/* D7 */  CH_R | CH_ARSIMLET,   /*tah*/
/* D8 */  CH_R | CH_ARSIMLET,   /*thah*/
/* D9 */  CH_R | CH_ARSIMLET,   /*ein*/
/* DA */  CH_R | CH_ARSIMLET,   /*ghein*/
/* DB */  CH_R | CH_ARSIMLET,   /*ein*/
/* DC */  CH_R | CH_ARSIMLET,   /*alef madda*/
/* DD */  CH_R | CH_ARSIMLET,   /*alef hamza*/
/* DE */  CH_R | CH_ARSIMLET,   /*alef*/
/* DF */  CH_R | CH_ARSIMLET,   /*feh*/
/* E0 */  CH_R | CH_ARSIMLET,   /*wasla*/
/* E1 */  CH_R | CH_ARSIMLET,   /*feh*/
/* E2 */  CH_R | CH_ARSIMLET,   /*quaf*/
/* E3 */  CH_R | CH_ARSIMLET,   /*kaf*/
/* E4 */  CH_R | CH_ARSIMLET,   /*lam*/
/* E5 */  CH_R | CH_ARSIMLET,   /*meem*/
/* E6 */  CH_R | CH_ARSIMLET,   /*noun*/
/* E7 */  CH_R | CH_ARSIMLET,   /*heh*/
/* E8 */  CH_R | CH_ARSIMLET,   /*waw*/
/* E9 */  CH_R | CH_ARSIMLET,   /*yeh*/
/* EA */  CH_R | CH_ARSIMLET,   /*yeh nokteten*/
/* EB */  CH_R | CH_NSPACE,   /*fathatan*/
/* EC */  CH_R | CH_NSPACE,   /*damattan*/
/* ED */  CH_R | CH_NSPACE,   /*kassratan*/
/* EE */  CH_R | CH_NSPACE,   /*fatha*/
/* EF */  CH_R | CH_NSPACE,   /*damma*/
/* F0 */  CH_R | CH_NSPACE,   /*kasra*/
/* F1 */  CH_R | CH_NSPACE,   /*shadda*/
/* F2 */  CH_R | CH_ARSIMLET | CH_NSPACE,   /*soukoun*/
/* F3 */  CH_R | CH_ARSIMLET,   /*quaf*/
/* F4 */  CH_R | CH_ARSIMLET,   /*kaf*/
/* F5 */  CH_R | CH_ARSIMLET,   /*lam*/
/* F6 */  CH_R | CH_ARSIMLET,   /*tail*/
/* F7 */  CH_R | CH_ARSIMLET,   /*lam alef madda*/
/* F8 */  CH_R | CH_ARSIMLET,   /*lam alef hamza*/
/* F9 */  CH_R | CH_ARSIMLET,   /*lam alef hamza under*/
/* FA */  CH_R | CH_ARSIMLET,   /*lam alef*/
/* FB */  CH_R | CH_ARSIMLET,   /*meem*/
/* FC */  CH_R | CH_ARSIMLET,   /*noun*/
/* FD */  CH_R | CH_ARSIMLET,   /*heh*/
/* FE */  CH_R | CH_ARSIMLET,   /*heh*/
/* FF */  CH_N    /* *** UNASSIGNED*/
  },
  /* 1089 AIX Arabic = ISO 8859-6*/
  {
/* 00 */  CH_N,   /*<U0000> (CC) Null*/
/* 01 */  CH_N,   /*<U0001> (CC) Start of Heading*/
/* 02 */  CH_N,   /*<U0002> (CC) Start of Text*/
/* 03 */  CH_N,   /*<U0003> (CC) End of Text*/
/* 04 */  CH_N,   /*<U0004> (CC) End of Transmission*/
/* 05 */  CH_N,   /*<U0005> (CC) Enquiry*/
/* 06 */  CH_N,   /*<U0006> (CC) Acknowledge*/
/* 07 */  CH_N,   /*<U0007> (CC) Bell*/
/* 08 */  CH_N,   /*<U0008> (CC) Backspace*/
/* 09 */  CH_S,   /*<U0009> (CC) Horizontal Tabulation*/
/* 0A */  CH_N,   /*<U000A> (CC) Line Feed*/
/* 0B */  CH_N,   /*<U000B> (CC) Vertical Tabulation*/
/* 0C */  CH_N,   /*<U000C> (CC) Form Feed*/
/* 0D */  CH_N,   /*<U000D> (CC) Carriage Return*/
/* 0E */  CH_N,   /*<U000E> (CC) Locking Shift One (Shift Out*/
/* 0F */  CH_N,   /*<U000F> (CC) Locking Shift Zero (Shift In*/
/* 10 */  CH_N,   /*<U0010> (CC) Data Link Escape*/
/* 11 */  CH_N,   /*<U0011> (CC) Device Control One*/
/* 12 */  CH_N,   /*<U0012> (CC) Device Control Two*/
/* 13 */  CH_N,   /*<U0013> (CC) Device Control Three*/
/* 14 */  CH_N,   /*<U0014> (CC) Device Control Four*/
/* 15 */  CH_N,   /*<U0015> (CC) Negative Acknowledge*/
/* 16 */  CH_N,   /*<U0016> (CC) Synchronous Idle*/
/* 17 */  CH_N,   /*<U0017> (CC) End of Transmission Block*/
/* 18 */  CH_N,   /*<U0018> (CC) Cancel*/
/* 19 */  CH_N,   /*<U0019> (CC) End of Medium*/
/* 1A */  CH_N,   /*<U001A> (CC) Substitute*/
/* 1B */  CH_N,   /*<U001B> (CC) Escape*/
/* 1C */  CH_N,   /*<U001C> (CC) Information File Separator*/
/* 1D */  CH_N,   /*<U001D> (CC) Group Separator*/
/* 1E */  CH_N,   /*<U001E> (CC) Record Separator*/
/* 1F */  CH_N,   /*<U001F> (CC) Unit Separator*/
/* 20 */  CH_W,   /*<U0020> SPACE*/
/* 21 */  CH_N,   /*<U0021> EXCLAMATION MARK*/
/* 22 */  CH_N,   /*<U0022> QUOTATION MARK*/
/* 23 */  CH_ET,    /*<U0023> NUMBER SIGN*/
/* 24 */  CH_ET,    /*<U0024> DOLLAR SIGN*/
/* 25 */  CH_ET,    /*<U0025> PERCENT SIGN*/
/* 26 */  CH_N,   /*<U0026> AMPERSAND*/
/* 27 */  CH_N,   /*<U0027> APOSTROPHE*/
/* 28 */  CH_N,   /*<U0028> LEFT PARENTHESIS*/
/* 29 */  CH_N,   /*<U0029> RIGHT PARENTHESIS*/
/* 2A */  CH_N,   /*<U002A> ASTERISK*/
/* 2B */  CH_ET,    /*<U002B> PLUS SIGN*/
/* 2C */  CH_CS,    /*<U002C> COMMA*/
/* 2D */  CH_ET,    /*<U002D> HYPHEN-MINUS*/
/* 2E */  CH_ES,    /*<U002E> FULL STOP*/
/* 2F */  CH_ES,    /*<U002F> SOLIDUS*/
/* 30 */  CH_EN | CH_EDIGIT,    /*<U0030> DIGIT ZERO*/
/* 31 */  CH_EN | CH_EDIGIT,    /*<U0031> DIGIT ONE*/
/* 32 */  CH_EN | CH_EDIGIT,    /*<U0032> DIGIT TWO*/
/* 33 */  CH_EN | CH_EDIGIT,    /*<U0033> DIGIT THREE*/
/* 34 */  CH_EN | CH_EDIGIT,    /*<U0034> DIGIT FOUR*/
/* 35 */  CH_EN | CH_EDIGIT,    /*<U0035> DIGIT FIVE*/
/* 36 */  CH_EN | CH_EDIGIT,    /*<U0036> DIGIT SIX*/
/* 37 */  CH_EN | CH_EDIGIT,    /*<U0037> DIGIT SEVEN*/
/* 38 */  CH_EN | CH_EDIGIT,    /*<U0038> DIGIT EIGHT*/
/* 39 */  CH_EN | CH_EDIGIT,    /*<U0039> DIGIT NINE*/
/* 3A */  CH_CS,    /*<U003A> COLON*/
/* 3B */  CH_N,   /*<U003B> SEMICOLON*/
/* 3C */  CH_N,   /*<U003C> LESS-THAN SIGN*/
/* 3D */  CH_N,   /*<U003D> EQUALS SIGN*/
/* 3E */  CH_N,   /*<U003E> GREATER-THAN SIGN*/
/* 3F */  CH_N,   /*<U003F> QUESTION MARK*/
/* 40 */  CH_N,   /*<U0040> COMMERCIAL AT*/
/* 41 */  CH_L,   /*<U0041> LATIN CAPITAL LETTER A*/
/* 42 */  CH_L,   /*<U0042> LATIN CAPITAL LETTER B*/
/* 43 */  CH_L,   /*<U0043> LATIN CAPITAL LETTER C*/
/* 44 */  CH_L,   /*<U0044> LATIN CAPITAL LETTER D*/
/* 45 */  CH_L,   /*<U0045> LATIN CAPITAL LETTER E*/
/* 46 */  CH_L,   /*<U0046> LATIN CAPITAL LETTER F*/
/* 47 */  CH_L,   /*<U0047> LATIN CAPITAL LETTER G*/
/* 48 */  CH_L,   /*<U0048> LATIN CAPITAL LETTER H*/
/* 49 */  CH_L,   /*<U0049> LATIN CAPITAL LETTER I*/
/* 4A */  CH_L,   /*<U004A> LATIN CAPITAL LETTER J*/
/* 4B */  CH_L,   /*<U004B> LATIN CAPITAL LETTER K*/
/* 4C */  CH_L,   /*<U004C> LATIN CAPITAL LETTER L*/
/* 4D */  CH_L,   /*<U004D> LATIN CAPITAL LETTER M*/
/* 4E */  CH_L,   /*<U004E> LATIN CAPITAL LETTER N*/
/* 4F */  CH_L,   /*<U004F> LATIN CAPITAL LETTER O*/
/* 50 */  CH_L,   /*<U0050> LATIN CAPITAL LETTER P*/
/* 51 */  CH_L,   /*<U0051> LATIN CAPITAL LETTER Q*/
/* 52 */  CH_L,   /*<U0052> LATIN CAPITAL LETTER R*/
/* 53 */  CH_L,   /*<U0053> LATIN CAPITAL LETTER S*/
/* 54 */  CH_L,   /*<U0054> LATIN CAPITAL LETTER T*/
/* 55 */  CH_L,   /*<U0055> LATIN CAPITAL LETTER U*/
/* 56 */  CH_L,   /*<U0056> LATIN CAPITAL LETTER V*/
/* 57 */  CH_L,   /*<U0057> LATIN CAPITAL LETTER W*/
/* 58 */  CH_L,   /*<U0058> LATIN CAPITAL LETTER X*/
/* 59 */  CH_L,   /*<U0059> LATIN CAPITAL LETTER Y*/
/* 5A */  CH_L,   /*<U005A> LATIN CAPITAL LETTER Z*/
/* 5B */  CH_N,   /*<U005B> LEFT SQUARE BRACKET*/
/* 5C */  CH_N,   /*<U005C> REVERSE SOLIDUS*/
/* 5D */  CH_N,   /*<U005D> RIGHT SQUARE BRACKET*/
/* 5E */  CH_N,   /*<U005E> CIRCUMFLEX ACCENT*/
/* 5F */  CH_N,   /*<U005F> LOW LINE*/
/* 60 */  CH_N,   /*<U0060> GRAVE ACCENT*/
/* 61 */  CH_L,   /*<U0061> LATIN SMALL LETTER A*/
/* 62 */  CH_L,   /*<U0062> LATIN SMALL LETTER B*/
/* 63 */  CH_L,   /*<U0063> LATIN SMALL LETTER C*/
/* 64 */  CH_L,   /*<U0064> LATIN SMALL LETTER D*/
/* 65 */  CH_L,   /*<U0065> LATIN SMALL LETTER E*/
/* 66 */  CH_L,   /*<U0066> LATIN SMALL LETTER F*/
/* 67 */  CH_L,   /*<U0067> LATIN SMALL LETTER G*/
/* 68 */  CH_L,   /*<U0068> LATIN SMALL LETTER H*/
/* 69 */  CH_L,   /*<U0069> LATIN SMALL LETTER I*/
/* 6A */  CH_L,   /*<U006A> LATIN SMALL LETTER J*/
/* 6B */  CH_L,   /*<U006B> LATIN SMALL LETTER K*/
/* 6C */  CH_L,   /*<U006C> LATIN SMALL LETTER L*/
/* 6D */  CH_L,   /*<U006D> LATIN SMALL LETTER M*/
/* 6E */  CH_L,   /*<U006E> LATIN SMALL LETTER N*/
/* 6F */  CH_L,   /*<U006F> LATIN SMALL LETTER O*/
/* 70 */  CH_L,   /*<U0070> LATIN SMALL LETTER P*/
/* 71 */  CH_L,   /*<U0071> LATIN SMALL LETTER Q*/
/* 72 */  CH_L,   /*<U0072> LATIN SMALL LETTER R*/
/* 73 */  CH_L,   /*<U0073> LATIN SMALL LETTER S*/
/* 74 */  CH_L,   /*<U0074> LATIN SMALL LETTER T*/
/* 75 */  CH_L,   /*<U0075> LATIN SMALL LETTER U*/
/* 76 */  CH_L,   /*<U0076> LATIN SMALL LETTER V*/
/* 77 */  CH_L,   /*<U0077> LATIN SMALL LETTER W*/
/* 78 */  CH_L,   /*<U0078> LATIN SMALL LETTER X*/
/* 79 */  CH_L,   /*<U0079> LATIN SMALL LETTER Y*/
/* 7A */  CH_L,   /*<U007A> LATIN SMALL LETTER Z*/
/* 7B */  CH_N,   /*<U007B> LEFT CURLY BRACKET*/
/* 7C */  CH_N,   /*<U007C> VERTICAL LINE*/
/* 7D */  CH_N,   /*<U007D> RIGHT CURLY BRACKET*/
/* 7E */  CH_N,   /*<U007E> TILDE*/
/* 7F */  CH_N,   /*(CC) Delete*/
/* 80 */  CH_N,     /* *** UNASSIGNED*/
/* 81 */  CH_N,     /* *** UNASSIGNED*/
/* 82 */  CH_N,     /* *** UNASSIGNED*/
/* 83 */  CH_N,     /* *** UNASSIGNED*/
/* 84 */  CH_N,     /* *** UNASSIGNED*/
/* 85 */  CH_N,     /* *** UNASSIGNED*/
/* 86 */  CH_N,     /* *** UNASSIGNED*/
/* 87 */  CH_N,     /* *** UNASSIGNED*/
/* 88 */  CH_N,     /* *** UNASSIGNED*/
/* 89 */  CH_N,     /* *** UNASSIGNED*/
/* 8A */  CH_N,     /* *** UNASSIGNED*/
/* 8B */  CH_N,     /* *** UNASSIGNED*/
/* 8C */  CH_N,     /* *** UNASSIGNED*/
/* 8D */  CH_N,     /* *** UNASSIGNED*/
/* 8E */  CH_N,     /* *** UNASSIGNED*/
/* 8F */  CH_N,     /* *** UNASSIGNED*/
/* 90 */  CH_N,     /* *** UNASSIGNED*/
/* 91 */  CH_N,     /* *** UNASSIGNED*/
/* 92 */  CH_N,     /* *** UNASSIGNED*/
/* 93 */  CH_N,     /* *** UNASSIGNED*/
/* 94 */  CH_N,     /* *** UNASSIGNED*/
/* 95 */  CH_N,     /* *** UNASSIGNED*/
/* 96 */  CH_N,     /* *** UNASSIGNED*/
/* 97 */  CH_N,     /* *** UNASSIGNED*/
/* 98 */  CH_N,     /* *** UNASSIGNED*/
/* 99 */  CH_N,     /* *** UNASSIGNED*/
/* 9A */  CH_N,     /* *** UNASSIGNED*/
/* 9B */  CH_N,     /* *** UNASSIGNED*/
/* 9C */  CH_N,     /* *** UNASSIGNED*/
/* 9D */  CH_N,     /* *** UNASSIGNED*/
/* 9E */  CH_N,     /* *** UNASSIGNED*/
/* 9F */  CH_N,     /* *** UNASSIGNED*/
/* A0 */  CH_W,   /*<U00A0> NO-BREAK SPACE*/
/* A1 */  CH_N,   /* *** UNASSIGNED*/
/* A2 */  CH_N,   /* *** UNASSIGNED*/
/* A3 */  CH_N,   /* *** UNASSIGNED*/
/* A4 */  CH_ET,    /*<U00A4> CURRENCY SIGN*/
/* A5 */  CH_N,   /* *** UNASSIGNED*/
/* A6 */  CH_N,   /* *** UNASSIGNED*/
/* A7 */  CH_N,   /* *** UNASSIGNED*/
/* A8 */  CH_N,   /* *** UNASSIGNED*/
/* A9 */  CH_N,   /* *** UNASSIGNED*/
/* AA */  CH_N,   /* *** UNASSIGNED*/
/* AB */  CH_N,   /* *** UNASSIGNED*/
/* AC */  CH_R,   /*<U060C> ARABIC COMMA*/
/* AD */  CH_N,   /*<U00AD> SOFT HYPHEN*/
/* AE */  CH_N,   /* *** UNASSIGNED*/
/* AF */  CH_N,   /* *** UNASSIGNED*/
/* B0 */  CH_N,   /* *** UNASSIGNED*/
/* B1 */  CH_N,   /* *** UNASSIGNED*/
/* B2 */  CH_N,   /* *** UNASSIGNED*/
/* B3 */  CH_N,   /* *** UNASSIGNED*/
/* B4 */  CH_N,   /* *** UNASSIGNED*/
/* B5 */  CH_N,   /* *** UNASSIGNED*/
/* B6 */  CH_N,   /* *** UNASSIGNED*/
/* B7 */  CH_N,   /* *** UNASSIGNED*/
/* B8 */  CH_N,   /* *** UNASSIGNED*/
/* B9 */  CH_N,   /* *** UNASSIGNED*/
/* BA */  CH_N,   /* *** UNASSIGNED*/
/* BB */  CH_R,     /*<U061B> ARABIC SEMICOLON*/
/* BC */  CH_N,   /* *** UNASSIGNED*/
/* BD */  CH_N,   /* *** UNASSIGNED*/
/* BE */  CH_N,   /* *** UNASSIGNED*/
/* BF */  CH_R,   /*<U061F> ARABIC QUESTION MARK*/
/* C0 */  CH_N,   /* ***UNASSIGNED*/
/* C1 */  CH_R | CH_ARPRSLET,   /*<UFE80> ARABIC LETTER HAMZA*/
/* C2 */  CH_R | CH_ARPRSLET,   /*<UFE81> ARABIC LETTER ALEF WITH MADDA ABOVE*/
/* C3 */  CH_R | CH_ARPRSLET,   /*<UFE83> ARABIC LETTER ALEF WITH HAMZA ABOVE*/
/* C4 */  CH_R | CH_ARPRSLET,   /*<UFE85> ARABIC LETTER WAW WITH HAMZA ABOVE*/
/* C5 */  CH_R | CH_ARPRSLET,   /*<UFE87> ARABIC LETTER ALEF WITH HAMZA BELOW*/
/* C6 */  CH_R | CH_ARPRSLET,   /*<UFE89> ARABIC LETTER YEH WITH HAMZA ABOVE*/
/* C7 */  CH_R | CH_ARPRSLET,   /*<UFE8D> ARABIC LETTER ALEF*/
/* C8 */  CH_R | CH_ARPRSLET,   /*<UFE8F> ARABIC LETTER BEH*/
/* C9 */  CH_R | CH_ARPRSLET,   /*<UFE93> ARABIC LETTER TEH MARBUTA*/
/* CA */  CH_R | CH_ARPRSLET,   /*<UFE95> ARABIC LETTER TEH*/
/* CB */  CH_R | CH_ARPRSLET,   /*<UFE99> ARABIC LETTER THEH*/
/* CC */  CH_R | CH_ARPRSLET,   /*<UFE9D> ARABIC LETTER JEEM*/
/* CD */  CH_R | CH_ARPRSLET,   /*<UFEA1> ARABIC LETTER HAH*/
/* CE */  CH_R | CH_ARPRSLET,   /*<UFEA5> ARABIC LETTER KHAH*/
/* CF */  CH_R | CH_ARPRSLET,   /*<UFEA9> ARABIC LETTER DAL*/
/* D0 */  CH_R | CH_ARPRSLET,   /*<UFEAB> ARABIC LETTER THAL*/
/* D1 */  CH_R | CH_ARPRSLET,   /*<UFEAD> ARABIC LETTER REH*/
/* D2 */  CH_R | CH_ARPRSLET,   /*<UFEAF> ARABIC LETTER ZAIN*/
/* D3 */  CH_R | CH_ARSIMLET,   /*<U0633> ARABIC LETTER SEEN*/
/* D4 */  CH_R | CH_ARSIMLET,   /*<U0634> ARABIC LETTER SHEEN*/
/* D5 */  CH_R | CH_ARSIMLET,   /*<U0635> ARABIC LETTER SAD*/
/* D6 */  CH_R | CH_ARSIMLET,   /*<U0636> ARABIC LETTER DAD*/
/* D7 */  CH_R | CH_ARPRSLET,   /*<UFEC1> ARABIC LETTER TAH*/
/* D8 */  CH_R | CH_ARPRSLET,   /*<UFEC5> ARABIC LETTER ZAH*/
/* D9 */  CH_R | CH_ARPRSLET,   /*<UFEC9> ARABIC LETTER AIN*/
/* DA */  CH_R | CH_ARPRSLET,   /*<UFECD> ARABIC LETTER GHAIN*/
/* DB */  CH_N,   /* *** UNASSIGNED*/
/* DC */  CH_N,   /* *** UNASSIGNED*/
/* DD */  CH_N,   /* *** UNASSIGNED*/
/* DE */  CH_N,   /* *** UNASSIGNED*/
/* DF */  CH_N,   /* *** UNASSIGNED*/
/* E0 */  CH_R | CH_ARSIMLET,   /*<U0640> ARABIC TATWEEL*/
/* E1 */  CH_R | CH_ARPRSLET,   /*<UFED1> ARABIC LETTER FEH ISOLATED FORM*/
/* E2 */  CH_R | CH_ARPRSLET,   /*<UFED5> ARABIC LETTER QAF ISOLATED FORM*/
/* E3 */  CH_R | CH_ARPRSLET,   /*<UFED9> ARABIC LETTER KAF ISOLATED FORM*/
/* E4 */  CH_R | CH_ARPRSLET,   /*<UFEDD> ARABIC LETTER LAM ISOLATED FORM*/
/* E5 */  CH_R | CH_ARPRSLET,   /*<UFEE1> ARABIC LETTER MEEM ISOLATED FORM*/
/* E6 */  CH_R | CH_ARPRSLET,   /*<UFEE5> ARABIC LETTER NOON ISOLATED FORM*/
/* E7 */  CH_R | CH_ARPRSLET,   /*<UFEE9> ARABIC LETTER HEH ISOLATED FORM*/
/* E8 */  CH_R | CH_ARPRSLET,   /*<UFEED> ARABIC LETTER WAW ISOLATED FORM*/
/* E9 */  CH_R | CH_ARPRSLET,   /*<UFEEF> ARABIC LETTER ALEF MAKSURA ISOLATED FORM*/
/* EA */  CH_R | CH_ARPRSLET,   /*<UFEF1> ARABIC LETTER YEH ISOLATED FORM*/
/* EB */  CH_R | CH_ARSIMLET | CH_NSPACE,   /*<U064B> ARABIC FATHATAN*/
/* EC */  CH_R | CH_ARSIMLET | CH_NSPACE,   /*<U064C> ARABIC DAMMATAN*/
/* ED */  CH_R | CH_ARSIMLET | CH_NSPACE,   /*<U064D> ARABIC KASRATAN*/
/* EE */  CH_R | CH_ARSIMLET | CH_NSPACE,   /*<U064E> ARABIC FATHA*/
/* EF */  CH_R | CH_ARSIMLET | CH_NSPACE,   /*<U064F> ARABIC DAMMA*/
/* F0 */  CH_R | CH_ARSIMLET | CH_NSPACE,   /*<U0650> ARABIC KASRA*/
/* F1 */  CH_R | CH_ARSIMLET | CH_NSPACE,   /*<U0651> ARABIC SHADDA*/
/* F2 */  CH_R | CH_ARSIMLET | CH_NSPACE,   /*<U0652> ARABIC SUKUN*/
/* F3 */  CH_N,   /* *** UNASSIGNED*/
/* F4 */  CH_N,   /* *** UNASSIGNED*/
/* F5 */  CH_N,   /* *** UNASSIGNED*/
/* F6 */  CH_N,   /* *** UNASSIGNED*/
/* F7 */  CH_N,   /* *** UNASSIGNED*/
/* F8 */  CH_N,   /* *** UNASSIGNED*/
/* F9 */  CH_N,   /* *** UNASSIGNED*/
/* FA */  CH_N,   /* *** UNASSIGNED*/
/* FB */  CH_N,   /* *** UNASSIGNED*/
/* FC */  CH_N,   /* *** UNASSIGNED*/
/* FD */  CH_N,   /* *** UNASSIGNED*/
/* FE */  CH_N,   /* *** UNASSIGNED*/
/* FF */  CH_N    /*<U009F>       EIGHT ONES*/
  },
  /*1256 Windows Arabic*/
  {
/* 00 */  CH_N,   /*(CC) Null*/
/* 01 */  CH_N,   /*(CC) Start of Heading*/
/* 02 */  CH_N,   /*(CC) Start of Text*/
/* 03 */  CH_N,   /*(CC) End of Text*/
/* 04 */  CH_N,   /*(CC) End of Transmission*/
/* 05 */  CH_N,   /*(CC) Enquiry*/
/* 06 */  CH_N,   /*(CC) Acknowledge*/
/* 07 */  CH_N,   /*(CC) Bell*/
/* 08 */  CH_N,   /*(CC) Backspace*/
/* 09 */  CH_S,   /*(CC) Horizontal Tabulation*/
/* 0A */  CH_N,   /*(CC) Line Feed*/
/* 0B */  CH_N,   /*(CC) Vertical Tabulation*/
/* 0C */  CH_N,   /*(CC) Form Feed*/
/* 0D */  CH_N,   /*(CC) Carriage Return*/
/* 0E */  CH_N,   /*(CC) Locking Shift One (Shift Out*/
/* 0F */  CH_N,   /*(CC) Locking Shift Zero (Shift In*/
/* 10 */  CH_N,   /*(CC) Data Link Escape*/
/* 11 */  CH_N,   /*(CC) Device Control One*/
/* 12 */  CH_N,   /*(CC) Device Control Two*/
/* 13 */  CH_N,   /*(CC) Device Control Three*/
/* 14 */  CH_N,   /*(CC) Device Control Four*/
/* 15 */  CH_N,   /*(CC) Negative Acknowledge*/
/* 16 */  CH_N,   /*(CC) Synchronous Idle*/
/* 17 */  CH_N,   /*(CC) End of Transmission Block*/
/* 18 */  CH_N,   /*(CC) Cancel*/
/* 19 */  CH_N,   /*(CC) End of Medium*/
/* 1A */  CH_N,   /*(CC) Substitute*/
/* 1B */  CH_N,   /*(CC) Escape*/
/* 1C */  CH_N,   /*(CC) Information File Separator*/
/* 1D */  CH_N,   /*(CC) Group Separator*/
/* 1E */  CH_N,   /*(CC) Record Separator*/
/* 1F */  CH_N,   /*(CC) Unit Separator*/
/* 20 */  CH_W,   /*SPACE*/
/* 21 */  CH_N,   /*EXCLAMATION MARK*/
/* 22 */  CH_N,   /*QUOTATION MARK*/
/* 23 */  CH_ET,    /*NUMBER SIGN*/
/* 24 */  CH_ET,    /*DOLLAR SIGN*/
/* 25 */  CH_ET,    /*PERCENT SIGN*/
/* 26 */  CH_N,   /*AMPERSAND*/
/* 27 */  CH_N,   /*APOSTROPHE*/
/* 28 */  CH_N,   /*LEFT PARENTHESIS*/
/* 29 */  CH_N,   /*RIGHT PARENTHESIS*/
/* 2A */  CH_N,   /*ASTERISK*/
/* 2B */  CH_ET,    /*PLUS SIGN*/
/* 2C */  CH_CS,    /*COMMA*/
/* 2D */  CH_ET,    /*HYPHEN-MINUS*/
/* 2E */  CH_ES,    /*FULL STOP*/
/* 2F */  CH_ES,    /*SOLIDUS*/
/* 30 */  CH_EN | CH_EDIGIT,    /*DIGIT ZERO*/
/* 31 */  CH_EN | CH_EDIGIT,    /*DIGIT ONE*/
/* 32 */  CH_EN | CH_EDIGIT,    /*DIGIT TWO*/
/* 33 */  CH_EN | CH_EDIGIT,    /*DIGIT THREE*/
/* 34 */  CH_EN | CH_EDIGIT,    /*DIGIT FOUR*/
/* 35 */  CH_EN | CH_EDIGIT,    /*DIGIT FIVE*/
/* 36 */  CH_EN | CH_EDIGIT,    /*DIGIT SIX*/
/* 37 */  CH_EN | CH_EDIGIT,    /*DIGIT SEVEN*/
/* 38 */  CH_EN | CH_EDIGIT,    /*DIGIT EIGHT*/
/* 39 */  CH_EN | CH_EDIGIT,    /*DIGIT NINE*/
/* 3A */  CH_CS,    /*COLON*/
/* 3B */  CH_N,   /*SEMICOLON*/
/* 3C */  CH_N,   /*LESS-THAN SIGN*/
/* 3D */  CH_N,   /*EQUALS SIGN*/
/* 3E */  CH_N,   /*GREATER-THAN SIGN*/
/* 3F */  CH_N,   /*QUESTION MARK*/
/* 40 */  CH_N,   /*COMMERCIAL AT*/
/* 41 */  CH_L,   /*LATIN CAPITAL LETTER A*/
/* 42 */  CH_L,   /*LATIN CAPITAL LETTER B*/
/* 43 */  CH_L,   /*LATIN CAPITAL LETTER C*/
/* 44 */  CH_L,   /*LATIN CAPITAL LETTER D*/
/* 45 */  CH_L,   /*LATIN CAPITAL LETTER E*/
/* 46 */  CH_L,   /*LATIN CAPITAL LETTER F*/
/* 47 */  CH_L,   /*LATIN CAPITAL LETTER G*/
/* 48 */  CH_L,   /*LATIN CAPITAL LETTER H*/
/* 49 */  CH_L,   /*LATIN CAPITAL LETTER I*/
/* 4A */  CH_L,   /*LATIN CAPITAL LETTER J*/
/* 4B */  CH_L,   /*LATIN CAPITAL LETTER K*/
/* 4C */  CH_L,   /*LATIN CAPITAL LETTER L*/
/* 4D */  CH_L,   /*LATIN CAPITAL LETTER M*/
/* 4E */  CH_L,   /*LATIN CAPITAL LETTER N*/
/* 4F */  CH_L,   /*LATIN CAPITAL LETTER O*/
/* 50 */  CH_L,   /*LATIN CAPITAL LETTER P*/
/* 51 */  CH_L,   /*LATIN CAPITAL LETTER Q*/
/* 52 */  CH_L,   /*LATIN CAPITAL LETTER R*/
/* 53 */  CH_L,   /*LATIN CAPITAL LETTER S*/
/* 54 */  CH_L,   /*LATIN CAPITAL LETTER T*/
/* 55 */  CH_L,   /*LATIN CAPITAL LETTER U*/
/* 56 */  CH_L,   /*LATIN CAPITAL LETTER V*/
/* 57 */  CH_L,   /*LATIN CAPITAL LETTER W*/
/* 58 */  CH_L,   /*LATIN CAPITAL LETTER X*/
/* 59 */  CH_L,   /*LATIN CAPITAL LETTER Y*/
/* 5A */  CH_L,   /*LATIN CAPITAL LETTER Z*/
/* 5B */  CH_N,   /*LEFT SQUARE BRACKET*/
/* 5C */  CH_N,   /*REVERSE SOLIDUS*/
/* 5D */  CH_N,   /*RIGHT SQUARE BRACKET*/
/* 5E */  CH_N,   /*CIRCUMFLEX ACCENT*/
/* 5F */  CH_N,   /*LOW LINE*/
/* 60 */  CH_N,   /*GRAVE ACCENT*/
/* 61 */  CH_L,   /*LATIN SMALL LETTER A*/
/* 62 */  CH_L,   /*LATIN SMALL LETTER B*/
/* 63 */  CH_L,   /*LATIN SMALL LETTER C*/
/* 64 */  CH_L,   /*LATIN SMALL LETTER D*/
/* 65 */  CH_L,   /*LATIN SMALL LETTER E*/
/* 66 */  CH_L,   /*LATIN SMALL LETTER F*/
/* 67 */  CH_L,   /*LATIN SMALL LETTER G*/
/* 68 */  CH_L,   /*LATIN SMALL LETTER H*/
/* 69 */  CH_L,   /*LATIN SMALL LETTER I*/
/* 6A */  CH_L,   /*LATIN SMALL LETTER J*/
/* 6B */  CH_L,   /*LATIN SMALL LETTER K*/
/* 6C */  CH_L,   /*LATIN SMALL LETTER L*/
/* 6D */  CH_L,   /*LATIN SMALL LETTER M*/
/* 6E */  CH_L,   /*LATIN SMALL LETTER N*/
/* 6F */  CH_L,   /*LATIN SMALL LETTER O*/
/* 70 */  CH_L,   /*LATIN SMALL LETTER P*/
/* 71 */  CH_L,   /*LATIN SMALL LETTER Q*/
/* 72 */  CH_L,   /*LATIN SMALL LETTER R*/
/* 73 */  CH_L,   /*LATIN SMALL LETTER S*/
/* 74 */  CH_L,   /*LATIN SMALL LETTER T*/
/* 75 */  CH_L,   /*LATIN SMALL LETTER U*/
/* 76 */  CH_L,   /*LATIN SMALL LETTER V*/
/* 77 */  CH_L,   /*LATIN SMALL LETTER W*/
/* 78 */  CH_L,   /*LATIN SMALL LETTER X*/
/* 79 */  CH_L,   /*LATIN SMALL LETTER Y*/
/* 7A */  CH_L,   /*LATIN SMALL LETTER Z*/
/* 7B */  CH_N,   /*LEFT CURLY BRACKET*/
/* 7C */  CH_N,   /*VERTICAL LINE*/
/* 7D */  CH_N,   /*RIGHT CURLY BRACKET*/
/* 7E */  CH_N,   /*TILDE*/
/* 7F */  CH_N,   /*(CC) Delete*/
/* 80 */  CH_N,     /* *** UNASSIGNED*/
/* 81 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER PEH*/
/* 82 */  CH_N,   /*SINGLE LOW-9 QUOTATION MARK*/
/* 83 */  CH_N,   /*LATIN SMALL LETTER F WITH HOOK*/
/* 84 */  CH_N,   /*DOUBLE LOW-9 QUOTATION MARK*/
/* 85 */  CH_N,   /*HORIZONTAL ELLIPSIS*/
/* 86 */  CH_N,   /*DAGGER*/
/* 87 */  CH_N,   /*DOUBLE DAGGER*/
/* 88 */  CH_L,   /*MODIFIER LETTER CIRCUMFLEX ACCENT*/
/* 89 */  CH_ET,    /*PER MILLE SIGN*/
/* 8A */  CH_N,   /*(CC) Line Tabultion Set*/
/* 8B */  CH_N,   /*SINGLE LEFT-POINTING ANGLE QUOTATION MARK*/
/* 8C */  CH_N,   /*LATIN CAPITAL LIGATURE OE*/
/* 8D */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER TCHEH*/
/* 8E */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER JEH*/
/* 8F */  CH_N,   /*(CC) Single Shift Three*/
/* 90 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER GAF*/
/* 91 */  CH_N,   /*LEFT SINGLE QUOTATION MARK*/
/* 92 */  CH_N,   /*RIGHT SINGLE QUOTATION MARK*/
/* 93 */  CH_N,   /*LEFT DOUBLE QUOTATION MARK*/
/* 94 */  CH_N,   /*RIGHT DOUBLE QUOTATION MARK*/
/* 95 */  CH_N,   /*BULLET*/
/* 96 */  CH_N,   /*EN DASH*/
/* 97 */  CH_N,   /*EM DASH*/
/* 98 */  CH_N,   /*(CC) Start of String*/
/* 99 */  CH_N,   /*TRADE MARK SIGN*/
/* 9A */  CH_N,   /*(CC) Single Character Introducer*/
/* 9B */  CH_N,   /*SINGLE RIGHT-POINTING ANGLE QUOTATION MARK*/
/* 9C */  CH_L,   /*LATIN SMALL LIGATURE OE*/
/* 9D */  CH_N,   /*ZERO WIDTH NON-JOINER*/
/* 9E */  CH_N,   /*ZERO WIDTH JOINER*/
/* 9F */  CH_N,   /*(CC) Application Program Command*/
/* A0 */  CH_W,   /*NO-BREAK SPACE*/
/* A1 */  CH_R,   /*ARABIC COMMA*/
/* A2 */  CH_ET,    /*CENT SIGN*/
/* A3 */  CH_ET,    /*POUND SIGN*/
/* A4 */  CH_ET,    /*CURRENCY SIGN*/
/* A5 */  CH_ET,    /*YEN SIGN*/
/* A6 */  CH_N,   /*BROKEN BAR*/
/* A7 */  CH_N,   /*SECTION SIGN*/
/* A8 */  CH_N,   /*DIAERESIS*/
/* A9 */  CH_N,   /*COPYRIGHT SIGN*/
/* AA */  CH_N,     /* ***UNASSIGNED*/
/* AB */  CH_N,   /*LEFT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* AC */  CH_N,   /*NOT SIGN*/
/* AD */  CH_N,   /*SOFT HYPHEN*/
/* AE */  CH_N,   /*REGISTERED SIGN*/
/* AF */  CH_N,   /*MACRON*/
/* B0 */  CH_ET,    /*DEGREE SIGN*/
/* B1 */  CH_ET,    /*PLUS-MINUS SIGN*/
/* B2 */  CH_EN,    /*SUPERSCRIPT TWO*/
/* B3 */  CH_EN,    /*SUPERSCRIPT THREE*/
/* B4 */  CH_N,   /*ACUTE ACCENT*/
/* B5 */  CH_N,   /*MICRO SIGN*/
/* B6 */  CH_N,   /*PILCROW SIGN*/
/* B7 */  CH_N,   /*MIDDLE DOT*/
/* B8 */  CH_N,   /*CEDILLA*/
/* B9 */  CH_EN,    /*SUPERSCRIPT ONE*/
/* BA */  CH_R,     /*ARABIC SEMICOLON*/
/* BB */  CH_N,   /*RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* BC */  CH_N,   /*VULGAR FRACTION ONE QUARTER*/
/* BD */  CH_N,   /*VULGAR FRACTION ONE HALF*/
/* BE */  CH_N,   /*VULGAR FRACTION THREE QUARTERS*/
/* BF */  CH_R,   /*ARABIC QUESTION MARK*/
/* C0 */  CH_N,   /* ***UNASSIGNED*/
/* C1 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER HAMZA*/
/* C2 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER ALEF WITH MADDA ABOVE*/
/* C3 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER ALEF WITH HAMZA ABOVE*/
/* C4 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER WAW WITH HAMZA ABOVE*/
/* C5 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER ALEF WITH HAMZA BELOW*/
/* C6 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER YEH WITH HAMZA ABOVE*/
/* C7 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER ALEF*/
/* C8 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER BEH*/
/* C9 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER TEH MARBUTA*/
/* CA */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER TEH*/
/* CB */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER THEH*/
/* CC */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER JEEM*/
/* CD */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER HAH*/
/* CE */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER KHAH*/
/* CF */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER DAL*/
/* D0 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER THAL*/
/* D1 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER REH*/
/* D2 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER ZAIN*/
/* D3 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER SEEN*/
/* D4 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER SHEEN*/
/* D5 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER SAD*/
/* D6 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER DAD*/
/* D7 */  CH_N,   /*MULTIPLICATION SIGN*/
/* D8 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER TAH*/
/* D9 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER ZAH*/
/* DA */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER AIN*/
/* DB */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER GHAIN*/
/* DC */  CH_R | CH_ARSIMLET,   /*ARABIC TATWEEL*/
/* DD */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER FEH*/
/* DE */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER QAF*/
/* DF */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER KAF*/
/* E0 */  CH_L,   /*LATIN SMALL LETTER A WITH GRAVE*/
/* E1 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER LAM*/
/* E2 */  CH_L,   /*LATIN SMALL LETTER A WITH CIRCUMFLEX*/
/* E3 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER MEEM*/
/* E4 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER NOON*/
/* E5 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER HEH*/
/* E6 */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER WAW*/
/* E7 */  CH_L,   /*LATIN SMALL LETTER C WITH CEDILLA*/
/* E8 */  CH_L,   /*LATIN SMALL LETTER E WITH GRAVE*/
/* E9 */  CH_L,   /*LATIN SMALL LETTER E WITH ACUTE*/
/* EA */  CH_L,   /*LATIN SMALL LETTER E WITH CIRCUMFLEX*/
/* EB */  CH_L,   /*LATIN SMALL LETTER E WITH DIAERESIS*/
/* EC */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER ALEF MAKSURA*/
/* ED */  CH_R | CH_ARSIMLET,   /*ARABIC LETTER YEH*/
/* EE */  CH_L,   /*LATIN SMALL LETTER I WITH CIRCUMFLEX*/
/* EF */  CH_L,   /*LATIN SMALL LETTER I WITH DIAERESIS*/
/* F0 */  CH_R | CH_NSPACE,   /*ARABIC FATHATAN*/
/* F1 */  CH_R | CH_NSPACE,   /*ARABIC DAMMATAN*/
/* F2 */  CH_R | CH_NSPACE,   /*ARABIC KASRATAN*/
/* F3 */  CH_R | CH_NSPACE,   /*ARABIC FATHA*/
/* F4 */  CH_L,   /*LATIN SMALL LETTER O WITH CIRCUMFLEX*/
/* F5 */  CH_R | CH_NSPACE,   /*ARABIC DAMMA*/
/* F6 */  CH_R | CH_NSPACE,   /*ARABIC KASRA*/
/* F7 */  CH_N,   /*DIVISION SIGN*/
/* F8 */  CH_R | CH_NSPACE,   /*ARABIC SHADDA*/
/* F9 */  CH_L,   /*LATIN SMALL LETTER U WITH GRAVE*/
/* FA */  CH_R | CH_NSPACE | CH_ARSIMLET,   /*ARABIC SUKUN*/
/* FB */  CH_L,   /*LATIN SMALL LETTER U WITH CIRCUMFLEX*/
/* FC */  CH_L,   /*LATIN SMALL LETTER U WITH DIAERESIS*/
/* FD */  CH_L | CH_BIDICTRL,   /*LEFT-TO-RIGHT MARK*/
/* FE */  CH_R | CH_BIDICTRL,   /*RIGHT-TO-LEFT MARK*/
  },
  /* 424 MF/AS 400 Hebrew*/
  {
/* 00 */  0,      /*<U0000>       NULL*/
/* 01 */  0,      /*<U0001>       START OF HEADING*/
/* 02 */  0,      /*<U0002>       START OF TEXT*/
/* 03 */  0,      /*<U0003>       END OF TEXT*/
/* 04 */  0,      /*<U009C> */
/* 05 */  CH_S,   /*<U0009>       HORIZONTAL TABULATION*/
/* 06 */  0,      /*<U0086> */
/* 07 */  0,      /*<U007F>       DELETE*/
/* 08 */  0,      /*<U0097> */
/* 09 */  0,      /*<U008D> */
/* 0A */  0,      /*<U008E> */
/* 0B */  0,      /*<U000B>       VERTICAL TABULATION*/
/* 0C */  0,      /*<U000C>       FORM FEED*/
/* 0D */  0,      /*<U000D>       CARRIAGE RETURN*/
/* 0E */  0,      /*<U000E>       SHIFT OUT*/
/* 0F */  0,      /*<U000F>       SHIFT IN*/
/* 10 */  0,      /*<U0010>       DATA LINK ESCAPE*/
/* 11 */  0,      /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */  0,      /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */  0,      /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */  0,      /*<U009D> */
/* 15 */  0,      /*<U0085> */
/* 16 */  0,      /*<U0008>       BACKSPACE*/
/* 17 */  0,      /*<U0087> */
/* 18 */  0,      /*<U0018>       CANCEL*/
/* 19 */  0,      /*<U0019>       END OF MEDIUM*/
/* 1A */  0,      /*<U0092> */
/* 1B */  0,      /*<U008F> */
/* 1C */  0,      /*<U001C>       FILE SEPARATOR*/
/* 1D */  0,      /*<U001D>       GROUP SEPARATOR*/
/* 1E */  0,      /*<U001E>       RECORD SEPARATOR*/
/* 1F */  0,      /*<U001F>       UNIT SEPARATOR*/
/* 20 */  0,      /*<U0080> */
/* 21 */  0,      /*<U0081> */
/* 22 */  0,      /*<U0082> */
/* 23 */  0,      /*<U0083> */
/* 24 */  0,      /*<U0084> */
/* 25 */  0,      /*<U000A>       LINE FEED*/
/* 26 */  0,      /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 27 */  0,      /*<U001B>       ESCAPE*/
/* 28 */  0,      /*<U0088> */
/* 29 */  0,      /*<U0089> */
/* 2A */  0,      /*<U008A> */
/* 2B */  0,      /*<U008B> */
/* 2C */  0,      /*<U008C> */
/* 2D */  0,      /*<U0005>       ENQUIRY*/
/* 2E */  0,      /*<U0006>       ACKNOWLEDGE*/
/* 2F */  0,      /*<U0007>       BELL*/
/* 30 */  0,      /*<U0090> */
/* 31 */  0,      /*<U0091> */
/* 32 */  0,      /*<U0016>       SYNCHRONOUS IDLE*/
/* 33 */  0,      /*<U0093> */
/* 34 */  0,      /*<U0094> */
/* 35 */  0,      /*<U0095> */
/* 36 */  0,      /*<U0096> */
/* 37 */  0,      /*<U0004>       END OF TRANSMISSION*/
/* 38 */  0,      /*<U0098> */
/* 39 */  0,      /*<U0099> */
/* 3A */  0,      /*<U009A> */
/* 3B */  0,      /*<U009B> */
/* 3C */  0,      /*<U0014>       DEVICE CONTROL FOUR*/
/* 3D */  0,      /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 3E */  0,      /*<U009E> */
/* 3F */  0,      /*<U001A>       SUBSTITUTE*/
/* 40 */  CH_W,   /*<U0020>   SPACE*/
/* 41 */  CH_R | CH_HEBLET,   /*<U05D0>       HEBREW LETTER ALEF*/
/* 42 */  CH_R | CH_HEBLET,   /*<U05D1>       HEBREW LETTER BET*/
/* 43 */  CH_R | CH_HEBLET,   /*<U05D2>       HEBREW LETTER GIMEL*/
/* 44 */  CH_R | CH_HEBLET,   /*<U05D3>       HEBREW LETTER DALET*/
/* 45 */  CH_R | CH_HEBLET,   /*<U05D4>       HEBREW LETTER HE*/
/* 46 */  CH_R | CH_HEBLET,   /*<U05D5>       HEBREW LETTER VAV*/
/* 47 */  CH_R | CH_HEBLET,   /*<U05D6>       HEBREW LETTER ZAYIN*/
/* 48 */  CH_R | CH_HEBLET,   /*<U05D7>       HEBREW LETTER HET*/
/* 49 */  CH_R | CH_HEBLET,   /*<U05D8>       HEBREW LETTER TET*/
/* 4A */  CH_ET,    /*<U00A2>       CENT SIGN*/
/* 4B */  CH_ES,    /*<U002E>       PERIOD*/
/* 4C */  CH_N,   /*<U003C>       LESS-THAN SIGN*/
/* 4D */  CH_N,   /*<U0028>       OPENING PARENTHESIS*/
/* 4E */  CH_ET,    /*<U002B>       PLUS SIGN*/
/* 4F */  CH_N,   /*<U007C>       VERTICAL BAR*/
/* 50 */  CH_N,   /*<U0026>       AMPERSAND*/
/* 51 */  CH_R | CH_HEBLET,   /*<U05D9>       HEBREW LETTER YOD*/
/* 52 */  CH_R | CH_HEBLET,   /*<U05DA>       HEBREW LETTER FINAL KAF*/
/* 53 */  CH_R | CH_HEBLET,   /*<U05DB>       HEBREW LETTER KAF*/
/* 54 */  CH_R | CH_HEBLET,   /*<U05DC>       HEBREW LETTER LAMED*/
/* 55 */  CH_R | CH_HEBLET,   /*<U05DD>       HEBREW LETTER FINAL MEM*/
/* 56 */  CH_R | CH_HEBLET,   /*<U05DE>       HEBREW LETTER MEM*/
/* 57 */  CH_R | CH_HEBLET,   /*<U05DF>       HEBREW LETTER FINAL NUN*/
/* 58 */  CH_R | CH_HEBLET,   /*<U05E0>       HEBREW LETTER NUN*/
/* 59 */  CH_R | CH_HEBLET,   /*<U05E1>       HEBREW LETTER SAMEKH*/
/* 5A */  CH_N,   /*<U0021>       EXCLAMATION MARK*/
/* 5B */  CH_ET,    /*<U0024>       DOLLAR SIGN*/
/* 5C */  CH_N,   /*<U002A>       ASTERISK*/
/* 5D */  CH_N,   /*<U0029>       CLOSING PARENTHESIS*/
/* 5E */  CH_N,   /*<U003B>       SEMICOLON*/
/* 5F */  CH_N,   /*<U00AC>       NOT SIGN*/
/* 60 */  CH_ET,    /*<U002D>       HYPHEN-MINUS*/
/* 61 */  CH_ES,    /*<U002F>       SLASH*/
/* 62 */  CH_R | CH_HEBLET,   /*<U05E2>       HEBREW LETTER AYIN*/
/* 63 */  CH_R | CH_HEBLET,   /*<U05E3>       HEBREW LETTER FINAL PE*/
/* 64 */  CH_R | CH_HEBLET,   /*<U05E4>       HEBREW LETTER PE*/
/* 65 */  CH_R | CH_HEBLET,   /*<U05E5>       HEBREW LETTER FINAL TSADI*/
/* 66 */  CH_R | CH_HEBLET,   /*<U05E6>       HEBREW LETTER TSADI*/
/* 67 */  CH_R | CH_HEBLET,   /*<U05E7>       HEBREW LETTER QOF*/
/* 68 */  CH_R | CH_HEBLET,   /*<U05E8>       HEBREW LETTER RESH*/
/* 69 */  CH_R | CH_HEBLET,   /*<U05E9>       HEBREW LETTER SHIN*/
/* 6A */  CH_N,   /*<U00A6>       BROKEN VERTICAL BAR*/
/* 6B */  CH_CS,    /*<U002C>       COMMA*/
/* 6C */  CH_ET,    /*<U0025>       PERCENT SIGN*/
/* 6D */  CH_N,   /*<U005F>       SPACING UNDERSCORE*/
/* 6E */  CH_N,   /*<U003E>       GREATER-THAN SIGN*/
/* 6F */  CH_N,   /*<U003F>       QUESTION MARK*/
/* 70 */  CH_N,   /* **UNASSIGNED*/
/* 71 */  CH_R | CH_HEBLET,   /*<U05EA>       HEBREW LETTER TAV*/
/* 72 */  CH_N,   /* **UNASSIGNED*/
/* 73 */  CH_N,   /* **UNASSIGNED*/
/* 74 */  CH_W,   /*<U00A0>       NON-BREAKING SPACE*/
/* 75 */  CH_N,   /* **UNASSIGNED*/
/* 76 */  CH_N,   /* **UNASSIGNED*/
/* 77 */  CH_N,   /* **UNASSIGNED*/
/* 78 */  CH_N,   /*<U2017>       SPACING DOUBLE UNDERSCORE*/
/* 79 */  CH_N,   /*<U0060>       SPACING GRAVE*/
/* 7A */  CH_CS,    /*<U003A>       COLON*/
/* 7B */  CH_ET,    /*<U0023>       NUMBER SIGN*/
/* 7C */  CH_N,   /*<U0040>       COMMERCIAL AT*/
/* 7D */  CH_N,   /*<U0027>       APOSTROPHE-QUOTE*/
/* 7E */  CH_N,   /*<U003D>       EQUALS SIGN*/
/* 7F */  CH_N,   /*<U0022>       QUOTATION MARK*/
/* 80 */  CH_N,   /* ***UNASSIGNED*/
/* 81 */  CH_L,   /*<U0061>       LATIN SMALL LETTER A*/
/* 82 */  CH_L,   /*<U0062>       LATIN SMALL LETTER B*/
/* 83 */  CH_L,   /*<U0063>       LATIN SMALL LETTER C*/
/* 84 */  CH_L,   /*<U0064>       LATIN SMALL LETTER D*/
/* 85 */  CH_L,   /*<U0065>       LATIN SMALL LETTER E*/
/* 86 */  CH_L,   /*<U0066>       LATIN SMALL LETTER F*/
/* 87 */  CH_L,   /*<U0067>       LATIN SMALL LETTER G*/
/* 88 */  CH_L,   /*<U0068>       LATIN SMALL LETTER H*/
/* 89 */  CH_L,   /*<U0069>       LATIN SMALL LETTER I*/
/* 8A */  CH_N,   /*<U00AB>       LEFT POINTING GUILLEMET*/
/* 8B */  CH_N,   /*<U00BB>       RIGHT POINTING GUILLEMET*/
/* 8C */  CH_N,   /* **UNASSIGNED*/
/* 8D */  CH_N,   /* **UNASSIGNED*/
/* 8E */  CH_N,   /* **UNASSIGNED*/
/* 8F */  CH_ET,    /*<U00B1>       PLUS-OR-MINUS SIGN*/
/* 90 */  CH_ET,    /*<U00B0>       DEGREE SIGN*/
/* 91 */  CH_L,   /*<U006A>       LATIN SMALL LETTER J*/
/* 92 */  CH_L,   /*<U006B>       LATIN SMALL LETTER K*/
/* 93 */  CH_L,   /*<U006C>       LATIN SMALL LETTER L*/
/* 94 */  CH_L,   /*<U006D>       LATIN SMALL LETTER M*/
/* 95 */  CH_L,   /*<U006E>       LATIN SMALL LETTER N*/
/* 96 */  CH_L,   /*<U006F>       LATIN SMALL LETTER O*/
/* 97 */  CH_L,   /*<U0070>       LATIN SMALL LETTER P*/
/* 98 */  CH_L,   /*<U0071>       LATIN SMALL LETTER Q*/
/* 99 */  CH_L,   /*<U0072>       LATIN SMALL LETTER R*/
/* 9A */  CH_N,   /* **UNASSIGNED*/
/* 9B */  CH_N,   /* **UNASSIGNED*/
/* 9C */  CH_N,   /* **UNASSIGNED*/
/* 9D */  CH_N,   /*<U00B8>       SPACING CEDILLA*/
/* 9E */  CH_N,   /* **UNASSIGNED*/
/* 9F */  CH_ET,    /*<U00A4>       CURRENCY SIGN*/
/* A0 */  CH_N,   /*<U00B5>       MICRO SIGN*/
/* A1 */  CH_N,   /*<U007E>       TILDE*/
/* A2 */  CH_L,   /*<U0073>       LATIN SMALL LETTER S*/
/* A3 */  CH_L,   /*<U0074>       LATIN SMALL LETTER T*/
/* A4 */  CH_L,   /*<U0075>       LATIN SMALL LETTER U*/
/* A5 */  CH_L,   /*<U0076>       LATIN SMALL LETTER V*/
/* A6 */  CH_L,   /*<U0077>       LATIN SMALL LETTER W*/
/* A7 */  CH_L,   /*<U0078>       LATIN SMALL LETTER X*/
/* A8 */  CH_L,   /*<U0079>       LATIN SMALL LETTER Y*/
/* A9 */  CH_L,   /*<U007A>       LATIN SMALL LETTER Z*/
/* AA */  CH_N,   /* **UNASSIGNED*/
/* AB */  CH_N,   /* **UNASSIGNED*/
/* AC */  CH_N,   /* **UNASSIGNED*/
/* AD */  CH_N,   /* **UNASSIGNED*/
/* AE */  CH_N,   /* **UNASSIGNED*/
/* AF */  CH_N,   /*<U00AE>       REGISTERED TRADE MARK SIGN*/
/* B0 */  CH_N,   /*<U005E>       SPACING CIRCUMFLEX*/
/* B1 */  CH_ET,    /*<U00A3>       POUND SIGN*/
/* B2 */  CH_ET,    /*<U00A5>       YEN SIGN*/
/* B3 */  CH_N,   /*<U2022>       BULLET*/
/* B4 */  CH_N,   /*<U00A9>       COPYRIGHT SIGN*/
/* B5 */  CH_N,   /*<U00A7>       SECTION SIGN*/
/* B6 */  CH_N,   /*<U00B6>       PARAGRAPH SIGN*/
/* B7 */  CH_N,   /*<U00BC>       FRACTION ONE QUARTER*/
/* B8 */  CH_N,   /*<U00BD>       FRACTION ONE HALF*/
/* B9 */  CH_N,   /*<U00BE>       FRACTION THREE QUARTERS*/
/* BA */  CH_N,   /*<U005B>       OPENING SQUARE BRACKET*/
/* BB */  CH_N,   /*<U005D>       CLOSING SQUARE BRACKET*/
/* BC */  CH_N,   /*<U00AF>       SPACING MACRON*/
/* BD */  CH_N,   /*<U00A8>       SPACING DIAERESIS*/
/* BE */  CH_N,   /*<U00B4>       SPACING ACUTE*/
/* BF */  CH_N,   /*<U00D7>       MULTIPLICATION SIGN*/
/* C0 */  CH_N,   /*<U007B>       OPENING CURLY BRACKET*/
/* C1 */  CH_L,   /*<U0041>       LATIN CAPITAL LETTER A*/
/* C2 */  CH_L,   /*<U0042>       LATIN CAPITAL LETTER B*/
/* C3 */  CH_L,   /*<U0043>       LATIN CAPITAL LETTER C*/
/* C4 */  CH_L,   /*<U0044>       LATIN CAPITAL LETTER D*/
/* C5 */  CH_L,   /*<U0045>       LATIN CAPITAL LETTER E*/
/* C6 */  CH_L,   /*<U0046>       LATIN CAPITAL LETTER F*/
/* C7 */  CH_L,   /*<U0047>       LATIN CAPITAL LETTER G*/
/* C8 */  CH_L,   /*<U0048>       LATIN CAPITAL LETTER H*/
/* C9 */  CH_L,   /*<U0049>       LATIN CAPITAL LETTER I*/
/* CA */  CH_N,   /*<U00AD>       SOFT HYPHEN*/
/* CB */  CH_N,   /* *** UNASSIGNED*/
/* CC */  CH_N,   /* *** UNASSIGNED*/
/* CD */  CH_N,   /* *** UNASSIGNED*/
/* CE */  CH_N,   /* *** UNASSIGNED*/
/* CF */  CH_N,   /* *** UNASSIGNED*/
/* D0 */  CH_N,   /*<U007D>       CLOSING CURLY BRACKET*/
/* D1 */  CH_L,   /*<U004A>       LATIN CAPITAL LETTER J*/
/* D2 */  CH_L,   /*<U004B>       LATIN CAPITAL LETTER K*/
/* D3 */  CH_L,   /*<U004C>       LATIN CAPITAL LETTER L*/
/* D4 */  CH_L,   /*<U004D>       LATIN CAPITAL LETTER M*/
/* D5 */  CH_L,   /*<U004E>       LATIN CAPITAL LETTER N*/
/* D6 */  CH_L,   /*<U004F>       LATIN CAPITAL LETTER O*/
/* D7 */  CH_L,   /*<U0050>       LATIN CAPITAL LETTER P*/
/* D8 */  CH_L,   /*<U0051>       LATIN CAPITAL LETTER Q*/
/* D9 */  CH_L,   /*<U0052>       LATIN CAPITAL LETTER R*/
/* DA */  CH_EN,    /*<U00B9>       SUPERSCRIPT DIGIT ONE*/
/* DB */  CH_N | CH_BIDICTRL,   /*LEFT-TO-RIGHT OVERRIDE*/
/* DC */  CH_N | CH_BIDICTRL,   /*RIGHT-TO-LEFT OVERRIDE*/
/* DD */  CH_N,   /*POP DIRECTION FORMATTING*/
/* DE */  CH_N,   /* *** UNASSIGNED*/
/* DF */  CH_N,   /* *** UNASSIGNED*/
/* E0 */  CH_N,   /*<U005C>       BACKSLASH*/
/* E1 */  CH_N,   /*<U00F7>       DIVISION SIGN*/
/* E2 */  CH_L,   /*<U0053>       LATIN CAPITAL LETTER S*/
/* E3 */  CH_L,   /*<U0054>       LATIN CAPITAL LETTER T*/
/* E4 */  CH_L,   /*<U0055>       LATIN CAPITAL LETTER U*/
/* E5 */  CH_L,   /*<U0056>       LATIN CAPITAL LETTER V*/
/* E6 */  CH_L,   /*<U0057>       LATIN CAPITAL LETTER W*/
/* E7 */  CH_L,   /*<U0058>       LATIN CAPITAL LETTER X*/
/* E8 */  CH_L,   /*<U0059>       LATIN CAPITAL LETTER Y*/
/* E9 */  CH_L,   /*<U005A>       LATIN CAPITAL LETTER Z*/
/* EA */  CH_EN,    /*<U00B2>       SUPERSCRIPT DIGIT TWO*/
/* EB */  CH_N,   /* *** UNASSIGNED*/
/* EC */  CH_N,     /* *** UNASSIGNED*/
/* ED */  CH_N,     /* *** UNASSIGNED*/
/* EE */  CH_N,     /* *** UNASSIGNED*/
/* EF */  CH_N,     /* *** UNASSIGNED*/
/* F0 */  CH_EN | CH_EDIGIT,    /*<U0030>       DIGIT ZERO*/
/* F1 */  CH_EN | CH_EDIGIT,    /*<U0031>       DIGIT ONE*/
/* F2 */  CH_EN | CH_EDIGIT,    /*<U0032>       DIGIT TWO*/
/* F3 */  CH_EN | CH_EDIGIT,    /*<U0033>       DIGIT THREE*/
/* F4 */  CH_EN | CH_EDIGIT,    /*<U0034>       DIGIT FOUR*/
/* F5 */  CH_EN | CH_EDIGIT,    /*<U0035>       DIGIT FIVE*/
/* F6 */  CH_EN | CH_EDIGIT,    /*<U0036>       DIGIT SIX*/
/* F7 */  CH_EN | CH_EDIGIT,    /*<U0037>       DIGIT SEVEN*/
/* F8 */  CH_EN | CH_EDIGIT,    /*<U0038>       DIGIT EIGHT*/
/* F9 */  CH_EN | CH_EDIGIT,    /*<U0039>       DIGIT NINE*/
/* FA */  CH_EN,    /*<U00B3>       SUPERSCRIPT DIGIT THREE*/
/* FB */  CH_N | CH_BIDICTRL,   /*LEFT-TO-RIGHT EMBEDDED*/
/* FC */  CH_N | CH_BIDICTRL,   /*RIGHT-TO-LEFT EMBEDDED*/
/* FD */  CH_L | CH_BIDICTRL,   /*LEFT-TO-RIGHT MARKER*/
/* FE */  CH_R | CH_BIDICTRL,   /*RIGHT-TO-LEFT MARKER*/
/* FF */  CH_N    /*<U009F>       EIGHT ONES*/
  },
  /* 862 OS/2 Hebrew*/
  {
/* 00 */  CH_N,   /*<U0000>       NULL*/
/* 01 */  CH_N,   /*<U0001>       START OF HEADING*/
/* 02 */  CH_N,   /*<U0002>       START OF TEXT*/
/* 03 */  CH_N,   /*<U0003>       END OF TEXT*/
/* 04 */  CH_N,   /*<U0004>       END OF TRANSMISSION*/
/* 05 */  CH_N,   /*<U0005>       ENQUIRY*/
/* 06 */  CH_N,   /*<U0006>       ACKNOWLEDGE*/
/* 07 */  CH_N,   /*<U0007>       BELL*/
/* 08 */  CH_N,   /*<U0008>       BACKSPACE*/
/* 09 */  CH_S,   /*<U0009>       HORIZONTAL TABULATION*/
/* 0A */  CH_N,   /*<U000A>       LINE FEED*/
/* 0B */  CH_N,   /*<U000B>       VERTICAL TABULATION*/
/* 0C */  CH_N,   /*<U000C>       FORM FEED*/
/* 0D */  CH_N,   /*<U000D>       CARRIAGE RETURN*/
/* 0E */  CH_N,   /*<U000E>       SHIFT OUT*/
/* 0F */  CH_N,   /*<U000F>       SHIFT IN*/
/* 10 */  CH_N,   /*<U0010>       DATA LINK ESCAPE*/
/* 11 */  CH_N,   /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */  CH_N,   /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */  CH_N,   /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */  CH_N,   /*<U0014>       DEVICE CONTROL FOUR*/
/* 15 */  CH_N,   /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 16 */  CH_N,   /*<U0016>       SYNCHRONOUS IDLE*/
/* 17 */  CH_N,   /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 18 */  CH_N,   /*<U0018>       CANCEL*/
/* 19 */  CH_N,   /*<U0019>       END OF MEDIUM*/
/* 1A */  CH_N,   /*<U001C>       FILE SEPARATOR*/
/* 1B */  CH_N,   /*<U001B>       ESCAPE*/
/* 1C */  CH_N,   /*<U007F>       DELETE*/
/* 1D */  CH_N,   /*<U001D>       GROUP SEPARATOR*/
/* 1E */  CH_N,   /*<U001E>       RECORD SEPARATOR*/
/* 1F */  CH_N,   /*<U001F>       UNIT SEPARATOR*/
/* 20 */  CH_W,   /*<U0020>       SPACE*/
/* 21 */  CH_N,   /*<U0021>       EXCLAMATION MARK*/
/* 22 */  CH_N,   /*<U0022>       QUOTATION MARK*/
/* 23 */  CH_ET,    /*<U0023>       NUMBER SIGN*/
/* 24 */  CH_ET,    /*<U0024>       DOLLAR SIGN*/
/* 25 */  CH_ET,    /*<U0025>       PERCENT SIGN*/
/* 26 */  CH_N,   /*<U0026>       AMPERSAND*/
/* 27 */  CH_N,   /*<U0027>       APOSTROPHE*/
/* 28 */  CH_N,   /*<U0028>       OPENING PARENTHESIS*/
/* 29 */  CH_N,   /*<U0029>       CLOSING PARENTHESIS*/
/* 2A */  CH_N,   /*<U002A>       ASTERISK*/
/* 2B */  CH_ET,    /*<U002B>       PLUS SIGN*/
/* 2C */  CH_CS,    /*<U002C>       COMMA*/
/* 2D */  CH_ET,    /*<U002D>       HYPHEN-MINUS*/
/* 2E */  CH_ES,    /*<U002E>       PERIOD*/
/* 2F */  CH_ES,    /*<U002F>       SLASH*/
/* 30 */  CH_EN | CH_EDIGIT,    /*<U0030>       DIGIT ZERO*/
/* 31 */  CH_EN | CH_EDIGIT,    /*<U0031>       DIGIT ONE*/
/* 32 */  CH_EN | CH_EDIGIT,    /*<U0032>       DIGIT TWO*/
/* 33 */  CH_EN | CH_EDIGIT,    /*<U0033>       DIGIT THREE*/
/* 34 */  CH_EN | CH_EDIGIT,    /*<U0034>       DIGIT FOUR*/
/* 35 */  CH_EN | CH_EDIGIT,    /*<U0035>       DIGIT FIVE*/
/* 36 */  CH_EN | CH_EDIGIT,    /*<U0036>       DIGIT SIX*/
/* 37 */  CH_EN | CH_EDIGIT,    /*<U0037>       DIGIT SEVEN*/
/* 38 */  CH_EN | CH_EDIGIT,    /*<U0038>       DIGIT EIGHT*/
/* 39 */  CH_EN | CH_EDIGIT,    /*<U0039>       DIGIT NINE*/
/* 3A */  CH_CS,    /*<U003A>       COLON*/
/* 3B */  CH_N,   /*<U003B>       SEMICOLON*/
/* 3C */  CH_N,   /*<U003C>       LESS-THAN SIGN*/
/* 3D */  CH_N,   /*<U003D>       EQUALS SIGN*/
/* 3E */  CH_N,   /*<U003E>       GREATER-THAN SIGN*/
/* 3F */  CH_N,   /*<U003F>       QUESTION MARK*/
/* 40 */  CH_N,   /*<U0040>       COMMERCIAL AT*/
/* 41 */  CH_L,   /*<U0041>       LATIN CAPITAL LETTER A*/
/* 42 */  CH_L,   /*<U0042>       LATIN CAPITAL LETTER B*/
/* 43 */  CH_L,   /*<U0043>       LATIN CAPITAL LETTER C*/
/* 44 */  CH_L,   /*<U0044>       LATIN CAPITAL LETTER D*/
/* 45 */  CH_L,   /*<U0045>       LATIN CAPITAL LETTER E*/
/* 46 */  CH_L,   /*<U0046>       LATIN CAPITAL LETTER F*/
/* 47 */  CH_L,   /*<U0047>       LATIN CAPITAL LETTER G*/
/* 48 */  CH_L,   /*<U0048>       LATIN CAPITAL LETTER H*/
/* 49 */  CH_L,   /*<U0049>       LATIN CAPITAL LETTER I*/
/* 4A */  CH_L,   /*<U004A>       LATIN CAPITAL LETTER J*/
/* 4B */  CH_L,   /*<U004B>       LATIN CAPITAL LETTER K*/
/* 4C */  CH_L,   /*<U004C>       LATIN CAPITAL LETTER L*/
/* 4D */  CH_L,   /*<U004D>       LATIN CAPITAL LETTER M*/
/* 4E */  CH_L,   /*<U004E>       LATIN CAPITAL LETTER N*/
/* 4F */  CH_L,   /*<U004F>       LATIN CAPITAL LETTER O*/
/* 50 */  CH_L,   /*<U0050>       LATIN CAPITAL LETTER P*/
/* 51 */  CH_L,   /*<U0051>       LATIN CAPITAL LETTER Q*/
/* 52 */  CH_L,   /*<U0052>       LATIN CAPITAL LETTER R*/
/* 53 */  CH_L,   /*<U0053>       LATIN CAPITAL LETTER S*/
/* 54 */  CH_L,   /*<U0054>       LATIN CAPITAL LETTER T*/
/* 55 */  CH_L,   /*<U0055>       LATIN CAPITAL LETTER U*/
/* 56 */  CH_L,   /*<U0056>       LATIN CAPITAL LETTER V*/
/* 57 */  CH_L,   /*<U0057>       LATIN CAPITAL LETTER W*/
/* 58 */  CH_L,   /*<U0058>       LATIN CAPITAL LETTER X*/
/* 59 */  CH_L,   /*<U0059>       LATIN CAPITAL LETTER Y*/
/* 5A */  CH_L,   /*<U005A>       LATIN CAPITAL LETTER Z*/
/* 5B */  CH_N,   /*<U005B>       OPENING SQUARE BRACKET*/
/* 5C */  CH_N,   /*<U005C>       BACKSLASH*/
/* 5D */  CH_N,   /*<U005D>       CLOSING SQUARE BRACKET*/
/* 5E */  CH_N,   /*<U005E>       SPACING CIRCUMFLEX*/
/* 5F */  CH_N,   /*<U005F>       SPACING UNDERSCORE*/
/* 60 */  CH_N,   /*<U0060>       SPACING GRAVE*/
/* 61 */  CH_L,   /*<U0061>       LATIN SMALL LETTER A*/
/* 62 */  CH_L,   /*<U0062>       LATIN SMALL LETTER B*/
/* 63 */  CH_L,   /*<U0063>       LATIN SMALL LETTER C*/
/* 64 */  CH_L,   /*<U0064>       LATIN SMALL LETTER D*/
/* 65 */  CH_L,   /*<U0065>       LATIN SMALL LETTER E*/
/* 66 */  CH_L,   /*<U0066>       LATIN SMALL LETTER F*/
/* 67 */  CH_L,   /*<U0067>       LATIN SMALL LETTER G*/
/* 68 */  CH_L,   /*<U0068>       LATIN SMALL LETTER H*/
/* 69 */  CH_L,   /*<U0069>       LATIN SMALL LETTER I*/
/* 6A */  CH_L,   /*<U006A>       LATIN SMALL LETTER J*/
/* 6B */  CH_L,   /*<U006B>       LATIN SMALL LETTER K*/
/* 6C */  CH_L,   /*<U006C>       LATIN SMALL LETTER L*/
/* 6D */  CH_L,   /*<U006D>       LATIN SMALL LETTER M*/
/* 6E */  CH_L,   /*<U006E>       LATIN SMALL LETTER N*/
/* 6F */  CH_L,   /*<U006F>       LATIN SMALL LETTER O*/
/* 70 */  CH_L,   /*<U0070>       LATIN SMALL LETTER P*/
/* 71 */  CH_L,   /*<U0071>       LATIN SMALL LETTER Q*/
/* 72 */  CH_L,   /*<U0072>       LATIN SMALL LETTER R*/
/* 73 */  CH_L,   /*<U0073>       LATIN SMALL LETTER S*/
/* 74 */  CH_L,   /*<U0074>       LATIN SMALL LETTER T*/
/* 75 */  CH_L,   /*<U0075>       LATIN SMALL LETTER U*/
/* 76 */  CH_L,   /*<U0076>       LATIN SMALL LETTER V*/
/* 77 */  CH_L,   /*<U0077>       LATIN SMALL LETTER W*/
/* 78 */  CH_L,   /*<U0078>       LATIN SMALL LETTER X*/
/* 79 */  CH_L,   /*<U0079>       LATIN SMALL LETTER Y*/
/* 7A */  CH_L,   /*<U007A>       LATIN SMALL LETTER Z*/
/* 7B */  CH_N,   /*<U007B>       OPENING CURLY BRACKET*/
/* 7C */  CH_N,   /*<U007C>       VERTICAL BAR*/
/* 7D */  CH_N,   /*<U007D>       CLOSING CURLY BRACKET*/
/* 7E */  CH_N,   /*<U007E>       SPACING TILDE*/
/* 7F */  CH_N,   /*<U001A>       SUBSTITUTE*/
/* 80 */  CH_R | CH_HEBLET,   /*<U05D0>       HEBREW LETTER ALEF*/
/* 81 */  CH_R | CH_HEBLET,   /*<U05D1>       HEBREW LETTER BET*/
/* 82 */  CH_R | CH_HEBLET,   /*<U05D2>       HEBREW LETTER GIMEL*/
/* 83 */  CH_R | CH_HEBLET,   /*<U05D3>       HEBREW LETTER DALET*/
/* 84 */  CH_R | CH_HEBLET,   /*<U05D4>       HEBREW LETTER HE*/
/* 85 */  CH_R | CH_HEBLET,   /*<U05D5>       HEBREW LETTER VAV*/
/* 86 */  CH_R | CH_HEBLET,   /*<U05D6>       HEBREW LETTER ZAYIN*/
/* 87 */  CH_R | CH_HEBLET,   /*<U05D7>       HEBREW LETTER HET*/
/* 88 */  CH_R | CH_HEBLET,   /*<U05D8>       HEBREW LETTER TET*/
/* 89 */  CH_R | CH_HEBLET,   /*<U05D9>       HEBREW LETTER YOD*/
/* 8A */  CH_R | CH_HEBLET,   /*<U05DA>       HEBREW LETTER FINAL KAF*/
/* 8B */  CH_R | CH_HEBLET,   /*<U05DB>       HEBREW LETTER KAF*/
/* 8C */  CH_R | CH_HEBLET,   /*<U05DC>       HEBREW LETTER LAMED*/
/* 8D */  CH_R | CH_HEBLET,   /*<U05DD>       HEBREW LETTER FINAL MEM*/
/* 8E */  CH_R | CH_HEBLET,   /*<U05DE>       HEBREW LETTER MEM*/
/* 8F */  CH_R | CH_HEBLET,   /*<U05DF>       HEBREW LETTER FINAL NUN*/
/* 90 */  CH_R | CH_HEBLET,   /*<U05E0>       HEBREW LETTER NUN*/
/* 91 */  CH_R | CH_HEBLET,   /*<U05E1>       HEBREW LETTER SAMEKH*/
/* 92 */  CH_R | CH_HEBLET,   /*<U05E2>       HEBREW LETTER AYIN*/
/* 93 */  CH_R | CH_HEBLET,   /*<U05E3>       HEBREW LETTER FINAL PE*/
/* 94 */  CH_R | CH_HEBLET,   /*<U05E4>       HEBREW LETTER PE*/
/* 95 */  CH_R | CH_HEBLET,   /*<U05E5>       HEBREW LETTER FINAL TSADI*/
/* 96 */  CH_R | CH_HEBLET,   /*<U05E6>       HEBREW LETTER TSADI*/
/* 97 */  CH_R | CH_HEBLET,   /*<U05E7>       HEBREW LETTER QOF*/
/* 98 */  CH_R | CH_HEBLET,   /*<U05E8>       HEBREW LETTER RESH*/
/* 99 */  CH_R | CH_HEBLET,   /*<U05E9>       HEBREW LETTER SHIN*/
/* 9A */  CH_R | CH_HEBLET,   /*<U05EA>       HEBREW LETTER TAV*/
/* 9B */  CH_ET,    /*<U00A2>       CENT SIGN*/
/* 9C */  CH_ET,    /*<U00A3>       POUND SIGN*/
/* 9D */  CH_ET,    /*<U00A5>       YEN SIGN*/
/* 9E */  CH_ET,    /*<U20A7>       PESETA SIGN*/
/* 9F */  CH_ET,    /*<U0192>       LATIN SMALL LETTER SCRIPT F (FLORIN SIGN)*/
/* A0 */  CH_L,   /*<U00E1>       LATIN SMALL LETTER A ACUTE*/
/* A1 */  CH_L,   /*<U00ED>       LATIN SMALL LETTER I ACUTE*/
/* A2 */  CH_L,   /*<U00F3>       LATIN SMALL LETTER O ACUTE*/
/* A3 */  CH_L,   /*<U00FA>       LATIN SMALL LETTER U ACUTE*/
/* A4 */  CH_L,   /*<U00F1>       LATIN SMALL LETTER N TILDE*/
/* A5 */  CH_L,   /*<U00D1>       LATIN CAPITAL LETTER N TILDE*/
/* A6 */  CH_N,   /*<U00AA>       FEMININE ORDINAL INDICATOR*/
/* A7 */  CH_N,   /*<U00BA>       MASCULINE ORDINAL INDICATOR*/
/* A8 */  CH_N,   /*<U00BF>       INVERTED QUESTION MARK*/
/* A9 */  CH_N,   /*<U2310>       REVERSED NOT SIGN*/
/* AA */  CH_N,   /*<U00AC>       NOT SIGN*/
/* AB */  CH_N,   /*<U00BD>       FRACTION ONE HALF*/
/* AC */  CH_N,   /*<U00BC>       FRACTION ONE QUARTER*/
/* AD */  CH_N,   /*<U00A1>       INVERTED EXCLAMATION MARK*/
/* AE */  CH_N,   /*<U00AB>       LEFT POINTING GUILLEMET*/
/* AF */  CH_N,   /*<U00BB>       RIGHT POINTING GUILLEMET*/
/* B0 */  CH_N,   /*<U2591>       LIGHT SHADE*/
/* B1 */  CH_N,   /*<U2592>       MEDIUM SHADE*/
/* B2 */  CH_N,   /*<U2593>       DARK SHADE*/
/* B3 */  CH_N,   /*<U2502>       FORMS LIGHT VERTICAL*/
/* B4 */  CH_N,   /*<U2524>       FORMS LIGHT VERTICAL AND LEFT*/
/* B5 */  CH_N,   /*<U2561>       FORMS VERTICAL SINGLE AND LEFT DOUBLE*/
/* B6 */  CH_N,   /*<U2562>       FORMS VERTICAL DOUBLE AND LEFT SINGLE*/
/* B7 */  CH_N,   /*<U2556>       FORMS DOWN DOUBLE AND LEFT SINGLE*/
/* B8 */  CH_N,   /*<U2555>       FORMS DOWN SINGLE AND LEFT DOUBLE*/
/* B9 */  CH_N,   /*<U2563>       FORMS DOUBLE VERTICAL AND LEFT*/
/* BA */  CH_N,   /*<U2551>       FORMS DOUBLE VERTICAL*/
/* BB */  CH_N,   /*<U2557>       FORMS DOUBLE DOWN AND LEFT*/
/* BC */  CH_N,   /*<U255D>       FORMS DOUBLE UP AND LEFT*/
/* BD */  CH_N,   /*<U255C>       FORMS UP DOUBLE AND LEFT SINGLE*/
/* BE */  CH_N,   /*<U255B>       FORMS UP SINGLE AND LEFT DOUBLE*/
/* BF */  CH_N,   /*<U2510>       FORMS LIGHT DOWN AND LEFT*/
/* C0 */  CH_N,   /*<U2514>       FORMS LIGHT UP AND RIGHT*/
/* C1 */  CH_N,   /*<U2534>       FORMS LIGHT UP AND HORIZONTAL*/
/* C2 */  CH_N,   /*<U252C>       FORMS LIGHT DOWN AND HORIZONTAL*/
/* C3 */  CH_N,   /*<U251C>       FORMS LIGHT VERTICAL AND RIGHT*/
/* C4 */  CH_N,   /*<U2500>       FORMS LIGHT HORIZONTAL*/
/* C5 */  CH_N,   /*<U253C>       FORMS LIGHT VERTICAL AND HORIZONTAL*/
/* C6 */  CH_N,   /*<U255E>       FORMS VERTICAL SINGLE AND RIGHT DOUBLE*/
/* C7 */  CH_N,   /*<U255F>       FORMS VERTICAL DOUBLE AND RIGHT SINGLE*/
/* C8 */  CH_N,   /*<U255A>       FORMS DOUBLE UP AND RIGHT*/
/* C9 */  CH_N,   /*<U2554>       FORMS DOUBLE DOWN AND RIGHT*/
/* CA */  CH_N,   /*<U2569>       FORMS DOUBLE UP AND HORIZONTAL*/
/* CB */  CH_N,   /*<U2566>       FORMS DOUBLE DOWN AND HORIZONTAL*/
/* CC */  CH_N,   /*<U2560>       FORMS DOUBLE VERTICAL AND RIGHT*/
/* CD */  CH_N,   /*<U2550>       FORMS DOUBLE HORIZONTAL*/
/* CE */  CH_N,   /*<U256C>       FORMS DOUBLE VERTICAL AND HORIZONTAL*/
/* CF */  CH_N,   /*<U2567>       FORMS UP SINGLE AND HORIZONTAL DOUBLE*/
/* D0 */  CH_N,   /*<U2568>       FORMS UP DOUBLE AND HORIZONTAL SINGLE*/
/* D1 */  CH_N,   /*<U2564>       FORMS DOWN SINGLE AND HORIZONTAL DOUBLE*/
/* D2 */  CH_N,   /*<U2565>       FORMS DOWN DOUBLE AND HORIZONTAL SINGLE*/
/* D3 */  CH_N,   /*<U2559>       FORMS UP DOUBLE AND RIGHT SINGLE*/
/* D4 */  CH_N,   /*<U2558>       FORMS UP SINGLE AND RIGHT DOUBLE*/
/* D5 */  CH_N,   /*<U2552>       FORMS DOWN SINGLE AND RIGHT DOUBLE*/
/* D6 */  CH_N,   /*<U2553>       FORMS DOWN DOUBLE AND RIGHT SINGLE*/
/* D7 */  CH_N,   /*<U256B>       FORMS VERTICAL DOUBLE AND HORIZONTAL SINGLE*/
/* D8 */  CH_N,   /*<U256A>       FORMS VERTICAL SINGLE AND HORIZONTAL DOUBLE*/
/* D9 */  CH_N,   /*<U2518>       FORMS LIGHT UP AND LEFT*/
/* DA */  CH_N,   /*<U250C>       FORMS LIGHT DOWN AND RIGHT*/
/* DB */  CH_N,   /*<U2588>       FULL BLOCK*/
/* DC */  CH_N,   /*<U2584>       LOWER HALF BLOCK*/
/* DD */  CH_N,   /*<U258C>       LEFT HALF BLOCK*/
/* DE */  CH_N,   /*<U2590>       RIGHT HALF BLOCK*/
/* DF */  CH_N,   /*<U2580>       UPPER HALF BLOCK*/
/* E0 */  CH_L,   /*<U03B1>       GREEK SMALL LETTER ALPHA*/
/* E1 */  CH_L,   /*<U00DF>       LATIN SMALL LETTER SHARP S*/
/* E2 */  CH_L,   /*<U0393>       GREEK CAPITAL LETTER GAMMA*/
/* E3 */  CH_L,   /*<U03C0>       GREEK SMALL LETTER PI*/
/* E4 */  CH_L,   /*<U03A3>       GREEK CAPITAL LETTER SIGMA*/
/* E5 */  CH_L,   /*<U03C3>       GREEK SMALL LETTER SIGMA*/
/* E6 */  CH_L,   /*<U03BC>       GREEK SMALL LETTER MU*/
/* E7 */  CH_L,   /*<U03C4>       GREEK SMALL LETTER TAU*/
/* E8 */  CH_L,   /*<U03A6>       GREEK CAPITAL LETTER PHI*/
/* E9 */  CH_L,   /*<U0398>       GREEK CAPITAL LETTER THETA*/
/* EA */  CH_L,   /*<U03A9>       GREEK CAPITAL LETTER OMEGA*/
/* EB */  CH_L,   /*<U03B4>       GREEK SMALL LETTER DELTA*/
/* EC */  CH_N,   /*<U221E>       INFINITY*/
/* ED */  CH_L,   /*<U03C6>       GREEK SMALL LETTER PHI*/
/* EE */  CH_L,   /*<U03B5>       GREEK SMALL LETTER EPSILON*/
/* EF */  CH_N,   /*<U2229>       INTERSECTION*/
/* F0 */  CH_N,   /*<U2261>       IDENTICAL TO*/
/* F1 */  CH_ET,    /*<U00B1>       PLUS-OR-MINUS SIGN*/
/* F2 */  CH_N,   /*<U2265>       GREATER THAN OR EQUAL TO*/
/* F3 */  CH_N,   /*<U2264>       LESS THAN OR EQUAL TO*/
/* F4 */  CH_N,   /*<U2320>       TOP HALF INTEGRAL*/
/* F5 */  CH_N,   /*<U2321>       BOTTOM HALF INTEGRAL*/
/* F6 */  CH_N,   /*<U00F7>       DIVISION SIGN*/
/* F7 */  CH_N,   /*<U2248>       ALMOST EQUAL TO*/
/* F8 */  CH_ET,    /*<U00B0>       DEGREE SIGN*/
/* F9 */  CH_N,   /*<U2219>       DOT OPERATOR*/
/* FA */  CH_N,   /*<U00B7>       MIDDLE DOT*/
/* FB */  CH_N,   /*<U221A>       SQUARE ROOT*/
/* FC */  CH_N,   /*<U207F>       SUPERSCRIPT LATIN SMALL LETTER N*/
/* FD */  CH_EN,    /*<U00B2>       SUPERSCRIPT DIGIT TWO*/
/* FE */  CH_N,   /*<U25A0>       BLACK SQUARE*/
/* FF */  CH_W    /*<U00A0>       NON-BREAKING SPACE*/
  },
  /*856 AIX Hebrew*/
  {
/* 00 */  CH_N,   /*<U0000>       NULL*/
/* 01 */  CH_N,   /*<U0001>       START OF HEADING*/
/* 02 */  CH_N,   /*<U0002>       START OF TEXT*/
/* 03 */  CH_N,   /*<U0003>       END OF TEXT*/
/* 04 */  CH_N,   /*<U0004>       END OF TRANSMISSION*/
/* 05 */  CH_N,   /*<U0005>       ENQUIRY*/
/* 06 */  CH_N,   /*<U0006>       ACKNOWLEDGE*/
/* 07 */  CH_N,   /*<U0007>       BELL*/
/* 08 */  CH_N,   /*<U0008>       BACKSPACE*/
/* 09 */  CH_S,   /*<U0009>       HORIZONTAL TABULATION*/
/* 0A */  CH_N,   /*<U000A>       LINE FEED*/
/* 0B */  CH_N,   /*<U000B>       VERTICAL TABULATION*/
/* 0C */  CH_N,   /*<U000C>       FORM FEED*/
/* 0D */  CH_N,   /*<U000D>       CARRIAGE RETURN*/
/* 0E */  CH_N,   /*<U000E>       SHIFT OUT*/
/* 0F */  CH_N,   /*<U000F>       SHIFT IN*/
/* 10 */  CH_N,   /*<U0010>       DATA LINK ESCAPE*/
/* 11 */  CH_N,   /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */  CH_N,   /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */  CH_N,   /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */  CH_N,   /*<U0014>       DEVICE CONTROL FOUR*/
/* 15 */  CH_N,   /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 16 */  CH_N,   /*<U0016>       SYNCHRONOUS IDLE*/
/* 17 */  CH_N,   /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 18 */  CH_N,   /*<U0018>       CANCEL*/
/* 19 */  CH_N,   /*<U0019>       END OF MEDIUM*/
/* 1A */  CH_N,   /*<U001C>       FILE SEPARATOR*/
/* 1B */  CH_N,   /*<U001B>       ESCAPE*/
/* 1C */  CH_N,   /*<U007F>       DELETE*/
/* 1D */  CH_N,   /*<U001D>       GROUP SEPARATOR*/
/* 1E */  CH_N,   /*<U001E>       RECORD SEPARATOR*/
/* 1F */  CH_N,   /*<U001F>       UNIT SEPARATOR*/
/* 20 */  CH_W,   /*<U0020>       SPACE*/
/* 21 */  CH_N,   /*<U0021>       EXCLAMATION MARK*/
/* 22 */  CH_N,   /*<U0022>       QUOTATION MARK*/
/* 23 */  CH_ET,    /*<U0023>       NUMBER SIGN*/
/* 24 */  CH_ET,    /*<U0024>       DOLLAR SIGN*/
/* 25 */  CH_ET,    /*<U0025>       PERCENT SIGN*/
/* 26 */  CH_N,   /*<U0026>       AMPERSAND*/
/* 27 */  CH_N,   /*<U0027>       APOSTROPHE*/
/* 28 */  CH_N,   /*<U0028>       OPENING PARENTHESIS*/
/* 29 */  CH_N,   /*<U0029>       CLOSING PARENTHESIS*/
/* 2A */  CH_N,   /*<U002A>       ASTERISK*/
/* 2B */  CH_ET,    /*<U002B>       PLUS SIGN*/
/* 2C */  CH_CS,    /*<U002C>       COMMA*/
/* 2D */  CH_ET,    /*<U002D>       HYPHEN-MINUS*/
/* 2E */  CH_ES,    /*<U002E>       PERIOD*/
/* 2F */  CH_ES,    /*<U002F>       SLASH*/
/* 30 */  CH_EN | CH_EDIGIT,    /*<U0030>       DIGIT ZERO*/
/* 31 */  CH_EN | CH_EDIGIT,    /*<U0031>       DIGIT ONE*/
/* 32 */  CH_EN | CH_EDIGIT,    /*<U0032>       DIGIT TWO*/
/* 33 */  CH_EN | CH_EDIGIT,    /*<U0033>       DIGIT THREE*/
/* 34 */  CH_EN | CH_EDIGIT,    /*<U0034>       DIGIT FOUR*/
/* 35 */  CH_EN | CH_EDIGIT,    /*<U0035>       DIGIT FIVE*/
/* 36 */  CH_EN | CH_EDIGIT,    /*<U0036>       DIGIT SIX*/
/* 37 */  CH_EN | CH_EDIGIT,    /*<U0037>       DIGIT SEVEN*/
/* 38 */  CH_EN | CH_EDIGIT,    /*<U0038>       DIGIT EIGHT*/
/* 39 */  CH_EN | CH_EDIGIT,    /*<U0039>       DIGIT NINE*/
/* 3A */  CH_CS,    /*<U003A>       COLON*/
/* 3B */  CH_N,   /*<U003B>       SEMICOLON*/
/* 3C */  CH_N,   /*<U003C>       LESS-THAN SIGN*/
/* 3D */  CH_N,   /*<U003D>       EQUALS SIGN*/
/* 3E */  CH_N,   /*<U003E>       GREATER-THAN SIGN*/
/* 3F */  CH_N,   /*<U003F>       QUESTION MARK*/
/* 40 */  CH_N,   /*<U0040>       COMMERCIAL AT*/
/* 41 */  CH_L,   /*<U0041>       LATIN CAPITAL LETTER A*/
/* 42 */  CH_L,   /*<U0042>       LATIN CAPITAL LETTER B*/
/* 43 */  CH_L,   /*<U0043>       LATIN CAPITAL LETTER C*/
/* 44 */  CH_L,   /*<U0044>       LATIN CAPITAL LETTER D*/
/* 45 */  CH_L,   /*<U0045>       LATIN CAPITAL LETTER E*/
/* 46 */  CH_L,   /*<U0046>       LATIN CAPITAL LETTER F*/
/* 47 */  CH_L,   /*<U0047>       LATIN CAPITAL LETTER G*/
/* 48 */  CH_L,   /*<U0048>       LATIN CAPITAL LETTER H*/
/* 49 */  CH_L,   /*<U0049>       LATIN CAPITAL LETTER I*/
/* 4A */  CH_L,   /*<U004A>       LATIN CAPITAL LETTER J*/
/* 4B */  CH_L,   /*<U004B>       LATIN CAPITAL LETTER K*/
/* 4C */  CH_L,   /*<U004C>       LATIN CAPITAL LETTER L*/
/* 4D */  CH_L,   /*<U004D>       LATIN CAPITAL LETTER M*/
/* 4E */  CH_L,   /*<U004E>       LATIN CAPITAL LETTER N*/
/* 4F */  CH_L,   /*<U004F>       LATIN CAPITAL LETTER O*/
/* 50 */  CH_L,   /*<U0050>       LATIN CAPITAL LETTER P*/
/* 51 */  CH_L,   /*<U0051>       LATIN CAPITAL LETTER Q*/
/* 52 */  CH_L,   /*<U0052>       LATIN CAPITAL LETTER R*/
/* 53 */  CH_L,   /*<U0053>       LATIN CAPITAL LETTER S*/
/* 54 */  CH_L,   /*<U0054>       LATIN CAPITAL LETTER T*/
/* 55 */  CH_L,   /*<U0055>       LATIN CAPITAL LETTER U*/
/* 56 */  CH_L,   /*<U0056>       LATIN CAPITAL LETTER V*/
/* 57 */  CH_L,   /*<U0057>       LATIN CAPITAL LETTER W*/
/* 58 */  CH_L,   /*<U0058>       LATIN CAPITAL LETTER X*/
/* 59 */  CH_L,   /*<U0059>       LATIN CAPITAL LETTER Y*/
/* 5A */  CH_L,   /*<U005A>       LATIN CAPITAL LETTER Z*/
/* 5B */  CH_N,   /*<U005B>       OPENING SQUARE BRACKET*/
/* 5C */  CH_N,   /*<U005C>       BACKSLASH*/
/* 5D */  CH_N,   /*<U005D>       CLOSING SQUARE BRACKET*/
/* 5E */  CH_N,   /*<U005E>       SPACING CIRCUMFLEX*/
/* 5F */  CH_N,   /*<U005F>       SPACING UNDERSCORE*/
/* 60 */  CH_N,   /*<U0060>       SPACING GRAVE*/
/* 61 */  CH_L,   /*<U0061>       LATIN SMALL LETTER A*/
/* 62 */  CH_L,   /*<U0062>       LATIN SMALL LETTER B*/
/* 63 */  CH_L,   /*<U0063>       LATIN SMALL LETTER C*/
/* 64 */  CH_L,   /*<U0064>       LATIN SMALL LETTER D*/
/* 65 */  CH_L,   /*<U0065>       LATIN SMALL LETTER E*/
/* 66 */  CH_L,   /*<U0066>       LATIN SMALL LETTER F*/
/* 67 */  CH_L,   /*<U0067>       LATIN SMALL LETTER G*/
/* 68 */  CH_L,   /*<U0068>       LATIN SMALL LETTER H*/
/* 69 */  CH_L,   /*<U0069>       LATIN SMALL LETTER I*/
/* 6A */  CH_L,   /*<U006A>       LATIN SMALL LETTER J*/
/* 6B */  CH_L,   /*<U006B>       LATIN SMALL LETTER K*/
/* 6C */  CH_L,   /*<U006C>       LATIN SMALL LETTER L*/
/* 6D */  CH_L,   /*<U006D>       LATIN SMALL LETTER M*/
/* 6E */  CH_L,   /*<U006E>       LATIN SMALL LETTER N*/
/* 6F */  CH_L,   /*<U006F>       LATIN SMALL LETTER O*/
/* 70 */  CH_L,   /*<U0070>       LATIN SMALL LETTER P*/
/* 71 */  CH_L,   /*<U0071>       LATIN SMALL LETTER Q*/
/* 72 */  CH_L,   /*<U0072>       LATIN SMALL LETTER R*/
/* 73 */  CH_L,   /*<U0073>       LATIN SMALL LETTER S*/
/* 74 */  CH_L,   /*<U0074>       LATIN SMALL LETTER T*/
/* 75 */  CH_L,   /*<U0075>       LATIN SMALL LETTER U*/
/* 76 */  CH_L,   /*<U0076>       LATIN SMALL LETTER V*/
/* 77 */  CH_L,   /*<U0077>       LATIN SMALL LETTER W*/
/* 78 */  CH_L,   /*<U0078>       LATIN SMALL LETTER X*/
/* 79 */  CH_L,   /*<U0079>       LATIN SMALL LETTER Y*/
/* 7A */  CH_L,   /*<U007A>       LATIN SMALL LETTER Z*/
/* 7B */  CH_N,   /*<U007B>       OPENING CURLY BRACKET*/
/* 7C */  CH_N,   /*<U007C>       VERTICAL BAR*/
/* 7D */  CH_N,   /*<U007D>       CLOSING CURLY BRACKET*/
/* 7E */  CH_N,   /*<U007E>       SPACING TILDE*/
/* 7F */  CH_N,   /*<U001A>       SUBSTITUTE*/
/* 80 */  CH_R | CH_HEBLET,   /*<U05D0>       HEBREW LETTER ALEF*/
/* 81 */  CH_R | CH_HEBLET,   /*<U05D1>       HEBREW LETTER BET*/
/* 82 */  CH_R | CH_HEBLET,   /*<U05D2>       HEBREW LETTER GIMEL*/
/* 83 */  CH_R | CH_HEBLET,   /*<U05D3>       HEBREW LETTER DALET*/
/* 84 */  CH_R | CH_HEBLET,   /*<U05D4>       HEBREW LETTER HE*/
/* 85 */  CH_R | CH_HEBLET,   /*<U05D5>       HEBREW LETTER VAV*/
/* 86 */  CH_R | CH_HEBLET,   /*<U05D6>       HEBREW LETTER ZAYIN*/
/* 87 */  CH_R | CH_HEBLET,   /*<U05D7>       HEBREW LETTER HET*/
/* 88 */  CH_R | CH_HEBLET,   /*<U05D8>       HEBREW LETTER TET*/
/* 89 */  CH_R | CH_HEBLET,   /*<U05D9>       HEBREW LETTER YOD*/
/* 8A */  CH_R | CH_HEBLET,   /*<U05DA>       HEBREW LETTER FINAL KAF*/
/* 8B */  CH_R | CH_HEBLET,   /*<U05DB>       HEBREW LETTER KAF*/
/* 8C */  CH_R | CH_HEBLET,   /*<U05DC>       HEBREW LETTER LAMED*/
/* 8D */  CH_R | CH_HEBLET,   /*<U05DD>       HEBREW LETTER FINAL MEM*/
/* 8E */  CH_R | CH_HEBLET,   /*<U05DE>       HEBREW LETTER MEM*/
/* 8F */  CH_R | CH_HEBLET,   /*<U05DF>       HEBREW LETTER FINAL NUN*/
/* 90 */  CH_R | CH_HEBLET,   /*<U05E0>       HEBREW LETTER NUN*/
/* 91 */  CH_R | CH_HEBLET,   /*<U05E1>       HEBREW LETTER SAMEKH*/
/* 92 */  CH_R | CH_HEBLET,   /*<U05E2>       HEBREW LETTER AYIN*/
/* 93 */  CH_R | CH_HEBLET,   /*<U05E3>       HEBREW LETTER FINAL PE*/
/* 94 */  CH_R | CH_HEBLET,   /*<U05E4>       HEBREW LETTER PE*/
/* 95 */  CH_R | CH_HEBLET,   /*<U05E5>       HEBREW LETTER FINAL TSADI*/
/* 96 */  CH_R | CH_HEBLET,   /*<U05E6>       HEBREW LETTER TSADI*/
/* 97 */  CH_R | CH_HEBLET,   /*<U05E7>       HEBREW LETTER QOF*/
/* 98 */  CH_R | CH_HEBLET,   /*<U05E8>       HEBREW LETTER RESH*/
/* 99 */  CH_R | CH_HEBLET,   /*<U05E9>       HEBREW LETTER SHIN*/
/* 9A */  CH_R | CH_HEBLET,   /*<U05EA>       HEBREW LETTER TAV*/
/* 9B */  CH_ET,    /*NEW SHEKELS SIGN*/
/* 9C */  CH_ET,    /*POUNDS SIGN*/
/* 9D */  CH_ET,    /*CENTS SIGN*/
/* 9E */  CH_N,   /*<U00D7>       MULTIPLICATION SIGN*/
/* 9F */  CH_N,   /* *** UNASSIGNED*/
/* A0 */  CH_N,   /* *** UNASSIGNED*/
/* A1 */  CH_N,   /* *** UNASSIGNED*/
/* A2 */  CH_N,   /* *** UNASSIGNED*/
/* A3 */  CH_N,   /* *** UNASSIGNED*/
/* A4 */  CH_N,   /* *** UNASSIGNED*/
/* A5 */  CH_N,   /* *** UNASSIGNED*/
/* A6 */  CH_N,   /* *** UNASSIGNED*/
/* A7 */  CH_N,   /* *** UNASSIGNED*/
/* A8 */  CH_N,   /* *** UNASSIGNED*/
/* A9 */  CH_N,   /*<U00AE>       REGISTERED TRADE MARK SIGN*/
/* AA */  CH_N,   /*<U00AC>       NOT SIGN*/
/* AB */  CH_N,   /*<U00BD>       FRACTION ONE HALF*/
/* AC */  CH_N,   /*<U00BC>       FRACTION ONE QUARTER*/
/* AD */  CH_N,   /* *** UNASSIGNED*/
/* AE */  CH_N,   /*<U00AB>       LEFT POINTING GUILLEMET*/
/* AF */  CH_N,   /*<U00BB>       RIGHT POINTING GUILLEMET*/
/* B0 */  CH_N,   /*<U2591>       LIGHT SHADE*/
/* B1 */  CH_N,   /*<U2592>       MEDIUM SHADE*/
/* B2 */  CH_N,   /*<U2593>       DARK SHADE*/
/* B3 */  CH_N,   /*<U2502>       FORMS LIGHT VERTICAL*/
/* B4 */  CH_N,   /*<U2524>       FORMS LIGHT VERTICAL AND LEFT*/
/* B5 */  CH_N,   /* *** UNASSIGNED*/
/* B6 */  CH_N,   /* *** UNASSIGNED*/
/* B7 */  CH_N,   /* *** UNASSIGNED*/
/* B8 */  CH_N,   /*<U00A9>       COPYRIGHT SIGN*/
/* B9 */  CH_ET,    /*YEN SIGN*/
/* BA */  CH_N,   /*<U2551>       FORMS DOUBLE VERTICAL*/
/* BB */  CH_N,   /*<U2557>       FORMS DOUBLE DOWN AND LEFT*/
/* BC */  CH_N,   /*<U255D>       FORMS DOUBLE UP AND LEFT*/
/* BD */  CH_N,   /*<U00A2>       CENT SIGN*/
/* BE */  CH_N,   /*<U00A5>       YEN SIGN*/
/* BF */  CH_N,   /*<U2510>       FORMS LIGHT DOWN AND LEFT*/
/* C0 */  CH_N,   /*<U2514>       FORMS LIGHT UP AND RIGHT*/
/* C1 */  CH_N,   /*<U2534>       FORMS LIGHT UP AND HORIZONTAL*/
/* C2 */  CH_N,   /*<U252C>       FORMS LIGHT DOWN AND HORIZONTAL*/
/* C3 */  CH_N,   /*<U251C>       FORMS LIGHT VERTICAL AND RIGHT*/
/* C4 */  CH_N,   /*<U2500>       FORMS LIGHT HORIZONTAL*/
/* C5 */  CH_N,   /*<U253C>       FORMS LIGHT VERTICAL AND HORIZONTAL*/
/* C6 */  CH_N,   /* *** UNASSIGNED*/
/* C7 */  CH_N,   /* *** UNASSIGNED*/
/* C8 */  CH_N,   /*<U255A>       FORMS DOUBLE UP AND RIGHT*/
/* C9 */  CH_N,   /*<U2554>       FORMS DOUBLE DOWN AND RIGHT*/
/* CA */  CH_N,   /*<U2569>       FORMS DOUBLE UP AND HORIZONTAL*/
/* CB */  CH_N,   /*<U2566>       FORMS DOUBLE DOWN AND HORIZONTAL*/
/* CC */  CH_N,   /*<U2560>       FORMS DOUBLE VERTICAL AND RIGHT*/
/* CD */  CH_N,   /*<U2550>       FORMS DOUBLE HORIZONTAL*/
/* CE */  CH_N,   /*<U256C>       FORMS DOUBLE VERTICAL AND HORIZONTAL*/
/* CF */  CH_ET,    /*<U00A4>       CURRENCY SIGN*/
/* D0 */  CH_N,   /* *** UNASSIGNED*/
/* D1 */  CH_N,   /* *** UNASSIGNED*/
/* D2 */  CH_N,   /* *** UNASSIGNED*/
/* D3 */  CH_N,   /* *** UNASSIGNED*/
/* D4 */  CH_N,   /* *** UNASSIGNED*/
/* D5 */  CH_N,   /* *** UNASSIGNED*/
/* D6 */  CH_N,   /* *** UNASSIGNED*/
/* D7 */  CH_N,   /* *** UNASSIGNED*/
/* D8 */  CH_N,   /* *** UNASSIGNED*/
/* D9 */  CH_N,   /*<U2518>       FORMS LIGHT UP AND LEFT*/
/* DA */  CH_N,   /*<U250C>       FORMS LIGHT DOWN AND RIGHT*/
/* DB */  CH_N,   /*<U2588>       FULL BLOCK*/
/* DC */  CH_N,   /*<U2584>       LOWER HALF BLOCK*/
/* DD */  CH_N,   /*<U00A6>       BROKEN VERTICAL BAR*/
/* DE */  CH_N,   /* **UNASSIGNED*/
/* DF */  CH_N,   /*<U2580>       UPPER HALF BLOCK*/
/* E0 */  CH_N,   /* *** UNASSIGNED*/
/* E1 */  CH_N,   /* *** UNASSIGNED*/
/* E2 */  CH_N,   /* *** UNASSIGNED*/
/* E3 */  CH_N,   /* *** UNASSIGNED*/
/* E4 */  CH_N,   /* *** UNASSIGNED*/
/* E5 */  CH_N,   /* *** UNASSIGNED*/
/* E6 */  CH_N,   /*<U00B5>       MICRO SIGN*/
/* E7 */  CH_L | CH_BIDICTRL,   /*LEFT TO RIGHT MARKER*/
/* E8 */  CH_R | CH_BIDICTRL,   /*RIGHT TO LEFT MARKER*/
/* E9 */  CH_N | CH_BIDICTRL,     /*LEFT TO RIGHT EMBED*/
/* EA */  CH_N | CH_BIDICTRL,   /*RIGHT TO LEFT EMBED*/
/* EB */  CH_N | CH_BIDICTRL,   /*LEFT TO RIGHT OVERRIDE*/
/* EC */  CH_N | CH_BIDICTRL,   /*RIGHT TO LEFT OVERRIDE*/
/* ED */  CH_N | CH_BIDICTRL,   /*POP DIRECTIONAL FORMATTING*/
/* EE */  CH_N,   /*<U00AF>       SPACING MACRON*/
/* EF */  CH_N,   /*<U00B4>       SPACING ACUTE*/
/* F0 */  CH_N,   /*<U00AD>       SOFT HYPHEN*/
/* F1 */  CH_ET,    /*<U00B1>       PLUS-OR-MINUS SIGN*/
/* F2 */  CH_N,   /*<U2017>       SPACING DOUBLE UNDERSCORE*/
/* F3 */  CH_N,   /*<U00BE>       FRACTION THREE QUARTERS*/
/* F4 */  CH_N,   /*<U00B6>       PARAGRAPH SIGN*/
/* F5 */  CH_N,   /*<U00A7>       SECTION SIGN*/
/* F6 */  CH_N,   /*<U00F7>       DIVISION SIGN*/
/* F7 */  CH_N,   /*<U00B8>       SPACING CEDILLA*/
/* F8 */  CH_ET,    /*<U00B0>       DEGREE SIGN*/
/* F9 */  CH_N,   /*<U00A8>       SPACING DIAERESIS*/
/* FA */  CH_N,   /*<U2022>       BULLET*/
/* FB */  CH_EN,    /*<U00B9>       SUPERSCRIPT DIGIT ONE*/
/* FC */  CH_EN,    /*<U00B3>       SUPERSCRIPT DIGIT THREE*/
/* FD */  CH_EN,    /*<U00B2>       SUPERSCRIPT DIGIT TWO*/
/* FE */  CH_N,   /*<U25A0>       BLACK SQUARE*/
/* FF */  CH_W    /*<U00A0>       NON-BREAKING SPACE*/
  },
  /* 916 AIX Hebrew = ISO 8859-8*/
  {
/* 00 */  CH_N,   /*<U0000> (CC) Null*/
/* 01 */  CH_N,   /*<U0001> (CC) Start of Heading*/
/* 02 */  CH_N,   /*<U0002> (CC) Start of Text*/
/* 03 */  CH_N,   /*<U0003> (CC) End of Text*/
/* 04 */  CH_N,   /*<U0004> (CC) End of Transmission*/
/* 05 */  CH_N,   /*<U0005> (CC) Enquiry*/
/* 06 */  CH_N,   /*<U0006> (CC) Acknowledge*/
/* 07 */  CH_N,   /*<U0007> (CC) Bell*/
/* 08 */  CH_N,   /*<U0008> (CC) Backspace*/
/* 09 */  CH_S,   /*<U0009> (CC) Horizontal Tabulation*/
/* 0A */  CH_N,   /*<U000A> (CC) Line Feed*/
/* 0B */  CH_N,   /*<U000B> (CC) Vertical Tabulation*/
/* 0C */  CH_N,   /*<U000C> (CC) Form Feed*/
/* 0D */  CH_N,   /*<U000D> (CC) Carriage Return*/
/* 0E */  CH_N,   /*<U000E> (CC) Locking Shift One (Shift Out*/
/* 0F */  CH_N,   /*<U000F> (CC) Locking Shift Zero (Shift In*/
/* 10 */  CH_N,   /*<U0010> (CC) Data Link Escape*/
/* 11 */  CH_N,   /*<U0011> (CC) Device Control One*/
/* 12 */  CH_N,   /*<U0012> (CC) Device Control Two*/
/* 13 */  CH_N,   /*<U0013> (CC) Device Control Three*/
/* 14 */  CH_N,   /*<U0014> (CC) Device Control Four*/
/* 15 */  CH_N,   /*<U0015> (CC) Negative Acknowledge*/
/* 16 */  CH_N,   /*<U0016> (CC) Synchronous Idle*/
/* 17 */  CH_N,   /*<U0017> (CC) End of Transmission Block*/
/* 18 */  CH_N,   /*<U0018> (CC) Cancel*/
/* 19 */  CH_N,   /*<U0019> (CC) End of Medium*/
/* 1A */  CH_N,   /*<U001A> (CC) Substitute*/
/* 1B */  CH_N,   /*<U001B> (CC) Escape*/
/* 1C */  CH_N,   /*<U001C> (CC) Information File Separator*/
/* 1D */  CH_N,   /*<U001D> (CC) Group Separator*/
/* 1E */  CH_N,   /*<U001E> (CC) Record Separator*/
/* 1F */  CH_N,   /*<U001F> (CC) Unit Separator*/
/* 20 */  CH_W,   /*<U0020> SPACE*/
/* 21 */  CH_N,   /*<U0021> EXCLAMATION MARK*/
/* 22 */  CH_N,   /*<U0022> QUOTATION MARK*/
/* 23 */  CH_ET,    /*<U0023> NUMBER SIGN*/
/* 24 */  CH_ET,    /*<U0024> DOLLAR SIGN*/
/* 25 */  CH_ET,    /*<U0025> PERCENT SIGN*/
/* 26 */  CH_N,   /*<U0026> AMPERSAND*/
/* 27 */  CH_N,   /*<U0027> APOSTROPHE*/
/* 28 */  CH_N,   /*<U0028> LEFT PARENTHESIS*/
/* 29 */  CH_N,   /*<U0029> RIGHT PARENTHESIS*/
/* 2A */  CH_N,   /*<U002A> ASTERISK*/
/* 2B */  CH_ET,    /*<U002B> PLUS SIGN*/
/* 2C */  CH_CS,    /*<U002C> COMMA*/
/* 2D */  CH_ET,    /*<U002D> HYPHEN-MINUS*/
/* 2E */  CH_ES,    /*<U002E> FULL STOP*/
/* 2F */  CH_ES,    /*<U002F> SOLIDUS*/
/* 30 */  CH_EN | CH_EDIGIT,    /*<U0030> DIGIT ZERO*/
/* 31 */  CH_EN | CH_EDIGIT,    /*<U0031> DIGIT ONE*/
/* 32 */  CH_EN | CH_EDIGIT,    /*<U0032> DIGIT TWO*/
/* 33 */  CH_EN | CH_EDIGIT,    /*<U0033> DIGIT THREE*/
/* 34 */  CH_EN | CH_EDIGIT,    /*<U0034> DIGIT FOUR*/
/* 35 */  CH_EN | CH_EDIGIT,    /*<U0035> DIGIT FIVE*/
/* 36 */  CH_EN | CH_EDIGIT,    /*<U0036> DIGIT SIX*/
/* 37 */  CH_EN | CH_EDIGIT,    /*<U0037> DIGIT SEVEN*/
/* 38 */  CH_EN | CH_EDIGIT,    /*<U0038> DIGIT EIGHT*/
/* 39 */  CH_EN | CH_EDIGIT,    /*<U0039> DIGIT NINE*/
/* 3A */  CH_CS,    /*<U003A> COLON*/
/* 3B */  CH_N,   /*<U003B> SEMICOLON*/
/* 3C */  CH_N,   /*<U003C> LESS-THAN SIGN*/
/* 3D */  CH_N,   /*<U003D> EQUALS SIGN*/
/* 3E */  CH_N,   /*<U003E> GREATER-THAN SIGN*/
/* 3F */  CH_N,   /*<U003F> QUESTION MARK*/
/* 40 */  CH_N,   /*<U0040> COMMERCIAL AT*/
/* 41 */  CH_L,   /*<U0041> LATIN CAPITAL LETTER A*/
/* 42 */  CH_L,   /*<U0042> LATIN CAPITAL LETTER B*/
/* 43 */  CH_L,   /*<U0043> LATIN CAPITAL LETTER C*/
/* 44 */  CH_L,   /*<U0044> LATIN CAPITAL LETTER D*/
/* 45 */  CH_L,   /*<U0045> LATIN CAPITAL LETTER E*/
/* 46 */  CH_L,   /*<U0046> LATIN CAPITAL LETTER F*/
/* 47 */  CH_L,   /*<U0047> LATIN CAPITAL LETTER G*/
/* 48 */  CH_L,   /*<U0048> LATIN CAPITAL LETTER H*/
/* 49 */  CH_L,   /*<U0049> LATIN CAPITAL LETTER I*/
/* 4A */  CH_L,   /*<U004A> LATIN CAPITAL LETTER J*/
/* 4B */  CH_L,   /*<U004B> LATIN CAPITAL LETTER K*/
/* 4C */  CH_L,   /*<U004C> LATIN CAPITAL LETTER L*/
/* 4D */  CH_L,   /*<U004D> LATIN CAPITAL LETTER M*/
/* 4E */  CH_L,   /*<U004E> LATIN CAPITAL LETTER N*/
/* 4F */  CH_L,   /*<U004F> LATIN CAPITAL LETTER O*/
/* 50 */  CH_L,   /*<U0050> LATIN CAPITAL LETTER P*/
/* 51 */  CH_L,   /*<U0051> LATIN CAPITAL LETTER Q*/
/* 52 */  CH_L,   /*<U0052> LATIN CAPITAL LETTER R*/
/* 53 */  CH_L,   /*<U0053> LATIN CAPITAL LETTER S*/
/* 54 */  CH_L,   /*<U0054> LATIN CAPITAL LETTER T*/
/* 55 */  CH_L,   /*<U0055> LATIN CAPITAL LETTER U*/
/* 56 */  CH_L,   /*<U0056> LATIN CAPITAL LETTER V*/
/* 57 */  CH_L,   /*<U0057> LATIN CAPITAL LETTER W*/
/* 58 */  CH_L,   /*<U0058> LATIN CAPITAL LETTER X*/
/* 59 */  CH_L,   /*<U0059> LATIN CAPITAL LETTER Y*/
/* 5A */  CH_L,   /*<U005A> LATIN CAPITAL LETTER Z*/
/* 5B */  CH_N,   /*<U005B> LEFT SQUARE BRACKET*/
/* 5C */  CH_N,   /*<U005C> REVERSE SOLIDUS*/
/* 5D */  CH_N,   /*<U005D> RIGHT SQUARE BRACKET*/
/* 5E */  CH_N,   /*<U005E> CIRCUMFLEX ACCENT*/
/* 5F */  CH_N,   /*<U005F> LOW LINE*/
/* 60 */  CH_N,   /*<U0060> GRAVE ACCENT*/
/* 61 */  CH_L,   /*<U0061> LATIN SMALL LETTER A*/
/* 62 */  CH_L,   /*<U0062> LATIN SMALL LETTER B*/
/* 63 */  CH_L,   /*<U0063> LATIN SMALL LETTER C*/
/* 64 */  CH_L,   /*<U0064> LATIN SMALL LETTER D*/
/* 65 */  CH_L,   /*<U0065> LATIN SMALL LETTER E*/
/* 66 */  CH_L,   /*<U0066> LATIN SMALL LETTER F*/
/* 67 */  CH_L,   /*<U0067> LATIN SMALL LETTER G*/
/* 68 */  CH_L,   /*<U0068> LATIN SMALL LETTER H*/
/* 69 */  CH_L,   /*<U0069> LATIN SMALL LETTER I*/
/* 6A */  CH_L,   /*<U006A> LATIN SMALL LETTER J*/
/* 6B */  CH_L,   /*<U006B> LATIN SMALL LETTER K*/
/* 6C */  CH_L,   /*<U006C> LATIN SMALL LETTER L*/
/* 6D */  CH_L,   /*<U006D> LATIN SMALL LETTER M*/
/* 6E */  CH_L,   /*<U006E> LATIN SMALL LETTER N*/
/* 6F */  CH_L,   /*<U006F> LATIN SMALL LETTER O*/
/* 70 */  CH_L,   /*<U0070> LATIN SMALL LETTER P*/
/* 71 */  CH_L,   /*<U0071> LATIN SMALL LETTER Q*/
/* 72 */  CH_L,   /*<U0072> LATIN SMALL LETTER R*/
/* 73 */  CH_L,   /*<U0073> LATIN SMALL LETTER S*/
/* 74 */  CH_L,   /*<U0074> LATIN SMALL LETTER T*/
/* 75 */  CH_L,   /*<U0075> LATIN SMALL LETTER U*/
/* 76 */  CH_L,   /*<U0076> LATIN SMALL LETTER V*/
/* 77 */  CH_L,   /*<U0077> LATIN SMALL LETTER W*/
/* 78 */  CH_L,   /*<U0078> LATIN SMALL LETTER X*/
/* 79 */  CH_L,   /*<U0079> LATIN SMALL LETTER Y*/
/* 7A */  CH_L,   /*<U007A> LATIN SMALL LETTER Z*/
/* 7B */  CH_N,   /*<U007B> LEFT CURLY BRACKET*/
/* 7C */  CH_N,   /*<U007C> VERTICAL LINE*/
/* 7D */  CH_N,   /*<U007D> RIGHT CURLY BRACKET*/
/* 7E */  CH_N,   /*<U007E> TILDE*/
/* 7F */  CH_N,   /*(CC) Delete*/
/* 80 */  CH_N,     /* *** UNASSIGNED*/
/* 81 */  CH_N,     /* *** UNASSIGNED*/
/* 82 */  CH_N,     /* *** UNASSIGNED*/
/* 83 */  CH_N,     /* *** UNASSIGNED*/
/* 84 */  CH_N,     /* *** UNASSIGNED*/
/* 85 */  CH_N,     /* *** UNASSIGNED*/
/* 86 */  CH_N,     /* *** UNASSIGNED*/
/* 87 */  CH_N,     /* *** UNASSIGNED*/
/* 88 */  CH_N,     /* *** UNASSIGNED*/
/* 89 */  CH_N,     /* *** UNASSIGNED*/
/* 8A */  CH_N,     /* *** UNASSIGNED*/
/* 8B */  CH_N,     /* *** UNASSIGNED*/
/* 8C */  CH_N,     /* *** UNASSIGNED*/
/* 8D */  CH_N,     /* *** UNASSIGNED*/
/* 8E */  CH_N,     /* *** UNASSIGNED*/
/* 8F */  CH_N,     /* *** UNASSIGNED*/
/* 90 */  CH_N,     /* *** UNASSIGNED*/
/* 91 */  CH_N,     /* *** UNASSIGNED*/
/* 92 */  CH_N,     /* *** UNASSIGNED*/
/* 93 */  CH_N,     /* *** UNASSIGNED*/
/* 94 */  CH_N,     /* *** UNASSIGNED*/
/* 95 */  CH_N,     /* *** UNASSIGNED*/
/* 96 */  CH_N,     /* *** UNASSIGNED*/
/* 97 */  CH_N,     /* *** UNASSIGNED*/
/* 98 */  CH_N,     /* *** UNASSIGNED*/
/* 99 */  CH_N,     /* *** UNASSIGNED*/
/* 9A */  CH_N,     /* *** UNASSIGNED*/
/* 9B */  CH_N,     /* *** UNASSIGNED*/
/* 9C */  CH_N,     /* *** UNASSIGNED*/
/* 9D */  CH_N,     /* *** UNASSIGNED*/
/* 9E */  CH_N,     /* *** UNASSIGNED*/
/* 9F */  CH_N,     /* *** UNASSIGNED*/
/* A0 */  CH_W,   /*<U00A0> NO-BREAK SPACE*/
/* A1 */  CH_N,   /* *** UNASSIGNED*/
/* A2 */  CH_ET,    /*<U00A2> CENT SIGN*/
/* A3 */  CH_ET,    /*<U00A3> POUND SIGN*/
/* A4 */  CH_ET,    /*<U00A4> CURRENCY SIGN*/
/* A5 */  CH_ET,    /*<U00A5> YEN SIGN*/
/* A6 */  CH_N,   /*<U00A6> BROKEN BAR*/
/* A7 */  CH_N,   /*<U00A7> SECTION SIGN*/
/* A8 */  CH_N,   /*<U00A8> DIAERESIS*/
/* A9 */  CH_N,   /*<U00A9> COPYRIGHT SIGN*/
/* AA */  CH_N,     /*<U00D7> MULTIPLICATION SIGN*/
/* AB */  CH_N,   /*<U00AB> LEFT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* AC */  CH_N,   /*<U00AC> NOT SIGN*/
/* AD */  CH_N,   /*<U00AD> SOFT HYPHEN*/
/* AE */  CH_N,   /*<U00AE> REGISTERED SIGN*/
/* AF */  CH_N,   /*<U203E> MACRON*/
/* B0 */  CH_ET,    /*<U00B0> DEGREE SIGN*/
/* B1 */  CH_ET,    /*<U00B1> PLUS-MINUS SIGN*/
/* B2 */  CH_EN,    /*<U00B2> SUPERSCRIPT TWO*/
/* B3 */  CH_EN,    /*<U00B3> SUPERSCRIPT THREE*/
/* B4 */  CH_N,   /*<U00B4> ACUTE ACCENT*/
/* B5 */  CH_N,   /*<U00B5> MICRO SIGN*/
/* B6 */  CH_N,   /*<U00B6> PILCROW SIGN*/
/* B7 */  CH_N,   /*<U00B7> MIDDLE DOT*/
/* B8 */  CH_N,   /*<U00B8> CEDILLA*/
/* B9 */  CH_EN,    /*<U00B9> SUPERSCRIPT ONE*/
/* BA */  CH_N,     /*<U00F7> DIVISION SIGN*/
/* BB */  CH_N,   /*<U00BB> RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* BC */  CH_N,   /*<U00BC> VULGAR FRACTION ONE QUARTER*/
/* BD */  CH_N,   /*<U00BD> VULGAR FRACTION ONE HALF*/
/* BE */  CH_N,   /*<U00BE> VULGAR FRACTION THREE QUARTERS*/
/* BF */  CH_N,   /* ***UNASSIGNED*/
/* C0 */  CH_N,   /* ***UNASSIGNED*/
/* C1 */  CH_N,   /* ***UNASSIGNED*/
/* C2 */  CH_N,   /* ***UNASSIGNED*/
/* C3 */  CH_N,   /* ***UNASSIGNED*/
/* C4 */  CH_N,   /* ***UNASSIGNED*/
/* C5 */  CH_N,   /* ***UNASSIGNED*/
/* C6 */  CH_N,   /* ***UNASSIGNED*/
/* C7 */  CH_N,   /* ***UNASSIGNED*/
/* C8 */  CH_N,   /* ***UNASSIGNED*/
/* C9 */  CH_N,   /* ***UNASSIGNED*/
/* CA */  CH_N,   /* ***UNASSIGNED*/
/* CB */  CH_N,   /* ***UNASSIGNED*/
/* CC */  CH_N,   /* ***UNASSIGNED*/
/* CD */  CH_N,   /* ***UNASSIGNED*/
/* CE */  CH_N,   /* ***UNASSIGNED*/
/* CF */  CH_N,   /* ***UNASSIGNED*/
/* D0 */  CH_N,   /* ***UNASSIGNED*/
/* D1 */  CH_N,   /* ***UNASSIGNED*/
/* D2 */  CH_N,   /* ***UNASSIGNED*/
/* D3 */  CH_N,   /* ***UNASSIGNED*/
/* D4 */  CH_N,   /* ***UNASSIGNED*/
/* D5 */  CH_N,   /* ***UNASSIGNED*/
/* D6 */  CH_N,   /* ***UNASSIGNED*/
/* D7 */  CH_N,   /* ***UNASSIGNED*/
/* D8 */  CH_N,   /* ***UNASSIGNED*/
/* D9 */  CH_N,   /* ***UNASSIGNED*/
/* DA */  CH_N,   /* ***UNASSIGNED*/
/* DB */  CH_N,   /* ***UNASSIGNED*/
/* DC */  CH_N,   /* ***UNASSIGNED*/
/* DD */  CH_N,   /* ***UNASSIGNED*/
/* DE */  CH_N,   /* ***UNASSIGNED*/
/* DF */  CH_N,   /*<U2017> DOUBLE LOW LINE*/
/* E0 */  CH_R | CH_HEBLET,   /*<U05D0> HEBREW LETTER ALEF*/
/* E1 */  CH_R | CH_HEBLET,   /*<U05D1> HEBREW LETTER BEIT*/
/* E2 */  CH_R | CH_HEBLET,   /*<U05D2> HEBREW LETTER GIMMEL*/
/* E3 */  CH_R | CH_HEBLET,   /*<U05D3> HEBREW LETTER DALET*/
/* E4 */  CH_R | CH_HEBLET,   /*<U05D4> HEBREW LETTER HE*/
/* E5 */  CH_R | CH_HEBLET,   /*<U05D5> HEBREW LETTER VAV*/
/* E6 */  CH_R | CH_HEBLET,   /*<U05D6> HEBREW LETTER ZAIN*/
/* E7 */  CH_R | CH_HEBLET,   /*<U05D7> HEBREW LETTER HET*/
/* E8 */  CH_R | CH_HEBLET,   /*<U05D8> HEBREW LETTER TET*/
/* E9 */  CH_R | CH_HEBLET,   /*<U05D9> HEBREW LETTER YOD*/
/* EA */  CH_R | CH_HEBLET,   /*<U05DA> HEBREW LETTER FINAL KAF*/
/* EB */  CH_R | CH_HEBLET,   /*<U05DB> HEBREW LETTER MEDIAL KAF*/
/* EC */  CH_R | CH_HEBLET,   /*<U05DC> HEBREW LETTER LAMED*/
/* ED */  CH_R | CH_HEBLET,   /*<U05DD> HEBREW LETTER FINAL MEM*/
/* EE */  CH_R | CH_HEBLET,   /*<U05DE> HEBREW LETTER MEDIAL MEM*/
/* EF */  CH_R | CH_HEBLET,   /*<U05DF> HEBREW LETTER FINAL NUN*/
/* F0 */  CH_R | CH_HEBLET,   /*<U05E0> HEBREW LETTER MEDIAL NUN*/
/* F1 */  CH_R | CH_HEBLET,   /*<U05E1> HEBREW LETTER SAMECH*/
/* F2 */  CH_R | CH_HEBLET,   /*<U05E2> HEBREW LETTER AIN*/
/* F3 */  CH_R | CH_HEBLET,   /*<U05E3> HEBREW LETTER FINAL PE*/
/* F4 */  CH_R | CH_HEBLET,   /*<U05E4> HEBREW LETTER MEDIAL PE*/
/* F5 */  CH_R | CH_HEBLET,   /*<U05E5> HEBREW LETTER FINAL TZADDI*/
/* F6 */  CH_R | CH_HEBLET,   /*<U05E6> HEBREW LETTER MEDIAL TZADDI*/
/* F7 */  CH_R | CH_HEBLET,   /*<U05E7> HEBREW LETTER QUF*/
/* F8 */  CH_R | CH_HEBLET,   /*<U05E8> HEBREW LETTER RESH*/
/* F9 */  CH_R | CH_HEBLET,   /*<U05E9> HEBREW LETTER SHIN*/
/* FA */  CH_R | CH_HEBLET,   /*<U05EA> HEBREW LETTER TAV*/
/* FB */  CH_N,   /* *** UNASSIGNED*/
/* FC */  CH_N,   /* *** UNASSIGNED*/
/* FD */  CH_N,   /* *** UNASSIGNED*/
/* FE */  CH_N,   /* *** UNASSIGNED*/
  },
  /* 1255 Windows Hebrew*/
  {
/* 00 */  CH_N,   /*<U0000> (CC) Null*/
/* 01 */  CH_N,   /*<U0001> (CC) Start of Heading*/
/* 02 */  CH_N,   /*<U0002> (CC) Start of Text*/
/* 03 */  CH_N,   /*<U0003> (CC) End of Text*/
/* 04 */  CH_N,   /*<U0004> (CC) End of Transmission*/
/* 05 */  CH_N,   /*<U0005> (CC) Enquiry*/
/* 06 */  CH_N,   /*<U0006> (CC) Acknowledge*/
/* 07 */  CH_N,   /*<U0007> (CC) Bell*/
/* 08 */  CH_N,   /*<U0008> (CC) Backspace*/
/* 09 */  CH_S,   /*<U0009> (CC) Horizontal Tabulation*/
/* 0A */  CH_N,   /*<U000A> (CC) Line Feed*/
/* 0B */  CH_N,   /*<U000B> (CC) Vertical Tabulation*/
/* 0C */  CH_N,   /*<U000C> (CC) Form Feed*/
/* 0D */  CH_N,   /*<U000D> (CC) Carriage Return*/
/* 0E */  CH_N,   /*<U000E> (CC) Locking Shift One (Shift Out*/
/* 0F */  CH_N,   /*<U000F> (CC) Locking Shift Zero (Shift In*/
/* 10 */  CH_N,   /*<U0010> (CC) Data Link Escape*/
/* 11 */  CH_N,   /*<U0011> (CC) Device Control One*/
/* 12 */  CH_N,   /*<U0012> (CC) Device Control Two*/
/* 13 */  CH_N,   /*<U0013> (CC) Device Control Three*/
/* 14 */  CH_N,   /*<U0014> (CC) Device Control Four*/
/* 15 */  CH_N,   /*<U0015> (CC) Negative Acknowledge*/
/* 16 */  CH_N,   /*<U0016> (CC) Synchronous Idle*/
/* 17 */  CH_N,   /*<U0017> (CC) End of Transmission Block*/
/* 18 */  CH_N,   /*<U0018> (CC) Cancel*/
/* 19 */  CH_N,   /*<U0019> (CC) End of Medium*/
/* 1A */  CH_N,   /*<U001A> (CC) Substitute*/
/* 1B */  CH_N,   /*<U001B> (CC) Escape*/
/* 1C */  CH_N,   /*<U001C> (CC) Information File Separator*/
/* 1D */  CH_N,   /*<U001D> (CC) Group Separator*/
/* 1E */  CH_N,   /*<U001E> (CC) Record Separator*/
/* 1F */  CH_N,   /*<U001F> (CC) Unit Separator*/
/* 20 */  CH_W,   /*<U0020> SPACE*/
/* 21 */  CH_N,   /*<U0021> EXCLAMATION MARK*/
/* 22 */  CH_N,   /*<U0022> QUOTATION MARK*/
/* 23 */  CH_ET,    /*<U0023> NUMBER SIGN*/
/* 24 */  CH_ET,    /*<U0024> DOLLAR SIGN*/
/* 25 */  CH_ET,    /*<U0025> PERCENT SIGN*/
/* 26 */  CH_N,   /*<U0026> AMPERSAND*/
/* 27 */  CH_N,   /*<U0027> APOSTROPHE*/
/* 28 */  CH_N,   /*<U0028> LEFT PARENTHESIS*/
/* 29 */  CH_N,   /*<U0029> RIGHT PARENTHESIS*/
/* 2A */  CH_N,   /*<U002A> ASTERISK*/
/* 2B */  CH_ET,    /*<U002B> PLUS SIGN*/
/* 2C */  CH_CS,    /*<U002C> COMMA*/
/* 2D */  CH_ET,    /*<U002D> HYPHEN-MINUS*/
/* 2E */  CH_ES,    /*<U002E> FULL STOP*/
/* 2F */  CH_ES,    /*<U002F> SOLIDUS*/
/* 30 */  CH_EN | CH_EDIGIT,    /*<U0030> DIGIT ZERO*/
/* 31 */  CH_EN | CH_EDIGIT,    /*<U0031> DIGIT ONE*/
/* 32 */  CH_EN | CH_EDIGIT,    /*<U0032> DIGIT TWO*/
/* 33 */  CH_EN | CH_EDIGIT,    /*<U0033> DIGIT THREE*/
/* 34 */  CH_EN | CH_EDIGIT,    /*<U0034> DIGIT FOUR*/
/* 35 */  CH_EN | CH_EDIGIT,    /*<U0035> DIGIT FIVE*/
/* 36 */  CH_EN | CH_EDIGIT,    /*<U0036> DIGIT SIX*/
/* 37 */  CH_EN | CH_EDIGIT,    /*<U0037> DIGIT SEVEN*/
/* 38 */  CH_EN | CH_EDIGIT,    /*<U0038> DIGIT EIGHT*/
/* 39 */  CH_EN | CH_EDIGIT,    /*<U0039> DIGIT NINE*/
/* 3A */  CH_CS,    /*<U003A> COLON*/
/* 3B */  CH_N,   /*<U003B> SEMICOLON*/
/* 3C */  CH_N,   /*<U003C> LESS-THAN SIGN*/
/* 3D */  CH_N,   /*<U003D> EQUALS SIGN*/
/* 3E */  CH_N,   /*<U003E> GREATER-THAN SIGN*/
/* 3F */  CH_N,   /*<U003F> QUESTION MARK*/
/* 40 */  CH_N,   /*<U0040> COMMERCIAL AT*/
/* 41 */  CH_L,   /*<U0041> LATIN CAPITAL LETTER A*/
/* 42 */  CH_L,   /*<U0042> LATIN CAPITAL LETTER B*/
/* 43 */  CH_L,   /*<U0043> LATIN CAPITAL LETTER C*/
/* 44 */  CH_L,   /*<U0044> LATIN CAPITAL LETTER D*/
/* 45 */  CH_L,   /*<U0045> LATIN CAPITAL LETTER E*/
/* 46 */  CH_L,   /*<U0046> LATIN CAPITAL LETTER F*/
/* 47 */  CH_L,   /*<U0047> LATIN CAPITAL LETTER G*/
/* 48 */  CH_L,   /*<U0048> LATIN CAPITAL LETTER H*/
/* 49 */  CH_L,   /*<U0049> LATIN CAPITAL LETTER I*/
/* 4A */  CH_L,   /*<U004A> LATIN CAPITAL LETTER J*/
/* 4B */  CH_L,   /*<U004B> LATIN CAPITAL LETTER K*/
/* 4C */  CH_L,   /*<U004C> LATIN CAPITAL LETTER L*/
/* 4D */  CH_L,   /*<U004D> LATIN CAPITAL LETTER M*/
/* 4E */  CH_L,   /*<U004E> LATIN CAPITAL LETTER N*/
/* 4F */  CH_L,   /*<U004F> LATIN CAPITAL LETTER O*/
/* 50 */  CH_L,   /*<U0050> LATIN CAPITAL LETTER P*/
/* 51 */  CH_L,   /*<U0051> LATIN CAPITAL LETTER Q*/
/* 52 */  CH_L,   /*<U0052> LATIN CAPITAL LETTER R*/
/* 53 */  CH_L,   /*<U0053> LATIN CAPITAL LETTER S*/
/* 54 */  CH_L,   /*<U0054> LATIN CAPITAL LETTER T*/
/* 55 */  CH_L,   /*<U0055> LATIN CAPITAL LETTER U*/
/* 56 */  CH_L,   /*<U0056> LATIN CAPITAL LETTER V*/
/* 57 */  CH_L,   /*<U0057> LATIN CAPITAL LETTER W*/
/* 58 */  CH_L,   /*<U0058> LATIN CAPITAL LETTER X*/
/* 59 */  CH_L,   /*<U0059> LATIN CAPITAL LETTER Y*/
/* 5A */  CH_L,   /*<U005A> LATIN CAPITAL LETTER Z*/
/* 5B */  CH_N,   /*<U005B> LEFT SQUARE BRACKET*/
/* 5C */  CH_N,   /*<U005C> REVERSE SOLIDUS*/
/* 5D */  CH_N,   /*<U005D> RIGHT SQUARE BRACKET*/
/* 5E */  CH_N,   /*<U005E> CIRCUMFLEX ACCENT*/
/* 5F */  CH_N,   /*<U005F> LOW LINE*/
/* 60 */  CH_N,   /*<U0060> GRAVE ACCENT*/
/* 61 */  CH_L,   /*<U0061> LATIN SMALL LETTER A*/
/* 62 */  CH_L,   /*<U0062> LATIN SMALL LETTER B*/
/* 63 */  CH_L,   /*<U0063> LATIN SMALL LETTER C*/
/* 64 */  CH_L,   /*<U0064> LATIN SMALL LETTER D*/
/* 65 */  CH_L,   /*<U0065> LATIN SMALL LETTER E*/
/* 66 */  CH_L,   /*<U0066> LATIN SMALL LETTER F*/
/* 67 */  CH_L,   /*<U0067> LATIN SMALL LETTER G*/
/* 68 */  CH_L,   /*<U0068> LATIN SMALL LETTER H*/
/* 69 */  CH_L,   /*<U0069> LATIN SMALL LETTER I*/
/* 6A */  CH_L,   /*<U006A> LATIN SMALL LETTER J*/
/* 6B */  CH_L,   /*<U006B> LATIN SMALL LETTER K*/
/* 6C */  CH_L,   /*<U006C> LATIN SMALL LETTER L*/
/* 6D */  CH_L,   /*<U006D> LATIN SMALL LETTER M*/
/* 6E */  CH_L,   /*<U006E> LATIN SMALL LETTER N*/
/* 6F */  CH_L,   /*<U006F> LATIN SMALL LETTER O*/
/* 70 */  CH_L,   /*<U0070> LATIN SMALL LETTER P*/
/* 71 */  CH_L,   /*<U0071> LATIN SMALL LETTER Q*/
/* 72 */  CH_L,   /*<U0072> LATIN SMALL LETTER R*/
/* 73 */  CH_L,   /*<U0073> LATIN SMALL LETTER S*/
/* 74 */  CH_L,   /*<U0074> LATIN SMALL LETTER T*/
/* 75 */  CH_L,   /*<U0075> LATIN SMALL LETTER U*/
/* 76 */  CH_L,   /*<U0076> LATIN SMALL LETTER V*/
/* 77 */  CH_L,   /*<U0077> LATIN SMALL LETTER W*/
/* 78 */  CH_L,   /*<U0078> LATIN SMALL LETTER X*/
/* 79 */  CH_L,   /*<U0079> LATIN SMALL LETTER Y*/
/* 7A */  CH_L,   /*<U007A> LATIN SMALL LETTER Z*/
/* 7B */  CH_N,   /*<U007B> LEFT CURLY BRACKET*/
/* 7C */  CH_N,   /*<U007C> VERTICAL LINE*/
/* 7D */  CH_N,   /*<U007D> RIGHT CURLY BRACKET*/
/* 7E */  CH_N,   /*<U007E> TILDE*/
/* 7F */  CH_N,   /*<U007F> (CC) Delete*/
/* 80 */  CH_N,     /* *** UNASSIGNED*/
/* 81 */  CH_N,     /* *** UNASSIGNED*/
/* 82 */  CH_N,   /*<U201A> SINGLE LOW-9 QUOTATION MARK*/
/* 83 */  CH_N,   /*<U0192> LATIN SMALL LETTER F WITH HOOK*/
/* 84 */  CH_N,   /*<U201E> DOUBLE LOW-9 QUOTATION MARK*/
/* 85 */  CH_N,   /*<U2026> HORIZONTAL ELLIPSIS*/
/* 86 */  CH_N,   /*<U2020> DAGGER*/
/* 87 */  CH_N,   /*<U2021> DOUBLE DAGGER*/
/* 88 */  CH_L,   /*<U02C6> MODIFIER LETTER CIRCUMFLEX ACCENT*/
/* 89 */  CH_ET,    /*<U2030> PER MILLE SIGN*/
/* 8A */  CH_N,   /* *** UNASSIGNED*/
/* 8B */  CH_N,   /*<U2039> SINGLE LEFT-POINTING ANGLE QUOTATION MARK*/
/* 8C */  CH_N,     /* *** UNASSIGNED*/
/* 8D */  CH_N,     /* *** UNASSIGNED*/
/* 8E */  CH_N,     /* *** UNASSIGNED*/
/* 8F */  CH_N,     /* *** UNASSIGNED*/
/* 90 */  CH_N,     /* *** UNASSIGNED*/
/* 91 */  CH_N,   /*<U2018> LEFT SINGLE QUOTATION MARK*/
/* 92 */  CH_N,   /*<U2019> RIGHT SINGLE QUOTATION MARK*/
/* 93 */  CH_N,   /*<U201C> LEFT DOUBLE QUOTATION MARK*/
/* 94 */  CH_N,   /*<U201D> RIGHT DOUBLE QUOTATION MARK*/
/* 95 */  CH_N,   /*<U2022> BULLET*/
/* 96 */  CH_N,   /*<U2013> EN DASH*/
/* 97 */  CH_N,   /*<U2014> EM DASH*/
/* 98 */  CH_L,   /*<U02DC> TILDE*/
/* 99 */  CH_N,   /*<U2122> TRADE MARK SIGN*/
/* 9A */  CH_N,     /* *** UNASSIGNED*/
/* 9B */  CH_N,   /*<U203A> SINGLE RIGHT-POINTING ANGLE QUOTATION MARK*/
/* 9C */  CH_N,     /* *** UNASSIGNED*/
/* 9D */  CH_N,     /* *** UNASSIGNED*/
/* 9E */  CH_N,     /* *** UNASSIGNED*/
/* 9F */  CH_N,     /* *** UNASSIGNED*/
/* A0 */  CH_W,   /*<U00A0> NO-BREAK SPACE*/
/* A1 */  CH_N,     /* *** UNASSIGNED*/
/* A2 */  CH_ET,    /*<U00A2> CENT SIGN*/
/* A3 */  CH_ET,    /*<U00A3> POUND SIGN*/
/* A4 */  CH_ET,    /*<U20AA> NEW SHEKEL SIGN*/
/* A5 */  CH_ET,    /*<U00A5> YEN SIGN*/
/* A6 */  CH_N,   /*<U00A6> BROKEN BAR*/
/* A7 */  CH_N,   /*<U00A7> SECTION SIGN*/
/* A8 */  CH_N,   /*<U00A8> DIAERESIS*/
/* A9 */  CH_N,   /*<U00A9> COPYRIGHT SIGN*/
/* AA */  CH_N,     /* ***UNASSIGNED*/
/* AB */  CH_N,   /*<U00AB> LEFT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* AC */  CH_N,   /*<U00AC> NOT SIGN*/
/* AD */  CH_N,   /*<U00AD> SOFT HYPHEN*/
/* AE */  CH_N,   /*<U00AE> REGISTERED SIGN*/
/* AF */  CH_N,   /*<U00AF> MACRON*/
/* B0 */  CH_ET,    /*<U00B0> DEGREE SIGN*/
/* B1 */  CH_ET,    /*<U00B1> PLUS-MINUS SIGN*/
/* B2 */  CH_EN,    /*<U00B2> SUPERSCRIPT TWO*/
/* B3 */  CH_EN,    /*<U00B3> SUPERSCRIPT THREE*/
/* B4 */  CH_N,   /*<U00B4> ACUTE ACCENT*/
/* B5 */  CH_N,   /*<U00B5> MICRO SIGN*/
/* B6 */  CH_N,   /*<U00B6> PILCROW SIGN*/
/* B7 */  CH_N,   /*<U00B7> MIDDLE DOT*/
/* B8 */  CH_N,     /* *** UNASSIGNED*/
/* B9 */  CH_EN,    /*<U00B9> SUPERSCRIPT ONE*/
/* BA */  CH_N,     /* *** UNASSIGNED*/
/* BB */  CH_N,   /*<U00BB> RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* BC */  CH_N,   /*<U00BC> VULGAR FRACTION ONE QUARTER*/
/* BD */  CH_N,   /*<U00BD> VULGAR FRACTION ONE HALF*/
/* BE */  CH_N,   /*<U00BE> VULGAR FRACTION THREE QUARTERS*/
/* BF */  CH_N,     /* *** UNASSIGNED*/
/* C0 */  CH_R | CH_NSPACE,   /*<U05B0> HEBREW POINT SHEVA*/
/* C1 */  CH_R | CH_NSPACE,   /*<U05B1> HEBREW POINT HATAF SEGOL*/
/* C2 */  CH_R | CH_NSPACE,   /*<U05B2> HEBREW POINT HATAF PATAH*/
/* C3 */  CH_R | CH_NSPACE,   /*<U05B3> HEBREW POINT HATAF KAMATS*/
/* C4 */  CH_R | CH_NSPACE,   /*<U05B4> HEBREW POINT HIRIQ*/
/* C5 */  CH_R | CH_NSPACE,   /*<U05B5> HEBREW POINT TSERE*/
/* C6 */  CH_R | CH_NSPACE,   /*<U05B6> HEBREW POINT SEGOL*/
/* C7 */  CH_R | CH_NSPACE,   /*<U05B7> HEBREW POINT PATAH*/
/* C8 */  CH_R | CH_NSPACE,   /*<U05B8> HEBREW POINT QAMATS*/
/* C9 */  CH_R | CH_NSPACE,   /*<U05B9> HEBREW POINT HOLAM*/
/* CA */  CH_N,         /*<U05BA> HEBREW POINT*/
/* CB */  CH_R | CH_NSPACE,   /*<U05BB> HEBREW POINT QUBUTS*/
/* CC */  CH_R | CH_NSPACE,   /*<U05BC> HEBREW POINT DAGESH OR MAPPIQ*/
/* CD */  CH_R | CH_NSPACE,   /*<U05BD> HEBREW POINT METEG*/
/* CE */  CH_R,         /*<U05BE> HEBREW PUNCTUATION MAQAF*/
/* CF */  CH_R | CH_NSPACE,   /*<U05BF> HEBREW PUNCTUATION RAFE*/
/* D0 */  CH_R,         /*<U05C0> HEBREW PUNCTUATION PASEQ*/
/* D1 */  CH_R | CH_NSPACE,   /*<U05C1> HEBREW POINT SHIN DOT*/
/* D2 */  CH_R | CH_NSPACE,   /*<U05C2> HEBREW POINT SIN DOT*/
/* D3 */  CH_R,         /*<U05C3> HEBREW PUNCTUATION SOF PASUK*/
/* D4 */  CH_R | CH_HEBLET,   /*<U05F0> HEBREW LIGATURE YIDDISH DOUBLE VAV*/
/* D5 */  CH_R | CH_HEBLET,   /*<U05F1> HEBREW LIGATURE YIDDISH VAV YOD*/
/* D6 */  CH_R | CH_HEBLET,   /*<U05F2> HEBREW LIGATURE YIDDISH DOUBLE YOD*/
/* D7 */  CH_N,   /* *** UNASSIGNED*/
/* D8 */  CH_N,   /* *** UNASSIGNED*/
/* D9 */  CH_N,   /* *** UNASSIGNED*/
/* DA */  CH_N,   /* *** UNASSIGNED*/
/* DB */  CH_N,   /* *** UNASSIGNED*/
/* DC */  CH_N,   /* *** UNASSIGNED*/
/* DD */  CH_N,   /* *** UNASSIGNED*/
/* DE */  CH_N,   /* *** UNASSIGNED*/
/* DF */  CH_N,   /* *** UNASSIGNED*/
/* E0 */  CH_R | CH_HEBLET,   /*<U05D0> HEBREW LETTER ALEF*/
/* E1 */  CH_R | CH_HEBLET,   /*<U05D1> HEBREW LETTER BEIT*/
/* E2 */  CH_R | CH_HEBLET,   /*<U05D2> HEBREW LETTER GIMMEL*/
/* E3 */  CH_R | CH_HEBLET,   /*<U05D3> HEBREW LETTER DALET*/
/* E4 */  CH_R | CH_HEBLET,   /*<U05D4> HEBREW LETTER HE*/
/* E5 */  CH_R | CH_HEBLET,   /*<U05D5> HEBREW LETTER VAV*/
/* E6 */  CH_R | CH_HEBLET,   /*<U05D6> HEBREW LETTER ZAIN*/
/* E7 */  CH_R | CH_HEBLET,   /*<U05D7> HEBREW LETTER HET*/
/* E8 */  CH_R | CH_HEBLET,   /*<U05D8> HEBREW LETTER TET*/
/* E9 */  CH_R | CH_HEBLET,   /*<U05D9> HEBREW LETTER YOD*/
/* EA */  CH_R | CH_HEBLET,   /*<U05DA> HEBREW LETTER FINAL KAF*/
/* EB */  CH_R | CH_HEBLET,   /*<U05DB> HEBREW LETTER MEDIAL KAF*/
/* EC */  CH_R | CH_HEBLET,   /*<U05DC> HEBREW LETTER LAMED*/
/* ED */  CH_R | CH_HEBLET,   /*<U05DD> HEBREW LETTER FINAL MEM*/
/* EE */  CH_R | CH_HEBLET,   /*<U05DE> HEBREW LETTER MEDIAL MEM*/
/* EF */  CH_R | CH_HEBLET,   /*<U05DF> HEBREW LETTER FINAL NUN*/
/* F0 */  CH_R | CH_HEBLET,   /*<U05E0> HEBREW LETTER MEDIAL NUN*/
/* F1 */  CH_R | CH_HEBLET,   /*<U05E1> HEBREW LETTER SAMECH*/
/* F2 */  CH_R | CH_HEBLET,   /*<U05E2> HEBREW LETTER AIN*/
/* F3 */  CH_R | CH_HEBLET,   /*<U05E3> HEBREW LETTER FINAL PE*/
/* F4 */  CH_R | CH_HEBLET,   /*<U05E4> HEBREW LETTER MEDIAL PE*/
/* F5 */  CH_R | CH_HEBLET,   /*<U05E5> HEBREW LETTER FINAL TZADDI*/
/* F6 */  CH_R | CH_HEBLET,   /*<U05E6> HEBREW LETTER MEDIAL TZADDI*/
/* F7 */  CH_R | CH_HEBLET,   /*<U05E7> HEBREW LETTER QUF*/
/* F8 */  CH_R | CH_HEBLET,   /*<U05E8> HEBREW LETTER RESH*/
/* F9 */  CH_R | CH_HEBLET,   /*<U05E9> HEBREW LETTER SHIN*/
/* FA */  CH_R | CH_HEBLET,   /*<U05EA> HEBREW LETTER TAV*/
/* FB */  CH_N,   /* *** UNASSIGNED*/
/* FC */  CH_N,   /* *** UNASSIGNED*/
/* FD */  CH_L | CH_BIDICTRL,   /*<U200E> LEFT-TO-RIGHT MARK*/
/* FE */  CH_R | CH_BIDICTRL,   /*<U200F> RIGHT-TO-LEFT MARK*/
  }
};

/**********************  Symmetric pairs table *************************/
unsigned char symmpair [10] [256]={
  /* 420 MF/AS 400 Arabic*/
  {
/* 00 */ 0x00,  /*<U0000>.      NULL*/
/* 01 */ 0x01,  /*<U0001>.      START OF HEADING*/
/* 02 */ 0x02,  /*<U0002>.      START OF TEXT*/
/* 03 */ 0x03,  /*<U0003>.      END OF TEXT*/
/* 04 */ 0x04,  /*<U009C>.*/
/* 05 */ 0x05,  /*<U0009>.      HORIZONTAL TABULATION*/
/* 06 */ 0x06,  /*<U0086>.*/
/* 07 */ 0x07,  /*<U007F>.      DELETE*/
/* 08 */ 0x08,  /*<U0097>.*/
/* 09 */ 0x09,  /*<U008D>.*/
/* 0A */ 0x0A,  /*<U008E>.*/
/* 0B */ 0x0B,  /*<U000B>.      VERTICAL TABULATION*/
/* 0C */ 0x0C,  /*<U000C>.      FORM FEED*/
/* 0D */ 0x0D,  /*<U000D>.      CARRIAGE RETURN*/
/* 0E */ 0x0E,  /*<U000E>.      SHIFT OUT*/
/* 0F */ 0x0F,  /*<U000F>.      SHIFT IN*/
/* 10 */ 0x10,  /*<U0010>.      DATA LINK ESCAPE*/
/* 11 */ 0x11,  /*<U0011>.      DEVICE CONTROL ONE*/
/* 12 */ 0x12,  /*<U0012>.      DEVICE CONTROL TWO*/
/* 13 */ 0x13,  /*<U0013>.      DEVICE CONTROL THREE*/
/* 14 */ 0x14,  /*<U009D>.*/
/* 15 */ 0x15,  /*<U0085>.*/
/* 16 */ 0x16,  /*<U0008>.      BACKSPACE*/
/* 17 */ 0x17,  /*<U0087>.*/
/* 18 */ 0x18,  /*<U0018>.      CANCEL*/
/* 19 */ 0x19,  /*<U0019>.      END OF MEDIUM*/
/* 1A */ 0x1A,  /*<U0092>.*/
/* 1B */ 0x1B,  /*<U008F>.*/
/* 1C */ 0x1C,  /*<U001C>.      FILE SEPARATOR*/
/* 1D */ 0x1D,  /*<U001D>.      GROUP SEPARATOR*/
/* 1E */ 0x1E,  /*<U001E>.      RECORD SEPARATOR*/
/* 1F */ 0x1F,  /*<U001F>.      UNIT SEPARATOR*/
/* 20 */ 0x20,  /*<U0080>.*/
/* 21 */ 0x21,  /*<U0081>.*/
/* 22 */ 0x22,  /*<U0082>.*/
/* 23 */ 0x23,  /*<U0083>.*/
/* 24 */ 0x24,  /*<U0084>.*/
/* 25 */ 0x25,  /*<U000A>.      LINE FEED*/
/* 26 */ 0x26,  /*<U0017>.      END OF TRANSMISSION BLOCK*/
/* 27 */ 0x27,  /*<U001B>.      ESCAPE*/
/* 28 */ 0x28,  /*<U0088>.*/
/* 29 */ 0x29,  /*<U0089>.*/
/* 2A */ 0x2A,  /*<U008A>.*/
/* 2B */ 0x2B,  /*<U008B>.*/
/* 2C */ 0x2C,  /*<U008C>.*/
/* 2D */ 0x2D,  /*<U0005>.      ENQUIRY*/
/* 2E */ 0x2E,  /*<U0006>.      ACKNOWLEDGE*/
/* 2F */ 0x2F,  /*<U0007>.      BELL*/
/* 30 */ 0x30,  /*<U0090>.*/
/* 31 */ 0x31,  /*<U0091>.*/
/* 32 */ 0x32,  /*<U0016>.      SYNCHRONOUS IDLE*/
/* 33 */ 0x33,  /*<U0093>.*/
/* 34 */ 0x34,  /*<U0094>.*/
/* 35 */ 0x35,  /*<U0095>.*/
/* 36 */ 0x36,  /*<U0096>.*/
/* 37 */ 0x37,  /*<U0004>.      END OF TRANSMISSION*/
/* 38 */ 0x38,  /*<U0098>.*/
/* 39 */ 0x39,  /*<U0099>.*/
/* 3A */ 0x3A,  /*<U009A>.*/
/* 3B */ 0x3B,  /*<U009B>.*/
/* 3C */ 0x3C,  /*<U0014>.      DEVICE CONTROL FOUR*/
/* 3D */ 0x3D,  /*<U0015>.      NEGATIVE ACKNOWLEDGE*/
/* 3E */ 0x3E,  /*<U009E>.*/
/* 3F */ 0x3F,  /*<U001A>.      SUBSTITUTE*/
/* 40 */ 0x40,  /*<U0020>   SPACE*/
/* 41 */ 0x41,  /*<U00A0>       NON-BREAKING SPACE*/
/* 42 */ 0x42,  /*<UFE7C>       ARABIC SPACING SHADDAH*/
/* 43 */ 0x43,  /*<UFE7D>       ARABIC SHADDAH ON TATWEEL*/
/* 44 */ 0x44,  /*<U0640>       ARABIC TATWEEL*/
/* 45 */ 0x45,  /*<tail_c       kasseh (Arabic tail)*/
/* 46 */ 0x46,  /*<UFE80>       GLYPH FOR ISOLATE ARABIC HAMZAH*/
/* 47 */ 0x47,  /*<UFE81>       GLYPH FOR ISOLATE ARABIC MADDAH ON ALEF*/
/* 48 */ 0x48,  /*<UFE82>       GLYPH FOR FINAL ARABIC MADDAH ON ALEF*/
/* 49 */ 0x49,  /*<UFE83>       GLYPH FOR ISOLATE ARABIC HAMZAH ON ALEF*/
/* 4A */ 0x4A,  /*<U00A2>       CENT SIGN*/
/* 4B */ 0x4B,  /*<U002E>       PERIOD*/
/* 4C */ 0x6E,  /*<U003C>       LESS-THAN SIGN*/
/* 4D */ 0x5D,  /*<U0028>       OPENING PARENTHESIS*/
/* 4E */ 0x4E,  /*<U002B>       PLUS SIGN*/
/* 4F */ 0x4F,  /*<U007C>       VERTICAL BAR*/
/* 50 */ 0x50,  /*<U0026>       AMPERSAND*/
/* 51 */ 0x51,  /*<UFE84>       GLYPH FOR FINAL ARABIC HAMZAH ON ALEF*/
/* 52 */ 0x52,  /*<UFE86>       GLYPH FOR FINAL ARABIC HAMZAH ON WAW*/
/* 53 */ 0x53,  /* *** UNASSIGNED*/
/* 54 */ 0x54,  /* *** UNASSIGNED*/
/* 55 */ 0x55,  /*<UFE8A>       GLYPH FOR FINAL ARABIC HAMZAH ON YA*/
/* 56 */ 0x56,  /*<UFE8D>       GLYPH FOR ISOLATE ARABIC ALEF*/
/* 57 */ 0x57,  /*<UFE8E>       GLYPH FOR FINAL ARABIC ALEF*/
/* 58 */ 0x58,  /*<UFE90>       GLYPH FOR FINAL ARABIC BAA*/
/* 59 */ 0x59,  /*<UFE92>       GLYPH FOR MEDIAL ARABIC BAA*/
/* 5A */ 0x5A,  /*<U0021>       EXCLAMATION MARK*/
/* 5B */ 0x5B,  /*<U0024>       DOLLAR SIGN*/
/* 5C */ 0x5C,  /*<U002A>       ASTERISK*/
/* 5D */ 0x4D,  /*<U0029>       CLOSING PARENTHESIS*/
/* 5E */ 0x5E,  /*<U003B>       SEMICOLON*/
/* 5F */ 0x5F,  /*<U00AC>       NOT SIGN*/
/* 60 */ 0x60,  /*<U002D>       HYPHEN-MINUS*/
/* 61 */ 0x61,  /*<U002F>       SLASH*/
/* 62 */ 0x62,  /*<UFE94>       GLYPH FOR FINAL ARABIC TAA MARBUTAH*/
/* 63 */ 0x63,  /*<UFE96>       GLYPH FOR FINAL ARABIC TAA*/
/* 64 */ 0x64,  /*<UFE98>       GLYPH FOR MEDIAL ARABIC TAA*/
/* 65 */ 0x65,  /*<UFE9A>       GLYPH FOR FINAL ARABIC THAA*/
/* 66 */ 0x66,  /*<UFE9C>       GLYPH FOR MEDIAL ARABIC THAA*/
/* 67 */ 0x67,  /*<UFE9E>       GLYPH FOR FINAL ARABIC JEEM*/
/* 68 */ 0x68,  /*<UFEA0>       GLYPH FOR MEDIAL ARABIC JEEM*/
/* 69 */ 0x69,  /*<UFEA2>       GLYPH FOR FINAL ARABIC HAA*/
/* 6A */ 0x6A,  /*<U00A6>       BROKEN VERTICAL BAR*/
/* 6B */ 0x6B,  /*<U002C>       COMMA*/
/* 6C */ 0x6C,  /*<U066A>       ARABIC PERCENT SIGN*/
/* 6D */ 0x6D,  /*<U005F>       SPACING UNDERSCORE*/
/* 6E */ 0x4C,  /*<U003E>       GREATER-THAN SIGN*/
/* 6F */ 0x6F,  /*<U003F>       QUESTION MARK*/
/* 70 */ 0x70,  /*<UFEA4>       GLYPH FOR MEDIAL ARABIC HAA*/
/* 71 */ 0x71,  /*<UFEA6>       GLYPH FOR FINAL ARABIC KHAA*/
/* 72 */ 0x72,  /*<UFEA8>       GLYPH FOR MEDIAL ARABIC KHAA*/
/* 73 */ 0x73,  /*<UFEAA>       GLYPH FOR FINAL ARABIC DAL*/
/* 74 */ 0x74,  /*<UFEAC>       GLYPH FOR FINAL ARABIC THAL*/
/* 75 */ 0x75,  /*<UFEAE>       GLYPH FOR FINAL ARABIC RA*/
/* 76 */ 0x76,  /*<UFEB0>       GLYPH FOR FINAL ARABIC ZAIN*/
/* 77 */ 0x77,  /*<UFEB2>       GLYPH FOR FINAL ARABIC SEEN*/
/* 78 */ 0x78,  /*<UFEB4>       GLYPH FOR MEDIAL ARABIC SEEN*/
/* 79 */ 0x79,  /*<U060C>       ARABIC COMMA*/
/* 7A */ 0x7A,  /*<U003A>       COLON*/
/* 7B */ 0x7B,  /*<U0023>       NUMBER SIGN*/
/* 7C */ 0x7C,  /*<U0040>       COMMERCIAL AT*/
/* 7D */ 0x7D,  /*<U0027>       APOSTROPHE-QUOTE*/
/* 7E */ 0x7E,  /*<U003D>       EQUALS SIGN*/
/* 7F */ 0x7F,  /*<U0022>       QUOTATION MARK*/
/* 80 */ 0x80,  /*<UFEB6>       GLYPH FOR FINAL ARABIC SHEEN*/
/* 81 */ 0x81,  /*<U0061>       LATIN SMALL LETTER A*/
/* 82 */ 0x82,  /*<U0062>       LATIN SMALL LETTER B*/
/* 83 */ 0x83,  /*<U0063>       LATIN SMALL LETTER C*/
/* 84 */ 0x84,  /*<U0064>       LATIN SMALL LETTER D*/
/* 85 */ 0x85,  /*<U0065>       LATIN SMALL LETTER E*/
/* 86 */ 0x86,  /*<U0066>       LATIN SMALL LETTER F*/
/* 87 */ 0x87,  /*<U0067>       LATIN SMALL LETTER G*/
/* 88 */ 0x88,  /*<U0068>       LATIN SMALL LETTER H*/
/* 89 */ 0x89,  /*<U0069>       LATIN SMALL LETTER I*/
/* 8A */ 0x8A,  /*<UFEB8>       GLYPH FOR MEDIAL ARABIC SHEEN*/
/* 8B */ 0x8B,  /*<UFEBA>       GLYPH FOR FINAL ARABIC SAD*/
/* 8C */ 0x8C,  /*<UFEBC>       GLYPH FOR MEDIAL ARABIC SAD*/
/* 8D */ 0x8D,  /*<UFEBE>       GLYPH FOR FINAL ARABIC DAD*/
/* 8E */ 0x8E,  /*<UFEC0>       GLYPH FOR MEDIAL ARABIC DAD*/
/* 8F */ 0x8F,  /*<UFEC2>       GLYPH FOR FINAL ARABIC TAH*/
/* 90 */ 0x90,  /*<UFEC6>       GLYPH FOR FINAL ARABIC DHAH*/
/* 91 */ 0x91,  /*<U006A>       LATIN SMALL LETTER J*/
/* 92 */ 0x92,  /*<U006B>       LATIN SMALL LETTER K*/
/* 93 */ 0x93,  /*<U006C>       LATIN SMALL LETTER L*/
/* 94 */ 0x94,  /*<U006D>       LATIN SMALL LETTER M*/
/* 95 */ 0x95,  /*<U006E>       LATIN SMALL LETTER N*/
/* 96 */ 0x96,  /*<U006F>       LATIN SMALL LETTER O*/
/* 97 */ 0x97,  /*<U0070>       LATIN SMALL LETTER P*/
/* 98 */ 0x98,  /*<U0071>       LATIN SMALL LETTER Q*/
/* 99 */ 0x99,  /*<U0072>       LATIN SMALL LETTER R*/
/* 9A */ 0x9A,  /*<UFEC9>       GLYPH FOR ISOLATE ARABIC AIN*/
/* 9B */ 0x9B,  /*<UFECA>       GLYPH FOR FINAL ARABIC AIN*/
/* 9C */ 0x9C,  /*<UFECB>       GLYPH FOR INITIAL ARABIC AIN*/
/* 9D */ 0x9D,  /*<UFECC>       GLYPH FOR MEDIAL ARABIC AIN*/
/* 9E */ 0x9E,  /*<UFECD>       GLYPH FOR ISOLATE ARABIC GHAIN*/
/* 9F */ 0x9F,  /*<UFECE>       GLYPH FOR FINAL ARABIC GHAIN*/
/* A0 */ 0xA0,  /*<UFECF>       GLYPH FOR INITIAL ARABIC GHAIN*/
/* A1 */ 0xA1,  /*<U00F7>       DIVISION SIGN*/
/* A2 */ 0xA2,  /*<U0073>       LATIN SMALL LETTER S*/
/* A3 */ 0xA3,  /*<U0074>       LATIN SMALL LETTER T*/
/* A4 */ 0xA4,  /*<U0075>       LATIN SMALL LETTER U*/
/* A5 */ 0xA5,  /*<U0076>       LATIN SMALL LETTER V*/
/* A6 */ 0xA6,  /*<U0077>       LATIN SMALL LETTER W*/
/* A7 */ 0xA7,  /*<U0078>       LATIN SMALL LETTER X*/
/* A8 */ 0xA8,  /*<U0079>       LATIN SMALL LETTER Y*/
/* A9 */ 0xA9,  /*<U007A>       LATIN SMALL LETTER Z*/
/* AA */ 0xAA,  /*<UFED0>       GLYPH FOR MEDIAL ARABIC GHAIN*/
/* AB */ 0xAB,  /*<UFED2>       GLYPH FOR FINAL ARABIC FA*/
/* AC */ 0xAC,  /*<UFED4>       GLYPH FOR MEDIAL ARABIC FA*/
/* AD */ 0xAD,  /*<UFED6>       GLYPH FOR FINAL ARABIC QAF*/
/* AE */ 0xAE,  /*<UFED7>       GLYPH FOR MEDIAL ARABIC QAF*/
/* AF */ 0xAF,  /*<UFEDA>       GLYPH FOR FINAL ARABIC CAF*/
/* B0 */ 0xB0,  /*<UFEDC>       GLYPH FOR MEDIAL ARABIC CAF*/
/* B1 */ 0xB1,  /*<UFEDE>       GLYPH FOR FINAL ARABIC LAM*/
/* B2 */ 0xB2,  /*<UFEF5>       GLYPH FOR ISOLATE ARABIC MADDAH ON LIGATURE LAM ALEF*/
/* B3 */ 0xB3,  /*<UFEF6>       GLYPH FOR FINAL ARABIC MADDAH ON LIGATURE LAM ALEF*/
/* B4 */ 0xB4,  /*<UFEF7>       GLYPH FOR ISOLATE ARABIC HAMZAH ON LIGATURE LAM ALEF*/
/* B5 */ 0xB5,  /*<UFEF8>       GLYPH FOR FINAL ARABIC HAMZAH ON LIGATURE LAM ALEF*/
/* B6 */ 0xB6,  /* *** UNASSIGNED*/
/* B7 */ 0xB7,  /* *** UNASSIGNED*/
/* B8 */ 0xB8,  /*<UFEFB>       GLYPH FOR ISOLATE ARABIC LIGATURE LAM ALEF*/
/* B9 */ 0xB9,  /*<UFEFC>       GLYPH FOR FINAL ARABIC LIGATURE LAM ALEF*/
/* BA */ 0xBA,  /*<UFEE0>       GLYPH FOR MEDIAL ARABIC LAM*/
/* BB */ 0xBB,  /*<UFEE2>       GLYPH FOR FINAL ARABIC MEEM*/
/* BC */ 0xBC,  /*<UFEE4>       GLYPH FOR MEDIAL ARABIC MEEM*/
/* BD */ 0xBD,  /*<UFEE6>       GLYPH FOR FINAL ARABIC NOON*/
/* BE */ 0xBE,  /*<UFEE8>       GLYPH FOR MEDIAL ARABIC NOON*/
/* BF */ 0xBF,  /*<UFEEA>       GLYPH FOR FINAL ARABIC HA*/
/* C0 */ 0xC0,    /*<U061B>       ARABIC SEMICOLON*/
/* C1 */ 0xC1,  /*<U0041>       LATIN CAPITAL LETTER A*/
/* C2 */ 0xC2,  /*<U0042>       LATIN CAPITAL LETTER B*/
/* C3 */ 0xC3,  /*<U0043>       LATIN CAPITAL LETTER C*/
/* C4 */ 0xC4,  /*<U0044>       LATIN CAPITAL LETTER D*/
/* C5 */ 0xC5,  /*<U0045>       LATIN CAPITAL LETTER E*/
/* C6 */ 0xC6,  /*<U0046>       LATIN CAPITAL LETTER F*/
/* C7 */ 0xC7,  /*<U0047>       LATIN CAPITAL LETTER G*/
/* C8 */ 0xC8,  /*<U0048>       LATIN CAPITAL LETTER H*/
/* C9 */ 0xC9,  /*<U0049>       LATIN CAPITAL LETTER I*/
/* CA */ 0xCA,  /*<U00AD>       SOFT HYPHEN*/
/* CB */ 0xCB,  /*<UFEEB>       GLYPH FOR INITIAL ARABIC HA*/
/* CC */ 0xCC,  /* *** UNASSIGNED*/
/* CD */ 0xCD,  /*<UFEEC>       GLYPH FOR MEDIAL ARABIC HA*/
/* CE */ 0xCE,  /* *** UNASSIGNED*/
/* CF */ 0xCF,  /*<UFEEE>       GLYPH FOR FINAL ARABIC WAW*/
/* D0 */ 0xD0,  /*<U061F>       ARABIC QUESTION MARK*/
/* D1 */ 0xD1,  /*<U004A>       LATIN CAPITAL LETTER J*/
/* D2 */ 0xD2,  /*<U004B>       LATIN CAPITAL LETTER K*/
/* D3 */ 0xD3,  /*<U004C>       LATIN CAPITAL LETTER L*/
/* D4 */ 0xD4,  /*<U004D>       LATIN CAPITAL LETTER M*/
/* D5 */ 0xD5,  /*<U004E>       LATIN CAPITAL LETTER N*/
/* D6 */ 0xD6,  /*<U004F>       LATIN CAPITAL LETTER O*/
/* D7 */ 0xD7,  /*<U0050>       LATIN CAPITAL LETTER P*/
/* D8 */ 0xD8,  /*<U0051>       LATIN CAPITAL LETTER Q*/
/* D9 */ 0xD9,  /*<U0052>       LATIN CAPITAL LETTER R*/
/* DA */ 0xDA,  /*<UFEEF>       GLYPH FOR ISOLATE ARABIC ALEF MAQSURAH*/
/* DB */ 0xDB,  /*<UFEF0>       GLYPH FOR FINAL ARABIC ALEF MAQSURAH*/
/* DC */ 0xDC,  /*<UFEF1>       GLYPH FOR ISOLATE ARABIC YA*/
/* DD */ 0xDD,  /*<UFEF2>       GLYPH FOR FINAL ARABIC YA*/
/* DE */ 0xDE,  /*<UFEF4>       GLYPH FOR MEDIAL ARABIC YA*/
/* DF */ 0xDF,  /*<U0660>       ARABIC-INDIC DIGIT ZERO*/
/* E0 */ 0xE0,  /*<U00D7>       MULTIPLICATION SIGN*/
/* E1 */ 0xE1,  /*<U2007>       FIGURE SPACE*/
/* E2 */ 0xE2,  /*<U0053>       LATIN CAPITAL LETTER S*/
/* E3 */ 0xE3,  /*<U0054>       LATIN CAPITAL LETTER T*/
/* E4 */ 0xE4,  /*<U0055>       LATIN CAPITAL LETTER U*/
/* E5 */ 0xE5,  /*<U0056>       LATIN CAPITAL LETTER V*/
/* E6 */ 0xE6,  /*<U0057>       LATIN CAPITAL LETTER W*/
/* E7 */ 0xE7,  /*<U0058>       LATIN CAPITAL LETTER X*/
/* E8 */ 0xE8,  /*<U0059>       LATIN CAPITAL LETTER Y*/
/* E9 */ 0xE9,  /*<U005A>       LATIN CAPITAL LETTER Z*/
/* EA */ 0xEA,    /*<U0661>       ARABIC-INDIC DIGIT ONE*/
/* EB */ 0xEB,    /*<U0662>       ARABIC-INDIC DIGIT TWO*/
/* EC */ 0xEC,    /* *** UNASSIGNED*/
/* ED */ 0xED,  /*<U0663>       ARABIC-INDIC DIGIT THREE*/
/* EE */ 0xEE,  /*<U0664>       ARABIC-INDIC DIGIT FOUR*/
/* EF */ 0xEF,  /*<U0665>       ARABIC-INDIC DIGIT FIVE*/
/* F0 */ 0xF0,  /*<U0030>       DIGIT ZERO*/
/* F1 */ 0xF1,  /*<U0031>       DIGIT ONE*/
/* F2 */ 0xF2,  /*<U0032>       DIGIT TWO*/
/* F3 */ 0xF3,  /*<U0033>       DIGIT THREE*/
/* F4 */ 0xF4,  /*<U0034>       DIGIT FOUR*/
/* F5 */ 0xF5,  /*<U0035>       DIGIT FIVE*/
/* F6 */ 0xF6,  /*<U0036>       DIGIT SIX*/
/* F7 */ 0xF7,  /*<U0037>       DIGIT SEVEN*/
/* F8 */ 0xF8,  /*<U0038>       DIGIT EIGHT*/
/* F9 */ 0xF9,  /*<U0039>       DIGIT NINE*/
/* FA */ 0xFA,  /* *** UNASSIGNED*/
/* FB */ 0xFB,  /*<U0666>       ARABIC-INDIC DIGIT SIX*/
/* FC */ 0xFC,  /*<U0667>       ARABIC-INDIC DIGIT SEVEN*/
/* FD */ 0xFD,  /*<U0668>       ARABIC-INDIC DIGIT EIGHT*/
/* FE */ 0xFE,  /*<U0669>       ARABIC-INDIC DIGIT NINE*/
/* FF */ 0xFF /*<U009F>       EIGHT ONES*/
  },
  /* 864 OS/2 Arabic*/
  {
/* 00 */ 0x00,  /*<U0000>       NULL*/
/* 01 */ 0x01,  /*<U0001>       START OF HEADING*/
/* 02 */ 0x02,  /*<U0002>       START OF TEXT*/
/* 03 */ 0x03,  /*<U0003>       END OF TEXT*/
/* 04 */ 0x04,  /*<U0004>       END OF TRANSMISSION*/
/* 05 */ 0x05,  /*<U0005>       ENQUIRY*/
/* 06 */ 0x06,  /*<U0006>       ACKNOWLEDGE*/
/* 07 */ 0x07,  /*<U0007>       BELL*/
/* 08 */ 0x08,  /*<U0008>       BACKSPACE*/
/* 09 */ 0x09,  /*<U0009>       HORIZONTAL TABULATION*/
/* 0A */ 0x0A,  /*<U000A>       LINE FEED*/
/* 0B */ 0x0B,  /*<U000B>       VERTICAL TABULATION*/
/* 0C */ 0x0C,  /*<U000C>       FORM FEED*/
/* 0D */ 0x0D,  /*<U000D>       CARRIAGE RETURN*/
/* 0E */ 0x0E,  /*<U000E>       SHIFT OUT*/
/* 0F */ 0x0F,  /*<U000F>       SHIFT IN*/
/* 10 */ 0x10,  /*<U0010>       DATA LINK ESCAPE*/
/* 11 */ 0x11,  /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */ 0x12,  /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */ 0x13,  /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */ 0x14,  /*<U0014>       DEVICE CONTROL FOUR*/
/* 15 */ 0x15,  /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 16 */ 0x16,  /*<U0016>       SYNCHRONOUS IDLE*/
/* 17 */ 0x17,  /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 18 */ 0x18,  /*<U0018>       CANCEL*/
/* 19 */ 0x19,  /*<U0019>       END OF MEDIUM*/
/* 1A */ 0x1A,  /*<U001C>       FILE SEPARATOR*/
/* 1B */ 0x1B,  /*<U001B>       ESCAPE*/
/* 1C */ 0x1C,  /*<U007F>       DELETE*/
/* 1D */ 0x1D,  /*<U001D>       GROUP SEPARATOR*/
/* 1E */ 0x1E,  /*<U001E>       RECORD SEPARATOR*/
/* 1F */ 0x1F,  /*<U001F>       UNIT SEPARATOR*/
/* 20 */ 0x20,  /*<U0020>              SPACE*/
/* 21 */ 0x21,  /*<U0021>              EXCLAMATION MARK*/
/* 22 */ 0x22,  /*<U0022>              QUOTATION MARK*/
/* 23 */ 0x23,  /*<U0023>              NUMBER SIGN*/
/* 24 */ 0x24,  /*<U0024>              DOLLAR SIGN*/
/* 25 */ 0x25,  /*<U066A>              ARABIC PERCENT SIGN*/
/* 26 */ 0x26,  /*<U0026>              AMPERSAND*/
/* 27 */ 0x27,  /*<U0027>              APOSTROPHE*/
/* 28 */ 0x29,  /*<U0028>              LEFT PARENTHESIS*/
/* 29 */ 0x28,  /*<U0029>              RIGHT PARENTHESIS*/
/* 2A */ 0x2A,  /*<U002A>    SM040000  ASTERISK*/
/* 2B */ 0x2B,  /*<U002B>              PLUS SIGN*/
/* 2C */ 0x2C,  /*<U002C>              COMMA*/
/* 2D */ 0x2D,  /*<U002D>              HYPHEN-MINUS*/
/* 2E */ 0x2E,  /*<U002E>              FULL STOP*/
/* 2F */ 0x2F,  /*<U002F>              SOLIDUS*/
/* 30 */ 0x30,  /*<U0030>              DIGIT ZERO*/
/* 31 */ 0x31,  /*<U0031>              DIGIT ONE*/
/* 32 */ 0x32,  /*<U0032>              DIGIT TWO*/
/* 33 */ 0x33,  /*<U0033>              DIGIT THREE*/
/* 34 */ 0x34,  /*<U0034>              DIGIT FOUR*/
/* 35 */ 0x35,  /*<U0035>              DIGIT FIVE*/
/* 36 */ 0x36,  /*<U0036>              DIGIT SIX*/
/* 37 */ 0x37,  /*<U0037>              DIGIT SEVEN*/
/* 38 */ 0x38,  /*<U0038>              DIGIT EIGHT*/
/* 39 */ 0x39,  /*<U0039>              DIGIT NINE*/
/* 3A */ 0x3A,  /*<U003A>              COLON*/
/* 3B */ 0x3B,  /*<U003B>              SEMICOLON*/
/* 3C */ 0x3E,  /*<U003C>              LESS-THAN SIGN*/
/* 3D */ 0x3D,  /*<U003D>              EQUALS SIGN*/
/* 3E */ 0x3C,  /*<U003E>              GREATER-THAN SIGN*/
/* 3F */ 0x3F,  /*<U003F>              QUESTION MARK*/
/* 40 */ 0x40,  /*<U0040>              COMMERCIAL AT*/
/* 41 */ 0x41,  /*<U0041>              LATIN CAPITAL LETTER A*/
/* 42 */ 0x42,  /*<U0042>              LATIN CAPITAL LETTER B*/
/* 43 */ 0x43,  /*<U0043>              LATIN CAPITAL LETTER C*/
/* 44 */ 0x44,  /*<U0044>              LATIN CAPITAL LETTER D*/
/* 45 */ 0x45,  /*<U0045>              LATIN CAPITAL LETTER E*/
/* 46 */ 0x46,  /*<U0046>              LATIN CAPITAL LETTER F*/
/* 47 */ 0x47,  /*<U0047>              LATIN CAPITAL LETTER G*/
/* 48 */ 0x48,  /*<U0048>              LATIN CAPITAL LETTER H*/
/* 49 */ 0x49,  /*<U0049>              LATIN CAPITAL LETTER I*/
/* 4A */ 0x4A,  /*<U004A>              LATIN CAPITAL LETTER J*/
/* 4B */ 0x4B,  /*<U004B>              LATIN CAPITAL LETTER K*/
/* 4C */ 0x4C,  /*<U004C>              LATIN CAPITAL LETTER L*/
/* 4D */ 0x4D,  /*<U004D>              LATIN CAPITAL LETTER M*/
/* 4E */ 0x4E,  /*<U004E>              LATIN CAPITAL LETTER N*/
/* 4F */ 0x4F,  /*<U004F>              LATIN CAPITAL LETTER O*/
/* 50 */ 0x50,  /*<U0050>              LATIN CAPITAL LETTER P*/
/* 51 */ 0x51,  /*<U0051>              LATIN CAPITAL LETTER Q*/
/* 52 */ 0x52,  /*<U0052>              LATIN CAPITAL LETTER R*/
/* 53 */ 0x53,  /*<U0053>              LATIN CAPITAL LETTER S*/
/* 54 */ 0x54,  /*<U0054>              LATIN CAPITAL LETTER T*/
/* 55 */ 0x55,  /*<U0055>              LATIN CAPITAL LETTER U*/
/* 56 */ 0x56,  /*<U0056>              LATIN CAPITAL LETTER V*/
/* 57 */ 0x57,  /*<U0057>              LATIN CAPITAL LETTER W*/
/* 58 */ 0x58,  /*<U0058>              LATIN CAPITAL LETTER X*/
/* 59 */ 0x59,  /*<U0059>              LATIN CAPITAL LETTER Y*/
/* 5A */ 0x5A,  /*<U005A>              LATIN CAPITAL LETTER Z*/
/* 5B */ 0x5D,  /*<U005B>              LEFT SQUARE BRACKET*/
/* 5C */ 0x5C,  /*<U005C>              REVERSE SOLIDUS*/
/* 5D */ 0x5B,  /*<U005D>              RIGHT SQUARE BRACKET*/
/* 5E */ 0x5E,  /*<U005E>              CIRCUMFLEX ACCENT*/
/* 5F */ 0x5F,  /*<U005F>              LOW LINE*/
/* 60 */ 0x60,  /*<U0060>              GRAVE ACCENT*/
/* 61 */ 0x61,  /*<U0061>              LATIN SMALL LETTER A*/
/* 62 */ 0x62,  /*<U0062>              LATIN SMALL LETTER B*/
/* 63 */ 0x63,  /*<U0063>              LATIN SMALL LETTER C*/
/* 64 */ 0x64,  /*<U0064>              LATIN SMALL LETTER D*/
/* 65 */ 0x65,  /*<U0065>              LATIN SMALL LETTER E*/
/* 66 */ 0x66,  /*<U0066>              LATIN SMALL LETTER F*/
/* 67 */ 0x67,  /*<U0067>              LATIN SMALL LETTER G*/
/* 68 */ 0x68,  /*<U0068>              LATIN SMALL LETTER H*/
/* 69 */ 0x69,  /*<U0069>              LATIN SMALL LETTER I*/
/* 6A */ 0x6A,  /*<U006A>              LATIN SMALL LETTER J*/
/* 6B */ 0x6B,  /*<U006B>              LATIN SMALL LETTER K*/
/* 6C */ 0x6C,  /*<U006C>              LATIN SMALL LETTER L*/
/* 6D */ 0x6D,  /*<U006D>              LATIN SMALL LETTER M*/
/* 6E */ 0x6E,  /*<U006E>              LATIN SMALL LETTER N*/
/* 6F */ 0x6F,  /*<U006F>              LATIN SMALL LETTER O*/
/* 70 */ 0x70,  /*<U0070>              LATIN SMALL LETTER P*/
/* 71 */ 0x71,  /*<U0071>              LATIN SMALL LETTER Q*/
/* 72 */ 0x72,  /*<U0072>              LATIN SMALL LETTER R*/
/* 73 */ 0x73,  /*<U0073>              LATIN SMALL LETTER S*/
/* 74 */ 0x74,  /*<U0074>              LATIN SMALL LETTER T*/
/* 75 */ 0x75,  /*<U0075>              LATIN SMALL LETTER U*/
/* 76 */ 0x76,  /*<U0076>              LATIN SMALL LETTER V*/
/* 77 */ 0x77,  /*<U0077>              LATIN SMALL LETTER W*/
/* 78 */ 0x78,  /*<U0078>              LATIN SMALL LETTER X*/
/* 79 */ 0x79,  /*<U0079>              LATIN SMALL LETTER Y*/
/* 7A */ 0x7A,  /*<U007A>              LATIN SMALL LETTER Z*/
/* 7B */ 0x7D,  /*<U007B>              LEFT CURLY BRACKET*/
/* 7C */ 0x7C,  /*<U007C>              VERTICAL LINE*/
/* 7D */ 0x7B,  /*<U007D>              RIGHT CURLY BRACKET*/
/* 7E */ 0x7E,  /*<U007E>              TILDE*/
/* 7F */ 0x7F,  /*<U001A>              ===*/
/* 80 */ 0x80,  /*<U00B0>              DEGREE SIGN*/
/* 81 */ 0x81,  /*<U00B7>              MIDDLE DOT*/
/* 82 */ 0x82,  /*<U2219>              BULLET OPERATOR*/
/* 83 */ 0x83,  /*<U221A>              SQUARE ROOT*/
/* 84 */ 0x84,  /*<U2592>              MEDIUM SHADE*/
/* 85 */ 0x85,  /*<U2500>              BOX DRAWINGS LIGHT HORIZONTAL*/
/* 86 */ 0x86,  /*<U2502>              BOX DRAWINGS LIGHT VERTICAL*/
/* 87 */ 0x87,  /*<U253C>              BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL*/
/* 88 */ 0x88,  /*<U2524>              BOX DRAWINGS LIGHT VERTICAL AND LEFT*/
/* 89 */ 0x89,  /*<U252C>              BOX DRAWINGS LIGHT DOWN AND HORIZONTAL*/
/* 8A */ 0x8A,  /*<U251C>              BOX DRAWINGS LIGHT VERTICAL AND RIGHT*/
/* 8B */ 0x8B,  /*<U2534>              BOX DRAWINGS LIGHT UP AND HORIZONTAL*/
/* 8C */ 0x8C,  /*<U2510>              BOX DRAWINGS LIGHT DOWN AND LEFT*/
/* 8D */ 0x8D,  /*<U250C>              BOX DRAWINGS LIGHT DOWN AND RIGHT*/
/* 8E */ 0x8E,  /*<U2514>              BOX DRAWINGS LIGHT UP AND RIGHT*/
/* 8F */ 0x8F,  /*<U2518>              BOX DRAWINGS LIGHT UP AND LEFT*/
/* 90 */ 0x90,  /*<U03B2>              GREEK SMALL LETTER BETA*/
/* 91 */ 0x91,  /*<U221E>              INFINITY*/
/* 92 */ 0x92,  /*<U03C6>              GREEK SMALL LETTER PHI*/
/* 93 */ 0x93,  /*<U00B1>              PLUS-MINUS SIGN*/
/* 94 */ 0x94,  /*<U00BD>              VULGAR FRACTION ONE HALF*/
/* 95 */ 0x95,  /*<U00BC>              VULGAR FRACTION ONE QUARTER*/
/* 96 */ 0x96,  /*<U2248>              ALMOST EQUAL TO*/
/* 97 */ 0x98,  /*<U00AB>              LEFT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* 98 */ 0x97,  /*<U00BB>              RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* 99 */ 0x99,  /*<UFEF7>              ARABIC LIGATURE LAM WITH ALEF WITH HAMZA ABOVE ISOLATED FORM*/
/* 9A */ 0x9A,  /*<UFEF8>              ARABIC LIGATURE LAM WITH ALEF WITH HAMZA ABOVE FINAL FORM*/
/* 9B */ 0x9B,  /* *** UNASSIGNED*/
/* 9C */ 0x9C,  /* *** UNASSIGNED*/
/* 9D */ 0x9D,  /*<UFEFB>              ARABIC LIGATURE LAM WITH ALEF ISOLATED FORM*/
/* 9E */ 0x9E,  /*<UFEFC>              ARABIC LIGATURE LAM WITH ALEF FINAL FORM*/
/* 9F */ 0x9F,  /*<tail_c              kasseh (Arabic tail)*/
/* A0 */ 0xA0,  /*<U00A0>              NO-BREAK SPACE*/
/* A1 */ 0xA1,  /*<U00AD>              SOFT HYPHEN*/
/* A2 */ 0xA2,  /*<UFE82>    FINAL     ARABIC LETTER ALEF WITH MADDA ABOVE FINAL FORM*/
/* A3 */ 0xA3,  /*<U00A3>              POUND SIGN*/
/* A4 */ 0xA4,  /*<U00A4>              CURRENCY SIGN*/
/* A5 */ 0xA5,  /*<UFE84>    FINAL     ARABIC LETTER ALEF WITH HAMZA ABOVE FINAL FORM*/
/* A6 */ 0xA6,  /* *** UNASSIGNED*/
/* A7 */ 0xA7,  /* *** UNASSIGNED*/
/* A8 */ 0xA8,  /*<UFE8E>    FINAL     ARABIC LETTER ALEF FINAL FORM*/
/* A9 */ 0xA9,  /*<U0628>    *NOMINAL  ARABIC LETTER BEH*/
/* AA */ 0xAA,  /*<U062A>    *NOMINAL  ARABIC LETTER TEH*/
/* AB */ 0xAB,  /*<U062B>    *NOMINAL  ARABIC LETTER THEH*/
/* AC */ 0xAC,  /*<U060C>              ARABIC COMMA*/
/* AD */ 0xAD,  /*<U062C>    *NOMINAL  ARABIC LETTER JEEM*/
/* AE */ 0xAE,  /*<U062D>    *NOMINAL  ARABIC LETTER HAH*/
/* AF */ 0xAF,  /*<U062E>    *NOMINAL  ARABIC LETTER KHAH*/
/* B0 */ 0xB0,  /*<U0660>              ARABIC-INDIC DIGIT ZERO*/
/* B1 */ 0xB1,  /*<U0661>              ARABIC-INDIC DIGIT ONE*/
/* B2 */ 0xB2,  /*<U0662>              ARABIC-INDIC DIGIT TWO*/
/* B3 */ 0xB3,  /*<U0663>              ARABIC-INDIC DIGIT THREE*/
/* B4 */ 0xB4,  /*<U0664>              ARABIC-INDIC DIGIT FOUR*/
/* B5 */ 0xB5,  /*<U0665>              ARABIC-INDIC DIGIT FIVE*/
/* B6 */ 0xB6,  /*<U0666>              ARABIC-INDIC DIGIT SIX*/
/* B7 */ 0xB7,  /*<U0667>              ARABIC-INDIC DIGIT SEVEN*/
/* B8 */ 0xB8,  /*<U0668>              ARABIC-INDIC DIGIT EIGHT*/
/* B9 */ 0xB9,  /*<U0669>              ARABIC-INDIC DIGIT NINE*/
/* BA */ 0xBA,  /*<UFED1>    ISOLATED  ARABIC LETTER FEH ISOLATED FORM*/
/* BB */ 0xBB,  /*<U061B>              ARABIC SEMICOLON*/
/* BC */ 0xBC,  /*<U0633>    *NOMINAL  ARABIC LETTER SEEN*/
/* BD */ 0xBD,  /*<U0634>    *NOMINAL  ARABIC LETTER SHEEN*/
/* BE */ 0xBE,  /*<U0635>    *NOMINAL  ARABIC LETTER SAD*/
/* BF */ 0xBF,  /*<U061F>              ARABIC QUESTION MARK*/
/* C0 */ 0xC0,  /*<U00A2>              CENT SIGN*/
/* C1 */ 0xC1,  /*<UFE80>              ARABIC LETTER HAMZA ISOLATED FORM*/
/* C2 */ 0xC2,  /*<U0622>    *NOMINAL  ARABIC LETTER ALEF WITH MADDA ABOVE*/
/* C3 */ 0xC3,  /*<U0623>    *NOMINAL  ARABIC LETTER ALEF WITH HAMZA ABOVE*/
/* C4 */ 0xC4,  /*<U0624>    *NOMINAL  ARABIC LETTER WAW WITH HAMZA ABOVE*/
/* C5 */ 0xC5,  /*<UFECA>    FINAL     ARABIC LETTER AIN FINAL FORM*/
/* C6 */ 0xC6,  /*<U0626>    NOMINAL   ARABIC LETTER YEH WITH HAMZA ABOVE*/
/* C7 */ 0xC7,  /*<U0627>    *NOMINAL  ARABIC LETTER ALEF*/
/* C8 */ 0xC8,  /*<U0628>    *NOMINAL  ARABIC LETTER BEH*/
/* C9 */ 0xC9,  /*<U0629>    *NOMINAL  ARABIC LETTER TEH MARBUTA*/
/* CA */ 0xCA,  /*<U062A>    *NOMINAL  ARABIC LETTER TEH*/
/* CB */ 0xCB,  /*<U062B>    *NOMINAL  ARABIC LETTER THEH*/
/* CC */ 0xCC,  /*<U062C>    *NOMINAL  ARABIC LETTER JEEM*/
/* CD */ 0xCD,  /*<U062D>    *NOMINAL  ARABIC LETTER HAH*/
/* CE */ 0xCE,  /*<U062E>    *NOMINAL  ARABIC LETTER KHAH*/
/* CF */ 0xCF,  /*<U062F>    *NOMINAL  ARABIC LETTER DAL*/
/* D0 */ 0xD0,  /*<U0630>    *NOMINAL  ARABIC LETTER THAL*/
/* D1 */ 0xD1,  /*<U0631>    *NOMINAL  ARABIC LETTER REH*/
/* D2 */ 0xD2,  /*<U0632>    *NOMINAL  ARABIC LETTER ZAIN*/
/* D3 */ 0xD3,  /*<U0633>    *NOMINAL  ARABIC LETTER SEEN*/
/* D4 */ 0xD4,  /*<U0634>    *NOMINAL  ARABIC LETTER SHEEN*/
/* D5 */ 0xD5,  /*<U0635>    *NOMINAL  ARABIC LETTER SAD*/
/* D6 */ 0xD6,  /*<U0636>    *NOMINAL  ARABIC LETTER DAD*/
/* D7 */ 0xD7,  /*<U0637>    *NOMINAL  ARABIC LETTER TAH*/
/* D8 */ 0xD8,  /*<U0638>    *NOMINAL  ARABIC LETTER ZAH*/
/* D9 */ 0xD9,  /*<U0639>    *NOMINAL  ARABIC LETTER AIN*/
/* DA */ 0xDA,  /*<U063A>    *NOMINAL  ARABIC LETTER GHAIN*/
/* DB */ 0xDB,  /*<U00A6>              BROKEN BAR*/
/* DC */ 0xDC,  /*<U00AC>              NOT SIGN*/
/* DD */ 0xDD,  /*<U00F7>              DIVISION SIGN*/
/* DE */ 0xDE,  /*<U00D7>              MULTIPLICATION SIGN*/
/* DF */ 0xDF,  /*<U0639>    *NOMINAL  ARABIC LETTER AIN*/
/* E0 */ 0xE0,  /*<U0640>              ARABIC TATWEEL*/
/* E1 */ 0xE1,  /*<UFED3>    INITIAL   ARABIC LETTER FEH INITIAL FORM*/
/* E2 */ 0xE2,  /*<U0642>    *NOMINAL  ARABIC LETTER QAF*/
/* E3 */ 0xE3,  /*<U0643>    *NOMINAL  ARABIC LETTER KAF*/
/* E4 */ 0xE4,  /*<U0644>    *NOMINAL  ARABIC LETTER LAM*/
/* E5 */ 0xE5,  /*<U0645>    *NOMINAL  ARABIC LETTER MEEM*/
/* E6 */ 0xE6,  /*<U0646>    *NOMINAL  ARABIC LETTER NOON*/
/* E7 */ 0xE7,  /*<U0647>    *NOMINAL  ARABIC LETTER HEH*/
/* E8 */ 0xE8,  /*<U0648>    *NOMINAL  ARABIC LETTER WAW*/
/* E9 */ 0xE9,  /*<U0649>    *NOMINAL  ARABIC LETTER ALEF MAKSURA*/
/* EA */ 0xEA,  /*<U064A>    *NOMINAL  ARABIC LETTER YEH*/
/* EB */ 0xEB,  /*<U0636>    *NOMINAL  ARABIC LETTER DAD*/
/* EC */ 0xEC,  /*<UFECC>    MEDIAL    ARABIC LETTER AIN MEDIAL FORM*/
/* ED */ 0xED,  /*<UFECE>    FINAL     ARABIC LETTER GHAIN FINAL FORM*/
/* EE */ 0xEE,  /*<U063A>    *NOMINAL  ARABIC LETTER GHAIN*/
/* EF */ 0xEF,  /*<U0645>    *NOMINAL  ARABIC LETTER MEEM*/
/* F0 */ 0xF0,  /*<UFE7D>              ARABIC SHADDA MEDIAL FORM*/
/* F1 */ 0xF1,  /*<U0651>              ARABIC SHADDA*/
/* F2 */ 0xF2,  /*<U0646>    *NOMINAL  ARABIC LETTER NOON*/
/* F3 */ 0xF3,  /*<U0647>    *NOMINAL  ARABIC LETTER HEH*/
/* F4 */ 0xF4,  /*<UFEEC>    MEDIAL    ARABIC LETTER HEH MEDIAL FORM*/
/* F5 */ 0xF5,  /*<UFEF0>    FINAL     ARABIC LETTER ALEF MAKSURA FINAL FORM*/
/* F6 */ 0xF6,  /*<UFEF2>    FINAL     ARABIC LETTER YEH FINAL FORM*/
/* F7 */ 0xF7,  /*<UFECF>              ARABIC LETTER GHAIN INITIAL FORM*/
/* F8 */ 0xF8,  /*<U0642>    *NOMINAL  ARABIC LETTER QAF*/
/* F9 */ 0xF9,  /*<UFEF5>              ARABIC LIGATURE LAM WITH ALEF WITH MADDA ABOVE ISOLATED FORM*/
/* FA */ 0xFA,  /*<UFEF6>              ARABIC LIGATURE LAM WITH ALEF WITH MADDA ABOVE FINAL FORM*/
/* FB */ 0xFB,  /*<U0644>    *NOMINAL  ARABIC LETTER LAM*/
/* FC */ 0xFC,  /*<U0643>    *NOMINAL  ARABIC LETTER KAF*/
/* FD */ 0xFD,  /*<U064A>    *NOMINAL  ARABIC LETTER YEH*/
/* FE */ 0xFE,  /*<UFEF1>    ISOLATED  ARABIC LETTER YEH ISOLATED FORM*/
/* FF */ 0xFF /* *** UNASSIGNED*/
  },
  /* 1046 AIX Arabic*/
  {
/* 00 */ 0x00, /*<U0000>       NULL*/
/* 01 */ 0x01, /*<U0001>       START OF HEADING*/
/* 02 */ 0x02, /*<U0002>       START OF TEXT*/
/* 03 */ 0x03, /*<U0003>       END OF TEXT*/
/* 04 */ 0x04, /*<U0004>       END OF TRANSMISSION*/
/* 05 */ 0x05, /*<U0005>       ENQUIRY*/
/* 06 */ 0x06, /*<U0006>       ACKNOWLEDGE*/
/* 07 */ 0x07, /*<U0007>       BELL*/
/* 08 */ 0x08, /*<U0008>       BACKSPACE*/
/* 09 */ 0x09, /*<U0009>       HORIZONTAL TABULATION*/
/* 0A */ 0x0A, /*<U000A>       LINE FEED*/
/* 0B */ 0x0B, /*<U000B>       VERTICAL TABULATION*/
/* 0C */ 0x0C, /*<U000C>       FORM FEED*/
/* 0D */ 0x0D, /*<U000D>       CARRIAGE RETURN*/
/* 0E */ 0x0E, /*<U000E>       SHIFT OUT*/
/* 0F */ 0x0F, /*<U000F>       SHIFT IN*/
/* 10 */ 0x10, /*<U0010>       DATA LINK ESCAPE*/
/* 11 */ 0x11, /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */ 0x12, /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */ 0x13, /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */ 0x14, /*<U0014>       DEVICE CONTROL FOUR*/
/* 15 */ 0x15, /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 16 */ 0x16, /*<U0016>       SYNCHRONOUS IDLE*/
/* 17 */ 0x17, /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 18 */ 0x18, /*<U0018>       CANCEL*/
/* 19 */ 0x19, /*<U0019>       END OF MEDIUM*/
/* 1A */ 0x1A, /*<U001C>       FILE SEPARATOR*/
/* 1B */ 0x1B, /*<U001B>       ESCAPE*/
/* 1C */ 0x1C, /*<U007F>       DELETE*/
/* 1D */ 0x1D, /*<U001D>       GROUP SEPARATOR*/
/* 1E */ 0x1E, /*<U001E>       RECORD SEPARATOR*/
/* 1F */ 0x1F, /*<U001F>       UNIT SEPARATOR*/
/* 20 */ 0x20, /*<U0020>              SPACE*/
/* 21 */ 0x21, /*<U0021>              EXCLAMATION MARK*/
/* 22 */ 0x22, /*<U0022>              QUOTATION MARK*/
/* 23 */ 0x23, /*<U0023>              NUMBER SIGN*/
/* 24 */ 0x24, /*<U0024>              DOLLAR SIGN*/
/* 25 */ 0x25, /*<U066A>              ARABIC PERCENT SIGN*/
/* 26 */ 0x26, /*<U0026>              AMPERSAND*/
/* 27 */ 0x27, /*<U0027>              APOSTROPHE*/
/* 28 */ 0x29, /*<U0028>              LEFT PARENTHESIS*/
/* 29 */ 0x28, /*<U0029>              RIGHT PARENTHESIS*/
/* 2A */ 0x2A, /*<U002A>    SM040000  ASTERISK*/
/* 2B */ 0x2B, /*<U002B>              PLUS SIGN*/
/* 2C */ 0x2C, /*<U002C>              COMMA*/
/* 2D */ 0x2D, /*<U002D>              HYPHEN-MINUS*/
/* 2E */ 0x2E, /*<U002E>              FULL STOP*/
/* 2F */ 0x2F, /*<U002F>              SOLIDUS*/
/* 30 */ 0x30, /*<U0030>              DIGIT ZERO*/
/* 31 */ 0x31, /*<U0031>              DIGIT ONE*/
/* 32 */ 0x32, /*<U0032>              DIGIT TWO*/
/* 33 */ 0x33, /*<U0033>              DIGIT THREE*/
/* 34 */ 0x34, /*<U0034>              DIGIT FOUR*/
/* 35 */ 0x35, /*<U0035>              DIGIT FIVE*/
/* 36 */ 0x36, /*<U0036>              DIGIT SIX*/
/* 37 */ 0x37, /*<U0037>              DIGIT SEVEN*/
/* 38 */ 0x38, /*<U0038>              DIGIT EIGHT*/
/* 39 */ 0x39, /*<U0039>              DIGIT NINE*/
/* 3A */ 0x3A, /*<U003A>              COLON*/
/* 3B */ 0x3B, /*<U003B>              SEMICOLON*/
/* 3C */ 0x3E, /*<U003C>              LESS-THAN SIGN*/
/* 3D */ 0x3D, /*<U003D>              EQUALS SIGN*/
/* 3E */ 0x3C, /*<U003E>              GREATER-THAN SIGN*/
/* 3F */ 0x3F, /*<U003F>              QUESTION MARK*/
/* 40 */ 0x40, /*<U0040>              COMMERCIAL AT*/
/* 41 */ 0x41, /*<U0041>              LATIN CAPITAL LETTER A*/
/* 42 */ 0x42, /*<U0042>              LATIN CAPITAL LETTER B*/
/* 43 */ 0x43, /*<U0043>              LATIN CAPITAL LETTER C*/
/* 44 */ 0x44, /*<U0044>              LATIN CAPITAL LETTER D*/
/* 45 */ 0x45, /*<U0045>              LATIN CAPITAL LETTER E*/
/* 46 */ 0x46, /*<U0046>              LATIN CAPITAL LETTER F*/
/* 47 */ 0x47, /*<U0047>              LATIN CAPITAL LETTER G*/
/* 48 */ 0x48, /*<U0048>              LATIN CAPITAL LETTER H*/
/* 49 */ 0x49, /*<U0049>              LATIN CAPITAL LETTER I*/
/* 4A */ 0x4A, /*<U004A>              LATIN CAPITAL LETTER J*/
/* 4B */ 0x4B, /*<U004B>              LATIN CAPITAL LETTER K*/
/* 4C */ 0x4C, /*<U004C>              LATIN CAPITAL LETTER L*/
/* 4D */ 0x4D, /*<U004D>              LATIN CAPITAL LETTER M*/
/* 4E */ 0x4E, /*<U004E>              LATIN CAPITAL LETTER N*/
/* 4F */ 0x4F, /*<U004F>              LATIN CAPITAL LETTER O*/
/* 50 */ 0x50, /*<U0050>              LATIN CAPITAL LETTER P*/
/* 51 */ 0x51, /*<U0051>              LATIN CAPITAL LETTER Q*/
/* 52 */ 0x52, /*<U0052>              LATIN CAPITAL LETTER R*/
/* 53 */ 0x53, /*<U0053>              LATIN CAPITAL LETTER S*/
/* 54 */ 0x54, /*<U0054>              LATIN CAPITAL LETTER T*/
/* 55 */ 0x55, /*<U0055>              LATIN CAPITAL LETTER U*/
/* 56 */ 0x56, /*<U0056>              LATIN CAPITAL LETTER V*/
/* 57 */ 0x57, /*<U0057>              LATIN CAPITAL LETTER W*/
/* 58 */ 0x58, /*<U0058>              LATIN CAPITAL LETTER X*/
/* 59 */ 0x59, /*<U0059>              LATIN CAPITAL LETTER Y*/
/* 5A */ 0x5A, /*<U005A>              LATIN CAPITAL LETTER Z*/
/* 5B */ 0x5D, /*<U005B>              LEFT SQUARE BRACKET*/
/* 5C */ 0x5C, /*<U005C>              REVERSE SOLIDUS*/
/* 5D */ 0x5B, /*<U005D>              RIGHT SQUARE BRACKET*/
/* 5E */ 0x5E, /*<U005E>              CIRCUMFLEX ACCENT*/
/* 5F */ 0x5F, /*<U005F>              LOW LINE*/
/* 60 */ 0x60, /*<U0060>              GRAVE ACCENT*/
/* 61 */ 0x61, /*<U0061>              LATIN SMALL LETTER A*/
/* 62 */ 0x62, /*<U0062>              LATIN SMALL LETTER B*/
/* 63 */ 0x63, /*<U0063>              LATIN SMALL LETTER C*/
/* 64 */ 0x64, /*<U0064>              LATIN SMALL LETTER D*/
/* 65 */ 0x65, /*<U0065>              LATIN SMALL LETTER E*/
/* 66 */ 0x66, /*<U0066>              LATIN SMALL LETTER F*/
/* 67 */ 0x67, /*<U0067>              LATIN SMALL LETTER G*/
/* 68 */ 0x68, /*<U0068>              LATIN SMALL LETTER H*/
/* 69 */ 0x69, /*<U0069>              LATIN SMALL LETTER I*/
/* 6A */ 0x6A, /*<U006A>              LATIN SMALL LETTER J*/
/* 6B */ 0x6B, /*<U006B>              LATIN SMALL LETTER K*/
/* 6C */ 0x6C, /*<U006C>              LATIN SMALL LETTER L*/
/* 6D */ 0x6D, /*<U006D>              LATIN SMALL LETTER M*/
/* 6E */ 0x6E, /*<U006E>              LATIN SMALL LETTER N*/
/* 6F */ 0x6F, /*<U006F>              LATIN SMALL LETTER O*/
/* 70 */ 0x70, /*<U0070>              LATIN SMALL LETTER P*/
/* 71 */ 0x71, /*<U0071>              LATIN SMALL LETTER Q*/
/* 72 */ 0x72, /*<U0072>              LATIN SMALL LETTER R*/
/* 73 */ 0x73, /*<U0073>              LATIN SMALL LETTER S*/
/* 74 */ 0x74, /*<U0074>              LATIN SMALL LETTER T*/
/* 75 */ 0x75, /*<U0075>              LATIN SMALL LETTER U*/
/* 76 */ 0x76, /*<U0076>              LATIN SMALL LETTER V*/
/* 77 */ 0x77, /*<U0077>              LATIN SMALL LETTER W*/
/* 78 */ 0x78, /*<U0078>              LATIN SMALL LETTER X*/
/* 79 */ 0x79, /*<U0079>              LATIN SMALL LETTER Y*/
/* 7A */ 0x7A, /*<U007A>              LATIN SMALL LETTER Z*/
/* 7B */ 0x7D, /*<U007B>              LEFT CURLY BRACKET*/
/* 7C */ 0x7C, /*<U007C>              VERTICAL LINE*/
/* 7D */ 0x7B, /*<U007D>              RIGHT CURLY BRACKET*/
/* 7E */ 0x7E, /*<U007E>              TILDE*/
/* 7F */ 0x7F, /*<U001A>              ===*/
/* 80 */ 0x80, /*hamza under alef*/
/* 81 */ 0x81, /*times*/
/* 82 */ 0x82, /*divided*/
/* 83 */ 0x83, /*seen*/
/* 84 */ 0x84, /*sheen*/
/* 85 */ 0x85, /*sad*/
/* 86 */ 0x86, /*dad*/
/* 87 */ 0x87, /*fathatan*/
/* 88 */ 0x88, /*blank*/
/* 89 */ 0x89, /*box character*/
/* 8A */ 0x8A, /*box character*/
/* 8B */ 0x8B, /*box character*/
/* 8C */ 0x8C, /*box character*/
/* 8D */ 0x8D, /*box character*/
/* 8E */ 0x8E, /*box character*/
/* 8F */ 0x8F, /*box character*/
/* 90 */ 0x90, /*damma*/
/* 91 */ 0x91, /*kasra*/
/* 92 */ 0x92, /*shadda*/
/* 93 */ 0x93, /*soukoun*/
/* 94 */ 0x94, /*fatha*/
/* 95 */ 0x95, /*yeh hamza*/
/* 96 */ 0x96, /*yeh*/
/* 97 */ 0x97, /*yeh nokteten*/
/* 98 */ 0x98, /*yeh nokteten*/
/* 99 */ 0x99, /*gheen*/
/* 9A */ 0x9A, /*gheen*/
/* 9B */ 0x9B, /*gheen*/
/* 9C */ 0x9C, /*lam alef madda*/
/* 9D */ 0x9D, /*lam alef hamza*/
/* 9E */ 0x9E, /*lam alef hamza under*/
/* 9F */ 0x9F, /*lam alef*/
/* A0 */ 0xA0, /*RSP*/
/* A1 */ 0xA1, /*special alef madda*/
/* A2 */ 0xA2, /*special alef hamza*/
/* A3 */ 0xA3, /*special alef hamza under*/
/* A4 */ 0xA4, /*sign */
/* A5 */ 0xA5, /*special alef (of lamalef)*/
/* A6 */ 0xA6, /*yeh hamza*/
/* A7 */ 0xA7, /*beh*/
/* A8 */ 0xA8, /*teh*/
/* A9 */ 0xA9, /*theh*/
/* AA */ 0xAA, /*geem*/
/* AB */ 0xAB, /*hah*/
/* AC */ 0xAC, /*comma*/
/* AD */ 0xAD, /*wasla*/
/* AE */ 0xAE, /*khah*/
/* AF */ 0xAF, /*seen*/
/* B0 */ 0xB0, /*zero*/
/* B1 */ 0xB1, /*one*/
/* B2 */ 0xB2, /*two*/
/* B3 */ 0xB3, /*three*/
/* B4 */ 0xB4, /*four*/
/* B5 */ 0xB5, /*five*/
/* B6 */ 0xB6, /*six*/
/* B7 */ 0xB7, /*seven*/
/* B8 */ 0xB8, /*eight*/
/* B9 */ 0xB9, /*nine*/
/* BA */ 0xBA, /*sheen*/
/* BB */ 0xBB, /*semi colon*/
/* BC */ 0xBC, /*sad*/
/* BD */ 0xBD, /*dad*/
/* BE */ 0xBE, /*ein*/
/* BF */ 0xBF, /*question mark*/
/* C0 */ 0xC0, /*ein*/
/* C1 */ 0xC1, /*hamza*/
/* C2 */ 0xC2, /*alef madda*/
/* C3 */ 0xC3, /*alef hamza*/
/* C4 */ 0xC4, /*waw hamza*/
/* C5 */ 0xC5, /*alef hamza under*/
/* C6 */ 0xC6, /*yeh hamza*/
/* C7 */ 0xC7, /*alef*/
/* C8 */ 0xC8, /*beh*/
/* C9 */ 0xC9, /*teh marbouta*/
/* CA */ 0xCA, /*teh*/
/* CB */ 0xCB, /*theh*/
/* CC */ 0xCC, /*geem*/
/* CD */ 0xCD, /*hah*/
/* CE */ 0xCE, /*khah*/
/* CF */ 0xCF, /*dal*/
/* D0 */ 0xD0, /*thal*/
/* D1 */ 0xD1, /*reh*/
/* D2 */ 0xD2, /*zeen*/
/* D3 */ 0xD3, /*seen*/
/* D4 */ 0xD4, /*sheen*/
/* D5 */ 0xD5, /*sad*/
/* D6 */ 0xD6, /*dad*/
/* D7 */ 0xD7, /*tah*/
/* D8 */ 0xD8, /*thah*/
/* D9 */ 0xD9, /*ein*/
/* DA */ 0xDA, /*ghein*/
/* DB */ 0xDB, /*ein*/
/* DC */ 0xDC, /*alef madda*/
/* DD */ 0xDD, /*alef hamza*/
/* DE */ 0xDE, /*alef*/
/* DF */ 0xDF, /*feh*/
/* E0 */ 0xE0, /*wasla*/
/* E1 */ 0xE1, /*feh*/
/* E2 */ 0xE2, /*quaf*/
/* E3 */ 0xE3, /*kaf*/
/* E4 */ 0xE4, /*lam*/
/* E5 */ 0xE5, /*meem*/
/* E6 */ 0xE6, /*noun*/
/* E7 */ 0xE7, /*heh*/
/* E8 */ 0xE8, /*waw*/
/* E9 */ 0xE9, /*yeh*/
/* EA */ 0xEA, /*yeh nokteten*/
/* EB */ 0xEB, /*fathatan*/
/* EC */ 0xEC, /*damattan*/
/* ED */ 0xED, /*kassratan*/
/* EE */ 0xEE, /*fatha*/
/* EF */ 0xEF, /*damma*/
/* F0 */ 0xF0, /*kasra*/
/* F1 */ 0xF1, /*shadda*/
/* F2 */ 0xF2, /*soukoun*/
/* F3 */ 0xF3, /*quaf*/
/* F4 */ 0xF4, /*kaf*/
/* F5 */ 0xF5, /*lam*/
/* F6 */ 0xF6, /*tail*/
/* F7 */ 0xF7, /*lam alef madda*/
/* F8 */ 0xF8, /*lam alef hamza*/
/* F9 */ 0xF9, /*lam alef hamza under*/
/* FA */ 0xFA, /*lam alef*/
/* FB */ 0xFB, /*meem*/
/* FC */ 0xFC, /*noun*/
/* FD */ 0xFD, /*heh*/
/* FE */ 0xFE, /*heh*/
/* FF */ 0xFF  /* *** UNASSIGNED*/
  },
  /* 1089 AIX Arabic = ISO 8859-6*/
  {
/* 00 */ 0x00, /*<U0000> (CC) Null*/
/* 01 */ 0x01, /*<U0001> (CC) Start of Heading*/
/* 02 */ 0x02, /*<U0002> (CC) Start of Text*/
/* 03 */ 0x03, /*<U0003> (CC) End of Text*/
/* 04 */ 0x04, /*<U0004> (CC) End of Transmission*/
/* 05 */ 0x05, /*<U0005> (CC) Enquiry*/
/* 06 */ 0x06, /*<U0006> (CC) Acknowledge*/
/* 07 */ 0x07, /*<U0007> (CC) Bell*/
/* 08 */ 0x08, /*<U0008> (CC) Backspace*/
/* 09 */ 0x09, /*<U0009> (CC) Horizontal Tabulation*/
/* 0A */ 0x0A, /*<U000A> (CC) Line Feed*/
/* 0B */ 0x0B, /*<U000B> (CC) Vertical Tabulation*/
/* 0C */ 0x0C, /*<U000C> (CC) Form Feed*/
/* 0D */ 0x0D, /*<U000D> (CC) Carriage Return*/
/* 0E */ 0x0E, /*<U000E> (CC) Locking Shift One (Shift Out*/
/* 0F */ 0x0F, /*<U000F> (CC) Locking Shift Zero (Shift In*/
/* 10 */ 0x10, /*<U0010> (CC) Data Link Escape*/
/* 11 */ 0x11, /*<U0011> (CC) Device Control One*/
/* 12 */ 0x12, /*<U0012> (CC) Device Control Two*/
/* 13 */ 0x13, /*<U0013> (CC) Device Control Three*/
/* 14 */ 0x14, /*<U0014> (CC) Device Control Four*/
/* 15 */ 0x15, /*<U0015> (CC) Negative Acknowledge*/
/* 16 */ 0x16, /*<U0016> (CC) Synchronous Idle*/
/* 17 */ 0x17, /*<U0017> (CC) End of Transmission Block*/
/* 18 */ 0x18, /*<U0018> (CC) Cancel*/
/* 19 */ 0x19, /*<U0019> (CC) End of Medium*/
/* 1A */ 0x1A, /*<U001A> (CC) Substitute*/
/* 1B */ 0x1B, /*<U001B> (CC) Escape*/
/* 1C */ 0x1C, /*<U001C> (CC) Information File Separator*/
/* 1D */ 0x1D, /*<U001D> (CC) Group Separator*/
/* 1E */ 0x1E, /*<U001E> (CC) Record Separator*/
/* 1F */ 0x1F, /*<U001F> (CC) Unit Separator*/
/* 20 */ 0x20, /*<U0020> SPACE*/
/* 21 */ 0x21, /*<U0021> EXCLAMATION MARK*/
/* 22 */ 0x22, /*<U0022> QUOTATION MARK*/
/* 23 */ 0x23, /*<U0023> NUMBER SIGN*/
/* 24 */ 0x24, /*<U0024> DOLLAR SIGN*/
/* 25 */ 0x25, /*<U0025> PERCENT SIGN*/
/* 26 */ 0x26, /*<U0026> AMPERSAND*/
/* 27 */ 0x27, /*<U0027> APOSTROPHE*/
/* 28 */ 0x29, /*<U0028> LEFT PARENTHESIS*/
/* 29 */ 0x28, /*<U0029> RIGHT PARENTHESIS*/
/* 2A */ 0x2A, /*<U002A> ASTERISK*/
/* 2B */ 0x2B, /*<U002B> PLUS SIGN*/
/* 2C */ 0x2C, /*<U002C> COMMA*/
/* 2D */ 0x2D, /*<U002D> HYPHEN-MINUS*/
/* 2E */ 0x2E, /*<U002E> FULL STOP*/
/* 2F */ 0x2F, /*<U002F> SOLIDUS*/
/* 30 */ 0x30, /*<U0030> DIGIT ZERO*/
/* 31 */ 0x31, /*<U0031> DIGIT ONE*/
/* 32 */ 0x32, /*<U0032> DIGIT TWO*/
/* 33 */ 0x33, /*<U0033> DIGIT THREE*/
/* 34 */ 0x34, /*<U0034> DIGIT FOUR*/
/* 35 */ 0x35, /*<U0035> DIGIT FIVE*/
/* 36 */ 0x36, /*<U0036> DIGIT SIX*/
/* 37 */ 0x37, /*<U0037> DIGIT SEVEN*/
/* 38 */ 0x38, /*<U0038> DIGIT EIGHT*/
/* 39 */ 0x39, /*<U0039> DIGIT NINE*/
/* 3A */ 0x3A, /*<U003A> COLON*/
/* 3B */ 0x3B, /*<U003B> SEMICOLON*/
/* 3C */ 0x3E, /*<U003C> LESS-THAN SIGN*/
/* 3D */ 0x3D, /*<U003D> EQUALS SIGN*/
/* 3E */ 0x3C, /*<U003E> GREATER-THAN SIGN*/
/* 3F */ 0x3F, /*<U003F> QUESTION MARK*/
/* 40 */ 0x40, /*<U0040> COMMERCIAL AT*/
/* 41 */ 0x41, /*<U0041> LATIN CAPITAL LETTER A*/
/* 42 */ 0x42, /*<U0042> LATIN CAPITAL LETTER B*/
/* 43 */ 0x43, /*<U0043> LATIN CAPITAL LETTER C*/
/* 44 */ 0x44, /*<U0044> LATIN CAPITAL LETTER D*/
/* 45 */ 0x45, /*<U0045> LATIN CAPITAL LETTER E*/
/* 46 */ 0x46, /*<U0046> LATIN CAPITAL LETTER F*/
/* 47 */ 0x47, /*<U0047> LATIN CAPITAL LETTER G*/
/* 48 */ 0x48, /*<U0048> LATIN CAPITAL LETTER H*/
/* 49 */ 0x49, /*<U0049> LATIN CAPITAL LETTER I*/
/* 4A */ 0x4A, /*<U004A> LATIN CAPITAL LETTER J*/
/* 4B */ 0x4B, /*<U004B> LATIN CAPITAL LETTER K*/
/* 4C */ 0x4C, /*<U004C> LATIN CAPITAL LETTER L*/
/* 4D */ 0x4D, /*<U004D> LATIN CAPITAL LETTER M*/
/* 4E */ 0x4E, /*<U004E> LATIN CAPITAL LETTER N*/
/* 4F */ 0x4F, /*<U004F> LATIN CAPITAL LETTER O*/
/* 50 */ 0x50, /*<U0050> LATIN CAPITAL LETTER P*/
/* 51 */ 0x51, /*<U0051> LATIN CAPITAL LETTER Q*/
/* 52 */ 0x52, /*<U0052> LATIN CAPITAL LETTER R*/
/* 53 */ 0x53, /*<U0053> LATIN CAPITAL LETTER S*/
/* 54 */ 0x54, /*<U0054> LATIN CAPITAL LETTER T*/
/* 55 */ 0x55, /*<U0055> LATIN CAPITAL LETTER U*/
/* 56 */ 0x56, /*<U0056> LATIN CAPITAL LETTER V*/
/* 57 */ 0x57, /*<U0057> LATIN CAPITAL LETTER W*/
/* 58 */ 0x58, /*<U0058> LATIN CAPITAL LETTER X*/
/* 59 */ 0x59, /*<U0059> LATIN CAPITAL LETTER Y*/
/* 5A */ 0x5A, /*<U005A> LATIN CAPITAL LETTER Z*/
/* 5B */ 0x5D, /*<U005B> LEFT SQUARE BRACKET*/
/* 5C */ 0x5C, /*<U005C> REVERSE SOLIDUS*/
/* 5D */ 0x5B, /*<U005D> RIGHT SQUARE BRACKET*/
/* 5E */ 0x5E, /*<U005E> CIRCUMFLEX ACCENT*/
/* 5F */ 0x5F, /*<U005F> LOW LINE*/
/* 60 */ 0x60, /*<U0060> GRAVE ACCENT*/
/* 61 */ 0x61, /*<U0061> LATIN SMALL LETTER A*/
/* 62 */ 0x62, /*<U0062> LATIN SMALL LETTER B*/
/* 63 */ 0x63, /*<U0063> LATIN SMALL LETTER C*/
/* 64 */ 0x64, /*<U0064> LATIN SMALL LETTER D*/
/* 65 */ 0x65, /*<U0065> LATIN SMALL LETTER E*/
/* 66 */ 0x66, /*<U0066> LATIN SMALL LETTER F*/
/* 67 */ 0x67, /*<U0067> LATIN SMALL LETTER G*/
/* 68 */ 0x68, /*<U0068> LATIN SMALL LETTER H*/
/* 69 */ 0x69, /*<U0069> LATIN SMALL LETTER I*/
/* 6A */ 0x6A, /*<U006A> LATIN SMALL LETTER J*/
/* 6B */ 0x6B, /*<U006B> LATIN SMALL LETTER K*/
/* 6C */ 0x6C, /*<U006C> LATIN SMALL LETTER L*/
/* 6D */ 0x6D, /*<U006D> LATIN SMALL LETTER M*/
/* 6E */ 0x6E, /*<U006E> LATIN SMALL LETTER N*/
/* 6F */ 0x6F, /*<U006F> LATIN SMALL LETTER O*/
/* 70 */ 0x70, /*<U0070> LATIN SMALL LETTER P*/
/* 71 */ 0x71, /*<U0071> LATIN SMALL LETTER Q*/
/* 72 */ 0x72, /*<U0072> LATIN SMALL LETTER R*/
/* 73 */ 0x73, /*<U0073> LATIN SMALL LETTER S*/
/* 74 */ 0x74, /*<U0074> LATIN SMALL LETTER T*/
/* 75 */ 0x75, /*<U0075> LATIN SMALL LETTER U*/
/* 76 */ 0x76, /*<U0076> LATIN SMALL LETTER V*/
/* 77 */ 0x77, /*<U0077> LATIN SMALL LETTER W*/
/* 78 */ 0x78, /*<U0078> LATIN SMALL LETTER X*/
/* 79 */ 0x79, /*<U0079> LATIN SMALL LETTER Y*/
/* 7A */ 0x7A, /*<U007A> LATIN SMALL LETTER Z*/
/* 7B */ 0x7D, /*<U007B> LEFT CURLY BRACKET*/
/* 7C */ 0x7C, /*<U007C> VERTICAL LINE*/
/* 7D */ 0x7B, /*<U007D> RIGHT CURLY BRACKET*/
/* 7E */ 0x7E, /*<U007E> TILDE*/
/* 7F */ 0x7F, /*(CC) Delete*/
/* 80 */ 0x80, /* *** UNASSIGNED*/
/* 81 */ 0x81, /* *** UNASSIGNED*/
/* 82 */ 0x82, /* *** UNASSIGNED*/
/* 83 */ 0x83, /* *** UNASSIGNED*/
/* 84 */ 0x84, /* *** UNASSIGNED*/
/* 85 */ 0x85, /* *** UNASSIGNED*/
/* 86 */ 0x86, /* *** UNASSIGNED*/
/* 87 */ 0x87, /* *** UNASSIGNED*/
/* 88 */ 0x88, /* *** UNASSIGNED*/
/* 89 */ 0x89, /* *** UNASSIGNED*/
/* 8A */ 0x8A, /* *** UNASSIGNED*/
/* 8B */ 0x8B, /* *** UNASSIGNED*/
/* 8C */ 0x8C, /* *** UNASSIGNED*/
/* 8D */ 0x8D, /* *** UNASSIGNED*/
/* 8E */ 0x8E, /* *** UNASSIGNED*/
/* 8F */ 0x8F, /* *** UNASSIGNED*/
/* 90 */ 0x90, /* *** UNASSIGNED*/
/* 91 */ 0x91, /* *** UNASSIGNED*/
/* 92 */ 0x92, /* *** UNASSIGNED*/
/* 93 */ 0x93, /* *** UNASSIGNED*/
/* 94 */ 0x94, /* *** UNASSIGNED*/
/* 95 */ 0x95, /* *** UNASSIGNED*/
/* 96 */ 0x96, /* *** UNASSIGNED*/
/* 97 */ 0x97, /* *** UNASSIGNED*/
/* 98 */ 0x98, /* *** UNASSIGNED*/
/* 99 */ 0x99, /* *** UNASSIGNED*/
/* 9A */ 0x9A, /* *** UNASSIGNED*/
/* 9B */ 0x9B, /* *** UNASSIGNED*/
/* 9C */ 0x9C, /* *** UNASSIGNED*/
/* 9D */ 0x9D, /* *** UNASSIGNED*/
/* 9E */ 0x9E, /* *** UNASSIGNED*/
/* 9F */ 0x9F, /* *** UNASSIGNED*/
/* A0 */ 0xA0, /*<U00A0> NO-BREAK SPACE*/
/* A1 */ 0xA1, /* *** UNASSIGNED*/
/* A2 */ 0xA2, /* *** UNASSIGNED*/
/* A3 */ 0xA3, /* *** UNASSIGNED*/
/* A4 */ 0xA4, /*<U00A4> CURRENCY SIGN*/
/* A5 */ 0xA5, /* *** UNASSIGNED*/
/* A6 */ 0xA6, /* *** UNASSIGNED*/
/* A7 */ 0xA7, /* *** UNASSIGNED*/
/* A8 */ 0xA8, /* *** UNASSIGNED*/
/* A9 */ 0xA9, /* *** UNASSIGNED*/
/* AA */ 0xAA, /* *** UNASSIGNED*/
/* AB */ 0xAB, /* *** UNASSIGNED*/
/* AC */ 0xAC, /*<U060C> ARABIC COMMA*/
/* AD */ 0xAD, /*<U00AD> SOFT HYPHEN*/
/* AE */ 0xAE, /* *** UNASSIGNED*/
/* AF */ 0xAF, /* *** UNASSIGNED*/
/* B0 */ 0xB0, /* *** UNASSIGNED*/
/* B1 */ 0xB1, /* *** UNASSIGNED*/
/* B2 */ 0xB2, /* *** UNASSIGNED*/
/* B3 */ 0xB3, /* *** UNASSIGNED*/
/* B4 */ 0xB4, /* *** UNASSIGNED*/
/* B5 */ 0xB5, /* *** UNASSIGNED*/
/* B6 */ 0xB6, /* *** UNASSIGNED*/
/* B7 */ 0xB7, /* *** UNASSIGNED*/
/* B8 */ 0xB8, /* *** UNASSIGNED*/
/* B9 */ 0xB9, /* *** UNASSIGNED*/
/* BA */ 0xBA, /* *** UNASSIGNED*/
/* BB */ 0xBB, /*<U061B> ARABIC SEMICOLON*/
/* BC */ 0xBC, /* *** UNASSIGNED*/
/* BD */ 0xBD, /* *** UNASSIGNED*/
/* BE */ 0xBE, /* *** UNASSIGNED*/
/* BF */ 0xBF, /*<U061F> ARABIC QUESTION MARK*/
/* C0 */ 0xC0, /* ***UNASSIGNED*/
/* C1 */ 0xC1, /*<UFE80> ARABIC LETTER HAMZA*/
/* C2 */ 0xC2, /*<UFE81> ARABIC LETTER ALEF WITH MADDA ABOVE*/
/* C3 */ 0xC3, /*<UFE83> ARABIC LETTER ALEF WITH HAMZA ABOVE*/
/* C4 */ 0xC4, /*<UFE85> ARABIC LETTER WAW WITH HAMZA ABOVE*/
/* C5 */ 0xC5, /*<UFE87> ARABIC LETTER ALEF WITH HAMZA BELOW*/
/* C6 */ 0xC6, /*<UFE89> ARABIC LETTER YEH WITH HAMZA ABOVE*/
/* C7 */ 0xC7, /*<UFE8D> ARABIC LETTER ALEF*/
/* C8 */ 0xC8, /*<UFE8F> ARABIC LETTER BEH*/
/* C9 */ 0xC9, /*<UFE93> ARABIC LETTER TEH MARBUTA*/
/* CA */ 0xCA, /*<UFE95> ARABIC LETTER TEH*/
/* CB */ 0xCB, /*<UFE99> ARABIC LETTER THEH*/
/* CC */ 0xCC, /*<UFE9D> ARABIC LETTER JEEM*/
/* CD */ 0xCD, /*<UFEA1> ARABIC LETTER HAH*/
/* CE */ 0xCE, /*<UFEA5> ARABIC LETTER KHAH*/
/* CF */ 0xCF, /*<UFEA9> ARABIC LETTER DAL*/
/* D0 */ 0xD0, /*<UFEAB> ARABIC LETTER THAL*/
/* D1 */ 0xD1, /*<UFEAD> ARABIC LETTER REH*/
/* D2 */ 0xD2, /*<UFEAF> ARABIC LETTER ZAIN*/
/* D3 */ 0xD3, /*<U0633> ARABIC LETTER SEEN*/
/* D4 */ 0xD4, /*<U0634> ARABIC LETTER SHEEN*/
/* D5 */ 0xD5, /*<U0635> ARABIC LETTER SAD*/
/* D6 */ 0xD6, /*<U0636> ARABIC LETTER DAD*/
/* D7 */ 0xD7, /*<UFEC1> ARABIC LETTER TAH*/
/* D8 */ 0xD8, /*<UFEC5> ARABIC LETTER ZAH*/
/* D9 */ 0xD9, /*<UFEC9> ARABIC LETTER AIN*/
/* DA */ 0xDA, /*<UFECD> ARABIC LETTER GHAIN*/
/* DB */ 0xDB, /* *** UNASSIGNED*/
/* DC */ 0xDC, /* *** UNASSIGNED*/
/* DD */ 0xDD, /* *** UNASSIGNED*/
/* DE */ 0xDE, /* *** UNASSIGNED*/
/* DF */ 0xDF, /* *** UNASSIGNED*/
/* E0 */ 0xE0, /*<U0640> ARABIC TATWEEL*/
/* E1 */ 0xE1, /*<UFED1> ARABIC LETTER FEH ISOLATED FORM*/
/* E2 */ 0xE2, /*<UFED5> ARABIC LETTER QAF ISOLATED FORM*/
/* E3 */ 0xE3, /*<UFED9> ARABIC LETTER KAF ISOLATED FORM*/
/* E4 */ 0xE4, /*<UFEDD> ARABIC LETTER LAM ISOLATED FORM*/
/* E5 */ 0xE5, /*<UFEE1> ARABIC LETTER MEEM ISOLATED FORM*/
/* E6 */ 0xE6, /*<UFEE5> ARABIC LETTER NOON ISOLATED FORM*/
/* E7 */ 0xE7, /*<UFEE9> ARABIC LETTER HEH ISOLATED FORM*/
/* E8 */ 0xE8, /*<UFEED> ARABIC LETTER WAW ISOLATED FORM*/
/* E9 */ 0xE9, /*<UFEEF> ARABIC LETTER ALEF MAKSURA ISOLATED FORM*/
/* EA */ 0xEA, /*<UFEF1> ARABIC LETTER YEH ISOLATED FORM*/
/* EB */ 0xEB, /*<U064B> ARABIC FATHATAN*/
/* EC */ 0xEC, /*<U064C> ARABIC DAMMATAN*/
/* ED */ 0xED, /*<U064D> ARABIC KASRATAN*/
/* EE */ 0xEE, /*<U064E> ARABIC FATHA*/
/* EF */ 0xEF, /*<U064F> ARABIC DAMMA*/
/* F0 */ 0xF0, /*<U0650> ARABIC KASRA*/
/* F1 */ 0xF1, /*<U0651> ARABIC SHADDA*/
/* F2 */ 0xF2, /*<U0652> ARABIC SUKUN*/
/* F3 */ 0xF3, /* *** UNASSIGNED*/
/* F4 */ 0xF4, /* *** UNASSIGNED*/
/* F5 */ 0xF5, /* *** UNASSIGNED*/
/* F6 */ 0xF6, /* *** UNASSIGNED*/
/* F7 */ 0xF7, /* *** UNASSIGNED*/
/* F8 */ 0xF8, /* *** UNASSIGNED*/
/* F9 */ 0xF9, /* *** UNASSIGNED*/
/* FA */ 0xFA, /* *** UNASSIGNED*/
/* FB */ 0xFB, /* *** UNASSIGNED*/
/* FC */ 0xFC, /* *** UNASSIGNED*/
/* FD */ 0xFD, /* *** UNASSIGNED*/
/* FE */ 0xFE, /* *** UNASSIGNED*/
/* FF */ 0xFF  /* *** UNASSIGNED*/
  },
  /* 1256 Windows Arabic*/
{
/* 00 */ 0x00,  /*(CC) Null*/
/* 01 */ 0x01,  /*(CC) Start of Heading*/
/* 02 */ 0x02,  /*(CC) Start of Text*/
/* 03 */ 0x03,  /*(CC) End of Text*/
/* 04 */ 0x04,  /*(CC) End of Transmission*/
/* 05 */ 0x05,  /*(CC) Enquiry*/
/* 06 */ 0x06,  /*(CC) Acknowledge*/
/* 07 */ 0x07,  /*(CC) Bell*/
/* 08 */ 0x08,  /*(CC) Backspace*/
/* 09 */ 0x09,  /*(CC) Horizontal Tabulation*/
/* 0A */ 0x0A,  /*(CC) Line Feed*/
/* 0B */ 0x0B,  /*(CC) Vertical Tabulation*/
/* 0C */ 0x0C,  /*(CC) Form Feed*/
/* 0D */ 0x0D,  /*(CC) Carriage Return*/
/* 0E */ 0x0E,  /*(CC) Locking Shift One (Shift Out*/
/* 0F */ 0x0F,  /*(CC) Locking Shift Zero (Shift In*/
/* 10 */ 0x10,  /*(CC) Data Link Escape*/
/* 11 */ 0x11,  /*(CC) Device Control One*/
/* 12 */ 0x12,  /*(CC) Device Control Two*/
/* 13 */ 0x13,  /*(CC) Device Control Three*/
/* 14 */ 0x14,  /*(CC) Device Control Four*/
/* 15 */ 0x15,  /*(CC) Negative Acknowledge*/
/* 16 */ 0x16,  /*(CC) Synchronous Idle*/
/* 17 */ 0x17,  /*(CC) End of Transmission Block*/
/* 18 */ 0x18,  /*(CC) Cancel*/
/* 19 */ 0x19,  /*(CC) End of Medium*/
/* 1A */ 0x1A,  /*(CC) Substitute*/
/* 1B */ 0x1B,  /*(CC) Escape*/
/* 1C */ 0x1C,  /*(CC) Information File Separator*/
/* 1D */ 0x1D,  /*(CC) Group Separator*/
/* 1E */ 0x1E,  /*(CC) Record Separator*/
/* 1F */ 0x1F,  /*(CC) Unit Separator*/
/* 20 */ 0x20,  /*SPACE*/
/* 21 */ 0x21,  /*EXCLAMATION MARK*/
/* 22 */ 0x22,  /*QUOTATION MARK*/
/* 23 */ 0x23,  /*NUMBER SIGN*/
/* 24 */ 0x24,  /*DOLLAR SIGN*/
/* 25 */ 0x25,  /*PERCENT SIGN*/
/* 26 */ 0x26,  /*AMPERSAND*/
/* 27 */ 0x27,  /*APOSTROPHE*/
/* 28 */ 0x29,  /*LEFT PARENTHESIS*/
/* 29 */ 0x28,  /*RIGHT PARENTHESIS*/
/* 2A */ 0x2A,  /*ASTERISK*/
/* 2B */ 0x2B,  /*PLUS SIGN*/
/* 2C */ 0x2C,  /*COMMA*/
/* 2D */ 0x2D,  /*HYPHEN-MINUS*/
/* 2E */ 0x2E,  /*FULL STOP*/
/* 2F */ 0x2F,  /*SOLIDUS*/
/* 30 */ 0x30,  /*DIGIT ZERO*/
/* 31 */ 0x31,  /*DIGIT ONE*/
/* 32 */ 0x32,  /*DIGIT TWO*/
/* 33 */ 0x33,  /*DIGIT THREE*/
/* 34 */ 0x34,  /*DIGIT FOUR*/
/* 35 */ 0x35,  /*DIGIT FIVE*/
/* 36 */ 0x36,  /*DIGIT SIX*/
/* 37 */ 0x37,  /*DIGIT SEVEN*/
/* 38 */ 0x38,  /*DIGIT EIGHT*/
/* 39 */ 0x39,  /*DIGIT NINE*/
/* 3A */ 0x3A,  /*COLON*/
/* 3B */ 0x3B,  /*SEMICOLON*/
/* 3C */ 0x3E,  /*LESS-THAN SIGN*/
/* 3D */ 0x3D,  /*EQUALS SIGN*/
/* 3E */ 0x3C,  /*GREATER-THAN SIGN*/
/* 3F */ 0x3F,  /*QUESTION MARK*/
/* 40 */ 0x40,  /*COMMERCIAL AT*/
/* 41 */ 0x41,  /*LATIN CAPITAL LETTER A*/
/* 42 */ 0x42,  /*LATIN CAPITAL LETTER B*/
/* 43 */ 0x43,  /*LATIN CAPITAL LETTER C*/
/* 44 */ 0x44,  /*LATIN CAPITAL LETTER D*/
/* 45 */ 0x45,  /*LATIN CAPITAL LETTER E*/
/* 46 */ 0x46,  /*LATIN CAPITAL LETTER F*/
/* 47 */ 0x47,  /*LATIN CAPITAL LETTER G*/
/* 48 */ 0x48,  /*LATIN CAPITAL LETTER H*/
/* 49 */ 0x49,  /*LATIN CAPITAL LETTER I*/
/* 4A */ 0x4A,  /*LATIN CAPITAL LETTER J*/
/* 4B */ 0x4B,  /*LATIN CAPITAL LETTER K*/
/* 4C */ 0x4C,  /*LATIN CAPITAL LETTER L*/
/* 4D */ 0x4D,  /*LATIN CAPITAL LETTER M*/
/* 4E */ 0x4E,  /*LATIN CAPITAL LETTER N*/
/* 4F */ 0x4F,  /*LATIN CAPITAL LETTER O*/
/* 50 */ 0x50,  /*LATIN CAPITAL LETTER P*/
/* 51 */ 0x51,  /*LATIN CAPITAL LETTER Q*/
/* 52 */ 0x52,  /*LATIN CAPITAL LETTER R*/
/* 53 */ 0x53,  /*LATIN CAPITAL LETTER S*/
/* 54 */ 0x54,  /*LATIN CAPITAL LETTER T*/
/* 55 */ 0x55,  /*LATIN CAPITAL LETTER U*/
/* 56 */ 0x56,  /*LATIN CAPITAL LETTER V*/
/* 57 */ 0x57,  /*LATIN CAPITAL LETTER W*/
/* 58 */ 0x58,  /*LATIN CAPITAL LETTER X*/
/* 59 */ 0x59,  /*LATIN CAPITAL LETTER Y*/
/* 5A */ 0x5A,  /*LATIN CAPITAL LETTER Z*/
/* 5B */ 0x5D,  /*LEFT SQUARE BRACKET*/
/* 5C */ 0x5C,  /*REVERSE SOLIDUS*/
/* 5D */ 0x5B,  /*RIGHT SQUARE BRACKET*/
/* 5E */ 0x5E,  /*CIRCUMFLEX ACCENT*/
/* 5F */ 0x5F,  /*LOW LINE*/
/* 60 */ 0x60,  /*GRAVE ACCENT*/
/* 61 */ 0x61,  /*LATIN SMALL LETTER A*/
/* 62 */ 0x62,  /*LATIN SMALL LETTER B*/
/* 63 */ 0x63,  /*LATIN SMALL LETTER C*/
/* 64 */ 0x64,  /*LATIN SMALL LETTER D*/
/* 65 */ 0x65,  /*LATIN SMALL LETTER E*/
/* 66 */ 0x66,  /*LATIN SMALL LETTER F*/
/* 67 */ 0x67,  /*LATIN SMALL LETTER G*/
/* 68 */ 0x68,  /*LATIN SMALL LETTER H*/
/* 69 */ 0x69,  /*LATIN SMALL LETTER I*/
/* 6A */ 0x6A,  /*LATIN SMALL LETTER J*/
/* 6B */ 0x6B,  /*LATIN SMALL LETTER K*/
/* 6C */ 0x6C,  /*LATIN SMALL LETTER L*/
/* 6D */ 0x6D,  /*LATIN SMALL LETTER M*/
/* 6E */ 0x6E,  /*LATIN SMALL LETTER N*/
/* 6F */ 0x6F,  /*LATIN SMALL LETTER O*/
/* 70 */ 0x70,  /*LATIN SMALL LETTER P*/
/* 71 */ 0x71,  /*LATIN SMALL LETTER Q*/
/* 72 */ 0x72,  /*LATIN SMALL LETTER R*/
/* 73 */ 0x73,  /*LATIN SMALL LETTER S*/
/* 74 */ 0x74,  /*LATIN SMALL LETTER T*/
/* 75 */ 0x75,  /*LATIN SMALL LETTER U*/
/* 76 */ 0x76,  /*LATIN SMALL LETTER V*/
/* 77 */ 0x77,  /*LATIN SMALL LETTER W*/
/* 78 */ 0x78,  /*LATIN SMALL LETTER X*/
/* 79 */ 0x79,  /*LATIN SMALL LETTER Y*/
/* 7A */ 0x7A,  /*LATIN SMALL LETTER Z*/
/* 7B */ 0x7D,  /*LEFT CURLY BRACKET*/
/* 7C */ 0x7C,  /*VERTICAL LINE*/
/* 7D */ 0x7B,  /*RIGHT CURLY BRACKET*/
/* 7E */ 0x7E,  /*TILDE*/
/* 7F */ 0x7F,  /*(CC) Delete*/
/* 80 */ 0x80,  /* *** UNASSIGNED*/
/* 81 */ 0x81,  /*ARABIC LETTER PEH*/
/* 82 */ 0x82,  /*SINGLE LOW-9 QUOTATION MARK*/
/* 83 */ 0x83,  /*LATIN SMALL LETTER F WITH HOOK*/
/* 84 */ 0x84,  /*DOUBLE LOW-9 QUOTATION MARK*/
/* 85 */ 0x85,  /*HORIZONTAL ELLIPSIS*/
/* 86 */ 0x86,  /*DAGGER*/
/* 87 */ 0x87,  /*DOUBLE DAGGER*/
/* 88 */ 0x88,  /*MODIFIER LETTER CIRCUMFLEX ACCENT*/
/* 89 */ 0x89,  /*PER MILLE SIGN*/
/* 8A */ 0x8A,  /*(CC) Line Tabultion Set*/
/* 8B */ 0x8B,  /*SINGLE LEFT-POINTING ANGLE QUOTATION MARK*/
/* 8C */ 0x8C,  /*LATIN CAPITAL LIGATURE OE*/
/* 8D */ 0x8D,  /*ARABIC LETTER TCHEH*/
/* 8E */ 0x8E,  /*ARABIC LETTER JEH*/
/* 8F */ 0x8F,  /*(CC) Single Shift Three*/
/* 90 */ 0x90,  /*ARABIC LETTER GAF*/
/* 91 */ 0x91,  /*LEFT SINGLE QUOTATION MARK*/
/* 92 */ 0x92,  /*RIGHT SINGLE QUOTATION MARK*/
/* 93 */ 0x93,  /*LEFT DOUBLE QUOTATION MARK*/
/* 94 */ 0x94,  /*RIGHT DOUBLE QUOTATION MARK*/
/* 95 */ 0x95,  /*BULLET*/
/* 96 */ 0x96,  /*EN DASH*/
/* 97 */ 0x97,  /*EM DASH*/
/* 98 */ 0x98,  /*(CC) Start of String*/
/* 99 */ 0x99,  /*TRADE MARK SIGN*/
/* 9A */ 0x9A,  /*(CC) Single Character Introducer*/
/* 9B */ 0x9B,  /*SINGLE RIGHT-POINTING ANGLE QUOTATION MARK*/
/* 9C */ 0x9C,  /*LATIN SMALL LIGATURE OE*/
/* 9D */ 0x9D,  /*ZERO WIDTH NON-JOINER*/
/* 9E */ 0x9E,  /*ZERO WIDTH JOINER*/
/* 9F */ 0x9F,  /*(CC) Application Program Command*/
/* A0 */ 0xA0,  /*NO-BREAK SPACE*/
/* A1 */ 0xA1,  /*ARABIC COMMA*/
/* A2 */ 0xA2,  /*CENT SIGN*/
/* A3 */ 0xA3,  /*POUND SIGN*/
/* A4 */ 0xA4,  /*CURRENCY SIGN*/
/* A5 */ 0xA5,  /*YEN SIGN*/
/* A6 */ 0xA6,  /*BROKEN BAR*/
/* A7 */ 0xA7,  /*SECTION SIGN*/
/* A8 */ 0xA8,  /*DIAERESIS*/
/* A9 */ 0xA9,  /*COPYRIGHT SIGN*/
/* AA */ 0xAA,  /* ***UNASSIGNED*/
/* AB */ 0xBB,  /*LEFT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* AC */ 0xAC,  /*NOT SIGN*/
/* AD */ 0xAD,  /*SOFT HYPHEN*/
/* AE */ 0xAE,  /*REGISTERED SIGN*/
/* AF */ 0xAF,  /*MACRON*/
/* B0 */ 0xB0,  /*DEGREE SIGN*/
/* B1 */ 0xB1,  /*PLUS-MINUS SIGN*/
/* B2 */ 0xB2,  /*SUPERSCRIPT TWO*/
/* B3 */ 0xB3,  /*SUPERSCRIPT THREE*/
/* B4 */ 0xB4,  /*ACUTE ACCENT*/
/* B5 */ 0xB5,  /*MICRO SIGN*/
/* B6 */ 0xB6,  /*PILCROW SIGN*/
/* B7 */ 0xB7,  /*MIDDLE DOT*/
/* B8 */ 0xB8,  /*CEDILLA*/
/* B9 */ 0xB9,  /*SUPERSCRIPT ONE*/
/* BA */ 0xBA,  /*ARABIC SEMICOLON*/
/* BB */ 0xAB,  /*RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* BC */ 0xBC,  /*VULGAR FRACTION ONE QUARTER*/
/* BD */ 0xBD,  /*VULGAR FRACTION ONE HALF*/
/* BE */ 0xBE,  /*VULGAR FRACTION THREE QUARTERS*/
/* BF */ 0xBF,  /*ARABIC QUESTION MARK*/
/* C0 */ 0xC0,  /* ***UNASSIGNED*/
/* C1 */ 0xC1,  /*ARABIC LETTER HAMZA*/
/* C2 */ 0xC2,  /*ARABIC LETTER ALEF WITH MADDA ABOVE*/
/* C3 */ 0xC3,  /*ARABIC LETTER ALEF WITH HAMZA ABOVE*/
/* C4 */ 0xC4,  /*ARABIC LETTER WAW WITH HAMZA ABOVE*/
/* C5 */ 0xC5,  /*ARABIC LETTER ALEF WITH HAMZA BELOW*/
/* C6 */ 0xC6,  /*ARABIC LETTER YEH WITH HAMZA ABOVE*/
/* C7 */ 0xC7,  /*ARABIC LETTER ALEF*/
/* C8 */ 0xC8,  /*ARABIC LETTER BEH*/
/* C9 */ 0xC9,  /*ARABIC LETTER TEH MARBUTA*/
/* CA */ 0xCA,  /*ARABIC LETTER TEH*/
/* CB */ 0xCB,  /*ARABIC LETTER THEH*/
/* CC */ 0xCC,  /*ARABIC LETTER JEEM*/
/* CD */ 0xCD,  /*ARABIC LETTER HAH*/
/* CE */ 0xCE,  /*ARABIC LETTER KHAH*/
/* CF */ 0xCF,  /*ARABIC LETTER DAL*/
/* D0 */ 0xD0,  /*ARABIC LETTER THAL*/
/* D1 */ 0xD1,  /*ARABIC LETTER REH*/
/* D2 */ 0xD2,  /*ARABIC LETTER ZAIN*/
/* D3 */ 0xD3,  /*ARABIC LETTER SEEN*/
/* D4 */ 0xD4,  /*ARABIC LETTER SHEEN*/
/* D5 */ 0xD5,  /*ARABIC LETTER SAD*/
/* D6 */ 0xD6,  /*ARABIC LETTER DAD*/
/* D7 */ 0xD7,  /*MULTIPLICATION SIGN*/
/* D8 */ 0xD8,  /*ARABIC LETTER TAH*/
/* D9 */ 0xD9,  /*ARABIC LETTER ZAH*/
/* DA */ 0xDA,  /*ARABIC LETTER AIN*/
/* DB */ 0xDB,  /*ARABIC LETTER GHAIN*/
/* DC */ 0xDC,  /*ARABIC TATWEEL*/
/* DD */ 0xDD,  /*ARABIC LETTER FEH*/
/* DE */ 0xDE,  /*ARABIC LETTER QAF*/
/* DF */ 0xDF,  /*ARABIC LETTER KAF*/
/* E0 */ 0xE0,  /*LATIN SMALL LETTER A WITH GRAVE*/
/* E1 */ 0xE1,  /*ARABIC LETTER LAM*/
/* E2 */ 0xE2,  /*LATIN SMALL LETTER A WITH CIRCUMFLEX*/
/* E3 */ 0xE3,  /*ARABIC LETTER MEEM*/
/* E4 */ 0xE4,  /*ARABIC LETTER NOON*/
/* E5 */ 0xE5,  /*ARABIC LETTER HEH*/
/* E6 */ 0xE6,  /*ARABIC LETTER WAW*/
/* E7 */ 0xE7,  /*LATIN SMALL LETTER C WITH CEDILLA*/
/* E8 */ 0xE8,  /*LATIN SMALL LETTER E WITH GRAVE*/
/* E9 */ 0xE9,  /*LATIN SMALL LETTER E WITH ACUTE*/
/* EA */ 0xEA,  /*LATIN SMALL LETTER E WITH CIRCUMFLEX*/
/* EB */ 0xEB,  /*LATIN SMALL LETTER E WITH DIAERESIS*/
/* EC */ 0xEC,  /*ARABIC LETTER ALEF MAKSURA*/
/* ED */ 0xED,  /*ARABIC LETTER YEH*/
/* EE */ 0xEE,  /*LATIN SMALL LETTER I WITH CIRCUMFLEX*/
/* EF */ 0xEF,  /*LATIN SMALL LETTER I WITH DIAERESIS*/
/* F0 */ 0xF0,  /*ARABIC FATHATAN*/
/* F1 */ 0xF1,  /*ARABIC DAMMATAN*/
/* F2 */ 0xF2,  /*ARABIC KASRATAN*/
/* F3 */ 0xF3,  /*ARABIC FATHA*/
/* F4 */ 0xF4,  /*LATIN SMALL LETTER O WITH CIRCUMFLEX*/
/* F5 */ 0xF5,  /*ARABIC DAMMA*/
/* F6 */ 0xF6,  /*ARABIC KASRA*/
/* F7 */ 0xF7,  /*DIVISION SIGN*/
/* F8 */ 0xF8,  /*ARABIC SHADDA*/
/* F9 */ 0xF9,  /*LATIN SMALL LETTER U WITH GRAVE*/
/* FA */ 0xFA,  /*ARABIC SUKUN*/
/* FB */ 0xFB,  /*LATIN SMALL LETTER U WITH CIRCUMFLEX*/
/* FC */ 0xFC,  /*LATIN SMALL LETTER U WITH DIAERESIS*/
/* FD */ 0xFD,  /*LEFT-TO-RIGHT MARK*/
/* FE */ 0xFE,  /*RIGHT-TO-LEFT MARK*/
/* FF */ 0xFF
},
  /* 424 MF/AS 400 Hebrew*/
  {
/* 00 */ 0x00,  /*<U0000>       NULL*/
/* 01 */ 0x01,  /*<U0001>       START OF HEADING*/
/* 02 */ 0x02,  /*<U0002>       START OF TEXT*/
/* 03 */ 0x03,  /*<U0003>       END OF TEXT*/
/* 04 */ 0x04,  /*<U009C> <U009C>*/
/* 05 */ 0x05,  /*<U0009>       HORIZONTAL TABULATION*/
/* 06 */ 0x06,  /*<U0086> <U0086>*/
/* 07 */ 0x07,  /*<U007F>       DELETE*/
/* 08 */ 0x08,  /*<U0097> <U0097>*/
/* 09 */ 0x09,  /*<U008D> <U008D>*/
/* 0A */ 0x0A,  /*<U008E> <U008E>*/
/* 0B */ 0x0B,  /*<U000B>       VERTICAL TABULATION*/
/* 0C */ 0x0C,  /*<U000C>       FORM FEED*/
/* 0D */ 0x0D,  /*<U000D>       CARRIAGE RETURN*/
/* 0E */ 0x0E,  /*<U000E>       SHIFT OUT*/
/* 0F */ 0x0F,  /*<U000F>       SHIFT IN*/
/* 10 */ 0x10,  /*<U0010>       DATA LINK ESCAPE*/
/* 11 */ 0x11,  /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */ 0x12,  /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */ 0x13,  /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */ 0x14,  /*<U009D> <U009D>*/
/* 15 */ 0x15,  /*<U0085> <U0085>*/
/* 16 */ 0x16,  /*<U0008>       BACKSPACE*/
/* 17 */ 0x17,  /*<U0087> <U0087>*/
/* 18 */ 0x18,  /*<U0018>       CANCEL*/
/* 19 */ 0x19,  /*<U0019>       END OF MEDIUM*/
/* 1A */ 0x1A,  /*<U0092> <U0092>*/
/* 1B */ 0x1B,  /*<U008F> <U008F>*/
/* 1C */ 0x1C,  /*<U001C>       FILE SEPARATOR*/
/* 1D */ 0x1D,  /*<U001D>       GROUP SEPARATOR*/
/* 1E */ 0x1E,  /*<U001E>       RECORD SEPARATOR*/
/* 1F */ 0x1F,  /*<U001F>       UNIT SEPARATOR*/
/* 20 */ 0x20,  /*<U0080> <U0080>*/
/* 21 */ 0x21,  /*<U0081> <U0081>*/
/* 22 */ 0x22,  /*<U0082> <U0082>*/
/* 23 */ 0x23,  /*<U0083> <U0083>*/
/* 24 */ 0x24,  /*<U0084> <U0084>*/
/* 25 */ 0x25,  /*<U000A>       LINE FEED*/
/* 26 */ 0x26,  /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 27 */ 0x27,  /*<U001B>       ESCAPE*/
/* 28 */ 0x28,  /*<U0088> <U0088>*/
/* 29 */ 0x29,  /*<U0089> <U0089>*/
/* 2A */ 0x2A,  /*<U008A> <U008A>*/
/* 2B */ 0x2B,  /*<U008B> <U008B>*/
/* 2C */ 0x2C,  /*<U008C> <U008C>*/
/* 2D */ 0x2D,  /*<U0005>       ENQUIRY*/
/* 2E */ 0x2E,  /*<U0006>       ACKNOWLEDGE*/
/* 2F */ 0x2F,  /*<U0007>       BELL*/
/* 30 */ 0x30,  /*<U0090> <U0090>*/
/* 31 */ 0x31,  /*<U0091> <U0091>*/
/* 32 */ 0x32,  /*<U0016>       SYNCHRONOUS IDLE*/
/* 33 */ 0x33,  /*<U0093> <U0093>*/
/* 34 */ 0x34,  /*<U0094> <U0094>*/
/* 35 */ 0x35,  /*<U0095> <U0095>*/
/* 36 */ 0x36,  /*<U0096> <U0096>*/
/* 37 */ 0x37,  /*<U0004>       END OF TRANSMISSION*/
/* 38 */ 0x38,  /*<U0098> <U0098>*/
/* 39 */ 0x39,  /*<U0099> <U0099>*/
/* 3A */ 0x3A,  /*<U009A> <U009A>*/
/* 3B */ 0x3B,  /*<U009B> <U009B>*/
/* 3C */ 0x3C,  /*<U0014>       DEVICE CONTROL FOUR*/
/* 3D */ 0x3D,  /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 3E */ 0x3E,  /*<U009E> <U009E>*/
/* 3F */ 0x3F,  /*<U001A>       SUBSTITUTE*/
/* 40 */ 0x40,  /*<U0020>       SPACE*/
/* 41 */ 0x41,  /*<U05D0>       HEBREW LETTER ALEF*/
/* 42 */ 0x42,  /*<U05D1>       HEBREW LETTER BET*/
/* 43 */ 0x43,  /*<U05D2>       HEBREW LETTER GIMEL*/
/* 44 */ 0x44,  /*<U05D3>       HEBREW LETTER DALET*/
/* 45 */ 0x45,  /*<U05D4>       HEBREW LETTER HE*/
/* 46 */ 0x46,  /*<U05D5>       HEBREW LETTER VAV*/
/* 47 */ 0x47,  /*<U05D6>       HEBREW LETTER ZAYIN*/
/* 48 */ 0x48,  /*<U05D7>       HEBREW LETTER HET*/
/* 49 */ 0x49,  /*<U05D8>       HEBREW LETTER TET*/
/* 4A */ 0x4A,  /*<U00A2>       CENT SIGN*/
/* 4B */ 0x4B,  /*<U002E>       PERIOD*/
/* 4C */ 0x6E,  /*<U003C>       LESS-THAN SIGN*/
/* 4D */ 0x5D,  /*<U0028>       OPENING PARENTHESIS*/
/* 4E */ 0x4E,  /*<U002B>       PLUS SIGN*/
/* 4F */ 0x4F,  /*<U007C>       VERTICAL BAR*/
/* 50 */ 0x50,  /*<U0026>       AMPERSAND*/
/* 51 */ 0x51,  /*<U05D9>       HEBREW LETTER YOD*/
/* 52 */ 0x52,  /*<U05DA>       HEBREW LETTER FINAL KAF*/
/* 53 */ 0x53,  /*<U05DB>       HEBREW LETTER KAF*/
/* 54 */ 0x54,  /*<U05DC>       HEBREW LETTER LAMED*/
/* 55 */ 0x55,  /*<U05DD>       HEBREW LETTER FINAL MEM*/
/* 56 */ 0x56,  /*<U05DE>       HEBREW LETTER MEM*/
/* 57 */ 0x57,  /*<U05DF>       HEBREW LETTER FINAL NUN*/
/* 58 */ 0x58,  /*<U05E0>       HEBREW LETTER NUN*/
/* 59 */ 0x59,  /*<U05E1>       HEBREW LETTER SAMEKH*/
/* 5A */ 0x5A,  /*<U0021>       EXCLAMATION MARK*/
/* 5B */ 0x5B,  /*<U0024>       DOLLAR SIGN*/
/* 5C */ 0x5C,  /*<U002A>       ASTERISK*/
/* 5D */ 0x4D,  /*<U0029>       CLOSING PARENTHESIS*/
/* 5E */ 0x5E,  /*<U003B>       SEMICOLON*/
/* 5F */ 0x5F,  /*<U00AC>       NOT SIGN*/
/* 60 */ 0x60,  /*<U002D>       HYPHEN-MINUS*/
/* 61 */ 0x61,  /*<U002F>       SLASH*/
/* 62 */ 0x62,  /*<U05E2>       HEBREW LETTER AYIN*/
/* 63 */ 0x63,  /*<U05E3>       HEBREW LETTER FINAL PE*/
/* 64 */ 0x64,  /*<U05E4>       HEBREW LETTER PE*/
/* 65 */ 0x65,  /*<U05E5>       HEBREW LETTER FINAL TSADI*/
/* 66 */ 0x66,  /*<U05E6>       HEBREW LETTER TSADI*/
/* 67 */ 0x67,  /*<U05E7>       HEBREW LETTER QOF*/
/* 68 */ 0x68,  /*<U05E8>       HEBREW LETTER RESH*/
/* 69 */ 0x69,  /*<U05E9>       HEBREW LETTER SHIN*/
/* 6A */ 0x6A,  /*<U00A6>       BROKEN VERTICAL BAR*/
/* 6B */ 0x6B,  /*<U002C>       COMMA*/
/* 6C */ 0x6C,  /*<U0025>       PERCENT SIGN*/
/* 6D */ 0x6D,  /*<U005F>       SPACING UNDERSCORE*/
/* 6E */ 0x4C,  /*<U003E>       GREATER-THAN SIGN*/
/* 6F */ 0x6F,  /*<U003F>       QUESTION MARK*/
/* 70 */ 0x70,  /* **UNASSIGNED*/
/* 71 */ 0x71,  /*<U05EA>       HEBREW LETTER TAV*/
/* 72 */ 0x72,  /* **UNASSIGNED*/
/* 73 */ 0x73,  /* **UNASSIGNED*/
/* 74 */ 0x74,  /*<U00A0>       NON-BREAKING SPACE*/
/* 75 */ 0x75,  /* **UNASSIGNED*/
/* 76 */ 0x76,  /* **UNASSIGNED*/
/* 77 */ 0x77,  /* **UNASSIGNED*/
/* 78 */ 0x78,  /*<U2017>       SPACING DOUBLE UNDERSCORE*/
/* 79 */ 0x79,  /*<U0060>       SPACING GRAVE*/
/* 7A */ 0x7A,  /*<U003A>       COLON*/
/* 7B */ 0x7B,  /*<U0023>       NUMBER SIGN*/
/* 7C */ 0x7C,  /*<U0040>       COMMERCIAL AT*/
/* 7D */ 0x7D,  /*<U0027>       APOSTROPHE-QUOTE*/
/* 7E */ 0x7E,  /*<U003D>       EQUALS SIGN*/
/* 7F */ 0x7F,  /*<U0022>       QUOTATION MARK*/
/* 80 */ 0x80,  /* ***UNASSIGNED*/
/* 81 */ 0x81,  /*<U0061>       LATIN SMALL LETTER A*/
/* 82 */ 0x82,  /*<U0062>       LATIN SMALL LETTER B*/
/* 83 */ 0x83,  /*<U0063>       LATIN SMALL LETTER C*/
/* 84 */ 0x84,  /*<U0064>       LATIN SMALL LETTER D*/
/* 85 */ 0x85,  /*<U0065>       LATIN SMALL LETTER E*/
/* 86 */ 0x86,  /*<U0066>       LATIN SMALL LETTER F*/
/* 87 */ 0x87,  /*<U0067>       LATIN SMALL LETTER G*/
/* 88 */ 0x88,  /*<U0068>       LATIN SMALL LETTER H*/
/* 89 */ 0x89,  /*<U0069>       LATIN SMALL LETTER I*/
/* 8A */ 0x8B,  /*<U00AB>       LEFT POINTING GUILLEMET*/
/* 8B */ 0x8A,  /*<U00BB>       RIGHT POINTING GUILLEMET*/
/* 8C */ 0x8C,  /* **UNASSIGNED*/
/* 8D */ 0x8D,  /* **UNASSIGNED*/
/* 8E */ 0x8E,  /* **UNASSIGNED*/
/* 8F */ 0x8F,  /*<U00B1>       PLUS-OR-MINUS SIGN*/
/* 90 */ 0x90,  /*<U00B0>       DEGREE SIGN*/
/* 91 */ 0x91,  /*<U006A>       LATIN SMALL LETTER J*/
/* 92 */ 0x92,  /*<U006B>       LATIN SMALL LETTER K*/
/* 93 */ 0x93,  /*<U006C>       LATIN SMALL LETTER L*/
/* 94 */ 0x94,  /*<U006D>       LATIN SMALL LETTER M*/
/* 95 */ 0x95,  /*<U006E>       LATIN SMALL LETTER N*/
/* 96 */ 0x96,  /*<U006F>       LATIN SMALL LETTER O*/
/* 97 */ 0x97,  /*<U0070>       LATIN SMALL LETTER P*/
/* 98 */ 0x98,  /*<U0071>       LATIN SMALL LETTER Q*/
/* 99 */ 0x99,  /*<U0072>       LATIN SMALL LETTER R*/
/* 9A */ 0x9A,  /* **UNASSIGNED*/
/* 9B */ 0x9B,  /* **UNASSIGNED*/
/* 9C */ 0x9C,  /* **UNASSIGNED*/
/* 9D */ 0x9D,  /*<U00B8>       SPACING CEDILLA*/
/* 9E */ 0x9E,  /* **UNASSIGNED*/
/* 9F */ 0x9F,  /*<U00A4>       CURRENCY SIGN*/
/* A0 */ 0xA0,  /*<U00B5>       MICRO SIGN*/
/* A1 */ 0xA1,  /*<U007E>       TILDE*/
/* A2 */ 0xA2,  /*<U0073>       LATIN SMALL LETTER S*/
/* A3 */ 0xA3,  /*<U0074>       LATIN SMALL LETTER T*/
/* A4 */ 0xA4,  /*<U0075>       LATIN SMALL LETTER U*/
/* A5 */ 0xA5,  /*<U0076>       LATIN SMALL LETTER V*/
/* A6 */ 0xA6,  /*<U0077>       LATIN SMALL LETTER W*/
/* A7 */ 0xA7,  /*<U0078>       LATIN SMALL LETTER X*/
/* A8 */ 0xA8,  /*<U0079>       LATIN SMALL LETTER Y*/
/* A9 */ 0xA9,  /*<U007A>       LATIN SMALL LETTER Z*/
/* AA */ 0xAA,  /* **UNASSIGNED*/
/* AB */ 0xAB,  /* **UNASSIGNED*/
/* AC */ 0xAC,  /* **UNASSIGNED*/
/* AD */ 0xAD,  /* **UNASSIGNED*/
/* AE */ 0xAE,  /* **UNASSIGNED*/
/* AF */ 0xAF,  /*<U00AE>       REGISTERED TRADE MARK SIGN*/
/* B0 */ 0xB0,  /*<U005E>       SPACING CIRCUMFLEX*/
/* B1 */ 0xB1,  /*<U00A3>       POUND SIGN*/
/* B2 */ 0xB2,  /*<U00A5>       YEN SIGN*/
/* B3 */ 0xB3,  /*<U2022>       BULLET*/
/* B4 */ 0xB4,  /*<U00A9>       COPYRIGHT SIGN*/
/* B5 */ 0xB5,  /*<U00A7>       SECTION SIGN*/
/* B6 */ 0xB6,  /*<U00B6>       PARAGRAPH SIGN*/
/* B7 */ 0xB7,  /*<U00BC>       FRACTION ONE QUARTER*/
/* B8 */ 0xB8,  /*<U00BD>       FRACTION ONE HALF*/
/* B9 */ 0xB9,  /*<U00BE>       FRACTION THREE QUARTERS*/
/* BA */ 0xBB,  /*<U005B>       OPENING SQUARE BRACKET*/
/* BB */ 0xBA,  /*<U005D>       CLOSING SQUARE BRACKET*/
/* BC */ 0xBC,  /*<U00AF>       SPACING MACRON*/
/* BD */ 0xBD,  /*<U00A8>       SPACING DIAERESIS*/
/* BE */ 0xBE,  /*<U00B4>       SPACING ACUTE*/
/* BF */ 0xBF,  /*<U00D7>       MULTIPLICATION SIGN*/
/* C0 */ 0xD0,  /*<U007B>       OPENING CURLY BRACKET*/
/* C1 */ 0xC1,  /*<U0041>       LATIN CAPITAL LETTER A*/
/* C2 */ 0xC2,  /*<U0042>       LATIN CAPITAL LETTER B*/
/* C3 */ 0xC3,  /*<U0043>       LATIN CAPITAL LETTER C*/
/* C4 */ 0xC4,  /*<U0044>       LATIN CAPITAL LETTER D*/
/* C5 */ 0xC5,  /*<U0045>       LATIN CAPITAL LETTER E*/
/* C6 */ 0xC6,  /*<U0046>       LATIN CAPITAL LETTER F*/
/* C7 */ 0xC7,  /*<U0047>       LATIN CAPITAL LETTER G*/
/* C8 */ 0xC8,  /*<U0048>       LATIN CAPITAL LETTER H*/
/* C9 */ 0xC9,  /*<U0049>       LATIN CAPITAL LETTER I*/
/* CA */ 0xCA,  /*<U00AD>       SOFT HYPHEN*/
/* CB */ 0xCB,  /* *** UNASSIGNED*/
/* CC */ 0xCC,  /* *** UNASSIGNED*/
/* CD */ 0xCD,  /* *** UNASSIGNED*/
/* CE */ 0xCE,  /* *** UNASSIGNED*/
/* CF */ 0xCF,  /* *** UNASSIGNED*/
/* D0 */ 0xC0,  /*<U007D>       CLOSING CURLY BRACKET*/
/* D1 */ 0xD1,  /*<U004A>       LATIN CAPITAL LETTER J*/
/* D2 */ 0xD2,  /*<U004B>       LATIN CAPITAL LETTER K*/
/* D3 */ 0xD3,  /*<U004C>       LATIN CAPITAL LETTER L*/
/* D4 */ 0xD4,  /*<U004D>       LATIN CAPITAL LETTER M*/
/* D5 */ 0xD5,  /*<U004E>       LATIN CAPITAL LETTER N*/
/* D6 */ 0xD6,  /*<U004F>       LATIN CAPITAL LETTER O*/
/* D7 */ 0xD7,  /*<U0050>       LATIN CAPITAL LETTER P*/
/* D8 */ 0xD8,  /*<U0051>       LATIN CAPITAL LETTER Q*/
/* D9 */ 0xD9,  /*<U0052>       LATIN CAPITAL LETTER R*/
/* DA */ 0xDA,  /*<U00B9>       SUPERSCRIPT DIGIT ONE*/
/* DB */ 0xDB,  /*LEFT-TO-RIGHT OVERRIDE*/
/* DC */ 0xDC,  /*RIGHT-TO-LEFT OVERRIDE*/
/* DD */ 0xDD,  /*POP DIRECTION FORMATTING*/
/* DE */ 0xDE,  /* *** UNASSIGNED*/
/* DF */ 0xDF,  /* *** UNASSIGNED*/
/* E0 */ 0xE0,  /*<U005C>       BACKSLASH*/
/* E1 */ 0xE1,  /*<U00F7>       DIVISION SIGN*/
/* E2 */ 0xE2,  /*<U0053>       LATIN CAPITAL LETTER S*/
/* E3 */ 0xE3,  /*<U0054>       LATIN CAPITAL LETTER T*/
/* E4 */ 0xE4,  /*<U0055>       LATIN CAPITAL LETTER U*/
/* E5 */ 0xE5,  /*<U0056>       LATIN CAPITAL LETTER V*/
/* E6 */ 0xE6,  /*<U0057>       LATIN CAPITAL LETTER W*/
/* E7 */ 0xE7,  /*<U0058>       LATIN CAPITAL LETTER X*/
/* E8 */ 0xE8,  /*<U0059>       LATIN CAPITAL LETTER Y*/
/* E9 */ 0xE9,  /*<U005A>       LATIN CAPITAL LETTER Z*/
/* EA */ 0xEA,  /*<U00B2>       SUPERSCRIPT DIGIT TWO*/
/* EB */ 0xEB,  /* *** UNASSIGNED*/
/* EC */ 0xEC,  /* *** UNASSIGNED*/
/* ED */ 0xED,  /* *** UNASSIGNED*/
/* EE */ 0xEE,  /* *** UNASSIGNED*/
/* EF */ 0xEF,  /* *** UNASSIGNED*/
/* F0 */ 0xF0,  /*<U0030>       DIGIT ZERO*/
/* F1 */ 0xF1,  /*<U0031>       DIGIT ONE*/
/* F2 */ 0xF2,  /*<U0032>       DIGIT TWO*/
/* F3 */ 0xF3,  /*<U0033>       DIGIT THREE*/
/* F4 */ 0xF4,  /*<U0034>       DIGIT FOUR*/
/* F5 */ 0xF5,  /*<U0035>       DIGIT FIVE*/
/* F6 */ 0xF6,  /*<U0036>       DIGIT SIX*/
/* F7 */ 0xF7,  /*<U0037>       DIGIT SEVEN*/
/* F8 */ 0xF8,  /*<U0038>       DIGIT EIGHT*/
/* F9 */ 0xF9,  /*<U0039>       DIGIT NINE*/
/* FA */ 0xFA,  /*<U00B3>       SUPERSCRIPT DIGIT THREE*/
/* FB */ 0xFB,  /*LEFT-TO-RIGHT EMBEDDED*/
/* FC */ 0xFC,  /*RIGHT-TO-LEFT EMBEDDED*/
/* FD */ 0xFD,  /*LEFT-TO-RIGHT MARKER*/
/* FE */ 0xFE,  /*RIGHT-TO-LEFT MARKER*/
/* FF */ 0xFF /*<U009F>       EIGHT ONES*/
  },
  /* 862 OS/2 Hebrew*/
  {
/* 00 */ 0x00,  /*<U0000>       NULL*/
/* 01 */ 0x01,  /*<U0001>       START OF HEADING*/
/* 02 */ 0x02,  /*<U0002>       START OF TEXT*/
/* 03 */ 0x03,  /*<U0003>       END OF TEXT*/
/* 04 */ 0x04,  /*<U0004>       END OF TRANSMISSION*/
/* 05 */ 0x05,  /*<U0005>       ENQUIRY*/
/* 06 */ 0x06,  /*<U0006>       ACKNOWLEDGE*/
/* 07 */ 0x07,  /*<U0007>       BELL*/
/* 08 */ 0x08,  /*<U0008>       BACKSPACE*/
/* 09 */ 0x09,  /*<U0009>       HORIZONTAL TABULATION*/
/* 0A */ 0x0A,  /*<U000A>       LINE FEED*/
/* 0B */ 0x0B,  /*<U000B>       VERTICAL TABULATION*/
/* 0C */ 0x0C,  /*<U000C>       FORM FEED*/
/* 0D */ 0x0D,  /*<U000D>       CARRIAGE RETURN*/
/* 0E */ 0x0E,  /*<U000E>       SHIFT OUT*/
/* 0F */ 0x0F,  /*<U000F>       SHIFT IN*/
/* 10 */ 0x10,  /*<U0010>       DATA LINK ESCAPE*/
/* 11 */ 0x11,  /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */ 0x12,  /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */ 0x13,  /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */ 0x14,  /*<U0014>       DEVICE CONTROL FOUR*/
/* 15 */ 0x15,  /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 16 */ 0x16,  /*<U0016>       SYNCHRONOUS IDLE*/
/* 17 */ 0x17,  /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 18 */ 0x18,  /*<U0018>       CANCEL*/
/* 19 */ 0x19,  /*<U0019>       END OF MEDIUM*/
/* 1A */ 0x1A,  /*<U001C>       FILE SEPARATOR*/
/* 1B */ 0x1B,  /*<U001B>       ESCAPE*/
/* 1C */ 0x1C,  /*<U007F>       DELETE*/
/* 1D */ 0x1D,  /*<U001D>       GROUP SEPARATOR*/
/* 1E */ 0x1E,  /*<U001E>       RECORD SEPARATOR*/
/* 1F */ 0x1F,  /*<U001F>       UNIT SEPARATOR*/
/* 20 */ 0x20,  /*<U0020>       SPACE*/
/* 21 */ 0x21,  /*<U0021>       EXCLAMATION MARK*/
/* 22 */ 0x22,  /*<U0022>       QUOTATION MARK*/
/* 23 */ 0x23,  /*<U0023>       NUMBER SIGN*/
/* 24 */ 0x24,  /*<U0024>       DOLLAR SIGN*/
/* 25 */ 0x25,  /*<U0025>       PERCENT SIGN*/
/* 26 */ 0x26,  /*<U0026>       AMPERSAND*/
/* 27 */ 0x27,  /*<U0027>       APOSTROPHE*/
/* 28 */ 0x29,  /*<U0028>       OPENING PARENTHESIS*/
/* 29 */ 0x28,  /*<U0029>       CLOSING PARENTHESIS*/
/* 2A */ 0x2A,  /*<U002A>       ASTERISK*/
/* 2B */ 0x2B,  /*<U002B>       PLUS SIGN*/
/* 2C */ 0x2C,  /*<U002C>       COMMA*/
/* 2D */ 0x2D,  /*<U002D>       HYPHEN-MINUS*/
/* 2E */ 0x2E,  /*<U002E>       PERIOD*/
/* 2F */ 0x2F,  /*<U002F>       SLASH*/
/* 30 */ 0x30,  /*<U0030>       DIGIT ZERO*/
/* 31 */ 0x31,  /*<U0031>       DIGIT ONE*/
/* 32 */ 0x32,  /*<U0032>       DIGIT TWO*/
/* 33 */ 0x33,  /*<U0033>       DIGIT THREE*/
/* 34 */ 0x34,  /*<U0034>       DIGIT FOUR*/
/* 35 */ 0x35,  /*<U0035>       DIGIT FIVE*/
/* 36 */ 0x36,  /*<U0036>       DIGIT SIX*/
/* 37 */ 0x37,  /*<U0037>       DIGIT SEVEN*/
/* 38 */ 0x38,  /*<U0038>       DIGIT EIGHT*/
/* 39 */ 0x39,  /*<U0039>       DIGIT NINE*/
/* 3A */ 0x3A,  /*<U003A>       COLON*/
/* 3B */ 0x3B,  /*<U003B>       SEMICOLON*/
/* 3C */ 0x3E,  /*<U003C>       LESS-THAN SIGN*/
/* 3D */ 0x3D,  /*<U003D>       EQUALS SIGN*/
/* 3E */ 0x3C,  /*<U003E>       GREATER-THAN SIGN*/
/* 3F */ 0x3F,  /*<U003F>       QUESTION MARK*/
/* 40 */ 0x40,  /*<U0040>       COMMERCIAL AT*/
/* 41 */ 0x41,  /*<U0041>       LATIN CAPITAL LETTER A*/
/* 42 */ 0x42,  /*<U0042>       LATIN CAPITAL LETTER B*/
/* 43 */ 0x43,  /*<U0043>       LATIN CAPITAL LETTER C*/
/* 44 */ 0x44,  /*<U0044>       LATIN CAPITAL LETTER D*/
/* 45 */ 0x45,  /*<U0045>       LATIN CAPITAL LETTER E*/
/* 46 */ 0x46,  /*<U0046>       LATIN CAPITAL LETTER F*/
/* 47 */ 0x47,  /*<U0047>       LATIN CAPITAL LETTER G*/
/* 48 */ 0x48,  /*<U0048>       LATIN CAPITAL LETTER H*/
/* 49 */ 0x49,  /*<U0049>       LATIN CAPITAL LETTER I*/
/* 4A */ 0x4A,  /*<U004A>       LATIN CAPITAL LETTER J*/
/* 4B */ 0x4B,  /*<U004B>       LATIN CAPITAL LETTER K*/
/* 4C */ 0x4C,  /*<U004C>       LATIN CAPITAL LETTER L*/
/* 4D */ 0x4D,  /*<U004D>       LATIN CAPITAL LETTER M*/
/* 4E */ 0x4E,  /*<U004E>       LATIN CAPITAL LETTER N*/
/* 4F */ 0x4F,  /*<U004F>       LATIN CAPITAL LETTER O*/
/* 50 */ 0x50,  /*<U0050>       LATIN CAPITAL LETTER P*/
/* 51 */ 0x51,  /*<U0051>       LATIN CAPITAL LETTER Q*/
/* 52 */ 0x52,  /*<U0052>       LATIN CAPITAL LETTER R*/
/* 53 */ 0x53,  /*<U0053>       LATIN CAPITAL LETTER S*/
/* 54 */ 0x54,  /*<U0054>       LATIN CAPITAL LETTER T*/
/* 55 */ 0x55,  /*<U0055>       LATIN CAPITAL LETTER U*/
/* 56 */ 0x56,  /*<U0056>       LATIN CAPITAL LETTER V*/
/* 57 */ 0x57,  /*<U0057>       LATIN CAPITAL LETTER W*/
/* 58 */ 0x58,  /*<U0058>       LATIN CAPITAL LETTER X*/
/* 59 */ 0x59,  /*<U0059>       LATIN CAPITAL LETTER Y*/
/* 5A */ 0x5A,  /*<U005A>       LATIN CAPITAL LETTER Z*/
/* 5B */ 0x5D,  /*<U005B>       OPENING SQUARE BRACKET*/
/* 5C */ 0x5C,  /*<U005C>       BACKSLASH*/
/* 5D */ 0x5B,  /*<U005D>       CLOSING SQUARE BRACKET*/
/* 5E */ 0x5E,  /*<U005E>       SPACING CIRCUMFLEX*/
/* 5F */ 0x5F,  /*<U005F>       SPACING UNDERSCORE*/
/* 60 */ 0x60,  /*<U0060>       SPACING GRAVE*/
/* 61 */ 0x61,  /*<U0061>       LATIN SMALL LETTER A*/
/* 62 */ 0x62,  /*<U0062>       LATIN SMALL LETTER B*/
/* 63 */ 0x63,  /*<U0063>       LATIN SMALL LETTER C*/
/* 64 */ 0x64,  /*<U0064>       LATIN SMALL LETTER D*/
/* 65 */ 0x65,  /*<U0065>       LATIN SMALL LETTER E*/
/* 66 */ 0x66,  /*<U0066>       LATIN SMALL LETTER F*/
/* 67 */ 0x67,  /*<U0067>       LATIN SMALL LETTER G*/
/* 68 */ 0x68,  /*<U0068>       LATIN SMALL LETTER H*/
/* 69 */ 0x69,  /*<U0069>       LATIN SMALL LETTER I*/
/* 6A */ 0x6A,  /*<U006A>       LATIN SMALL LETTER J*/
/* 6B */ 0x6B,  /*<U006B>       LATIN SMALL LETTER K*/
/* 6C */ 0x6C,  /*<U006C>       LATIN SMALL LETTER L*/
/* 6D */ 0x6D,  /*<U006D>       LATIN SMALL LETTER M*/
/* 6E */ 0x6E,  /*<U006E>       LATIN SMALL LETTER N*/
/* 6F */ 0x6F,  /*<U006F>       LATIN SMALL LETTER O*/
/* 70 */ 0x70,  /*<U0070>       LATIN SMALL LETTER P*/
/* 71 */ 0x71,  /*<U0071>       LATIN SMALL LETTER Q*/
/* 72 */ 0x72,  /*<U0072>       LATIN SMALL LETTER R*/
/* 73 */ 0x73,  /*<U0073>       LATIN SMALL LETTER S*/
/* 74 */ 0x74,  /*<U0074>       LATIN SMALL LETTER T*/
/* 75 */ 0x75,  /*<U0075>       LATIN SMALL LETTER U*/
/* 76 */ 0x76,  /*<U0076>       LATIN SMALL LETTER V*/
/* 77 */ 0x77,  /*<U0077>       LATIN SMALL LETTER W*/
/* 78 */ 0x78,  /*<U0078>       LATIN SMALL LETTER X*/
/* 79 */ 0x79,  /*<U0079>       LATIN SMALL LETTER Y*/
/* 7A */ 0x7A,  /*<U007A>       LATIN SMALL LETTER Z*/
/* 7B */ 0x7D,  /*<U007B>       OPENING CURLY BRACKET*/
/* 7C */ 0x7C,  /*<U007C>       VERTICAL BAR*/
/* 7D */ 0x7B,  /*<U007D>       CLOSING CURLY BRACKET*/
/* 7E */ 0x7E,  /*<U007E>       SPACING TILDE*/
/* 7F */ 0x7F,  /*<U001A>       SUBSTITUTE*/
/* 80 */ 0x80,  /*<U05D0>       HEBREW LETTER ALEF*/
/* 81 */ 0x81,  /*<U05D1>       HEBREW LETTER BET*/
/* 82 */ 0x82,  /*<U05D2>       HEBREW LETTER GIMEL*/
/* 83 */ 0x83,  /*<U05D3>       HEBREW LETTER DALET*/
/* 84 */ 0x84,  /*<U05D4>       HEBREW LETTER HE*/
/* 85 */ 0x85,  /*<U05D5>       HEBREW LETTER VAV*/
/* 86 */ 0x86,  /*<U05D6>       HEBREW LETTER ZAYIN*/
/* 87 */ 0x87,  /*<U05D7>       HEBREW LETTER HET*/
/* 88 */ 0x88,  /*<U05D8>       HEBREW LETTER TET*/
/* 89 */ 0x89,  /*<U05D9>       HEBREW LETTER YOD*/
/* 8A */ 0x8A,  /*<U05DA>       HEBREW LETTER FINAL KAF*/
/* 8B */ 0x8B,  /*<U05DB>       HEBREW LETTER KAF*/
/* 8C */ 0x8C,  /*<U05DC>       HEBREW LETTER LAMED*/
/* 8D */ 0x8D,  /*<U05DD>       HEBREW LETTER FINAL MEM*/
/* 8E */ 0x8E,  /*<U05DE>       HEBREW LETTER MEM*/
/* 8F */ 0x8F,  /*<U05DF>       HEBREW LETTER FINAL NUN*/
/* 90 */ 0x90,  /*<U05E0>       HEBREW LETTER NUN*/
/* 91 */ 0x91,  /*<U05E1>       HEBREW LETTER SAMEKH*/
/* 92 */ 0x92,  /*<U05E2>       HEBREW LETTER AYIN*/
/* 93 */ 0x93,  /*<U05E3>       HEBREW LETTER FINAL PE*/
/* 94 */ 0x94,  /*<U05E4>       HEBREW LETTER PE*/
/* 95 */ 0x95,  /*<U05E5>       HEBREW LETTER FINAL TSADI*/
/* 96 */ 0x96,  /*<U05E6>       HEBREW LETTER TSADI*/
/* 97 */ 0x97,  /*<U05E7>       HEBREW LETTER QOF*/
/* 98 */ 0x98,  /*<U05E8>       HEBREW LETTER RESH*/
/* 99 */ 0x99,  /*<U05E9>       HEBREW LETTER SHIN*/
/* 9A */ 0x9A,  /*<U05EA>       HEBREW LETTER TAV*/
/* 9B */ 0x9B,  /*<U00A2>       CENT SIGN*/
/* 9C */ 0x9C,  /*<U00A3>       POUND SIGN*/
/* 9D */ 0x9D,  /*<U00A5>       YEN SIGN*/
/* 9E */ 0x9E,  /*<U20A7>       PESETA SIGN*/
/* 9F */ 0x9F,  /*<U0192>       LATIN SMALL LETTER SCRIPT F (FLORIN SIGN)*/
/* A0 */ 0xA0,  /*<U00E1>       LATIN SMALL LETTER A ACUTE*/
/* A1 */ 0xA1,  /*<U00ED>       LATIN SMALL LETTER I ACUTE*/
/* A2 */ 0xA2,  /*<U00F3>       LATIN SMALL LETTER O ACUTE*/
/* A3 */ 0xA3,  /*<U00FA>       LATIN SMALL LETTER U ACUTE*/
/* A4 */ 0xA4,  /*<U00F1>       LATIN SMALL LETTER N TILDE*/
/* A5 */ 0xA5,  /*<U00D1>       LATIN CAPITAL LETTER N TILDE*/
/* A6 */ 0xA6,  /*<U00AA>       FEMININE ORDINAL INDICATOR*/
/* A7 */ 0xA7,  /*<U00BA>       MASCULINE ORDINAL INDICATOR*/
/* A8 */ 0xA8,  /*<U00BF>       INVERTED QUESTION MARK*/
/* A9 */ 0xA9,  /*<U2310>       REVERSED NOT SIGN*/
/* AA */ 0xAA,  /*<U00AC>       NOT SIGN*/
/* AB */ 0xAB,  /*<U00BD>       FRACTION ONE HALF*/
/* AC */ 0xAC,  /*<U00BC>       FRACTION ONE QUARTER*/
/* AD */ 0xAD,  /*<U00A1>       INVERTED EXCLAMATION MARK*/
/* AE */ 0xAF,  /*<U00AB>       LEFT POINTING GUILLEMET*/
/* AF */ 0xAE,  /*<U00BB>       RIGHT POINTING GUILLEMET*/
/* B0 */ 0xB0,  /*<U2591>       LIGHT SHADE*/
/* B1 */ 0xB1,  /*<U2592>       MEDIUM SHADE*/
/* B2 */ 0xB2,  /*<U2593>       DARK SHADE*/
/* B3 */ 0xB3,  /*<U2502>       FORMS LIGHT VERTICAL*/
/* B4 */ 0xB4,  /*<U2524>       FORMS LIGHT VERTICAL AND LEFT*/
/* B5 */ 0xB5,  /*<U2561>       FORMS VERTICAL SINGLE AND LEFT DOUBLE*/
/* B6 */ 0xB6,  /*<U2562>       FORMS VERTICAL DOUBLE AND LEFT SINGLE*/
/* B7 */ 0xB7,  /*<U2556>       FORMS DOWN DOUBLE AND LEFT SINGLE*/
/* B8 */ 0xB8,  /*<U2555>       FORMS DOWN SINGLE AND LEFT DOUBLE*/
/* B9 */ 0xB9,  /*<U2563>       FORMS DOUBLE VERTICAL AND LEFT*/
/* BA */ 0xBA,  /*<U2551>       FORMS DOUBLE VERTICAL*/
/* BB */ 0xBB,  /*<U2557>       FORMS DOUBLE DOWN AND LEFT*/
/* BC */ 0xBC,  /*<U255D>       FORMS DOUBLE UP AND LEFT*/
/* BD */ 0xBD,  /*<U255C>       FORMS UP DOUBLE AND LEFT SINGLE*/
/* BE */ 0xBE,  /*<U255B>       FORMS UP SINGLE AND LEFT DOUBLE*/
/* BF */ 0xBF,  /*<U2510>       FORMS LIGHT DOWN AND LEFT*/
/* C0 */ 0xC0,  /*<U2514>       FORMS LIGHT UP AND RIGHT*/
/* C1 */ 0xC1,  /*<U2534>       FORMS LIGHT UP AND HORIZONTAL*/
/* C2 */ 0xC2,  /*<U252C>       FORMS LIGHT DOWN AND HORIZONTAL*/
/* C3 */ 0xC3,  /*<U251C>       FORMS LIGHT VERTICAL AND RIGHT*/
/* C4 */ 0xC4,  /*<U2500>       FORMS LIGHT HORIZONTAL*/
/* C5 */ 0xC5,  /*<U253C>       FORMS LIGHT VERTICAL AND HORIZONTAL*/
/* C6 */ 0xC6,  /*<U255E>       FORMS VERTICAL SINGLE AND RIGHT DOUBLE*/
/* C7 */ 0xC7,  /*<U255F>       FORMS VERTICAL DOUBLE AND RIGHT SINGLE*/
/* C8 */ 0xC8,  /*<U255A>       FORMS DOUBLE UP AND RIGHT*/
/* C9 */ 0xC9,  /*<U2554>       FORMS DOUBLE DOWN AND RIGHT*/
/* CA */ 0xCA,  /*<U2569>       FORMS DOUBLE UP AND HORIZONTAL*/
/* CB */ 0xCB,  /*<U2566>       FORMS DOUBLE DOWN AND HORIZONTAL*/
/* CC */ 0xCC,  /*<U2560>       FORMS DOUBLE VERTICAL AND RIGHT*/
/* CD */ 0xCD,  /*<U2550>       FORMS DOUBLE HORIZONTAL*/
/* CE */ 0xCE,  /*<U256C>       FORMS DOUBLE VERTICAL AND HORIZONTAL*/
/* CF */ 0xCF,  /*<U2567>       FORMS UP SINGLE AND HORIZONTAL DOUBLE*/
/* D0 */ 0xD0,  /*<U2568>       FORMS UP DOUBLE AND HORIZONTAL SINGLE*/
/* D1 */ 0xD1,  /*<U2564>       FORMS DOWN SINGLE AND HORIZONTAL DOUBLE*/
/* D2 */ 0xD2,  /*<U2565>       FORMS DOWN DOUBLE AND HORIZONTAL SINGLE*/
/* D3 */ 0xD3,  /*<U2559>       FORMS UP DOUBLE AND RIGHT SINGLE*/
/* D4 */ 0xD4,  /*<U2558>       FORMS UP SINGLE AND RIGHT DOUBLE*/
/* D5 */ 0xD5,  /*<U2552>       FORMS DOWN SINGLE AND RIGHT DOUBLE*/
/* D6 */ 0xD6,  /*<U2553>       FORMS DOWN DOUBLE AND RIGHT SINGLE*/
/* D7 */ 0xD7,  /*<U256B>       FORMS VERTICAL DOUBLE AND HORIZONTAL SINGLE*/
/* D8 */ 0xD8,  /*<U256A>       FORMS VERTICAL SINGLE AND HORIZONTAL DOUBLE*/
/* D9 */ 0xD9,  /*<U2518>       FORMS LIGHT UP AND LEFT*/
/* DA */ 0xDA,  /*<U250C>       FORMS LIGHT DOWN AND RIGHT*/
/* DB */ 0xDB,  /*<U2588>       FULL BLOCK*/
/* DC */ 0xDC,  /*<U2584>       LOWER HALF BLOCK*/
/* DD */ 0xDD,  /*<U258C>       LEFT HALF BLOCK*/
/* DE */ 0xDE,  /*<U2590>       RIGHT HALF BLOCK*/
/* DF */ 0xDF,  /*<U2580>       UPPER HALF BLOCK*/
/* E0 */ 0xE0,  /*<U03B1>       GREEK SMALL LETTER ALPHA*/
/* E1 */ 0xE1,  /*<U00DF>       LATIN SMALL LETTER SHARP S*/
/* E2 */ 0xE2,  /*<U0393>       GREEK CAPITAL LETTER GAMMA*/
/* E3 */ 0xE3,  /*<U03C0>       GREEK SMALL LETTER PI*/
/* E4 */ 0xE4,  /*<U03A3>       GREEK CAPITAL LETTER SIGMA*/
/* E5 */ 0xE5,  /*<U03C3>       GREEK SMALL LETTER SIGMA*/
/* E6 */ 0xE6,  /*<U03BC>       GREEK SMALL LETTER MU*/
/* E7 */ 0xE7,  /*<U03C4>       GREEK SMALL LETTER TAU*/
/* E8 */ 0xE8,  /*<U03A6>       GREEK CAPITAL LETTER PHI*/
/* E9 */ 0xE9,  /*<U0398>       GREEK CAPITAL LETTER THETA*/
/* EA */ 0xEA,  /*<U03A9>       GREEK CAPITAL LETTER OMEGA*/
/* EB */ 0xEB,  /*<U03B4>       GREEK SMALL LETTER DELTA*/
/* EC */ 0xEC,  /*<U221E>       INFINITY*/
/* ED */ 0xED,  /*<U03C6>       GREEK SMALL LETTER PHI*/
/* EE */ 0xEE,  /*<U03B5>       GREEK SMALL LETTER EPSILON*/
/* EF */ 0xEF,  /*<U2229>       INTERSECTION*/
/* F0 */ 0xF0,  /*<U2261>       IDENTICAL TO*/
/* F1 */ 0xF1,  /*<U00B1>       PLUS-OR-MINUS SIGN*/
/* F2 */ 0xF2,  /*<U2265>       GREATER THAN OR EQUAL TO*/
/* F3 */ 0xF3,  /*<U2264>       LESS THAN OR EQUAL TO*/
/* F4 */ 0xF4,  /*<U2320>       TOP HALF INTEGRAL*/
/* F5 */ 0xF5,  /*<U2321>       BOTTOM HALF INTEGRAL*/
/* F6 */ 0xF6,  /*<U00F7>       DIVISION SIGN*/
/* F7 */ 0xF7,  /*<U2248>       ALMOST EQUAL TO*/
/* F8 */ 0xF8,  /*<U00B0>       DEGREE SIGN*/
/* F9 */ 0xF9,  /*<U2219>       DOT OPERATOR*/
/* FA */ 0xFA,  /*<U00B7>       MIDDLE DOT*/
/* FB */ 0xFB,  /*<U221A>       SQUARE ROOT*/
/* FC */ 0xFC,  /*<U207F>       SUPERSCRIPT LATIN SMALL LETTER N*/
/* FD */ 0xFD,  /*<U00B2>       SUPERSCRIPT DIGIT TWO*/
/* FE */ 0xFE,  /*<U25A0>       BLACK SQUARE*/
/* FF */ 0xFF /*<U00A0>       NON-BREAKING SPACE*/
  },
  /* 856 AIX Hebrew*/
  {
/* 00 */ 0x00, /*<U0000>       NULL*/
/* 01 */ 0x01, /*<U0001>       START OF HEADING*/
/* 02 */ 0x02, /*<U0002>       START OF TEXT*/
/* 03 */ 0x03, /*<U0003>       END OF TEXT*/
/* 04 */ 0x04, /*<U0004>       END OF TRANSMISSION*/
/* 05 */ 0x05, /*<U0005>       ENQUIRY*/
/* 06 */ 0x06, /*<U0006>       ACKNOWLEDGE*/
/* 07 */ 0x07, /*<U0007>       BELL*/
/* 08 */ 0x08, /*<U0008>       BACKSPACE*/
/* 09 */ 0x09, /*<U0009>       HORIZONTAL TABULATION*/
/* 0A */ 0x0A, /*<U000A>       LINE FEED*/
/* 0B */ 0x0B, /*<U000B>       VERTICAL TABULATION*/
/* 0C */ 0x0C, /*<U000C>       FORM FEED*/
/* 0D */ 0x0D, /*<U000D>       CARRIAGE RETURN*/
/* 0E */ 0x0E, /*<U000E>       SHIFT OUT*/
/* 0F */ 0x0F, /*<U000F>       SHIFT IN*/
/* 10 */ 0x10, /*<U0010>       DATA LINK ESCAPE*/
/* 11 */ 0x11, /*<U0011>       DEVICE CONTROL ONE*/
/* 12 */ 0x12, /*<U0012>       DEVICE CONTROL TWO*/
/* 13 */ 0x13, /*<U0013>       DEVICE CONTROL THREE*/
/* 14 */ 0x14, /*<U0014>       DEVICE CONTROL FOUR*/
/* 15 */ 0x15, /*<U0015>       NEGATIVE ACKNOWLEDGE*/
/* 16 */ 0x16, /*<U0016>       SYNCHRONOUS IDLE*/
/* 17 */ 0x17, /*<U0017>       END OF TRANSMISSION BLOCK*/
/* 18 */ 0x18, /*<U0018>       CANCEL*/
/* 19 */ 0x19, /*<U0019>       END OF MEDIUM*/
/* 1A */ 0x1A, /*<U001C>       FILE SEPARATOR*/
/* 1B */ 0x1B, /*<U001B>       ESCAPE*/
/* 1C */ 0x1C, /*<U007F>       DELETE*/
/* 1D */ 0x1D, /*<U001D>       GROUP SEPARATOR*/
/* 1E */ 0x1E, /*<U001E>       RECORD SEPARATOR*/
/* 1F */ 0x1F, /*<U001F>       UNIT SEPARATOR*/
/* 20 */ 0x20, /*<U0020>       SPACE*/
/* 21 */ 0x21, /*<U0021>       EXCLAMATION MARK*/
/* 22 */ 0x22, /*<U0022>       QUOTATION MARK*/
/* 23 */ 0x23, /*<U0023>       NUMBER SIGN*/
/* 24 */ 0x24, /*<U0024>       DOLLAR SIGN*/
/* 25 */ 0x25, /*<U0025>       PERCENT SIGN*/
/* 26 */ 0x26, /*<U0026>       AMPERSAND*/
/* 27 */ 0x27, /*<U0027>       APOSTROPHE*/
/* 28 */ 0x29, /*<U0028>       OPENING PARENTHESIS*/
/* 29 */ 0x28, /*<U0029>       CLOSING PARENTHESIS*/
/* 2A */ 0x2A, /*<U002A>       ASTERISK*/
/* 2B */ 0x2B, /*<U002B>       PLUS SIGN*/
/* 2C */ 0x2C, /*<U002C>       COMMA*/
/* 2D */ 0x2D, /*<U002D>       HYPHEN-MINUS*/
/* 2E */ 0x2E, /*<U002E>       PERIOD*/
/* 2F */ 0x2F, /*<U002F>       SLASH*/
/* 30 */ 0x30, /*<U0030>       DIGIT ZERO*/
/* 31 */ 0x31, /*<U0031>       DIGIT ONE*/
/* 32 */ 0x32, /*<U0032>       DIGIT TWO*/
/* 33 */ 0x33, /*<U0033>       DIGIT THREE*/
/* 34 */ 0x34, /*<U0034>       DIGIT FOUR*/
/* 35 */ 0x35, /*<U0035>       DIGIT FIVE*/
/* 36 */ 0x36, /*<U0036>       DIGIT SIX*/
/* 37 */ 0x37, /*<U0037>       DIGIT SEVEN*/
/* 38 */ 0x38, /*<U0038>       DIGIT EIGHT*/
/* 39 */ 0x39, /*<U0039>       DIGIT NINE*/
/* 3A */ 0x3A, /*<U003A>       COLON*/
/* 3B */ 0x3B, /*<U003B>       SEMICOLON*/
/* 3C */ 0x3E, /*<U003C>       LESS-THAN SIGN*/
/* 3D */ 0x3D, /*<U003D>       EQUALS SIGN*/
/* 3E */ 0x3C, /*<U003E>       GREATER-THAN SIGN*/
/* 3F */ 0x3F, /*<U003F>       QUESTION MARK*/
/* 40 */ 0x40, /*<U0040>       COMMERCIAL AT*/
/* 41 */ 0x41, /*<U0041>       LATIN CAPITAL LETTER A*/
/* 42 */ 0x42, /*<U0042>       LATIN CAPITAL LETTER B*/
/* 43 */ 0x43, /*<U0043>       LATIN CAPITAL LETTER C*/
/* 44 */ 0x44, /*<U0044>       LATIN CAPITAL LETTER D*/
/* 45 */ 0x45, /*<U0045>       LATIN CAPITAL LETTER E*/
/* 46 */ 0x46, /*<U0046>       LATIN CAPITAL LETTER F*/
/* 47 */ 0x47, /*<U0047>       LATIN CAPITAL LETTER G*/
/* 48 */ 0x48, /*<U0048>       LATIN CAPITAL LETTER H*/
/* 49 */ 0x49, /*<U0049>       LATIN CAPITAL LETTER I*/
/* 4A */ 0x4A, /*<U004A>       LATIN CAPITAL LETTER J*/
/* 4B */ 0x4B, /*<U004B>       LATIN CAPITAL LETTER K*/
/* 4C */ 0x4C, /*<U004C>       LATIN CAPITAL LETTER L*/
/* 4D */ 0x4D, /*<U004D>       LATIN CAPITAL LETTER M*/
/* 4E */ 0x4E, /*<U004E>       LATIN CAPITAL LETTER N*/
/* 4F */ 0x4F, /*<U004F>       LATIN CAPITAL LETTER O*/
/* 50 */ 0x50, /*<U0050>       LATIN CAPITAL LETTER P*/
/* 51 */ 0x51, /*<U0051>       LATIN CAPITAL LETTER Q*/
/* 52 */ 0x52, /*<U0052>       LATIN CAPITAL LETTER R*/
/* 53 */ 0x53, /*<U0053>       LATIN CAPITAL LETTER S*/
/* 54 */ 0x54, /*<U0054>       LATIN CAPITAL LETTER T*/
/* 55 */ 0x55, /*<U0055>       LATIN CAPITAL LETTER U*/
/* 56 */ 0x56, /*<U0056>       LATIN CAPITAL LETTER V*/
/* 57 */ 0x57, /*<U0057>       LATIN CAPITAL LETTER W*/
/* 58 */ 0x58, /*<U0058>       LATIN CAPITAL LETTER X*/
/* 59 */ 0x59, /*<U0059>       LATIN CAPITAL LETTER Y*/
/* 5A */ 0x5A, /*<U005A>       LATIN CAPITAL LETTER Z*/
/* 5B */ 0x5D, /*<U005B>       OPENING SQUARE BRACKET*/
/* 5C */ 0x5C, /*<U005C>       BACKSLASH*/
/* 5D */ 0x5B, /*<U005D>       CLOSING SQUARE BRACKET*/
/* 5E */ 0x5E, /*<U005E>       SPACING CIRCUMFLEX*/
/* 5F */ 0x5F, /*<U005F>       SPACING UNDERSCORE*/
/* 60 */ 0x60, /*<U0060>       SPACING GRAVE*/
/* 61 */ 0x61, /*<U0061>       LATIN SMALL LETTER A*/
/* 62 */ 0x62, /*<U0062>       LATIN SMALL LETTER B*/
/* 63 */ 0x63, /*<U0063>       LATIN SMALL LETTER C*/
/* 64 */ 0x64, /*<U0064>       LATIN SMALL LETTER D*/
/* 65 */ 0x65, /*<U0065>       LATIN SMALL LETTER E*/
/* 66 */ 0x66, /*<U0066>       LATIN SMALL LETTER F*/
/* 67 */ 0x67, /*<U0067>       LATIN SMALL LETTER G*/
/* 68 */ 0x68, /*<U0068>       LATIN SMALL LETTER H*/
/* 69 */ 0x69, /*<U0069>       LATIN SMALL LETTER I*/
/* 6A */ 0x6A, /*<U006A>       LATIN SMALL LETTER J*/
/* 6B */ 0x6B, /*<U006B>       LATIN SMALL LETTER K*/
/* 6C */ 0x6C, /*<U006C>       LATIN SMALL LETTER L*/
/* 6D */ 0x6D, /*<U006D>       LATIN SMALL LETTER M*/
/* 6E */ 0x6E, /*<U006E>       LATIN SMALL LETTER N*/
/* 6F */ 0x6F, /*<U006F>       LATIN SMALL LETTER O*/
/* 70 */ 0x70, /*<U0070>       LATIN SMALL LETTER P*/
/* 71 */ 0x71, /*<U0071>       LATIN SMALL LETTER Q*/
/* 72 */ 0x72, /*<U0072>       LATIN SMALL LETTER R*/
/* 73 */ 0x73, /*<U0073>       LATIN SMALL LETTER S*/
/* 74 */ 0x74, /*<U0074>       LATIN SMALL LETTER T*/
/* 75 */ 0x75, /*<U0075>       LATIN SMALL LETTER U*/
/* 76 */ 0x76, /*<U0076>       LATIN SMALL LETTER V*/
/* 77 */ 0x77, /*<U0077>       LATIN SMALL LETTER W*/
/* 78 */ 0x78, /*<U0078>       LATIN SMALL LETTER X*/
/* 79 */ 0x79, /*<U0079>       LATIN SMALL LETTER Y*/
/* 7A */ 0x7A, /*<U007A>       LATIN SMALL LETTER Z*/
/* 7B */ 0x7D, /*<U007B>       OPENING CURLY BRACKET*/
/* 7C */ 0x7C, /*<U007C>       VERTICAL BAR*/
/* 7D */ 0x7B, /*<U007D>       CLOSING CURLY BRACKET*/
/* 7E */ 0x7E, /*<U007E>       SPACING TILDE*/
/* 7F */ 0x7F, /*<U001A>       SUBSTITUTE*/
/* 80 */ 0x80, /*<U05D0>       HEBREW LETTER ALEF*/
/* 81 */ 0x81, /*<U05D1>       HEBREW LETTER BET*/
/* 82 */ 0x82, /*<U05D2>       HEBREW LETTER GIMEL*/
/* 83 */ 0x83, /*<U05D3>       HEBREW LETTER DALET*/
/* 84 */ 0x84, /*<U05D4>       HEBREW LETTER HE*/
/* 85 */ 0x85, /*<U05D5>       HEBREW LETTER VAV*/
/* 86 */ 0x86, /*<U05D6>       HEBREW LETTER ZAYIN*/
/* 87 */ 0x87, /*<U05D7>       HEBREW LETTER HET*/
/* 88 */ 0x88, /*<U05D8>       HEBREW LETTER TET*/
/* 89 */ 0x89, /*<U05D9>       HEBREW LETTER YOD*/
/* 8A */ 0x8A, /*<U05DA>       HEBREW LETTER FINAL KAF*/
/* 8B */ 0x8B, /*<U05DB>       HEBREW LETTER KAF*/
/* 8C */ 0x8C, /*<U05DC>       HEBREW LETTER LAMED*/
/* 8D */ 0x8D, /*<U05DD>       HEBREW LETTER FINAL MEM*/
/* 8E */ 0x8E, /*<U05DE>       HEBREW LETTER MEM*/
/* 8F */ 0x8F, /*<U05DF>       HEBREW LETTER FINAL NUN*/
/* 90 */ 0x90, /*<U05E0>       HEBREW LETTER NUN*/
/* 91 */ 0x91, /*<U05E1>       HEBREW LETTER SAMEKH*/
/* 92 */ 0x92, /*<U05E2>       HEBREW LETTER AYIN*/
/* 93 */ 0x93, /*<U05E3>       HEBREW LETTER FINAL PE*/
/* 94 */ 0x94, /*<U05E4>       HEBREW LETTER PE*/
/* 95 */ 0x95, /*<U05E5>       HEBREW LETTER FINAL TSADI*/
/* 96 */ 0x96, /*<U05E6>       HEBREW LETTER TSADI*/
/* 97 */ 0x97, /*<U05E7>       HEBREW LETTER QOF*/
/* 98 */ 0x98, /*<U05E8>       HEBREW LETTER RESH*/
/* 99 */ 0x99, /*<U05E9>       HEBREW LETTER SHIN*/
/* 9A */ 0x9A, /*<U05EA>       HEBREW LETTER TAV*/
/* 9B */ 0x9B, /*NEW SHEKELS SIGN*/
/* 9C */ 0x9C, /*POUNDS SIGN*/
/* 9D */ 0x9D, /*CENTS SIGN*/
/* 9E */ 0x9E, /*<U00D7>       MULTIPLICATION SIGN*/
/* 9F */ 0x9F, /* *** UNASSIGNED*/
/* A0 */ 0xA0, /* *** UNASSIGNED*/
/* A1 */ 0xA1, /* *** UNASSIGNED*/
/* A2 */ 0xA2, /* *** UNASSIGNED*/
/* A3 */ 0xA3, /* *** UNASSIGNED*/
/* A4 */ 0xA4, /* *** UNASSIGNED*/
/* A5 */ 0xA5, /* *** UNASSIGNED*/
/* A6 */ 0xA6, /* *** UNASSIGNED*/
/* A7 */ 0xA7, /* *** UNASSIGNED*/
/* A8 */ 0xA8, /* *** UNASSIGNED*/
/* A9 */ 0xA9, /*<U00AE>       REGISTERED TRADE MARK SIGN*/
/* AA */ 0xAA, /*<U00AC>       NOT SIGN*/
/* AB */ 0xAB, /*<U00BD>       FRACTION ONE HALF*/
/* AC */ 0xAC, /*<U00BC>       FRACTION ONE QUARTER*/
/* AD */ 0xAD, /* *** UNASSIGNED*/
/* AE */ 0xAF, /*<U00AB>       LEFT POINTING GUILLEMET*/
/* AF */ 0xAE, /*<U00BB>       RIGHT POINTING GUILLEMET*/
/* B0 */ 0xB0, /*<U2591>       LIGHT SHADE*/
/* B1 */ 0xB1, /*<U2592>       MEDIUM SHADE*/
/* B2 */ 0xB2, /*<U2593>       DARK SHADE*/
/* B3 */ 0xB3, /*<U2502>       FORMS LIGHT VERTICAL*/
/* B4 */ 0xB4, /*<U2524>       FORMS LIGHT VERTICAL AND LEFT*/
/* B5 */ 0xB5, /* *** UNASSIGNED*/
/* B6 */ 0xB6, /* *** UNASSIGNED*/
/* B7 */ 0xB7, /* *** UNASSIGNED*/
/* B8 */ 0xB8, /*<U00A9>       COPYRIGHT SIGN*/
/* B9 */ 0xB9, /*YEN SIGN*/
/* BA */ 0xBA, /*<U2551>       FORMS DOUBLE VERTICAL*/
/* BB */ 0xBB, /*<U2557>       FORMS DOUBLE DOWN AND LEFT*/
/* BC */ 0xBC, /*<U255D>       FORMS DOUBLE UP AND LEFT*/
/* BD */ 0xBD, /*<U00A2>       CENT SIGN*/
/* BE */ 0xBE, /*<U00A5>       YEN SIGN*/
/* BF */ 0xBF, /*<U2510>       FORMS LIGHT DOWN AND LEFT*/
/* C0 */ 0xC0, /*<U2514>       FORMS LIGHT UP AND RIGHT*/
/* C1 */ 0xC1, /*<U2534>       FORMS LIGHT UP AND HORIZONTAL*/
/* C2 */ 0xC2, /*<U252C>       FORMS LIGHT DOWN AND HORIZONTAL*/
/* C3 */ 0xC3, /*<U251C>       FORMS LIGHT VERTICAL AND RIGHT*/
/* C4 */ 0xC4, /*<U2500>       FORMS LIGHT HORIZONTAL*/
/* C5 */ 0xC5, /*<U253C>       FORMS LIGHT VERTICAL AND HORIZONTAL*/
/* C6 */ 0xC6, /* *** UNASSIGNED*/
/* C7 */ 0xC7, /* *** UNASSIGNED*/
/* C8 */ 0xC8, /*<U255A>       FORMS DOUBLE UP AND RIGHT*/
/* C9 */ 0xC9, /*<U2554>       FORMS DOUBLE DOWN AND RIGHT*/
/* CA */ 0xCA, /*<U2569>       FORMS DOUBLE UP AND HORIZONTAL*/
/* CB */ 0xCB, /*<U2566>       FORMS DOUBLE DOWN AND HORIZONTAL*/
/* CC */ 0xCC, /*<U2560>       FORMS DOUBLE VERTICAL AND RIGHT*/
/* CD */ 0xCD, /*<U2550>       FORMS DOUBLE HORIZONTAL*/
/* CE */ 0xCE, /*<U256C>       FORMS DOUBLE VERTICAL AND HORIZONTAL*/
/* CF */ 0xCF, /*<U00A4>       CURRENCY SIGN*/
/* D0 */ 0xD0, /* *** UNASSIGNED*/
/* D1 */ 0xD1, /* *** UNASSIGNED*/
/* D2 */ 0xD2, /* *** UNASSIGNED*/
/* D3 */ 0xD3, /* *** UNASSIGNED*/
/* D4 */ 0xD4, /* *** UNASSIGNED*/
/* D5 */ 0xD5, /* *** UNASSIGNED*/
/* D6 */ 0xD6, /* *** UNASSIGNED*/
/* D7 */ 0xD7, /* *** UNASSIGNED*/
/* D8 */ 0xD8, /* *** UNASSIGNED*/
/* D9 */ 0xD9, /*<U2518>       FORMS LIGHT UP AND LEFT*/
/* DA */ 0xDA, /*<U250C>       FORMS LIGHT DOWN AND RIGHT*/
/* DB */ 0xDB, /*<U2588>       FULL BLOCK*/
/* DC */ 0xDC, /*<U2584>       LOWER HALF BLOCK*/
/* DD */ 0xDD, /*<U00A6>       BROKEN VERTICAL BAR*/
/* DE */ 0xDE, /* **UNASSIGNED*/
/* DF */ 0xDF, /*<U2580>       UPPER HALF BLOCK*/
/* E0 */ 0xE0, /* *** UNASSIGNED*/
/* E1 */ 0xE1, /* *** UNASSIGNED*/
/* E2 */ 0xE2, /* *** UNASSIGNED*/
/* E3 */ 0xE3, /* *** UNASSIGNED*/
/* E4 */ 0xE4, /* *** UNASSIGNED*/
/* E5 */ 0xE5, /* *** UNASSIGNED*/
/* E6 */ 0xE6, /*<U00B5>       MICRO SIGN*/
/* E7 */ 0xE7, /*LEFT TO RIGHT MARKER*/
/* E8 */ 0xE8, /*RIGHT TO LEFT MARKER*/
/* E9 */ 0xE9, /*LEFT TO RIGHT EMBED*/
/* EA */ 0xEA, /*RIGHT TO LEFT EMBED*/
/* EB */ 0xEB, /*LEFT TO RIGHT OVERRIDE*/
/* EC */ 0xEC, /*RIGHT TO LEFT OVERRIDE*/
/* ED */ 0xEA, /*POP DIRECTIONAL FORMATTING*/
/* EE */ 0xEE, /*<U00AF>       SPACING MACRON*/
/* EF */ 0xEF, /*<U00B4>       SPACING ACUTE*/
/* F0 */ 0xF0, /*<U00AD>       SOFT HYPHEN*/
/* F1 */ 0xF1, /*<U00B1>       PLUS-OR-MINUS SIGN*/
/* F2 */ 0xF1, /*<U2017>       SPACING DOUBLE UNDERSCORE*/
/* F3 */ 0xF3, /*<U00BE>       FRACTION THREE QUARTERS*/
/* F4 */ 0xF4, /*<U00B6>       PARAGRAPH SIGN*/
/* F5 */ 0xF5, /*<U00A7>       SECTION SIGN*/
/* F6 */ 0xF6, /*<U00F7>       DIVISION SIGN*/
/* F7 */ 0xF7, /*<U00B8>       SPACING CEDILLA*/
/* F8 */ 0xF8, /*<U00B0>       DEGREE SIGN*/
/* F9 */ 0xF9, /*<U00A8>       SPACING DIAERESIS*/
/* FA */ 0xFA, /*<U2022>       BULLET*/
/* FB */ 0xFB, /*<U00B9>       SUPERSCRIPT DIGIT ONE*/
/* FC */ 0xFC, /*<U00B3>       SUPERSCRIPT DIGIT THREE*/
/* FD */ 0xFD, /*<U00B2>       SUPERSCRIPT DIGIT TWO*/
/* FE */ 0xFE, /*<U25A0>       BLACK SQUARE*/
/* FF */ 0xFF  /*<U00A0>       NON-BREAKING SPACE*/
  },
  /* 916 AIX Hebrew = ISO 8859-8*/
  {
/* 00 */ 0x00, /*<U0000> (CC) Null*/
/* 01 */ 0x01, /*<U0001> (CC) Start of Heading*/
/* 02 */ 0x02, /*<U0002> (CC) Start of Text*/
/* 03 */ 0x03, /*<U0003> (CC) End of Text*/
/* 04 */ 0x04, /*<U0004> (CC) End of Transmission*/
/* 05 */ 0x05, /*<U0005> (CC) Enquiry*/
/* 06 */ 0x06, /*<U0006> (CC) Acknowledge*/
/* 07 */ 0x07, /*<U0007> (CC) Bell*/
/* 08 */ 0x08, /*<U0008> (CC) Backspace*/
/* 09 */ 0x09, /*<U0009> (CC) Horizontal Tabulation*/
/* 0A */ 0x0A, /*<U000A> (CC) Line Feed*/
/* 0B */ 0x0B, /*<U000B> (CC) Vertical Tabulation*/
/* 0C */ 0x0C, /*<U000C> (CC) Form Feed*/
/* 0D */ 0x0D, /*<U000D> (CC) Carriage Return*/
/* 0E */ 0x0E, /*<U000E> (CC) Locking Shift One (Shift Out*/
/* 0F */ 0x0F, /*<U000F> (CC) Locking Shift Zero (Shift In*/
/* 10 */ 0x10, /*<U0010> (CC) Data Link Escape*/
/* 11 */ 0x11, /*<U0011> (CC) Device Control One*/
/* 12 */ 0x12, /*<U0012> (CC) Device Control Two*/
/* 13 */ 0x13, /*<U0013> (CC) Device Control Three*/
/* 14 */ 0x14, /*<U0014> (CC) Device Control Four*/
/* 15 */ 0x15, /*<U0015> (CC) Negative Acknowledge*/
/* 16 */ 0x16, /*<U0016> (CC) Synchronous Idle*/
/* 17 */ 0x17, /*<U0017> (CC) End of Transmission Block*/
/* 18 */ 0x18, /*<U0018> (CC) Cancel*/
/* 19 */ 0x19, /*<U0019> (CC) End of Medium*/
/* 1A */ 0x1A, /*<U001A> (CC) Substitute*/
/* 1B */ 0x1B, /*<U001B> (CC) Escape*/
/* 1C */ 0x1C, /*<U001C> (CC) Information File Separator*/
/* 1D */ 0x1D, /*<U001D> (CC) Group Separator*/
/* 1E */ 0x1E, /*<U001E> (CC) Record Separator*/
/* 1F */ 0x1F, /*<U001F> (CC) Unit Separator*/
/* 20 */ 0x20, /*<U0020> SPACE*/
/* 21 */ 0x21, /*<U0021> EXCLAMATION MARK*/
/* 22 */ 0x22, /*<U0022> QUOTATION MARK*/
/* 23 */ 0x23, /*<U0023> NUMBER SIGN*/
/* 24 */ 0x24, /*<U0024> DOLLAR SIGN*/
/* 25 */ 0x25, /*<U0025> PERCENT SIGN*/
/* 26 */ 0x26, /*<U0026> AMPERSAND*/
/* 27 */ 0x27, /*<U0027> APOSTROPHE*/
/* 28 */ 0x29, /*<U0028> LEFT PARENTHESIS*/
/* 29 */ 0x28, /*<U0029> RIGHT PARENTHESIS*/
/* 2A */ 0x2A, /*<U002A> ASTERISK*/
/* 2B */ 0x2B, /*<U002B> PLUS SIGN*/
/* 2C */ 0x2C, /*<U002C> COMMA*/
/* 2D */ 0x2D, /*<U002D> HYPHEN-MINUS*/
/* 2E */ 0x2E, /*<U002E> FULL STOP*/
/* 2F */ 0x2F, /*<U002F> SOLIDUS*/
/* 30 */ 0x30, /*<U0030> DIGIT ZERO*/
/* 31 */ 0x31, /*<U0031> DIGIT ONE*/
/* 32 */ 0x32, /*<U0032> DIGIT TWO*/
/* 33 */ 0x33, /*<U0033> DIGIT THREE*/
/* 34 */ 0x34, /*<U0034> DIGIT FOUR*/
/* 35 */ 0x35, /*<U0035> DIGIT FIVE*/
/* 36 */ 0x36, /*<U0036> DIGIT SIX*/
/* 37 */ 0x37, /*<U0037> DIGIT SEVEN*/
/* 38 */ 0x38, /*<U0038> DIGIT EIGHT*/
/* 39 */ 0x39, /*<U0039> DIGIT NINE*/
/* 3A */ 0x3A, /*<U003A> COLON*/
/* 3B */ 0x3B, /*<U003B> SEMICOLON*/
/* 3C */ 0x3E, /*<U003C> LESS-THAN SIGN*/
/* 3D */ 0x3D, /*<U003D> EQUALS SIGN*/
/* 3E */ 0x3C, /*<U003E> GREATER-THAN SIGN*/
/* 3F */ 0x3F, /*<U003F> QUESTION MARK*/
/* 40 */ 0x40, /*<U0040> COMMERCIAL AT*/
/* 41 */ 0x41, /*<U0041> LATIN CAPITAL LETTER A*/
/* 42 */ 0x42, /*<U0042> LATIN CAPITAL LETTER B*/
/* 43 */ 0x43, /*<U0043> LATIN CAPITAL LETTER C*/
/* 44 */ 0x44, /*<U0044> LATIN CAPITAL LETTER D*/
/* 45 */ 0x45, /*<U0045> LATIN CAPITAL LETTER E*/
/* 46 */ 0x46, /*<U0046> LATIN CAPITAL LETTER F*/
/* 47 */ 0x47, /*<U0047> LATIN CAPITAL LETTER G*/
/* 48 */ 0x48, /*<U0048> LATIN CAPITAL LETTER H*/
/* 49 */ 0x49, /*<U0049> LATIN CAPITAL LETTER I*/
/* 4A */ 0x4A, /*<U004A> LATIN CAPITAL LETTER J*/
/* 4B */ 0x4B, /*<U004B> LATIN CAPITAL LETTER K*/
/* 4C */ 0x4C, /*<U004C> LATIN CAPITAL LETTER L*/
/* 4D */ 0x4D, /*<U004D> LATIN CAPITAL LETTER M*/
/* 4E */ 0x4E, /*<U004E> LATIN CAPITAL LETTER N*/
/* 4F */ 0x4F, /*<U004F> LATIN CAPITAL LETTER O*/
/* 50 */ 0x50, /*<U0050> LATIN CAPITAL LETTER P*/
/* 51 */ 0x51, /*<U0051> LATIN CAPITAL LETTER Q*/
/* 52 */ 0x52, /*<U0052> LATIN CAPITAL LETTER R*/
/* 53 */ 0x53, /*<U0053> LATIN CAPITAL LETTER S*/
/* 54 */ 0x54, /*<U0054> LATIN CAPITAL LETTER T*/
/* 55 */ 0x55, /*<U0055> LATIN CAPITAL LETTER U*/
/* 56 */ 0x56, /*<U0056> LATIN CAPITAL LETTER V*/
/* 57 */ 0x57, /*<U0057> LATIN CAPITAL LETTER W*/
/* 58 */ 0x58, /*<U0058> LATIN CAPITAL LETTER X*/
/* 59 */ 0x59, /*<U0059> LATIN CAPITAL LETTER Y*/
/* 5A */ 0x5A, /*<U005A> LATIN CAPITAL LETTER Z*/
/* 5B */ 0x5D, /*<U005B> LEFT SQUARE BRACKET*/
/* 5C */ 0x5C, /*<U005C> REVERSE SOLIDUS*/
/* 5D */ 0x5B, /*<U005D> RIGHT SQUARE BRACKET*/
/* 5E */ 0x5E, /*<U005E> CIRCUMFLEX ACCENT*/
/* 5F */ 0x5F, /*<U005F> LOW LINE*/
/* 60 */ 0x60, /*<U0060> GRAVE ACCENT*/
/* 61 */ 0x61, /*<U0061> LATIN SMALL LETTER A*/
/* 62 */ 0x62, /*<U0062> LATIN SMALL LETTER B*/
/* 63 */ 0x63, /*<U0063> LATIN SMALL LETTER C*/
/* 64 */ 0x64, /*<U0064> LATIN SMALL LETTER D*/
/* 65 */ 0x65, /*<U0065> LATIN SMALL LETTER E*/
/* 66 */ 0x66, /*<U0066> LATIN SMALL LETTER F*/
/* 67 */ 0x67, /*<U0067> LATIN SMALL LETTER G*/
/* 68 */ 0x68, /*<U0068> LATIN SMALL LETTER H*/
/* 69 */ 0x69, /*<U0069> LATIN SMALL LETTER I*/
/* 6A */ 0x6A, /*<U006A> LATIN SMALL LETTER J*/
/* 6B */ 0x6B, /*<U006B> LATIN SMALL LETTER K*/
/* 6C */ 0x6C, /*<U006C> LATIN SMALL LETTER L*/
/* 6D */ 0x6D, /*<U006D> LATIN SMALL LETTER M*/
/* 6E */ 0x6E, /*<U006E> LATIN SMALL LETTER N*/
/* 6F */ 0x6F, /*<U006F> LATIN SMALL LETTER O*/
/* 70 */ 0x70, /*<U0070> LATIN SMALL LETTER P*/
/* 71 */ 0x71, /*<U0071> LATIN SMALL LETTER Q*/
/* 72 */ 0x72, /*<U0072> LATIN SMALL LETTER R*/
/* 73 */ 0x73, /*<U0073> LATIN SMALL LETTER S*/
/* 74 */ 0x74, /*<U0074> LATIN SMALL LETTER T*/
/* 75 */ 0x75, /*<U0075> LATIN SMALL LETTER U*/
/* 76 */ 0x76, /*<U0076> LATIN SMALL LETTER V*/
/* 77 */ 0x77, /*<U0077> LATIN SMALL LETTER W*/
/* 78 */ 0x78, /*<U0078> LATIN SMALL LETTER X*/
/* 79 */ 0x79, /*<U0079> LATIN SMALL LETTER Y*/
/* 7A */ 0x7A, /*<U007A> LATIN SMALL LETTER Z*/
/* 7B */ 0x7D, /*<U007B> LEFT CURLY BRACKET*/
/* 7C */ 0x7C, /*<U007C> VERTICAL LINE*/
/* 7D */ 0x7B, /*<U007D> RIGHT CURLY BRACKET*/
/* 7E */ 0x7E, /*<U007E> TILDE*/
/* 7F */ 0x7F, /*(CC) Delete*/
/* 80 */ 0x80, /* *** UNASSIGNED*/
/* 81 */ 0x81, /* *** UNASSIGNED*/
/* 82 */ 0x82, /* *** UNASSIGNED*/
/* 83 */ 0x83, /* *** UNASSIGNED*/
/* 84 */ 0x84, /* *** UNASSIGNED*/
/* 85 */ 0x85, /* *** UNASSIGNED*/
/* 86 */ 0x86, /* *** UNASSIGNED*/
/* 87 */ 0x87, /* *** UNASSIGNED*/
/* 88 */ 0x88, /* *** UNASSIGNED*/
/* 89 */ 0x89, /* *** UNASSIGNED*/
/* 8A */ 0x8A, /* *** UNASSIGNED*/
/* 8B */ 0x8B, /* *** UNASSIGNED*/
/* 8C */ 0x8C, /* *** UNASSIGNED*/
/* 8D */ 0x8D, /* *** UNASSIGNED*/
/* 8E */ 0x8E, /* *** UNASSIGNED*/
/* 8F */ 0x8F, /* *** UNASSIGNED*/
/* 90 */ 0x90, /* *** UNASSIGNED*/
/* 91 */ 0x91, /* *** UNASSIGNED*/
/* 92 */ 0x92, /* *** UNASSIGNED*/
/* 93 */ 0x93, /* *** UNASSIGNED*/
/* 94 */ 0x94, /* *** UNASSIGNED*/
/* 95 */ 0x95, /* *** UNASSIGNED*/
/* 96 */ 0x96, /* *** UNASSIGNED*/
/* 97 */ 0x97, /* *** UNASSIGNED*/
/* 98 */ 0x98, /* *** UNASSIGNED*/
/* 99 */ 0x99, /* *** UNASSIGNED*/
/* 9A */ 0x9A, /* *** UNASSIGNED*/
/* 9B */ 0x9B, /* *** UNASSIGNED*/
/* 9C */ 0x9C, /* *** UNASSIGNED*/
/* 9D */ 0x9D, /* *** UNASSIGNED*/
/* 9E */ 0x9E, /* *** UNASSIGNED*/
/* 9F */ 0x9F, /* *** UNASSIGNED*/
/* A0 */ 0xA0, /*<U00A0> NO-BREAK SPACE*/
/* A1 */ 0xA1, /* *** UNASSIGNED*/
/* A2 */ 0xA2, /*<U00A2> CENT SIGN*/
/* A3 */ 0xA3, /*<U00A3> POUND SIGN*/
/* A4 */ 0xA4, /*<U00A4> CURRENCY SIGN*/
/* A5 */ 0xA5, /*<U00A5> YEN SIGN*/
/* A6 */ 0xA6, /*<U00A6> BROKEN BAR*/
/* A7 */ 0xA7, /*<U00A7> SECTION SIGN*/
/* A8 */ 0xA8, /*<U00A8> DIAERESIS*/
/* A9 */ 0xA9, /*<U00A9> COPYRIGHT SIGN*/
/* AA */ 0xAA, /*<U00D7> MULTIPLICATION SIGN*/
/* AB */ 0xBB, /*<U00AB> LEFT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* AC */ 0xAC, /*<U00AC> NOT SIGN*/
/* AD */ 0xAD, /*<U00AD> SOFT HYPHEN*/
/* AE */ 0xAE, /*<U00AE> REGISTERED SIGN*/
/* AF */ 0xAF, /*<U203E> MACRON*/
/* B0 */ 0xB0, /*<U00B0> DEGREE SIGN*/
/* B1 */ 0xB1, /*<U00B1> PLUS-MINUS SIGN*/
/* B2 */ 0xB2, /*<U00B2> SUPERSCRIPT TWO*/
/* B3 */ 0xB3, /*<U00B3> SUPERSCRIPT THREE*/
/* B4 */ 0xB4, /*<U00B4> ACUTE ACCENT*/
/* B5 */ 0xB5, /*<U00B5> MICRO SIGN*/
/* B6 */ 0xB6, /*<U00B6> PILCROW SIGN*/
/* B7 */ 0xB7, /*<U00B7> MIDDLE DOT*/
/* B8 */ 0xB8, /*<U00B8> CEDILLA*/
/* B9 */ 0xB9, /*<U00B9> SUPERSCRIPT ONE*/
/* BA */ 0xBA, /*<U00F7> DIVISION SIGN*/
/* BB */ 0xAB, /*<U00BB> RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* BC */ 0xBC, /*<U00BC> VULGAR FRACTION ONE QUARTER*/
/* BD */ 0xBD, /*<U00BD> VULGAR FRACTION ONE HALF*/
/* BE */ 0xBE, /*<U00BE> VULGAR FRACTION THREE QUARTERS*/
/* BF */ 0xBF, /* ***UNASSIGNED*/
/* C0 */ 0xC0, /* ***UNASSIGNED*/
/* C1 */ 0xC1, /* ***UNASSIGNED*/
/* C2 */ 0xC2, /* ***UNASSIGNED*/
/* C3 */ 0xC3, /* ***UNASSIGNED*/
/* C4 */ 0xC4, /* ***UNASSIGNED*/
/* C5 */ 0xC5, /* ***UNASSIGNED*/
/* C6 */ 0xC6, /* ***UNASSIGNED*/
/* C7 */ 0xC7, /* ***UNASSIGNED*/
/* C8 */ 0xC8, /* ***UNASSIGNED*/
/* C9 */ 0xC9, /* ***UNASSIGNED*/
/* CA */ 0xCA, /* ***UNASSIGNED*/
/* CB */ 0xCB, /* ***UNASSIGNED*/
/* CC */ 0xCC, /* ***UNASSIGNED*/
/* CD */ 0xCD, /* ***UNASSIGNED*/
/* CE */ 0xCE, /* ***UNASSIGNED*/
/* CF */ 0xCF, /* ***UNASSIGNED*/
/* D0 */ 0xD0, /* ***UNASSIGNED*/
/* D1 */ 0xD1, /* ***UNASSIGNED*/
/* D2 */ 0xD2, /* ***UNASSIGNED*/
/* D3 */ 0xD3, /* ***UNASSIGNED*/
/* D4 */ 0xD4, /* ***UNASSIGNED*/
/* D5 */ 0xD5, /* ***UNASSIGNED*/
/* D6 */ 0xD6, /* ***UNASSIGNED*/
/* D7 */ 0xD7, /* ***UNASSIGNED*/
/* D8 */ 0xD8, /* ***UNASSIGNED*/
/* D9 */ 0xD9, /* ***UNASSIGNED*/
/* DA */ 0xDA, /* ***UNASSIGNED*/
/* DB */ 0xDB, /* ***UNASSIGNED*/
/* DC */ 0xDC, /* ***UNASSIGNED*/
/* DD */ 0xDD, /* ***UNASSIGNED*/
/* DE */ 0xDE, /* ***UNASSIGNED*/
/* DF */ 0xDF, /*<U2017> DOUBLE LOW LINE*/
/* E0 */ 0xE0, /*<U05D0> HEBREW LETTER ALEF*/
/* E1 */ 0xE1, /*<U05D1> HEBREW LETTER BEIT*/
/* E2 */ 0xE2, /*<U05D2> HEBREW LETTER GIMMEL*/
/* E3 */ 0xE3, /*<U05D3> HEBREW LETTER DALET*/
/* E4 */ 0xE4, /*<U05D4> HEBREW LETTER HE*/
/* E5 */ 0xE5, /*<U05D5> HEBREW LETTER VAV*/
/* E6 */ 0xE6, /*<U05D6> HEBREW LETTER ZAIN*/
/* E7 */ 0xE7, /*<U05D7> HEBREW LETTER HET*/
/* E8 */ 0xE8, /*<U05D8> HEBREW LETTER TET*/
/* E9 */ 0xE9, /*<U05D9> HEBREW LETTER YOD*/
/* EA */ 0xEA, /*<U05DA> HEBREW LETTER FINAL KAF*/
/* EB */ 0xEB, /*<U05DB> HEBREW LETTER MEDIAL KAF*/
/* EC */ 0xEC, /*<U05DC> HEBREW LETTER LAMED*/
/* ED */ 0xED, /*<U05DD> HEBREW LETTER FINAL MEM*/
/* EE */ 0xEE, /*<U05DE> HEBREW LETTER MEDIAL MEM*/
/* EF */ 0xEF, /*<U05DF> HEBREW LETTER FINAL NUN*/
/* F0 */ 0xF0, /*<U05E0> HEBREW LETTER MEDIAL NUN*/
/* F1 */ 0xF1, /*<U05E1> HEBREW LETTER SAMECH*/
/* F2 */ 0xF2, /*<U05E2> HEBREW LETTER AIN*/
/* F3 */ 0xF3, /*<U05E3> HEBREW LETTER FINAL PE*/
/* F4 */ 0xF4, /*<U05E4> HEBREW LETTER MEDIAL PE*/
/* F5 */ 0xF5, /*<U05E5> HEBREW LETTER FINAL TZADDI*/
/* F6 */ 0xF6, /*<U05E6> HEBREW LETTER MEDIAL TZADDI*/
/* F7 */ 0xF7, /*<U05E7> HEBREW LETTER QUF*/
/* F8 */ 0xF8, /*<U05E8> HEBREW LETTER RESH*/
/* F9 */ 0xF9, /*<U05E9> HEBREW LETTER SHIN*/
/* FA */ 0xFA, /*<U05EA> HEBREW LETTER TAV*/
/* FB */ 0xFB, /* *** UNASSIGNED*/
/* FC */ 0xFC, /* *** UNASSIGNED*/
/* FD */ 0xFD, /* *** UNASSIGNED*/
/* FE */ 0xFE, /* *** UNASSIGNED*/
/* FF */ 0xFF  /* *** UNASSIGNED*/
  },
  /* 1255 Windows Hebrew*/
{
/* 00 */ 0x00, /*(CC) Null*/
/* 01 */ 0x01, /*(CC) Start of Heading*/
/* 02 */ 0x02, /*(CC) Start of Text*/
/* 03 */ 0x03, /*(CC) End of Text*/
/* 04 */ 0x04, /*(CC) End of Transmission*/
/* 05 */ 0x05, /*(CC) Enquiry*/
/* 06 */ 0x06, /*(CC) Acknowledge*/
/* 07 */ 0x07, /*(CC) Bell*/
/* 08 */ 0x08, /*(CC) Backspace*/
/* 09 */ 0x09, /*(CC) Horizontal Tabulation*/
/* 0A */ 0x0A, /*(CC) Line Feed*/
/* 0B */ 0x0B, /*(CC) Vertical Tabulation*/
/* 0C */ 0x0C, /*(CC) Form Feed*/
/* 0D */ 0x0D, /*(CC) Carriage Return*/
/* 0E */ 0x0E, /*(CC) Locking Shift One (Shift Out*/
/* 0F */ 0x0F, /*(CC) Locking Shift Zero (Shift In*/
/* 10 */ 0x10, /*(CC) Data Link Escape*/
/* 11 */ 0x11, /*(CC) Device Control One*/
/* 12 */ 0x12, /*(CC) Device Control Two*/
/* 13 */ 0x13, /*(CC) Device Control Three*/
/* 14 */ 0x14, /*(CC) Device Control Four*/
/* 15 */ 0x15, /*(CC) Negative Acknowledge*/
/* 16 */ 0x16, /*(CC) Synchronous Idle*/
/* 17 */ 0x17, /*(CC) End of Transmission Block*/
/* 18 */ 0x18, /*(CC) Cancel*/
/* 19 */ 0x19, /*(CC) End of Medium*/
/* 1A */ 0x1A, /*(CC) Substitute*/
/* 1B */ 0x1B, /*(CC) Escape*/
/* 1C */ 0x1C, /*(CC) Information File Separator*/
/* 1D */ 0x1D, /*(CC) Group Separator*/
/* 1E */ 0x1E, /*(CC) Record Separator*/
/* 1F */ 0x1F, /*(CC) Unit Separator*/
/* 20 */ 0x20, /*SPACE*/
/* 21 */ 0x21, /*EXCLAMATION MARK*/
/* 22 */ 0x22, /*QUOTATION MARK*/
/* 23 */ 0x23, /*NUMBER SIGN*/
/* 24 */ 0x24, /*DOLLAR SIGN*/
/* 25 */ 0x25, /*PERCENT SIGN*/
/* 26 */ 0x26, /*AMPERSAND*/
/* 27 */ 0x27, /*APOSTROPHE*/
/* 28 */ 0x29, /*LEFT PARENTHESIS*/
/* 29 */ 0x28, /*RIGHT PARENTHESIS*/
/* 2A */ 0x2A, /*ASTERISK*/
/* 2B */ 0x2B, /*PLUS SIGN*/
/* 2C */ 0x2C, /*COMMA*/
/* 2D */ 0x2D, /*HYPHEN-MINUS*/
/* 2E */ 0x2E, /*FULL STOP*/
/* 2F */ 0x2F, /*SOLIDUS*/
/* 30 */ 0x30, /*DIGIT ZERO*/
/* 31 */ 0x31, /*DIGIT ONE*/
/* 32 */ 0x32, /*DIGIT TWO*/
/* 33 */ 0x33, /*DIGIT THREE*/
/* 34 */ 0x34, /*DIGIT FOUR*/
/* 35 */ 0x35, /*DIGIT FIVE*/
/* 36 */ 0x36, /*DIGIT SIX*/
/* 37 */ 0x37, /*DIGIT SEVEN*/
/* 38 */ 0x38, /*DIGIT EIGHT*/
/* 39 */ 0x39, /*DIGIT NINE*/
/* 3A */ 0x3A, /*COLON*/
/* 3B */ 0x3B, /*SEMICOLON*/
/* 3C */ 0x3E, /*LESS-THAN SIGN*/
/* 3D */ 0x3D, /*EQUALS SIGN*/
/* 3E */ 0x3C, /*GREATER-THAN SIGN*/
/* 3F */ 0x3F, /*QUESTION MARK*/
/* 40 */ 0x40, /*COMMERCIAL AT*/
/* 41 */ 0x41, /*LATIN CAPITAL LETTER A*/
/* 42 */ 0x42, /*LATIN CAPITAL LETTER B*/
/* 43 */ 0x43, /*LATIN CAPITAL LETTER C*/
/* 44 */ 0x44, /*LATIN CAPITAL LETTER D*/
/* 45 */ 0x45, /*LATIN CAPITAL LETTER E*/
/* 46 */ 0x46, /*LATIN CAPITAL LETTER F*/
/* 47 */ 0x47, /*LATIN CAPITAL LETTER G*/
/* 48 */ 0x48, /*LATIN CAPITAL LETTER H*/
/* 49 */ 0x49, /*LATIN CAPITAL LETTER I*/
/* 4A */ 0x4A, /*LATIN CAPITAL LETTER J*/
/* 4B */ 0x4B, /*LATIN CAPITAL LETTER K*/
/* 4C */ 0x4C, /*LATIN CAPITAL LETTER L*/
/* 4D */ 0x4D, /*LATIN CAPITAL LETTER M*/
/* 4E */ 0x4E, /*LATIN CAPITAL LETTER N*/
/* 4F */ 0x4F, /*LATIN CAPITAL LETTER O*/
/* 50 */ 0x50, /*LATIN CAPITAL LETTER P*/
/* 51 */ 0x51, /*LATIN CAPITAL LETTER Q*/
/* 52 */ 0x52, /*LATIN CAPITAL LETTER R*/
/* 53 */ 0x53, /*LATIN CAPITAL LETTER S*/
/* 54 */ 0x54, /*LATIN CAPITAL LETTER T*/
/* 55 */ 0x55, /*LATIN CAPITAL LETTER U*/
/* 56 */ 0x56, /*LATIN CAPITAL LETTER V*/
/* 57 */ 0x57, /*LATIN CAPITAL LETTER W*/
/* 58 */ 0x58, /*LATIN CAPITAL LETTER X*/
/* 59 */ 0x59, /*LATIN CAPITAL LETTER Y*/
/* 5A */ 0x5A, /*LATIN CAPITAL LETTER Z*/
/* 5B */ 0x5D, /*LEFT SQUARE BRACKET*/
/* 5C */ 0x5C, /*REVERSE SOLIDUS*/
/* 5D */ 0x5B, /*RIGHT SQUARE BRACKET*/
/* 5E */ 0x5E, /*CIRCUMFLEX ACCENT*/
/* 5F */ 0x5F, /*LOW LINE*/
/* 60 */ 0x60, /*GRAVE ACCENT*/
/* 61 */ 0x61, /*LATIN SMALL LETTER A*/
/* 62 */ 0x62, /*LATIN SMALL LETTER B*/
/* 63 */ 0x63, /*LATIN SMALL LETTER C*/
/* 64 */ 0x64, /*LATIN SMALL LETTER D*/
/* 65 */ 0x65, /*LATIN SMALL LETTER E*/
/* 66 */ 0x66, /*LATIN SMALL LETTER F*/
/* 67 */ 0x67, /*LATIN SMALL LETTER G*/
/* 68 */ 0x68, /*LATIN SMALL LETTER H*/
/* 69 */ 0x69, /*LATIN SMALL LETTER I*/
/* 6A */ 0x6A, /*LATIN SMALL LETTER J*/
/* 6B */ 0x6B, /*LATIN SMALL LETTER K*/
/* 6C */ 0x6C, /*LATIN SMALL LETTER L*/
/* 6D */ 0x6D, /*LATIN SMALL LETTER M*/
/* 6E */ 0x6E, /*LATIN SMALL LETTER N*/
/* 6F */ 0x6F, /*LATIN SMALL LETTER O*/
/* 70 */ 0x70, /*LATIN SMALL LETTER P*/
/* 71 */ 0x71, /*LATIN SMALL LETTER Q*/
/* 72 */ 0x72, /*LATIN SMALL LETTER R*/
/* 73 */ 0x73, /*LATIN SMALL LETTER S*/
/* 74 */ 0x74, /*LATIN SMALL LETTER T*/
/* 75 */ 0x75, /*LATIN SMALL LETTER U*/
/* 76 */ 0x76, /*LATIN SMALL LETTER V*/
/* 77 */ 0x77, /*LATIN SMALL LETTER W*/
/* 78 */ 0x78, /*LATIN SMALL LETTER X*/
/* 79 */ 0x79, /*LATIN SMALL LETTER Y*/
/* 7A */ 0x7A, /*LATIN SMALL LETTER Z*/
/* 7B */ 0x7D, /*LEFT CURLY BRACKET*/
/* 7C */ 0x7C, /*VERTICAL LINE*/
/* 7D */ 0x7B, /*RIGHT CURLY BRACKET*/
/* 7E */ 0x7E, /*TILDE*/
/* 7F */ 0x7F, /*(CC) Delete*/
/* 80 */ 0x80, /* *** UNASSIGNED*/
/* 81 */ 0x81, /* *** UNASSIGNED*/
/* 82 */ 0x82, /*SINGLE LOW-9 QUOTATION MARK*/
/* 83 */ 0x83, /*LATIN SMALL LETTER F WITH HOOK*/
/* 84 */ 0x84, /*DOUBLE LOW-9 QUOTATION MARK*/
/* 85 */ 0x85, /*HORIZONTAL ELLIPSIS*/
/* 86 */ 0x86, /*DAGGER*/
/* 87 */ 0x87, /*DOUBLE DAGGER*/
/* 88 */ 0x88, /*MODIFIER LETTER CIRCUMFLEX ACCENT*/
/* 89 */ 0x89, /*PER MILLE SIGN*/
/* 8A */ 0x8A, /*(CC) Line Tabultion Set*/
/* 8B */ 0x8B, /*SINGLE LEFT-POINTING ANGLE QUOTATION MARK*/
/* 8C */ 0x8C, /* *** UNASSIGNED*/
/* 8D */ 0x8D, /* *** UNASSIGNED*/
/* 8E */ 0x8E, /* *** UNASSIGNED*/
/* 8F */ 0x8F, /* *** UNASSIGNED*/
/* 90 */ 0x90, /* *** UNASSIGNED*/
/* 91 */ 0x91, /*LEFT SINGLE QUOTATION MARK*/
/* 92 */ 0x92, /*RIGHT SINGLE QUOTATION MARK*/
/* 93 */ 0x93, /*LEFT DOUBLE QUOTATION MARK*/
/* 94 */ 0x94, /*RIGHT DOUBLE QUOTATION MARK*/
/* 95 */ 0x95, /*BULLET*/
/* 96 */ 0x96, /*EN DASH*/
/* 97 */ 0x97, /*EM DASH*/
/* 98 */ 0x98, /*TILDE*/
/* 99 */ 0x99, /*TRADE MARK SIGN*/
/* 9A */ 0x9A, /* *** UNASSIGNED*/
/* 9B */ 0x9B, /*SINGLE RIGHT-POINTING ANGLE QUOTATION MARK*/
/* 9C */ 0x9C, /* *** UNASSIGNED*/
/* 9D */ 0x9D, /* *** UNASSIGNED*/
/* 9E */ 0x9E, /* *** UNASSIGNED*/
/* 9F */ 0x9F, /* *** UNASSIGNED*/
/* A0 */ 0xA0, /*NO-BREAK SPACE*/
/* A1 */ 0xA1, /* *** UNASSIGNED*/
/* A2 */ 0xA2, /*CENT SIGN*/
/* A3 */ 0xA3, /*POUND SIGN*/
/* A4 */ 0xA4, /*NEW SHEKEL SIGN*/
/* A5 */ 0xA5, /*YEN SIGN*/
/* A6 */ 0xA6, /*BROKEN BAR*/
/* A7 */ 0xA7, /*SECTION SIGN*/
/* A8 */ 0xA8, /*DIAERESIS*/
/* A9 */ 0xA9, /*COPYRIGHT SIGN*/
/* AA */ 0xAA, /* ***UNASSIGNED*/
/* AB */ 0xBB, /*LEFT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* AC */ 0xAC, /*NOT SIGN*/
/* AD */ 0xAD, /*SOFT HYPHEN*/
/* AE */ 0xAE, /*REGISTERED SIGN*/
/* AF */ 0xAF, /*MACRON*/
/* B0 */ 0xB0, /*DEGREE SIGN*/
/* B1 */ 0xB1, /*PLUS-MINUS SIGN*/
/* B2 */ 0xB2, /*SUPERSCRIPT TWO*/
/* B3 */ 0xB3, /*SUPERSCRIPT THREE*/
/* B4 */ 0xB4, /*ACUTE ACCENT*/
/* B5 */ 0xB5, /*MICRO SIGN*/
/* B6 */ 0xB6, /*PILCROW SIGN*/
/* B7 */ 0xB7, /*MIDDLE DOT*/
/* B8 */ 0xB8, /* *** UNASSIGNED*/
/* B9 */ 0xB9, /*SUPERSCRIPT ONE*/
/* BA */ 0xBA, /* *** UNASSIGNED*/
/* BB */ 0xAB, /*RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK*/
/* BC */ 0xBC, /*VULGAR FRACTION ONE QUARTER*/
/* BD */ 0xBD, /*VULGAR FRACTION ONE HALF*/
/* BE */ 0xBE, /*VULGAR FRACTION THREE QUARTERS*/
/* BF */ 0xBF, /* *** UNASSIGNED*/
/* C0 */ 0xC0, /*HEBREW POINT SHEVA*/
/* C1 */ 0xC1, /*HEBREW POINT HATAF SEGOL*/
/* C2 */ 0xC2, /*HEBREW POINT HATAF PATAH*/
/* C3 */ 0xC3, /*HEBREW POINT HATAF KAMATS*/
/* C4 */ 0xC4, /*HEBREW POINT HIRIQ*/
/* C5 */ 0xC5, /*HEBREW POINT TSERE*/
/* C6 */ 0xC6, /*HEBREW POINT SEGOL*/
/* C7 */ 0xC7, /*HEBREW POINT PATAH*/
/* C8 */ 0xC8, /*HEBREW POINT QAMATS*/
/* C9 */ 0xC9, /*HEBREW POINT HOLAM*/
/* CA */ 0xCA, /* *** UNASSIGNED*/
/* CB */ 0xCB, /*HEBREW POINT QUBUTS*/
/* CC */ 0xCC, /*HEBREW POINT DAGESH OR MAPPIQ*/
/* CD */ 0xCD, /*HEBREW POINT METEG*/
/* CE */ 0xCE, /*HEBREW PUNCTUATION METEG*/
/* CF */ 0xCF, /*HEBREW PUNCTUATION RAFE*/
/* D0 */ 0xD0, /*HEBREW PUNCTUATION PASEQ*/
/* D1 */ 0xD1, /*HEBREW POINT SHIN DOT*/
/* D2 */ 0xD2, /*HEBREW POINT SIN DOT*/
/* D3 */ 0xD3, /*HEBREW PUNCTUATION SOF PASUK*/
/* D4 */ 0xD4, /*HEBREW LIGATURE YIDDISH DOUBLE VAV*/
/* D5 */ 0xD5, /*HEBREW LIGATURE YIDDISH VAV YOD*/
/* D6 */ 0xD6, /*HEBREW LIGATURE YIDDISH DOUBLE YOD*/
/* D7 */ 0xD7, /* *** UNASSIGNED*/
/* D8 */ 0xD8, /* *** UNASSIGNED*/
/* D9 */ 0xD9, /* *** UNASSIGNED*/
/* DA */ 0xDA, /* *** UNASSIGNED*/
/* DB */ 0xDB, /* *** UNASSIGNED*/
/* DC */ 0xDC, /* *** UNASSIGNED*/
/* DD */ 0xDD, /* *** UNASSIGNED*/
/* DE */ 0xDE, /* *** UNASSIGNED*/
/* DF */ 0xDF, /* *** UNASSIGNED*/
/* E0 */ 0xE0, /*HEBREW LETTER ALEF*/
/* E1 */ 0xE1, /*HEBREW LETTER BEIT*/
/* E2 */ 0xE2, /*HEBREW LETTER GIMMEL*/
/* E3 */ 0xE3, /*HEBREW LETTER DALET*/
/* E4 */ 0xE4, /*HEBREW LETTER HE*/
/* E5 */ 0xE5, /*HEBREW LETTER VAV*/
/* E6 */ 0xE6, /*HEBREW LETTER ZAIN*/
/* E7 */ 0xE7, /*HEBREW LETTER HET*/
/* E8 */ 0xE8, /*HEBREW LETTER TET*/
/* E9 */ 0xE9, /*HEBREW LETTER YOD*/
/* EA */ 0xEA, /*HEBREW LETTER FINAL KAF*/
/* EB */ 0xEB, /*HEBREW LETTER MEDIAL KAF*/
/* EC */ 0xEC, /*HEBREW LETTER LAMED*/
/* ED */ 0xED, /*HEBREW LETTER FINAL MEM*/
/* EE */ 0xEE, /*HEBREW LETTER MEDIAL MEM*/
/* EF */ 0xEF, /*HEBREW LETTER FINAL NUN*/
/* F0 */ 0xF0, /*HEBREW LETTER MEDIAL NUN*/
/* F1 */ 0xF1, /*HEBREW LETTER SAMECH*/
/* F2 */ 0xF2, /*HEBREW LETTER AIN*/
/* F3 */ 0xF3, /*HEBREW LETTER FINAL PE*/
/* F4 */ 0xF4, /*HEBREW LETTER MEDIAL PE*/
/* F5 */ 0xF5, /*HEBREW LETTER FINAL TZADDI*/
/* F6 */ 0xF6, /*HEBREW LETTER MEDIAL TZADDI*/
/* F7 */ 0xF7, /*HEBREW LETTER QUF*/
/* F8 */ 0xF8, /*HEBREW LETTER RESH*/
/* F9 */ 0xF9, /*HEBREW LETTER SHIN*/
/* FA */ 0xFA, /*HEBREW LETTER TAV*/
/* FB */ 0xFB, /* *** UNASSIGNED*/
/* FC */ 0xFC, /* *** UNASSIGNED*/
/* FD */ 0xFD, /*LEFT-TO-RIGHT MARK*/
/* FE */ 0xFE, /*RIGHT-TO-LEFT MARK*/
/* FF */ 0xFF
}
};


/* Mapping Arrays */
static unsigned char From1046[2] [256]=
{
  /* From 1046 to 420*/
  { /*   0     1,    2,    3,    4,    5,    6,    7,    8,    9,    a,    b,    c,    d,    e,    f */
/* 0 */ 0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f, 0x16, 0x05, 0x25, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
/* 1 */ 0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26, 0x18, 0x19, 0x1c, 0x27, 0x07, 0x1d, 0x1e, 0x1f,
/* 2 */ 0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d, 0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,
/* 3 */ 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f,
/* 4 */ 0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
/* 5 */ 0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0x04, 0x06, 0x08, 0x5f, 0x6d,
/* 6 */ 0x14, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
/* 7 */ 0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0x15, 0x4f, 0x17, 0x1a, 0x3f,
/* 8 */ 0x57, 0xe0, 0xa1, 0x77, 0x80, 0x8b, 0x8d, 0x44, 0x22, 0x23, 0x24, 0x28, 0x29, 0x2b, 0x2c, 0xe1,
/* 9 */ 0x44, 0x44, 0x43, 0x44, 0x44, 0x55, 0xdb, 0xde, 0xdd, 0x9f, 0xa0, 0xaa, 0xb3, 0xb5, 0xb9, 0xb9,
/* a */ 0x41, 0x47, 0x49, 0x56, 0x3e, 0x56, 0x55, 0x59, 0x64, 0x66, 0x68, 0x70, 0x79, 0xca, 0x72, 0x78,
/* b */ 0xdf, 0xea, 0xeb, 0xed, 0xee, 0xef, 0xfb, 0xfc, 0xfd, 0xfe, 0x8a, 0xc0, 0x8c, 0x8e, 0x9b, 0xd0,
/* c */ 0x9c, 0x46, 0x47, 0x49, 0x52, 0x56, 0x55, 0x56, 0x58, 0x62, 0x63, 0x65, 0x67, 0x69, 0x71, 0x73,
/* d */ 0x74, 0x75, 0x76, 0x77, 0x80, 0x8b, 0x8d, 0x8f, 0x90, 0x9a, 0x9e, 0x9d, 0x48, 0x51, 0x57, 0xac,
/* e */ 0x44, 0xab, 0xad, 0xaf, 0xb1, 0xbb, 0xbd, 0xcb, 0xcf, 0xda, 0xdc, 0x40, 0x40, 0x40, 0x40, 0x40,
/* f */ 0x40, 0x42, 0x40, 0xae, 0xb0, 0xba, 0x45, 0xb2, 0xb4, 0xb8, 0xb8, 0xbc, 0xbe, 0xcd, 0xbf, 0x4a
  },
  /* From1046 To 864*/
  { /*   0     1,    2,    3,    4,    5,    6,    7,    8,    9,    a,    b,    c,    d,    e,    f */
/* 0 */ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
/* 1 */ 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1B, 0x1c, 0x1D, 0x1E, 0x1F,
/* 2 */ 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
/* 3 */ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
/* 4 */ 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
/* 5 */ 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
/* 6 */ 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
/* 7 */ 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7f,
/* 8 */ 0xa8, 0xde, 0xdd, 0xbc, 0xbd, 0xbe, 0xeb, 0xe0, 0xc0, 0xfe, 0x86, 0x85, 0x8c, 0x8d, 0x8e, 0x8f,
/* 9 */ 0xe0, 0xe0, 0xf0, 0xe0, 0xe0, 0xc6, 0xf5, 0xea, 0xf6, 0xed, 0xda, 0xf7, 0xfa, 0x9a, 0x9e, 0x9e,
/* a */ 0xa0, 0xc2, 0xc3, 0xc7, 0xa4, 0xc7, 0xc6, 0xc8, 0xca, 0xcb, 0xcc, 0xcd, 0xac, 0xa1, 0xce, 0xd3,
/* b */ 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xd4, 0xbb, 0xd5, 0xd6, 0xc5, 0xbf,
/* c */ 0xd9, 0xc1, 0xc2, 0xc3, 0xc4, 0xc7, 0xc6, 0xc7, 0xa9, 0xc9, 0xaa, 0xab, 0xad, 0xae, 0xaf, 0xcf,
/* d */ 0xd0, 0xd1, 0xd2, 0xbc, 0xbd, 0xbe, 0xeb, 0xd7, 0xd8, 0xdf, 0xee, 0xec, 0xa2, 0xa5, 0xa8, 0xe1,
/* e */ 0xe0, 0xba, 0xf8, 0xfc, 0xfb, 0xef, 0xf2, 0xe7, 0xe8, 0xe9, 0xfd, 0x20, 0x20, 0x20, 0x20, 0x20,
/* f */ 0x20, 0xf1, 0x20, 0xe2, 0xe3, 0xe4, 0x9f, 0xf9, 0x99, 0x9d, 0x9d, 0xe5, 0xe6, 0xf4, 0xf3, 0xff
  }
};

static unsigned char To1046[2] [256]=
{
  /* From 420 to 1046*/
   {  /*   0     1,    2,    3,    4,    5,    6,    7,    8,    9,    a,    b,    c,    d,    e,    f */
/* 0 */ 0x00, 0x01, 0x02, 0x03, 0x5b, 0x09, 0x5c, 0x1c, 0x5d, 0x7f, 0x7f, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
/* 1 */ 0x10, 0x11, 0x12, 0x13, 0x60, 0x7b, 0x08, 0x7d, 0x18, 0x19, 0x7e, 0x7f, 0x1a, 0x1D, 0x1E, 0x1F,
/* 2 */ 0x7f, 0x7f, 0x88, 0x89, 0x8a, 0x0A, 0x17, 0x1B, 0x8b, 0x8c, 0x7f, 0x8d, 0x8e, 0x05, 0x06, 0x07,
/* 3 */ 0x7f, 0x7f, 0x16, 0x7f, 0x7f, 0x7f, 0x7f, 0x04, 0x7f, 0x7f, 0x7f, 0x7f, 0x14, 0x15, 0xa4, 0x7f,
/* 4 */ 0x20, 0xa0, 0xF1, 0x92, 0xE0, 0xF6, 0xC1, 0xC2, 0xDC, 0xC3, 0xff, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
/* 5 */ 0x26, 0xDD, 0xC4, 0x7f, 0x7f, 0xA6, 0xC7, 0xDE, 0xC8, 0xA7, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
/* 6 */ 0x2D, 0x2F, 0xC9, 0xCA, 0xA8, 0xCB, 0xA9, 0xCC, 0xAA, 0xCD, 0x7c, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
/* 7 */ 0xAB, 0xCE, 0xAE, 0xCF, 0xD0, 0xD1, 0xD2, 0x83, 0xAF, 0xAC, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
/* 8 */ 0x84, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xBA, 0x85, 0xBC, 0x86, 0xBD, 0xD7,
/* 9 */ 0xD8, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xD9, 0xBE, 0xC0, 0xDB, 0xDA, 0x99,
/* a */ 0x9A, 0x82, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x9B, 0xE1, 0xDF, 0xE2, 0xF3, 0xE3,
/* b */ 0xF4, 0xE4, 0xF7, 0x9C, 0xF8, 0x9D, 0x7f, 0x7f, 0xFA, 0x9F, 0xF5, 0xE5, 0xFB, 0xE6, 0xFC, 0xFE,
/* c */ 0xBB, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xAD, 0xE7, 0x7f, 0xFD, 0x7f, 0xE8,
/* d */ 0xBF, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xE9, 0x96, 0xEA, 0x98, 0x97, 0xB0,
/* e */ 0x81, 0x8f, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xB1, 0xB2, 0x7f, 0xB3, 0xB4, 0xB5,
/* f */ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x7f, 0xB6, 0xB7, 0xB8, 0xB9, 0x7f
   },
   /*From 864 to 1046*/
   { /*   0     1,    2,    3,    4,    5,    6,    7,    8,    9,    a,    b,    c,    d,    e,    f */
/* 0 */ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
/* 1 */ 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1B, 0x1c, 0x1D, 0x1E, 0x1F,
/* 2 */ 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
/* 3 */ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
/* 4 */ 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
/* 5 */ 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
/* 6 */ 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
/* 7 */ 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7f,
/* 8 */ 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x8b, 0x8a, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x8c, 0x8d, 0x8e, 0x8f,
/* 9 */ 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0xf8, 0x9d, 0x7f, 0x7f, 0xfa, 0x9f, 0xf6,
/* a */ 0xa0, 0xad, 0xdc, 0x7f, 0xa4, 0xdd, 0x7f, 0x7f, 0xde, 0xc8, 0xca, 0xcb, 0xac, 0xcc, 0xcd, 0xce,
/* b */ 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xe1, 0xbb, 0x83, 0x84, 0x85, 0xbf,
/* c */ 0x88, 0xc1, 0xc2, 0xc3, 0xc4, 0xbe, 0xa6, 0xc7, 0xa7, 0xc9, 0xa8, 0xa9, 0xaa, 0xab, 0xae, 0xcf,
/* d */ 0xd0, 0xd1, 0xd2, 0xaf, 0xba, 0xbc, 0xbd, 0xd7, 0xd8, 0xc0, 0x9a, 0x7c, 0x5e, 0x82, 0x81, 0xd9,
/* e */ 0xe0, 0xdf, 0xf3, 0xf4, 0xf5, 0xfb, 0xfc, 0xe7, 0xe8, 0xe9, 0x97, 0x86, 0xdb, 0x99, 0xda, 0xe5,
/* f */ 0x92, 0xf1, 0xe6, 0xfe, 0xfd, 0x96, 0x98, 0xfb, 0xe2, 0xf7, 0x9c, 0xe4, 0xe3, 0xea, 0x89, 0xff
   }
};

/* Function Prototypes */
void MapFrom1046(unsigned char* fromBuffer, unsigned char* toBuffer,int size, CODEPAGE codepage);
void MapTo1046(unsigned char* fromBuffer, unsigned char* toBuffer,int size, CODEPAGE codepage);
#ifndef _ARABIC_H_
#define _ARABIC_H_

#define ArabicSpecialShaping  (1L << 26)
#define ArabicOneCellShaping  (1L << 27)
#define WordBreak   (1L << 28)

#define MaskArabic      0xf0000000
#define MaskSpecialShaping    0x30000000
#define MaskOneCellShaping    0x40000000
#define MaskWordBreak     0x80000000

/* for the ArabicSpecialShaping */
#define TEXT_STANDARD     0x00000000
#define TEXT_SPECIAL      0x10000000
#define TEXT_COMPOSED     0x20000000


/* For the ArabicOneCellShaping */
#define ONECELL_SEEN      0x40000000
#define TWOCELL_SEEN      0x00000000

/* For the WordBreak */
#define BREAK       0x80000000
#define NO_BREAK      0x00000000

#define EDITINPUT     0
#define EDITREPLACE     1


/*typedef struct {*/

/*  ArabicMethods methods; */

/*  LayoutTextDescriptor       OneCell;*/
/*  LayoutTextDescriptor       Word;*/
/*  LayoutTextDescriptor       SpecialSh;*/
/*  char                       *tempbuf;*/
/*  size_t                     temp_count;*/
/*  size_t                     temp_index;*/
/*  char                       *InCharSet;*/
/*  char               ShapeState;*/
/*  */
/*  void *uni_data;*/
/*} ArabicRec, *Arabic;*/
#endif
