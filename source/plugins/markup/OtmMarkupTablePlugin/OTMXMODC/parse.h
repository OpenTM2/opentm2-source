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
* Define TranslationManager/2 analysis program for OpenDocument XML files.
*
* NOTES:
*                added defines for mangling of c++ names.
*

*/

#ifdef __cplusplus
extern "C" {
#endif
/* this should be called by EQFPRESEG2 */
    BOOL PreParse1(PSZ in, PSZ out, PSZ style, HWND hSlider) ;
    BOOL PreParse2(PSZ in, PSZ out, HWND hSlider) ;
    BOOL PostParse(PSZ in, PSZ out, HWND hSlider) ;
    BOOL GetRcd(char *Rcd, long MaxLen, FILE *file, BOOL FullTag) ;
#ifdef __cplusplus
}
#endif


