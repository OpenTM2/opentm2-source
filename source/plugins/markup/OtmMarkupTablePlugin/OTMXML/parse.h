/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/*
*
* parse.h
*
* Define TranslationManager/2 analysis program for WINDOWS INF
* files.
*
*/
    /* This is called by EQFPRESEG2 */
    BOOL PreParse(PSZ in, PSZ out, PSZ markup, PSZ sourcefile, PSZ tempfile, HWND hSlider);

    /* This is called by EQFPOSTSEG2 */
    BOOL PostParse(PSZ in, PSZ out, HWND hSlider);

    /* This is called by EQFPOSTUNSEG2 */
    BOOL PostExport(PSZ in, PSZ out, PSZ markup ) ;


