/*! EQF Startup Utility functions header file
	Description:       This is header file contains the function prototypes
					and type definitions used by the EQFDRVEX.C module.
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

/*****************************************************************************
 * function prototypes of internal functions                                 *
 *****************************************************************************/

INT_PTR CALLBACK TWBDrivesDlgProc ( HWND, WINMSG, WPARAM, LPARAM );
MRESULT          TWBDrivesInit ( HWND, WPARAM, LPARAM );
MRESULT          TWBDrivesControl ( HWND, WPARAM, LPARAM );
MRESULT          TWBDrivesCommand ( HWND, WPARAM, LPARAM );
MRESULT          TWBDrivesInitialize ( HWND, WPARAM, LPARAM );
MRESULT          TWBDrivesClose ( HWND, WPARAM, LPARAM );
BOOL             fCheckForRemoteTms (CHAR cDrive);  // !!!! CHM
INT_PTR CALLBACK  TWBConnectDlgProc ( HWND, WINMSG, WPARAM, LPARAM );
MRESULT          TWBConnectInit ( HWND, WPARAM, LPARAM );
MRESULT          TWBConnectControl ( HWND, SHORT, SHORT );
MRESULT          TWBConnectCommand ( HWND, SHORT, SHORT );
MRESULT          TWBConnectInitialize ( HWND, WPARAM, LPARAM );
MRESULT          TWBConnectClose ( HWND, WPARAM, LPARAM );


/*****************************************************************************
 * function prototypes of exported functions                                 *
 *****************************************************************************/
__declspec(dllexport)
VOID             TWBSetFontCharacteristics( VOID );
__declspec(dllexport)
VOID             TWBDrives( HWND, HMODULE, BOOL );
__declspec(dllexport)
BOOL             TWBCheckExpirationDate( PSZ, PSZ );
__declspec(dllexport)
VOID             TWBConnect( HWND, HMODULE, USHORT );
__declspec(dllexport)
VOID             TWBSystemProps();


/*****************************************************************************
 * type definitions                                                          *
 *****************************************************************************/
typedef struct
{
   USHORT   usLength;
   USHORT   usReserved;
   HMODULE  hResourceMod;
   BOOL     fEqfTmCom;
} DRIVES_DLG_INIT, FAR * PDRIVES_DLG_INIT;

typedef struct _TWBDRIVESIDA
{
   CHAR    chPrimaryDrive;             // primary EQF drive
   CHAR    szDrives[MAX_DRIVELIST];    // available drives
   BOOL    afDrives[MAX_DRIVELIST];    // current set state of drives
   BOOL    fEqfTmCom;                  // !!!! CHM is TM Com Code calling ?
   HMODULE hResourceMod;               // !!!! CHM resource module handle
} TWBDRIVESIDA, *PTWBDRIVESIDA;

/**********************************************************************/
/* defines for DOS calls                                              */
/**********************************************************************/
#define FIND_DIR        0x0010

/**********************************************************************/
/* define for expiration check for DEMO                               */
/**********************************************************************/
#define EXPIRATION_COUNT  5184000     // so much seconds do 60 days have
#define KEY             157932876     // cypher installation date

/**********************************************************************/
/* defines for the new expiration date check                          */
/**********************************************************************/

#define EXP_DATE_FILENAME        "EQFSYS.DAT"



#define VERSION_OFFSET           3234
#define MONTH_MULTIPLICATOR      32L
#define YEAR_MULTIPLICATOR       420L
/* define a value used if no expiration date shall be used: 7.1.1968 */
#define NO_EXPIRY                (7L + MONTH_MULTIPLICATOR + \
                                  1968L*YEAR_MULTIPLICATOR)
#define EXPIRY_OFFSET            2367143L

typedef struct
{
   SHORT    sVersion;
   ULONG    ulInstDate;
   ULONG    ulExpDate;
} EXP_DATE, FAR * PEXP_DATE;

// stuff for system properties dialog

// system properties dialog instance data area
typedef struct _TWBSYSPROPIDA
{                                           //procedure InvokeAnalysisDlgProc
   HWND         hwndPages[20];              // array with property pages
   HWND         hwndTabCtrl;                // handle of tab control
   CHAR         szLogoTime[80];             // buffer for logo display time string
   SHORT        sLogoDisplayTime;           // logo display time
   CHAR         szBrowser[MAX_LONGPATH];    // browser path and name
   CHAR         szProgramPath[MAX_LONGPATH];// buffer for program directory
   CHAR         szTemp[MAX_LONGPATH];       // temp buffer for path names
   PROPSYSTEM   SysProps;                   // copy of system properties
} TWBSYSPROPIDA, *PTWBSYSPROPIDA;

INT_PTR CALLBACK TwbSysPropDlgProc( HWND, WINMSG, WPARAM, LPARAM );
BOOL TwbSysPropPropertySheetLoad( HWND, PTWBSYSPROPIDA );
MRESULT TwbSysPropPropertySheetNotification( HWND, WPARAM, LPARAM );
INT_PTR CALLBACK TWB_SYSPROP_GENERAL_DLGPROC( HWND, WINMSG, WPARAM, LPARAM );
MRESULT TwbSysPropCommand( HWND, WPARAM, LPARAM );
