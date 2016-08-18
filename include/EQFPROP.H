//+----------------------------------------------------------------------------+
//|  EQFPROP.H                                                                 |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Description:  defines for PROPS= Handling and Evaluation Utilities        |
//+----------------------------------------------------------------------------+

// definitions when used outside of EQF.H
#ifndef _EQF_H_INCLUDED
  typedef wchar_t * PSZ_W;
#endif

__declspec(dllexport)
USHORT PropsInit
(
  PHANDLE   phProps,               // address of variable receiving the props handle
  PSZ       pszFolObjName          // folder object name (e.g. C:\EQF\SAMPLE.F00)
);

__declspec(dllexport)
USHORT PropsTerminate
(
  HANDLE     hProps                // props handle created using PropsInit
);

__declspec(dllexport)
USHORT PropsEvaluate
(
  HANDLE    hProps,                // props handle created using PropsInit
  PSZ_W     pszStatement,          // statement being evaluated
  PBOOL     pbResult               // address of caller’s result flag
);

__declspec(dllexport)
USHORT PropsResetAll
(
  HANDLE     hProps                // props handle created using PropsInit
);

__declspec(dllexport)
USHORT PropsSetValue
(
  HANDLE    hProps,                // props handle created using PropsInit
  PSZ_W     pszName,               // name of the variable
  PSZ_W     pszValue               // new value for the variable
);

__declspec(dllexport)
USHORT PropsLoadValFile
( 
  HANDLE        hProp,             // props area handle
  PSZ           pszValFile         // fully qualified VAl file name
);
