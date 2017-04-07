* ==================================================================================================
* Tool Name     : EqfNfluent.trg
* Version       : V2.6
* Purpose       : Trigger file for the MT-output creation in various formats (especially for nFluent)
* Copyright     : Copyright (c) 2012-2017, International Business Machines Corporation and others.  All rights reserved.
* Author        : Gerhard Queck
* Creation Date : 2010/01/01 (YYYY/MM/DD)
* Updates       : Gerhard Fetz:  2012/04/30: Added new file header and more detailed descriptions
*                 Gerhard Queck: 2012/04/10: Added new switch NOPROPOSAL
*                 Gerhard Queck: 2013/03/13: Added new switches for no duplicates versions of the output files
*                 Gerhard Queck: 2013/07/12: Added new switches NOPROPOSALEXP and NOPROPOSALEXP_NODUPLICATES
*                 David Walter:  2014/10/20: Added new switch XLIFF
*                 Gerhard Queck: 2015/11/17: Some minor rework
*                 Gerhard Queck: 2016/02/18: Added a small description text to NOPROPOSAL output switches
*                 Gerhard Queck: 2016/03/01: Added new switch INCLUDEWORDCOUNT
*                 Gerhard Queck: 2017/03/15: Adjusted description of NOPROPOSAL option
* ==================================================================================================
*
* INFORMATION: Place an asterisk (*) before any of the following switches to disable the output creation.
*
* =============================================================================================================
* == PURPOSE: Switch to globally activate the MT-logging in the Translation Environment. ======================
MTLOGGING
*
*
* =============================================================================================================
* == PURPOSE: Switch to add word count information to all XML and EXP output files.      ======================
*INCLUDEWORDCOUNT
*
* Note: In the EXP formats the word count info is stored in the AddData section of each segment and the total 
*       word count is stored in the description field of the EXP file.
*       In the XML formats the word count is stored in words= attribute of the <s> elements and the total word
*       count is stored in the <totalWords> element. 
*
* =============================================================================================================
* == PURPOSE: Switch to create an XML-file which contains ALL source-segments but NO exact matches.          == 
NOMATCH
*
*
* PURPOSE: Same output as NOMATCH but without duplicate segments                                             == 
*NOMATCH_NODUPLICATE
*
* Note: Only one of the switches NOMATCH and NOMATCH_NODUPLICATE can be active at a time   
*
* =============================================================================================================
*
* =============================================================================================================
* == PURPOSE: Switch to create an EXP-file which contains ALL source-segments but NO exact matches.          == 
*NOMATCHEXP
*
* PURPOSE: Same output as NOMATCHEXP but without duplicate segments                                          == 
*NOMATCHEXP_NODUPLICATE
*
* Note: Only one of the switches NOMATCHEXP and NOMATCHEXP_NODUPLICATE can be active at a time   
*
* =============================================================================================================
*
* =============================================================================================================
* == PURPOSE: Switch to create an XML-file which contains ALL source-segments EVEN exact matches.            == 
*ALLSEGS
*
* PURPOSE: Same output as ALLSEGS but without duplicate segments                                             == 
*ALLSEGS_NODUPLICATE 
*
* Note: Only one of the switches ALLSEGS and ALLSEGS_NODUPLICATE can be active at a time   
*
* =============================================================================================================
*
* =============================================================================================================
* == PURPOSE: Switch to create an EXP-file which contains ALL source-segments EVEN exact matches.            == 
*ALLSEGSEXP
*
* PURPOSE: Same output as ALLSEGSEXP but without duplicate segments                                          == 
*ALLSEGSEXP_NODUPLICATE 
*
* Note: Only one of the switches ALLSEGSEXP and ALLSEGSEXP_NODUPLICATE can be active at a time   
*
* =============================================================================================================
*
* =============================================================================================================
* == PURPOSE: Switch to create an XML-file which contains ALL source-segments and ALL target-segments        ==
* ==          including e.g. EXACT or FUZZY matches. In addition the MATCH-rate is written into the output.  == 
*ALLWMATCH
*
* PURPOSE: Same output as ALLWMATCH but without duplicate segments                                            == 
*ALLWMATCH_NODUPLICATE 
*
* Note: Only one of the switches ALLWMATCH and ALLWMATCH_NODUPLICATE can be active at a time   
*
* =============================================================================================================
*
* =============================================================================================================
* == PURPOSE: Switch to create an XML-file which contains ALL source-segments and ALL target-segments        ==
* ==          including e.g. EXACT or FUZZY matches. In addition the MATCH-rate and the SOURCE of the        ==
* ==          translation memory proposal is written into the output.                                        == 
* ALLWMATCHSOURCE
*
* PURPOSE: Same output as ALLWMATCHSOURCE but without duplicate segments                                     == 
*ALLWMATCHSOURCE_NODUPLICATE 
*
* Note: Only one of the switches ALLWMATCHSOURCE and ALLWMATCHSOURCE_NODUPLICATE can be active at a time   
*
* =============================================================================================================
*
* =============================================================================================================
* == PURPOSE: Switch to create an XML-file which contains ALL source-segments which have no proposal at all  ==
*             This means no EXACT match, no MT-match, no GLOBAL MEMORY match, and no FUZZY match must exist  ==
*             in an attached translation memory."                                                            ==
*NOPROPOSAL
*
* PURPOSE: Same output as NOPROPOSAL but without duplicate segments                                          == 
*NOPROPOSAL_NODUPLICATE 
*
* Note: Only one of the switches NOPROPOSAL and NOPROPOSAL_NODUPLICATE can be active at a time   
*
* =============================================================================================================
*
* =============================================================================================================
* == PURPOSE: Switch to create an EXP-file which contains ALL source-segments which have no proposal at all. == 
*             also ignoring FUZZY matches with a fuzziness below 50%.                                        == 
*NOPROPOSALEXP
*
* PURPOSE: Same output as NOPROPOSAL but without duplicate segments                                          == 
*NOPROPOSALEXP_NODUPLICATE 
*
* Note: Only one of the switches NOPROPOSALEXP and NOPROPOSALEXP_NODUPLICATE can be active at a time   
*
* =============================================================================================================
*
* =============================================================================================================
* == PURPOSE: Switch to create an XLIFF-file which contains ALL source-segments, along with all proposals.   == 
*XLIFF
*
* =============================================================================================================

