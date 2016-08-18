/*! EQFEVENT.H
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

#ifndef EVENTLOG
  #ifndef _DEBUG
    #define NO_DEBUGEVENTS
    #define NO_ERREVENTS
    #define NO_INFOEVENTS
  #endif
#endif

/**********************************************************************/
/* Name of event log file                                             */
/**********************************************************************/
#define EVENTLOGFILENAME   "EQFEVENT.LOG"

/**********************************************************************/
/* Max number of events in event lists                                */
/**********************************************************************/
#define MAX_EVENT_ENTRIES   4096
#define MAX_EVENT2_ENTRIES  4500

#undef EVENTDEF
#undef EVENTLISTSTART
#undef EVENTLISTEND

/**********************************************************************/
/* Structure for event string tables                                  */
/**********************************************************************/
#ifdef STRING_EVENT_IDS
typedef struct _EVENTSTRING
{
  char *pszEvent;                      // string value
  int iNum;                            // ID for string value
} EVENTSTRING, *PEVENTSTRING;
#endif

/**********************************************************************/
/* Macro to either define the event ID as a numeric value or to       */
/* use the event ID as a string (used by EVENTLOG program).           */
/**********************************************************************/
#ifdef STRING_EVENT_IDS
   #define EVENTDEF( id, num ) { #id, num },
#else
   #define EVENTDEF( id, num ) id = num,
