/*! EQFLIST.H           Header file for public list handler functions            
	Description: Header file containing prototypes for list handler functions    
				which are available to other TM/2 modules.                      
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

//+----------------------------------------------------------------------------+

//+----------------------------------------------------------------------------+
// Function name:     LstReadNoiseExclList   Read an exclusion/noise list       
//+----------------------------------------------------------------------------+
// Function call:     LstReadNoiseExclList( pszListName, pusListSize,           
//                                          ppExcList );                        
//+----------------------------------------------------------------------------+
// Description:       Read an internal format noise/exclusion list into         
//                    memory and do a consistency check. Errors are handled     
//                    within this procedure.                                    
//+----------------------------------------------------------------------------+
// Input parameter:   PSZ             pszListName  name of list to read         
//                    PUSHORT         pusListSize  address of list size var     
//                    PEXCLUSIONLIST  *ppExclList  address of excl. list ptr    
//+----------------------------------------------------------------------------+
// Returncode type:   USHORT                                                    
//+----------------------------------------------------------------------------+
// Returncodes:       NO_ERROR          function completed OK                   
//                    other             DOS error codes                         
//+----------------------------------------------------------------------------+
__declspec(dllexport) USHORT  LstReadNoiseExclList( PSZ, PULONG, PEXCLUSIONLIST *);
__declspec(dllexport) BOOL    LstEditAddenda( PSZ pszLanguage );
__declspec(dllexport) BOOL    LstEditAbbrev( PSZ pszLanguage );
