/*! \file
	Description: Program to create initial translation memories

	Copyright Notice:
	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved

	The algorithm used is two folded.
		1.) Align within paragraphs
			This is a widely researched topic. We used an algorithm
			very similar to the one proposed by Gale and Church
		2.) Find matching paragraphs
			The algorithm proposed in Gale and Church didn't work
			out quite well in real environment.
			We developed an own algorithm which might fit into the
			area of finding similarities in the structure of files.
			The algorithm is as follows:
			 Build a matrix with the source and target file.
			 This matrix contains as elements all detected NOP
			 segments. To limit the number only the first and last
			 NOP segment is taken in the case of consecutive NOP
			 segments.
			 Now mark all matching NOPs with 1's in the matrix.
			 This will end up as follows (see example)
				 a    b   c    d
			   a 1    0   0    0
			   c 0    0   1    0
			   d 0    0   0    1

			 To determine the best fitting block, add up any match.
			 values in the diagonale
			 In our example:
				 a    b   c    d
			   a 1    0   0    0
			   c 0    0   1    0
			   d 0    0   0    2
			 Now find the biggest block. The usual approach is via
			 recursion.

			 This algorithm is extended in the way that instead
			 of using n*m elements we use only n+m+1, which is
			 very fast in comparison to the full exploited
			 algorithm.
			 It is based on ideas mentionned in an article in
			 the Information Processing Letters dated 09/90
			 from Wu, Manber, Myers, Miller titled
			  'An O(NP) Sequence comparison algorithm'
*/

#define INITTADUMMYTAG            // initialize dummy tags
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_FOLDER           // folder functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API

  // use import DLL defines for hResMod and dbcs_cp ...
  #define DLLIMPORTRESMOD
  #define DLLIMPORTDBCSCP

#include <eqf.h>                  // General Translation Manager include file
#include <eqfb.id>                // id used for task display...

#include "EQFITM.H"
#include <malloc.h>
#include <eqfsetup.h>             // twbinit, etc.
#include <math.h>                 // square root..
#include "eqfsetup.h"             // twbinit, etc.
#include "eqfmemie.h"
#include "eqftmtag.h"
#include "eqffunci.h"
#include "OtmProposal.h"
#include "core\memory\MemoryFactory.h"
#include "core\utilities\LanguageFactory.h"

#define DELTA      5                   // lag possible in prepareparagraph
#define MAX_SENTENCES  150             // number of sentences in paragraph ..
#define ITM_MAX_RANDOM 60                  // tags are significant...
#define NOPEXLEVEL 3/4                 // if one NOP occurs in more than 75% of
                                       // all NOPs exclude it from anchor list
//#define NOPEXBIGLEVEL 1/5             // in big files, NOP is excluded here
#define NOPEXBIGLEVEL 1/3             // in big files, NOP is excluded here
#define BIGLEVEL   10000               // if a file has more than 10000 segs..
#define MAXNOPALLOWED 500              // maximum nops allowed
//#define MAXNOPALLOWED 60
#define MINOCCURRENCE 10              // don't excl. NOPS which occur less
//#define MINOCCURRENCE 200              // don't excl. NOPS which occur less
#define MAX_EQNUM 50                   // max. assistant anchors per paragraph
#define STR_AS400 L"AS/400"

/**********************************************************************/
/* Distribution function PHI(x) of normal distribute N(0,1)           */
/* each value should be divided by 1000 to get the values of the      */
/* table e.g. out of the book of Hartung 'Statistics'                 */
/* Example:         PHI(1,56) = 0.9406;    PHI(0.09) = 0.5359         */
/* The n-th line starts with  PHI(0,n);                               */
/* the m-th value in line n is  PHI(0,nm);                            */
/*                                                                    */
/* Addition to the table: PHI(-x) = 1-PHI(x);                         */
/**********************************************************************/

#define  MAX_NORMDIST      309         // maximum points in distribution

USHORT usNormalDist[MAX_NORMDIST + 1] =
   { 5000, 5040, 5080, 5120, 5160, 5199, 5239, 5279, 5319, 5359,
     5398, 5438, 5478, 5517, 5557, 5596, 5636, 5675, 5714, 5753,
     5793, 5832, 5871, 5910, 5948, 5987, 6026, 6064, 6103, 6141,
     6179, 6217, 6255, 6293, 6331, 6368, 6406, 6443, 6480, 6517,
     6554, 6591, 6628, 6664, 6700, 6736, 6772, 6808, 6844, 6879,
     6915, 6950, 6985, 7019, 7054, 7088, 7123, 7157, 7190, 7224,
     7257, 7291, 7324, 7357, 7389, 7422, 7454, 7486, 7517, 7549,
     7580, 7611, 7642, 7673, 7704, 7734, 7764, 7794, 7823, 7852,
     7881, 7910, 7939, 7967, 7995, 8023, 8051, 8078, 8106, 8133,
     8159, 8186, 8212, 8238, 8264, 8289, 8315, 8340, 8365, 8389,
     8413, 8438, 8361, 8485, 8508, 8531, 8554, 8577, 8599, 8621,
     8643, 8665, 8686, 8708, 8729, 8749, 8770, 8790, 8810, 8830,
     8849, 8869, 8888, 8907, 8925, 8944, 8961, 8980, 8997, 9015,
     9032, 9049, 9066, 9082, 9099, 9115, 9131, 9147, 9162, 9177,
     9192, 9207, 9222, 9236, 9251, 9265, 9279, 9292, 9306, 9319,
     9332, 9345, 9358, 9370, 9382, 9394, 9406, 9418, 9429, 9441,
     9452, 9463, 9474, 9484, 9495, 9505, 9515, 9525, 9535, 9545,
     9554, 9564, 9583, 9582, 9591, 9599, 9608, 9616, 9625, 9633,
     8641, 9649, 9656, 9664, 9671, 9678, 9686, 9693, 9699, 9706,
     9713, 9719, 9726, 9762, 9728, 9744, 9750, 9756, 9761, 9767,
     9772, 9778, 9783, 9788, 9793, 9798, 9803, 9808, 9812, 9817,
     9821, 9826, 9830, 9834, 9838, 9842, 9846, 9850, 9854, 9857,
     9861, 9864, 9867, 9871, 9875, 9878, 9881, 9884, 9887, 9890,
     9893, 9896, 9898, 9901, 9904, 9906, 9909, 9911, 9912, 9916,
     9918, 9920, 9922, 9925, 9927, 9929, 9931, 9932, 9934, 9936,
     9938, 9940, 9941, 9943, 9945, 9946, 9948, 9949, 9951, 9952,
     9953, 9955, 9956, 9957, 9959, 9960, 9961, 9962, 9963, 9964,
     9965, 9966, 9967, 9968, 9969, 9980, 9971, 9972, 9973, 9974,
     9974, 9975, 9976, 9977, 9977, 9978, 9979, 9979, 9980, 9981,
     9981, 9982, 9982, 9983, 9984, 9984, 9985, 9985, 9986, 9986,
     9987, 9987, 9987, 9988, 9988, 9989, 9989, 9989, 9990, 9990 };




/**********************************************************************/
/* use macros to make program a little bit more readable              */
/**********************************************************************/
#define DIST( x,y )  psDistances[ ((ULONG)x) * ((ulNumY)+1) + (y)]
#define PATHX( x,y ) psPathX[ ((ULONG)x) * ((ulNumY)+1) + (y)]
#define PATHY( x,y ) psPathY[ ((ULONG)x) * ((ulNumY)+1) + (y)]


PITMIDA pStaticITMIda;                 // global ITMIda value


/**********************************************************************/
/* test output ...                                                    */
/**********************************************************************/
#ifdef _DEBUG
  // #define ITMTEST
#endif

#ifdef ITMTEST
  FILE *fOut;                          // test output
  ULONG   ulNumOf21;
  ULONG   ulNumOf10;
  ULONG   ulNumNot11;
#endif

#ifdef ITMTEST
  FILE *LogOpen()
  {
    char str[60];
    UtlMakeEQFPath( str, NULC, LOG_PATH, NULL );
    UtlMkDir( str, 0, FALSE );
    strcat( str, "\\ITMALIGN.LOG" );
    return( fopen( str, "a" ) );
  }
#endif

static VOID MakeHashValue ( PULONG, USHORT, PSZ_W, PULONG );
static BOOL AddToHashBlock ( PITMNOPSEGS, ULONG, ULONG, PSZ_W, ULONG );
static BOOL AlignParagraphs ( PITMIDA );
static VOID PrepareMatrix ( PITMIDA, USHORT, USHORT, USHORT, USHORT );
static ULONG GetNumberOfToBe ( PTBDOCUMENT );
static VOID GetMeanVar ( PITMIDA );
static LONG GetSizeOfBlock ( PITMIDA,PTBDOCUMENT, ULONG, ULONG );
static BOOL GetBothTok(PITMIDA, PTBDOCUMENT,SHORT, PITMNOPSEGS, ULONG);
static BOOL   PostParseAnchor (PITMNOPSEGS, PITMNOPSEGS);


static BOOL CheckNops ( PITMIDA, PTBDOCUMENT, PITMNOPCOUNT );
static VOID AlignCopy ( PALLALIGNED, ULONG, ULONG );
static BOOL PrepareVisSeg ( PITMIDA, ULONG, ULONG, USHORT );
static VOID NopUsage ( PITMIDA );
static BOOL CheckUnaligned (PITMIDA, PALLALIGNED, BOOL, BOOL );
static BOOL CheckTriple ( PITMIDA, PALLALIGNED, BOOL );
static BOOL CheckXtoX ( PITMIDA, PALLALIGNED, BOOL );
static VOID AlignLastSplit ( PALLALIGNED, ULONG, USHORT );
static ULONG CheckDuplicate (PULONG, ULONG, ULONG );
static BOOL CheckIfExcluded ( PITMNOPCOUNT, PSZ_W, SHORT, SHORT );
static BOOL AddToExclNop ( PITMNOPCOUNT, SHORT, SHORT, PSZ_W, USHORT);
static VOID ExclSingleNop ( PITMIDA, PITMNOPCOUNT, PITMNOPCOUNT, PITMNOPCOUNT );
static BOOL EQFITMDoAlign ( PITMIDA );
static SHORT EQFITMDocTagRead ( PITMIDA, PTBDOCUMENT, PSZ, USHORT );
static BOOL  EQFITMPrepOnCond ( PITMIDA );
static VOID Check2Unaligned ( PALLALIGNED, PITMIDA, BOOL );
static BOOL IsOvercross ( PALLALIGNED, PITMIDA, ULONG );
static BOOL TagsAreEqual(SHORT, SHORT, SHORT, PSZ_W, PSZ_W, LONG);
static BOOL ITMIsAnchor ( PITMIDA, ULONG, ULONG, BOOL );
static ULONG  ITMCrossOut ( PITMIDA, PTBDOCUMENT, PTBSEGMENT );

static ULONG ITMInsertAnchor ( PITMIDA, ULONG, USHORT, PULONG, PULONG);
static USHORT ITMCountEqNum(PITMIDA, PTBDOCUMENT, ULONG, ULONG, PSZ_W, PULONG);
static ULONG  ITMFind1stEqNum(PITMIDA, ULONG, ULONG, PSZ_W);
static VOID   PostParseEqualNums(PITMIDA);
static BOOL   HashBlockAlloc(PITMNOPSEGS);
static VOID   PostParseUndoCrossedOut(PITMIDA);
static void   ITMDoUndoCrossOut( ULONG, ULONG, PTBDOCUMENT);

static BOOL   ITMDelUnsureAnchor (PITMNOPSEGS, PITMNOPSEGS);




static BOOL  ITMDelAnchorIfEqual( FUZZYTOK HUGE * , FUZZYTOK HUGE * ,
                                  FUZZYTOK HUGE *);
static BOOL  ITMDelAnchorIfEqual ( FUZZYTOK HUGE *, FUZZYTOK HUGE *, FUZZYTOK HUGE *);
static VOID   ITMDelDupPerSeg(PITMNOPSEGS);
VOID   UtlSetString(SHORT, PSZ);
/**********************************************************************/
/* the following defines are used to define the normalized weight     */
/* according to the definitions in the paper                          */
/*   1-1  match     0.89                                              */
/*   1-0 or 0-1     0.0099                                            */
/*   2-1 or 1-2     0.089                                             */
/*   2-2  match     0.011                                             */
/**********************************************************************/
//original version
//#define PENALTY21   230                // -100 * log(prob. 2-1 match/ 1-1 match
//#define PENALTY22   440                // -100 * log(prob. 2-2 match/ 1-1 match
//#define PENALTY01   450                // -100 * log(prob. 0-1 match/ 1-1 match
//vers 1
//#define PENALTY21   316                // -100 * log(prob. 2-1 match/ 1-1 match
//#define PENALTY22   444                // -100 * log(prob. 2-2 match/ 1-1 match
//#define PENALTY01   455                // -100 * log(prob. 0-1 match/ 1-1 match
//vers2
//#define PENALTY21   316                // -100 * log(prob. 2-1 match/ 1-1 match
//#define PENALTY22   445                // -100 * log(prob. 2-2 match/ 1-1 match
//#define PENALTY01   524                // -100 * log(prob. 0-1 match/ 1-1 match
//vers 3
  #define PENALTY21   458                // -100 * log(prob. 2-1 match/ 1-1 match
  #define PENALTY22   448                // -100 * log(prob. 2-2 match/ 1-1 match
  #define PENALTY01   689                // -100 * log(prob. 0-1 match/ 1-1 match

//vers 4
//#define PENALTY21   691                // -100 * log(prob. 2-1 match/ 1-1 match
//#define PENALTY22   691                // -100 * log(prob. 2-2 match/ 1-1 match
//#define PENALTY01   921                // -100 * log(prob. 0-1 match/ 1-1 match
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMDistMeasure
//------------------------------------------------------------------------------
// Function call:     ITMDistMeasure( sX1, sY1, sX2, sY2, sMean, sVar );
//------------------------------------------------------------------------------
// Description:       define the distance measure between two segments to be
//                    aligned
//                    This function might be called with sX2, sY2 or both of
//                    them to be set to 0.
//------------------------------------------------------------------------------
// Parameters:        SHORT sX1     length of first source segment
//                    SHORT sY1     length of first target segment
//                    SHORT sX2     length of second source segment
//                    SHORT sY2     length of second target segment
//                    DOUBLE dbRatio estimated mean
//                    DOUBLE dbVar  estimated variance
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       Distance distance measure between these two seg.
//------------------------------------------------------------------------------
// Function flow:     get the distance weight and add the penalty.
//------------------------------------------------------------------------------
static
SHORT ITMDistMeasure
(
  SHORT  sX1,                          // length of first source segment
  SHORT  sY1,                          // length of first target segment
  SHORT  sX2,                          // length of second source segment
  SHORT  sY2,                          // length of second target segment
  DOUBLE dbRatio,                      // language ratio
  DOUBLE dbVar                         // language variance
)
{
  SHORT  sDistance;      //  distance of the two segments ...


  /********************************************************************/
  /* get the distance weight and add penalty if necessary             */
  /********************************************************************/
  sDistance = ITMMatch( (SHORT)(sX1 + sX2), (SHORT)(sY1 + sY2),
                           dbRatio, dbVar );

  if ( !sX2 && !sY2 )         // no second parameter specified
  {
    if ( !( sX1 && sY1 ) )
    {
      sDistance += PENALTY01;     // insertion or deletion
    } /* endif */
  }
  else if ( !( sX2 && sY2 ))
  {
    sDistance += PENALTY21;      // contraction or expansion
  }
  else
  {
    sDistance += PENALTY22;      // merge two sentences
  } /* endif */

  return ( sDistance );
} /* end of function ITMDistMeasure */




//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMMatch
//------------------------------------------------------------------------------
// Function call:     ITMMatch( SHORT, SHORT, DOUBLE, DOUBLE );
//------------------------------------------------------------------------------
// Description:       This function determines the distance value for the
//                    passed two sentences depending on the passed mean and
//                    variance.
//
//------------------------------------------------------------------------------
// Parameters:        SHORT    length of source segment
//                    SHORT    length of target segment
//                    DOUBLE   mean
//                    DOUBLE   variance
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       value of distance
//------------------------------------------------------------------------------
// Function flow:     if lenght of both sentences 0 than return 0
//                    else
//                      determine mean of the passed segments
//                      determine position in normal distribution
//                      if pos. outside of range
//                        use highest available position
//                      endif
//
//                      get probability and find distance value
//
//                    endif
//                    return value
//------------------------------------------------------------------------------

static
SHORT ITMMatch
(
  SHORT  sLen1,                        // length of first argument
  SHORT  sLen2,                        // length of second argument
  DOUBLE dbRatio,                      // language ratio
  DOUBLE dbVar                         // variance
)
{
  SHORT  sValue;                       // return value

  USHORT usProb = 0;                   // prbability
  DOUBLE dbMean;                       // mean
  DOUBLE dbZ;                          // value to look up

  if ( ! sLen1 && ! sLen2 )            // both values zero
  {
    sValue = 0;
  }
  else
  {
    dbMean = (sLen1 + ((DOUBLE) sLen2)/ dbRatio)/2;
    dbZ = ((DOUBLE)( dbRatio * sLen1 - sLen2 ))/ sqrt( dbVar * dbMean) ;
    if ( dbZ < (DOUBLE) 0 )
    {
      dbZ = - dbZ;                     // deal only with positive values
    } /* endif */
    dbZ *= 1000;                       // normalize it ...

    if (dbZ < (DOUBLE)(MAX_NORMDIST - 1))
    {
      usProb = 2 * ( 10000 - usNormalDist[ (SHORT) dbZ]);
    }
    else if (dbZ < (DOUBLE) MAX_NORMDIST + 5)
    {
       usProb = 2* ( 10000 - 9990);
    }
    else if (dbZ < (DOUBLE) MAX_NORMDIST + 10)
    {
      usProb = 2 * (10000 - 9991);
    }
    else if (dbZ < (DOUBLE) MAX_NORMDIST + 20)
    {
      usProb = 2 * (10000 - 9992);
    }
    else if (dbZ < (DOUBLE) MAX_NORMDIST + 40)
    {
      usProb = 2 * (10000 - 9993);
    }
    else if (dbZ < (DOUBLE) MAX_NORMDIST + 60)
    {
      usProb = 2 * (10000 - 9994);
    }
    else if (dbZ < (DOUBLE) MAX_NORMDIST + 80)
    {
      usProb = 2 * (10000 - 9995);
    }
    else if (dbZ < (DOUBLE) MAX_NORMDIST + 160)
    {
      usProb = 2 * (10000 - 9996);
    }
    else if (dbZ < (DOUBLE) MAX_NORMDIST + 320)
    {
      usProb = 2 * (10000 - 9997);
    }
    else if (dbZ < (DOUBLE) MAX_NORMDIST + 480)
    {
      usProb = 2 * (10000 - 9998);
    }
    else
    {
      usProb = 0;
    } /* endif */
    if ( usProb )
    {
      sValue = (SHORT) ((-100) * log( (DOUBLE)usProb/10000 ));
    }
    else
    {
      if (dbZ < (DOUBLE)(1000))
      {
        sValue = 1250;
      }
      else if (dbZ < (DOUBLE)(1250))
      {
        sValue = 1850;
      }
      else
      {
        sValue = BIG_DISTANCE;
      } /* endif */
    } /* endif */
#ifdef ITMTEST
    {
//      if (sValue < BIG_DISTANCE )
//      {
//        fOut      = fopen ( "ITMSTAT.OUT", "a" );
//        fprintf( fOut,
//                 "L1 L2 dbZ usProb sValue %4d %4d  %4.4f %4d %4d\n",
//                  sLen1, sLen2,
//                  dbZ, usProb, sValue );
//        fclose( fOut      );
//      }
   }
#endif

  } /* endif */

  return( sValue );
} /* end of function ITMMatch */


/**********************************************************************/
/* alignement loop                                                    */
/* return true if okay, false if no memory                            */
/**********************************************************************/
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMSeqAlign
//------------------------------------------------------------------------------
// Function call:     ITMSeqAlign( *sxY, *sY, usNumX, usNumY, ppAlign,
//                                 *sNum, dbMean, dbVar );
//------------------------------------------------------------------------------
// Description:       allocate temp. arrays and try to align the passed paragr.
//------------------------------------------------------------------------------
// Parameters:        PSHORT   psX    pointer to source length array
//                    PSHORT   psY    pointer to target length array
//                    ULONG    ulNumX number of source segments
//                    ULONG    ulNumY number of target segments
//                    DOUBLE   dbMean estimated mean
//                    DOUBLE   dbVar  estimated variance
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     alignement worked out okay
//                    FALSE    some problem in aligning the paragraph
//------------------------------------------------------------------------------
// Function flow:     determine size of alignment matrix and allocate the nec.
//                    space for it.
//                    if okay
//                      loop through all segments and try to calculate the
//                        distance matrix according to the algorithm of Gale
//                        and Church
//                      do backtracking to find minimum distance
//                      prepare the output structure
//                      free anything not needed any more
//------------------------------------------------------------------------------


BOOL ITMSeqAlign
(
  SHORT  *sX,                 // source length array
  SHORT  *sY,                 // target length array
  ULONG  ulNumX,              // number of source sentences
  ULONG  ulNumY,              // number of target sentences
  PALIGNEMENT * ppAlign,      // return adress of aligned data
  SHORT  *sNum,               // number of aligned sentences
  DOUBLE dbRatio,             // language ratio
  DOUBLE dbVar                // language ratio
)
{
  PSHORT psDistances = NULL;
  PSHORT psPathX = NULL;
  PSHORT psPathY = NULL;
  BOOL  fOK;                           // success indicator
  USHORT  d1, d2, d3, d4, d5, d6;      // temp.var for distances
  USHORT  dMin = 0;                        // minimal value
  SHORT  i, j;                         // indeces for for-loops
  SHORT n = 0;                             // number of aligned sentences
  SHORT  oi, oj;                       // temp variables
  SHORT  di, dj;                       // temp distance  variables
  PALIGNEMENT  pActAlign = NULL;              // active alignment
  PALIGNEMENT pRAlign = NULL;
  ULONG ulSize = 0;

  /********************************************************************/
  /* limit alignment matrix - just in case ....                       */
  /********************************************************************/
  if ( ulNumX > MAX_SENTENCES )
  {
    ulNumX = MAX_SENTENCES;
  } /* endif */
  if ( ulNumY > MAX_SENTENCES )
  {
    ulNumY = MAX_SENTENCES;
  } /* endif */


  /********************************************************************/
  /* size of alignment matrix...                                      */
  /********************************************************************/
  ulSize =  (((ULONG)(ulNumX+1))* (ulNumY+1)* sizeof( SHORT ));

  *sNum = 0;                           // init number of aligned sentences
  ulSize = max( ulSize, MIN_ALLOC );
  fOK = UtlAlloc( (PVOID *) &psDistances, 0L, ulSize, ERROR_STORAGE);
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &psPathX, 0L, ulSize, ERROR_STORAGE);
  } /* endif */
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &psPathY, 0L, ulSize, ERROR_STORAGE);
  } /* endif */

  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &pRAlign,
                    0L,
                    (ULONG) max((ulNumX+ulNumY)*sizeof(ALIGNEMENT), MIN_ALLOC),
                    ERROR_STORAGE );
  } /* endif */

  if ( fOK )
  {
    for ( j=0; j <= (SHORT) ulNumY; j++ )
    {
      for ( i=0; i <= (SHORT) ulNumX; i++ )
      {

        /****************************************************************/
        /* calculate the 6 distances                                    */
        /****************************************************************/
        d1 = (i>0 && j>0) ?            // substitution
             DIST(i-1,j-1) + ITMDistMeasure(sX[i-1],sY[j-1],0,0,dbRatio,dbVar )
                       : MAXINT;
        d2 = (i>0) ?                   // deletion
             DIST(i-1,j) + ITMDistMeasure(sX[i-1],0,0,0,dbRatio,dbVar) : MAXINT;
        d3 = (j>0) ?                   // insertion
             DIST(i,j-1) + ITMDistMeasure(0,sY[j-1],0,0,dbRatio,dbVar) : MAXINT;
        d4 = (i>1 && j>0) ?            // contraction
             DIST(i-2,j-1) + ITMDistMeasure(sX[i-2],sY[j-1],sX[i-1],0,dbRatio,dbVar)
                    :MAXINT;
        d5 = (i>0 && j>1) ?            // expansion
             DIST(i-1,j-2) + ITMDistMeasure(sX[i-1],sY[j-2],0,sY[j-1],dbRatio,dbVar)
                    :MAXINT;
        d6 = (i>1 && j>1) ?            // merging
             DIST(i-2,j-2) +
                ITMDistMeasure(sX[i-2],sY[j-2],sX[i-1],sY[j-1],dbRatio,dbVar):MAXINT;

        /****************************************************************/
        /* find the minimum                                             */
        /****************************************************************/
        dMin = min( d1, d2 );
        dMin = min( dMin, d3 );
        dMin = min( dMin, d4 );
        dMin = min( dMin, d5 );
        dMin = min( dMin, d6 );

        /****************************************************************/
        /* now store values for backtracking ...                        */
        /*                                                              */
        /****************************************************************/
        if ( dMin == MAXINT )
        {
          DIST(i,j) = 0;
        }
        else if ( dMin == d1 )
        {
          DIST(i,j) = d1;
          PATHX(i,j) = i-1;
          PATHY(i,j) = j-1;
        }
        else if ( dMin == d2 )
        {
          DIST(i,j) = d2;
          PATHX(i,j) = i-1;
          PATHY(i,j) = j;
        }
        else if ( dMin == d3 )
        {
          DIST(i,j) = d3;
          PATHX(i,j) = i;
          PATHY(i,j) = j-1;
        }
        else if ( dMin == d4 )
        {
          DIST(i,j) = d4;
          PATHX(i,j) = i-2;
          PATHY(i,j) = j-1;
        }
        else if ( dMin == d5 )
        {
          DIST(i,j) = d5;
          PATHX(i,j) = i-1;
          PATHY(i,j) = j-2;
        }
        else
        {
          DIST(i,j) = d6;
          PATHX(i,j) = i-2;
          PATHY(i,j) = j-2;
        } /* endif */
      } /* endfor */
    } /* endfor */

    /********************************************************************/
    /* now do backtracking to find out the MAX-LikeliHood match         */
    /********************************************************************/
    n = 0;
    for ( i=(SHORT) ulNumX, j=(SHORT) ulNumY; i>0 || j>0; i=oi, j=oj )
    {
      oi = PATHX(i,j);
      oj = PATHY(i,j);
      di = i - oi;
      dj = j - oj;


      pActAlign = pRAlign+n;
      n++;

      /****************************************************************/
      /* in the following each setting is listed but those set to 0   */
      /* are only listed as comments ...                              */
      /****************************************************************/

      if ( di == 1 && dj == 1 )
      {
        pActAlign->sX1 = sX[i-1];
        pActAlign->sY1 = sY[j-1];
       // pActAlign->sX2 = 0;
       // pActAlign->sY2 = 0;
        pActAlign->sDist = DIST(i, j) - DIST( i-1, j-1);
      }
      else if ( di == 1 && dj == 0 )
      {
        pActAlign->sX1 = sX[i-1];
       //  pActAlign->sY1 = 0;
       //  pActAlign->sX2 = 0;
       //  pActAlign->sY2 = 0;
        pActAlign->sDist = DIST(i, j) - DIST( i-1, j);
      }
      else if ( di == 0 && dj == 1 )
      {
       //  pActAlign->sX1 = 0;
        pActAlign->sY1 = sY[j-1];
       //  pActAlign->sX2 = 0;
       //  pActAlign->sY2 = 0;
        pActAlign->sDist = DIST(i, j) - DIST( i, j-1);
      }
      else if ( dj == 1 )
      {
        pActAlign->sX1 = sX[i-2];
        pActAlign->sY1 = sY[j-1];
        pActAlign->sX2 = sX[i-1];
       //  pActAlign->sY2 = 0;
        pActAlign->sDist = DIST(i, j) - DIST( i-2, j-1);
      }
      else if ( di == 1 )
      {
        pActAlign->sX1 = sX[i-1];
        pActAlign->sY1 = sY[j-2];
       //  pActAlign->sX2 = 0;
        pActAlign->sY2 = sY[j-1];
        pActAlign->sDist = DIST(i, j) - DIST( i-1, j-2);
      }
      else if ( di == 2 && dj==2 )
      {
        pActAlign->sX1 = sX[i-2];
        pActAlign->sY1 = sY[j-2];
        pActAlign->sX2 = sX[i-1];
        pActAlign->sY2 = sY[j-1];
        pActAlign->sDist = DIST(i, j) - DIST( i-2, j-2);
      }
      else
      {
        DosBeep( 1200, 175 );
        break;
      } /* endif */

      /****************************************************************/
      /* security check -- we should never come here, but one never   */
      /*   know....                                                   */
      /****************************************************************/
      if ( oi < 0 || oj < 0 || (oi > (SHORT) ulNumX) || (oj >(SHORT) ulNumY) )
      {
        DosBeep( 1200, 175 );
        i = j = 0;
      } /* endif */

    } /* endfor */
  } /* endif */
  /********************************************************************/
  /* now prepare the output structure                                 */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) ppAlign,
                    0L,
                    (ULONG)max( n *sizeof(ALIGNEMENT), MIN_ALLOC),
                    ERROR_STORAGE );
  } /* endif */

  if ( fOK  )
  {
    for ( i=0; i<n; i++ )
    {
      memcpy( (*ppAlign + (n-i-1)), pRAlign+i, sizeof( ALIGNEMENT ));
    } /* endfor */
    *sNum = n;                         // store number of aligned sentences
  } /* endif */

  /********************************************************************/
  /* free anything not needed any more                                */
  /********************************************************************/

  UtlAlloc( (PVOID *) &psDistances, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &psPathX, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &psPathY, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pRAlign, 0L, 0L, NOMSG );

  return( fOK);
} /* end of function ITMSeqAlign */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFITMPrepOnCond
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       prepare files if necessary
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    ok
//------------------------------------------------------------------------------
// Prerequesits:      _
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------
static BOOL
EQFITMPrepOnCond
(
  PITMIDA    pITMIda
)
{
  SHORT     sRc;
  BOOL      fOK = TRUE;
  CHAR      chText[1];

  /********************************************************************/
  /* reset slider                                                     */
  /********************************************************************/
  chText[0] = EOS;
  WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
              MP1FROMSHORT(-1), chText );
  WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
              MP1FROMSHORT(-2), chText );
  WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
              MP1FROMSHORT(0), NULL );
  /**************************************************************/
  /* if only prepare required, do nothing if prepared file      */
  /* exists already                                             */
  /**************************************************************/
  if ( (!pITMIda->fPrepare) &&
        IsPrepared(pITMIda, &(pITMIda->usNumPrepared)) )
  {
    sRc = EQFITMReadAli(pITMIda, TRUE);              //read in alifile
    switch ( sRc )
    {
      case  REALIGN:                    //alifile in error,
        EQFITMDelAli(pITMIda);          //del alifile/Num in propfile
        break;
      case  GOON:                        //alifile is ok
        break;
      default :
        fOK = FALSE;
        break;
    } /* endswitch */
  } /* endif */
  if ( fOK && !IsPrepared(pITMIda, &(pITMIda->usNumPrepared)) )
  {
    pITMIda->usStatus = ITM_STAT_ANALYSIS;
    pITMIda->itmSrcNop.ulAlloc = 0;
    pITMIda->itmTgtNop.ulAlloc = 0;
    pITMIda->itmSrcNop.ulUsed = 0; // nothing used yet
    pITMIda->itmTgtNop.ulUsed = 0; // nothing used yet
    pITMIda->itmSrcText.ulAlloc = 0;
    pITMIda->itmTgtText.ulAlloc = 0;
    pITMIda->itmSrcText.ulUsed = 0;// nothing used yet
    pITMIda->itmTgtText.ulUsed = 0;// nothing used yet
    pITMIda->fParallel = FALSE;

    fOK = PrepFiles( pITMIda );
  } /* endif */

  return (fOK);
} /* end of function EQFITMPrepOnCond */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFITMProcess
//------------------------------------------------------------------------------
// Function call:     for windows
//------------------------------------------------------------------------------
// Description:       EQFITMProcess will run as processing as substitution
//                    for the thread in OS/2.
//                    the WM_EQF_PROCESSTASK msg is posted in ITMWNDProc
//                    which calls EQFITMProcess with the corresponding
//                    parameters pulActTask and pusAddTask which control
//                    which part of process is executed.
//------------------------------------------------------------------------------
// Parameters:        HWND     hwnd,          handle of window
//                    PUSHORT  pulActTask,    task to be done
//                    PULONG   pulAddParam    additional parameter for
//                                            task  ITM_PROCESS_ALIGNSEGS
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     switch ( kind of task to do)
//                      case  ITM_PROCESS_TMOPEN:
//                             open memory
//                      case  ITM_PROCESS_TEMPFOLD
//                             open temporary folder
//                      case  ITM_PROCESS_PREPFILE:
//                             if possible
//                                get next filepair
//                                if ok prepare next filepair
//                             else
//                                set next task to ITM_PROCESS_CLOSE
//                             endif
//                      case  ITM_PROCESS_READSRC:
//                            init tbdoc structure values
//                            load tagtable if not yet done
//                            read in source file
//                      case  ITM_PROCESS_READTGT:
//                            read in target file
//                      case  ITM_PROCESS_NOPUSAGE:
//                            check whether all NOPS are used as anchors
//                      case  ITM_PROCESS_TOKSRC:
//                            build src anchor candidate list
//                      case  ITM_PROCESS_TOKTGT:
//                            build tgt anchor candidate list
//                      case ITM_PROCESS_ALIGNPARA:
//                          align paragraphs with ITMLCS
//                      case ITM_PROCESS_MEANVAR:
//                          calc mean and variance of document for Gale&CHurch
//                          set init values for aligning inside of paragraphs
//                      case ITM_PROCESS_ALIGNSEGS:
//                          align current paragraph
//                          fill align struct
//                          point to next paragraph which is not empty
//                          if next paragraph is available
//                            next task is ITM_PROCESS_ALIGNSEGS
//                             and additional param points to next paragraph
//                          else next task is ITM_PROCESS_PARSEALIGN
//                      case ITM_PROCESS_PARSEALIGN:
//                         parse alignstruct to make sure each segments
//                         is aligned no more than once
//                      case ITM_PROCESS_JOINSEG:
//                         join segments which belong to 1:2 or 2:1 match
//                      case ITM_PROCESS_MEMSAVE:
//                         if no visualization
//                            save alignments in memory
//                         else
//                            set status to visualization
//                         endif
//                         set next task to ITM_PROCESS_PREPFILE
//                      case ITM_PROCESS_CLOSE:
//                         close open memory
//                      case ITM_PROCESS_REMOVEFLD:
//                         remove temporary folder structure
//                    endswitch
//                    if not ok
//                       set    fKill = TRUE;
//                    return ptr to next task
//------------------------------------------------------------------------------
VOID
EQFITMProcess
(
  HWND     hwnd,
  PITMIDA  pITMIda,
  PUSHORT  pusActTask,
  PULONG   pulAddParam
)
{
  BOOL       fOK = TRUE;               // success indicator
  SHORT      sRc = 0;                  // return code from EQFB functions
  SHORT      sNum = 0;                     // number of aligned sentences
  ULONG      ulUsed = 0;                   // index to active block
  PSZ        pTemp = NULL;                    // temp file pointer
  SHORT      sLangID = 0;

  USHORT     usNextTask = ITM_PROCESS_END;
   USHORT    usNewPerc = 0;
   ULONG     ulOemCP = 0L;


  pITMIda->fBusy = TRUE;
  switch ( *pusActTask )
  {
    case  ITM_PROCESS_TMOPEN:
      /********************************************************************/
      /* try to open translation memory file                              */
      /********************************************************************/
      if ( ITMTmOpen(pITMIda) )
      {
        pITMIda->fKill = TRUE;
      } /* endif */

      UtlTime( &pITMIda->lTime );

      if ( !pITMIda->fKill && pITMIda->fSGMLITM )
      {

        fOK = ! UtlBufOpen( &pITMIda->pBufCB, pITMIda->chSGMLMem,
                            ITM_BUFSIZE, FILE_CREATE, TRUE );
        if ( fOK && (pITMIda->usSGMLFormat == SGMLFORMAT_UNICODE))
        {
          // write Unicode prefix to outfile
          fOK = !UtlBufWrite( pITMIda->pBufCB, UNICODEFILEPREFIX,
                               (SHORT)strlen(UNICODEFILEPREFIX), TRUE );
        } /* endif */
        if ( fOK )
        {
          fOK = ! UtlBufWriteConv( pITMIda->pBufCB, NTM_BEGIN_TAGW,
                               (USHORT)UTF16strlenBYTE( NTM_BEGIN_TAGW ),
                                TRUE, pITMIda->usSGMLFormat, pITMIda->ulSGMLFormatCP,
                                pITMIda->ulAnsiCP);
        } /* endif */
      } /* endif */
      usNextTask = ITM_PROCESS_TEMPFOLD;
      break;
    case  ITM_PROCESS_TEMPFOLD:
      /********************************************************************/
      /* create Temporary folder structure to copy files into             */
      /********************************************************************/
      if ( !pITMIda->fNoAna)
      {                                                              /* @KIT1008C5*/
        pITMIda->fKill |= ! CreateFolderStruct(pITMIda,TEMPFOLDERSRCNEW);
        if ( !pITMIda->fKill )
        {
          pITMIda->fKill |= ! CreateFolderStruct( pITMIda, TEMPFOLDERTGTNEW );
        } /* endif */
      } /* endif */
      usNextTask = ITM_PROCESS_PREPFILE;
      break;
    case  ITM_PROCESS_PREPFILE:
      /****************************************************************/
      /* get the file pair for processing ...                         */
      /****************************************************************/
      if ( pITMIda->usArgc >= 2 )
      {
        pITMIda->fKill  |= ! GetNextFilePair (hwnd, pITMIda);
        if (!pITMIda->fKill)
        {
          memset( &pITMIda->TBSourceDoc, 0, sizeof( TBDOCUMENT ));
          memset( &pITMIda->TBTargetDoc, 0, sizeof( TBDOCUMENT ));
        } /* endif */
        if ( ! pITMIda->fKill )
        {
          if ( EQFITMPrepOnCond(pITMIda))
          {
            usNextTask = ITM_PROCESS_ANALYSE;
          }
          else
          {
            pITMIda->fKill = TRUE;
          } /* endif */
        } /* endif */
      }
      else
      {
        ITMComplete (hwnd, pITMIda);

        usNextTask = ITM_PROCESS_CLOSE;
      } /* endif */

      break;
    case  ITM_PROCESS_ANALYSE:
      if ( ITMAnalyseFiles(pITMIda))
      {
        usNextTask = ITM_PROCESS_READSRC;
      }
      else
      {
        pITMIda->fKill = TRUE;
      } /* endif */
      break;
    case  ITM_PROCESS_READSRC:
      /****************************************************************/
      /* create doc structure and read in segmented Source and target */
      /* use EQFBFileRead from Translation Processor                  */
      /* The segment table is used for further alignement             */
      /* Do nothing if already prepared and 'Prepare' pressed now     */
      /****************************************************************/
      if ( !(pITMIda->fPrepare && pITMIda->usNumPrepared) )
      {
        pITMIda->usStatus = ITM_STAT_PREPARE;
        /**************************************************************/
        /* load internal table if not yet done ...                    */
        /**************************************************************/
        {
          CHAR chTagFile[ MAX_FILESPEC ];

          pTemp = UtlGetFnameFromPath(pITMIda->chQFTagTable);
          Utlstrccpy( chTagFile, pTemp, DOT );
          sRc = TALoadTagTable( chTagFile,
                            (PLOADEDTABLE *) &pITMIda->TBSourceDoc.pQFTagTable,
                              TRUE, FALSE );

          if ( !sRc )
          {
            pITMIda->TBTargetDoc.pQFTagTable = pITMIda->TBSourceDoc.pQFTagTable;
          } /* endif */
        }
        /**************************************************************/
        /* init doc structure values                                  */
        /**************************************************************/
        if ( !sRc && !pITMIda->fKill)
        {
          sRc = EQFITMDocTagRead(pITMIda, &(pITMIda->TBSourceDoc),
                                 pITMIda->chSegSourceFile, SSOURCE_DOC);
        } /* endif */

        /**************************************************************/
        /* set return codes if necessary ...                          */
        /**************************************************************/
        if ( sRc )
        {
          fOK = FALSE;
          pITMIda->fKill = TRUE;
        } /* endif */
      } /* endif */
      usNextTask = ITM_PROCESS_READTGT;
      break;
    case  ITM_PROCESS_READTGT:
      if ( !sRc && !pITMIda->fKill &&
             !(pITMIda->fPrepare && pITMIda->usNumPrepared) )
      {
        sRc = EQFITMDocTagRead(pITMIda, &(pITMIda->TBTargetDoc),
                               pITMIda->chSegTargetFile, STARGET_DOC);
      } /* endif */

      /**************************************************************/
      /* set return codes if necessary ...                          */
      /**************************************************************/
      if ( sRc )
      {
        fOK = FALSE;
        pITMIda->fKill = TRUE;
      } /* endif */
      if ( !pITMIda->usNumPrepared )
      {
        usNextTask = ITM_PROCESS_NOPUSAGE;
      }
      else
      {
        usNextTask = ITM_PROCESS_MEMSAVE;
      } /* endif */


      if ( fOK )
    {
        // add source and target language to determine if we have BIDI or not ...
        pITMIda->TBSourceDoc.usLangTypeTgt = MorphGetLanguageType( pITMIda->szSourceLang );
        pITMIda->TBSourceDoc.usLangTypeSrc = MorphGetLanguageType( pITMIda->szSourceLang );
        pITMIda->TBTargetDoc.usLangTypeTgt = MorphGetLanguageType( pITMIda->szTargetLang );
        pITMIda->TBTargetDoc.usLangTypeSrc = MorphGetLanguageType( pITMIda->szTargetLang );

        pITMIda->TBSourceDoc.ulOemCodePage = GetLangCodePage(OEM_CP, pITMIda->szSourceLang);
        pITMIda->TBSourceDoc.ulAnsiCodePage = GetLangCodePage(ANSI_CP, pITMIda->szSourceLang);
        pITMIda->TBTargetDoc.ulOemCodePage = GetLangCodePage( OEM_CP, pITMIda->szTargetLang);
        pITMIda->TBTargetDoc.ulAnsiCodePage = GetLangCodePage( ANSI_CP, pITMIda->szTargetLang);

    } /* endif */

      break;
    case  ITM_PROCESS_NOPUSAGE:
      /**************************************************************/
      /* do plausability check whether NOPS should be used          */
      /*    as anchors or not                                       */
      /**************************************************************/
      if ( fOK )
      {
      } /* endif */
      NopUsage(pITMIda);
      usNextTask = ITM_PROCESS_TOKSRC;
      break;
    case  ITM_PROCESS_TOKSRC:
      fOK = !MorphGetLanguageID( pITMIda->szSourceLang, &sLangID );
      if (fOK)
      {
        ulOemCP = GetLangOEMCP( pITMIda->szSourceLang);
        fOK = GetBothTok(pITMIda,&(pITMIda->TBSourceDoc),sLangID,
                     &pITMIda->itmSrcNop, ulOemCP);
      }
      usNextTask = ITM_PROCESS_TOKTGT;
      break;
    case  ITM_PROCESS_TOKTGT:
      fOK = !MorphGetLanguageID( pITMIda->szTargetLang, &sLangID );
      if ( fOK )
      {
        ulOemCP = GetLangOEMCP( pITMIda->szTargetLang);
        fOK = GetBothTok(pITMIda,&(pITMIda->TBTargetDoc),sLangID,
                       &pITMIda->itmTgtNop, ulOemCP);
      } /* endif */
      if ( fOK )
      {
          if ((pITMIda->itmSrcNop.ulAlloc >= MAX_ALLOC ) ||
              (pITMIda->itmTgtNop.ulAlloc >= MAX_ALLOC) )
          {
            ITMDelDupPerSeg(&(pITMIda->itmSrcNop));
            ITMDelDupPerSeg(&(pITMIda->itmTgtNop));
          } /* endif */
      } /* endif */
      usNextTask = ITM_PROCESS_ALIGNPARA;
      break;
    case  ITM_PROCESS_ALIGNPARA:
      /****************************************************************/
      /* now align the paragraphs                                     */
      /****************************************************************/
      AlignParagraphs( pITMIda );
      usNextTask = ITM_PROCESS_MEANVAR;
      break;
    case  ITM_PROCESS_MEANVAR:
      /****************************************************************/
      /* set up mean and variance estimates                           */
      /****************************************************************/
      GetMeanVar( pITMIda );
      if ( ! pITMIda->fKill )
      {
        pITMIda->usStatus = ITM_STAT_ALIGN;
        pITMIda->usLastSegFilled = 0;
        ulUsed = 0;
        /******************************************************************/
        /* add dummy entry at position zero in alignment struct           */
        /******************************************************************/
        pITMIda->Aligned.ulFillStart = 0;
        pITMIda->Aligned.ulFillEnd   = 0;
        pITMIda->Aligned.ulFillIndex = 0;

        fOK = AddToAlignStruct(&pITMIda->Aligned,0, 0, NUL_ONE, 0, INITFILL );
      }
      usNextTask = ITM_PROCESS_ALIGNSEGS;
      *pulAddParam = ulUsed;
      pITMIda->usOldPerc = 0;
      break;
    case  ITM_PROCESS_ALIGNSEGS:
      ulUsed = *pulAddParam;
      if ( ulUsed < pITMIda->itmSrcNop.ulUsed && ! pITMIda->fKill)
      {
        usNewPerc = (SHORT)(((LONG)ulUsed*20L)/pITMIda->itmSrcNop.ulUsed);
        if ( usNewPerc != pITMIda->usOldPerc )
        {
          pITMIda->usOldPerc = usNewPerc;
          WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
             MP1FROMSHORT(pITMIda->usOldPerc + (pITMIda->usStartSlider)),
             NULL );
        } /* endif */
        /************************************************************/
        /* find next chunk of data to be aligned                    */
        /************************************************************/
        fOK = PrepareNextBlock( pITMIda, ulUsed );

        /************************************************************/
        /* align them                                               */
        /************************************************************/
        if ( fOK && ! pITMIda->fKill)
        {
          fOK = ITMSeqAlign( pITMIda->itmSrcText.psPara,
                             pITMIda->itmTgtText.psPara,
                             pITMIda->itmSrcText.ulUsed,
                             pITMIda->itmTgtText.ulUsed,
                             &pITMIda->pAlign, &sNum,
                             pITMIda->dbMean,
                             pITMIda->dbVar );
        } /* endif */

        /************************************************************/
        /* fill translation memory                                  */
        /************************************************************/
        if ( fOK && !pITMIda->fKill )
        {
          fOK = FillAlignStruct(pITMIda, pITMIda->pAlign, sNum, INITFILL);
          /**********************************************************/
          /* free the aligned resources                             */
          /**********************************************************/
          UtlAlloc( (PVOID *) &pITMIda->pAlign, 0L, 0L, NOMSG );
        } /* endif */

        /************************************************************/
        /* point to next block                                      */
        /************************************************************/
        ulUsed++;
        while ( ulUsed < pITMIda->itmSrcNop.ulUsed &&
              (pITMIda->itmSrcNop.pulSegs[ulUsed] ==
               pITMIda->itmSrcNop.pulSegs[ulUsed+1] ))
        {
           ulUsed++;
        } /* endwhile */
      } /* endif */

      if ( ulUsed < pITMIda->itmSrcNop.ulUsed && fOK && ! pITMIda->fKill)
      {
        usNextTask = ITM_PROCESS_ALIGNSEGS;
        *pulAddParam = ulUsed;
      }
      else
      {
        if ( fOK && !pITMIda->fKill )
        {
          usNextTask = ITM_PROCESS_PARSEALIGN;
        } /* endif */
      }
      break;
    case  ITM_PROCESS_PARSEALIGN:
#ifdef ITMTEST
    {
      FILE *log;                          // test output
      ULONG ulTemp1;

      log = LogOpen();

      fprintf ( log, "Before ParseAlign \n");
      for ( ulTemp1=0;ulTemp1 < (pITMIda->Aligned.ulUsed);ulTemp1++ )
      {
        fprintf( log,
                 "%4d Seg: %4d %4d %4d %4d\n",
                  ulTemp1,
                  pITMIda->Aligned.pulSrc[ulTemp1],
                  pITMIda->Aligned.pulTgt1[ulTemp1],
                  (USHORT) pITMIda->Aligned.pbType[ulTemp1],
                  (USHORT) pITMIda->Aligned.psDist[ulTemp1]);
      } /* endfor */
    fclose( log );
   }
#endif
      fOK = ParseAlignStruct(pITMIda, &pITMIda->Aligned);
#ifdef ITMTEST
 {
    ULONG   ulTemp1;
    FILE *log;                          // test output

      log = LogOpen();

      fprintf (log, "After  ParseAlign \n");
      for ( ulTemp1=0;ulTemp1 < (pITMIda->Aligned.ulUsed);ulTemp1++ )
      {
        fprintf( log,
                 "%4d Seg: %4d %4d 4%d \n",
                  ulTemp1,
                  pITMIda->Aligned.pulSrc[ulTemp1],
                  pITMIda->Aligned.pulTgt1[ulTemp1],
                  (USHORT) pITMIda->Aligned.pbType[ulTemp1]);
      } /* endfor */
  fclose( log );
}
#endif
      usNextTask = ITM_PROCESS_JOINSEG;
      break;
    case  ITM_PROCESS_JOINSEG:
      if ( fOK )
      {
         BuildJoinSeg(pITMIda);
      } /* endif */
      usNextTask = ITM_PROCESS_MEMSAVE;
      break;
    case  ITM_PROCESS_MEMSAVE:
      if ( !pITMIda->fVisual && !pITMIda->fPrepare)
      {

        fOK = ITMFuncMemSave ( pITMIda, FALSE );
        usNextTask = ITM_PROCESS_PREPFILE;
      }
      else
      {
        pITMIda->usStatus = ITM_STAT_VISUAL;
//      TimerStartVisual( hwnd, pITMIda );
        usNextTask = ITM_PROCESS_VISUAL;
      } /* endif */
      break;
    case  ITM_PROCESS_CLOSE:
//      ITMCloseProcessing( pITMIda );
      usNextTask = ITM_PROCESS_END;
      break;
    default :
      break;
  } /* endswitch */

  /********************************************************************/
  /* set value of next act. task                                      */
  /********************************************************************/
  *pusActTask = usNextTask;
  if ( !fOK )
  {
    pITMIda->fKill = TRUE;
  } /* endif */

  return;
} /* end of function EQFITMProcess */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     AddToBlock
//------------------------------------------------------------------------------
// Function call:     AddToBlock(pITMAlign,ulStartSeg, ulEndSeg, usBlockLen);
//------------------------------------------------------------------------------
// Description:       add a new segment to a paragraph
//------------------------------------------------------------------------------
// Parameters:        PITMALIGN  pITMAlign,        ITM Align struct to be used
//                    USHORT     usStartSeg,       start segment to be used
//                    USHORT     usEndSeg,         number of end segments
//                    USHORT     usBlockLen        length of block
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    if ok
//                    FALSE   if something went wrong
//------------------------------------------------------------------------------
// Function flow:     if all allocated segments are already used
//                      allocate new space
//                    endif
//                    if ok
//                      fill in the new values (blocklen,StartSeg,EndSeg)
//                        for the paragraph
//                      point to next entry
//                    endif
//                    return success
//------------------------------------------------------------------------------
static
BOOL AddToBlock
(
  PITMALIGN  pITMAlign,                // ITM Align struct to be used
  ULONG      ulStartSeg,               // start segment to be used
  ULONG      ulEndSeg,                 // number of end segments
  ULONG      ulBlockLen                // length of block
)
{
  BOOL  fOK = TRUE;                    // success indicator
  LONG  lOldLen;                       // old length
  LONG  lNewLen;                       // new length

  /********************************************************************/
  /* check if still one segment is free to be filled ...              */
  /* if not allocate memory first ...                                 */
  /********************************************************************/
  if ( pITMAlign->ulAlloc == pITMAlign->ulUsed )
  {
    pITMAlign->ulAlloc += NEW_ALIGN;
    /******************************************************************/
    /* get old and new length for reallocation...                     */
    /******************************************************************/
    lOldLen =  (LONG) pITMAlign->ulUsed * sizeof( PSHORT );
    lNewLen =  (LONG) pITMAlign->ulAlloc * sizeof( PSHORT );

#ifdef ITMTEST
//  fOut      = fopen ( "ITMSTAT.OUT", "a" );
//      fprintf( fOut, "Alloc in AddTOBlock \n" );
//  fclose( fOut      );
#endif
    fOK = UtlAlloc( (PVOID *) &(pITMAlign->psPara), lOldLen, lNewLen, ERROR_STORAGE );
    if ( fOK  )
    {
      lOldLen =  (LONG) pITMAlign->ulUsed * sizeof( ULONG );
      lNewLen =  (LONG) pITMAlign->ulAlloc * sizeof( ULONG );

      fOK = UtlAlloc( (PVOID *) &(pITMAlign->pulSegStart),
                      lOldLen, lNewLen, ERROR_STORAGE );
    } /* endif */
    if ( fOK  )
    {
      fOK = UtlAlloc( (PVOID *) &(pITMAlign->pulSegEnd),
                      lOldLen, lNewLen, ERROR_STORAGE );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* fill in the new values for this NOP block                        */
  /********************************************************************/
  if ( fOK )
  {
    pITMAlign->psPara[pITMAlign->ulUsed] = (SHORT) ulBlockLen;
    pITMAlign->pulSegStart[pITMAlign->ulUsed] = ulStartSeg;
    pITMAlign->pulSegEnd[pITMAlign->ulUsed] = ulEndSeg;
    pITMAlign->ulUsed ++;              // point to next entry
  } /* endif */
  return ( fOK );
} /* end of function AddToBlock */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFTextSegm
//------------------------------------------------------------------------------
// Function call:     EQFTextSegm( pITMIDa, hwnd, pFolder, pInFile )
//------------------------------------------------------------------------------
// Description:       do on the spot invocation of text segmentation
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda   pointer to instance data
//                    HWND     hwnd      window handle
//                    PSZ      pFolder   pointer to folder name
//                    PSZ      pInFile   pointer to input file name
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     text file segmented
//                    FALSE    error happened during segmentation
//------------------------------------------------------------------------------
// Side effects:      Allocated memory for TAINPUT structure will be freed in
//                    Text Analysis.
//------------------------------------------------------------------------------
// Function flow:     allocate TAINPUT structure
//                    if okay then
//                      fill in initial values
//                      call TextSegmentation; set fOK
//                    endif
//                    return fOK
//------------------------------------------------------------------------------

BOOL EQFTextSegm
(
   PITMIDA pITMIda,                    // pointer to ida
   HWND hwnd,                          // window handle of calling process
   PSZ pFolderName,                    // folder name
   PSZ pInFile                         // input file name
)
{
  BOOL
    fOK = TRUE;
  PSZ        pTempData;                // pointer to temporary buffer
  CHAR       szBuf[ MAX_LANGUAGE_PROPERTIES];    // temp buffer ...
  /******************************************************************/
  /* call text analysis process ...                                 */
  /******************************************************************/
  CHAR szFile[ MAX_PATH144 ];
  CHAR chShortFName[ MAX_FILESPEC ];

//pIda->fAnalysisIsRunning = TRUE;
  pTempData = UtlGetFnameFromPath( pFolderName );
  UtlMakeEQFPath( szFile, pFolderName[0],
                  SYSTEM_PATH, pTempData );
  strcat( szFile, BACKSLASH_STR );
 // handle long or short file name

  if ( UtlIsLongFileName( pInFile ))
  {
    BOOL fIsNew;                   // new document flag

    // get the correct short file name for the document
    FolLongToShortDocName( pTempData,
                           pInFile,
                           chShortFName,
                           &fIsNew );
    pInFile = chShortFName;
  } /* endif */

  strcat( szFile, pInFile );

  fOK = TAAnalyzeFile( szFile , hwnd, 0, NULL, &pITMIda->pfKillAnalysis );

   //check segmented target file
  if ( fOK && !pITMIda->fKill )
  {
    //check if segmented target file exists
    UtlMakeEQFPath( szBuf, pFolderName[0], DIRSEGSOURCEDOC_PATH, pTempData );
    strcat( szBuf, BACKSLASH_STR );
    strcat( szBuf, pInFile );
    fOK = ITMFileExist( szBuf );
    if( !fOK )
    {
       pITMIda->fKill = TRUE;
    }/* endif fOK*/
  }/* endif fOK*/
  return fOK;
} /* end of function EQFTextSegm */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     FreeNOPDoc
//------------------------------------------------------------------------------
// Function call:     FreeNOPDoc( pTBDoc, pITMNopSegs );
//------------------------------------------------------------------------------
// Description:       Free the allocated space for the document structure
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pTBDoc   pointer to docum. instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     free buffers for input and tokens
//                    loop thru seg table and free all these buffers
//                    free segtable and undo segment
//                    free anchor token (pITMNopSegs)
//                    return
//------------------------------------------------------------------------------
static
VOID FreeNOPDoc
(
  PTBDOCUMENT pDoc,
  PITMNOPSEGS pITMNopSegs
)
{
   /* Free up the document space */
   EQFBFreeDoc( &pDoc, EQFBFREEDOC_NODOCIDAFREE );

   if ( pITMNopSegs->pTokenList )
   {
     FREEHUGE( pITMNopSegs->pTokenList );
     pITMNopSegs->pTokenList = NULL;
   } /* endif */
   if ( pITMNopSegs->pulSegs)
   {
     FREEHUGE( pITMNopSegs->pulSegs );
     pITMNopSegs->pulSegs = NULL;
   } /* endif */
   pITMNopSegs->ulAlloc = 0;
   pITMNopSegs->ulUsed  = 0;

} /* end of function FreeNOPDoc */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     WriteSegm
//------------------------------------------------------------------------------
// Function call:     WriteSegm( pITMIda, ulSegNum )
//------------------------------------------------------------------------------
// Description:       write the segment in the SGML file
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda pointer to ida
//                    USHORT   ulSegNum segment number
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE         everything okay
//                    FALSE        something wrong
//------------------------------------------------------------------------------
// Function flow:     write control information
//                    if okay
//                      write source data
//                    if okay
//                      write target data
//                    if okay
//                      write end segment data
//                    return
//------------------------------------------------------------------------------

static BOOL
WriteSegm
(
  PITMIDA pITMIda,                     // pointer to instance data
  ULONG   ulSegNum                     // segment number
)
{
  BOOL  fOK;                           // success indicator
  char  chMTFlag = '1';
  PSZ   pszLongName;
  PSZ   pName;

  if (*(pITMIda->pInFile) == BACKSLASH)
  {
    pName = pITMIda->pInFile + 1;
  }
  else
  {
    pName = pITMIda->pInFile;
  }
  swprintf(pITMIda->szBufferW, L"<Segment>%010ld\r\n", ulSegNum );

  fOK = ! UtlBufWriteConv( pITMIda->pBufCB, pITMIda->szBufferW,
                       (USHORT)UTF16strlenBYTE( pITMIda->szBufferW ),
                       TRUE, pITMIda->usSGMLFormat, pITMIda->ulSGMLFormatCP,
                       pITMIda->ulAnsiCP);
  if ( fOK )
  {
     /*****************************************************************/
     /* prepare and write the control tag                             */
     /*****************************************************************/
     pszLongName = UtlGetFnameFromPath( pITMIda->chSourceFile );
     if ( !pszLongName )
     {
       pszLongName = pITMIda->chSourceFile;
     } /* endif */
     sprintf( pITMIda->szBuffer,
              "%s%06lu%s%c%s%016lu%s%s%s%s%s%s%s%s%s%s%s%s%s",
             //                                           +- </Control>
             //                                         +- long file name
             //                                       +--- X15
             //                                     +----- file name
             //                                   +------- X15
             //                                 +--------- tag table
             //                               +----------- X15
             //                             +------------- author
             //                           +--------------- X15
             //                         +----------------- target lng
             //                       +------------------- X15
             //                     +--------------------- source lng
             //                   +----------------------- X15
             //             +----------------------------- time
             //           +------------------------------- X15
             //         +--------------------------------- mt flag
             //       +----------------------------------- X15
             //  +---------------------------------------- seg id
             //+------------------------------------------ <Control>
              MEM_CONTROL_BEGIN_TAG,
              ulSegNum,
              X15_STR,
              chMTFlag,
              X15_STR,
              0L,
              X15_STR,
              pITMIda->szSourceLang,
              X15_STR,
              pITMIda->szTargetLang,
              X15_STR,
              "ITM",
              X15_STR,
              pITMIda->chTagTableName,
              X15_STR,
              pITMIda->chShortSrcFName,
              X15_STR,
              pName,
              MEM_CONTROL_END_TAG );

    ASCII2Unicode(pITMIda->szBuffer, pITMIda->szBufferW, 0L );

    fOK = ! UtlBufWriteConv( pITMIda->pBufCB, pITMIda->szBufferW,
                         (USHORT)UTF16strlenBYTE( pITMIda->szBufferW ),
                         TRUE,  pITMIda->usSGMLFormat, pITMIda->ulSGMLFormatCP,
                         pITMIda->ulAnsiCP);
  } /* endif */
  if ( fOK )
  {
    EQFBBufRemoveTRNote(pITMIda->szSourceSeg, pITMIda->pLoadedTable,
                           pITMIda->TBSourceDoc.pfnUserExit,
                           pITMIda->TBSourceDoc.pfnUserExitW,
                           pITMIda->TBSourceDoc.ulOemCodePage);
    EQFBBufRemoveTRNote(pITMIda->szTargetSeg, pITMIda->pLoadedTable,
                         pITMIda->TBTargetDoc.pfnUserExit,
                         pITMIda->TBTargetDoc.pfnUserExitW,
                         pITMIda->TBTargetDoc.ulOemCodePage);
    if (pITMIda->TBSourceDoc.fLineWrap && pITMIda->TBSourceDoc.fAutoLineWrap )
    {
         USHORT  i;
         USHORT  usSegOffset = 0;
         EQFBBufRemoveSoftLF(pITMIda->TBSourceDoc.hwndRichEdit, pITMIda->szSourceSeg, &i, &usSegOffset);
         usSegOffset = 0;
         EQFBBufRemoveSoftLF(pITMIda->TBSourceDoc.hwndRichEdit, pITMIda->szTargetSeg, &i, &usSegOffset);
    }
   /******************************************************************/
    /* convert LF into CRLF format .....                              */
    /******************************************************************/
    usConvertCRLFW(pITMIda->szSourceSeg,  // pointer to character input area
                   (USHORT)(UTF16strlenCHAR( pITMIda->szSourceSeg) + 1),
                   pITMIda->szBufferW,
                   FALSE);                // conversion mode :  CRLF
    UTF16strcpy( pITMIda->szSourceSeg, pITMIda->szBufferW );


    swprintf(pITMIda->szBufferW, L"<Source>%s</Source>\r\n", pITMIda->szSourceSeg);
    fOK = ! UtlBufWriteConv( pITMIda->pBufCB, pITMIda->szBufferW,
                       (USHORT)UTF16strlenBYTE( pITMIda->szBufferW ),
                       TRUE, pITMIda->usSGMLFormat, pITMIda->ulSGMLFormatCP,
                       pITMIda->ulAnsiCP);
  } /* endif */

  if ( fOK )
  {
    usConvertCRLFW(pITMIda->szTargetSeg,  // pointer to character input area
                   (USHORT)(UTF16strlenCHAR( pITMIda->szTargetSeg) + 1),
                   pITMIda->szBufferW,
                   FALSE);                // conversion mode :  CRLF
    UTF16strcpy( pITMIda->szTargetSeg, pITMIda->szBufferW );
    swprintf(pITMIda->szBufferW, L"<Target>%s</Target>\r\n", pITMIda->szTargetSeg);
    fOK = ! UtlBufWriteConv( pITMIda->pBufCB, pITMIda->szBufferW,
                         (USHORT)UTF16strlenBYTE( pITMIda->szBufferW ),
                         TRUE, pITMIda->usSGMLFormat, pITMIda->ulSGMLFormatCP,
                         pITMIda->ulAnsiCP);
  } /* endif */

  if ( fOK )
  {
    fOK = ! UtlBufWriteConv( pITMIda->pBufCB, ITM_ESEGMENT,
                         (USHORT)UTF16strlenBYTE( ITM_ESEGMENT),
                         TRUE, pITMIda->usSGMLFormat, pITMIda->ulSGMLFormatCP,
                         pITMIda->ulAnsiCP);
  } /* endif */

  return fOK;
} /* end of function WriteSegm */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMTmOpen
//------------------------------------------------------------------------------
// Function call:     ITMTmOpen( PITMIDA );
//------------------------------------------------------------------------------
// Description:       This function will open the specified translation
//                    memory and allocate the necessary structures.
//------------------------------------------------------------------------------
// Parameters:        PITMIDA              pointer to ida
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       ERROR_MSG_HANDLED    error message already handled
//                    NO_ERROR             everything went okay
//------------------------------------------------------------------------------
// Side effects:      Specified Translation Memory will be opened
//------------------------------------------------------------------------------
// Function flow:     Allocate the buffers necessary for Input and Output,
//                    if not okay then
//                      set usRc to ERROR_MSG_HANDLED
//                    endif
//                    if okay so far
//                       open TM in NONEXCLUSIVE mode and allow for error handl
//                       set usRc to ERROR_MSG_HANDLED
//                    endif
//                    return
//------------------------------------------------------------------------------

static USHORT
ITMTmOpen
(
  PITMIDA  pITMIda
)
{
  USHORT        usRc = NO_ERROR;
  //ULONG         ulSize;                // size of entry
  PSZ           pTemp;                 // pointer to TM Name
  PSZ           pTempLang = NULL;
  HMODULE       hResMod;
  CHAR chText[80];

   hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   LOADSTRING( NULLHANDLE, hResMod, IDS_ITM_PREPARE, chText);

   WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
               MP1FROMSHORT(-1), chText );
   chText[0] = EOS;
   WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
               MP1FROMSHORT(-2), chText );
   WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
               MP1FROMSHORT(0), NULL );
   pITMIda->usStartSlider = 0;
  // pITMIda->htm = NULLHANDLE;
  /*----------------------------------------------------------------------------
  * Allocation for REP and GET buffers.
  *---------------------------------------------------------------------------*/
  //ulSize = max (sizeof (TMX_GET_IN_W), sizeof (TMX_PUT_IN_W));
  //if ( UtlAlloc( (PVOID *) &pITMIda->pTMIn, 0L, ulSize, ERROR_STORAGE ) )
  //{
  //  ulSize = max (sizeof (TMX_GET_OUT_W), sizeof (TMX_PUT_OUT_W));
  //  if (! UtlAlloc( (PVOID *) &pITMIda->pTMOut, 0L, ulSize, ERROR_STORAGE ))
  //  {
  //       usRc = ERROR_MSG_HANDLED;                 // set dummy return code
  //  } /* endif */
  //}
  //else
  //{
  //   usRc = ERROR_MSG_HANDLED;                      // set dummy return code
  //} /* endif */

  if ( !usRc )
  {
    if (pITMIda->szSourceLang[0] == EOS)
    {
      pTempLang = pITMIda->szSourceLang;
    }

    if ( !UtlCheckIfExist( pITMIda->chTranslMemory, TM_OBJECT ) )
    {
      usRc = TRUE;
      pTemp = pITMIda->chTranslMemory;
      ITMUtlError( pITMIda, ERROR_MEMORY_NOTFOUND, MB_CANCEL, 1, &pTemp, EQF_ERROR);
    }

    /******************************************************************/
    /* check if target language linguistic support is available       */
    /* if not display warning message and tell user that he can go on */
    /* nevertheless the result is not so good.                        */
    /******************************************************************/
    if ( !usRc )
    {
      if ( !pITMIda->fNoAna && !isValidLanguage( pITMIda->szTargetLang, FALSE ))
      {
        USHORT usMBID;
        USHORT usStatus = pITMIda->usStatus;
        pITMIda->usStatus = ITM_STAT_ERROR;
        pTemp = pITMIda->szTargetLang;
        usMBID = ITMUtlError( pITMIda, ITM_LING_SUPPORT_MISSING,
                              MB_YESNO | MB_DEFBUTTON2,
                              1, &pTemp, EQF_ERROR );
        if ( usMBID == MBID_YES )
        {
          /****************************************************************/
          /* if user wants to go on we use the source language            */
          /****************************************************************/
          strcpy( pITMIda->szTargetLang, pITMIda->szSourceLang );
        }
        else
        {
          usRc = TRUE;
        } /* endif */
        pITMIda->usStatus = usStatus;
      } /* endif */
    } /* endif */
//@@#endif
  } /* endif */


  if ( ! pITMIda->fNoTMDB )
  {
    if (usRc==NO_ERROR)
    {
      int iRC = 0;
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      OtmMemory *pMemory = pFactory->openMemory( NULL, pITMIda->chLongTranslMemory, EXCLUSIVE, &iRC );
      usRc = (unsigned short)iRC;
      if(usRc!=NO_ERROR || pMemory==NULL)
      {
          /****************************************************************/
          /* Perform appropriate error handling                           */
          /****************************************************************/
          switch ( usRc )
          {
            case FILE_MIGHT_BE_CORRUPTED:
            case VERSION_MISMATCH:
            case CORRUPT_VERSION_MISMATCH:
            case BTREE_CORRUPTED:
            case BTREE_FILE_NOTFOUND:
              pTemp = pITMIda->chLongTranslMemory;
              ITMClose(pITMIda);
              ITMUtlError( pITMIda, ITM_TM_NEEDS_ORGANIZE,
                           MB_CANCEL, 1, &pTemp,EQF_ERROR );
              break;

            case  BTREE_IN_USE:
              pTemp = pITMIda->chLongTranslMemory;

              ITMUtlError( pITMIda, ERROR_MEM_NOT_ACCESSIBLE,
                           MB_CANCEL, 1, &pTemp,EQF_ERROR );
              break;

            default:
              //MemRcHandling( usRc, pITMIda->chTranslMemory, &pITMIda->htm,
              //               EMPTY_STRING  );
              break;
          } /* endswitch */ 
          usRc = ERROR_MSG_HANDLED;   // set dummy return code

      }//end if
      else // no error
      {  
         pITMIda->pMem = pMemory;
         pITMIda->pProposal = new OtmProposal();
      }
      
    }
  } /* endif */
  return usRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMTmClose
//------------------------------------------------------------------------------
// Function call:     ITMTmClose( PITMIDA )
//------------------------------------------------------------------------------
// Description:       close the translation memory
//------------------------------------------------------------------------------
// Parameters:        PITMIDA              pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       ...                  error message from TM
//                    NO_ERROR             everything went okay
//------------------------------------------------------------------------------
// Function flow:     if TM open then
//                      call TmClose, set return code
//                      if okay then
//                        reset handle to TM
//                      endif
//                    endif
//                    return returncode
//------------------------------------------------------------------------------
static
USHORT  ITMClose
(
   PITMIDA pITMIda                        // pointer to itmida structure
)

{
  USHORT   usRc = NO_ERROR;
  if(pITMIda->pMem != NULL)
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pFactory->closeMemory(pITMIda->pMem);
    pITMIda->pMem = NULL;
  }

  if(pITMIda->pProposal != NULL)
  {
     delete pITMIda->pProposal;
     pITMIda->pProposal = NULL;
  }

  return usRc;
} // end 'ITMTmClose'

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMTmReplace
//------------------------------------------------------------------------------
// Function call:     ITMTMReplace( PITMIDA, ULONG,  PSZ, PSZ );
//------------------------------------------------------------------------------
// Description:       replace the segment in the translation memory
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA        pointer to document ida
//                    PSTEQFSAB            send ahead buffer with segment
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       FALSE                error message from TM
//                    TRUE                 everything went okay
//------------------------------------------------------------------------------
// Function flow:     setup prefix for IN
//                    setup segment
//                    set 'm' flag if alignment was never visualized
//                    call TmReplace for doing the Replace call; set return cod
//                    call WriteSegm if sgmlmem is required
//                    return returncode
//------------------------------------------------------------------------------
static
BOOL ITMTmReplace
(
   PITMIDA   pITMIda,
   PSZ       pszFileName,
   ULONG     ulSegNum,
   PSZ_W     pSrcSeg,
   PSZ_W     pTgtSeg,
   BOOL      fMFlag
)
{
  BOOL   fOK = TRUE;
  unsigned short   usTranslationFlag;
  //PTMX_PUT_IN_W  pstRepIn;
  //PTMX_PUT_OUT_W pstRepOut;

  if ( ! pITMIda->fNoTMDB && pITMIda->pProposal )
  {
    pITMIda->pProposal->clear();
    //pstRepIn  = (PTMX_PUT_IN_W) pITMIda->pTMIn;
    //pstRepOut = (PTMX_PUT_OUT_W) pITMIda->pTMOut;
    //memset (pstRepIn, 0, sizeof (TMX_PUT_IN_W));
    //memset (pstRepOut, 0, sizeof (TMX_PUT_OUT_W));

    EQFBBufRemoveTRNote(pSrcSeg, pITMIda->pLoadedTable,
                        pITMIda->TBSourceDoc.pfnUserExit,
                        pITMIda->TBSourceDoc.pfnUserExitW,
                        pITMIda->TBSourceDoc.ulOemCodePage);
    EQFBBufRemoveTRNote(pTgtSeg, pITMIda->pLoadedTable,
                        pITMIda->TBTargetDoc.pfnUserExit,
                        pITMIda->TBTargetDoc.pfnUserExitW,
                        pITMIda->TBTargetDoc.ulOemCodePage);

    /*******************************************************************/
   /* delete softlf's if have been added                              */
   /*******************************************************************/
   if (pITMIda->TBSourceDoc.fLineWrap && pITMIda->TBSourceDoc.fAutoLineWrap )
   {
     USHORT  i;
     USHORT  usSegOffset = 0;
     EQFBBufRemoveSoftLF(pITMIda->TBSourceDoc.hwndRichEdit, pSrcSeg, &i, &usSegOffset);
     usSegOffset = 0;
     EQFBBufRemoveSoftLF(pITMIda->TBSourceDoc.hwndRichEdit, pTgtSeg, &i, &usSegOffset);
   }

    if (IsDBCS_CP(pITMIda->TBSourceDoc.ulOemCodePage)
        || IsDBCS_CP(pITMIda->TBTargetDoc.ulOemCodePage) )
    {
      ULONG len;
      wchar_t *pszSrc = NULL;
      wchar_t *pszDst = NULL;

      len = UTF16strlenCHAR( pSrcSeg ) + 1;
      pszSrc = new wchar_t[len];
      if(pszSrc != NULL)
      {  
         EQFBUtlConvertSOSI( pszSrc, &len,
                          pSrcSeg, DELETE_SOSI, pITMIda->TBSourceDoc.ulOemCodePage );
         pITMIda->pProposal->setSource(pszSrc);
         delete []pszSrc;
      }

      len = UTF16strlenCHAR( pTgtSeg ) + 1;
      pszDst = new wchar_t[len];
      if(pszDst != NULL)
      {
         EQFBUtlConvertSOSI( pszDst, &len,
                          pTgtSeg, DELETE_SOSI, pITMIda->TBTargetDoc.ulOemCodePage );
         pITMIda->pProposal->setTarget(pszDst);
         delete []pszDst;
      }
    }
    else
    {
       pITMIda->pProposal->setSource(pSrcSeg);
       pITMIda->pProposal->setTarget(pTgtSeg);
    } /* endif */

    //strcpy(pstRepIn->stTmPut.szFileName, pITMIda->chShortSrcFName );
    pITMIda->pProposal->setDocShortName(pITMIda->chShortSrcFName);

    if ( !pszFileName )
    {
      pszFileName = pITMIda->chSourceFile;
    } /* endif */
    if (*pszFileName == BACKSLASH)
    {
      //strcpy (pstRepIn->stTmPut.szLongName, pszFileName + 1);
      pITMIda->pProposal->setDocName(pszFileName + 1);
    }
    else
    {
      //strcpy(pstRepIn->stTmPut.szLongName, pszFileName );
      pITMIda->pProposal->setDocName(pszFileName);
    } /* endif */

#ifdef RAS400_ITM
    usTranslationFlag = fMFlag ? TRANSLFLAG_MACHINE : TRANSLFLAG_NORMAL;
#else
    usTranslationFlag = pITMIda->fVisual ? TRANSLFLAG_NORMAL : TRANSLFLAG_MACHINE;
#endif

    switch( usTranslationFlag )
    {
      case TRANSL_SOURCE_MANUAL: pITMIda->pProposal->setType( OtmProposal::eptManual ); break;
      case TRANSL_SOURCE_MANCHINE: pITMIda->pProposal->setType( OtmProposal::eptMachine ); break;
      case TRANSL_SOURCE_GLOBMEMORY: pITMIda->pProposal->setType( OtmProposal::eptGlobalMemory ); break;
      default: pITMIda->pProposal->setType( OtmProposal::eptUndefined ); break;
    }

    //pstRepIn->stTmPut.ulSourceSegmentId = ulSegNum;
    pITMIda->pProposal->setSegmentNum(ulSegNum);
    //strcpy( pstRepIn->stTmPut.szTagTable, pITMIda->chTagTableName );
    pITMIda->pProposal->setMarkup(pITMIda->chTagTableName);
    //strcpy( pstRepIn->stTmPut.szSourceLanguage, pITMIda->szSourceLang );
    //strcpy( pstRepIn->stTmPut.szTargetLanguage, pITMIda->szTargetInputLang );
    pITMIda->pProposal->setSourceLanguage(pITMIda->szSourceLang );
    pITMIda->pProposal->setTargetLanguage(pITMIda->szTargetInputLang );

    //fOK = !TmReplaceW(pITMIda->htm,              // Memory database handle
    //                  pITMIda->chTranslMemory,   // Full translation memory path
    //                  pstRepIn,                  // Pointer to the REP_IN structure
    //                  pstRepOut,                 // Pointer to the REP_OUT structure
    //                  FALSE );                   // Message handling parameter
    pITMIda->pMem->putProposal(*(pITMIda->pProposal));

  } /* endif */

  if ( fOK && pITMIda->fSGMLITM )
  {
    fOK = WriteSegm ( pITMIda, ulSegNum );
  } /* endif */

  return fOK;
} // end 'ITMTmReplace'

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMCleanUp
//------------------------------------------------------------------------------
// Function call:     ITMCleanUp( usTermCode );
//------------------------------------------------------------------------------
// Description:       cleanup in case of Trap D's or other akward problems
//------------------------------------------------------------------------------
// Parameters:        USHORT  usTermCode   termination code (not used but
//                                          required by DosExitList )
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Prerequesits:      ITMCleanUp has to be added to the DosExitList ....
//------------------------------------------------------------------------------
// Function flow:     if TranslMemory open
//                      Close it
//                    go ahead in chain with next DosExitList
//------------------------------------------------------------------------------
//
//VOID PASCAL FAR ITMCleanUp
//(
//   USHORT usTermCode                   // termination code
//)
//
//{
//  usTermCode;                          // avoid compiler warnings
//
//  /********************************************************************/
//  /* if translation memory still open close it.                       */
//  /* This has to be done with a static pointer to get access to the   */
//  /* handles...                                                       */
//  /********************************************************************/
//  if ( pStaticITMIda )
//  {
//    if ( pStaticITMIda->htm )
//    {
//      TmClose( pStaticITMIda->htm, pStaticITMIda->chTranslMemory, FALSE, 0 );
//    } /* endif */
//
//    if ( pStaticITMIda->pBufCB )
//    {
//      UtlBufClose( pStaticITMIda->pBufCB, FALSE );
//    } /* endif */
//
//    pStaticITMIda = NULL;
//  } /* endif */
//
//
//#ifdef ITMTEST
//  if ( fOut )
//  {
//    fclose( fOut );
//  } /* endif */
//
//#endif
//
//  DosExitList(EXLST_EXIT, 0L);        /* termination complete     */
//} /* end of function ITMCleanUp */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PrepareNextBlock
//------------------------------------------------------------------------------
// Function call:     PrepareNextBlock( pITMIda, usNum );
//------------------------------------------------------------------------------
// Description:       init and fill block structure with next block
//------------------------------------------------------------------------------
// Parameters:        PITMIDA pITMIda        ITM ida
//                    USHORT  usNum          number of block to be used
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE  if success
//                    FALSE otherwise
//------------------------------------------------------------------------------
// Function flow:     init used elements of ITMIda
//                    fill the blocks(GetSegmentBlock)
//                    return success indicator
//------------------------------------------------------------------------------

static BOOL
PrepareNextBlock
(
  PITMIDA pITMIda,                     // ITM ida
  ULONG   ulNum                        // number of block to be used
)
{
  BOOL  fOK = TRUE;                    // success indicator
  LONG  lLen;

  /********************************************************************/
  /* init used elements                                               */
  /********************************************************************/
  lLen = sizeof(SHORT) * pITMIda->itmSrcText.ulUsed;
  memset(pITMIda->itmSrcText.psPara, 0, lLen );
  memset(pITMIda->itmSrcText.pulSegStart, 0, lLen );
  memset(pITMIda->itmSrcText.pulSegEnd, 0, lLen );
  pITMIda->itmSrcText.ulUsed = 0;
  lLen = sizeof(SHORT) * pITMIda->itmTgtText.ulUsed;
  memset(pITMIda->itmTgtText.psPara, 0, lLen );
  memset(pITMIda->itmTgtText.pulSegStart, 0, lLen );
  memset(pITMIda->itmTgtText.pulSegEnd, 0, lLen );
  pITMIda->itmTgtText.ulUsed = 0;

  /********************************************************************/
  /* now fill the blocks ...                                          */
  /********************************************************************/
  fOK = GetSegmentBlock( pITMIda, &pITMIda->TBSourceDoc,
                         &pITMIda->itmSrcText,
                         pITMIda->itmSrcNop.pulSegs[ulNum],
                         pITMIda->itmSrcNop.pulSegs[ulNum+1]);
  if ( fOK )
  {
    fOK = GetSegmentBlock( pITMIda, &pITMIda->TBTargetDoc,
                           &pITMIda->itmTgtText,
                           pITMIda->itmTgtNop.pulSegs[ulNum],
                           pITMIda->itmTgtNop.pulSegs[ulNum+1]);
  } /* endif */
  return ( fOK );
} /* end of function PrepareNextBlock */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     GetNumberOfToBe
//------------------------------------------------------------------------------
// Function call:     usNum = GetNumberOfToBe( pDoc );
//------------------------------------------------------------------------------
// Description:       gets the number of segments to be translated
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc    pointer to document struct
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       number of segments to be translated
//------------------------------------------------------------------------------
// Function flow:     loop through all segments and count the ToBe
//------------------------------------------------------------------------------
static ULONG
GetNumberOfToBe
(
  PTBDOCUMENT pDoc
)
{
  ULONG   ulTransSeg;                  // segments to be translated
  ULONG   ulSegNum;                    // segment number
  PTBSEGMENT pSeg;                     // pointer to segment

  /********************************************************************/
  /* find number of TOBE segments   in Segmented Source               */
  /********************************************************************/
  ulSegNum =  1;
  ulTransSeg = 0;
  pSeg = EQFBGetSegW(pDoc, ulSegNum);
  while ( pSeg)
  {
//    if ( (pSeg->qStatus == QF_TOBE) || (pSeg->qStatus == QF_XLATED) )
    if ( (pSeg->qStatus == QF_TOBE) || (pSeg->qStatus == QF_XLATED)
            || (pSeg->qStatus == QF_ATTR))
    {
      ulTransSeg++;
    } /* endif */
    ulSegNum++;
    pSeg = EQFBGetSegW(pDoc, ulSegNum);
  } /* endwhile */

  return (ulTransSeg);
} /* end of function GetNumberOfToBe */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     GetSegmentBlock
//------------------------------------------------------------------------------
// Function call:     GetSegmentBlock( pDoc, pITMParaAlign, pITMAlign, usNum);
//------------------------------------------------------------------------------
// Description:       get segments of selected paragraph to be aligned
//------------------------------------------------------------------------------
//                    PITMIDA     pITMIda
// Parameters:        PTBDOCUMENT pDoc,              ptr to doc instance
//                    PITMALIGN   pITMParaAlign      ptr to paragraph struct
//                    PITMALIGN   pITMAlign,         ptr to alignment struct
//                    USHORT      usNum              number of block to take
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE      if success
//                    FALSE     else
//------------------------------------------------------------------------------
// Function flow:     get segment number of start and end segment
//                    while not at end of block
//                     switch (pSeg->qStatus)
//                       case NOP : combine all nop segments
//                       case QF_CROSSED_OUT or OVERCROSS: do not add
//                             these segments to block
//                       default: count length of unprotected parts in segment
//                           add segment to block
//                     endswitch
//                   endwhile
//                    return success indicator
//------------------------------------------------------------------------------
BOOL
GetSegmentBlock
(
   PITMIDA     pITMIda,
   PTBDOCUMENT pDoc,                   // document ida
   PITMALIGN   pITMAlign,              // aligned segments for selected par.
   ULONG       ulStartSeg,
   ULONG       ulEndSeg
)
{
  ULONG ulTempSeg;                     // temporary segment number
  PTBSEGMENT pSeg;                     // pointer to segment
  ULONG  ulLen;                        // length indication
  BOOL   fOK = TRUE;                   // success indicator
  USHORT usColPos = 0;
  PSTARTSTOP  pstCurrent;             // ptr to entries of start/stop table
  USHORT usRC;
  /********************************************************************/
  /* get segment number of start and end segment                      */
  /********************************************************************/
  if ( ulStartSeg == 0 )
  {
    ulLen = 0;
    ulStartSeg++;
  } /* endif */
  /********************************************************************/
  /* fill structures into provided block                              */
  /********************************************************************/
  pSeg = EQFBGetVisSeg(pDoc, &ulStartSeg);
  while ( pSeg && fOK && ulStartSeg <= ulEndSeg )
  {
    /******************************************************************/
    /* attributes should be taken as nops                             */
    /******************************************************************/
    switch ( pSeg->qStatus )
    {
      case  QF_NOP:
      case  QF_NOP_ANCHOR_1:
      case  QF_NOP_ANCHOR_2:
      case  QF_NOP_ANCHOR_3:
          /****************************************************************/
          /* combine all NOP segments ....                                */
          /****************************************************************/
          ulLen = 0;
          ulTempSeg = ulStartSeg;
          while ( (pSeg && ulTempSeg <= ulEndSeg) &&
             ( (pSeg->qStatus == QF_NOP)
             || (pSeg->qStatus == QF_NOP_ANCHOR_1)
             || (pSeg->qStatus == QF_NOP_ANCHOR_2)
             || (pSeg->qStatus == QF_NOP_ANCHOR_3)
             || (pSeg->qStatus == QF_OVERCROSS ) ) )
          {
            ulLen += pSeg->usLength;
            ulTempSeg++;                         // point to next segment
            pSeg = EQFBGetVisSeg(pDoc, &ulTempSeg);
          } /* endwhile */
          ulTempSeg --;                          // we've gone 1 too far

          ulStartSeg = ulTempSeg;                // reset to new start segment
        break;
      case  QF_CROSSED_OUT:
      case  QF_CROSSED_OUT_NOP:
      case  QF_OVERCROSS:
        /**************************************************************/
        /* do not add crossed out segments to block                   */
        /**************************************************************/
        break;
      default :
         ulLen = 0;
         if ( pSeg->pusBPET == NULL )
         {
           usRC = TACreateProtectTableW( pSeg->pDataW,
                                       ((PLOADEDTABLE)pITMIda->pLoadedTable),
                                       usColPos,
                                       (PTOKENENTRY) pDoc->pTokBuf,
                                       TOK_BUFFER_SIZE,
                                       (PSTARTSTOP *) &(pSeg->pusBPET),
                                       pDoc->pfnUserExit,
                                       pDoc->pfnUserExitW,
                                       pDoc->ulOemCodePage);
         } /*endif*/
         if ( pSeg->pusBPET)
         {
           // add lengths in unprotected parts
           pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
           while ( pstCurrent->usType != 0 )
           {
              if ( pstCurrent->usType == UNPROTECTED_CHAR )
              {
                ulLen += pstCurrent->usStop - pstCurrent->usStart + 1;
              } /* endif */
              pstCurrent++;
           } /* endwhile */
         }
         else
         {
           ulLen = pSeg->usLength;
         } /* endif */
         // MIF: TOBE segment has only protected chars!!
         //thus : minimal length = 1 per default; fix 4.4.95
         if (!ulLen )                                             /* Pr70 */
         {                                                        /* Pr70 */
           ulLen = 1;                                             /* Pr70 */
         }
         fOK = AddToBlock( pITMAlign, ulStartSeg, ulStartSeg, ulLen );
        break;
    } /* endswitch */

    ulStartSeg++;                         // point to next segment
    pSeg = EQFBGetVisSeg(pDoc, &ulStartSeg);
  } /* endwhile */

  return ( fOK );
} /* end of function GetSegmentBlock */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     CreateFolderStruct
//------------------------------------------------------------------------------
// Function call:     CreateFolderStruct
//------------------------------------------------------------------------------
// Description:       Create a temporary folder structure to allow for use
//                    of text analysis
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda
//                    PSZ      pFolderName
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     structure was created
//                    FALSE    structure could not be created
//------------------------------------------------------------------------------
// Function flow:     get system drive
//                    create folder name
//                    make the requested directories
//                    return success indicator
//------------------------------------------------------------------------------
static BOOL
CreateFolderStruct
(
  PITMIDA pITMIda,
  PSZ     pFolderName                  // folder name
)
{
  CHAR szDrive[ MAX_DRIVE ];           // variable for getting system drive
  CHAR szTempDir[ MAX_EQF_PATH ];      // temp. directory
  BOOL fOK;                            // success indicator
  PPROPFOLDER     pProp = NULL;
  EQFINFO         ErrorInfo;
  HPROP           hProp = NULL;        // handle to properties
  PSZ             pErrParm = NULL;            // pointer to error parameter
  BOOL            fSrcFolder = TRUE;
  CHAR            chFolderName[MAX_FILESPEC];
  CHAR            chTempName[MAX_FILESPEC];

  DWORD dwProcessID = GetCurrentProcessId();

  fSrcFolder = strcmp(pFolderName, TEMPFOLDERTGTNEW);

  UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
  // keep "$$Sxxxxx.FOO, $$Txxxxx.F00" format for ITM folder!!
  strcpy(chFolderName, pFolderName);
  chFolderName[3] = EOS;

  sprintf(chTempName, "%s%5.5X.F00", chFolderName, dwProcessID);
  UtlMakeEQFPath( szTempDir, szDrive[0], DIRSOURCEDOC_PATH, chTempName );

  if (fSrcFolder)
  {
    strcpy(pITMIda->chITMSFolder, chTempName);
  }
  else
  {
	  strcpy(pITMIda->chITMTFolder, chTempName);
  }
  fOK =  ! UtlMkMultDir( szTempDir, TRUE);

  if ( fOK )
  {
    UtlMakeEQFPath( szTempDir, szDrive[0],DIRSEGSOURCEDOC_PATH, chTempName );
    fOK = ! UtlMkMultDir( szTempDir, TRUE);
  } /* endif */
  if ( fOK )
  {
    UtlMakeEQFPath( szTempDir, szDrive[0],DIRSEGTARGETDOC_PATH, chTempName );
    fOK = ! UtlMkMultDir( szTempDir, TRUE);
  } /* endif */
  if ( fOK )
  {
    UtlMakeEQFPath( szTempDir, szDrive[0],PROPERTY_PATH, chTempName );
    fOK = ! UtlMkMultDir( szTempDir, TRUE);
  } /* endif */

  /********************************************************************/
  /* create folder properties                                         */
  /********************************************************************/
  if ( fOK )
  {
    fOK =  UtlAlloc( (PVOID *) &pProp, 0L, (LONG) sizeof( *pProp), ERROR_STORAGE );
  } /* endif */
  if ( fOK )
  {
    strcpy( pProp->PropHead.szName, chTempName );
    UtlMakeEQFPath( pProp->PropHead.szPath, szDrive[0], SYSTEM_PATH, NULL );
    pProp->chDrive = szDrive[0];

    if (fSrcFolder /*!strcmp(pFolderName, TEMPFOLDERSRC)*/ )
    {
      strcpy( pProp->szSourceLang, pITMIda->szSourceLang );
      strcpy( pProp->szTargetLang, pITMIda->szTargetLang );
    }
    else
    {
      strcpy( pProp->szSourceLang, pITMIda->szTargetLang );
      strcpy( pProp->szTargetLang, pITMIda->szSourceLang );
    } /* endif */

    strcpy( pProp->szLongMemory, pITMIda->chLongTranslMemory );
    pErrParm = UtlGetFnameFromPath( pITMIda->chTranslMemory );
    if ( pErrParm )
    {
      strcpy( pProp->szMemory, pErrParm );
    } /* endif */

    strcpy( pProp->szFormat, pITMIda->chTagTableName );

    strcat( strcpy( szTempDir, pProp->PropHead.szPath), "\\");
    strcat( szTempDir, pProp->PropHead.szName);
    pProp->PropHead.usClass = PROP_CLASS_FOLDER;

    hProp = CreateProperties( szTempDir, NULL, PROP_CLASS_FOLDER, &ErrorInfo);

    if ( !hProp )
    {
      /****************************************************************/
      /* something might be over from last time - delete it and try   */
      /* again....                                                    */
      /****************************************************************/
      DeleteProperties( szTempDir, NULL, &ErrorInfo );
      hProp = CreateProperties( szTempDir, NULL, PROP_CLASS_FOLDER, &ErrorInfo);
    } /* endif */
    if( !hProp)
    {
      pErrParm = pProp->PropHead.szName;
      ITMUtlError( pITMIda, ERROR_CREATE_PROPERTIES, MB_CANCEL,
                   1, &pErrParm, EQF_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */


  if ( fOK  )
  {
    if( PutAllProperties( hProp, pProp, &ErrorInfo)
        || SaveProperties( hProp, &ErrorInfo))
    {
      ITMUtlError( pITMIda, ERROR_WRITE_PROPERTIES,
                   MB_CANCEL, 0, NULL, EQF_ERROR );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( pProp )
  {
    UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
  } /* endif */
  if ( hProp )
  {
    CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
  } /* endif */

  return ( fOK );
} /* end of function CreateFolderStruct */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     RemoveFolderStruct
//------------------------------------------------------------------------------
// Function call:     RemoveFolderStruct();
//------------------------------------------------------------------------------
// Description:       Remove the directory structure created
//------------------------------------------------------------------------------
// Parameters:        PSZ  pFolderName          name of folder to be removed
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get the system drive
//                    create root directory of temporary folder and remove it
//------------------------------------------------------------------------------
static VOID
RemoveFolderStruct
(
  PSZ  pFolderName                     // pointer to folder name
)
{
  CHAR szDrive[ MAX_DRIVE ];           // variable for getting system drive
  CHAR szTempDir[ MAX_EQF_PATH ];      // temp. directory
  EQFINFO         ErrorInfo;


  UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
  if (pFolderName && *pFolderName)
  {
     UtlMakeEQFPath( szTempDir, szDrive[0], SYSTEM_PATH, pFolderName );
     UtlRemoveDir( szTempDir, FALSE );

     DeleteProperties( szTempDir, NULL, &ErrorInfo );
  }
  else
  {
	  USHORT usI = 1;
	  usI = usI + 1;
	  //should not happen!! if Foldername is empty,
	  // total contents  of eqf dir. would be deleted!
  }

} /* end of function RemoveFolderStruct */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PrepFiles
//------------------------------------------------------------------------------
// Function call:     PrepFiles( pITMIda )
//------------------------------------------------------------------------------
// Description:       prepare the files for processing, i.e. if files are not
//                    analysed copy them into temp. folder and analyse them
//------------------------------------------------------------------------------
// Parameters:        PITMIDA pITMIda
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE  success
//                    FALSE else
//------------------------------------------------------------------------------
// Function flow:     if specified files already ananlysed
//                      copy only file names
//                    else
//                      build temp name and copy file to it
//                      analyse source file
//                      if ok
//                        build temp name and copy target file under it
//                      endif
//                      if ok
//                        analyse file
//                      endif
//                    endif
//------------------------------------------------------------------------------
static BOOL
PrepFiles
(
  PITMIDA pITMIda
)
{
  BOOL fOK = TRUE;                     // success indicator
  PSZ  pTempName;                      // pointer to temp. filename
  PSZ  pSrcFile;                       // pointer to source filename
  PSZ  pTgtFile;                       // pointer to source filename
  PSZ  pSrcFileOrg;
  PSZ  pTgtFileOrg;
  CHAR chTempName[ MAX_EQF_PATH ];     // temp. filename
  CHAR szDrive[ MAX_DRIVE ];           // drive
  CHAR chShortFName[ MAX_FILESPEC ];   // tempory short filename
  CHAR chText[80];
  HPROP        hpropRc;                //return form create properties
  ULONG        ulErrorInfo;
  HMODULE hResMod;

  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  LOADSTRING( NULLHANDLE, hResMod, IDS_ITM_PREPAREENV, chText);
  WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
              MP1FROMSHORT(-1), chText );
  pITMIda->usStartSlider = 0;
  /********************************************************************/
  /* check if source and target file exists ...                       */
  /********************************************************************/
  if ( !ITMFileExist( pITMIda->chSourceFile ) )                       /* @CA7 */
  {                                                                   /* @CA7 */
    fOK = FALSE;                                                      /* @CA7 */
    pTempName = pITMIda->chSourceFile;                                /* @CA7 */
  } /* endif */                                                       /* @CA7 */
  if ( !fOK )
  {
     ITMUtlError( pITMIda, FILE_NOT_EXISTS, MB_CANCEL, 1, &pTempName, EQF_ERROR );
  }                                                                    /* @CA7 */
  if ( fOK && !ITMFileExist( pITMIda->chTargetFile ) )                /* @CA7 */
  {                                                                   /* @CA7 */
    fOK = FALSE;                                                      /* @CA7 */
    pTempName = pITMIda->chTargetFile;                                /* @CA7 */
  } /* endif */                                                       /* @CA7 */

  if ( !fOK )
  {
    ITMUtlError( pITMIda, FILE_NOT_EXISTS, MB_CANCEL, 1, &pTempName, EQF_ERROR );
  }
  else
  {
    /**************************************************************/
    /* if specified files not analysed                            */
    /* we have to copy the files and analyse them                 */
    /**************************************************************/
    if ( pITMIda->fNoAna )
    {
        strcpy(pITMIda->chSegSourceFile, pITMIda->chSourceFile);
        strcpy(pITMIda->chSegTargetFile, pITMIda->chTargetFile);
    }
    else
    {
      /******************************************************************/
      /* build the temp name and copy the file to it......              */
      /******************************************************************/
      pTempName = chTempName;

      UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
      UtlMakeEQFPath( pTempName, szDrive[0], SYSTEM_PATH, pITMIda->chITMSFolder );
      strcat( pTempName, BACKSLASH_STR );

      pSrcFile = UtlGetFnameFromPath( pITMIda->chSourceFile );
      if ( !pSrcFile )
      {
        pSrcFile = pITMIda->chSourceFile;
      } /* endif */

    pSrcFileOrg = pSrcFile;

      // handle long or short file name
      if ( UtlIsLongFileName( pSrcFile ))
      {
        BOOL fIsNew;                   // new document flag

        // get the correct short file name for the document
        FolLongToShortDocName( pTempName,
                               pSrcFile,
                               chShortFName,
                               &fIsNew );
        pSrcFile = chShortFName;
      } /* endif */

      strcat( pTempName, pSrcFile );
      strcpy( pITMIda->chShortSrcFName, pSrcFile );  // remember filename

      hpropRc = CreateProperties( pTempName, NULL,
                                  PROP_CLASS_DOCUMENT, &ulErrorInfo );
      if ( !hpropRc )
      {
        /****************************************************************/
        /* something might be over from last time - delete it and try   */
        /* again....                                                    */
        /****************************************************************/
        DeleteProperties( pTempName, NULL, &ulErrorInfo );
        hpropRc = CreateProperties( pTempName, NULL,
                                    PROP_CLASS_DOCUMENT, &ulErrorInfo );
      } /* endif */

      if ( !hpropRc )               //error from property handler
      {
        //set fOK to FALSE
        fOK = FALSE;
        ITMUtlError( pITMIda, ERROR_CREATE_PROP, MB_CANCEL, 1,
                     &pSrcFile, EQF_ERROR );
      }
      else
      {
         PPROPDOCUMENT pDocProp;
         PSZ pszLongName = pSrcFileOrg;

           // store long name in properties (use name from old properties
           // for existing documents)
         pDocProp = (PPROPDOCUMENT) MakePropPtrFromHnd( hpropRc );
         if ( pITMIda->szSrcStartPath[0] != EOS )
         {
           pszLongName = pITMIda->chSourceFile + strlen(pITMIda->szSrcStartPath);
         } /* endif */
         strcpy( pDocProp->szLongName, pszLongName );

         CloseProperties( hpropRc, PROP_FILE, &ulErrorInfo );
         UtlMakeEQFPath( pTempName, szDrive[0],
                         DIRSOURCEDOC_PATH, pITMIda->chITMSFolder );
         strcat( pTempName, BACKSLASH_STR );
         strcat( pTempName, pSrcFile );
         fOK = ! UtlCopy( pITMIda->chSourceFile, pTempName, 1, 0L, TRUE );
         if ( fOK )
         {
           UtlSetFileMode( pTempName, FILE_NORMAL, 0L, FALSE );
         } /* endif */
      } /* endif */

      if ( fOK )
      {
        /******************************************************************/
        /* build the temp name and copy the target file under it....      */
        /******************************************************************/
        UtlMakeEQFPath( pTempName, szDrive[0], SYSTEM_PATH, pITMIda->chITMTFolder );
        strcat( pTempName, BACKSLASH_STR );
        pTgtFile = UtlGetFnameFromPath( pITMIda->chTargetFile );
        if ( !pTgtFile )
        {
          pTgtFile = pITMIda->chTargetFile;
        } /* endif */
    pTgtFileOrg = pTgtFile;

        // handle long or short file name
        if ( UtlIsLongFileName( pTgtFile ))
        {
          BOOL fIsNew;                   // new document flag

          // get the correct short file name for the document
          FolLongToShortDocName( pTempName,
                                 pTgtFile,
                                 chShortFName,
                                 &fIsNew );
          pTgtFile = chShortFName;
        } /* endif */

        strcat( pTempName, pTgtFile );
        strcpy( pITMIda->chShortTgtFName, pTgtFile );      // remember filename

        hpropRc = CreateProperties( pTempName, NULL,
                                    PROP_CLASS_DOCUMENT, &ulErrorInfo );
        if ( !hpropRc )
        {
          /****************************************************************/
          /* something might be over from last time - delete it and try   */
          /* again....                                                    */
          /****************************************************************/
          DeleteProperties( pTempName, NULL, &ulErrorInfo );
          hpropRc = CreateProperties( pTempName, NULL,
                                      PROP_CLASS_DOCUMENT, &ulErrorInfo );
        } /* endif */

        if ( !hpropRc )               //error from property handler
        {
          //set fOK to FALSE
          fOK = FALSE;
          ITMUtlError( pITMIda, ERROR_CREATE_PROP, MB_CANCEL, 1,
                       &pTgtFile, EQF_ERROR );
        }
        else
        {

           PPROPDOCUMENT pDocProp;

           // store long name in properties (use name from old properties
           // for existing documents)
           pDocProp = (PPROPDOCUMENT) MakePropPtrFromHnd( hpropRc );
           strcpy( pDocProp->szLongName, pTgtFileOrg );

           CloseProperties( hpropRc, PROP_FILE, &ulErrorInfo );
           UtlMakeEQFPath( pTempName, szDrive[0],
                           DIRSOURCEDOC_PATH, pITMIda->chITMTFolder );
           strcat( pTempName, BACKSLASH_STR );
           strcat( pTempName, pTgtFile );
           fOK = ! UtlCopy( pITMIda->chTargetFile, pTempName, 1, 0L, TRUE );
           if ( fOK )
           {
             UtlSetFileMode( pTempName, FILE_NORMAL, 0L, FALSE );
           } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

  } /* endif */

  return( fOK );
}

/**********************************************************************/
/* analyse the files (if needed)...                                   */
/**********************************************************************/
static BOOL
ITMAnalyseFiles
(
  PITMIDA pITMIda
)
{
  BOOL fOK = TRUE;
  PSZ  pSrcFile;                       // pointer to source filename
  PSZ  pTgtFile;                       // pointer to source filename
  CHAR chTempName[ MAX_EQF_PATH ];     // temp. filename
  CHAR szDrive[ MAX_DRIVE ];           // variable for getting system drive
  CHAR chTextBuf[40];
  /******************************************************************/
  /* analyse the source file ....                                   */
  /******************************************************************/
  if ( !IsPrepared(pITMIda, &(pITMIda->usNumPrepared)) && !pITMIda->fNoAna)
  {
    HMODULE hResMod;
    hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
    UtlMakeEQFPath( chTempName, szDrive[0], SYSTEM_PATH, pITMIda->chITMSFolder );
    pSrcFile = UtlGetFnameFromPath( pITMIda->chSourceFile );
    if ( !pSrcFile )
    {
      pSrcFile = pITMIda->chSourceFile;
    } /* endif */
    LOADSTRING( NULLHANDLE, hResMod, IDS_ITM_SOURCEFILE, chTextBuf);
    sprintf( pITMIda->szBuffer, chTextBuf, pSrcFile );
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(-2), pITMIda->szBuffer );
    pITMIda->usStartSlider = 0;

    fOK = EQFTextSegm( pITMIda,                      // pointer to ida
                       pITMIda->hwnd,                // window handle
                       chTempName,                   // folder name
                       pITMIda->chShortSrcFName);    // input file name

    if ( fOK )
    {
      UtlMakeEQFPath( chTempName, szDrive[0], SYSTEM_PATH, pITMIda->chITMTFolder );
      pTgtFile = UtlGetFnameFromPath( pITMIda->chTargetFile );
      if ( !pTgtFile )
      {
        pTgtFile = pITMIda->chTargetFile;
      } /* endif */

      LOADSTRING( NULLHANDLE, hResMod, IDS_ITM_TARGETFILE, chTextBuf);
      sprintf( pITMIda->szBuffer, chTextBuf, pTgtFile );
      WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                  MP1FROMSHORT(-2), pITMIda->szBuffer );
      pITMIda->usStartSlider = 0;

      fOK = EQFTextSegm( pITMIda,
                         pITMIda->hwnd,                // window handle
                         chTempName,                   // folder name
                         pITMIda->chShortTgtFName );   // input file name

    } /* endif */
    /********************************************************************/
    /* store the file names of the segmented file if segmentation       */
    /* was okay...                                                      */
    /* Be aware that the directory has to be the SSOURCE                */
    /********************************************************************/
    if ( fOK )
    {
      UtlMakeEQFPath( chTempName, szDrive[0],
                      DIRSEGSOURCEDOC_PATH, pITMIda->chITMSFolder );
      strcat( chTempName, BACKSLASH_STR );

      strcpy(pITMIda->chSegSourceFile, chTempName);
      strcat( pITMIda->chSegSourceFile, pITMIda->chShortSrcFName );

      UtlMakeEQFPath( chTempName, szDrive[0],
                      DIRSEGSOURCEDOC_PATH, pITMIda->chITMTFolder );
      strcat( chTempName, BACKSLASH_STR );
      strcpy(pITMIda->chSegTargetFile, chTempName);
      strcat( pITMIda->chSegTargetFile, pITMIda->chShortTgtFName );
    } /* endif */
  } /* endif */
  return fOK;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     RemoveFiles
//------------------------------------------------------------------------------
// Function call:     RemoveFiles( pITMIda );
//------------------------------------------------------------------------------
// Description:       remove any temporary build files
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda   pointer to instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if analyse was requested
//                      find primary drive
//                      build temp names and delete the documents for
//                      the source and the target.
//------------------------------------------------------------------------------
static VOID
RemoveFiles
(
  PITMIDA pITMIda                // pointer to ida
)
{
  PSZ  pTempName;                      // pointer to temp. filename
  CHAR chTempName[ MAX_EQF_PATH ];     // temp. filename
  CHAR szDrive[ MAX_DRIVE ];           // drive


  /**************************************************************/
  /* if we have copied the files, try to remove them ....       */
  /**************************************************************/
  if ( ! pITMIda->fNoAna )
  {
    USHORT usDummy;

    /******************************************************************/
    /* find the primary drive ....                                    */
    /******************************************************************/
    UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
    /******************************************************************/
    /* build the temp names and delete the files                      */
    /******************************************************************/
    pTempName = chTempName;

    UtlMakeEQFPath( pTempName, szDrive[0],
                    SYSTEM_PATH, pITMIda->chITMSFolder  );

    strcat( pTempName, BACKSLASH_STR );

    strcat( pTempName, pITMIda->chShortSrcFName );
    usDummy = 0;
    DocumentDelete( pTempName, FALSE, &usDummy );

    UtlMakeEQFPath( pTempName, szDrive[0],
                    SYSTEM_PATH, pITMIda->chITMTFolder );

    strcat( pTempName, BACKSLASH_STR );
    strcat( pTempName, pITMIda->chShortTgtFName );
    usDummy = 0;
    DocumentDelete( pTempName, FALSE, &usDummy );

  } /* endif */
} /* end of function RemoveFiles */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMHandlers
//------------------------------------------------------------------------------
// Function call:     ITMHandlers( hab );
//------------------------------------------------------------------------------
// Description:       start all handlers to simulate the TranslationManager
//                    environment.
//------------------------------------------------------------------------------
// Parameters:        HAB  hab    Anchor block handle
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   handlers could be started
//                    FALSE  something went wrong
//------------------------------------------------------------------------------
// Prerequesits:      TranslationManager has to be installed
//------------------------------------------------------------------------------
// Function flow:     start the following handlers:
//                       OBJECTMANAGER, PROPERTYHANDLER, FOLDERLISTHANDLER,
//                       FOLDERHANDLER, DOCUMENTHANDLER, ANALYSISHANDLER,
//                       MEMORYHANDLER
//                    display error message if something went wrong.
//                    return success indicator
//------------------------------------------------------------------------------

BOOL ITMHandlers
(
  HAB  hab
)
{
  BOOL  fOK;                           // success indicator
  fOK = TWBInit( hab );

  /************************************************************/
  /* set initial EQF drive list                               */
  /************************************************************/
  if ( fOK )
  {
    PPROPSYSTEM  pSysProp;               // system properties
    CHAR  chDrives[30];
    pSysProp = (PPROPSYSTEM) MakePropPtrFromHnd( EqfQuerySystemPropHnd());
    strupr( pSysProp->szDriveList );
    UtlSetString( QST_ORGEQFDRIVES, pSysProp->szDriveList );

    /************************************************************/
    /* force a refresh of QST_VALIDEQFDRIVES string             */
    /************************************************************/
    UtlGetCheckedEqfDrives( chDrives );
  } /* endif */
  /********************************************************************/
  /* we only need the TranslationMemory (in addition to the standard  */
  /********************************************************************/
  if ( fOK ) fOK = TwbStartListHandler( FOLDERLISTHANDLER,
                                        FolderListHandlerCallBack, NULL );
  if ( fOK ) fOK = TwbStartListHandler( FOLDERHANDLER,
                                        FolderHandlerCallBack, NULL );
  if ( fOK ) fOK = TwbStartListHandler( TAGTABLEHANDLER,
                                        TagListHandlerCallBack, NULL );
  if ( fOK ) fOK = TwbStartListHandler( MEMORYHANDLER,
                                        MemListHandlerCallBack, NULL );

  if ( fOK ) fOK = TwbStartHandler( hab, DOCUMENTHANDLER,
                                        DOCUMENTHANDLERWP, NULL );
  if ( fOK ) fOK = TwbStartHandler( hab, ANALYSISHANDLER,
                                         ANALYSISHANDLERWP, NULL );
  return ( fOK );
} /* end of function ITMHandlers */
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MakeHashValues
//------------------------------------------------------------------------------
// Function call:     MakeHashValues( pulRandom, ulMaxNum, pszString, pulHash);
//------------------------------------------------------------------------------
// Description:       build a quasi hash value of the passed string
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     go through the passed string and build hashvalue
//                    of all characters up to the specified max value
//------------------------------------------------------------------------------
static VOID
MakeHashValue
(
  PULONG    pulRandom,                 // array of random numbers for hashing
  USHORT    usMaxRandom,
  PSZ_W     pData,
  PULONG    pulHashVal
)
{
  USHORT usRandomIndex = 0;
  ULONG  ulHashVal = 0;
  CHAR_W c;

  while ( ((c = *pData++)!= 0) && (usRandomIndex < usMaxRandom))
  {
    switch ( c )
    {
      case ' ':
      case '\n':
      case '\r':
      case '\t':
        break;
      default :
        ulHashVal += pulRandom[usRandomIndex++] * c;
        break;
    } /* endswitch */
  } /* endwhile */
  *pulHashVal = ulHashVal;

  return;
} /* end of function MakeHashValue */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     AddToHashBlock
//------------------------------------------------------------------------------
// Function call:     AddToHashBlock( pITMNopSegs, pSeg,     ulHashVal);
//------------------------------------------------------------------------------
// Description:       add the subject NOP segment to our list of synch points
//------------------------------------------------------------------------------
// Parameters:        PITMNOPSEGS pITMNopSegs,
//                    ULONG       ulSegNum,
//                    USHORT      usLength,
//                    PSZ         pData,
//                    ULONG       ulHash,
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE  everything okay
//                    FALSE allocation error
//------------------------------------------------------------------------------
// Function flow:     check if still is space in list - if not allocate chunk
//                    add this element to our list
//------------------------------------------------------------------------------
// GQ 2016/11/28: undo of Li Pings fix for P403330 - the fix had the sideeffect to eliminate all blanks in segments following under certain conditions
//int filterSpace(wchar_t* pInData)
//{
//	if(pInData == NULL)
//		return 0;
//
//	wchar_t *pTemp = pInData;
//	wchar_t *pPos = pInData;
//	while(*pTemp != L'\0')
//	{
//		if(*pTemp==L' ' || *pTemp==L'\t')
//		{
//			pTemp++;
//		}
//		else
//		{
//			if(pPos == pTemp)
//				pPos++,pTemp++;
//			else
//				*pPos++ = *pTemp++;
//		}
//		   
//	}
//	if(pPos != pTemp)
//	    *pPos = L'\0';
//
//	return (pTemp-pPos);
//}
static
BOOL AddToHashBlock
(
  PITMNOPSEGS pITMNopSegs,             // ITM Align struct to be used
  ULONG       ulSegNum,
  ULONG       ulLength,
  PSZ_W       pData,
  ULONG       ulHash                            // hash value
)
{
  BOOL  fOK = TRUE;                    // success indicator

  FUZZYTOK HUGE * pToken;

  /********************************************************************/
  /* check if alloc is possible: if not, delete tokens if more than   */
  /* one per segment                                                  */
  /********************************************************************/
  ULONG    ulMax = MAX_ALLOC;
  if ( (pITMNopSegs->ulAlloc <= pITMNopSegs->ulUsed )
         && (pITMNopSegs->ulAlloc >= ulMax ) )
  {
    ITMDelDupPerSeg(pITMNopSegs);
  } /* endif */
  /********************************************************************/
  /* check if still one segment is free to be filled ...              */
  /* if not allocate memory first ...                                 */
  /********************************************************************/
 // if ( pITMNopSegs->ulUsed >= ulMax  )
 // {
 //   fOK = FALSE;
 // } /* endif */

  if ( fOK && (pITMNopSegs->ulAlloc <= pITMNopSegs->ulUsed  ))
  {
      fOK = HashBlockAlloc(pITMNopSegs);
  } /* endif */
  /********************************************************************/
  /* fill in the new values for this NOP block                        */
  /********************************************************************/
  if ( fOK )
  {
    pITMNopSegs->pulSegs[pITMNopSegs->ulUsed] = ulSegNum;

    pToken = &(pITMNopSegs->pTokenList[pITMNopSegs->ulUsed]);
    pToken->ulHash = ulHash;

// GQ 2016/11/28: undo of Li Pings fix for P403330 - the fix had the sideeffect to eliminate all blanks in segments following under certain conditions
//	// Fix P403330 begin
//	int spaces = filterSpace(pData);
//	// Fix P403330 end
    pToken->pData = pData;

    pToken->usStart= 0;

//	// Fix P403330 begin
//    pToken->usStop = (USHORT)(ulLength - 1 - spaces );
//	// Fix P403330 end
    pToken->usStop = (USHORT)(ulLength - 1);

    /******************************************************************/
    /* dummy setting for sType, just use MARK_DELETED                 */
    /******************************************************************/
    pToken->sType  = MARK_DELETED  ;
    pToken->fConnected = FALSE ;
    pITMNopSegs->ulUsed ++;              // point to next entry
  } /* endif */
  return ( fOK );
} /* end of function AddToHashBlock */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     HashBlockAlloc
//------------------------------------------------------------------------------
// Function call:     HashBlockAlloc( pITMNopSegs, pSeg,     ulHashVal);
//------------------------------------------------------------------------------
// Description:       alloc more space
//------------------------------------------------------------------------------
// Parameters:        PITMNOPSEGS pITMNopSegs,
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE  everything okay
//                    FALSE allocation error
//------------------------------------------------------------------------------
// Function flow:     calculate old length and new length
//                    for pulSegs and pTokenlist:
//                      alloc new space (huge alloc)
//                      copy old contents to new block
//                      free old block
//------------------------------------------------------------------------------

static
BOOL HashBlockAlloc
(
  PITMNOPSEGS pITMNopSegs              // ITM Align struct to be used
)
{
  BOOL  fOK = TRUE;                    // success indicator
  LONG   lNewLen;
  LONG   lOldLen;
  ULONG  HUGE *pulSegs;                // pointer to segment numbers
  FUZZYTOK HUGE *pTokenList;           // pointer to fuzzy tokens
  ULONG         ulAllocOld;
  ULONG         ulAllocNew;

//  ulAllocOld = pITMNopSegs->ulAlloc;
  ulAllocOld = pITMNopSegs->ulAlloc;
  ulAllocNew = ulAllocOld + NEW_HASHBLOCK;
//  if ( ulAllocOld == MAX_ALLOC )
//  {
//    fOK = FALSE;
//  }
//  else
//  {
//    if ( ulAllocNew > MAX_ALLOC )
//    {
//      ulAllocNew = MAX_ALLOC;
//    } /* endif */
//  } /* endif */
  if ( fOK )
  {
//    lOldLen =  (LONG) (pITMNopSegs->ulAlloc) * sizeof( USHORT );
    lOldLen =  (LONG) (ulAllocOld) * sizeof( ULONG );

    lNewLen =  (LONG) (ulAllocNew) * sizeof( ULONG );
    ALLOCHUGE( pulSegs, ULONG*, ulAllocNew, sizeof(ULONG) );
    if ( pulSegs )
    {
      if ( lOldLen )
      {
        memcpy( pulSegs, pITMNopSegs->pulSegs, lOldLen );
      } /* endif */

      pITMNopSegs->ulAlloc = ulAllocNew;
      if ( pITMNopSegs->pulSegs )
      {
        FREEHUGE( pITMNopSegs->pulSegs );
      } /* endif */
      pITMNopSegs->pulSegs = pulSegs;
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    lOldLen =  (LONG) (ulAllocOld) * sizeof( FUZZYTOK );
    lNewLen =  (LONG) (pITMNopSegs->ulAlloc) * sizeof( FUZZYTOK );

    ALLOCHUGE( pTokenList, FUZZYTOK*, pITMNopSegs->ulAlloc, sizeof( FUZZYTOK) );
    if ( pTokenList )
    {
      if ( lOldLen )
      {
        memcpy( pTokenList, pITMNopSegs->pTokenList, lOldLen );
      } /* endif */
      if ( pITMNopSegs->pTokenList )
      {
        FREEHUGE( pITMNopSegs->pTokenList );
      } /* endif */

      pITMNopSegs->pTokenList = pTokenList;
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */
  return ( fOK );
} /* end of function AddToHashBlock */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     AlignParagraphs
//------------------------------------------------------------------------------
// Function call:     AlignParagraphs( pITMIda );
//------------------------------------------------------------------------------
// Description:       align the paragraphs using the above outlined algorithm.
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   every thing okay
//                    FALSE  allocation error
//------------------------------------------------------------------------------
// Function flow:     allocate enough space to hold rows of matrix pointers
//                    fill backward anchor candidate list for src and tgt
//                    prepare the matrix for a best fit..(ITMLCS)
//                     now all aligned candidates are marked with MARK_EQUAL
//                    build anchor list in each document
//                    parse anchorlist: delete multiples
//                    free any allocated resources
//------------------------------------------------------------------------------

static BOOL
AlignParagraphs
(
  PITMIDA  pITMIda
)
{
  BOOL fOK = TRUE;                     // success indicator
  PITMNOPSEGS  pSrcNop;                // pointer to source nop segments
  PITMNOPSEGS  pTgtNop;                // pointer to target nop segments
  ULONG        k;
  ULONG        ulAct;
  FUZZYTOK HUGE *pSrcBack;
  FUZZYTOK HUGE *pTgtBack;
  FUZZYTOK HUGE *pTestToken;
  ITMLCSTOKEN     LCSString1;
  ITMLCSTOKEN     LCSString2;
  BOOL     fAnchorDel;
  HWND     hwndTemp = pITMIda->hwnd;

  /********************************************************************/
  /* get pointer to source and target nop segments ...                */
  /********************************************************************/
  pSrcNop = &pITMIda->itmSrcNop;
  pTgtNop = &pITMIda->itmTgtNop;

  ALLOCHUGE( pSrcBack, FUZZYTOK*, (pSrcNop->ulUsed + 1), sizeof( FUZZYTOK) );
  ALLOCHUGE( pTgtBack, FUZZYTOK*, (pTgtNop->ulUsed + 1), sizeof( FUZZYTOK) );

  memset( &LCSString1, 0, sizeof( ITMLCSTOKEN ));
  memset( &LCSString2, 0, sizeof( ITMLCSTOKEN ));

  if ( (!pSrcBack) || (!pTgtBack) )
  {
    fOK = FALSE;
  } /* endif */
  if ( fOK )
  {
#ifdef ITMTEST
//    fOut      = fopen ( "ITMSTAT.OUT", "a" );
//    fprintf(fOut, "Source, Target %s, %s\n", pITMIda->chSourceFile, pITMIda->chTargetFile);
//    fprintf(fOut, "Number of Anchor candidates in src:%4d\n", pSrcNop->ulUsed);
//    fprintf(fOut, "Number of Anchor candidates in tgt:%4d\n", pTgtNop->ulUsed);
//    //ulDisps = 0l;
//  for ( k=0;k < pSrcNop->ulUsed ;k++)
//  {
//    fprintf(fOut, "%4d %s\n", pSrcNop->pulSegs[k],
//                pSrcNop->pTokenList[k].pData);
//  } /* endfor */
//  for ( k=0;k < pTgtNop->ulUsed ;k++)
//  {
//    fprintf(fOut, "%4d %s\n", pTgtNop->pulSegs[k],
//                pTgtNop->pTokenList[k].pData);
//  } /* endfor */
//  fprintf(fOut, "Anchor candidates:Src %4d Tgt: %4d\n",
//                   pSrcNop->ulUsed, pTgtNop->ulUsed );
//    fclose( fOut      );
#endif
    /******************************************************************/
    /* fill backward tokenlist for source and target                  */
    /******************************************************************/
    pTestToken = pSrcNop->pTokenList;
    for ( k=pSrcNop->ulUsed  ;k > 0 ; k-- )
    {
      *(pSrcBack+k- 1) = *pTestToken;
      pTestToken ++;
    } /* endfor */
    pTestToken = pTgtNop->pTokenList;
    for ( k=pTgtNop->ulUsed ;k > 0 ; k-- )
    {
      *(pTgtBack+k - 1) = *pTestToken;
      pTestToken ++;
    } /* endfor */
    /******************************************************************/
    /* call recursive function to find longest common subsequence     */
    /* of matching nops                                               */
    /******************************************************************/
    LCSString1.pTokenList = pSrcNop->pTokenList;
    LCSString1.pBackList = pSrcBack;
    LCSString1.lStart = 0;
    LCSString1.lStop = (LONG)pSrcNop->ulUsed;
    LCSString1.lTotalLen = (LONG)pSrcNop->ulUsed;

    LCSString2.pTokenList = pTgtNop->pTokenList;
    LCSString2.pBackList = pTgtBack;
    LCSString2.lStart = 0;
    LCSString2.lStop = (LONG)pTgtNop->ulUsed;
    LCSString2.lTotalLen = (LONG)pTgtNop->ulUsed;

    pITMIda->usStartSlider ++;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
    UtlDispatch();
    fOK = ( (hwndTemp == HWND_FUNCIF) || (hwndTemp && WinIsWindow( (HAB) NULL, hwndTemp )));  // Test MK Non-DDE
  } /* endif */
  if ( fOK )
  {
    /******************************************************************/
    /* set init values for slider during ITMLCS                       */
    /******************************************************************/
    pITMIda->usOldPerc = 0;
    pITMIda->usAnchorCount = 0;

    ITMLCS ( LCSString1,LCSString2,pITMIda);
    pITMIda->usStartSlider +=25;
#ifdef ITMTEST
          fOut      = fopen ( "ITMSTAT.OUT", "a" );

//          fprintf(fOut, "Number of UtlDispatch in ITMLCS: %10d\n", ulDisps);

          fclose( fOut      );
#endif
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
    UtlDispatch();
    fOK = ( (hwndTemp == HWND_FUNCIF) || (hwndTemp && WinIsWindow( (HAB) NULL, hwndTemp )));        // Test MK Non-DDE
  } /* endif */
  if ( fOK )
  {
    /******************************************************************/
    /* now all aligned nops are marked with sType = MARK_EQUAL        */
    /******************************************************************/
    fAnchorDel = TRUE;
    /******************************************************************/
    /* use counter k to avoid more than 10 loops                      */
    /* delete anchors which are not 100% sure                         */
    /******************************************************************/
    k = 0;
    while ( fAnchorDel && (k < 10) )
    {
      fAnchorDel = ITMDelUnsureAnchor (pSrcNop, pTgtNop);
      k++;
    } /* endwhile */
    pITMIda->usStartSlider ++;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );

#ifdef ITMTEST
//   {
//     BOOL fFound = FALSE;
//     CHAR chS[42], chT[42];
//     USHORT i, j;
//
//    fOut      = fopen ( "ITMSTAT.OUT", "a" );
//  fprintf(fOut, "Anchors\n");
//  k = 0;
//  j=0;
//  while (k < pSrcNop->ulUsed )
//  {
//    if ( pSrcNop->pTokenList[k].sType == MARK_EQUAL)
//    {
//      memcpy( chS, pSrcNop->pTokenList[k].pData, 40);
//      chS[40] = EOS;
//      for ( i=0;i<40 ;i++ )
//      {
//        if ( chS[i] == '\n' )
//        {
//          chS[i] = EOS;
//          break;
//        } /* endif */
//      } /* endfor */
//      while ( !fFound && (j<pTgtNop->ulUsed) )
//      {
//        if ( pTgtNop->pTokenList[j].sType == MARK_EQUAL )
//        {
//          memcpy( chT, pTgtNop->pTokenList[j].pData, 40);
//          chT[40] = EOS;
//          for ( i=0;i<40 ;i++ )
//          {
//            if ( chT[i] == '\n' )
//            {
//              chT[i] = EOS;
//              break;
//            } /* endif */
//          } /* endfor */
//          fprintf(fOut, "%4d %4d %s %s\n",
//                        pSrcNop->pulSegs[k],pTgtNop->pulSegs[j],
//                         chS, chT );
//          k++;
//          j++;
//          fFound = TRUE;
//        }
//        else
//        {
//          j++;
//        } /* endif */
//      } /* endwhile */
//      fFound = FALSE;
//    }
//    else
//    {
//      k++;
//    } /* endif */
//  } /* endwhile */
//  fprintf(fOut, "Number of Synchpoints: %4d\n", pSrcNop->ulUsed);
//  fclose( fOut      );
//  }
#endif
    ulAct = 0;
    for ( k=0;k<pSrcNop->ulUsed ;k++ )
    {
      if ( pSrcNop->pTokenList[k].sType == MARK_EQUAL)
      {
        pSrcNop->pulSegs[ulAct++] = pSrcNop->pulSegs[k];
        /**************************************************************/
        /* add line for output with text                              */
        /**************************************************************/
      } /* endif */
    } /* endfor */
    pSrcNop->ulUsed = ulAct;

    ulAct = 0;
    for ( k=0;k<pTgtNop->ulUsed ;k++ )
    {
      if ( pTgtNop->pTokenList[k].sType == MARK_EQUAL)
      {
        pTgtNop->pulSegs[ulAct++] = pTgtNop->pulSegs[k];
      } /* endif */
    } /* endfor */
    pTgtNop->ulUsed = ulAct;
    /******************************************************************/
    /* delete anchors which are not 1:1, delete multiples of the same */
    /* entry                                                          */
    /******************************************************************/
#ifdef ITMTEST
//      fOut      = fopen ( "ITMSTAT.OUT", "a" );
//    fprintf(fOut, "Anchors\n");
//    for ( k=0;k < pSrcNop->ulUsed ;k++)
//    {
//      fprintf(fOut, "%4d %4d %s\n", pSrcNop->pulSegs[k],
//                  pTgtNop->pulSegs[k],pSrcNop->pTokenList[k].pData);
//    } /* endfor */
//    fprintf(fOut, "Number of Synchpoints: %4d\n", pSrcNop->ulUsed);
//    fclose( fOut      );
#endif
    fOK = PostParseAnchor (pSrcNop, pTgtNop);
    pITMIda->usStartSlider += 2;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
#ifdef ITMTEST
//    fOut      = fopen ( "ITMSTAT.OUT", "a" );
//  fprintf(fOut, "Anchors\n");
//  for ( k=0;k < pSrcNop->ulUsed ;k++)
//  {
//    fprintf(fOut, "%4d %4d\n", pSrcNop->pulSegs[k],pTgtNop->pulSegs[k]);
//  } /* endfor */
//  fprintf(fOut, "Number of Synchpoints: %4d\n", pSrcNop->ulUsed);
//  fclose( fOut      );
#endif
    if ( pSrcBack )
    {
      FREEHUGE( pSrcBack );
      pSrcBack = NULL;
    } /* endif */
    if ( pTgtBack )
    {
      FREEHUGE( pTgtBack );
      pTgtBack = NULL;
    } /* endif */
    /******************************************************************/
    /* undo the cross-out: example: in wp6, tables are crossed-out    */
    /* to garantuee that table cells are only aligned with tablecells */
    /* and never with any text outside of a table; using the general  */
    /* tags TAG_ITM_STARTX and TAG_ITM_ENDX in the 1st approach       */
    /* implies that now footer and header are also aligned again if   */
    /* occur in both documents; it is nec. that if TAG_ITM_STARTX or  */
    /* ENDX is set, also TAG_ITM_PART is set, to garantuee that the   */
    /* start and end of the crossed-out area become an anchor if they */
    /* occur in both documents!! ( Start  + Part= 4+2= AddInfo 6,     */
    /*                             End + Part = 8+2=AddInfo 10        */
    /******************************************************************/

    PostParseUndoCrossedOut(pITMIda);
    pITMIda->usStartSlider ++;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
    /******************************************************************/
    /* wo werden die tokenlists freigegeben??? mit der Ida            */
    /******************************************************************/
    PostParseEqualNums(pITMIda );    // handle NOPs with AddInfo EQUALNUM
    pITMIda->usStartSlider ++;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
  }
  else
  {
    /******************************************************************/
    /* alloc error                                                    */
    /******************************************************************/
    fOK = FALSE;
  } /* endif */

  return( fOK );
} /* end of function AlignParagraphs */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     GetMeanVar
//------------------------------------------------------------------------------
// Function call:     GetMeanVar( pITMIda );
//------------------------------------------------------------------------------
// Description:       determine the mean and the variance of the documents
//------------------------------------------------------------------------------
// Parameters:        PITMIDA pITMIda               instance data structure
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     for i=0 to number of all used paragraphs
//                      to get variance:
//                      sum up square differences between src and target length
//                      divide it thru length of source since
//                      model assumes s**2 proportional to length of source
//                    endfor
//                    to get mean: divide sum of trt lengths thru src lengths
//                    count number of segments to be translated in each doc
//------------------------------------------------------------------------------
static VOID
GetMeanVar
(
  PITMIDA pITMIda                   // instance data structure
)
{
  ULONG      ulUsed;
  ULONG      i;                         // index
  LONG       sSrc;
  LONG       sTgt;
  DOUBLE     dbVar = (DOUBLE) 0;
  DOUBLE     dbVarTemp;
  ULONG      ulSrcLen = 0;              // source length
  ULONG      ulTgtLen = 0;              // target length
  USHORT     usRealUsed = 0;

  /********************************************************************/
  /* sum up square differences and divide it thru length of src       */
  /* since our model assumes s**2 is proportional to the              */
  /* length of the source                                             */
  /*  -> get estimate of s**2 using the number of paragraphs          */
  /*                                                                  */
  /* Rational: use only the first segment of the paragraph because    */
  /*           this should be sufficient for our test variance ...    */
  /********************************************************************/
  ulUsed = min( pITMIda->itmSrcNop.ulUsed, pITMIda->itmTgtNop.ulUsed );
  if ( ulUsed )
  {
    for ( i=0; i<ulUsed; i++ )
    {
      sSrc = GetSizeOfBlock( pITMIda,
                             &pITMIda->TBSourceDoc,
                             pITMIda->itmSrcNop.pulSegs[i],
                             pITMIda->itmSrcNop.pulSegs[i+1] );
      ulSrcLen += sSrc;
      sTgt = GetSizeOfBlock( pITMIda,
                             &pITMIda->TBTargetDoc,
                             pITMIda->itmTgtNop.pulSegs[i],
                             pITMIda->itmTgtNop.pulSegs[i+1] );
      ulTgtLen += sTgt;
      if ( sSrc )
      {
                usRealUsed ++;
        dbVarTemp = (DOUBLE)(sSrc - sTgt);
        dbVar += ( dbVarTemp * dbVarTemp ) / sSrc;
      } /* endif */
    } /* endfor */
    /******************************************************************/
    /* check for boundary conditions ...                              */
    /******************************************************************/
    if ( dbVar == (DOUBLE) 0 )
    {
      dbVar = EPSILON;                // minimal value
    } /* endif */
    if ( ulSrcLen == 0L )
    {
      ulSrcLen = 1;                    // minimal value
    } /* endif */
    if ( ulTgtLen == 0L )
    {
      ulTgtLen = 1;                    // minimal value
    } /* endif */
    if (usRealUsed == 0 )
    {
      usRealUsed = 1;                  // avoid trap at division!
    }
    pITMIda->dbVar = dbVar / usRealUsed;
#ifdef ITMTEST
        { DOUBLE dbTest;
     fOut      = fopen ( "ITMSTAT.OUT", "a" );
         dbTest = dbVar / ulUsed;
     fprintf( fOut,
       "ulUsed: %2d usRealUsed: %2d OldVar: %5.5f NewVar: %5.5f\n",
                     ulUsed, usRealUsed, dbTest, pITMIda->dbVar);
     fclose( fOut      );
        }
#endif

        // assure dbVar at least 16 and at most 100
        if (pITMIda->dbVar < (DOUBLE)20 )
        {
                pITMIda->dbVar = (DOUBLE)20;
        } /* endif */
        if (pITMIda->dbVar > (DOUBLE)100 )
                pITMIda->dbVar = (DOUBLE)100;
    pITMIda->dbMean = ((DOUBLE) ulTgtLen)/ulSrcLen;

    pITMIda->stSrcInfo.ulSegTotal = (ULONG) GetNumberOfToBe(
                                                &(pITMIda->TBSourceDoc));
    pITMIda->stTgtInfo.ulSegTotal = (ULONG) GetNumberOfToBe(
                                                &(pITMIda->TBTargetDoc));
    pITMIda->ulSegTotal += pITMIda->stSrcInfo.ulSegTotal;
  } /* endif */
  pITMIda->usStartSlider ++;
  WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
              MP1FROMSHORT(pITMIda->usStartSlider), NULL );
} /* end of function GetMeanVar */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     GetSizeOfBlock
//------------------------------------------------------------------------------
// Function call:
//------------------------------------------------------------------------------
// Description:       returns size     of the ToBe translated characters
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda,
//                    PTBDOCUMENT pDoc,
//                    USHORT  usStartSeg,
//                    USHORT  usEndSeg
//------------------------------------------------------------------------------
// Returncode type:   LONG
//------------------------------------------------------------------------------
// Returncodes:       number of characters in block
//------------------------------------------------------------------------------
// Function flow:     while not end of block
//                      if segment is to be translated
//                        add lengths in unprotected parts
//                    endwhile
//                    return total 'unprotected' length
//------------------------------------------------------------------------------

static LONG
GetSizeOfBlock
(
  PITMIDA  pITMIda,
  PTBDOCUMENT pDoc,
  ULONG   ulStartSeg,
  ULONG   ulEndSeg
)
{
  PTBSEGMENT  pSeg;
  LONG        usLen = 0;
  USHORT      usColPos = 0;
  PSTARTSTOP  pstCurrent;
  USHORT      usRC;

  if ( !ulEndSeg )
  {
    ulEndSeg = pDoc->ulMaxSeg;
  } /* endif */
  if ( !ulStartSeg )
  {
          ulStartSeg = 1;
  } /* endif */
  /********************************************************************/
  /* fill structures into provided block                              */
  /********************************************************************/
  pSeg = EQFBGetSegW(pDoc, ulStartSeg);
  while ( pSeg && (ulStartSeg <= ulEndSeg) )
  {
    if ( (pSeg->qStatus == QF_TOBE) || (pSeg->qStatus == QF_XLATED) )
    {
/**********************************************************************/
/* to be changed!! in reald, TACreateProtectatble should be called onl*/
/* only once and the result should be kept in an array                */
/**********************************************************************/
      if ( !pSeg->pusBPET )
      {
        usRC = TACreateProtectTableW( pSeg->pDataW,
                                    ((PLOADEDTABLE)pITMIda->pLoadedTable),
                                    usColPos,
                                    (PTOKENENTRY) pDoc->pTokBuf,
                                    TOK_BUFFER_SIZE,
                                    (PSTARTSTOP *) &(pSeg->pusBPET),
                                    pDoc->pfnUserExit,
                                    pDoc->pfnUserExitW, pDoc->ulOemCodePage);
      } /*endif*/
      if ( pSeg->pusBPET)
      {
        // add lengths in unprotected parts
        pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
        while ( pstCurrent->usType != 0 )
        {
           if ( pstCurrent->usType == UNPROTECTED_CHAR )
           {
             usLen += (LONG) (pstCurrent->usStop - pstCurrent->usStart + 1);
           } /* endif */
           pstCurrent++;
        } /* endwhile */
      }
      else
      {
        usLen = (LONG)(pSeg->usLength);
      } /* endif */
    } /* endif */
    ulStartSeg++;
    pSeg = EQFBGetSegW(pDoc, ulStartSeg);
  } /* endwhile */

  return ( usLen );
} /* end of function GetSizeOfBlock */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     static GetBothTok(PITMIDA,PTBDOCUMENT,SHORT,PITMNOPSEGS)
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       build list of anchor candidates (NOP and MORPH tokens)
//------------------------------------------------------------------------------
// Parameters:        PITMIDA       pITMIda,
//                    PTBDOCUMENT   pDoc,
//                    SHORT         sLanguageId
//                    PITMNOPSEGS   pITMNopSegs
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   success
//                    FALSE  error
//------------------------------------------------------------------------------
// Function flow:     load document tagtable
//                    build random sequence for hashing
//                    add dummy token at begin of candidate list
//                    while not at last segment of file
//                      switch (pSeg->qStatus)
//                       case nop segment: if NOP is not in list of exluded
//                        nops, add it to candidate list
//                       default: find morph tokens in segment
//                           TF_NUMBER token: add to candidate list
//                           TF_ALLCAPS and length > 1: add
//                           TF_NOLOOKUP and length >1: add
//                      endswitch
//                      goto next segment
//                    endwhile
//                    add dummy token at end of candidate list
//------------------------------------------------------------------------------

static
BOOL GetBothTok
(
  PITMIDA       pITMIda,
  PTBDOCUMENT   pDoc,
  SHORT         sLanguageId,
  PITMNOPSEGS   pITMNopSegs,
  ULONG         ulOemCP
)
{
  ULONG        ulSegNum = 0;
  PTBSEGMENT   pSeg = NULL;
  BOOL         fOK = TRUE;
  PCHAR_W      pRest = NULL;
  USHORT       usColPos = 0;
  PTOKENENTRY  pTok = NULL;
  CHAR_W       chTemp;
  USHORT       usListSize = 0;
  PFLAGOFFSLIST pTermList = NULL;
  PFLAGOFFSLIST pActTerm = NULL;
  BOOL          fFound;
  SHORT         sRc;
  CHAR_W        szMorphData[MAX_TERM_LEN];
  ULONG         ulLength = 0;

  ULONG         ulRandom[ITM_MAX_RANDOM];
  USHORT        usRandomIndex = 0;
  USHORT        usMaxRandom = 0;
  ULONG         ulHash = 0;
  BOOL          fUseNop;
  USHORT        usNewPerc = 0;
  USHORT        usOldPerc = 0;
  HWND          hwndTemp = pITMIda->hwnd;
  /********************************************************************/
  /* load document tag table                                          */
  /********************************************************************/
  fOK = !TALoadTagTable( pITMIda->chTagTableName,
                         (PLOADEDTABLE *) &pITMIda->pLoadedTable,
                         FALSE, FALSE );
  pDoc->pDocTagTable = pITMIda->pLoadedTable;
  /********************************************************************/
  /* build random sequences for NOP anchors                           */
  /********************************************************************/
  if ( fOK )
  {
    if ( pITMIda->usLevel && pITMIda->usLevel < ITM_MAX_RANDOM )
    {
      usMaxRandom = pITMIda->usLevel;
    }
    else
    {
      usMaxRandom = ITM_MAX_RANDOM;
    } /* endif */

    /********************************************************************/
    /* random sequences, see e.g. the book of Wirth...                  */
    /********************************************************************/
    ulRandom[0] = 0xABCDEF01;
    for (usRandomIndex = 1; usRandomIndex < ITM_MAX_RANDOM; usRandomIndex++)
    {
        ulRandom[usRandomIndex] = ulRandom[usRandomIndex - 1] * 5 + 0xABCDEF01;
    }

    pITMIda->usStartSlider ++;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
    /********************************************************************/
    /* we are in sync at least at the begin of the file                 */
    /********************************************************************/
    ulSegNum = 0;
    ulLength = 1;
    szMorphData[0] = EOS;
    fOK = AddToHashBlock (pITMNopSegs, ulSegNum, ulLength,
                           szMorphData, 0L );

  } /* endif */
  /********************************************************************/
  /* now go through the file and add possible sync. points            */
  /********************************************************************/

  ulSegNum = 1;
  pSeg = EQFBGetSegW(pDoc, ulSegNum);
  while ( pSeg && fOK)
  {
    usNewPerc = (USHORT) (ulSegNum * 10 / pDoc->ulMaxSeg);
    if ( usNewPerc != usOldPerc )              //update slider
    {
      UtlDispatch();
      fOK = ( (hwndTemp == HWND_FUNCIF) || (hwndTemp && WinIsWindow( (HAB) NULL, hwndTemp )));    //Test MK Non-DDE
      if ( fOK )
      {
        usOldPerc = usNewPerc;
        WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                    MP1FROMSHORT(usOldPerc + (pITMIda->usStartSlider)), NULL );
      } /* endif */
    } /* endif */
    if ( fOK )
    {
      switch ( pSeg->qStatus )
      {
        case  QF_NOP:
        case  QF_ATTR:
          TATagTokenizeW( pSeg->pDataW,
                         ((PLOADEDTABLE)pITMIda->pLoadedTable),
                         TRUE,
                         &pRest,
                         &usColPos,
                         (PTOKENENTRY) pDoc->pTokBuf,
                         TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
          pTok = (PTOKENENTRY) pDoc->pTokBuf;
          while ( fOK && (pTok->sTokenid != ENDOFLIST) )
          {
            // either we are dealing with real tags or we have single text
            // tokens as nops (happens if markup is empty and user exit
            // does analysis
            fUseNop = TRUE;
            if ( (pTok->sTokenid < 0 ) ||
                 (pTok->sAddInfo & TAG_ITM_IGNORE ) ||
                 (pTok->sAddInfo & TAG_ITM_EQUALNUM ) )
            {
              fUseNop = FALSE;
              if ((pTok->sTokenid < 0) &&
                   (pTok == (PTOKENENTRY) pDoc->pTokBuf) &&
                   ((pTok+1)->sTokenid == ENDOFLIST) )
                   {
                    fUseNop = TRUE;
                   }
            } /* endif */
            if (fUseNop)
            {
              ulLength = ITMGetRelLength(pITMIda->pLoadedTable,
                                         pTok->usLength,
                                         pTok->sAddInfo,
                                         pTok->sTokenid, szMorphData);

              /**************************************************************/
              /* NOPs as candidates in synch list                           */
              /* check if it is the excluded NOP                            */
              /**************************************************************/
              fUseNop =  CheckIfExcluded (&(pITMIda->stExclNopCnt),
                                          pTok->pDataStringW,
                                          pTok->sTokenid, pTok->sAddInfo );
              if ( fUseNop )
              {
                /******************************************************/
                /* usLength limited to at most MAX_TERM_LEN-1 in      */
                /* ITMGetRelLength ...                                */
                /******************************************************/
                UTF16strncpy(szMorphData, pTok->pDataStringW, ulLength);
                szMorphData[ulLength] = EOS;
                MakeHashValue( ulRandom, usMaxRandom, szMorphData, &ulHash );

                /**********************************************************/
                /* fill anchor candidate list: use sTokenID as Hash       */
                /**********************************************************/
                fOK = AddToHashBlock (pITMNopSegs, ulSegNum,
                                      ulLength, pTok->pDataStringW,
                                      ulHash  );
                if ( fOK )
                {
                  UtlDispatch();
                  fOK = ( (hwndTemp == HWND_FUNCIF) || (hwndTemp && WinIsWindow( (HAB) NULL, hwndTemp ))); //TEST MK for NON-DDE
                } /* endif */
              } /* endif */
            } /* endif */
            pTok++;
          } /* endwhile */
          break;
        case QF_CROSSED_OUT:        //do not use as anchor candidate
        case QF_CROSSED_OUT_NOP:
          break;
        default :
          /**************************************************************/
          /* find morph tokens for candidates in synch list             */
          /**************************************************************/

          TATagTokenizeW( pSeg->pDataW,
                         ((PLOADEDTABLE)pITMIda->pLoadedTable),
                         TRUE,
                         &pRest,
                         &usColPos,
                         (PTOKENENTRY) pDoc->pTokBuf,
                         TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
          pTok = (PTOKENENTRY) pDoc->pTokBuf;
          while ( (pTok->sTokenid != ENDOFLIST) && fOK )
          {
            if ( pTok->sTokenid == TEXT_TOKEN )   //check no inline tags
            {
              usListSize =0;
              pTermList = NULL;
              chTemp =*(pTok->pDataStringW+pTok->usLength);
              *(pTok->pDataStringW+pTok->usLength) = EOS;
              sRc = MorphTokenizeW( sLanguageId, pTok->pDataStringW,
                                   &usListSize, (PVOID *) &pTermList,
                                   MORPH_FLAG_OFFSLIST, ulOemCP);
              *(pTok->pDataStringW+pTok->usLength) = chTemp;
              if ( pTermList )
              {
                pActTerm = pTermList;
                while ( pActTerm->usLen && fOK )
                {
                  fFound = FALSE;
                  fFound = GetSpecialTok(pTok, pActTerm );
                  if (!fFound)
                  {
                    if ( pActTerm->lFlags & TF_NUMBER )
                    {
                      fFound = TRUE;
                    } /* endif */
                    if ( (pActTerm->lFlags & TF_ALLCAPS )
                         && (pActTerm->usLen > 1))
                    {
                      fFound = TRUE;
                    } /* endif */
                    if ( (pActTerm->lFlags & TF_ABBR )
                         && (pActTerm->usLen > 1))
                    {
                      fFound = TRUE;
                    } /* endif */
                    if ( (pActTerm->lFlags & TF_NOLOOKUP)
                         && (pActTerm->usLen > 1))
                    {
                      fFound = TRUE;
                    } /* endif */
                  } /* endif */

                  if ( fFound )
                  {
                    USHORT usMaxLen = min( MAX_TERM_LEN-1, pActTerm->usLen );
                    UTF16strncpy(&szMorphData[0],
                            pTok->pDataStringW+(pActTerm->usOffs),
                            usMaxLen);
                    szMorphData[usMaxLen] = EOS;
                    MakeHashValue( ulRandom, usMaxRandom, szMorphData, &ulHash );
  #ifdef ITMTEST
//                    fOut      = fopen ( "ITMSTAT.OUT", "a" );
//                    fprintf( fOut,"@@ %d %d %d %d %s\n",pSeg->ulSegNum, pActTerm->usOffs, pActTerm->usLen, MAX_TERM_LEN,  szMorphData);
//                    fclose( fOut      );
//                    fOut = NULL;
  #endif
                    /*****************************************************/
                    /* fill ITMNOP struct; use lFlags as Hash value      */
                    /*****************************************************/
                    fOK = AddToHashBlock (pITMNopSegs, ulSegNum,
                                          pActTerm->usLen,
                                          pTok->pDataStringW+(pActTerm->usOffs),
                                          ulHash  );
                  } /* endif */
                  pActTerm++;
                } /* endwhile */
              } /* endif */
              UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG);
            }
            else
            {
  //          /**********************************************************/
  //          /* new approach: add inline tags also as tokens to        */
  //          /* anchor candidate list ( pTOk->sTokenID != TEXT)        */
  //          /* use as Hash:                                           */
  //          /* tested with 10147.f4-en4: no improvement, so forget it */
  //          /**********************************************************/
  //          fOK = AddToHashBlock (pITMNopSegs, pSeg->ulSegNum,
  //                                pTok->usLength,
  //                                pTok->pDataString,
  //                                (ULONG)pTok->sTokenid );
              /**************************************************************/
              /* add inline tags to anchor cand.list if they have ADDINFO   */
              /* TAG_ITM_RELEVANT ( 32 );                                   */
              /* TAG_ITM_RELEVANT is only effective for inline tags         */
              /* (KAT0288: Bookmaster: "id=" needs this ADDINFO )           */
              /**************************************************************/
              if (pTok->sAddInfo == TAG_ITM_RELEVANT)
              {
                ulLength = ITMGetRelLength(pITMIda->pLoadedTable,
                                           pTok->usLength,
                                           pTok->sAddInfo,
                                           pTok->sTokenid, szMorphData);

                /******************************************************/
                /* usLength limited to at most MAX_TERM_LEN-1 in      */
                /* ITMGetRelLength ...                                */
                /******************************************************/
                UTF16strncpy(szMorphData, pTok->pDataStringW, ulLength);
                szMorphData[ulLength] = EOS;
                MakeHashValue( ulRandom, usMaxRandom, szMorphData, &ulHash );

                fOK = AddToHashBlock (pITMNopSegs, ulSegNum,
                                      ulLength, pTok->pDataStringW,
                                      ulHash  );
              } /* endif */
            } /* endif */
            pTok++;
          } /* endwhile */

          break;
      } /* endswitch */
      ulSegNum++;                         // point to next segment
      pSeg = EQFBGetSegW(pDoc, ulSegNum);
    } /* endif */
  } /* endwhile */
  /********************************************************************/
  /* we are in sync at least at the end of the file                   */
  /********************************************************************/
  if ( fOK )
  {
    ulLength = 1;
    szMorphData[0] = EOS;
    fOK = AddToHashBlock (pITMNopSegs, ulSegNum, ulLength,
                           szMorphData, 0L  );
  }
  else
  {
#ifdef ITMTEST
//            fprintf( fOut,"AddToHash UtlAlloc fails");
#endif
  } /* endif */
  /********************************************************************/
  /* free document tag table --> placeholder for new code ....        */
  /********************************************************************/
  pITMIda->usStartSlider += 10;        // checknops is 10% of aligning
  return (fOK);
} /* end of function static GetBothTok(PITMIDA, PTBDOCUMENT,SHORT,PITMNOPSEGS) */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     GetSpecialTok
//------------------------------------------------------------------------------
// Function call:     GetSpecialTok  (pTok, pActTerm)
//------------------------------------------------------------------------------
// Description:       set fFound and Flags for special words such as
//                    AS/400, F1 - F22, A4, ( first = uppercase, followed by
//                    numbers)
//------------------------------------------------------------------------------
// Parameters:        PTOKENENTRY  pTok
//                    PFLAGOFFSLIST pActTerm
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE          special word found
//                    FALSE         token is no special word
//------------------------------------------------------------------------------
// Function flow:     compare whether tok is special word
//------------------------------------------------------------------------------
BOOL
GetSpecialTok
(
   PTOKENENTRY    pTok,
   PFLAGOFFSLIST  pActTerm
)
{
  BOOL     fFound = FALSE;
  PSZ_W    pData;
  LONG     lLenAS400 = 0;
  USHORT   usPos = 0;

  pData = pTok->pDataStringW + (pActTerm->usOffs);
  usPos = pActTerm->usLen;
  if (usPos > 1)
  {
    if ( isupper((UCHAR)*pData ) )
    {
       lLenAS400 = UTF16strlenCHAR(STR_AS400);
       if (memcmp(STR_AS400, pData, lLenAS400 * sizeof(CHAR_W)) == 0)
       {
         pActTerm->lFlags |=TF_NUMBER;
         if ( pActTerm->usLen < lLenAS400 )
         {
           // in same languages, AS/400 is 3 tokens; in these cases it is
           // nec to reset lFlags of the next 2 tokens to avoid re-usage!

           PFLAGOFFSLIST pNextActTerm;
           PFLAGOFFSLIST pNextNextActTerm;
           USHORT        usSumOfLengths = 0;

           pNextActTerm = pActTerm;
           pNextActTerm ++;

           usSumOfLengths = usPos + pNextActTerm->usLen;
           if ( (pNextActTerm->usOffs == pActTerm->usOffs + usPos) &&
                (usSumOfLengths <= lLenAS400) )
           {
             // reset flag of next term to avoid that it is used too
             pNextActTerm->lFlags = 0L;
             pNextNextActTerm = pNextActTerm;
             pNextNextActTerm ++;
             if ((pNextNextActTerm->usOffs == pActTerm->usOffs + usSumOfLengths)
                  && (usSumOfLengths + pNextNextActTerm->usLen <= lLenAS400 ))
             {
               pNextNextActTerm->lFlags = 0L;
             }
           }
         }
         pActTerm->usLen = (USHORT)(lLenAS400);
         fFound = TRUE;
         pActTerm->lFlags |= TF_NUMBER;
       }
       else
       {
         usPos--;
         pData++;
         while ( usPos && isdigit((UCHAR)*pData) )
         {
           usPos--;
           pData++;
         } /* endwhile */
         if ( usPos == 0 )
         {
           fFound = TRUE;
           pActTerm->lFlags |= TF_NUMBER;
         } /* endif */
       } /* endif */
    }  /* endif */
  } /* endif */

  return (fFound);
} /* end of function static GetSpecialTok(PToKENENTRY, PFLAGOFFSLIST) */
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     CheckIfExcluded
//------------------------------------------------------------------------------
// Function call:     CheckIfExcluded(pstExclNop, pData, sTokenID, sAddInfo)
//------------------------------------------------------------------------------
// Description:       check if NOP is excluded to be an anchor candidate
//------------------------------------------------------------------------------
// Parameters:        PITMNOPCOUNT pstExclNop
//                    PSZ          pData,
//                    SHORT        sTokenID,
//                    SHORT        sAddInfo
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE          use nop
//                    FALSE         nop is excluded
//------------------------------------------------------------------------------
// Function flow:     loop thru structure with excluded nops
//                       compare current nop with nop in exclusion structure
//                       if equal set return to false
//------------------------------------------------------------------------------
static BOOL
CheckIfExcluded
(
   PITMNOPCOUNT pstExclNop,
   PSZ_W        pData,
   SHORT        sTokenID,
   SHORT        sAddInfo
)
{
  BOOL         fUseNop = TRUE;
  USHORT       i = 0;

  /********************************************************************/
  /* loop thru NOP exclusion structure                                */
  /********************************************************************/
  while ( fUseNop && (i < pstExclNop->ulUsed ))
  {
    if ( TagsAreEqual(sTokenID, pstExclNop->psTokenID[i], sAddInfo,
                 pData, pstExclNop->ppData[i], pstExclNop->pusLen[i]) )
    {
      fUseNop = FALSE;
    }
    else
    {
      i++;
    } /* endif */
//    if ( (ulHash == pstExclNop->pulHash[i] ) &&
//         ItmCompChars(pData, pstExclNop->ppData[i] ) )
////       (strlen(pData) == strlen(pstExclNop->ppData[i]) ) &&
////       (!strncmp(pData, pstExclNop->ppData[i], strlen(pData) ) ) )
//    {
//      fUseNop = FALSE;
//    }
//    else
//    {
//      i++;
//    } /* endif */
  } /* endwhile */

  return (fUseNop );
} /* end of function CheckIfExcluded */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PostParseAnchor
//------------------------------------------------------------------------------
// Function call:     PostParseAnchor(PITMNOPSEGS, PITMNOPSEGS)
//------------------------------------------------------------------------------
// Description:       parse anchor list to delete multiples
//------------------------------------------------------------------------------
// Parameters:        PITMNOPSEGS   pSrcNop
//                    PITMNOPSEGS   pTgtNop
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE
//------------------------------------------------------------------------------
// Function flow:     while not at end of anchor list
//                      remember previous src and tgt anchor
//                      if current anchor different to last one
//                        goto next anchor
//                      else
//                        if tgt anchor equal to last tgt
//                           loop thru all equal tgts
//                          (equal tgtsegnum is anchor with different srcsegs)
//                          set delete count to number of equal tgts to
//                        else if src anchor equal to last src
//                         count all equal srcs
//                        endif
//                      endif
//                      if multiples have been counted
//                        delete them except the 1st
//                    endwhile
//                    NOTE:
//                    now changed so that in a sequence of anchors, where at
//                    least one segnum is twice, always the 1st anchor remains,
//                    i.e. is not deleted.
//                    Example:
//                            NOW                |      OLD
//                    src    tgt      src   tgt  |  src  tgt        src  tgt
//                    3       4  -->>   3   4    |   3   4   -->>   all anchors
//                    3       5                  |   3   5          deleted
//------------------------------------------------------------------------------

static
BOOL PostParseAnchor
(
  PITMNOPSEGS   pSrcNop,
  PITMNOPSEGS   pTgtNop
)
{
  BOOL      fOK = TRUE;
  ULONG     k = 1;
  ULONG     ulLastSrc;
  ULONG     ulLastTgt;
  ULONG     ulSrc;
  ULONG     ulTgt;
  ULONG     ulSrcIndex=0;
  ULONG     ulTgtIndex=0;
  ULONG     ulStartDel = 0;
  ULONG     ulCountDel = 0;
  LONG      l;

  while ( k < pSrcNop->ulUsed )
  {

    ulLastSrc = pSrcNop->pulSegs[k-1];
    ulLastTgt = pTgtNop->pulSegs[k-1];
    ulSrc = pSrcNop->pulSegs[k];
    ulTgt = pTgtNop->pulSegs[k];
    if ( ulSrc != ulLastSrc  )
    {
      if ( ulTgt != ulLastTgt )
      {
        k++;
      }
      else
      {
        ulTgtIndex = 0;
        while ( ulTgt == ulLastTgt )
        {
          ulTgtIndex++;
          ulTgt = pTgtNop->pulSegs[ulTgtIndex+k];
        }
         //equal TgtSegnum is anchor with different SrcSegnums
         if ( ulCountDel == 0 )
         {
           ulStartDel = k;
           ulCountDel = ulTgtIndex;
         }
         else
         {
           ulCountDel += ulTgtIndex;
         } /* endif */
         k += ulTgtIndex;
      } /* endif */
    }
    else                               //sources are equal
    {
     ulSrcIndex = 0;
     while ( ulSrc == ulLastSrc  )
     {
       ulSrcIndex++;
       ulSrc = pSrcNop->pulSegs[ulSrcIndex+k];
     } /* endwhile */

     if (ulCountDel == 0 )
     {
       ulStartDel = k;
       ulCountDel = ulSrcIndex;
     }
     else
     {
       ulCountDel += ulSrcIndex;
     } /* endif */

     k += ulSrcIndex;
    } /* endif */

    /******************************************************************/
    /* if multiples found, delete them except the 1st                 */
    /******************************************************************/
    if ( ulCountDel > 0 )
    {
      for ( l=ulStartDel;l<(LONG)pTgtNop->ulUsed ;l++ )
      {
        pTgtNop->pulSegs[l] = pTgtNop->pulSegs[l+ulCountDel];
        pSrcNop->pulSegs[l] = pSrcNop->pulSegs[l+ulCountDel];
      } /* endfor */
      pSrcNop->ulUsed -= ulCountDel;
      k -= ulCountDel;
      ulStartDel = ulCountDel = 0;
    } /* endif */

  } /* endwhile */
  return (fOK);
} /* end of function static PostParseAnchor */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     FillALignStruct(PITMIDa, PALIGNEMENT,SHORT, SHORT)
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       loop through all aligned segments of the paragraph
//                    and fill the alignment structure
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda  pointer to instance data
//                    PALIGNMENT pAlign pointer  to alignment array
//                    SHORT    sNum     number of aligned sentences
//                    SHORT    sFill
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   success
//                    FALSE  error, alignment could not be added, mem-shortage
//------------------------------------------------------------------------------
// Function flow:     if there are aligned segments
//                     while not thru all aligned segments
//                       get ptr to current alignment in paragraph
//                       if no X value of current alignment
//                         store NUL_ONE alignment in global alignstruct
//                       else if no Y value in current alignment
//                         store ONE_NUL in global alignstruct
//                       else if a 2nd X and a 2nd Y value in current alignm.
//                         store two ONE_ONE alignments
//                       else if a 2nd X value exists
//                         store two ONE_ONE alignments with same Y value
//                       else if a 2nd Y value exists:
//                         store two ONE_ONE alignments with same X value
//                       else store regular ONE_ONE
//                      get next aligned segments in paragraph
//                     endwhile
//                     if indicator is set to free empty spaces
//                      (happens during last call of checking..)
//                      if not filled til 'FillEnd'
//                        move rest of entries to the left to delete free
//                         positions
//------------------------------------------------------------------------------


BOOL FillAlignStruct
(
   PITMIDA       pITMIda,
   PALIGNEMENT   pAlign,
   SHORT         sNum,
   SHORT         sFill
)
{
  BOOL fOK = TRUE;               // success indicator
  ULONG       i, j;                      // indices
  ULONG       ulSrc;
  ULONG       ulTgt1;
  PALIGNEMENT pCurAlign;
  ULONG       ulIndex;
  PALLALIGNED pAligned;
  ULONG       ulDelta;
  SHORT       k;
  /********************************************************************/
  /* print out the alignements ...                                    */
  /********************************************************************/
  /********************************************************************/
  /* get starting points i.e. start segments                          */
  /********************************************************************/
  if ( sNum )
  {
    i = 0 ;          // pITMIda->itmSrcText.pulSegStart[ 0 ];
    j = 0;           // pITMIda->itmTgtText.pulSegStart[ 0 ];
    k = 0;
/**********************************************************************/
/* decrease sNum by 1 to garantuee that boundery seg is not written   */
/* twice   is wrong!!!                                                */
/**********************************************************************/
    while ( (k < (sNum)) && fOK && !pITMIda->fKill )
    {
      pCurAlign = pAlign+k;
     /******************************************************************/
     /* determine type of alignement                                   */
     /******************************************************************/
     if ( ! pCurAlign->sX1 )
     {
       ulSrc = 0;
       ulTgt1 = pITMIda->itmTgtText.pulSegStart[j];
       fOK = AddToAlignStruct(&pITMIda->Aligned,ulSrc,
                         ulTgt1, NUL_ONE, pCurAlign->sDist, sFill );
       j++;
     }
     else if ( !pCurAlign->sY1  )
     {
       ulTgt1 = 0;
       ulSrc = pITMIda->itmSrcText.pulSegStart[i];
       fOK = AddToAlignStruct(&pITMIda->Aligned,ulSrc,
                         ulTgt1, ONE_NUL, pCurAlign->sDist, sFill  );
       i++;
     }
     else if ( pCurAlign->sX2 && pCurAlign->sY2 )
     {
       /****************************************************************/
       /* it is a 2-2                                                  */
       /****************************************************************/
       ulSrc = pITMIda->itmSrcText.pulSegStart[i];
       ulTgt1 = pITMIda->itmTgtText.pulSegStart[j];
       fOK = AddToAlignStruct(&pITMIda->Aligned, ulSrc,
                          ulTgt1, ONE_ONE, pCurAlign->sDist, sFill  );
       i++;
       j++;
       ulSrc = pITMIda->itmSrcText.pulSegStart[i];
       ulTgt1 = pITMIda->itmTgtText.pulSegStart[j];
       if ( fOK )
       {
         fOK = AddToAlignStruct(&pITMIda->Aligned, ulSrc,
                            ulTgt1, ONE_ONE, pCurAlign->sDist, sFill  );
       } /* endif */
       i++;
       j++;

     }
     else if ( pCurAlign->sX2 )     // it is a 2-1
     {
       ulSrc = pITMIda->itmSrcText.pulSegStart[i];
       ulTgt1 = pITMIda->itmTgtText.pulSegStart[j];
       fOK = AddToAlignStruct(&pITMIda->Aligned, ulSrc,
                           ulTgt1, ONE_ONE, pCurAlign->sDist, sFill  );
       i++;
       ulSrc = pITMIda->itmSrcText.pulSegStart[i];
       if ( fOK )
       {
         fOK = AddToAlignStruct(&pITMIda->Aligned, ulSrc,
                             ulTgt1, ONE_ONE, pCurAlign->sDist, sFill  );
       } /* endif */

       i++;
       j++;
     }
     else if ( pCurAlign->sY2 )                    // it is an ONE_TWO
     {
       /***************************************************************/
       /* changed 20.2.94:split up in two 1:1 alignments              */
       /***************************************************************/
       ulSrc = pITMIda->itmSrcText.pulSegStart[i];
       ulTgt1 = pITMIda->itmTgtText.pulSegStart[j];
       fOK = AddToAlignStruct(&pITMIda->Aligned, ulSrc,
                           ulTgt1, ONE_ONE, pCurAlign->sDist, sFill  );
       j++;
       ulTgt1 = pITMIda->itmTgtText.pulSegStart[j];
       if ( fOK )
       {
         fOK = AddToAlignStruct(&pITMIda->Aligned, ulSrc,
                             ulTgt1, ONE_ONE, pCurAlign->sDist, sFill  );
       } /* endif */
       i++;
       j++;
     }
     else
     {
       ulSrc = pITMIda->itmSrcText.pulSegStart[i];
       ulTgt1 = pITMIda->itmTgtText.pulSegStart[j];
       fOK = AddToAlignStruct(&pITMIda->Aligned, ulSrc,
                        ulTgt1, ONE_ONE, pCurAlign->sDist, sFill  );
       i++;
       j++;
     } /* endif */
     k++;                                // point to next aligned stuff
    } /* endwhile */
  } /* endif */
  if ( (sFill == USERFREE ) && fOK )
  {
    pAligned = &(pITMIda->Aligned);
    ulIndex = pAligned->ulFillIndex;          //pts to next free position
    if ( ulIndex <= pAligned->ulFillEnd )
    {
      ulDelta =  pAligned->ulFillEnd - ulIndex + 1;
      /**************************************************************/
      /* delete the positions which are free now                    */
      /**************************************************************/
      pAligned->ulUsed -= ulDelta;
      for ( i = ulIndex ; i <= pAligned->ulUsed ;i++ )
      {
         pAligned->pulSrc[i]  = pAligned->pulSrc[i + ulDelta];
         pAligned->pulTgt1[i] = pAligned->pulTgt1[i + ulDelta];
         pAligned->psDist[i]  = pAligned->psDist[i + ulDelta];
         pAligned->pbType[i]  = pAligned->pbType[i + ulDelta];
      } /* endfor */
      i++;
      for ( i=pAligned->ulUsed + 1;i <= pAligned->ulUsed + ulDelta ;i++ )
      {
        pAligned->pulSrc[i] = 0;        // set empty ones to 0
        pAligned->pulTgt1[i]  = 0;
      } /* endfor */
    } /* endif */
  } /* endif */

  return (fOK);
} /* end of function static FillALignStruct */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     AddToAlignStruct
//------------------------------------------------------------------------------
// Function call:     AddToAlignStruct (pAligned,
//                                     ulSrcs1, ulTgt1, sType,
//                                                        sFill             )
//------------------------------------------------------------------------------
// Description:       add a new alignment to alignment structure
//------------------------------------------------------------------------------
// Parameters:        PALLALIGNED pAligned,        ITM Align struct to be used
//                    ULONG      ulSrc,          src segnum
//                    ULONG      ulTgt1,         tgt segnum
//                    USHORT     usType,         type of alignment
//                    SHORT      sDist,          distance of alignment
//                    SHORT      sFill
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    if ok
//                    FALSE   if something went wrong
//------------------------------------------------------------------------------
// Function flow:     if all allocated segments are already used
//                      allocate new space
//                    endif
//                    if ok
//                      if initially filling the alignstructure
//                        fill in the entry at end of structure
//                      else (filling due to realign)
//                        fill at next 'whole' stored in ulFillIndex
//                        if ulFillIndex > ulFillEnd, make room for one entry
//                      point to next entry
//                    endif
//                    return whether successful
//------------------------------------------------------------------------------

BOOL AddToAlignStruct
(
 PALLALIGNED pAligned,
 ULONG       ulSrc,
 ULONG       ulTgt1,
 USHORT     usType,
 SHORT      sDist,
 SHORT      sFill
)
{

  BOOL  fOK = TRUE;                    // success indicator
  LONG  lOldLen;                       // old length
  LONG  lNewLen;                       // new length
  ULONG ulUsed;
  BOOL  fTwice = FALSE;
  ULONG ulI;
  ULONG ulIndex;

  /********************************************************************/
  /* check if still one segment is free to be filled ...              */
  /* if not allocate memory first ...                                 */
  /********************************************************************/
  if ( pAligned->ulAlloc <= pAligned->ulUsed + 1 )
  {
    ULONG ulAllocOrg = pAligned->ulAlloc;
    /******************************************************************/
    /* get old and new length for reallocation...                     */
    /******************************************************************/
    lOldLen =  (pAligned->ulAlloc) * sizeof( ULONG );
    pAligned->ulAlloc += NEW_ALIGN;
    lNewLen =  (LONG) pAligned->ulAlloc * sizeof( ULONG );

    fOK = UtlAlloc( (PVOID *) &(pAligned->pulSrc), lOldLen, lNewLen, ERROR_STORAGE );

    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *) &(pAligned->pulTgt1),
                      lOldLen, lNewLen, ERROR_STORAGE );
    } /* endif */
    if ( fOK )
    {
      lOldLen =  ulAllocOrg * sizeof( SHORT );
      lNewLen =  pAligned->ulAlloc * sizeof( SHORT );

      fOK = UtlAlloc( (PVOID *) &(pAligned->psDist),
                      lOldLen, lNewLen, ERROR_STORAGE );
    } /* endif */
    if ( fOK )
    {
      lOldLen =  ulAllocOrg * sizeof( BYTE );
      lNewLen =  pAligned->ulAlloc * sizeof( BYTE );
      fOK = UtlAlloc( (PVOID *) &(pAligned->pbType),
                      lOldLen, lNewLen, ERROR_STORAGE );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* fill in the new values for this NOP block                        */
  /********************************************************************/
  if ( fOK )
  {
    if (sFill == INITFILL )
    {
      if ( pAligned->ulUsed )
      {
        ulUsed = pAligned->ulUsed - 1;
        //add only if not equal to previous entry
        if ( (pAligned->pulSrc[ulUsed] == ulSrc) &&
             (pAligned->pulTgt1[ulUsed] == ulTgt1)  )
        {
          //do not add
          fTwice = TRUE;
        } /* endif */
        if ( (usType == NUL_ONE) && (pAligned->pulTgt1[ulUsed] == ulTgt1) )
        {
          fTwice = TRUE;               // seg is already aligned, so don't add
        } /* endif */
        if ( (usType == ONE_NUL) && (pAligned->pulSrc[ulUsed] == ulSrc) )
        {
          fTwice = TRUE;
        } /* endif */
      } /* endif */
    }
    else
    {
      if ( pAligned->ulFillIndex )
      {
        ulUsed = pAligned->ulFillIndex - 1;
        //add only if not equal to previous entry
        if ( (pAligned->pulSrc[ulUsed] == ulSrc) &&
             (pAligned->pulTgt1[ulUsed] == ulTgt1)  )
        {
          //do not add
          fTwice = TRUE;
        } /* endif */
        if ( (usType == NUL_ONE) && (pAligned->pulTgt1[ulUsed] == ulTgt1) )
        {
          fTwice = TRUE;               // seg is already aligned, so don't add
        } /* endif */
        if ( (usType == ONE_NUL) && (pAligned->pulSrc[ulUsed] == ulSrc) )
        {
          fTwice = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */
    if ( !fTwice )
    {
      if (sFill == INITFILL )
      {
        ulUsed  = pAligned->ulUsed;
        pAligned->pulSrc[ulUsed] = ulSrc;
        pAligned->pulTgt1[ulUsed] = ulTgt1;
        pAligned->psDist[ulUsed] = sDist;
        pAligned->pbType[ulUsed] = (BYTE)usType;
        pAligned->ulUsed ++;                       // point to next entry
      }
      else                                         // USERFILL or USERFREE
      {
        ulIndex = pAligned->ulFillIndex;
        if ( ulIndex > pAligned->ulFillEnd )
        {
          //insert space; one position is always free
          for ( ulI = pAligned->ulUsed ; ulI > pAligned->ulFillIndex ;ulI-- )
          {
             pAligned->pulSrc[ulI]  = pAligned->pulSrc[ulI - 1];
             pAligned->pulTgt1[ulI] = pAligned->pulTgt1[ulI - 1];
             pAligned->psDist[ulI]  = pAligned->psDist[ulI - 1];
             pAligned->pbType[ulI]  = pAligned->pbType[ulI - 1];
          } /* endfor */
          pAligned->ulUsed ++;
        } /* endif */
        pAligned->pulSrc[ulIndex] = ulSrc;
        pAligned->pulTgt1[ulIndex] = ulTgt1;
        pAligned->psDist[ulIndex] = sDist;
        pAligned->pbType[ulIndex] = (BYTE)usType;
        pAligned->ulFillIndex ++;
      } /* endif */
    } /* endif */
  } /* endif */
  return ( fOK );
} /* end of function AddTOALignStruct */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ParseAlignStruct
//------------------------------------------------------------------------------
// Function call:     ParseAlignStruct( pITMIda,pITMIda->Aligned)
//------------------------------------------------------------------------------
// Description:       delete overlapping etc. in alignment struct
//                    in the resulting alignment each segment must be aligned
//                    exactly ONCE, 2:1 and 1:2 alignments are allowed
//------------------------------------------------------------------------------
// Parameters:        PITMIDA     pITMIda,
//                    PALLALIGNED pAligned
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    successful
//                    fALSE   error occurred
//------------------------------------------------------------------------------
// Function flow:     Check 0:1 and 1:0 alignments (is deleted if it is
//                      contained in a 1:1 match)
//                    Check for triple occurrencies of a segment
//                    If changes in last check: Check for 0:1, 1:0 again
//                    Check 2:1 and 1:2 alignments:
//                      if the two segments are not consecutive, build a
//                      1:1 and 1:0 (0:1) alignment instead of the 2:1 (1:2)
//                    Check 0:1 / 1:0 again
//                    Check for X toX matches
//                    If something changed, Check 1:0 / 0:1 again
//------------------------------------------------------------------------------
BOOL
ParseAlignStruct
(
 PITMIDA     pITMIda,
 PALLALIGNED pAligned
)
{
  ULONG       ulUsed;
  PTBDOCUMENT pSrcDoc;
  PTBDOCUMENT pTgtDoc;
  BOOL        fOK = TRUE;
  BOOL        fSplit = FALSE;
  ULONG       ulMax;
  BOOL        fRealign;                   //TRUE if realign, FALSE if 1stalign
#ifdef ITMTEST
//    ULONG       ulTemp1;
#endif

  pSrcDoc = &(pITMIda->TBSourceDoc);
  pTgtDoc = &(pITMIda->TBTargetDoc);
  pITMIda->usStartSlider = 95;

  if ( pITMIda->stVisDocSrc.pVisState )
  {
    fRealign = TRUE;
  }
  else
  {
    fRealign = FALSE;
  } /* endif */
  /********************************************************************/
  /* delete 0:1 (1:0 ) match if it is contained in 1:1 match          */
  /********************************************************************/
  CheckUnaligned(pITMIda,pAligned, FALSE, fRealign );
  if ( !fRealign )
  {
    pITMIda->usStartSlider ++;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
  } /* endif */
  /********************************************************************/
  /* 3rd path : if a seg occurs more than twice, generate 0:1, 1:0    */
  /* matches :  x:1 -> 2:1 and some 1:0                               */
  /*            1:x -> 1:2 and some 0:1       ( x > 2)                */
  /* err in chap4 of adams: this must happen before 2nd path          */
  /********************************************************************/
  fSplit = CheckTriple ( pITMIda, pAligned, fRealign );
  if ( !fRealign )
  {
    pITMIda->usStartSlider ++;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
  } /* endif */
#ifdef ITMTEST
//  if (fSplit )
//  {
//    fOut = fopen ("ITMSTAT.OUT", "a" );
//    fprintf (fOut, "After CheckTriple \n");
//    for ( ulTemp1=0;ulTemp1 < (pITMIda->Aligned.ulUsed);ulTemp1++ )
//    {
//      fprintf( fOut,
//               "%4d Seg: %4d %4d 4%d \n",
//                ulTemp1,
//                pITMIda->Aligned.pulSrc[ulTemp1],
//                pITMIda->Aligned.pulTgt1[ulTemp1],
//                (USHORT) pITMIda->Aligned.pbType[ulTemp1]);
//    } /* endfor */
//    fclose(fOut);
//  } /* endif */
#endif
  if ( fSplit )
  {
    CheckUnaligned(pITMIda, pAligned, FALSE, fRealign );
    fSplit = FALSE;
  } /* endif */

  /********************************************************************/
  /* 2nd path: check 2:1 (1:2) alignments:if the 2 segs are not       */
  /* consecutive, split up in 1:1 and 1:0 (0:1) alignment             */
  /* if Realign, reparse only in realigned area(22.1.95)              */
  /********************************************************************/
#ifndef EU
  if (fRealign )
  {
    ulUsed = pAligned->ulFillStart;       // realigned area
    ulMax = pAligned->ulFillIndex;
  }
  else
  {
    ulUsed = 0;                           // total alignment list
    ulMax = pAligned->ulUsed;
  } /* endif */
  while ( ulUsed < ulMax )
  {
    if ((pAligned->pulSrc[ulUsed]) &&
         (pAligned->pulSrc[ulUsed] == pAligned->pulSrc[ulUsed+1]) )
    {
      if ( pAligned->pulTgt1[ulUsed] != (pAligned->pulTgt1[ulUsed+1] - 1) )
      {
         AlignSplit(pITMIda, pAligned, pSrcDoc, pTgtDoc,
                        ulUsed, NUL_ONE, fRealign);
         fSplit = TRUE;
//         ulUsed ++;
      } /* endif */
    }
    else
    {
      if ( (pAligned->pulTgt1[ulUsed] ) &&
           ( pAligned->pulTgt1[ulUsed] == pAligned->pulTgt1[ulUsed+1]) )
      {
        if ( pAligned->pulSrc[ulUsed] != (pAligned->pulSrc[ulUsed+1] - 1) )
        {
           AlignSplit(pITMIda, pAligned, pSrcDoc, pTgtDoc,
                        ulUsed, ONE_NUL, fRealign);
           fSplit = TRUE;
//           ulUsed++;
        } /* endif */
      } /* endif */
    } /* endif */

    ulUsed++;
  } /* endwhile */
#endif

  /********************************************************************/
  /* now in the Splitting new 0:1 1:0 matches are generated,          */
  /* hence a repetition of the 1st path is nec                        */
  /********************************************************************/
  if ( fSplit )
  {
    CheckUnaligned(pITMIda, pAligned, FALSE, fRealign );
    fSplit = FALSE;
  } /* endif */
  /********************************************************************/
  /* 4th path: split x:x matches in 2:1 / 1:2 and 0:1 / 1:0           */
  /* example:     a - b           a - b      2 : 1                    */
  /*              c - b    ->     c - b                               */
  /*              c - d           0 - d      0 : 1                    */
  /********************************************************************/
  fSplit = CheckXtoX (pITMIda, pAligned, fRealign );
  if ( !fRealign )
  {
    pITMIda->usStartSlider ++;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
  } /* endif */
#ifdef ITMTEST
//{ ULONG  ulTemp1;
//if (fSplit )
//{
//  fOut = fopen ("ITMSTAT.OUT", "a" );
//  fprintf (fOut, "After CheckXtoX   \n");
//  for ( ulTemp1=0;ulTemp1 < (pITMIda->Aligned.ulUsed);ulTemp1++ )
//  {
//    fprintf( fOut,
//             "%4d Seg: %4d %4d 4%d \n",
//              ulTemp1,
//              pITMIda->Aligned.pulSrc[ulTemp1],
//              pITMIda->Aligned.pulTgt1[ulTemp1],
//              (USHORT) pITMIda->Aligned.pbType[ulTemp1]);
//  } /* endfor */
//
//  fclose(fOut);
//} /* endif */
//}
#endif
//if ( fSplit )
//{
    fSplit = CheckUnaligned(pITMIda, pAligned, TRUE, fRealign );
    if ( fSplit  )
    {
      fSplit = CheckUnaligned(pITMIda, pAligned, TRUE, fRealign);
    } /* endif */
//} /* endif */

  Check2Unaligned(pAligned, pITMIda, fRealign);            // make 1:1 of 0:1 and 1:0

  if ( !fRealign )
  {
    pITMIda->usStartSlider ++;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
  } /* endif */
  return ( fOK );
} /* end of function ParseAlignStruct(pITMIda->Aligned) */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     CheckTriple
//------------------------------------------------------------------------------
// Function call:     CheckTriple(pITMIDa, pAligned, fRealign)
//------------------------------------------------------------------------------
// Description:       check whether one segnum occurs more than twice
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIDa
//                    PALLALIGNED  pAligned
//                    BOOL         fRealign
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    if something splitted
//                    FALSE   nothing changed
//------------------------------------------------------------------------------
// Function flow:     while not at end of alignment table
//                      check for 1:x alignment
//                      if x is more than two
//                        build 0:1 matches of all later alignments
//                    endwhile
//                    start at begin again
//                    while not at end of alignment table
//                      check for x:1 alignment
//                      if x is more than two
//                        build 1:0 matches of all later alignments
//                    endwhile
//                    Example:     seg 4 - seg 6         4 - 6   (1:2)
//                                 seg 4 - seg 7 --->    4 - 7
//                        ( 1:x )  seg 4 - seg 9         0 - 9    (0:1)
//                                 seg 4 - seg 10        0 - 10   (0:1)
//------------------------------------------------------------------------------
static
BOOL
CheckTriple
(
  PITMIDA      pITMIda,
  PALLALIGNED  pAligned,
  BOOL         fRealign
)
{
  ULONG    ulUsed = 0;
  ULONG    ulMax = 0;
  ULONG    ulI = 0;
  USHORT   usNumEqual = 0;
  BOOL     fSplit = FALSE;

  pITMIda;
  if (fRealign )
  {
    ulUsed = pAligned->ulUsed;            // realigned area
    ulMax = pAligned->ulFillIndex;
  }
  else
  {
    ulUsed = 0;                           // total alignment list
    ulMax = pAligned->ulUsed;
  } /* endif */

  while ( ulUsed < ulMax )
  {
    ulI = ulUsed;
    usNumEqual = 0;
    /******************************************************************/
    /* check for 1:x                                                  */
    /******************************************************************/
    while ( ( ulI < ulMax ) &&
            ( pAligned->pulSrc[ulI] ) &&
            (pAligned->pulSrc[ulI] == pAligned->pulSrc[ulI+1]) )
    {
      usNumEqual ++;
      ulI++;
    } /* endwhile */
    if ( usNumEqual > 1 )
    {
      usNumEqual --;
      ulUsed ++;
      while ( usNumEqual )                 // build 0:1 matches
      {
         AlignLastSplit(pAligned, ulUsed, NUL_ONE);
         fSplit = TRUE;
         usNumEqual --;
         ulUsed++;
      } /* endwhile */
    } /* endif */
    ulUsed++;
  } /* endwhile */

  if (fRealign )
  {
    ulUsed = pAligned->ulFillStart;       // realigned area
    ulMax = pAligned->ulFillIndex;
  }
  else
  {
    ulUsed = 0;                           // total alignment list
    ulMax = pAligned->ulUsed;
  } /* endif */
  while ( ulUsed < ulMax )
  {
    ulI = ulUsed;
    usNumEqual = 0;
    /******************************************************************/
    /* check for x:1                                                  */
    /******************************************************************/
    while ( ( ulI < ulMax ) &&
            ( pAligned->pulTgt1[ulI] ) &&
            ( pAligned->pulTgt1[ulI] == pAligned->pulTgt1[ulI+1]) )
    {
      usNumEqual ++;
      ulI++;
    } /* endwhile */
    if ( usNumEqual > 1 )
    {
      usNumEqual --;
      ulUsed ++;
      while ( usNumEqual )                 // build 1:0 matches
      {
         AlignLastSplit(pAligned, ulUsed, ONE_NUL);
         fSplit = TRUE;
         ulUsed++;
         usNumEqual --;
      } /* endwhile */
    } /* endif */
    ulUsed++;
  } /* endwhile */
  return (fSplit );
} /* end of function CheckTriple */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     CheckXtoX
//------------------------------------------------------------------------------
// Function call:     CheckXtoX(pALigned)
//------------------------------------------------------------------------------
// Description:       check for x : x alignments
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda
//                    PALLALIGNED  pAligned
//                    BOOL         fRealign
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    if something splitted
//                    FALSE   if no changes
//------------------------------------------------------------------------------
// Function flow:     while not thru all alignments
//                      if current 2:1 alignment overlaps previous 1:2 alignm.
//                        build 0:1 from last alignment
//                    endwhile
//                    example:
//                       (1:2)    4 - 6             4 - 6     (1:2)
//                                4 - 7  ------>    4 - 7
//                        (2:1)   5 - 7             5 - 0     ( 1 : 0)
//------------------------------------------------------------------------------
static
BOOL
CheckXtoX
(
  PITMIDA      pITMIda,
  PALLALIGNED  pAligned,
  BOOL         fRealign
)
{
  ULONG    ulUsed;
  ULONG    ulMax;
  BOOL     fTwice = FALSE;
  BOOL     fSplit = FALSE;

  pITMIda;
  if (fRealign )
  {
    ulUsed = pAligned->ulFillStart;       // parse realigned area only
    ulMax = pAligned->ulFillIndex;
  }
  else
  {
    ulUsed = 0;                           // parse total alignment list
    ulMax = pAligned->ulUsed;
  } /* endif */
  while ( ulUsed < ulMax )
  {
    /******************************************************************/
    /* check for 2:1                                                  */
    /******************************************************************/
    if ( pAligned->pulSrc[ulUsed] &&
         ( pAligned->pulSrc[ulUsed] == pAligned->pulSrc[ulUsed + 1] ) )
    {
      if ( fTwice )        // previous 1:2 match overlaps
      {
        AlignLastSplit(pAligned, ulUsed, NUL_ONE );
        fSplit = TRUE;
        fTwice = FALSE;
      }
      else
      {
        fTwice = TRUE;
      } /* endif */
    }
    else if ( pAligned->pulTgt1[ulUsed] &&
         ( pAligned->pulTgt1[ulUsed] == pAligned->pulTgt1[ulUsed + 1] ) )
    {
      if ( fTwice )        // previous 2:1 match overlaps
      {
        AlignLastSplit(pAligned, ulUsed, ONE_NUL );
        fSplit = TRUE;
        fTwice = FALSE;
      }
      else
      {
        fTwice = TRUE;
      } /* endif */
    }
    else
    {
      fTwice = FALSE;
    } /* endif */
    ulUsed++;

  } /* endwhile */

  return (fSplit );
} /* end of function CheckTriple */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     CheckUnaligned(pITMIda,pAligned, BOOL,BOOL )
//------------------------------------------------------------------------------
// Function call:     CheckUnaligned(pITMIda, pAligned, fLastCheck, fRealign)
//------------------------------------------------------------------------------
// Description:       check whether 1:0 or 0:1 alignments are contained
//                    in other 1:1 (1:2, 2:1)alignments; if so , delete them
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda,
//                    PALLALIGNED  pAligned,
//                    BOOL         fLastCheck,
//                    BOOL         fRealign
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   if something changed
//                    FALSE  nothing changed
//------------------------------------------------------------------------------
// Function flow:     parse for duplicates in src:
//                      while not thru total alignment table
//                        if 0:0 alignment happened, delete it
//                        check whether srcseg is duplicate (skip 0:1 entries)
//                        if so,
//                          if current 1:0 is contained in next 1:1,
//                           delete current 1:0
//                          if current 1:1 contains later 1:0, set the 1:0
//                          to 0:0 to delete it during the further processing
//                          if during last check and still unclear duplicates,
//                            set duplicate occurrence to 0:1
//                          if not duplicate, do not delete alignment
//                      endwhile
//                      set empty entries to zero
//                      parse for duplicates in target
//                        same as above for the source
//------------------------------------------------------------------------------
static BOOL
CheckUnaligned
(
 PITMIDA      pITMIda,
 PALLALIGNED  pAligned,
 BOOL         fLastCheck,
 BOOL         fRealign
)
{
  ULONG    ulUsed;
  ULONG    ulAct;
  ULONG    ulI;
  BOOL     fSplit = FALSE;
  ULONG    ulMax;
#ifdef ITMTEST
//  ULONG      ulTemp1;
#endif
  pITMIda;
  /********************************************************************/
  /* parse for duplicates in src                                      */
  /********************************************************************/
  if (fRealign )
  {
    ulUsed = pAligned->ulFillStart;       // realigned area
    ulMax = pAligned->ulFillIndex;
  }
  else
  {
    ulUsed = 1;                           // total alignment list
    ulMax = pAligned->ulUsed;
  } /* endif */

  ulAct = ulUsed;
  while ( ulUsed < ulMax )
  {
    /******************************************************************/
    /* if pAligned->pulSrc = pAligned->pulTgt1 = 0 then delete it     */
    /******************************************************************/
    if ( (pAligned->pulSrc[ulUsed] == 0)
        && (pAligned->pulTgt1[ulUsed] == 0) )
    {
      ulUsed++;                     //delete cur alignment

    } /* endif */
    /****************************************************************/
    /* check whether pulSrc[ulUsed] is duplicate                    */
    /* there may be lots of entries ( = 0) between the 2 equal srcsegs */
    /****************************************************************/
    ulI = CheckDuplicate (pAligned->pulSrc, ulUsed, pAligned->ulUsed );
    if ( ulI )
    {
      if ( (pAligned->pbType[ulUsed] == ONE_NUL) &&
               (pAligned->pbType[ulI] == ONE_ONE ))
      {
         /**************************************************************/
         /*delete the cur ONE_NUL alignment because it is contained in */
         /* the next   ONE_ONE alignment                               */
         /**************************************************************/
         ulUsed++;
      }
      else
      {
         if ( (pAligned->pbType[ulUsed] == ONE_ONE) &&
                  (pAligned->pbType[ulI] == ONE_NUL ))
         {
            pAligned->pulSrc[ulI] = 0;
            pAligned->pulTgt1[ulI] = 0;
            /**************************************************************/
            /*delete  ONE_NUL alignment later on; set it to 0 now         */
            /* the current ONE_ONE alignment contains later 1:0           */
            /**************************************************************/
         }
         else
         {
            /**************************************************************/
            /* sources equal but unclear what to do /e.g.pulSrc[] = 0!!   */
            /**************************************************************/
            if ( fLastCheck && (ulI > (ULONG)ulUsed + 1))
            {
              /*********************************************************/
              /* set 2nd occurrence to a 0:1 match                     */
              /*********************************************************/
              pAligned->pbType[ulI] = NUL_ONE;
              pAligned->pulSrc[ulI] = 0;
              fSplit = TRUE;
            }
            else
            {
               AlignCopy(pAligned, ulAct, ulUsed);
               ulAct++;
               ulUsed++;
            } /* endif */
         }
      } /* endif */
    }
    else
    {
      /**************************************************************/
      /* nothing special with alignment                             */
      /**************************************************************/
      AlignCopy(pAligned, ulAct, ulUsed);
      ulAct++;
      ulUsed++;
    } /* endif */
  } /* endwhile */
  /********************************************************************/
  /* move rest of alignmentlist (nec if realign)                      */
  /* only nec if something has been changed(ulAct != ulUsed)          */
  /********************************************************************/
  if (ulAct != ulUsed )
  {
    while (ulUsed < pAligned->ulUsed )
    {
      AlignCopy(pAligned, ulAct, ulUsed);
      ulAct++;
      ulUsed++;
    } /* endwhile */

    /********************************************************************/
    /* set empty entries to zero                                        */
    /********************************************************************/
    for ( ulI = ulAct;ulI <= pAligned->ulUsed;ulI++ )
    {
       pAligned->pulSrc[ulI]  = 0;
       pAligned->pulTgt1[ulI] = 0;
    } /* endfor */

    if (fRealign )
    {
      pAligned->ulFillIndex -= ulUsed - ulAct;
    } /* endif */
    pAligned->ulUsed = ulAct;

  } /* endif */
  /********************************************************************/
  /* parse for duplicates in target                                   */
  /********************************************************************/
  if (fRealign )
  {
    ulUsed = pAligned->ulFillStart;       // realigned area
    ulMax = pAligned->ulFillIndex;
  }
  else
  {
    ulUsed = 1;                           // total alignment list
    ulMax = pAligned->ulUsed;             // 1st entry remains unchanged
  } /* endif */

  ulAct = ulUsed;
  while ( ulUsed < ulMax )
  {
    /******************************************************************/
    /* if pAligned->pulSrc = pAligned->pulTgt1 = 0 then delete it     */
    /******************************************************************/
    if ( (pAligned->pulSrc[ulUsed] == 0)
        && (pAligned->pulTgt1[ulUsed] == 0) )
    {
      ulUsed++;                     //delete cur alignment
    } /* endif */
    ulI = CheckDuplicate (pAligned->pulTgt1, ulUsed, pAligned->ulUsed );
    if ( ulI )
    {
      if (( pAligned->pbType[ulUsed] == NUL_ONE) &&
         (pAligned->pbType[ulI] == ONE_ONE ) )
      {
        /**************************************************************/
        /*delete the cur NUL_ONE alignment because it is contained in */
        /* the next   ONE_ONE alignment                               */
        /**************************************************************/
        ulUsed++;
//      AlignCopy(pAligned, ulAct, ulUsed);
//      ulAct++;
//      ulUsed ++;
      }
      else
      {
        if ( (pAligned->pbType[ulUsed] == ONE_ONE) &&
                 (pAligned->pbType[ulI] == NUL_ONE ))
        {
           pAligned->pulSrc[ulI] = 0;
           pAligned->pulTgt1[ulI] = 0;
           /**************************************************************/
           /*delete         ONE_NUL alignment because it is contained in */
           /* the current ONE_ONE alignment                              */
           /**************************************************************/
//         AlignCopy(pAligned, ulAct, ulUsed);
//         ulAct++;
//         ulUsed += 2;
        }
        else
        {
           /**************************************************************/
           /* tgts    equal but unclear what to do /e.g.pulTgt[] = 0!!   */
           /**************************************************************/
           if ( fLastCheck && (ulI > (ULONG)ulUsed + 1))
           {
             /*********************************************************/
             /* set 2nd occurrence to a 1:0 match                     */
             /*********************************************************/
             pAligned->pbType[ulI] = ONE_NUL;
             pAligned->pulTgt1[ulI] = 0;
             fSplit = TRUE;
           }
           else
           {
             AlignCopy(pAligned, ulAct, ulUsed);
             ulAct++;
             ulUsed++;
           } /* endif */
        } /* endif */
      } /* endif */
    }
    else
    {
      /**************************************************************/
      /* nothing special with alignment                             */
      /**************************************************************/
      AlignCopy(pAligned, ulAct, ulUsed);
      ulAct++;
      ulUsed++;
    } /* endif */
  } /* endwhile */
  /********************************************************************/
  /* move rest of alignmentlist (nec if realign)                      */
  /********************************************************************/
  if (ulAct != ulUsed )
  {
    while (ulUsed < pAligned->ulUsed )
    {
      AlignCopy(pAligned, ulAct, ulUsed);
      ulAct++;
      ulUsed++;
    } /* endwhile */
    /********************************************************************/
    /* set empty entries to zero                                        */
    /********************************************************************/
    for ( ulI = ulAct;ulI <= pAligned->ulUsed;ulI++ )
    {
       pAligned->pulSrc[ulI]  = 0;
       pAligned->pulTgt1[ulI] = 0;
    } /* endfor */
    if (fRealign )
    {
      pAligned->ulFillIndex -= ulUsed - ulAct;
    } /* endif */

    pAligned->ulUsed = ulAct;
  } /* endif */

#ifdef ITMTEST
//{ ULONG  ulTemp1;
//if (fSplit )
//{
//  fOut = fopen ("ITMSTAT.OUT", "a" );
//  fprintf (fOut, "After CheckUnaligned   \n");
//  for ( ulTemp1=0;ulTemp1 < (pAligned->ulUsed);ulTemp1++ )
//  {
//    fprintf( fOut,
//             "%4d Seg: %4d %4d 4%d \n",
//              ulTemp1,
//              pAligned->pulSrc[ulTemp1],
//              pAligned->pulTgt1[ulTemp1],
//              (USHORT) pAligned->pbType[ulTemp1]);
//  } /* endfor */
//  fclose(fOut);
//} /* endif */
//}
#endif
  return (fSplit);
} /* end of function CheckUnaligned(pAligned ) */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     Check2Unaligned(pAligned, pITMIda,BOOL)
//------------------------------------------------------------------------------
// Function call:     Check2Unaligned(pAligned, pITMIda, fRealign)
//------------------------------------------------------------------------------
// Description:       check whether 1:0 or 0:1 alignments are contained
//                    in other 1:1 (1:2, 2:1)alignments; if so , delete them
//------------------------------------------------------------------------------
// Parameters:        PALLALIGNED  pAligned,
//                    PITMIDA      pITMIda,
//                    BOOL         fRealign
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   if something changed
//                    FALSE  nothing changed
//------------------------------------------------------------------------------
// Function flow:     parse for duplicates in src:
//------------------------------------------------------------------------------
static VOID
Check2Unaligned
(
 PALLALIGNED  pAligned,
 PITMIDA      pITMIda,
 BOOL         fRealign
)
{
  ULONG    ulUsed;
  ULONG    ulAct;
  ULONG    ulI;
  BOOL     fCandidate;
  ULONG    ulMax;

  /********************************************************************/
  /* parse for duplicates in src                                      */
  /********************************************************************/
  if (fRealign )
  {
    ulUsed = pAligned->ulFillStart;       // realigned area
    ulMax = pAligned->ulFillIndex;
  }
  else
  {
    ulUsed = 1;                           // total alignment list
    ulMax = pAligned->ulUsed;             // 1st entry remains unchanged
  } /* endif */

  ulAct = ulUsed;
  while ( ulUsed < ulMax )
  {
    /******************************************************************/
    /* if ulUsed is an 'unaligned' alignment ...                      */
    /******************************************************************/
    fCandidate = FALSE;
    if ( (pAligned->pbType[ulUsed] == ONE_NUL) &&
         (pAligned->pbType[ulUsed+1] == NUL_ONE) )
    {
      fCandidate = TRUE;
    } /* endif */

    if ( (pAligned->pbType[ulUsed] == NUL_ONE) &&
         (pAligned->pbType[ulUsed+1] == ONE_NUL) )
    {
      fCandidate = TRUE;
    } /* endif */
    /******************************************************************/
    /* check that candidate is surrounded by aligned segments         */
    /******************************************************************/
    if ( (pAligned->pbType[ulUsed + 2] == NUL_ONE) ||
          (pAligned->pbType[ulUsed + 2] == ONE_NUL) )
    {
      fCandidate = FALSE;
    } /* endif */
    if ( ulAct > 1 )       // ulAct points to next free position
    {
      if ( (pAligned->pbType[ulAct - 1] == NUL_ONE) ||
           (pAligned->pbType[ulAct - 1] == ONE_NUL)  )
      {
        fCandidate = FALSE;
      } /* endif */
    } /* endif */
    /******************************************************************/
    /* check that candidate is not overcrossing a NOP anchor          */
    /* ( NOP anchors are not contained in Alignment list )            */
    /* only during realignment pstVisDoc->pulAnchor is filled;        */
    /******************************************************************/
    if ( fCandidate)
    {
      fCandidate = IsOvercross(pAligned, pITMIda, ulUsed );
    } /* endif */
    /******************************************************************/
    /* if fCandidate: combine the  two unaligned segments to a 1:1    */
    /* else copy alignments                                           */
    /******************************************************************/
    if ( fCandidate )
    {
      if ( pAligned->pbType[ulUsed] == ONE_NUL)
      {
         pAligned->pulTgt1[ulUsed] = pAligned->pulTgt1[ulUsed+1];
      }
      else
      {
         pAligned->pulSrc[ulUsed] = pAligned->pulSrc[ulUsed+1];
      } /* endif */
      pAligned->pbType[ulUsed] = ONE_ONE;

      AlignCopy(pAligned, ulAct, ulUsed);    //copy entry from ulUsed to ulAct
      ulAct++;
      ulUsed ++;
      ulUsed++;                     //delete next alignment
    }
    else                            // copy entry
    {
      AlignCopy(pAligned, ulAct, ulUsed);
      ulAct++;
      ulUsed ++;
    } /* endif */
  } /* endwhile */

  if (ulAct != ulUsed )
  {
    while (ulUsed < pAligned->ulUsed )
    {
      AlignCopy(pAligned, ulAct, ulUsed);
      ulAct++;
      ulUsed++;
    } /* endwhile */
    /********************************************************************/
    /* set empty entries to zero                                        */
    /********************************************************************/
    for ( ulI = ulAct;ulI <= pAligned->ulUsed;ulI++ )
    {
       pAligned->pulSrc[ulI]  = 0;
       pAligned->pulTgt1[ulI] = 0;
    } /* endfor */
    if (fRealign )
    {
      pAligned->ulFillIndex -= ulUsed - ulAct;
    } /* endif */

    pAligned->ulUsed = ulAct;
  } /* endif */


#ifdef ITMTEST
//{
// ULONG  ulTemp1;
// if (ulAct != ulUsed )
// {
//    fOut = fopen("ITMSTAT.OUT", "a" );
//    fprintf (fOut, "After Check2Unaligned   \n");
//    for ( ulTemp1=0;ulTemp1 < (pAligned->ulUsed);ulTemp1++ )
//    {
//      fprintf( fOut,
//               "%4d Seg: %4d %4d 4%d \n",
//                ulTemp1,
//                pAligned->pulSrc[ulTemp1],
//                pAligned->pulTgt1[ulTemp1],
//                (USHORT) pAligned->pbType[ulTemp1]);
//    } /* endfor */
//    fclose(fOut);
// } /* endif */
//}
#endif
  return ;
} /* end of function Check2Unaligned(pAligned ) */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     IsOvercross
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       checks whether a to-be-created 1:1 will overcross an
//                    anchor
//------------------------------------------------------------------------------
// Parameters:        PALLALIGNED
//                    PITMIDA
//                    USHORT
//------------------------------------------------------------------------------
// Returncode type:   BOOL    fCandidate
//------------------------------------------------------------------------------
// Returncodes:       TRUE     1:1 alignment is ok
//                    FALSE    1:1 alignment overcrosses an anchor
//------------------------------------------------------------------------------
// Function flow:     pick src segnum and tgt segnum of the candidates
//                    if in realign mode
//                      find previous anchor
//                      find next anchor
//                    else
//                      loop thru anchor list til the anchor prev to src segnum
//                      if found loop til next anchor
//                    endif
//                    if not found or if tgtseg outside of paragraph which
//                    contains the srcseg, the 1:1 is overcrossing an anchor
//                    thus it is not a candidate to build a 1:1
//------------------------------------------------------------------------------
static BOOL
IsOvercross
(
 PALLALIGNED  pAligned,
 PITMIDA      pITMIda,
 ULONG        ulUsed
)
{
  ULONG       ulSrcSeg = 0;
  ULONG       ulTgtSeg = 0;
  ULONG       ulSrcStart = 0;
  ULONG       ulTgtStart = 0;
  ULONG       ulSrcEnd = 0;
  ULONG       ulTgtEnd = 0;
  PITMVISDOC  pVisSrc = NULL;
  BOOL        fCandidate = TRUE;
  PULONG      pSrcAnchor = NULL;
  PULONG      pTgtAnchor= NULL;
  ULONG       ulIndex = 0;
  BOOL        fFound = FALSE;

  /********************************************************************/
  /* pick srcsegnum and tgtsegnum which are candidates for a 1:1      */
  /********************************************************************/
  if ( pAligned->pbType[ulUsed] == ONE_NUL)
  {
     ulSrcSeg = pAligned->pulSrc[ulUsed];
     ulTgtSeg = pAligned->pulTgt1[ulUsed+1];
  }
  else
  {
     ulSrcSeg = pAligned->pulSrc[ulUsed + 1];
     ulTgtSeg = pAligned->pulTgt1[ulUsed];
  } /* endif */
  /********************************************************************/
  /* find anchors surrounding the candidate                           */
  /********************************************************************/
  ulSrcStart = ulSrcSeg - 1;
  ulSrcEnd = ulSrcSeg + 1;

  pVisSrc = &(pITMIda->stVisDocSrc);

  if ( pITMIda->stVisDocSrc.pVisState )           // user changed alignment
  {
    ulSrcStart = FindNextAnchor(pVisSrc, ulSrcStart,PREVIOUS);
    ulSrcEnd = FindNextAnchor(pVisSrc, ulSrcEnd, NEXT);
    if ( !ulSrcEnd )              // if at end of file
    {
      ulSrcEnd = pVisSrc->pDoc->ulMaxSeg;
    } /* endif */

    ulTgtStart = pVisSrc->pulAnchor[ulSrcStart];
    ulTgtEnd = pVisSrc->pulAnchor[ulSrcEnd];
    fFound = TRUE;
  }
  else                                            // initial alignment
  {
    pSrcAnchor = &(pITMIda->itmSrcNop.pulSegs[0]);
    pTgtAnchor = &(pITMIda->itmTgtNop.pulSegs[0]);
    ulIndex = 1;
    fFound = FALSE;
    while ( !fFound && (ulIndex < pITMIda->itmSrcNop.ulUsed) )
    {
      if ( pSrcAnchor[ulIndex] > ulSrcStart )
      {
        ulSrcStart = pSrcAnchor[ulIndex - 1];              // previous anchor
        ulTgtStart = pTgtAnchor[ulIndex - 1];
        fFound = TRUE;
      }
      else
      {
        ulIndex ++;
      } /* endif */
    } /* endwhile */
    if ( fFound )
    {
      fFound = FALSE;
      while ( !fFound && (ulIndex < pITMIda->itmSrcNop.ulUsed) )
      {
        if ( pSrcAnchor[ulIndex] >= ulSrcEnd )
        {
          ulSrcEnd = pSrcAnchor[ulIndex];
          ulTgtEnd = pTgtAnchor[ulIndex];
          fFound = TRUE;
        } /* endif */
        ulIndex++;
      } /* endwhile */
    }
    else
    {
      fCandidate = FALSE;                    // should not happen...
    } /* endif */
  } /* endif */
  /*****************************************************************/
  /* if tgtseg is not in the paragraph in which srcseg is, its     */
  /* overcrossing an anchor, and thus no candidate!                */
  /*****************************************************************/
  if (!fFound || ( ulTgtSeg < ulTgtStart) || ( ulTgtSeg > ulTgtEnd ))
  {
    fCandidate = FALSE;            //fOverCross = TRUE;
  } /* endif */
  return (fCandidate);
} /* end of function IsOvercross */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     CheckDuplicate
//------------------------------------------------------------------------------
// Function call:     CheckDuplicate(pulSegNums, ulCurrent, ulMax)
//------------------------------------------------------------------------------
// Description:       find out whether usCurrent occurs again in given list
//                    of segnums
//------------------------------------------------------------------------------
// Parameters:        PULONG    pulSegNums
//                    ULONG     ulCurrent,
//                    USHORT    ulMax
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       0           if segnum occurs only once
//                    ulI         position where ulSegNum occurs the 2nd time
//------------------------------------------------------------------------------
// Function flow:     while not at end and not at next higher segnum and not
//                       a duplicate is found
//                       increase index to list of segnums
//------------------------------------------------------------------------------
static ULONG
CheckDuplicate
(
   PULONG    pulSegNums,
   ULONG     ulCurrent,
   ULONG     ulMax
)
{
  ULONG      ulCurSeg;
  ULONG      ulI;
  BOOL       fNext = FALSE;            // next segnum different to usCurrent
                                       // found
  BOOL       fDuplicate = FALSE;

   ulCurSeg = pulSegNums[ulCurrent];
   ulI = ulCurrent + 1;

   // if usCurSeg=0, do of course not compare with next
   if (( ulI >= ulMax ) || (!ulCurSeg) )                          /* @Pr70 */
//   if ( usI >= ulMax )
   {
     fNext = TRUE;                        // no duplicate
   } /* endif */
   while ( ulCurSeg && (ulI < ulMax )&& !fDuplicate && !fNext )
   {
     if ( pulSegNums[ulI] == ulCurSeg )
     {
       fDuplicate = TRUE;                       //stop, duplicate found
     }
     else if (pulSegNums[ulI] > ulCurSeg )
     {                                          // next seg with higher segnum
       fNext = TRUE;
     }
     else if ( ulI < ulMax - 1 )
     {
       ulI ++;                                  // go on with search
     }
     else
     {                                          // end of list
       fNext = TRUE;
     } /* endif */
   } /* endwhile */

   if ( fNext )
   {
     ulI = 0;
   } /* endif */
   return (ulI);
} /* end of function CheckDuplicate */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     CheckNops
//------------------------------------------------------------------------------
// Function call:     CheckNops(pITMIda, pDoc, pstNopCnt)
//------------------------------------------------------------------------------
// Description:        check occurrency of NOPS
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda,
//                    PTBDOCUMENT  pDoc,
//                    PITMNOPCOUNT pstNopCnt
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    everything okay
//                    FALSE   allocation error
//------------------------------------------------------------------------------
// Function flow:     build quasi random sequence ( see e.g. Wirth )
//                    while not at last segment of doc
//                      switch (status of segment)
//                        case NOP:
//                          if current NOP is found in pstNopCnt,
//                            increase occurrence
//                          else add current NOP at end of pstNopCnt
//                       endswitch
//                    endwhile
//------------------------------------------------------------------------------

static BOOL
CheckNops
(
  PITMIDA      pITMIda,                // pointer to ida
  PTBDOCUMENT  pDoc,                   // pointer to document
  PITMNOPCOUNT pstNopCnt               // pointer to nop segment array
)
{
  BOOL        fOK = TRUE;
  PTBSEGMENT  pSeg;
  ULONG       ulSegNum;
  USHORT      i;
  BOOL        fFound;
  PCHAR_W     pRest = NULL;
  USHORT      usColPos;
  PTOKENENTRY pTok;
  ULONG       ulEndSeg;
  BOOL        fCrossOut = FALSE;
  LONG        lLength;
  CHAR_W      szMorphData[MAX_TERM_LEN];
  USHORT      usNewPerc;
  USHORT      usOldPerc = 0;
  HWND        hwndTemp = pITMIda->hwnd;

  /********************************************************************/
  /* now go through the file and add and count nops                   */
  /********************************************************************/
  ulSegNum = 1;
  pSeg = EQFBGetSegW(pDoc, ulSegNum);
  while ( pSeg && fOK)
  {

    usNewPerc = (USHORT) ((LONG)ulSegNum * 10 / pDoc->ulMaxSeg);
    if ( usNewPerc != usOldPerc )              //update slider
    {
      UtlDispatch();
      fOK = ( hwndTemp && WinIsWindow( (HAB) NULL, hwndTemp ));
      if ( fOK )
      {
        usOldPerc = usNewPerc;
        WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                    MP1FROMSHORT(usOldPerc + (pITMIda->usStartSlider)), NULL );
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      switch ( pSeg->qStatus )
      {
        case  QF_NOP:
          /********************************************************************/
          /* get the first and last of the NOPs in a block                    */
          /* fill in the new values for this NOP block                        */
          /********************************************************************/

            TATagTokenizeW( pSeg->pDataW,
                           ((PLOADEDTABLE)pITMIda->pLoadedTable),
                           TRUE,
                           &pRest,
                           &usColPos,
                           (PTOKENENTRY) pDoc->pTokBuf,
                           TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
            pTok = (PTOKENENTRY) pDoc->pTokBuf;

            while ( fOK && (pTok->sTokenid != ENDOFLIST ))
            {
              if ( pTok->sTokenid >= 0 )
              {
                /************************************************************/
                /* old:if nothing was currently crossed-out, go on as usual */
                /* new: startx seg is not x, so it can be counted...      */
                /************************************************************/
                fFound = FALSE;
                i = 0;
                lLength = ITMGetRelLength(pITMIda->pLoadedTable,
                                         pTok->usLength, pTok->sAddInfo,
                                         pTok->sTokenid, szMorphData);

                while ( i < pstNopCnt->ulUsed && !fFound )
                {
                  /**********************************************************/
                  /* if Tokenid equal and only part of the tag is relevant  */
                  /*                     or                                 */
                  /* if Tokenid equal and datastring is equal               */
                  /* then both toks are recognized as equal                 */
                  /**********************************************************/
                  if ( TagsAreEqual(pstNopCnt->psTokenID[i], pTok->sTokenid,
                          pTok->sAddInfo, pstNopCnt->ppData[i],
                          pTok->pDataStringW, lLength) )
                  {
                    fFound = TRUE;
                  } /* endif */
                  i++;
                } /* endwhile */
                if ( !fFound )
                {
                  fOK = NopCntAlloc(pstNopCnt);
                  if (fOK)
                  {
                    pstNopCnt->psTokenID[i] = pTok->sTokenid;
                    pstNopCnt->psAddInfo[i] = pTok->sAddInfo;
                    pstNopCnt->ppData[i] = pTok->pDataStringW;
                    pstNopCnt->pusOccur[i] = 1;
                    pstNopCnt->ulUsed ++;                  // point to next entry
                    pstNopCnt->pusLen[i] = (USHORT)(lLength);
                  }
                }
                else
                {
                  pstNopCnt->pusOccur[i-1]++;
                } /* endif */
              } /* endif */
              pTok++;
            } /* endwhile */

            ulEndSeg = ulSegNum;
            fCrossOut = FALSE;
            pTok = (PTOKENENTRY) pDoc->pTokBuf;
            while ( !fCrossOut && ( pTok->sTokenid != ENDOFLIST ))
            {
              if ( pTok->sTokenid >= 0 )
              {
                if ( !fCrossOut && (pTok->sAddInfo & TAG_ITM_STARTX ))
                {
                  /**********************************************************/
                  /* returns end of crossed-out area                        */
                  /* careful: pTok is changed!!!                            */
                  /**********************************************************/
                  ulEndSeg = ITMCrossOut(pITMIda, pDoc, pSeg);
                  if ( ulEndSeg != ulSegNum )
                  {
                    fCrossOut = TRUE;
                  } /* endif */
                } /* endif */
              } /* endif */
              if ( !fCrossOut )
              {
                pTok++;
              } /* endif */
            } /* endwhile */
            /**********************************************************/
            /* if segments has been crossed out, goon after end of    */
            /* crossed-out area                                       */
            /**********************************************************/
            ulSegNum = ulEndSeg;
          break;
        default :
          break;
      } /* endswitch */
      ulSegNum++;                         // point to next segment
      pSeg = EQFBGetSegW(pDoc, ulSegNum);
    } /* endif */
  } /* endwhile */
  pITMIda->usStartSlider += 10;        // checknops is 10% of aligning
  return( fOK );
} /* end of function CheckNops    */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     NopCntAlloc
//------------------------------------------------------------------------------
// Function call:     NopCntAlloc(pstNopCnt)
//------------------------------------------------------------------------------
// Description:       allocate structure for Nop count   array
//------------------------------------------------------------------------------
// Parameters:        PITMNOPCOUNT pstNopCnt       // ptr to nop segment array
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE    alloc is ok
//                    FALSE   could not alloc
//------------------------------------------------------------------------------
// Function flow:     if structure is filled
//                      increase by NEW_ALIGN many entries
//                      realloc Occurrence structure
//                      if ok realloc ppData structure
//                      if ok realloc pulHash structure
//------------------------------------------------------------------------------
BOOL
NopCntAlloc
(
  PITMNOPCOUNT pstNopCnt               // pointer to nop segment array
)
{
  BOOL     fOK = TRUE;
  LONG     lOldLen;
  LONG     lNewLen;


   if ( pstNopCnt->ulAlloc == pstNopCnt->ulUsed )
   {
     pstNopCnt->ulAlloc += NEW_ALIGN;
     /******************************************************************/
     /* get old and new length for reallocation...                     */
     /******************************************************************/
     lOldLen =  (LONG) pstNopCnt->ulUsed * sizeof( USHORT );
     lNewLen =  (LONG) pstNopCnt->ulAlloc * sizeof( USHORT );

     fOK = UtlAlloc( (PVOID *) &(pstNopCnt->pusOccur), lOldLen,
                       lNewLen, ERROR_STORAGE );
     if ( fOK  )
     {
       lOldLen =  (LONG) pstNopCnt->ulUsed * sizeof( PSZ );
       lNewLen =  (LONG) pstNopCnt->ulAlloc * sizeof( PSZ );
       fOK = UtlAlloc( (PVOID *) (PVOID *)&(pstNopCnt->ppData),
                       lOldLen, lNewLen, ERROR_STORAGE );
     } /* endif */
     if ( fOK  )
     {
       lOldLen =  (LONG) pstNopCnt->ulUsed * sizeof( SHORT );
       lNewLen =  (LONG) pstNopCnt->ulAlloc * sizeof( SHORT );
       fOK = UtlAlloc( (PVOID *) &(pstNopCnt->psTokenID),
                       lOldLen, lNewLen, ERROR_STORAGE );
     } /* endif */
     if ( fOK )
     {
       lOldLen =  (LONG) pstNopCnt->ulUsed * sizeof( SHORT );
       lNewLen =  (LONG) pstNopCnt->ulAlloc * sizeof( SHORT );
       fOK = UtlAlloc( (PVOID *) &(pstNopCnt->psAddInfo),
                       lOldLen, lNewLen, ERROR_STORAGE );
     } /* endif */

     if ( fOK )
     {
       lOldLen =  (LONG) pstNopCnt->ulUsed * sizeof( USHORT );
       lNewLen =  (LONG) pstNopCnt->ulAlloc * sizeof( USHORT );
       fOK = UtlAlloc( (PVOID *) &(pstNopCnt->pusLen), lOldLen,
                         lNewLen, ERROR_STORAGE );
     } /* endif */
   } /* endif */
   return (fOK);
} /* end of function NopCntAlloc */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     BuildJoinSeg
//------------------------------------------------------------------------------
// Function call:     BuildJoinSeg(pITMIda)
//------------------------------------------------------------------------------
// Description:       build joined segments for 2:1 and 1:2 matches
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     while not thru alignment struct
//                      pick current and next alignment
//                      if current and next src equal
//                        if both segments are not joined or split by the user
//                          join current and next tgt segment
//                          adjust alignment structure
//                        else
//                          split up 1:0 alignment in the alignment struct
//                      else if current and next tgt equal
//                        if both segments are not joined or split by the user
//                           join src segments
//                           build 2:1 alignmnet in alignment structure
//                        else split up 0:1 alignment
//                      endif
//                    endwhile
//------------------------------------------------------------------------------

VOID
BuildJoinSeg
(
  PITMIDA      pITMIda
)
{
  PALLALIGNED pAligned;
  ULONG       ulUsed;
  ULONG       ulAct;
  PTBDOCUMENT pSrcDoc;
  PTBDOCUMENT pTgtDoc;
  BOOL        fQuiet;
  BOOL        fOK;
  ULONG       ulSrcSeg;
  ULONG       ulSrcSegNext;
  ULONG       ulTgtSeg;
  ULONG       ulTgtSegNext;
  BOOL        fRealign;                                    //FALSE:1stalign, true: realignment
  ULONG       ulEnd;

  fQuiet = TRUE;
  pAligned = &(pITMIda->Aligned);
  pSrcDoc = &(pITMIda->TBSourceDoc);
  pTgtDoc = &(pITMIda->TBTargetDoc);
  if ( pITMIda->stVisDocSrc.pVisState )
  {
    fRealign = TRUE;
    ulUsed = pAligned->ulFillStart;
    ulAct = ulUsed;
    ulEnd = pAligned->ulFillIndex;
  }
  else
  {
    fRealign = FALSE;
    ulUsed = 0;
    ulAct = 0;
    ulEnd = pAligned->ulUsed;
  } /* endif */
  while ( ulUsed < ulEnd )
  {
    fOK = TRUE;
    ulSrcSeg = pAligned->pulSrc[ulUsed];
    ulSrcSegNext = pAligned->pulSrc[ulUsed + 1];
    ulTgtSeg = pAligned->pulTgt1[ulUsed];
    ulTgtSegNext = pAligned->pulTgt1[ulUsed + 1];
    if ( ulSrcSeg && ulTgtSeg )
    {
      if ( ulSrcSeg && (ulSrcSeg == ulSrcSegNext))
      {
        /****************************************************************/
        /* if ONE_TWO contains userjoined or usersplit segment,         */
        /* split up ( set fOK = FALSE to avoid joining )                */
        /****************************************************************/
        if ( fRealign )
        {
          if ( pITMIda->stVisDocSrc.pVisState[ulSrcSeg].UserJoin )
          {
            fOK = FALSE;
          } /* endif */
          if ( pITMIda->stVisDocTgt.pVisState[ulTgtSeg].UserJoin
               || pITMIda->stVisDocTgt.pVisState[ulTgtSeg].UserSplit )
          {
            fOK = FALSE;
          } /* endif */
          if ( pITMIda->stVisDocTgt.pVisState[ulTgtSegNext].UserJoin )
          {
            fOK = FALSE;
          } /* endif */
        } /* endif */
        /****************************************************************/
        /* delete joined entry( entry ulUsed + 1) in alignment struct   */
        /* 2 tgt segs, both src segs equal --> join tgts                */
        /****************************************************************/
#ifndef EU
        if ( fOK )
        {
          fOK = VisJoinSeg(pITMIda,&(pITMIda->stVisDocTgt),
                            pTgtDoc, ulTgtSeg, fQuiet);
        } /* endif */
#else
      {
        ULONG   ulTempSeg;
        ulTempSeg = ulTgtSeg + 1;
        EQFBGetVisSeg(pTgtDoc, &ulTempSeg);
        while (fOK && (ulTempSeg <= ulTgtSegNext) )
        {
          fOK = VisJoinSeg(pITMIda,&(pITMIda->stVisDocTgt),
                            pTgtDoc, ulTgtSeg, fQuiet);
          ulTempSeg = ulTgtSeg + 1;
          EQFBGetVisSeg(pTgtDoc, &ulTempSeg);
        } /* endwhile */
#ifdef ITMTEST
  fOut      = fopen ( "ITMSTAT.OUT", "a" );
  fprintf( fOut,"join tgt segs %4d %4d\n", ulTgtSeg, ulTgtSegNext );
  fclose( fOut      );
  fOut = NULL;
#endif
      }
#endif
        if ( fOK )
        {
          AlignCopy(pAligned, ulAct, ulUsed);
          pAligned->pbType[ulAct] = ONE_TWO;
          ulAct++;
          ulUsed += 2;
        }
        else
        {
          AlignCopy(pAligned, ulAct, ulUsed);
          ulAct++;
          ulUsed ++;
          AlignCopy(pAligned, ulAct, ulUsed);
          /**************************************************************/
          /* no join possible:split up into 1:1 and 1:0 alignments      */
          /* ulAct - 1 points to 1st alignment                          */
          /* both alignment entries remain                              */
          /**************************************************************/
          AlignSplit(pITMIda, pAligned, pSrcDoc, pTgtDoc,
                     (ulAct - 1), NUL_ONE, fRealign);
          ulAct++;
          ulUsed++;
        } /* endif */
      }
      else if (ulTgtSeg && (ulTgtSeg == ulTgtSegNext) )
      {
        /****************************************************************/
        /* if TWO_ONE contains userjoined segment, split up             */
        /* ( set fOK = FALSE to avoid joining )                         */
        /****************************************************************/
        if ( fRealign )
        {
          if ( pITMIda->stVisDocSrc.pVisState[ulSrcSeg].UserJoin
              || pITMIda->stVisDocSrc.pVisState[ulSrcSeg].UserSplit )
          {
            fOK = FALSE;
          } /* endif */
          if ( pITMIda->stVisDocSrc.pVisState[ulSrcSegNext].UserJoin )
          {
            fOK = FALSE;
          } /* endif */
          if ( pITMIda->stVisDocTgt.pVisState[ulTgtSeg].UserJoin )
          {
            fOK = FALSE;
          } /* endif */
        } /* endif */
        /****************************************************************/
        /* 2 src segs, both tgt segs equal-->join sources               */
        /****************************************************************/
#ifndef EU
        if ( fOK )
        {
          fOK = VisJoinSeg(pITMIda,&(pITMIda->stVisDocSrc),
                           pSrcDoc, ulSrcSeg, fQuiet);
        } /* endif */
#else
      {
        ULONG   ulTempSeg;
        ulTempSeg = ulSrcSeg + 1;
        EQFBGetVisSeg(pSrcDoc, &ulTempSeg);
        while (fOK && (ulTempSeg <= ulSrcSegNext) )
        {
          fOK = VisJoinSeg(pITMIda,&(pITMIda->stVisDocSrc),
                            pSrcDoc, ulSrcSeg, fQuiet);
          ulTempSeg = ulSrcSeg + 1;
          EQFBGetVisSeg(pSrcDoc, &ulTempSeg);
        } /* endwhile */
#ifdef ITMTEST
//fOut      = fopen ( "ITMSTAT.OUT", "a" );
//fprintf( fOut,"join src segs %4d %4d\n", ulSrcSeg, ulSrcSegNext );
//fclose( fOut      );
//fOut = NULL;
#endif
      }
#endif
        if ( fOK )
        {
          AlignCopy(pAligned, ulAct, ulUsed);
          pAligned->pbType[ulAct] = TWO_ONE;
          ulAct++;
          ulUsed += 2;
        }
        else
        {
          AlignCopy(pAligned, ulAct, ulUsed);
          ulAct++;
          ulUsed ++;
          AlignCopy(pAligned, ulAct, ulUsed);
          /**************************************************************/
          /* no join possible:split up into 1:1 and 1:0 alignments      */
          /* ulAct - 1 points to 1st alignment                          */
          /**************************************************************/
          AlignSplit(pITMIda,pAligned, pSrcDoc, pTgtDoc,
                     ( ulAct - 1), ONE_NUL, fRealign);
          ulAct++;
          ulUsed++;
        } /* endif */
      }
      else
      {
        AlignCopy(pAligned, ulAct, ulUsed);
        ulAct++;
        ulUsed ++;
      } /* endif */
    }
    else
    {
      AlignCopy(pAligned, ulAct, ulUsed);    //srcseg/tgtseg = 0
      ulAct++;
      ulUsed ++;
    } /* endif */
  } /* endwhile */

/**********************************************************************/
/* move rest of Alignment list; nec if during realign not total       */
/* list has been parsed                                               */
/**********************************************************************/
  while (ulUsed < pAligned->ulUsed )
  {
    AlignCopy(pAligned, ulAct, ulUsed);          //srcseg/tgtseg = 0
    ulAct++;
    ulUsed++;
  } /* endwhile */

  pAligned->ulFillIndex -= ulUsed - ulAct;  //update it, may be smaller now
  pAligned->ulUsed -= ulUsed - ulAct;            //update end of list

  if ( !fRealign )
  {
    pITMIda->usStartSlider ++;
    WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                MP1FROMSHORT(pITMIda->usStartSlider), NULL );
  } /* endif */

#ifdef ITMTEST
  {
  ULONG       ulTemp1;
  fOut      = fopen ( "ITMSTAT.OUT", "a" );
  fprintf (fOut, "With Joined Segs \n");
  ulNumOf21 = 0;
  ulNumOf10 = 0;
  ulNumNot11 = 0;
    for ( ulTemp1=1;ulTemp1 < (pITMIda->Aligned.ulUsed);ulTemp1++ )
    {
      if (pITMIda->Aligned.pbType[ulTemp1] != (BYTE)ONE_ONE)
      {
        fprintf( fOut,
               "%4d Seg: %4d %4d 4%d \n",
                ulNumNot11,
                pITMIda->Aligned.pulSrc[ulTemp1],
                pITMIda->Aligned.pulTgt1[ulTemp1],
                (USHORT) pITMIda->Aligned.pbType[ulTemp1]);
        ulNumNot11 ++;
        if (pITMIda->Aligned.pbType[ulTemp1] == (BYTE)TWO_ONE ||
            pITMIda->Aligned.pbType[ulTemp1] == (BYTE)ONE_TWO )
        {
          ulNumOf21 ++;
        }
        else
        {
          ulNumOf10 ++;
        }
      }
    } /* endfor */
    fprintf(fOut, "old Penalty, 1-0, 2:1, Not1:1, %5d %5d %5d\n" ,
            ulNumOf10,ulNumOf21, ulNumNot11 );
    fclose(fOut);
  }
#endif
} /* end of function BuildJoinSeg(pITMIda) */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     AlignCopy
//------------------------------------------------------------------------------
// Function call:     AlignCopy(pAligned, usDest, usFrom)
//------------------------------------------------------------------------------
// Description:       copy entry in alignment struct from 'usFrom' to 'usDest'
//------------------------------------------------------------------------------
// Parameters:        PALLALIGNED  pAligned
//                    USHORT       usDest,
//                    USHORT       usFrom
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     copy alignment entry
//------------------------------------------------------------------------------
static VOID
AlignCopy
(
  PALLALIGNED  pAligned,
  ULONG        ulDest,
  ULONG        ulFrom
)
{
    pAligned->pbType[ulDest] = pAligned->pbType[ulFrom];
    pAligned->pulSrc[ulDest] = pAligned->pulSrc[ulFrom];
    pAligned->pulTgt1[ulDest] = pAligned->pulTgt1[ulFrom];
    pAligned->psDist[ulDest] = pAligned->psDist[ulFrom];

} /* end of function AlignCopy */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     AlignSplit
//------------------------------------------------------------------------------
// Function call:     AlignSplit(pALigned, pSrcDoc, pTgtDoc, usIndex, usType)
//------------------------------------------------------------------------------
// Description:       split 2:1 in 1:1 and 1:0, or 1:2 in 1:1 and 0:1
//                    decide which one to split up relying on the segm. length
//------------------------------------------------------------------------------
// Parameters:        PALLALIGNED pAligned,
//                    PTBDOCUMENT pSrcDoc,
//                    PTBDOCUMENT pTgtDoc,
//                    ULONG       ulIndex,      // index of 1st alignment
//                    USHORT      usType        // type of 2nd alignment
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
//                      split up 2nd alignment( ->1:0 or 0:1 )
//                    else split up 1st alignment ( -> 1:0 or 0:1 )
//                    adjust type of the changed alignment
//------------------------------------------------------------------------------
VOID
AlignSplit
(
  PITMIDA     pITMIda,
  PALLALIGNED pAligned,
  PTBDOCUMENT pSrcDoc,
  PTBDOCUMENT pTgtDoc,
  ULONG       ulIndex,         // index of 1st alignment
  USHORT      usType,                  // type of 2nd alignment
  BOOL        fRealign
)
{
  PTBSEGMENT  pSeg1;
  PTBSEGMENT  pSeg2;
  PTBSEGMENT  pSeg3;
  ULONG       ulSeg1;
  ULONG       ulSeg2;
  ULONG       ulSeg3;
  BOOL        fFirstIsAnchor = FALSE;
  BOOL        fSecondIsAnchor = FALSE;

  if ( usType == NUL_ONE )
  {
    ulSeg1 = pAligned->pulTgt1[ulIndex];
    ulSeg2 = pAligned->pulTgt1[ulIndex+1];
    ulSeg3 = pAligned->pulSrc[ulIndex];
    pSeg1 = EQFBGetSegW(pTgtDoc, ulSeg1);
    pSeg2 = EQFBGetSegW(pTgtDoc, ulSeg2);
    pSeg3 = EQFBGetSegW(pSrcDoc, ulSeg3);
  }
  else
  {
    ulSeg1 = pAligned->pulSrc[ulIndex];
    ulSeg2 = pAligned->pulSrc[ulIndex+1];
    ulSeg3 = pAligned->pulTgt1[ulIndex];
    pSeg1 = EQFBGetSegW(pSrcDoc, ulSeg1);
    pSeg2 = EQFBGetSegW(pSrcDoc, ulSeg2);
    pSeg3 = EQFBGetSegW(pTgtDoc, ulSeg3);
  } /* endif */

/**********************************************************************/
/* fFirstIsAnchor is only relevant if the length differences are      */
/* equal                                                              */
/**********************************************************************/
//if ( abs (pSeg1->usLength - pSeg3->usLength) ==
//     abs (pSeg2->usLength - pSeg3->usLength) )
//{
    if (usType == NUL_ONE )
    {
      fFirstIsAnchor = ITMIsAnchor(pITMIda, ulSeg3, ulSeg1, fRealign);
      fSecondIsAnchor = ITMIsAnchor(pITMIda, ulSeg3, ulSeg2, fRealign);
    }
    else
    {
      fFirstIsAnchor = ITMIsAnchor(pITMIda, ulSeg1, ulSeg3, fRealign);
      fSecondIsAnchor = ITMIsAnchor(pITMIda, ulSeg2, ulSeg3, fRealign);
    } /* endif */

//} /* endif */

//if ( fFirstIsAnchor || ( abs (pSeg1->usLength - pSeg3->usLength) <
//                         abs (pSeg2->usLength - pSeg3->usLength) ) )
  if (pSeg1 && pSeg2 && pSeg3 &&
      ( abs (pSeg1->usLength - pSeg3->usLength) <
                           abs (pSeg2->usLength - pSeg3->usLength) ) )
  {
    if (!fFirstIsAnchor && fSecondIsAnchor)
    {
#ifdef ITMTEST
  fOut      = fopen ( "ITMSTAT.OUT", "a" );
  fprintf( fOut,"1st not anchor,2nd is anchor, but length fits for 1st  %4d %4d %4d\n",
                 pSeg1->ulSegNum, pSeg2->ulSegNum, pSeg3->ulSegNum  );
  fclose( fOut      );
  fOut = NULL;
#endif
    } /* endif */
  } /* endif */

  if (fFirstIsAnchor )
  {
    /***********************************************************/
    /* pSeg1 is the 1:1, pSeg2 is 0:1 or 1:0 alignment         */
    /* 2nd alignment is changed to 0:1 or 1:0                  */
    /***********************************************************/
    if ( usType == NUL_ONE )
    {
      pAligned->pulSrc[ulIndex + 1] = 0;
    }
    else
    {
      pAligned->pulTgt1[ulIndex + 1] = 0;
    } /* endif */
    pAligned->pbType[ulIndex + 1] = (BYTE)usType;
  }
  else
  {
    /***********************************************************/
    /* 1st alignment is changed to 0:1 or 1:0                  */
    /* pSeg2 is the 1:1, pSeg1 is 0:1 alignment                */
    /***********************************************************/

    if ( usType == NUL_ONE )
    {
      pAligned->pulSrc[ulIndex] = 0;
    }
    else
    {
      pAligned->pulTgt1[ulIndex] = 0;
    } /* endif */
    pAligned->pbType[ulIndex] = (BYTE)usType;
  } /* endif */
} /* end of function AlignSplit */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     AlignLastSplit
//------------------------------------------------------------------------------
// Function call:     AlignLastSplit(pAligned, usIndex, usType)
//------------------------------------------------------------------------------
// Description:       split 2:1 in 1:1 and 1:0, or 1:2 in 1:1 and 0:1
//------------------------------------------------------------------------------
// Parameters:        PALLALIGNED pAligned,
//                    USHORT      usIndex,
//                    USHORT      usType
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     if 2:1 , change 2nd alignment to 1:0
//                    if 1:2, change 2nd alignment to 0:1
//------------------------------------------------------------------------------
static VOID
AlignLastSplit
(
  PALLALIGNED pAligned,
  ULONG       ulIndex,         // index of 1st alignment
  USHORT      usType
)
{
  /***********************************************************/
  /* pSeg1 is the 1:1, pSeg2 is 0:1 or 1:0 alignment         */
  /* 2nd alignment is changed to 0:1 or 1:0                  */
  /***********************************************************/
  if ( usType == NUL_ONE )
  {
    pAligned->pulSrc[ulIndex + 1] = 0;
  }
  else
  {
    pAligned->pulTgt1[ulIndex + 1] = 0;
  } /* endif */
  pAligned->pbType[ulIndex + 1] = (BYTE)usType;
} /* end of function AlignLastSplit */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     BuildSplitSeg
//------------------------------------------------------------------------------
// Function call:     BuildSplitSeg(pITMIda, &ulFillStart, &ulFillEnd)
//------------------------------------------------------------------------------
// Description:       build Split  segments for 2:1 and 1:2 matches
//                    in specified range of alignment structure
//------------------------------------------------------------------------------
// Parameters:        PITMIDA      pITMIda,
//                    PUSHORT      pulFillStart
//                    PUSHORT      pulFillEnd
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     start at begin of invalidated region in alignment struct
//                    while not at end of inval region
//                      if srcsegment is joined and not manually joined by user
//                        split joined segment
//                        adjust alignment structure
//                      if tgtsegment is joined and not manually joined by user
//                        split joined segment
//                        adjust alignment structure
//                      adjust end of invalidated region(pulFillEnd)
//                    endwhile
//------------------------------------------------------------------------------
VOID
BuildSplitSeg
(
  PITMIDA      pITMIda,
  PULONG       pulFillStart,                 //start of inval.indices
  PULONG       pulFillEnd                    //end of inval indices
)
{
  PALLALIGNED pAligned;
  ULONG       ulIndex;
  PTBDOCUMENT pSrcDoc;
  PTBDOCUMENT pTgtDoc;
  PITMVISDOC  pVisSrc;
  PITMVISDOC  pVisTgt;
  BOOL        fQuiet;
  BOOL        fOK;
  ULONG       ulCurFillStart;
  ULONG       ulCurFillEnd;
  ULONG       ulSrcSeg;
  ULONG       ulTgtSeg;

  fQuiet = TRUE;
  /********************************************************************/
  /* split only in invalidated region                                 */
  /********************************************************************/
  /********************************************************************/
  /* invalidated indices in alignment structure must be adjusted      */
  /* during splitting in alignment structure                          */
  /********************************************************************/
  ulCurFillStart = *pulFillStart;
  ulCurFillEnd = *pulFillEnd;

  pAligned = &(pITMIda->Aligned);
  ulIndex = ulCurFillStart;
  pSrcDoc = &(pITMIda->TBSourceDoc);
  pTgtDoc = &(pITMIda->TBTargetDoc);
  pVisSrc = &(pITMIda->stVisDocSrc);
  pVisTgt = &(pITMIda->stVisDocTgt);
  while ( ulIndex <= ulCurFillEnd )
  {
    ulSrcSeg = pAligned->pulSrc[ulIndex];
    ulTgtSeg = pAligned->pulTgt1[ulIndex];
    if ( (pAligned->pbType[ulIndex] == (BYTE) TWO_ONE) &&
         !pVisSrc->pVisState[ulSrcSeg].UserJoin )
    {
      /****************************************************************/
      /* split  joined source segments                                */
      /****************************************************************/
      fOK = VisSplitSeg(pITMIda, pVisSrc, pSrcDoc, ulSrcSeg, fQuiet);
      if ( fOK )
      {
        AlignInsert(pAligned, ulIndex);
        ulCurFillEnd++;
        ulIndex ++;
        pAligned->pulSrc[ulIndex] = ulSrcSeg + 1;
      } /* endif */
    } /* endif */
    if ( pAligned->pbType[ulIndex] == (BYTE) ONE_TWO &&
          !pVisTgt->pVisState[ulTgtSeg].UserJoin  )
    {
      /****************************************************************/
      /* split joined tgt segments                                    */
      /****************************************************************/
      fOK = VisSplitSeg(pITMIda, pVisTgt, pTgtDoc, ulTgtSeg, fQuiet);
      if ( fOK )
      {
        AlignInsert(pAligned, ulIndex);
        ulCurFillEnd++;
        ulIndex++;
        pAligned->pulTgt1[ulIndex] = ulTgtSeg + 1;
      } /* endif */
    } /* endif */
    ulIndex ++;
  } /* endwhile */

  *pulFillEnd   = ulCurFillEnd;

} /* end of function BuildSplitSeg */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     AlignInsert
//------------------------------------------------------------------------------
// Function call:     AlignInsert(pAligned, ulUsed)
//------------------------------------------------------------------------------
// Description:       make room for a further alignment in alignment struct
//------------------------------------------------------------------------------
// Parameters:        PALLALIGNED  pAligned,
//                    USHORT       ulUsed
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     move contents of alignment struct to the right by
//                    one position, starting at position ulUsed
//------------------------------------------------------------------------------

VOID
AlignInsert
(
 PALLALIGNED  pAligned,
 ULONG        ulUsed
)
{
  ULONG     ulI;
  /********************************************************************/
  /* enough space should be allocated !!                              */
  /********************************************************************/
  /********************************************************************/
  /* insert one position at ulUsed + 1                                */
  /********************************************************************/
  pAligned->pbType[ulUsed] = ONE_ONE;

  for ( ulI = pAligned->ulUsed; ulI >= ulUsed; ulI-- )
  {
    pAligned->pulSrc[ulI+1]  = pAligned->pulSrc[ulI];
    pAligned->pulTgt1[ulI+1] = pAligned->pulTgt1[ulI];
    pAligned->psDist[ulI+1]  = pAligned->psDist[ulI];
    pAligned->pbType[ulI+1] = pAligned->pbType[ulI];
  } /* endfor */
  pAligned->ulUsed ++;

} /* end of function AlignInsert */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ITMFuncMemSave
//------------------------------------------------------------------------------
// Function call:     ITMFuncMemSave(pITMIda, fMsg )
//------------------------------------------------------------------------------
// Description:       save all alignments to the translation memory
//------------------------------------------------------------------------------
// Parameters:        PITMIDA pITMIda
//                    BOOL    fMsg     display success message
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   success
//                    FALSE  error
//------------------------------------------------------------------------------
// Function flow:     loop thru alignment structure
//                      if alignment type is 1:1, 1:2, 2:1
//                        get src and tgt data
//                        add entry to translation memory
//                    endof loop
//                    if Visual add overcross anchors to translation memory
//                    if filepair was prepared, delete continuation file
//                    delete filepair in filepairlist of propertyfile
//                    free allocated areas
//------------------------------------------------------------------------------
BOOL
ITMFuncMemSave
(
 PITMIDA  pITMIda,
 BOOL     fMsg
)
{
   PALLALIGNED  pAligned;
   ULONG        ulSegNum;
   PTBDOCUMENT  pTBDoc;
   USHORT       usType;
   BOOL         fAligned;
   PITMVISDOC   pVisSrc;
   ULONG        ulIndex;
   BOOL         fOK= TRUE;
   USHORT       usNumPrepared;
   CHAR         chTextBuf[40];
   CHAR         chText[80];
   CHAR         chFBuf[200]={0};
   HWND         hwndTemp = pITMIda->hwnd;
   BOOL         fMFlag= TRUE;
   HMODULE      hResMod;

   hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

   /*******************************************************************/
   /* set hour glass                                                  */
   /*******************************************************************/
   SETCURSOR( SPTR_WAIT );
   SETCAPTURE( pITMIda->hwnd );

   LOADSTRING( NULLHANDLE, hResMod, IDS_ITM_UPDATETM, chTextBuf);
   WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
               MP1FROMSHORT(-1), chTextBuf );
   LOADSTRING( NULLHANDLE, hResMod, IDS_ITM_TM, chTextBuf);
   Utlstrnccpy( chFBuf, pITMIda->chTransMemFname,sizeof(chFBuf)/sizeof(chFBuf[0])-1 ,DOT );
   sprintf( chText, chTextBuf, chFBuf );
   WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
               MP1FROMSHORT(-2), chText );
   WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
               MP1FROMSHORT(0), NULL );
   pITMIda->usStartSlider = 0;


   pTBDoc = &(pITMIda->TBSourceDoc);

   pAligned = &(pITMIda->Aligned);
   for ( ulIndex = 1; (ulIndex < pAligned->ulUsed) && fOK ; ulIndex++ )
   {

     usType = pAligned->pbType[ulIndex];
     if ( (usType == ONE_ONE) || (usType == ONE_TWO) || (usType == TWO_ONE))
     {
       fAligned = PrepareVisSeg (pITMIda, pAligned->pulSrc[ulIndex],
                                 pAligned->pulTgt1[ulIndex], usType );
       if ( fAligned )
       {
         fMFlag = ITMCheckQuality( pITMIda, pAligned, ulIndex );
         fOK = ITMTmReplace( pITMIda, pITMIda->pInFile,
                             pAligned->pulSrc[ulIndex],
                             pITMIda->szSourceSeg, pITMIda->szTargetSeg,
                             fMFlag );
       } /* endif */
     } /* endif */
     if ( fOK )
     {
      WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
                  MP1FROMSHORT( (SHORT)(((LONG)(ulIndex+1) * 100L) / pAligned->ulUsed) ),
                  NULL );
      UtlDispatch();
      fOK = ((hwndTemp == HWND_FUNCIF) || ( hwndTemp && WinIsWindow( (HAB) NULL, hwndTemp )));
     }
   } /* endfor */
   /*************************************************************/
   /* free space used to check quality                          */
   /*************************************************************/
   UtlAlloc ( (PVOID *)&pITMIda->stTgtNopCnt.pusOccur, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stTgtNopCnt.psTokenID, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stTgtNopCnt.psAddInfo, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stTgtNopCnt.ppData, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stTgtNopCnt.pusLen, 0L, 0L, NOMSG);

   UtlAlloc ( (PVOID *)&pITMIda->stSrcNopCnt.pusOccur, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stSrcNopCnt.psTokenID, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stSrcNopCnt.psAddInfo, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stSrcNopCnt.ppData, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stSrcNopCnt.pusLen, 0L, 0L, NOMSG);

   pITMIda->stSrcNopCnt.ulUsed = 0;
   pITMIda->stTgtNopCnt.ulUsed = 0;
   pITMIda->stSrcNopCnt.ulAlloc = 0;
   pITMIda->stTgtNopCnt.ulAlloc = 0;
   /*******************************************************************/
   /* add overcross anchors separately                                */
   /*******************************************************************/
   if ( fOK && pITMIda->fVisual )
   {
     pVisSrc = &(pITMIda->stVisDocSrc);
     fMFlag = TRUE;                  // all overcross get m-flag in memory!
     for ( ulSegNum = 1 ;(ulSegNum < pTBDoc->ulMaxSeg) && fOK ;ulSegNum++ )
     {
       if ( pVisSrc->pVisState[ulSegNum].OverCross  )
       {
         usType = ONE_ONE;
         fAligned = PrepareVisSeg (pITMIda, ulSegNum,
                                   pVisSrc->pulAnchor[ulSegNum], usType );
         if ( fAligned )
         {
           fOK = ITMTmReplace( pITMIda, pITMIda->pInFile,
                               ulSegNum,
                               pITMIda->szSourceSeg, pITMIda->szTargetSeg,
                               fMFlag);
         } /* endif */
       } /* endif */
       UtlDispatch();
       fOK = ( hwndTemp && WinIsWindow( (HAB) NULL, hwndTemp ));
     } /* endfor */
   } /* endif */
   /*******************************************************************/
   /* display message if requested...                                 */
   /*******************************************************************/
   if ( fOK && fMsg )
   {
     /****************************************************/
     /* display completion message                       */
     /****************************************************/
     PSZ    pData[3];                    // pointer to completion data strings
     pData[0] = ltoa (pITMIda->ulSegTotal,
                      pITMIda->szBuffer, 10 );
     pData[1] = ltoa (pITMIda->ulSegAligned,
                      pITMIda->szBuffer+MAX_SEGMENT_SIZE, 10 );
     WinShowWindow( pITMIda->hProcWnd, FALSE );  // hide process window ...
     ITMUtlError( pITMIda, ITM_COMPLETE, MB_OK, 2, &pData[0], EQF_INFO );
     pITMIda->ulSegTotal = pITMIda->ulSegAligned = 0L;
   } /* endif */

   /*******************************************************************/
   /* delete alifile/propfile entry ; delete filepair in list of      */
   /* filepairs in propertyfile                                       */
   /*******************************************************************/
   if ( (hwndTemp == HWND_FUNCIF) || (hwndTemp && WinIsWindow( (HAB) NULL, hwndTemp )))     // Test MK Non-DDE
   {
     usNumPrepared = pITMIda->usNumPrepared;
     EQFITMDelAli(pITMIda);               //delete alifile/Num in propertyfile
     EQFITMDelFilePairList(pITMIda, usNumPrepared);
     /********************************************************************/
     /* free all spaces                                                  */
     /********************************************************************/
     FreeAll (pITMIda);
     pITMIda->stVisDocSrc.fChanged = FALSE;
     pITMIda->stVisDocTgt.fChanged = FALSE;
   } /* endif */

   RELEASECAPTURE;
   SETCURSOR( SPTR_ARROW );

   return (fOK);
} /* end of function ITMFuncMemSave */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PrepareVisSeg
//------------------------------------------------------------------------------
// Function call:     PrepareVisSeg(pITMIda, ulSrcSeg, ulTgtSeg, usType)
//------------------------------------------------------------------------------
// Description:       get src and tgt segment data into buffer for writing to
//                    memory
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda,
//                    ULONG    ulSrcSeg,       // src segment numger
//                    ULONG    ulTgtSeg,       // tgt segment number
//                    USHORT   usType          //type of alignment(1:1 ,..
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE if both segs are visible
//                    FALSE else
//------------------------------------------------------------------------------
// Function flow:     increase counter of number of segs aligned by 1 or 2
//                    copy segment data into ITMIda buffer provided
//------------------------------------------------------------------------------
static BOOL
PrepareVisSeg
(
  PITMIDA  pITMIda,
  ULONG    ulSrcSeg,
  ULONG    ulTgtSeg,
  USHORT   usType
)
{
    PTBSEGMENT pSrcSeg;
    PTBSEGMENT pTgtSeg;
    BOOL   fAligned = TRUE;            // segment aligned ?

    usType = usType;
    memset( pITMIda->szTargetSeg, 0, MAX_SEGMENT_SIZE * sizeof(CHAR_W));
    memset( pITMIda->szSourceSeg, 0, MAX_SEGMENT_SIZE * sizeof(CHAR_W));

    pSrcSeg = EQFBGetVisSeg(&pITMIda->TBSourceDoc, &(ulSrcSeg) );

    pTgtSeg = EQFBGetVisSeg(&pITMIda->TBTargetDoc, &(ulTgtSeg) );

    if ( pSrcSeg && pTgtSeg )
    {
      if ( (usType == ONE_ONE ) || (usType == ONE_TWO) )
      {
        pITMIda->ulSegAligned ++;         // one more segment aligned...
      }
      else
      {
        pITMIda->ulSegAligned += 2;         // two more srcsegs aligned...
      } /* endif */

      UTF16strcpy( pITMIda->szSourceSeg, pSrcSeg->pDataW );
      UTF16strcpy( pITMIda->szTargetSeg, pTgtSeg->pDataW );
    }
    else
    {
      fAligned = FALSE;
    } /* endif */
    return fAligned;
} /* end of function PrepareVisSeg */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     NopUsage
//------------------------------------------------------------------------------
// Function call:     NopUsage(pITMIda)
//------------------------------------------------------------------------------
// Description:       decide whether the NOP with the highest occurrence
//                    should be used or not
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     fill SrcNopCnt ( cnt all different nops )
//                    fill TgtNopCnt
//                    if ok
//                       add those nops to exclusion Nop list which do not
//                       occur in other document (ExclSingleNop)
//                    count how many nops are left overall in src and tgt
//                    while src contains more than MAXNOPALLOWED nops
//                        find srcnop with highest occurrence
//                        add it to exclusion nop list
//                    if no nop excluded due to MAXNOPALLOWED define:
//                      check whether one NOP occurs too often
//                      if so, add it to exclusion nop list
//                    if no nop excluded in last two steps:
//                      if file more than BIGLEVEL many segments,
//                       add nop to exclusion list if it occurs in more than
//                       50% of all nops
//                    free allocated space
//------------------------------------------------------------------------------
static VOID
NopUsage
(
  PITMIDA   pITMIda
)
{
  ULONG      ulAllSrcNops = 0;
  ULONG      ulAllTgtNops = 0;
  ULONG      ulLimit = 0;
  ULONG      i = 0;
  USHORT     usMaxOccur = 0;
  BOOL       fOK;
  LONG       lCount = 0;
  ULONG      ulMaxi = 0;
  ULONG      ulSrcUsed, ulTgtUsed;
  PSZ_W      pMaxData = 0;
  BOOL       fUseNops = TRUE;
  SHORT      sTokenID = 0;                 // TokenID of tag to be added to excltab
  SHORT      sAddInfo = 0;                 // AddInfo of tag to be added to excltab
  USHORT     usLen = 0;                    // length of tag to be added to excltab
  CHAR       chTextBuf[80];
  HMODULE    hResMod;

  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
  /********************************************************************/
  /* start "aligning" slider                                          */
  /********************************************************************/
  LOADSTRING( NULLHANDLE, hResMod, IDS_ITM_ALIGNING, chTextBuf);
  WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
              MP1FROMSHORT(-1), chTextBuf );
  chTextBuf[0] = EOS;
  WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
              MP1FROMSHORT(-2), chTextBuf );
  WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
              MP1FROMSHORT(0), NULL );
  pITMIda->usStartSlider = 0;

   /*******************************************************************/
   /* fill struct with all different NOPs that occur and count how    */
   /* often each NOP occurs                                           */
   /*******************************************************************/
   fOK = CheckNops(pITMIda, &pITMIda->TBSourceDoc,
                    &pITMIda->stSrcNopCnt);
   if ( fOK )
   {
     fOK = CheckNops(pITMIda, &pITMIda->TBTargetDoc,
                    &pITMIda->stTgtNopCnt);
   } /* endif */

   if ( fOK )
   {
     pITMIda->stExclNopCnt.ulUsed = 0;
     pITMIda->stExclNopCnt.ulAlloc = 0;
     /*****************************************************************/
     /* delete those NOPs which do not exist in the other doc         */
     /*****************************************************************/
     ExclSingleNop( pITMIda, &pITMIda->stExclNopCnt,
                    &pITMIda->stSrcNopCnt,
                    &pITMIda->stTgtNopCnt );
     ExclSingleNop( pITMIda, &pITMIda->stExclNopCnt,
                    &pITMIda->stTgtNopCnt,
                    &pITMIda->stSrcNopCnt );
     /*****************************************************************/
     /* add up how many NOPs exist overall in src and tgt             */
     /*****************************************************************/
     ulSrcUsed = pITMIda->stSrcNopCnt.ulUsed;
     ulTgtUsed = pITMIda->stTgtNopCnt.ulUsed;
     ulAllSrcNops = 0;
     for ( i=0;i<ulSrcUsed ;i++ )
     {
       ulAllSrcNops += pITMIda->stSrcNopCnt.pusOccur[i];
     } /* endfor */
     ulAllTgtNops = 0;
     for ( i=0;i<ulTgtUsed ;i++ )
     {
       ulAllTgtNops += pITMIda->stTgtNopCnt.pusOccur[i];
     } /* endfor */
     /*****************************************************************/
     /* add nops to stExclNopCnt until at most MAXNOPALLOWED nops     */
     /* are left                                                      */
     /*****************************************************************/
//     usMaxOccur = MINOCCURRENCE + 1;
//     while ( (usAllSrcNops > MAXNOPALLOWED)
//            && (usMaxOccur > MINOCCURRENCE ))
//     {
//       /***************************************************************/
//       /* find srcnop with highest occurrence                         */
//       /***************************************************************/
//       usMaxOccur = 0;
//       for ( i=0;i < usSrcUsed ;i++ )
//       {
//         if ( pITMIda->stSrcNopCnt.pusOccur[i] > usMaxOccur )
//         {
//           usMaxOccur = pITMIda->stSrcNopCnt.pusOccur[i];
//           usMaxi = i;
//         } /* endif */
//       } /* endfor */
//       /***************************************************************/
//       /* find that nop with biggest occurrence in target              */
//       /***************************************************************/
//       if ( usMaxOccur > MINOCCURRENCE )
//       {
//         fFound = FALSE;
//         ulMaxHash = pITMIda->stSrcNopCnt.pulHash[usMaxi];
//         pMaxData = pITMIda->stSrcNopCnt.ppData[usMaxi];
//         i = 0;
//         while ( !fFound && (i < usTgtUsed))
//         {
//           if ( (ulMaxHash == pITMIda->stTgtNopCnt.pulHash[i] ) &&
//               (strncmp(pITMIda->stSrcNopCnt.ppData[usMaxi],
//                       pITMIda->stTgtNopCnt.ppData[i],
//                       strlen(pITMIda->stSrcNopCnt.ppData[usMaxi])) == 0 ) )
//           {
//             fFound = TRUE;
//           }
//           else
//           {
//             i++;
//           } /* endif */
//         } /* endwhile */
//         /***************************************************************/
//         /* add nop to exclude list and set src and tgt occurrence to 0 */
//         /* to avoid reselection of same NOp                            */
//         /***************************************************************/
//         fOK = AddToExclNop( &pITMIda->stExclNopCnt,
//                        ulMaxHash, pMaxData);
//         if ( fOK )
//         {
//           fUseNops = FALSE;
//           usAllSrcNops -= usMaxOccur;
//           pITMIda->stSrcNopCnt.pusOccur[usMaxi] = 0;
//         } /* endif */
//
//         if ( fFound && fOK)
//         {
//           usAllTgtNops -= pITMIda->stTgtNopCnt.pusOccur[i];
//           pITMIda->stTgtNopCnt.pusOccur[i] = 0;
//           /*************************************************************/
//           /* nop not found in tgt, delete occurrence in src to         */
//           /* avoid reselection of same nop and add nop to excludelist  */
//           /* ( nop can never become an anchor because it does not occur*/
//           /* in tgt )                                                  */
//           /*************************************************************/
//         } /* endif */
//       } /* endif */
//     } /* endwhile */

     /*****************************************************************/
     /* if no NOP yet excluded, check whether one nop occurs too often*/
     /*****************************************************************/
     if ( fUseNops)
     {
        lCount = ulAllSrcNops;
        ulLimit = (lCount * NOPEXLEVEL);
        i = 0;
        while (( i < ulSrcUsed) && fUseNops )
        {
          if ( pITMIda->stSrcNopCnt.pusOccur[i] > (USHORT) ulLimit )
          {
            fOK = AddToExclNop( &pITMIda->stExclNopCnt,
                                pITMIda->stSrcNopCnt.psTokenID[i],
                                pITMIda->stSrcNopCnt.psAddInfo[i],
                                pITMIda->stSrcNopCnt.ppData[i],
                                pITMIda->stSrcNopCnt.pusLen[i] );
            fUseNops = FALSE;
            ulAllSrcNops -= pITMIda->stSrcNopCnt.pusOccur[i];
            pITMIda->stSrcNopCnt.pusOccur[i] = 0;
          } /* endif */
          i++;
        } /* endwhile */
     } /* endif */
     /*****************************************************************/
     /* if no NOP yet excluded: if big file, exclude the NOP with     */
     /* the highest occurrence, if it is more than 50%                */
     /*****************************************************************/
     if ( fUseNops && (pITMIda->TBSourceDoc.ulMaxSeg > BIGLEVEL ))
     {
       /***************************************************************/
       /* lower limit 1/ 10th of all nops                             */
       /***************************************************************/
       ulLimit = ulAllSrcNops * NOPEXBIGLEVEL ;
       usMaxOccur = 0;
       i = 0;
       while (( i < ulSrcUsed) && fUseNops )
       {
         if ( (pITMIda->stSrcNopCnt.pusOccur[i] > usMaxOccur)
              && !(pITMIda->stSrcNopCnt.psAddInfo[i] & TAG_ITM_EQUALNUM) )
         {
           usMaxOccur = pITMIda->stSrcNopCnt.pusOccur[i];

           pMaxData = pITMIda->stSrcNopCnt.ppData[i];
           sAddInfo = pITMIda->stSrcNopCnt.psAddInfo[i];
           sTokenID = pITMIda->stSrcNopCnt.psTokenID[i];
           usLen    = pITMIda->stSrcNopCnt.pusLen[i];
           ulMaxi = i;
         } /* endif */
         i++;
       } /* endwhile */
       /***************************************************************/
       /* if occurrence is too low, do not exclude it                 */
       /***************************************************************/
       if ( usMaxOccur > (USHORT) ulLimit )
       {
          fOK = AddToExclNop( &pITMIda->stExclNopCnt, sTokenID,
                              sAddInfo, pMaxData, usLen );
          if ( fOK )
          {
            fUseNops = FALSE;
            ulAllSrcNops -= pITMIda->stSrcNopCnt.pusOccur[ulMaxi];
            pITMIda->stSrcNopCnt.pusOccur[i] = 0;
          } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */
   /*************************************************************/
   /* free space                                                */
   /*************************************************************/
   UtlAlloc ( (PVOID *)&pITMIda->stTgtNopCnt.pusOccur, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stTgtNopCnt.psTokenID, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stTgtNopCnt.psAddInfo, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stTgtNopCnt.ppData, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stTgtNopCnt.pusLen, 0L, 0L, NOMSG);

   UtlAlloc ( (PVOID *)&pITMIda->stSrcNopCnt.pusOccur, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stSrcNopCnt.psTokenID, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stSrcNopCnt.psAddInfo, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stSrcNopCnt.ppData, 0L, 0L, NOMSG);
   UtlAlloc ( (PVOID *)&pITMIda->stSrcNopCnt.pusLen, 0L, 0L, NOMSG);

   pITMIda->stSrcNopCnt.ulUsed = 0;
   pITMIda->stTgtNopCnt.ulUsed = 0;
   pITMIda->stSrcNopCnt.ulAlloc = 0;
   pITMIda->stTgtNopCnt.ulAlloc = 0;
} /* end of function NopUsage */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     AddToExclNop
//------------------------------------------------------------------------------
// Function call:     AddToExclNop(pExclNopCnt,sTokenid,sAddInfo,pData,usLen)
//------------------------------------------------------------------------------
// Description:       add given nop to exclusion nop list_
//------------------------------------------------------------------------------
// Parameters:        PITMNOPCOUNT  pExclNopCnt,
//                    SHORT         sTokenID,
//                    SHORT         sAddInfo
//                    PSZ           pData
//                    USHORT        usLen
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   alloc is ok
//                    FALSE   alloc not possible
//------------------------------------------------------------------------------
// Function flow:     check whether alloc nec; do it if nec;
//                    fill next entry in exclusion list
//------------------------------------------------------------------------------
static BOOL
AddToExclNop
(
  PITMNOPCOUNT  pExclNopCnt,
  SHORT         sTokenID,
  SHORT         sAddInfo,
  PSZ_W         pData,
  USHORT        usLen
)
{
  ULONG         j;
  BOOL          fOK;

  fOK = NopCntAlloc(pExclNopCnt);
  if ( fOK )
  {
    j = pExclNopCnt->ulUsed;
    pExclNopCnt->psTokenID[j] = sTokenID;
    pExclNopCnt->psAddInfo[j] = sAddInfo;
    pExclNopCnt->ppData[j] = pData;
    pExclNopCnt->pusOccur[j] = 1;
    pExclNopCnt->pusLen[j] = usLen;
    pExclNopCnt->ulUsed ++;                  // point to next entry
  } /* endif */

  return (fOK);
} /* end of function AddToExclNop */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ExclSingleNop
//------------------------------------------------------------------------------
// Function call:     ExclSingleNop(pExclNopCnt, pMainNopCnt, pRefNopCnt)
//------------------------------------------------------------------------------
// Description:       exclude nop if it occurs only in one document
//------------------------------------------------------------------------------
// Parameters:        PITMNOPCOUNT  pExclNopCnt,
//                    PITMNOPCOUNT  pMainNopCnt,
//                    PITMNOPCOUNT  pRefNopCnt
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     loop trhu all nops in Main nop structure
//                       loop thru all nops in Ref nop structure
//                       if equal nop found stop
//                       if no equal nop found add nop to nop exclusion list
//------------------------------------------------------------------------------
static VOID
ExclSingleNop
(
  PITMIDA       pITMIda,
  PITMNOPCOUNT  pExclNopCnt,
  PITMNOPCOUNT  pMainNopCnt,
  PITMNOPCOUNT  pRefNopCnt
)
{
  ULONG        i, j;
  ULONG        ulMainUsed;
  ULONG        ulRefUsed;
  SHORT        sAddInfo;
  SHORT        sTokenID;
  PSZ_W        pData;
  BOOL         fFound;
  BOOL         fOK = TRUE;
  USHORT       usLen;

  ulMainUsed = pMainNopCnt->ulUsed;
  ulRefUsed = pRefNopCnt->ulUsed;
  for ( i=0;i < ulMainUsed ;i++ )
  {
    sAddInfo = pMainNopCnt->psAddInfo[i];
    sTokenID = pMainNopCnt->psTokenID[i];
    pData = pMainNopCnt->ppData[i];
    usLen = pMainNopCnt->pusLen[i];
    j = 0;
    fFound = FALSE;
    while ( (j < ulRefUsed) && !fFound )
    {
      /****************************************************************/
      /* if tokenid is equal and only 1st part of token is relevant   */
      /*                or                                            */
      /* if tokenid equal and data is equal w/o respect to blanks     */
      /****************************************************************/
      if ( TagsAreEqual(sTokenID, pRefNopCnt->psTokenID[j], sAddInfo,
                   pData, pRefNopCnt->ppData[j], pRefNopCnt->pusLen[j]) )
///      if ( (ulHash == pRefNopCnt->pulHash[j] ) &&
///            ItmCompChars(pData, pRefNopCnt->ppData[j] ) )
/////        (strncmp(pData, pRefNopCnt->ppData[j],
/////                strlen(pData)) == 0 ) )
      {
        fFound = TRUE;
      }
      else
      {
        j++;
      } /* endif */
    } /* endwhile */
    if ( !fFound )
    {
      fOK = AddToExclNop( pExclNopCnt, sTokenID, sAddInfo, pData, usLen);
      if ( fOK )
      {
        pMainNopCnt->pusOccur[i] = 0;
      } /* endif */
    } /* endif */
  } /* endfor */

  pITMIda->usStartSlider++;
  WinSendMsg( pITMIda->hwnd, WM_EQF_UPDATESLIDER,
              MP1FROMSHORT(pITMIda->usStartSlider), NULL );

  return;
} /* end of function ExclSingleNop */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     FreeAll
//------------------------------------------------------------------------------
// Function call:     FreeAll(pITMIda)
//------------------------------------------------------------------------------
// Description:       free all allocated areas
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     FreeNopDoc's
//                    free list of excluded nops
//                    free ITMIda alignment structure
//                    remove the copied/analysed files
//------------------------------------------------------------------------------
VOID
FreeAll
(
 PITMIDA   pITMIda
)
{
   PALLALIGNED  pAligned;

  /****************************************************************/
  /* free any allocated resources                                 */
  /****************************************************************/
  FreeNOPDoc( &pITMIda->TBSourceDoc, &pITMIda->itmSrcNop );
  FreeNOPDoc( &pITMIda->TBTargetDoc, &pITMIda->itmTgtNop );

  /*******************************************************************/
  /* free nop exclusion list                                         */
  /*******************************************************************/
  UtlAlloc ( (PVOID *)&pITMIda->stExclNopCnt.pusOccur, 0L, 0L, NOMSG);
  UtlAlloc ( (PVOID *)&pITMIda->stExclNopCnt.psAddInfo, 0L, 0L, NOMSG);
  UtlAlloc ( (PVOID *)&pITMIda->stExclNopCnt.psTokenID, 0L, 0L, NOMSG);
  UtlAlloc ( (PVOID *)&pITMIda->stExclNopCnt.ppData, 0L, 0L, NOMSG);
  UtlAlloc ( (PVOID *)&pITMIda->stExclNopCnt.pusLen, 0L, 0L, NOMSG);

  pITMIda->stExclNopCnt.ulUsed = 0;
  pITMIda->stExclNopCnt.ulAlloc = 0;

  if (pITMIda->pulSrcNumAlign)
  {
    FREEHUGE(pITMIda->pulSrcNumAlign);

    if (pITMIda->stVisDocSrc.pulNumAligned == pITMIda->pulSrcNumAlign )
    {
		pITMIda->stVisDocSrc.pulNumAligned = NULL;
    }
    pITMIda->pulSrcNumAlign = NULL;
  }

  if (pITMIda->pulSrcAnchor)
  {
    FREEHUGE(pITMIda->pulSrcAnchor);
    if (pITMIda->stVisDocSrc.pulAnchor == pITMIda->pulSrcAnchor )
	{
			pITMIda->stVisDocSrc.pulAnchor = NULL;
    }
    pITMIda->pulSrcAnchor = NULL;
  }

  if (pITMIda->pSrcVisState)
  {
    FREEHUGE(pITMIda->pSrcVisState);
    if (pITMIda->stVisDocSrc.pVisState == pITMIda->pSrcVisState )
	{
			pITMIda->stVisDocSrc.pVisState = NULL;
    }
    pITMIda->pSrcVisState = NULL;
  }

  if (pITMIda->pulTgtNumAlign)
  {
    FREEHUGE(pITMIda->pulTgtNumAlign);
    if (pITMIda->stVisDocTgt.pulNumAligned == pITMIda->pulTgtNumAlign )
	{
			pITMIda->stVisDocTgt.pulNumAligned = NULL;
    }
    pITMIda->pulTgtNumAlign = NULL;
  }

  if (pITMIda->pulTgtAnchor)
  {
    FREEHUGE(pITMIda->pulTgtAnchor);
    if (pITMIda->stVisDocTgt.pulAnchor == pITMIda->pulTgtAnchor )
	{
			pITMIda->stVisDocTgt.pulAnchor = NULL;
    }
    pITMIda->pulTgtAnchor = NULL;
  }

  if (pITMIda->pTgtVisState)
  {
    FREEHUGE(pITMIda->pTgtVisState);
    if (pITMIda->stVisDocTgt.pVisState == pITMIda->pTgtVisState )
	{
			pITMIda->stVisDocTgt.pVisState = NULL;
    }
    pITMIda->pTgtVisState = NULL;
  }
  /********************************************************************/
  /* free alignment structure                                         */
  /********************************************************************/
  pAligned = &(pITMIda->Aligned);

  UtlAlloc( (PVOID *) &(pAligned->pulSrc), 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &(pAligned->pulTgt1), 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &(pAligned->psDist), 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &(pAligned->pbType), 0L, 0L, NOMSG );
  pAligned->ulAlloc = 0;
  pAligned->ulUsed  = 0;
  pAligned->ulFillStart  = 0;
  pAligned->ulFillEnd    = 0;
  pAligned->ulFillIndex  = 0;

  /******************************************************************/
  /* remove the copied/analysed files (if necessary)                */
  /******************************************************************/
  RemoveFiles( pITMIda );
  return;
} /* end of function FreeAll */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFITMDocTagRead
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       read in document tagtable and do EQFBFileRead
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       sRc
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------

SHORT
EQFITMDocTagRead
(
  PITMIDA       pITMIda,
  PTBDOCUMENT   pDoc,                     //TBSOurcedoc or TBTargetDoc
  PSZ           pSegFile,                 //name of segmented src/tgtfile
  USHORT        usDocType                 //ssource_DOC or STARGET_DOC
)
{
  SHORT         sRc = 0;

  pDoc->ulMaxSeg = 0;
  pDoc->ulMaxLine = 0;

  /********************************************************************/
  /* load document tag table                                          */
  /********************************************************************/
  sRc = TALoadTagTable( pITMIda->chTagTableName,
                         (PLOADEDTABLE *) &pITMIda->pLoadedTable,
                         FALSE, FALSE );
  pDoc->pDocTagTable = pITMIda->pLoadedTable;

  /**************************************************************/
  /* load files and put them into segment structure             */
  /**************************************************************/
  if ( !sRc && !pITMIda->fKill )
  {
    if ( ITMFileExist( pSegFile ) )
    {
      pDoc->docType = (DOCTYPE) usDocType;
//      sRc = EQFBFileRead( pSegFile, pDoc);
      sRc = EQFBFileReadExW( pSegFile, pDoc, FILEREAD_SINGLETABLE);
    }
    else
    {
      sRc = 1;
    } /* endif */
    /************************************************************/
    /* if not already cleaning up ...                           */
    /************************************************************/
    if ( sRc && ! pITMIda->fKill )
    {
      ITMUtlError( pITMIda, FILE_NOT_EXISTS, MB_CANCEL, 1, &pSegFile,EQF_ERROR );
    } /* endif */
  } /* endif */


  return (sRc);

} /* end of function EQFITMDocTagRead */

VOID ITMCloseProcessing
(
  PITMIDA pITMIda
)
{
   /********************************************************************/
   /* close open file ...                                              */
   /********************************************************************/
   if ( pITMIda->pBufCB )
   {
     UtlBufWriteConv( pITMIda->pBufCB, NTM_END_TAGW,
                      (USHORT)UTF16strlenBYTE(NTM_END_TAGW),
                      TRUE, pITMIda->usSGMLFormat, pITMIda->ulSGMLFormatCP,
                      pITMIda->ulAnsiCP);

     /******************************************************************/
     /* close it any way ...                                           */
     /******************************************************************/
     UtlBufClose( pITMIda->pBufCB, FALSE );
     pITMIda->pBufCB = NULL;
   } /* endif */
   /********************************************************************/
   /* close translation memory                                         */
   /********************************************************************/
   if ( pITMIda->pMem)
   {
     ITMClose( pITMIda );
   } /* endif */

   /********************************************************************/
   /* close and write property file                                    */
   /********************************************************************/
   EQFITMPropWrite ( pITMIda );
   /********************************************************************/
   /* remove temporary folder structure                                */
   /********************************************************************/
   if ( !pITMIda->fNoAna )
   {
     RemoveFolderStruct( pITMIda->chITMSFolder );
     RemoveFolderStruct( pITMIda->chITMTFolder );
   } /* endif */
}
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     ItmCompChars
//------------------------------------------------------------------------------
// Function call:     ItmCompChars(pData1, pData2)
//------------------------------------------------------------------------------
// Description:       compare 2 strings; equal if only differences in the
//                    number of leading or ending blanks
//------------------------------------------------------------------------------
// Parameters:        PSZ    pData1
//                    PSZ    pData2
//                    USHORT usLen2
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE            if equal
//                    FALSE           if not equal
//------------------------------------------------------------------------------
// Function flow:     skip leading blanks in both strings
//                    loop thru both strings as long they are equal
//                    if equal
//                       check that in longer string only blanks are
//                       remaining
//------------------------------------------------------------------------------
BOOL
ItmCompChars
(
   PSZ_W     pData1,
   PSZ_W     pData2,
   LONG      lLen2
)
{  BOOL      fEqual = TRUE;
   CHAR_W    c,d;

   /*******************************************************************/
   /* skip leading blanks                                             */
   /*******************************************************************/
   while ( *pData1 && (*pData1 == BLANK ))
   {
     pData1++;
   } /* endwhile */
   while ( lLen2 && (*pData2 == BLANK ) )
   {
     pData2++;
     lLen2--;
   } /* endwhile */
   /*******************************************************************/
   /* compare remaining characters                                    */
   /* stops if either at end of one string or strings are not equal   */
   /*******************************************************************/
   while ( lLen2 && (*pData1) && fEqual )
   {
     c = *pData1;
     d = *pData2;
     if ( UtlToUpperW(c) == UtlToUpperW(d) )
     {
       pData1++;
       pData2++;
       lLen2--;
     }
     else
     {
       fEqual = FALSE;
     } /* endif */
   } /* endwhile */
   if ( fEqual )
   {
     while ( lLen2 && fEqual )
     {
       if ( *pData2 == BLANK )
       {
         pData2++;
         lLen2--;
       }
       else
       {
         fEqual = FALSE;
       } /* endif */
     } /* endwhile */

   } /* endif */

  return (fEqual);
} /* end of function ItmCompChars */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TagsAreEqual
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       compares two tags, 1st using the tokenid
//------------------------------------------------------------------------------
// Parameters:
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE        tags are equal
//                    FALSE       tags are not equal
//------------------------------------------------------------------------------
// Function flow:     if tokenids are equal and only 1st part is relevant
//                     or
//                    if tokenids are equal and datastrings are equal
//                       return True ( tags are equal)
//                    else
//                       return FALSE
//------------------------------------------------------------------------------

static BOOL
TagsAreEqual
(
   SHORT    sTokenID,
   SHORT    sRefTokenID,
   SHORT    sAddInfo,
   PSZ_W    pData,
   PSZ_W    pRefData,
   LONG     lRefLen
)
{
  BOOL      fEqual = FALSE;

  /********************************************************************/
  /* compare with respect to the TAG_ITM_PART flag set in the sAddInfo*/
  /********************************************************************/
  if ( (sTokenID == sRefTokenID &&
          ((sAddInfo & TAG_ITM_PART) ||
           ItmCompChars(pData, pRefData, lRefLen) )  ))
  {
    fEqual = TRUE;
  } /* endif */
  return (fEqual);
} /* end of function TagsAreEqual */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMIsAnchor
//------------------------------------------------------------------------------
// Function call:     ITMIsAnchor(pITMIda, usSrcSeg, usTgtSeg, fRealign)
//------------------------------------------------------------------------------
// Description:       check if usSrcSeg is anchor with usTgtSeg
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda
//                    USHORT     usSrcSeg
//                    USHORT     usTgtSeg
//                    BOOL       fRealign     //true if during visual realign
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE        usSrcSeg is anchor with usTgtSeg
//                    FALSE       usSrcSeg is not anchor with usTgtSeg
//------------------------------------------------------------------------------
// Function flow:     if fRealign
//                       check pVisDoc->pulAnchor whether usSrcSeg is Anchor
//                       with usTgtSeg
//                    else
//                    endif
//------------------------------------------------------------------------------

static BOOL
ITMIsAnchor
(
   PITMIDA  pITMIda,
   ULONG    ulSrcSeg,
   ULONG    ulTgtSeg,
   BOOL     fRealign
)
{
  BOOL         fIsAnchor = FALSE;      // returns the result
  PITMNOPSEGS  pSrcNop;                // pointer to source nop segments
  PITMNOPSEGS  pTgtNop;                // pointer to target nop segments
  PITMVISDOC   pVisSrc;                // ptr to visual src doc.struct
  BOOL         fEnd = FALSE;           // true if srcseg found or endoflist
  ULONG        ulK = 0;                  // anchor list index

  if (fRealign )
  {
    /******************************************************************/
    /* realigning due to user interaction in visualization            */
    /* overcross anchors are handled as all other anchors             */
    /* (pVisState[usSrcSeg].OverCross is not checked)                 */
    /******************************************************************/
    pVisSrc = &(pITMIda->stVisDocSrc);
    if (pVisSrc->pulAnchor[ulSrcSeg] == ulTgtSeg )
    {
      fIsAnchor = TRUE;
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* ITM alignment                                                  */
    /******************************************************************/
    pSrcNop = &pITMIda->itmSrcNop;
    pTgtNop = &pITMIda->itmTgtNop;

    while (!fEnd )
    {
      /****************************************************************/
      /* anchor list is parsed: each segnum occurs at most once in    */
      /* the list                                                     */
      /****************************************************************/
      if (pSrcNop->pulSegs[ulK] == ulSrcSeg )
      {
        if (pTgtNop->pulSegs[ulK] == ulTgtSeg )
        {
          fIsAnchor = TRUE;                      //is anchor with usTgtSeg
        } /* endif */
        fEnd = TRUE;                           //stops searching
      }
      else
      {
        ulK++;
        if (ulK >= pSrcNop->ulUsed )                //goon while not at endoflist
        {
          fEnd = TRUE;                          // stops searching
        } /* endif */
      } /* endif */
    } /* endwhile */

  } /* endif */


  return (fIsAnchor);
} /* end of function ITMIsAnchor     */



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMCrossOut
//------------------------------------------------------------------------------
// Function call:     ITMCrossOut(pITMIda, pDoc, pSeg, fCrossOut)
//------------------------------------------------------------------------------
// Description:       cross out the segments in-between the X-NOPs
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda
//                    PTBDOCUMENT pDoc
//                    PTBSEGMENT  pSeg     Segment with STARTX Addinfo
//------------------------------------------------------------------------------
// Returncode type:   ULONG     ulSegNum
//------------------------------------------------------------------------------
// Returncodes:       ulSegNum = input ulSegNum, if no valid X-area found
//                    ulSegNum = last segment of X- area if found
//------------------------------------------------------------------------------
// Function flow:     while not at end of file and not at "ENDX" NOP
//                       tokenize NOP segment to get addinfo
//                       if NOP has STARTX : increase counter
//                       if NOP has ENDX:    decrease counter
//                       if at end of all nested X-regions:
//                          set end indicator
//                       else
//                          goto next segment
//                    endwhile
//                    if end of X-area found:
//                      set qStatus = QF_CROSSED_OUT for all segments in area
//                    endif
//                    return segment number of last segment of X-area
//------------------------------------------------------------------------------
// Note:          X-area: is the area surrounded by the STARTX /ENDX nop
//                 If STARTX /ENDX nops are nested, the X-area is from the
//                 1st STARTX until all "open" areas are closed.
//                 If the number of "open" areas is not equal to the number
//                 of areas which is closed ( STARTX occurs as often as ENDX),
//                 no X-area will be made.
//------------------------------------------------------------------------------

static ULONG
ITMCrossOut
(
   PITMIDA      pITMIda,
   PTBDOCUMENT  pDoc,
   PTBSEGMENT   pSeg
)
{
  USHORT       usNumofX = 0;               // stack count of opened/closed X-NOPS
  ULONG        ulSegStart;
  USHORT       fEndFound = FALSE;          // true if end of ignored-area found
  PCHAR_W      pRest = NULL;
  USHORT       usColPos = 0;
  PTOKENENTRY  pTok;
  ULONG        ulI;
  ULONG        ulSegNum;

  ulSegStart = pSeg->ulSegNum;
  ulSegNum = ulSegStart;

  /********************************************************************/
  /* loop from beginning of X_area til the end of X-area              */
  /********************************************************************/
  while ( pSeg && !fEndFound)
  {
    TATagTokenizeW( pSeg->pDataW,
                   ((PLOADEDTABLE)pITMIda->pLoadedTable),
                   TRUE,
                   &pRest,
                   &usColPos,
                   (PTOKENENTRY) pDoc->pTokBuf,
                   TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
    pTok = (PTOKENENTRY) pDoc->pTokBuf;
    while ( pTok->sTokenid != ENDOFLIST )
    {
      if ( pTok->sTokenid >= 0 )
      {
        if ( pTok->sAddInfo & TAG_ITM_STARTX )
        {
          usNumofX++;                        // one more X-Start-NOP
        } /* endif */
        if ( pTok->sAddInfo & TAG_ITM_ENDX )
        {                                    // one X-region is at the end
          usNumofX--;
        } /* endif */
      } /* endif */
      pTok++;
    } /* endwhile */

    if ( usNumofX == 0 )                // at end of all nested X-regions
    {
      fEndFound = TRUE;
    }
    else
    {
      ulSegNum++;                         // point to next segment
      pSeg = EQFBGetSegW(pDoc, ulSegNum);
    } /* endif */
  } /* endwhile */

  if ( fEndFound )
  {
    for ( ulI=ulSegStart+1 ; ulI < ulSegNum; ulI++ )     //cross-out X-area
    {
      pSeg = EQFBGetSegW(pDoc, ulI);
      if ( pSeg )
      {
        if (pSeg->qStatus == QF_NOP )
        {
          pSeg->qStatus = QF_CROSSED_OUT_NOP;
        }
        else
        {
          pSeg->qStatus = QF_CROSSED_OUT;
        }
      } /* endif */
    } /* endfor */
  }
  else
  {
    ulSegNum = ulSegStart;                             //return usSegStart
  } /* endif */
  return (ulSegNum);
} /* end of function ITMCrossOut     */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PostParseEqualNums
//------------------------------------------------------------------------------
// Function call:     PostParseEqualNums(pITMIda     )
//------------------------------------------------------------------------------
// Description:       parse NOPs with TAG_ITM_EQUALNUM
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     start at begin of anchor list
//                    while not at end of anchor list
//                      get srcsegment of current and next anchor from list
//                      init array to store a maximum of MAX_EQNUM new
//                       assistant anchors
//                      find 1st segment in current paragraph with "assistent"
//                       addinfo (TAG_ITM_EQUALNUM)
//                      if a "assistant" Nop segment is found
//                        count how often it occurs in current source paragraph
//                        count how often it occurs in current target paragraph
//                        if occurrence is equal, insert "assistant" NOps
//                        as anchors in anchor list
//                      endif
//                      goon with next entry in anchor list
//                    endwhile
//------------------------------------------------------------------------------

static void
PostParseEqualNums
(
   PITMIDA      pITMIda
)
{
  ULONG        ulNopIndex;
  CHAR_W       szTagData[MAX_TERM_LEN];
  ULONG        ulFoundSeg;
  ULONG        ulSegSrc[MAX_EQNUM];
  ULONG        ulSegTgt[MAX_EQNUM];
  ULONG        ulStartSeg;
  ULONG        ulEndSeg;
  USHORT       usCountSrc;
  USHORT       usCountTgt;

  ulNopIndex = 0;

  while ( ulNopIndex < pITMIda->itmSrcNop.ulUsed )
  {

    ulStartSeg = pITMIda->itmSrcNop.pulSegs[ulNopIndex];
    ulEndSeg = pITMIda->itmSrcNop.pulSegs[ulNopIndex+1];
    memset(&ulSegSrc[0], 0, MAX_EQNUM * sizeof(ULONG));
    memset(&ulSegTgt[0], 0, MAX_EQNUM * sizeof(ULONG));
    ulFoundSeg = 0;
    ulFoundSeg = ITMFind1stEqNum (pITMIda, ulStartSeg, ulEndSeg, szTagData );
    /******************************************************************/
    /* check how often the NOP occurs in both paragraphs              */
    /******************************************************************/
    if (ulFoundSeg )
    {
      usCountSrc = ITMCountEqNum(pITMIda, &pITMIda->TBSourceDoc,
                      ulFoundSeg, ulEndSeg,
                      szTagData, &ulSegSrc[0] );

      usCountTgt = ITMCountEqNum(pITMIda, &pITMIda->TBTargetDoc,
                     pITMIda->itmTgtNop.pulSegs[ulNopIndex],
                     pITMIda->itmTgtNop.pulSegs[ulNopIndex+1],
                     szTagData, &ulSegTgt[0] );
     /*****************************************************************/
     /* if NOP  occurrence is equal, add NOPs as anchors              */
     /*****************************************************************/
     if (usCountSrc == usCountTgt )
     {
       ulNopIndex = ITMInsertAnchor (pITMIda, ulNopIndex,
                                     usCountSrc,
                                     &ulSegSrc[0], &ulSegTgt[0] );
     }
     else
     {
       ulNopIndex++;
     } /* endif */

    }
    else
    {
      ulNopIndex++;                   // check next paragraph
    } /* endif */

  } /* endwhile */

  return;
} /* end of function PostParseEqualNums    */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMFind1stEqNum
//------------------------------------------------------------------------------
// Function call:     ITMFind1stEqNum
//------------------------------------------------------------------------------
// Description:       find 1st NOP with TAG_ITM_EQUALNUM in paragraph
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda
//                    USHORT     usStartSeg
//                    USHORT     usEndSeg
//                    PSZ        pTagData
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:              ulSegNum found
//                           0 if none is found
//------------------------------------------------------------------------------
// Function flow:     start with 1st segnum after beginning anchor
//                    while no assistant NOP found and not at end of block
//                      if segment if NOP:
//                         tokenize segment
//                         if segment has addinfo "TAG_ITM_EQUALNUM"
//                           remember segmentnumber and get Tokendata
//                         endif
//                      endif
//                      goto next segment
//                    endwhile
//                    return segmentnumber found
//------------------------------------------------------------------------------

static ULONG
ITMFind1stEqNum
(
   PITMIDA      pITMIda,
   ULONG        ulStartSeg,
   ULONG        ulEndSeg,
   PSZ_W        pTagData
)
{
  ULONG         ulFoundSeg = 0;
  PTBDOCUMENT   pDoc;
  ULONG         ulSegNum;
  PCHAR_W       pRest = NULL;
  USHORT        usColPos = 0;
  PTOKENENTRY   pTok;
  PTBSEGMENT    pSeg;

  pDoc = &(pITMIda->TBSourceDoc);
  ulSegNum = ulStartSeg;
  ulSegNum++;
  /********************************************************************/
  /* loop thru current source paragraph and search for a NOP with     */
  /* sAddInfo TAG_ITM_EQUALNUM                                        */
  /********************************************************************/
  while ((ulSegNum < ulEndSeg) && !ulFoundSeg)
  {
    pSeg = EQFBGetSegW(pDoc, ulSegNum);
    if (pSeg && (pSeg->qStatus == QF_NOP) )
    {
      TATagTokenizeW( pSeg->pDataW,
                     ((PLOADEDTABLE)pITMIda->pLoadedTable),
                     TRUE,
                     &pRest,
                     &usColPos,
                     (PTOKENENTRY) pDoc->pTokBuf,
                     TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
      pTok = (PTOKENENTRY) pDoc->pTokBuf;
      while ( pTok->sTokenid != ENDOFLIST )
      {
        if ( (pTok->sTokenid >= 0) &&
             (pTok->sAddInfo & TAG_ITM_EQUALNUM) )     // if NOP is "assistant"
        {
          ulFoundSeg = ulSegNum;
          if (! TAGetTagNameByIDW(pITMIda->pLoadedTable,
                           pTok->sTokenid, pTagData))
          {
            ulFoundSeg = 0;                        // something went wrong
          } /* endif */
        } /* endif */
        pTok++;
      } /* endwhile */

    } /* endif */
    ulSegNum++;
  } /* endwhile */

  return(ulFoundSeg);
} /* end of function ITMFind1stEqNum       */



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMCountEqNum
//------------------------------------------------------------------------------
// Function call:     ITMCountEqNum
//------------------------------------------------------------------------------
// Description:       find 1st NOP with TAG_ITM_EQUALNUM in paragraph
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc
//                    USHORT     usStartSeg
//                    USHORT     usEndSeg
//                    PSZ        pTagData
//                    PSZ        pSegNums
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:              ulSegNum found
//                           0 if none is found
//------------------------------------------------------------------------------
// Function flow:     while not end of block and not MAX_EQNUM many found
//                      get segment
//                      if segment is NOP
//                        tokenize it
//                        if segment is "assistant" and is the same as the
//                        first, increase counter and remember segment number
//                      endif
//                      increase segment number
//                    endwhile
//                    return number of "assistant" NOPs found
//------------------------------------------------------------------------------
static USHORT
ITMCountEqNum
(
   PITMIDA      pITMIda,
   PTBDOCUMENT  pDoc,
   ULONG        ulStartSeg,
   ULONG        ulEndSeg,
   PSZ_W        pTagData,
   PULONG       pulSegNums
)
{
  ULONG         ulSegNum;
  PCHAR_W       pRest = NULL;
  USHORT        usColPos = 0;
  PTOKENENTRY   pTok;
  USHORT        usCount = 0;
  CHAR_W        szCurTagData[MAX_TERM_LEN];
  PTBSEGMENT    pSeg;

  ulSegNum = ulStartSeg;
  /********************************************************************/
  /* loop thru current source paragraph and search for a NOP with     */
  /* sAddInfo TAG_ITM_EQUALNUM                                        */
  /********************************************************************/
  while ((ulSegNum < ulEndSeg) && (usCount < MAX_EQNUM) )
  {
    pSeg = EQFBGetSegW(pDoc, ulSegNum);
    if (pSeg && (pSeg->qStatus == QF_NOP) )
    {
      TATagTokenizeW( pSeg->pDataW,
                     ((PLOADEDTABLE)pITMIda->pLoadedTable),
                     TRUE,
                     &pRest,
                     &usColPos,
                     (PTOKENENTRY) pDoc->pTokBuf,
                     TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
      pTok = (PTOKENENTRY) pDoc->pTokBuf;
      while ( pTok->sTokenid != ENDOFLIST )
      {
        if ( (pTok->sTokenid >= 0 ) &&
             (pTok->sAddInfo & TAG_ITM_EQUALNUM) )
        {
          if ( TAGetTagNameByIDW(pITMIda->pLoadedTable,
                           pTok->sTokenid, szCurTagData))
          {
            /************************************************************/
            /* if the same NOP is found, increase counter and remember  */
            /* segment number.                                          */
            /************************************************************/
            if (!UTF16strcmp(pTagData, szCurTagData) )
            {
              usCount ++;
              *pulSegNums = ulSegNum;
              pulSegNums++;
            } /* endif */
          } /* endif */
        } /* endif */
        pTok++;
      } /* endwhile */

    } /* endif */
    ulSegNum++;
  } /* endwhile */

  return(usCount);
} /* end of function ITMCountEqNum         */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMInsertAnchor
//------------------------------------------------------------------------------
// Function call:     ITMInsertAnchor
//------------------------------------------------------------------------------
// Description:       find 1st NOP with TAG_ITM_EQUALNUM in paragraph
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda
//                    USHORT     usNopIndex
//                    USHORT     usCount
//                    PUSHORT    pulSrcSeg
//                    PUSHORT    pulTgtSeg
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:              next usNopIndex which should be checked
//------------------------------------------------------------------------------
// Function flow:     if not enough space already allocated for anchorlist
//                      allocate more space for anchor list
//                    if fOK
//                      move contents of anchorslist to make room
//                      fill in new anchors ( segmentnumbers are remembered)
//                    else
//                      do not store assistant anchors, goon with next anchor
//                    return next anchor in anchorlist
//------------------------------------------------------------------------------
static ULONG
ITMInsertAnchor
(
  PITMIDA   pITMIda,
  ULONG     ulNopIndex,
  USHORT    usCount,
  PULONG    pulSrcSeg,
  PULONG    pulTgtSeg
)
{
  PITMNOPSEGS   pSrcNop;
  PITMNOPSEGS   pTgtNop;
  ULONG         l;
  BOOL          fOK = TRUE;

    /******************************************************************/
    /* make room for usCount many new anchors                         */
    /******************************************************************/
    /******************************************************************/
    /* if nec, allocate more space !! TODO !!                         */
    /******************************************************************/
    pSrcNop = &pITMIda->itmSrcNop;
    pTgtNop = &pITMIda->itmTgtNop;

    if ( pSrcNop->ulUsed + usCount >= pSrcNop->ulAlloc )
    {
      fOK = HashBlockAlloc(pSrcNop);
      fOK = HashBlockAlloc(pTgtNop);
    } /* endif */
    if (fOK )
    {
      for (l=pSrcNop->ulUsed-1 ;l > ulNopIndex ;l-- )
      {
        pTgtNop->pulSegs[l+usCount] = pTgtNop->pulSegs[l];
        pSrcNop->pulSegs[l+usCount] = pSrcNop->pulSegs[l];
      } /* endfor */
      pSrcNop->ulUsed += usCount;
      pTgtNop->ulUsed += usCount;

      for (l=(ulNopIndex + 1) ;l < (usCount+ulNopIndex+1) ;l++ )
      {
        pTgtNop->pulSegs[l] = *pulTgtSeg;
        pSrcNop->pulSegs[l] = *pulSrcSeg;
        pulTgtSeg++;
        pulSrcSeg++;
        /****************************************************************/
        /* it is not nec to fill the other fields..??                   */
        /****************************************************************/
      } /* endfor */
      ulNopIndex +=usCount+1;
    }
    else
    {
      ulNopIndex++;          // alloc not possible, goon with next paragraph
    } /* endif */

  return(ulNopIndex);
} /* end of function ITMInsertAnchor       */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PostParseUndoCrossedOut
//------------------------------------------------------------------------------
// Function call:     PostParseUndoCrossedOut(pITMIda     )
//------------------------------------------------------------------------------
// Description:       make crossed-out areas active again if start and end
//                    are anchors ( this garantuees that x-area exists parallel
//                    in both documents)
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     start at begin of anchor list
//                    while not at end of anchor list
//                      if anchor at begin of paragraph is NOP
//                        tokenize it and search for token with STARTX
//                      if STARTX is found
//                        if anchor at end of paragraph is NOP
//                          tokenize it and search for token with ENDX
//                          if ENDX token is found
//                            undo the cross-out of this paragraph
//                      goon with next entry in anchor list
//                    endwhile
//------------------------------------------------------------------------------

static void
PostParseUndoCrossedOut
(
   PITMIDA      pITMIda
)
{
  ULONG        ulNopIndex;
  ULONG        ulStartSeg;
  ULONG        ulEndSeg;
  PTBSEGMENT   pSeg;
  PTOKENENTRY  pTok;
  USHORT       usColPos;
  PTBDOCUMENT  pSrcDoc;
  PTBDOCUMENT  pTgtDoc;
  PCHAR_W      pRest = NULL;
  BOOL         fAnchorIsStartX = FALSE;
  BOOL         fAnchorIsEndX = FALSE;


  ulNopIndex = 0;

  pSrcDoc = &(pITMIda->TBSourceDoc);
  pTgtDoc = &(pITMIda->TBTargetDoc);

  while ( ulNopIndex < pITMIda->itmSrcNop.ulUsed )
  {
    ulStartSeg = pITMIda->itmSrcNop.pulSegs[ulNopIndex];
    ulEndSeg = pITMIda->itmSrcNop.pulSegs[ulNopIndex+1];
    fAnchorIsStartX = FALSE;
    fAnchorIsEndX = FALSE;
    /******************************************************************/
    /* check whether usStartSeg is AddInfo StartX                     */
    /*  ( ulStartSeg is anchor)                                       */
    /******************************************************************/
    pSeg = EQFBGetSegW(pSrcDoc, ulStartSeg);
    if ( pSeg && (pSeg->qStatus == QF_NOP))
    {
       TATagTokenizeW( pSeg->pDataW,
                      ((PLOADEDTABLE)pITMIda->pLoadedTable),
                      TRUE,
                      &pRest,
                      &usColPos,
                      (PTOKENENTRY) pSrcDoc->pTokBuf,
                      TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
       pTok = (PTOKENENTRY) pSrcDoc->pTokBuf;
       while ( !fAnchorIsStartX &&
                (pTok->sTokenid != ENDOFLIST ))
       {
         if ( (pTok->sTokenid >= 0) &&
              (pTok->sAddInfo & TAG_ITM_STARTX ))
         {
            fAnchorIsStartX = TRUE;
         }
         else
         {
           pTok++;
         } /* endif */
       } /* endwhile */

       /**********************************************************/
       /* check that next anchor is end of crossed-out area      */
       /**********************************************************/
       if ( fAnchorIsStartX )
       {
         pSeg = EQFBGetSegW(pSrcDoc, ulEndSeg);
         if ( pSeg && (pSeg->qStatus == QF_NOP))
         {
            TATagTokenizeW( pSeg->pDataW,
                      ((PLOADEDTABLE)pITMIda->pLoadedTable),
                      TRUE,
                      &pRest,
                      &usColPos,
                      (PTOKENENTRY) pSrcDoc->pTokBuf,
                      TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );
            pTok = (PTOKENENTRY) pSrcDoc->pTokBuf;
            while ( !fAnchorIsEndX &&
                     (pTok->sTokenid != ENDOFLIST ))
            {
              if ( (pTok->sTokenid >= 0) &&
                   (pTok->sAddInfo & TAG_ITM_ENDX ))
              {
                 ITMDoUndoCrossOut(ulStartSeg, ulEndSeg, pSrcDoc);
                 ITMDoUndoCrossOut(pITMIda->itmTgtNop.pulSegs[ulNopIndex],
                                   pITMIda->itmTgtNop.pulSegs[ulNopIndex+1],
                                   pTgtDoc);
                 fAnchorIsEndX = TRUE;
                 ulNopIndex++;      // not nec. to check ulNopIndex +1 for STARTX
              }
              else
              {
                pTok++;
              } /* endif */
            } /* endwhile */
         } /* endif */
       } /* endif */
    } /* endif */
    ulNopIndex++;
  } /* endwhile */

  return;
}  /* end of function PostParseUndoCrossedOut */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMDoUndoCrossOut
//------------------------------------------------------------------------------
// Function call:     ITMDoUndoCrossOut(ulStartSeg, ulEndSeg, pDoc)
//------------------------------------------------------------------------------
// Description:       make crossed-out areas from usStartSeg until usENdSeg
//                    "active" again
//------------------------------------------------------------------------------
// Parameters:        ULONG   ulStartSeg
//                    ULONG   ulEndSeg
//                    PTBDOCUMENT pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     while not at end of area
//                      get segment
//                      if qStatus is QF_CROSSED_OUT, reset to QF_TOBE
//                      if qStatus is QF_CROSSED_OUT_NOP, reset to QF_NOP
//                    endwhile
//------------------------------------------------------------------------------
static void
ITMDoUndoCrossOut
(
  ULONG     ulStartSeg,
  ULONG     ulEndSeg,
  PTBDOCUMENT pDoc
)
{
     ULONG      ulI;
     BOOL       fEndFound = FALSE;
     PTBSEGMENT pSeg;

     ulI = ulStartSeg + 1;
     while (ulI < ulEndSeg && !fEndFound)
     {
       pSeg = EQFBGetSegW(pDoc, ulI);
       if (pSeg)
       {
         if (pSeg->qStatus == QF_CROSSED_OUT)
         {
           pSeg->qStatus = QF_TOBE;
           ulI++;
         }
         else if (pSeg->qStatus == QF_CROSSED_OUT_NOP)
         {
            pSeg->qStatus = QF_NOP;
            ulI++;
         }
         else
         {
            fEndFound = TRUE;
         }
       }
       else
       {
         fEndFound = TRUE;
       }
     } /* endwhile*/
  return;
  }  /* end of function ITMDoUndoCrossOut   */



LONG
ITMGetRelLength
(
     PVOID   pVoidTable,            // pointer to tag table
     USHORT  usTokLen,
     SHORT   sAddInfo,
     SHORT   sTokenid,
     PSZ_W   pszBuffer              // buffer for tag name
)
{
   LONG lLength;

    if ( sAddInfo & TAG_ITM_PART )
    {
      /**********************************************************/
      /* only start of tag is relevant, only this is compared   */
      /* during the anchor search                               */
      /* therefore change ItmCompChars, or ??                   */
      /* use GetTagNameByID to get relevant length              */
      /**********************************************************/
      if (TAGetTagNameByIDW(pVoidTable,
                       sTokenid, pszBuffer))
      {
        lLength = UTF16strlenCHAR(pszBuffer);
        if ( lLength &&
             pszBuffer[ lLength-1 ] == CHAR_MULT_SUBST )
        {
          lLength --;
        } /* endif */
      }
      else
      {
        lLength = usTokLen;
      } /* endif */
    }
    else
    {
      lLength = usTokLen;
    } /* endif */

    lLength = min( lLength, MAX_TERM_LEN-1 );           /* Pr70 */

    return (lLength);
}  /* end of ITMGetRelLength */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMDelUnsureAnchor
//------------------------------------------------------------------------------
// Function call:     ITMDelUnsureAnchor(PITMNOPSEGS, PITMNOPSEGS)
//------------------------------------------------------------------------------
// Description:       delete anchors which are not definite
//------------------------------------------------------------------------------
// Parameters:        PITMNOPSEGS   pSrcNop
//                    PITMNOPSEGS   pTgtNop
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     while not at end of src nop list
//                      find next src segnum which is anchor
//                      find tgt segnum of that anchor
//                      if previous src segnum is not eq.to current
//                        and is not anchored:
//                        if tokendata is equal to current tokendata,
//                          delete anchor  (ITMDelAnchorIfEqual)
//                      else if next srcsegnum is not eq. to current srcsegnum
//                         and is not anchored:
//                         ITMDelAnchorIfEqual
//                      else if prev tgtsegnum is not eq. to current tgtsegnum
//                          and is not anchored:
//                          ITMDelAnchorIfEqual
//                      else if next tgtsegnum is not eq. to current tgtsegnum
//                          and is not anchored:
//                          ITMDelAnchorIfEqual
//------------------------------------------------------------------------------
// Example:      case       srcseg tokendata anchored with   tokendata tgtseg
//                 1          1      AIX                                  3
//                            2      AIX     ------>          AIX         4
//               _____________________________________________________________
//                 2          1      AIX     ------->         AIX         3
//                            2      AIX                                  4
//               _____________________________________________________________
//                 3          1      AIX     ------->         AIX         3
//                            2                               AIX         4
//               _____________________________________________________________
//                 4          1                               AIX         3
//                            2      AIX    ---------->       AIX         4
//               _____________________________________________________________
//     case 1: Srcsegnum 1 can become anchor with tgtsegnum 4, but also
//             srcsegnum 2 can become anchor with tgtsegnum 4.
//             Because we do not know, which is correct, the anchor is
//             deleted.
//     The other cases can be explained analogues.
//------------------------------------------------------------------------------

static
BOOL ITMDelUnsureAnchor
(
  PITMNOPSEGS   pSrcNop,
  PITMNOPSEGS   pTgtNop
)
{
  ULONG     k = 1;
  ULONG     ulJ = 0;
  BOOL      fFound;
  BOOL      fAnchorDel;
  ULONG     ulSrcSegnum, ulTgtSegnum;
  FUZZYTOK  HUGE *pActToken;
  BOOL      fRcDel = FALSE;
  BOOL      fSegEqual;
  ULONG     ulIndex;

  for ( k=0;k<pSrcNop->ulUsed ;k++ )
  {
    if ( pSrcNop->pTokenList[k].sType == MARK_EQUAL)
    {
      /****************************************************************/
      /* find tgttoken which is anchor with this srctoken             */
      /****************************************************************/
      fFound = FALSE;
      while ( !fFound && (ulJ < pTgtNop->ulUsed))
      {
        if ( pTgtNop->pTokenList[ulJ].sType == MARK_EQUAL )
        {
          fFound = TRUE;
        } /* endif */
        ulJ++;                 // point to next token
      } /* endwhile */
      if ( fFound )
      {
        ulSrcSegnum = pSrcNop->pulSegs[k];
        ulTgtSegnum = pTgtNop->pulSegs[ulJ-1];
        pActToken = pSrcNop->pTokenList + k;
        fAnchorDel = FALSE;
        /**************************************************************/
        /* skip next/last token while it is not connected and         */
        /* of the same segment;                                       */
        /* check if last or next src token are equal to current token */
        /* if so, the anchor is not sure and is deleted               */
        /**************************************************************/
        fSegEqual = TRUE;
        ulIndex = k;
        while ( (ulIndex > 1) && fSegEqual &&
             (pSrcNop->pTokenList[ulIndex-1].sType != MARK_EQUAL ) )
        {
          if ( ulSrcSegnum != (pSrcNop->pulSegs[ulIndex-1]) )
          {
            fSegEqual = FALSE;
            fAnchorDel = ITMDelAnchorIfEqual( pActToken,
                                        pSrcNop->pTokenList + ulIndex - 1,
                                        pTgtNop->pTokenList + ulJ - 1);
          } /* endif */
          ulIndex--;
        } /* endif */
        fSegEqual = TRUE;
        ulIndex = k+1;
        while ( !fAnchorDel && fSegEqual &&
                   ((ulIndex) < pSrcNop->ulUsed) &&
                   (pSrcNop->pTokenList[ulIndex].sType != MARK_EQUAL) )
        {
          if ( ulSrcSegnum != (pSrcNop->pulSegs[ulIndex]) )
          {
            fSegEqual = FALSE;
            fAnchorDel = ITMDelAnchorIfEqual( pActToken,
                                        pSrcNop->pTokenList + ulIndex,
                                        pTgtNop->pTokenList + ulJ - 1);
          } /* endif */
          ulIndex++;
        } /* endwhile */
        /**************************************************************/
        /* check if last or next tgt token are equal to current token */
        /* if so, the anchor is not sure and is deleted               */
        /**************************************************************/
        fSegEqual = TRUE;
        ulIndex = ulJ;
        while ( !fAnchorDel && fSegEqual && (ulIndex > 2) &&
                   (pTgtNop->pTokenList[ulIndex-2].sType != MARK_EQUAL) )
        {
          if ( ulTgtSegnum != (pTgtNop->pulSegs[ulIndex-2]) )
          {
            fSegEqual = FALSE;
            fAnchorDel = ITMDelAnchorIfEqual( pActToken,
                                      pTgtNop->pTokenList + ulIndex -2,
                                      pTgtNop->pTokenList + ulJ - 1);
          } /* endif */
          ulIndex--;
        } /* endif */
        fSegEqual = TRUE;
        ulIndex = ulJ;
        while ( !fAnchorDel && fSegEqual && (ulIndex < pTgtNop->ulUsed) &&
                   (pTgtNop->pTokenList[ulIndex].sType != MARK_EQUAL) )
        {
          if ( ulTgtSegnum != (pTgtNop->pulSegs[ulIndex]) )
          {
            fSegEqual = FALSE;
            fAnchorDel = ITMDelAnchorIfEqual( pActToken,
                                           pTgtNop->pTokenList + ulIndex,
                                           pTgtNop->pTokenList + ulJ - 1);
          } /* endif */
          ulIndex++;
        } /* endif */
        if ( fAnchorDel )
        {
          fRcDel = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endfor */
  return(fRcDel);
} /* end of function static ITMDelUnsureAnchor */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMDelAnchorIfEqual
//------------------------------------------------------------------------------
// Function call:     ITMDelAnchorIfEqual(pActToken, pTestToken,
//                                        pTgtToken)
//------------------------------------------------------------------------------
// Description:       delete anchor if it is not definitely correct
//------------------------------------------------------------------------------
// Parameters:        #ifdef _TKT21
//                      FUZZYTOK *pActToken,
//                      FUZZYTOK *pTestToken,
//                      FUZZYTOK *pTgtToken,
//                    #else
//                      FUZZYTOK huge *  pActToken,
//                      FUZZYTOK huge *  pTestToken,
//                      FUZZYTOK huge *  pTgtToken,
//                    #endif
//------------------------------------------------------------------------------
// Returncode type:     VOID
//------------------------------------------------------------------------------
// Function flow:      if tokendata of testtoken and acttoken is equal
//                       delete anchor
//------------------------------------------------------------------------------
static BOOL
ITMDelAnchorIfEqual
(
  FUZZYTOK HUGE *pActToken,
  FUZZYTOK HUGE *pTestToken,
  FUZZYTOK HUGE *pTgtToken
)
{
  BOOL   fDel = FALSE;                   // anchor not deleted

    if ((pTestToken->ulHash == pActToken->ulHash) &&
         ItmLenComp(pTestToken->pData, pActToken->pData,
               (USHORT)(pTestToken->usStop - pTestToken->usStart + 1),
               (USHORT)(pActToken->usStop - pActToken->usStart + 1)) )
    {
      /**********************************************************/
      /* delete anchor src k with tgt usJ-1                     */
      /**********************************************************/
      pActToken->sType = MARK_DELETED;
      pTgtToken->sType = MARK_DELETED;
      fDel = TRUE;
    } /* endif */
   return(fDel);
} /* end of function ITMDelAnchorIfEqual */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMDelDupPerSeg
//------------------------------------------------------------------------------
// Function call:     ITMDelDupPerSeg(pITMNopSeg)
//------------------------------------------------------------------------------
// Description:       accept only one anchor candidate per segment
//------------------------------------------------------------------------------
// Parameters:        PITMNOPSEGS   pITMNopSeg
//------------------------------------------------------------------------------
// Returncode type:     VOID
//------------------------------------------------------------------------------
// Function flow:       while not at end of anchor candidate list
//                       while segment number of next candidate is equal
//                           to the current candidate, skip it
//                       if not at end ( and another segment number )
//                         copy next candidate in the list
//                      endwhile
//------------------------------------------------------------------------------
static VOID
ITMDelDupPerSeg
(
   PITMNOPSEGS    pITMNopSegs
)
{

  ULONG    ulFrom = 0;                     // index for dupl.checking
  ULONG    ulTo = 0;
  ULONG    ulToSegNum = 0;


  /********************************************************************/
  /* if more than one token per segment in list, delete the following */
  /* tokens of this segment                                           */
  /********************************************************************/
  while ( ulFrom < pITMNopSegs->ulUsed )
  {
    ulFrom++;
    ulToSegNum = pITMNopSegs->pulSegs[ulTo];
    while ( (ulFrom < pITMNopSegs->ulUsed ) &&
            (pITMNopSegs->pulSegs[ulFrom] == ulToSegNum ))
    {
      ulFrom++;
    } /* endwhile */
    if ( ulFrom < pITMNopSegs->ulUsed )
    {
      /****************************************************************/
      /* copy token from UsFrom -position to usTo position            */
      /****************************************************************/
      ulTo++;
      pITMNopSegs->pTokenList[ulTo] = pITMNopSegs->pTokenList[ulFrom];
      pITMNopSegs->pulSegs[ulTo] = pITMNopSegs->pulSegs[ulFrom];
    } /* endif */
  } /* endwhile */
  pITMNopSegs->ulUsed = ulTo+1;

  return;
} /* end of function ITMDelDupPerSeg */

__declspec(dllexport)
USHORT MemFuncCreateITM
(
    PSZ  pszMemName,
    PSZ  pszFilePairs,
    PSZ  pszMarkup,
    PSZ  pszSGMLMemFile,
    PSZ  pszSourceLanguage,
    PSZ  pszTargetLanguage,
    PSZ  pszSourceStartPath,
    PSZ  pszTargetStartPath,
    LONG lType
)
{

    USHORT   usRC = NO_ERROR;            // function return code
    BOOL     fOk = 1;
    PITMIDA  pITMIda = NULL;             // pointer to ida
    PSZ      pszParm;                    // pointer for error parameters
    PSZ      *ppListIndex = NULL;
    USHORT   usI = 0;
    CHAR     szDrive[ MAX_DRIVE ];       // variable for getting system drive
    CHAR     szEqfDrives[MAX_DRIVELIST]; // buffer for EQF drive letters
    USHORT   usNextTask;
    ULONG    ulAddParam;
    CHAR     szMemName[MAX_LONGFILESPEC];
    PSZ      pszFileList = NULL;


    // check required parameters
    if ( usRC == NO_ERROR )
    {
        if ( (pszMemName == NULL) || (*pszMemName == EOS) )
        {
            usRC = TMT_MANDCMDLINE;
            UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
    } /* endif */

   // Check memory name syntax
    if ( usRC == NO_ERROR )
    {
      if ( !UtlCheckLongName( pszMemName ) )
      {
        pszParm = pszMemName;
        UtlErrorHwnd(  ERROR_INV_LONGNAME, MB_CANCEL, 1,
                       &pszParm, EQF_ERROR, HWND_FUNCIF );
        usRC = ERROR_MEM_NAME_INVALID;
      } /* endif */
    } /* endif */
    if ( usRC == NO_ERROR )
    {
        strcpy(szMemName,pszMemName);
        if ( !UtlCheckIfExist( szMemName, TM_OBJECT ) )
        {
            PSZ pszParm = pszMemName;
            fOk = FALSE;
            usRC = ERROR_MEMORY_NOTFOUND;
            UtlErrorHwnd( usRC, MB_CANCEL, 1,
                          &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
    }

    // check if file list is valid
    if ( usRC == NO_ERROR )
    {
        if ( pszFilePairs == NULL || *pszFilePairs == EOS )
        {
            usRC = FUNC_MANDFILES;
            UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
        }
    }

    //  check if a markup name has been specified
    if ( usRC == NO_ERROR )
    {
      if ( (pszMarkup == NULL) || (*pszMarkup == EOS) )
      {
        usRC = ERROR_MUPROP_NO_TABLE_MAME;
        UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */

    //  check if markup name is valid
    if ( usRC == NO_ERROR )
    {
        if ( !UtlCheckIfExist( pszMarkup, MARKUP_OBJECT ) )
        {
          PSZ pszParm = pszMarkup;
          fOk = FALSE;
          usRC = ERROR_NO_FORMAT_TABLE_AVA;
          UtlErrorHwnd( usRC, MB_CANCEL, 1,
                        &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
    } /* endif */

    // check if source language has been specified
    if ( usRC == NO_ERROR )
    {
      if ( (pszSourceLanguage == NULL) || (*pszSourceLanguage == EOS) )
      {
        usRC = ERROR_NO_SOURCELANG;
        UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    } /* endif */

    // check if source language is valid
    if ( usRC == NO_ERROR )
    {
      LanguageFactory *pLangFactory = LanguageFactory::getInstance();
      if ( !pLangFactory->isSourceLanguage( pszSourceLanguage ) )
      {
        usRC = ERROR_PROPERTY_LANG_DATA;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszSourceLanguage, EQF_ERROR, HWND_FUNCIF  );
      } /* endif */
    } /* endif */

    // check if target language has been specified
    if ( usRC == NO_ERROR )
    {
        if ( (pszTargetLanguage == NULL) || (*pszTargetLanguage == EOS) )
        {
            usRC = ERROR_NO_TARGETLANG;
            UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
    } /* endif */

    // check if target language is valid
    if ( usRC == NO_ERROR )
    {
        LanguageFactory *pLangFactory = LanguageFactory::getInstance();
        if ( !pLangFactory->isTargetLanguage( pszTargetLanguage ) )
        {
          usRC = ERROR_PROPERTY_LANG_DATA;
          UtlErrorHwnd( ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,
                        &pszTargetLanguage, EQF_ERROR, HWND_FUNCIF  );
        } /* endif */
    } /* endif */

    // allocate storage for create IDA
    if ( usRC == NO_ERROR  )
    {
        if ( !UtlAllocHwnd( (PVOID *)&pITMIda, 0L, (LONG)sizeof(ITMIDA),
                            ERROR_STORAGE, HWND_FUNCIF ) )
        {
            usRC = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */
    } /* endif */

    //fill IDA values

    /*************************************************************/
    /* Check input parameters                                    */
    /* read in property file                                  */
    /* if error during PropRead, goon nevertheless, donot close ITM */
    /*************************************************************/


    if ( usRC == NO_ERROR )
    {
        pITMIda->hwnd = HWND_FUNCIF;
        pITMIda->fCurDispStatus = DISP_ALIGNING;
        pITMIda->fVisual = FALSE;
        pITMIda->fNoAna = lType & NOANA_TYP;
        pITMIda->fNoTMDB = lType & NOTM_TYP;
        pITMIda->fPrepare = lType & PREPARE_TYP;
        pITMIda->fNoConfirm = TRUE;
        pITMIda->fTimer = FALSE;

        //usRC =
        EQFITMPropRead(pITMIda);
    }



    if( usRC == NO_ERROR )
    {
        fOk = UtlCopyParameter(pITMIda->chTagTableName,
                               pszMarkup,
                               MAX_EQF_PATH,
                               TRUE);
        if ( fOk ) fOk = UtlCopyParameter(pITMIda->szSourceLang,
                               pszSourceLanguage,
                               sizeof(pITMIda->szSourceLang),
                               TRUE);
        if ( fOk ) fOk = UtlCopyParameter(pITMIda->szTargetLang,
                               pszTargetLanguage,
                               sizeof(pITMIda->szTargetLang),
                               TRUE);
        if ( fOk ) fOk = UtlCopyParameter(pITMIda->szTargetInputLang,
                               pszTargetLanguage,
                               sizeof(pITMIda->szTargetInputLang),
                               TRUE);
        if ( fOk ) fOk = UtlCopyParameter(pITMIda->chTranslMemory,
                               pszMemName,
                               sizeof(pITMIda->chTranslMemory),
                               TRUE);
        if (fOk)
        {
            PSZ pszTempFname = NULL;
            strcpy( pITMIda->chLongTranslMemory, pITMIda->chTranslMemory );
            pszTempFname = UtlGetFnameFromPath(pITMIda->chTranslMemory);
            if ( pszTempFname )
            {
                strcpy(pITMIda->chTransMemFname, pszTempFname);
            }
        }

        if( fOk && pszSGMLMemFile )
        {
            fOk = UtlCopyParameter(pITMIda->chSGMLMem,
                               pszSGMLMemFile,
                               sizeof(pITMIda->chSGMLMem)/sizeof(pITMIda->chSGMLMem[0]),//MAX_EQF_PATH,
                               TRUE);
            pITMIda->fSGMLITM = TRUE;
        }

        if(fOk && pszSourceStartPath && pszTargetStartPath )
        {
            fOk = UtlCopyParameter(pITMIda->szSrcStartPath,
                                 pszSourceStartPath,
                                 sizeof(pITMIda->szSrcStartPath)-1,
                                 TRUE);
            if ( fOk )
            {
              fOk = UtlCopyParameter(pITMIda->szTgtStartPath,
                               pszTargetStartPath,
                               sizeof(pITMIda->szTgtStartPath)-1,
                               TRUE);
            } /* endif */

        }
        else
        {
            pITMIda->szSrcStartPath[0] = EOS;
            pITMIda->szTgtStartPath[0] = EOS;
        }
        if ( !fOk ) usRC = ERROR_INTERNAL_PARM;
    }

     /**************************************************/
    /* set parameters and check that a even number is */
    /* available, else somewhere a correspondent part */
    /* of a file is missing                           */
    /**************************************************/
    if ( usRC == NO_ERROR )
    {
        // load list of files if a list file indicator has been used or copy file list to a buffer
        if ( *pszFilePairs == LISTINDICATOR )
        {
          // load list of files
          FILE *hfList = NULL;

          hfList = fopen( pszFilePairs + 1, "rb" );
          if ( hfList != NULL )
          {
            int iLen = _filelength( fileno(hfList) ) + 10;

            if ( UtlAllocHwnd( (PVOID *)&pszFileList, 0, iLen+10, ERROR_STORAGE, HWND_FUNCIF) )
            {
              fread( pszFileList, 1, iLen, hfList );
            }
            else
            {
              usRC = ERROR_NOT_ENOUGH_MEMORY;
            } /* endif */
            fclose( hfList );
          }
          else
          {
            PSZ pszParm = pszFilePairs + 1;
            usRC = ERROR_LIST_FILE_ACCESS;
            UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
          } /* endif */
        }
        else
        {
          UtlAlloc( (PVOID *)&pszFileList, 0L, strlen(pszFilePairs), NOMSG );
          sprintf(pszFileList,"(%s)\0",pszFilePairs);
        }
        
        if ( usRC == NO_ERROR  )
        {
          /**************************************************/
          /* work against pair of files                     */
          /**************************************************/
          fOk = UtlValidateList( pszFileList, &ppListIndex, 2000 );
          if ( !fOk )
          {
            PSZ pszParm = pszFilePairs + 1;
            usRC = DDE_ERRFILELIST;
            UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
          }
        }

        if ( usRC == NO_ERROR  )
        {
          fOk = CheckDoubleFilePairs (&ppListIndex);
          if ( fOk )
          {
            while ( ppListIndex[usI] )
            {
                usI++;
            } /* endwhile */
            if ( (usI % 2) )
            {
                usRC = ITM_NUMOFFILES;
                UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
                UtlAlloc( (PVOID *)&ppListIndex, 0L, 0L, NOMSG );
            } else
            {
                pITMIda->ppArgv = ppListIndex;
                pITMIda->usArgc = usI;
                fOk = FillPropFilePairs(pITMIda);
                if ( fOk )
                {
                  pITMIda->pstPropItm->szSrcDirectory[0] = EOS;
                  pITMIda->pstPropItm->szTgtDirectory[0] = EOS;
                } /* endif */
            } /* endif */
          }
          else
          {
            usRC = ITM_NUMOFFILES;
            UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
            UtlAlloc( (PVOID *)&ppListIndex, 0L, 0L, NOMSG );
          }

        }
    } // endif usRC

    if ( usRC == NO_ERROR )
    {
        fOk = EQFITMPropFill(pITMIda);
        strcpy(pITMIda->chQFTagTable, ITM_QFTAGTABLE);
        fOk = UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
        pITMIda->chQFTagTable[0] = szDrive[0];
       /* Get valid EQF drives                                           */
        fOk = UtlGetCheckedEqfDrives( szEqfDrives );

        usNextTask = ITM_PROCESS_TMOPEN;
        while ( usNextTask != ITM_PROCESS_END )
        {
            EQFITMProcess(NULL,pITMIda,&usNextTask,&ulAddParam);
            if ( pITMIda->usRC != NO_ERROR && pITMIda->usRC != ITM_COMPLETE)
      {
         usRC = pITMIda->usRC;
            }
        }
        ITMCloseProcessing( pITMIda );
    }
    return ( usRC );
}

