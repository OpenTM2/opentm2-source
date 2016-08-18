/*! MemoryUtil.H  prototypes and functions for CPP versions of memory base code

	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

/* functions located in TmPluginWrapper */

/* Open a memory */
__declspec(dllexport)
OtmMemory *OpenMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  unsigned short usOpenFlags,
  int *piErrorCode
);

__declspec(dllexport)
int ListMemories
(
	OtmMemoryPlugin::PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
	void *pvData,
	BOOL fWithDetails
);

__declspec(dllexport)
int CloseMemory
(
  OtmMemory *pMemory
);



