/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/*********************************************************************
 *
 * FUNCTION:
 *
 * Segmentation routine to segment JDK 1.1 Property files
 * for Translation in TM/2
 *
 ********************************************************************/

/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#ifndef _JAVASEG_H_INCLUDE_
    #define _JAVASEG_H_INCLUDE_


    #include <windows.h>
    #include <wtypes.h>

int parseJavaResource(char*, short, BOOL, HWND);
int chk_good_break_spot(short);

#endif