#endif
/**********************************************************************/
/* Defines the header of the event list either as a structure or as   */
/* a enumeration                                                      */
/**********************************************************************/
#ifdef STRING_EVENT_IDS
   #define EVENTLISTSTART( type, var ) \
     EVENTSTRING var[] = {
#else
   #define EVENTLISTSTART( type, var) typedef enum type {
#endif

#ifdef STRING_EVENT_IDS
   #define EVENTLISTEND( type, dummy ) { #dummy, 32000 } };
#else
   #define EVENTLISTEND( type, dummy ) dummy = 32000} type;
#endif

/**********************************************************************/
/* List of event locations (only use macro EVENTDEF for entries in        */
/* this list)                                                         */
/**********************************************************************/
EVENTLISTSTART( _EVENTLOC, EventLocations )
EVENTDEF( UTLERROR_LOC,              1 )
EVENTDEF( TWBSTART_LOC,              2 )
EVENTDEF( TWBSHUTDOWN_LOC,           3 )
EVENTDEF( QDAMUPDATELOCKREC_LOC,     4 )
EVENTDEF( QDAMGETUPDCOUNTER_LOC,     5 )
EVENTDEF( QDAMINCRUPDCOUNTER_LOC,    6 )
EVENTDEF( QDAMCHECKFORUPDATES_LOC,   7 )
EVENTDEF( QDAMPHYSLOCK_LOC,          8 )
EVENTDEF( QDAMRECORDTODISK_LOC,      9 )
EVENTDEF( QDAMDICTCREATELOCAL_LOC,   10 )
EVENTDEF( QDAMREADRECORD_LOC,        12 )
EVENTDEF( QDAMWRITEHEADER_LOC,       13 )
EVENTDEF( QDAMDICTOPENLOCAL_LOC,     14 )
EVENTDEF( QDAMDICTCLOSELOCAL_LOC,    15 )
EVENTDEF( QDAMDICTEXACTLOCAL_LOC,    16 )
EVENTDEF( QDAMDICTSUBSTRLOCAL_LOC,   17 )
EVENTDEF( QDAMDICTEQUIVLOCAL_LOC,    18 )
EVENTDEF( QDAMDICTFIRSTLOCAL_LOC,    19 )
EVENTDEF( QDAMDICTNEXTLOCAL_LOC,     20 )
EVENTDEF( QDAMDICTPREVLOCAL_LOC,     21 )
EVENTDEF( QDAMDICTCURRENTLOCAL_LOC,  22 )
EVENTDEF( QDAMNEXT_LOC,              23 )
EVENTDEF( QDAMFIRST_LOC,             24 )
EVENTDEF( QDAMGETSZKEYPARAM_LOC,     25 )
EVENTDEF( QDAMGETSZDATA_LOC,         26 )
EVENTDEF( QDAMLOCATEKEY_LOC,         27 )
EVENTDEF( QDAMVALIDATEINDEX_LOC,     28 )
EVENTDEF( NLPBACKWARDLIST_LOC,       29  )
EVENTDEF( NLPFORWARDDLIST_LOC,       30 )
EVENTDEF( NLPTERMLIST_LOC,           31 )
EVENTDEF( NLPLOCKENTRY_LOC,          32 )
EVENTDEF( NLPDELENTRYASD_LOC,        33 )
EVENTDEF( NLPUPDENTRYASD_LOC,        34 )
EVENTDEF( NLPINSENTRYASD_LOC,        35 )
EVENTDEF( NLPPRVTERMASD_LOC,         36 )
EVENTDEF( NLPNXTTERMASD_LOC,         37 )
EVENTDEF( NLPFNDNUMBERASD_LOC,       38 )
EVENTDEF( NLPRETENTRYASD_LOC,        39 )
EVENTDEF( NLPFNDMATCHASD_LOC,        40 )
EVENTDEF( NLPFNDEQUIVASD_LOC,        41 )
EVENTDEF( NLPFNDBEGINASD_LOC,        42 )
EVENTDEF( QDAMOPENASD_LOC,           43 )
EVENTDEF( QDAMWRITERECORD_LOC,       44 )
EVENTDEF( QDAMNEWRECORD_LOC,         45 )
EVENTDEF( QDAMFREERECORD_LOC,        46 )
EVENTDEF( QDAMUPDSIGNLOCAL_LOC,      47 )
EVENTDEF( QDAMDICTFLUSHLOCAL_LOC,    48 )
EVENTDEF( QDAMSPLITNODE_LOC,         49 )
EVENTDEF( QDAMINSERTKEY_LOC,         50 )
EVENTDEF( QDAMDICTINSERTLOCAL_LOC,   51 )
EVENTDEF( QDAMCHANGEKEY_LOC,         52 )
EVENTDEF( QDAMADDTOBUFFER_LOC,       53 )
EVENTDEF( QDAMDELETEDATAFROMBUFFER_LOC, 54 )
EVENTDEF( QDAMDICTUPDATELOCAL_LOC,   55 )
EVENTDEF( QDAMDICTCOPYLOCAL_LOC,     56 )
EVENTDEF( QDAMREDUCENODE_LOC,        57 )
EVENTDEF( QDAMDICTDELETELOCAL_LOC,   58 )
EVENTDEF( QDAMREADRECORDFROMDISK_LOC,59 )
EVENTDEF( EQFNTMUPDATE_LOC,          60 )
EVENTDEF( EQFNTMINSERT_LOC,          61 )
EVENTDEF( EQFNTMGET_LOC,             62 )
EVENTDEF( EQFNTMPHYSLOCK_LOC,        63 )
EVENTDEF( TMTXREPLACE_LOC,           64 )
EVENTDEF( TOKENIZESOURCE_LOC,        65 )
EVENTDEF( TOKENIZETARGET_LOC,        66 )
EVENTDEF( ADDTOTM_LOC,               67 )
EVENTDEF( FILLCLB_LOC,               68 )
EVENTDEF( DETERMINETMRECORD_LOC,     69 )
EVENTDEF( UPDATETMRECORD_LOC,        70 )
EVENTDEF( COMPAREPUTDATA_LOC,        71 )
EVENTDEF( ADDTMTARGET_LOC,           72 )
EVENTDEF( REPLACETMTARGET_LOC,       73 )
EVENTDEF( TMTXGET_LOC,               74 )
EVENTDEF( GETEXACTMATCH_LOC,         75 )
EVENTDEF( EXACTTEST_LOC,             76 )
EVENTDEF( FILLMATCHTABLE_LOC,        77 )
EVENTDEF( GETFUZZYMATCH_LOC,         78 )
EVENTDEF( FUZZYTEST_LOC,             79 )
EVENTDEF( FILLMATCHENTRY_LOC,        80 )
EVENTDEF( NTMREADWRITESEGMENT_LOC,   81 )
EVENTDEF( NTMLOCKTM_LOC,             82 )
EVENTDEF( NTMCHECKFORUPDATES_LOC,    83 )
EVENTDEF( TMOPEN_LOC,                84 )
EVENTDEF( TMCLOSE_LOC,               85 )
EVENTDEF( TMREPLACE_LOC,             86 )
EVENTDEF( TMGET_LOC,                 87 )
EVENTDEF( TMTXOPEN_LOC,              88 )
EVENTDEF( EQFNTMOPEN_LOC,            89 )
EVENTDEF( EQFNTMCLOSE_LOC,           90 )
EVENTDEF( EQFNTMSIGN_LOC,            91 )
EVENTDEF( UTILSEXIT_LOC,             92 )
EVENTDEF( QDAMLOCSUBSTR_LOC,         93 )
EVENTDEF( QDAMGETSZKEY_LOC,          94 )
EVENTDEF( QDAMCHECKCHECKSUM_LOC,     95 )
EVENTDEF( QDAMFINDRECORD_LOC,        96 )
EVENTDEF( QDAMSETRECDATA_LOC,        97 )
EVENTDEF( EQFDA_LOC,                 98 )
EVENTDEF( DICTCLEANUP_LOC,           99 )
EVENTDEF( EQFDACLOSE_LOC,           100 )
EVENTDEF( ASDOPEN_LOC,              101 )
EVENTDEF( ASDCLOSE_LOC,             102 )
EVENTDEF( ASDTRANSLATE_LOC,         103 )
EVENTDEF( TMUPDSEG_LOC,             104 )
EVENTDEF( DOCEXPCOMMAND_LOC,        105 )
EVENTDEF( DOCUMENTUNLOAD_LOC,       106 )
EVENTDEF( UNLOADDOCUMENT_LOC,       107 )
EVENTDEF( EQFUNSEGREVMARK_LOC,      108 )
EVENTDEF( UNSEGMENT_LOC,            109 )
EVENTDEF( ASDLISTINDEX_LOC,         110 )
EVENTDEF( NTMGETIDFROMNAME_LOC,     111 )
EVENTDEF( TWBEXCEPTION_LOC,         112 )
EVENTLISTEND( TEVENTLOC, DUMMY_LOC )

#ifdef DEF_EVENT_ENUMS
TEVENTLOC EVENTLOC;
#else
extern TEVENTLOC EVENTLOC;
#endif

/**********************************************************************/
/* List of event classes                                              */
/**********************************************************************/
EVENTLISTSTART( _EVENTCLASS, EventClasses )
EVENTDEF( NA_EVENT,             1 )
EVENTDEF( FUNCENTRY_EVENT,      2 )
EVENTDEF( REFRESH_EVENT,        5 )
EVENTDEF( INTFUNCFAILED_EVENT,  6 )
EVENTDEF( MSGBOX_EVENT,         7 )
EVENTDEF( INVOPERATION_EVENT,   8 )
EVENTDEF( READ_EVENT,           9 )
EVENTDEF( WAIT_EVENT,           10 )
EVENTDEF( STATE_EVENT,          11 )
EVENTDEF( FUNCEXIT_EVENT,       12 )
EVENTDEF( CHECKSUM_EVENT,       13 )
EVENTLISTEND( TEVENTCLASS, DUMMY_CLASS )

#ifdef DEF_EVENT_ENUMS
TEVENTCLASS EVENTCLASS;
#else
extern TEVENTCLASS EVENTCLASS;
#endif

/**********************************************************************/
/* Group/Area of events                                               */
/**********************************************************************/
EVENTLISTSTART( _EVENTGROUP, EventGroups )
EVENTDEF( NONE_GROUP,           0 )
EVENTDEF( TM_GROUP,             1 )
EVENTDEF( DICT_GROUP,           2 )
EVENTDEF( ANALYSIS_GROUP,       3 )
EVENTDEF( MORPH_GROUP,          4 )
EVENTDEF( DOC_GROUP,            5 )
EVENTDEF( GENERAL_GROUP,        6 )
EVENTDEF( DB_GROUP,             7 )
EVENTLISTEND( TEVENTGROUP, DUMMY_GROUP )

#ifdef DEF_EVENT_ENUMS
TEVENTGROUP EVENTGROUP;
#else
extern TEVENTGROUP EVENTGROUP;
#endif

/**********************************************************************/
/* Event structure as used in older event lists                       */
/*                                                                    */
/* Note: The fields of the first entry in the list are used in a      */
/*       different way:                                               */
/*       sEventID = number of first event in list                     */
/*       sClass   = number of last event in list                      */
/*       sRC      = total size of list in number of events            */
/**********************************************************************/
#ifndef STRING_EVENT_IDS

// record type identifier
typedef struct _EVENTENTRY
{
  LONG        lTime;                   // date and time of event
  SHORT       sType;                   // type of event
  SHORT       sEventId;                // ID of event
  SHORT       sClass;                  // class or type of event
  SHORT       sRC;                     // event return code
} EVENTENTRY, *PEVENTENTRY;

/**********************************************************************/
/* New event structure                                                */
/* For events of a record type of EVENTRECEXTENDED the subsequent     */
/* event record contains the string data for this event (the record   */
/* type of this record is EVENTRECSTRING                              */
/**********************************************************************/

// record type identifier
#define EVENTRECNORMAL   1             // self-contained event record
#define EVENTRECEXTENDED 2             // next record contains string for event
#define EVENTRECSTRING   3             // string data for EVENTRECEXTENDED record

typedef struct _EVENTENTRY2
{
  CHAR        cRecType;                // type of this event record
  CHAR        cType;                   // type of event
  SHORT       sClass;                  // class or type of event
  SHORT       sGroup;                  // group of this event (e.g DICT, TM, MORPH)
  LONG        lTime;                   // date and time of event
  SHORT       sEventId;                // ID of event (location)
  SHORT       sRC;                     // event return code or other info
} EVENTENTRY2, *PEVENTENTRY2;

/**********************************************************************/
/* Format of event table                                              */
/**********************************************************************/
#define EVENTMAGICWORD "EVN"           // event table identifier
#define EVENTTABLEVERSION  1           // current event table version

typedef struct _EVENTTABLE
{
  CHAR   szMagicWord[4];               // identifier for this type of event table
  SHORT  sVersion;                     // version of event table
  SHORT  sFirstEvent;                  // number of first event in list
  SHORT  sLastEvent;                   // number of last event in list
  SHORT  sListSize;                    // total size of list in number of events
  EVENTENTRY2 Event[MAX_EVENT2_ENTRIES]; // table with event entries
} EVENTTABLE, *PEVENTTABLE;

// Macros to access the elements of the first entry in the list
#define LASTEVENTINLIST( plist ) plist[0].sClass
#define FIRSTEVENTINLIST( plist ) plist[0].sEventId
#define EVENTLISTSIZE( plist ) plist[0].sRC
#endif

/**********************************************************************/
/* Event types                                                        */
/**********************************************************************/
#ifndef STRING_EVENT_IDS
typedef enum
{
  ERROR_EVENT,                         // event is caused by an error condition
  INFO_EVENT,                          // informational events (e.g. status events)
  EQFDEBUG_EVENT                       // only logged when debug version is build
} EVENTTYPE;
#endif

/**********************************************************************/
/* Event macros                                                       */
/**********************************************************************/
#ifndef STRING_EVENT_IDS
  #ifndef NO_ERREVENTS
    #define ERREVENT( id, class, rc ) \
      UtlEvent( ERROR_EVENT, id, class, rc )
  #else
    #define ERREVENT( id, class, rc )
  #endif

  #ifndef NO_ERREVENTS
    #define ERREVENT2( id, class, rc, gr, str ) \
      UtlEvent2( ERROR_EVENT, id, class, rc, gr, str )
  #else
    #define ERREVENT2( id, class, rc, gr, str )
  #endif

  #ifndef NO_INFOEVENTS
    #define INFOEVENT( id, class, rc ) \
      UtlEvent( INFO_EVENT, id, class, rc )
  #else
    #define INFOEVENT( id, class, rc )
  #endif

  #ifndef NO_INFOEVENTS
    #define INFOEVENT2( id, class, rc, gr, str ) \
      UtlEvent2( INFO_EVENT, id, class, rc, gr, str )
  #else
    #define INFOEVENT2( id, class, rc, gr, str )
  #endif

  #ifndef _DEBUG
    #define NO_DEBUGEVENTS
  #endif

  #ifndef NO_DEBUGEVENTS
    #define DEBUGEVENT( id, class, rc ) \
      UtlEvent( EQFDEBUG_EVENT, id, class, rc )
  #else
    #define DEBUGEVENT( id, class, rc )
  #endif

  #ifndef NO_DEBUGEVENTS
    #define DEBUGEVENT2( id, class, rc, gr, str ) \
      UtlEvent2( EQFDEBUG_EVENT, id, class, rc, gr, str )
  #else
    #define DEBUGEVENT2( id, class, rc, gr, str )
  #endif
#endif

/**********************************************************************/
/* Event recording function (do not call directly, use macros instead)*/
/**********************************************************************/
__declspec(dllexport)
BOOL UtlEvent
(
  SHORT sEventType,                    // type of the event
  SHORT sEventID,                      // the event identifier
  SHORT sEventClass,                   // the event class
  SHORT sEventRC                       // the event return code or error code
);
__declspec(dllexport)
BOOL UtlEvent2
(
  SHORT sEventType,                    // type of the event
  SHORT sEventID,                      // the event identifier
  SHORT sEventClass,                   // the event class
  SHORT sEventRC,                      // the event return code or error code
  SHORT sGroup,                        // the event group
  PSZ   pszAddData                     // additional data for event (string)
);

__declspec(dllexport)
BOOL UtlAddEventGroup
(
  SHORT sGroup                       // the event group
);

