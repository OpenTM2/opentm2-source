* ==================================================================================================
* Tool Name     : EqfNfluent.trg
* Version       : V2.2
* Purpose       : Trigger file for the MT-output creation in various formats (especially for nFluent)
* Copyright     : IBM 
* Author        : Gerhard Queck
* Creation Date : 01.01.2010
* Updates       : Gerhard Fetz:  04.30.2012: Added new file header and more detailed descriptions
*                 Gerhard Queck: 10.04.2012: Added new switch NOPROPOSAL
*                 Gerhard Queck: 13.03.2013: Added new switches for no duplicates versions of the output files
*                 Gerhard Queck: 12.07.2013: Added new switches NOPROPOSALEXP and NOPROPOSALEXP_NODUPLICATES
*                 Gerhard Queck: 11.11.2015: Added new switch INCLUDEWORDCOUNT
* ==================================================================================================
*
* INFORMATION: Place an asterisk (*) before any of the following switches to disable the output creation.
*
* =============================================================================================================
* == PURPOSE: Switch to globally activate the MT-logging in the Translation Environment. ======================
MTLOGGING
*
* =============================================================================================================
*
* =============================================================================================================
* == PURPOSE: Switch to add word count info to the MT output files.                                          == 
*INCLUDEWORDCOUNT
*
* =============================================================================================================
*
* =============================================================================================================
* == PURPOSE: Switch to create an XML-file which contains ALL source-segments but NO exact matches.          == 
*NOMATCH
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
NOMATCHEXP
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
ALLSEGSEXP
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
* == PURPOSE: Switch to create an XML-file which contains ALL source-segments which have no proposal at all. == 
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
NOPROPOSALEXP
*
* PURPOSE: Same output as NOPROPOSAL but without duplicate segments                                          == 
*NOPROPOSALEXP_NODUPLICATE 
*
* Note: Only one of the switches NOPROPOSALEXP and NOPROPOSALEXP_NODUPLICATE can be active at a time   
*
* =============================================================================================================
*XLIFF
