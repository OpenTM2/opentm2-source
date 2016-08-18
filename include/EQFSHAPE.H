// Copyright (c) 2002-2012, International Business Machines
// Corporation and others.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////
// EQFSHAPE.H : Special Arabic shaping for 864 to Unicode and vice versa
///////////////////////////////////////////////////////////////////////////////

// Add shaping to the converted Arabic string
void BidiConvertFETo06(LPWSTR lpWideCharStr, ULONG ulBufLen);
void BidiConvert06ToFE( LPWSTR lpWideCharStr, INT length );

/*FE to 06 conversion Table*/
const int TabFEto06[] =
                               {
                                0x0621,
                                0x0622,0x0000,
                                0x0623,0x0000,
                                0x0624,0x0000,
                                0x0625,0x0000,
                                0x0000,0x0000,0x0626,0x0000,
                                0x0627,0x0000,
                                0x0000,0x0000,0x0628,0x0000,
                                0x0629,0x0000,
                                0x0000,0x0000,0x062A,0x0000,
                                0x0000,0x0000,0x062B,0x0000,
                                0x0000,0x0000,0x062C,0x0000,
                                0x0000,0x0000,0x062D,0x0000,
                                0x0000,0x0000,0x062E,0x0000,
                                0x062F,0x0000,
                                0x0630,0x0000,
                                0x0631,0x0000,
                                0x0632,0x0000,
                                0x0000,0x0000,0x0633,0x0000,
                                0x0000,0x0000,0x0634,0x0000,
                                0x0000,0x0000,0x0635,0x0000,
                                0x0000,0x0000,0x0636,0x0000,
                                0x0637,0x0000,0x0000,0x0000,
                                0x0638,0x0000,0x0000,0x0000,
                                0x0000,0x0000,0x0639,0x0000,
                                0x0000,0x0000,0x063A,0x0000,
                                0x0000,0x0000,0x0641,0x0000,
                                0x0000,0x0000,0x0642,0x0000,
                                0x0000,0x0000,0x0643,0x0000,
                                0x0000,0x0000,0x0644,0x0000,
                                0x0000,0x0000,0x0645,0x0000,
                                0x0000,0x0000,0x0646,0x0000,
                                0x0000,0x0000,0x0647,0x0000,
                                0x0648,0x0000,
                                0x0649,0x0000,
                                0x0000,0x0000,0x064A
                               };

/*06 to FE conversion Table*/
const int Tab06ToFE[] = {
                                0xFE80, 0xFE81,
                                0xFE83, 0xFE85,
                                0xFE87, 0xFE8B,
                                0xFE8D, 0xFE91,
                                0xFE93, 0xFE97,
                                0xFE9B, 0xFE9F,
                                0xFEA3, 0xFEA7,
                                0xFEA9, 0xFEAB,
                                0xFEAD, 0xFEAF,
                0xFEB3, 0xFEB7,
                0xFEBB, 0xFEBF,
                0xFEC1, 0xFEC5,
                0xFECB, 0xFECF,
                0x0000, 0x0000,
                0x0000, 0x0000,
                0x0000, 0x0640, //RJ 030725: 0x0020/*Tatweel*/,
                0xFED3, 0xFED7,
                0xFEDB, 0xFEDF,
                0xFEE3, 0xFEE7,
                0xFEEB, 0xFEED,
                0xFEEF, 0xFEF3,
                               };

