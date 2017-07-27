/*
*  Copyright (C) 1998-2017, International Business Machines          
*         Corporation and others. All rights reserved 
*                 IBM Internal Use Only
*
* ValDocDocx.h
*
* CHANGES:
*      02/23/05   DAW  Inital version
*
*/

#define   MAX_RCD_LENGTH                 12288
#define   MAX_XML_RCD_LENGTH              8000
#define   MAX_XML_RCD_LENGTH2             8500      /* Buffer allocated size */
#define   XML_TAG_LEN                       80

#define   FILE_FORMAT_UNKNOWN                0     // Unknown file format
#define   FILE_FORMAT_WORD                   1     // Office 2003 format
#define   FILE_FORMAT_XML                    2     // XML format 
#define   FILE_FORMAT_ZIP                    3     // Office 2007 format



// Routines defined in ValDocDocx.cpp       ----------------------------------------------

    BOOL       ConvertWordToXml(char*, char*, char*, char*, USHORT* );
    BOOL       ExtractXmlFromZip(char*, char*, char*, char* );
    BOOL       ExecuteCommand( char*, char*, char* );
    BOOL       ConcatFiles( char*, char*, char* );


// Routines defined in ValDocDocxParse.CPP      ----------------------------------------------

    BOOL       Parse(PSZ, PSZ);
    BOOL       ShowIBMMessage( CHAR *, CHAR *, BOOL, BOOL ) ;


