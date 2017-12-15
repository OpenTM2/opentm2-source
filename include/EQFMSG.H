// Copyright (c) 1992-2016, International Business Machines
// Corporation and others.  All rights reserved.
//


#define ERROR_STORAGE                                        1
//* Define statement owner: DOE, XDD, LEE
// Text:       The system storage is insufficient.
// HelpTitle:  OpenTM2 - Help
// Help:       Your PC or PS/2 has run out of storage.  End all
//             unnecessary
//             programs, and restart OpenTM2.

#define ERROR_INTERNAL                                       2
//* Define statement owner: XQG, LEE
// Text:       An internal error or program error occurred.
// HelpTitle:  OpenTM2 - Help
// Help:       An unexpected error occurred.
//             Please note the conditions that have led to that error,
//             and contact your IBM representative.

#define ERROR_RESERVED_STRING                                3
//* Define statement owner: DOE, TAF, LEE
// Text:       The character string '%1' is reserved.
//             Change it and try again.
// HelpTitle:  OpenTM2 - Help
// Help:       You entered a character string which is reserved
//             for OpenTM2. Change the string and try again.

#define ERROR_INTERNAL_PARM                                  4
//* Define statement owner: JOR
// Text:       An internal error or program error occurred in file\n
//             '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       An unexpected error occurred.
//             Please note the conditions that have led to that error,
//             and contact your IBM representative.

#define ERROR_EQF_DRIVE_NOT_VALID                            5
//* Define statement owner: XQG, NEW
// Text:       OpenTM2 is not able to access drive '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Directories used by OpenTM2 may have been
//             deleted or the drive reformatted. Try to
//             activate the drive again by selecting it
//             in the additional drive list if the drive is displayed there.
// Cause:      Secondary OpenTM2 drive is no more valid in
// Cause:      'Configure drives...'

#define ERROR_EQF_DRIVE_NOT_ACCESSIBLE                       6
//* Define statement owner: DOE, LEE
// Text:       OpenTM2 is not able to access drive '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that all the drives OpenTM2
//             should have access to
//             are ready and configured as OpenTM2 drives.
//             Check this with the 'Configure drives...'
//             option in the 'Utilities' pull-down.

#define ERROR_PGM_VERSION_EXPIRED                            7
//* Define statement owner: DOE, LEE
// Text:       This demo version has expired and can no longer
//             be used.
// HelpTitle:  OpenTM2 - Help
// Help:       Ask for the full program version and install it.

#define UTL_PARAM_TOO_LONG                                   8
//* Define statement owner: JOR
// Text:       The parameter '%1' you want to handle is too long.
// HelpTitle:  OpenTM2 - Help
// Help:       The parameter you want to copy does not fit in
//             the provided area.

#define QUERY_CLOSE_TWB                                      9
//* Define statement owner: SOK
// Text:       Do you really want to close the workbench?
// HelpTitle:  OpenTM2 - Help
// Help:       When you close the workbench,
//             you have to restart OpenTM2 when you want
//             to continue.
//* -------------------------------------------------------
//*     Workbench messages
//* -------------------------------------------------------

#define QUERY_FORCE_SHUTDOWN                                10
//* Define statement owner: XQG, LEE
// Text:       One or more objects have not stopped.\n
//             Force shutdown?
// HelpTitle:  OpenTM2 - Help
// Help:       One or more active objects rejected your request to shut
//             down the OpenTM2 workbench because you might
//             lose data.
//             .br
//             To ensure data integrity, cancel the shutdown process and
//             close all open objects. Then request shutdown again.

#define QUERY_SAVECLOSE_TWB                                 11
//* Define statement owner: XQG, XDD, LEE
// Text:       Do you want to save the current workbench?
// HelpTitle:  OpenTM2 - Help
// Help:       When you save the current workbench, all components
//             that are now active are restarted the next time
//             you start the workbench.

#define ERROR_HELP_NOT_AVAILABLE                            12
//* Define statement owner: XQG, LEE
// Text:       The help file '%1' is missing or contains an error.
//             No help is available for the workbench.
// HelpTitle:  OpenTM2 - Help
// Help:       no help (help instance failed)
// Type:       MB_OK
// Cause:      WinCreateHelpInstance call failed
// Add.Info:   %1 = name of help file

#define ERROR_HELP_IN_ERROR                                 13
//* Define statement owner: XQG, LEE
// Text:       IPF could not be started.
//             No help is available for the Workbench.
// HelpTitle:  OpenTM2 - Help
// Help:       no help (help instance failed)
// Type:       MB_OK
// Cause:      WinCreateHelpInstance returned error code
// Add.Info:   none

#define ERROR_HELP_TERMINATED                               14
//* Define statement owner: XQG, TAF, LEE
// Text:       Help stopped due to insufficient storage.
//             No help is available for the Workbench.
// HelpTitle:  OpenTM2 - Help
// Help:       no help (help instance failed)
// Type:       MB_OK
// Cause:      HM_ERROR w/ HMERR_NO_MEMORY rc sent to workbench by help
// Cause:      manager
// Add.Info:   none

#define ERROR_HELP                                          15
//* Define statement owner: SOK, TAF, LEE
// Text:       This option is not available for this dialog.
// HelpTitle:  OpenTM2 - Help
// Help:       Your request for help could not be processed, because
//             there is no help defined for this function, or this
//             function is not available.
// Type:       MB_OK
// Cause:      HM_ERROR w/ other rc sent to workbench by help manager
// Add.Info:   none

#define ERROR_DRIVE_IN_USE                                  16
//* Define statement owner: XQG, TAF, LEE
// Text:       Drive '%1' is in use and cannot be removed as an
//             additional OpenTM2 drive.
// HelpTitle:  OpenTM2 - Help
// Help:       Close the dialog and wait until a OpenTM2
//             function accessing the referred drive has finished.
//             Close any document list accessing the referred drive before
//             you try removing the drive again.

#define ERROR_DID_NOT_START                                 17
//* Define statement owner: XQG, LEE
// Text:       The OpenTM2 component '%1' could not be started.
// HelpTitle:  OpenTM2 - Help
// Help:       The referenced component of the OpenTM2 system
//             could not be started for one of the following reasons:
//             :ul.
//             :li.Insufficient system resources (for example, main storage).
//             :li.Missing or damaged internal property files of
//             OpenTM2.
//             :li.General system failures.
//             :eul.
//             If you are sure that
//             you have sufficient main storage, try the following:
//             :ol.
//             :li.Try to restart OpenTM2.
//             :li.End other applications (for example, Communications
//             Manager) and restart OpenTM2.
//             :li.Reboot your workstation and restart OpenTM2.
//             :li.Reinstall OpenTM2 and try to restart it.
//             :li.Call your IBM representative.
//             :eol.

#define ERROR_RESOURCE_LOAD_FAILED                          18
//* Define statement owner: XQG, LEE
// Text:       The OpenTM2 resource '%1' could not be loaded.
// HelpTitle:  OpenTM2 - Help
// Help:       The OpenTM2 resource could not be loaded.
//             If you are sure that
//             you have sufficient main storage, try the following:
//             :ol.
//             :li.Restart OpenTM2
//             :li.End Communications Manager and restart
//             OpenTM2
//             :li.Reboot your workstation and restart OpenTM2
//             :li.Reinstall OpenTM2 and start it.
//             :eol.

#define ERROR_MAT_NOT_RUNNING                               19
//* Define statement owner: JOR, LEE
// Text:       OpenTM2 is not running.
// HelpTitle:  OpenTM2 - Help
// Help:       You tried to access OpenTM2, but
//             it is not running.
//             You need to start it first.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter
//* -------------------------------------------------------
//*     Disk manipulation messages
//* -------------------------------------------------------

#define ERROR_EQF_DISK_FULL                                 20
//* Define statement owner: SOK, LEE
// Text:       There is not enough space on the disk to write
//             the file '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified disk drive is full. Make space on this disk
//             drive and restart the function.

#define ERROR_INSUFF_RESOURCES                              21
//* Define statement owner: XQG, NEW
// Text:       Insufficient resources to start OpenTM2.
// HelpTitle:  OpenTM2 - Help
// Help:       OpenTM2 could not be started for one of
//             the following reasons:
//             :ul.
//             :li.Insufficient system resources (for example, main storage).
//             :li.General system failures.
//             :eul.
//             If you are sure that
//             you have sufficient main storage, try the following:
//             :ol.
//             :li.Try to restart OpenTM2.
//             :li.End other applications (for example, Communications
//             Manager) and restart OpenTM2.
//             :li.Reboot your workstation and restart OpenTM2.
//             :li.Reinstall OpenTM2 and try to restart it.
//             :li.Call your IBM representative.
//             :eol.

#define ERROR_DISK_IS_FULL                                  22
//* Define statement owner: DOE, TAF, LEE
// Text:       Disk '%1' is full.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified disk is full. Either create more space on
//             this disk and retry, or select another disk drive with
//             more space.

#define ERROR_WRITE_DISK_FULL                               23
//* Define statement owner: DOE, TAF, LEE
// Text:       There is not enough space on the disk to write
//             the file '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Either create more space on this disk and retry, or
//             select another disk drive with more space.

#define ERROR_DIALOG_LOAD_FAILED                            24
//* Define statement owner: XQG, NEW
// Text:       The selected dialog could not be loaded.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected dialog could not be loaded.
//             If you are sure that
//             you have sufficient main storage, try the following:
//             :ol.
//             :li.Restart OpenTM2
//             :li.Reboot your workstation and restart OpenTM2
//             :li.Reinstall OpenTM2 and start it.
//             :eol.

#define ERROR_DRIVE_NOT_VALID                               25
//* Define statement owner: XQG, NEW
// Text:       OpenTM2 is not able to access drive '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       If the drive in question is a network drive,
//             it may have been disconnected or the server may be
//             down. Contact your LAN administrator to check the
//             server.
//             .br
//             If the drive is a local drive the media may have
//             been removed (only for removable media drives
//             like CD, MO, ...) or the drive is not ready.
//             Check your drives (e.g. use CHKDSK from a command
//             prompt) or choose another drive for the operation.

#define ERROR_LAN_DRIVE_NOT_VALID                           26
//* Define statement owner: XQG, NEW
// Text:       Drive '%1' is no LAN drive nor is it shared with
//             other users.
// HelpTitle:  OpenTM2 - Help
// Help:       The function for which the drive was specified
//             requires a drive which can be shared among
//             several users.
//             .br
//             The specified drive is not located on a server
//             nor is it a drive shared with other users.
//             .br
//             The function is stopped.

#define ERROR_MEM_VERSION_NOT_SUPPORTED                     27
//* Define statement owner: XQG, NEW
// Text:       Translation Memory '%1' was created with an newer version
//             of OpenTM2.\n
//             It cannot be processed with the installed version
//             of OpenTM2.
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory you wanted to work with was
//             created with an newer version of OpenTM2
//             which uses a new data format.
//             You need a new version of OpenTM2.
//**********************************************************************/
//* TM Tool messages (inserted into free area)                         */
//**********************************************************************/

#define TMT_WRONGCMDLINE                                    28
//* Define statement owner: XQG
// Text:       The specified command line has an incorrect
//             syntax.\n
//             The wrong parameter is: '%1'\n
//             The correct syntax is:\n
//              EQFTMT  /ME[M]=memdb\n
//                      [[/FROMMARKUP|/FM]=frommu]\n
//                      [[/TOMARKUP|/TM]=tomu]\n
//                      [[/FROMLANG|/FL]=fromlang]\n
//                      [[/TOLANG|/TL]=tolang]\n
//                      [[/DATE|/DA]=yyyy-mm-dd [hh:mm[:ss]]]\n
//                      [[/DOC]=docname]\n
//                      [/TY[PE]=NOCONF]\n
//             where\n
//                memdb    is the Translation Memory database name\n
//                frommu   is the markup being changed\n
//                tomu     is the new markup\n
//                fromlang is the target language being changed\n
//                tomu     is the new target language\n
//                yyyy-mm-dd is the new date to be set for the segments\n
//                hh:mm:ss is the new time to be set for the segments\n
//                docname  is the document of the segments to be changed
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Add.Info:   '%1'  wrong parameter

#define TMT_MANDCMDLINE                                     29
//* Define statement owner: ROB
// Text:       The Translation Memory name as mandatory parameter
//             is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define TMT_COMPLETE                                        30
//* Define statement owner: ROB
// Text:       Change markup and target language successfully finished.\n
//               Segments processed:        %1\n
//               Markups  changed:          %2\n
//               Target language changed:   %3\n
//               Segment date changed:      %4\n
//               Machine flags changed:     %5
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Add.Info:   '%1' = number of segments that were processed,
// Add.Info:   '%2' = number of segments where markup was changed
// Add.Info:   '%3' = number of segments where language was changed
// Add.Info:   '%4' = number of segments where date was changed
// Type:       EQF_INFO
// Severity:   OK
// Cause:      see error message

#define TMT_EMPTYCMDLINE                                    31
//* Define statement owner: ROB
// Text:       No command line parameters are given.\n
//             The correct syntax is:\n
//              EQFTMT  /ME[M]=memdb\n
//                      [[/FROMMARKUP|/FM]=frommu]\n
//                      [[/TOMARKUP|/TM]=tomu]\n
//                      [[/FROMLANG|/FL]=fromlang]\n
//                      [[/TOLANG|/TL]=tolang]\n
//                      [[/DATE|/DA]=yyyy-mm-dd [hh:mm[:ss]]]\n
//                      [[/DOC]=docname]\n
//                      [/TY[PE]=NOCONF]\n
//             where\n
//                memdb    is the Translation Memory database name\n
//                frommu   is the markup being changed\n
//                tomu     is the new markup\n
//                fromlang is the target language being changed\n
//                tomu     is the new target language
//                yyyy-mm-dd is the new date to be set for the segments\n
//                hh:mm:ss is the new time to be set for the segments\n
//                docname  is the document of the segments to be changed
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Type:       EQF_ERROR
// Severity:   CANCEL

#define ERROR_INV_LANGUAGE                                  32
//* Define statement owner: XQG
// Text:       The specified language '%1' is not valid or the associated language
//             file is missing or corrupted.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified name is no name of a valid target language
//             or the language has not been installed yet.

#define TA_MANDTM                                           33
//* Define statement owner: ROB
// Text:       The Translation Memory name as mandatory parameter
//             is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define TA_MANDFOLDER                                       34
//* Define statement owner: ROB
// Text:       The folder name as mandatory parameter is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define TA_MANDMARKUP                                       35
//* Define statement owner: ROB
// Text:       The markup name as mandatory parameter is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!


#define FUNC_MANDINFILE                                     36
//* Define statement owner: XQG
// Text:       The name of the input file as mandatory parameter
//             is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!


#define FUNC_MANDFILES                                      37
//* Define statement owner: XQG
// Text:       No document(s) have been specified.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_INVALID_NUMERICAL_VALUE                      38
//* Define statement owner: XQG, NEW
// Text:       Not a valid numerical value: %1!
// HelpTitle:  OpenTM2 - Help
// Help:       Please type in a numerical value using only digits
//             and optionally preceeded by a plus or minus sign.


#define ERROR_BIDI_NOT_AVAILABLE                            39
//* Define statement owner: XJR
// Text:       The runtime library "%1" for enhanced bidirectional
//             support is not available or cannot be accessed.\n
//             No special bidirectional layouting will be available.
// HelpTitle:  OpenTM2 - Help
// Help:       Please check that the specified runtime library
//             is available in the \EQF\WIN directory.
//*
//* -------------------------------------------------------
//*     Directory and file messages
//* ------------------------------------------------------

#define ERROR_FILENAME_NOT_VALID                            40
//* Define statement owner: DOE, LEE
// Text:       The specified file name or path '%1' is not valid or
//             cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       You should check the following:
//             :ol.
//             :li.The file name needs to follow the naming conventions.
//             File names can have up to eight alphanumeric characters,
//             file extensions up to three.
//             :li.Ensure that the drive or directory you specified
//             exists and can be accessed.
//             :li.Ensure that the drive or directory is not
//             write-protected.
//             :li.Ensure that the drive or directory you specified is
//             not some other device (for example, a printer like LPT1).
//             :eol.
//             .br
//             Correct the name or names and try again.

#define ERROR_FILE_EXISTS_ALREADY                           41
//* Define statement owner: DOE, TAF, LEE
// Text:       The file '%1' exists already.\n
//             Do you want to replace it?
// HelpTitle:  OpenTM2 - Help
// Help:       You have asked to write to the specified file. However,
//             this file exists already.
//             Either replace the file, or exit without
//             replacing it.

#define PATH_NOTEXIST_CREATEIT                              42
//* Define statement owner: DOE, SOK, TAF, LEE
// Text:       The directory '%1' does not exist.\n
//             Do you want to create it?
// HelpTitle:  OpenTM2 - Help
// Help:       The specified directory does not exist. Either
//             create it, or exit without creating it.

#define FILE_NOT_EXISTS                                     43
//* Define statement owner: ...
#define ERROR_FILE_ACCESS_ERROR                             43
//* Define statement owner: DOE, TAF, NEW
// Text:       The file '%1' does not exist or cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified file cannot be accessed for one of the
//             following reasons:
//             :ul.
//             :li.The file is in use by another process. Wait until
//             the process is finished and try again.
//             :li.You are trying to access a drive that is currently not
//             accessible because it was either deleted manually or via
//             the Configure drives option on the 'Utilities' pull-down.
//             :li.The file has been deleted
//             :eul.

#define ERROR_FILE_READ_ERROR                               44
//* Define statement owner: DOE, LEE
// Text:       The file '%1' cannot be found or read. The DOS error
//             code is: %2.
// HelpTitle:  OpenTM2 - Help
// Help:       Either your disk contains an error, or an internal error
//             occurred.

#define NO_VALID_FORMAT                                     45
#define ERROR_FILE_INVALID_DATA                             45
//* Define statement owner: XQG, JOR, LEE
// Text:       The file '%1' contains data that cannot be processed.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified file is not in the format the process expects.
//             Retry the function. If the problem continues select another
//             file. If you are analyzing a document, end the Analysis dialog.
//             Select the correct markup table and restart the analyze
//             process.

#define NO_FILE_SELECTED                                    46
//* Define statement owner: DOE, LEE
// Text:       No file has been selected.
// HelpTitle:  OpenTM2 - Help
// Help:       Select a file before you select an action.

#define INFO_MEM_SELECTED                                   47
//* Define statement owner: XQG
// Text:       The requested Translation Memory is already selected.
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory entries must be unique in the list
//             of selected Translation Memory database.
//             A previously selected
//             Translation Memory cannot be selected again.

#define ERROR_PATH_NOT_EXIST                                48
//* Define statement owner: DOE, TAF, LEE
// Text:       The specified path does not exist.
// HelpTitle:  OpenTM2 - Help
// Help:       Correct the path name and try again.

#define ERROR_FILE_WRITE_ERROR                              49
//* Define statement owner: DOE  LEE
// Text:       An error occurred writing to file '%1'. The DOS error
//             code is: %2.
// HelpTitle:  OpenTM2 - Help
// Help:       Either your disk contains an error, or an internal error
//             occurred.

#define QUERY_DOCEXISTS_ALREADY                             50
//* Define statement owner: XQG
// Text:       The document '%1' exists already.\n
//             Existing source document: %2  Size: %3\n
//             Imported source document: %4  Size: %5\n
//             Do you want to replace the document with the document
//             imported with the folder?
// HelpTitle:  OpenTM2 - Help
// Help:       You are importing a folder and a document contained in the
//             folder already exists on your system.
//             :p.Use the supplied information and decide if the existing
//             document is to be replaced with the document contained in the
//             folder.
//             :p.Either replace the existing document, or leave the existing
//             document and ignore the imported one or cancel the folder
//             import.

#define WARNING_NO_ARCHTM_FOR_XLATE                         51
//* Define statement owner: XQG
// Text:       Documents with a markup of XLATEWIN can not be
//             used for the build archive Translation Memory function and
//             will be ignored. Do you want to continue with the other
//             documents?
// HelpTitle:  OpenTM2 - Help
// Help:       Some of the documents selected for the build archive
//             Translation Memory use XLATEWIN as markup.
//             As these documents use a different segmentation scheme
//             they cannot be used for this function and will be ignored.
//             :p.You can either continue the build archive Translation Memory
//             function with the remaining documents,
//             or you can cancel the function.
//             :p.Keep in mind that if you continue
//             the resulting Translation Memory
//             will not contain the translations of all selected
//             documents.

#define ERROR_ARCHTM_XLATEDOCS                              52
//* Define statement owner: XQG
// Text:       The documents selected for the build archive Translation Memory
//             function use XLATEWIN as markup.
//             These documents can not be used for the build archive
//             Translation Memory function. The function will be
//             stopped.
// HelpTitle:  OpenTM2 - Help
// Add.Info:   Help is not used as this is a batch only messages!
// Help:       The documents selected for the build archive
//             Translation Memory use XLATEWIN as markup.
//             As these documents use a different segmentation scheme
//             they cannot be used for this function and the function will be
//             stopped.

#define ERROR_FILE_EXISTS_BATCH                             53
//* Define statement owner: NEW
// Text:       The file '%1' exists already.\n
//             Repeat the command and specify /OV=YES to overwrite
//             the existing file or change the name of the output
//             file.
// HelpTitle:  OpenTM2 - Help
// Help:       Repeat the command and specify /OV=YES to overwrite
//             the existing file or change the name of the output
//             file.

#define ERROR_FILE_TOOLARGE                                 54
//* Define statement owner: XJR
// Text:       The file '%1' contains more segments than allowed
//             within OpenTM2.\n
//             The maximum number of segments is 65535.\n
//             Please split your file and translate the smaller files.
// HelpTitle:  OpenTM2 - Help
// Help:       Export your file, split it, import and translate
//             the new files.


#define ERROR_WORDCOUNT_INVALID_ADDINFO                       55
//* Define statement owner: XJR
// Text:       The data and/or the word count information
//             of the segment(s) %3 in document '%1' is corrupted.\n
//             \n
//             An error text has been inserted into the segment(s).\n
//             Look for the string "ERROR!" in the document.\n
//             \n
//             Please package all files from all subdirectories of '%2'
//             and send them to IBM for further investigation.
// HelpTitle:  OpenTM2 - Help
// Help:       Due to an internal error or an inconsistent version
//             the data or the counting information of some segments
//             has been damaged. The counting information of these
//             segments has been reset, the status of the segments has
//             been set to "untranslated" and an error text has been
//             added to the segments. Look fot the string "ERROR!"
//             to find and correct the data of the segments.
//             Please send the logging information which is
//             contained under the subdirectory ADDINFO of the
//             active folder to IBM.

#define ERROR_INV_LONGNAME                                  56
//* Define statement owner: XQG
// Text:       The entered name '%1' is not valid.
// HelpTitle:  OpenTM2 - Help
// Help:       The entered name may have up to 256 characters.
//             The characters backslash, asterisk and question mark
//             are not allowed.

#define ERROR_HISTLOG_CORRUPTED                           57
//* Define statement owner: XQG
// Text:       The file containing the folder history information
//             is damaged.
// HelpTitle:  OpenTM2 - Help
// Help:       The file containing the folder history is damaged.
//             Please note the conditions that have led to that error,
//             and contact your IBM representative.
// Type:       EQF_ERROR

#define ERROR_MISSING_EXPORTFOLDER_NAME                      58
//* Define statement owner: XQG
// Text:       The name for the exported folder is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter a name for the exported folder.
//             The name may have up to 255 characters.
//             The characters question mark, asterisk, and
//             backslash are not allowed.

#define ERROR_FOLDER_READ_FAILED                            59
//* Define statement owner: SOK
// Text:       An error occurred while importing the folder.
// HelpTitle:  OpenTM2 - Help
// Help:       Reconnect the drive.
//             There are several possibilities why the import failed.
//             :ul.
//             :li.The drive you want to import the folder from is
//             disconnected. Reconnect the drive.
//             :li.The disk you want to import the folder from has
//             been removed. Reinsert the disk.
//             :li.Because of some errors, which were reported prior
//             to this message folder import failed. Follow the
//             instructions in the previously displayed messages.
//             :eul.
//* -------------------------------------------------------
//*     Translation Memory messages
//* -------------------------------------------------------

#define ERROR_INVALID_SEGMENT                               60
//* Define statement owner: DOE, LEE
// Text:       The syntax of segment %1 in file '%2' is incorrect.
//             Do you want to skip this segment?
// HelpTitle:  OpenTM2 - Help
// Help:       You can either continue importing, in which case the segment
//             is skipped, or you can
//             cancel the import process and correct the syntax
//             error. Then try to import the segment again.

#define ERROR_INVALID_SEGMENT_ID                            61
//* Define statement owner: DOE, TAF, LEE
// Text:       The file '%2' contains a segment with an incorrect segment
//             identification. Check the segment with identification %1.
// HelpTitle:  OpenTM2 - Help
// Help:       The segment identification is a string of ten characters
//             separated from the tag &lt.Segment&gt. by one blank character.
//             This string is necessary to import a segment. Correct the
//             identification accordingly.

#define ERROR_MEM_LOAD_INITFAILED                           62
//* Define statement owner: DOE  LEE
// Text:       The initialization of import from file
//             '%1' into Translation Memory '%2' failed.
// HelpTitle:  OpenTM2 - Help
// Help:       The initialization process for import failed due
//             to an internal error.
//             You might not have enough system storage available.
//             Close some running applications, and retry this function.

#define MESSAGE_MEM_LOAD_COMPLETED                          63
//* Define statement owner: DOE, TAF, LEE
// Text:       The import of file '%2' into Translation Memory '%1'
//             was completed successfully.
//             %3 segments have been imported. %4 segments were
//             not valid or they are obsolete.
// HelpTitle:  OpenTM2 - Help
// Help:       Segments need to have valid syntax to be imported.
//             Segments that have a more recent equivalent in the
//             Translation Memory are also not imported.

#define MESSAGE_MEM_CREATE_COMPLETED                        64
//* Define statement owner: DOE, LEE
// Text:       Translation Memory '%1' was created successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
//*       The message is self-explanatory.

#define ERROR_MEM_LOAD_TERMINATED                           65
//* Define statement owner: DOE, LEE
//* LEE:  WHAT CAN THE USER DO HERE?
//* DOE:  This message is always displayed as second message.
//*       The previous message explains the exact reason.
//*
// Text:       The import of file '%2' into Translation Memory '%1'
//             has terminated abnormally.
// HelpTitle:  OpenTM2 - Help
// Help:       Due to a previously reported error condition
//             the import process stopped before it was successfully
//             completed.

#define ERROR_MEM_LOAD_NOSEGMENTS                           66
//* Define statement owner: DOE, LEE
// Text:       No segments have been imported from file '%2'
//             into Translation Memory '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       There are several possibilities why no segments were
//             imported:
//             :ul.
//             :li.The file contains no segments
//             :li.The syntax of all segments in not valid
//             :li.All segments already exist in the Translation Memory,
//             and are more recent than the segments you
//             want to import.
//             :eul.

#define ERROR_MEM_CREATE_NO_NAME                            67
//* Define statement owner: DOE, TAF, LEE
// Text:       No name has been specified for the new Translation Memory.
// HelpTitle:  OpenTM2 - Help
// Help:       To create a new Translation Memory you need to provide a name
//             for it.

#define ERROR_MEM_LOAD_TERM_FORCED                          68
//* Define statement owner: DOE, LEE
// Text:       The import of file '%2' into Translation Memory '%1' was
//             stopped before it completed.
// HelpTitle:  OpenTM2 - Help
// Help:       A close or stop request stopped the import process.
//             Restart the function to get a successful completion.

#define WARNING_MEM_NOT_SELECTED                            69
//* Define statement owner: DOE, TAF, LEE
// Text:       You need to select a Translation Memory before
//             choosing this action.
// HelpTitle:  OpenTM2 - Help
// Help:       You selected an action but not a Translation Memory.
//             Select a Translation Memory, and then the action
//             you want to be carried out.

#define ERROR_MEM_SAME_LANGUAGES                            70
//* Define statement owner: DOE, LEE
// Text:       The source and the target language are identical.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       Select 'Yes' if the source and target languages are
//             meant to be identical. Otherwise select another
//             source or target language.

#define ERROR_MEM_IN_EXCL_USE                               71
//* Define statement owner: DOE, TAF, LEE
// Text:       Translation Memory '%1' is being used at the moment by
//             another function.
// HelpTitle:  OpenTM2 - Help
// Help:       Check whether this Translation Memory is accessed by another
//             function.
//             This could be, for example, Organize,
//             Export, or another Translation Memory function.
//             To use the Translation Memory stop this function or wait until
//             it is finished.

#define ERROR_MEM_NAME_INVALID                              72
//* Define statement owner: DOE, LEE
// Text:       The name '%1' is incorrect or a Translation Memory with this
//             name exists already.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to create the Translation Memory, either specify
//             another name or correct the name.
//             The name can have up to
//             eight alphanumeric characters. Do not include a file
//             extension.

#define ERROR_SOURCE_LANG_DATA                              73
//* Define statement owner: DOE, LEE
// Text:       The '%1' language data in file '%2' is incorrect or damaged.
// HelpTitle:  OpenTM2 - Help
// Help:       This error is the result of an installation or programming
//             error. Try reinstalling OpenTM2.

#define ERROR_MEM_UNDEFINED                                 74
//* Define statement owner: DOE, LEE
// Text:       An error occurred while processing a function related
//             to Translation Memory '%1'.
//             The error code is %2.
// HelpTitle:  OpenTM2 - Help
// Help:       An unexpected error occurred.

#define ERROR_MEM_VERSION_MISMATCH                          75
//* Define statement owner: DOE, LEE
// Text:       Translation Memory '%1' was created with an older version
//             of OpenTM2.\n
//             Do you want to start the organize process of
//             Translation Memory '%1' now?
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory you wanted to work with was
//             created with an older version of OpenTM2.
//             Invoke the organize function for
//             this Translation Memory to fix the problem.
//             You need to organize it before you can use it.

#define ERROR_MEM_EXPORT_INITFAILED                         76
//* Define statement owner: DOE, LEE
// Text:       Exporting Translation Memory
//             '%1' into file '%2' failed during initialization.
// HelpTitle:  OpenTM2 - Help
// Help:       The initialization process for export failed due
//             to an internal error.
//             You might not have enough system storage available.
//             Close some running applications, and try
//             to export again.

#define ERROR_MEM_EXPORT_NOSEGMENTS                         77
//* Define statement owner: DOE, LEE
// Text:       The Translation Memory '%2' you want to export is empty.
//             Therefore the file '%1' is not created.
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory you want to export contains no
//             active segments, or it is
//             damaged and no segments could be retrieved.

#define MESSAGE_MEM_EXPORT_COMPLETED                        78
//* Define statement owner: DOE, LEE
// Text:       The export of Translation Memory '%2' to file '%1' completed
//             successfully. '%3' segments have been exported.
// HelpTitle:  OpenTM2 - Help
// Help:       The export of the Translation Memory has been successfully
//             completed. The displayed number of segments are now in the
//             exported Translation Memory.

#define ERROR_MEM_EXPORT_TERMINATED                         79
//* Define statement owner: DOE, LEE
// Text:       The export of Translation Memory '%2' to file '%1' terminated
//             abnormally.
// HelpTitle:  OpenTM2 - Help
// Help:       The export process stopped before it was successfully
//             completed. The disk or the diskette you want to export
//             the Translation Memory to might be damaged.

#define ERROR_MEM_EXPORT_TERM_FORCED                        80
//* Define statement owner: DOE, LEE
// Text:       The export of Translation Memory '%2' to file '%1' stopped
//             before it completed.
// HelpTitle:  OpenTM2 - Help
// Help:       A close or stop request stopped the export process.
//             Restart the function in order to get a successful
//             completion.

#define WARNING_DELETE_MEMORY                               81
//* Define statement owner: DOE, LEE
// Text:       Are you sure that you want to delete Translation
//             Memory '%1'?
// HelpTitle:  OpenTM2 - Help
// Help:       Delete the Translation Memory if you are sure that
//             you do not need it any more.
//             .br
//             Hint:
//             .br
//             You might want to export the Translation Memory before deleting
//             it to save its data.

#define ERROR_MEM_NOT_DELETED                               82
//* Define statement owner: DOE, LEE
// Text:       Translation Memory '%1' is in use at the moment and cannot be
//             deleted.
// HelpTitle:  OpenTM2 - Help
// Help:       Wait until the selected Translation Memory is not being
//             used any more before you delete it.

#define ERROR_MEM_NOT_ACCESSIBLE                            83
//* Define statement owner: DOE, LEE
// Text:       Translation Memory '%1' is in use at the moment.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified Translation Memory is in use at the moment by
//             another process. Wait until this process has finished before
//             you try again.

#define ERROR_MEM_MIGHT_BE_CORRUPTED                        84
//* Define statement owner: DOE, LEE
// Text:       Translation Memory '%1' is damaged.\n
//             Do you want to start the organize process for
//             Translation Memory '%1' now?
// HelpTitle:  OpenTM2 - Help
// Help:       The specified Translation Memory is in an undefined status
//             because a program or system stopped unexpectedly.
//             Invoke the organize function for
//             this Translation Memory to fix the problem.

#define ERROR_MEM_DISK_FULL                                 85
//* Define statement owner: DOE, NEW
// Text:       The disk that contains Translation Memory '%1' is full.
// HelpTitle:  OpenTM2 - Help
// Help:       If you are currently translating a document, end
//             this translation session. Make space on the disk that
//             contains the Translation Memory and restart the
//             translation session.
//             If you are currently organizing a Translation Memory,
//             make space on the disk where the Translation Memory is
//             located and restart the organize process.

#define ERROR_MEM_DB_FULL                                   86
//* Define statement owner: DOE, LEE
// Text:       Translation Memory '%1' is full.
// HelpTitle:  OpenTM2 - Help
// Help:       Due to the internal structure of Translation Memory databases
//             their size is limited. Reorganize the Translation
//             Memory you are using.

#define ERROR_MEM_ORGANIZE_NOSPACE                          87
//* Define statement owner: DOE, LEE
// Text:       Disk '%2' is %3 bytes short of space to organize
//             Translation Memory '%1'.\n
//             If you start organizing the process stops
//             when the disk gets full.
//             Do you want to start organizing anyway?
// HelpTitle:  OpenTM2 - Help
// Help:       Make more space available on the disk by deleting
//             unnecessary files before you restart this function.
// Type:       EQF_QUERY
// Severity:   YES NO
// Cause:      MemCheckSpaceForFile returns that disk space is short
// Add.Info:   '%1' = TM name, '%2' = disk drive
// Add.Info:   '%3' the number of bytes the disk is short

#define ERROR_MEM_ORGANIZE_INITFAILED                       88
//* Define statement owner: DOE, LEE
// Text:       Organizing Translation Memory
//             '%1' failed during initialization.
// HelpTitle:  OpenTM2 - Help
// Help:       The initialization process for organize failed due
//             to an unidentified error.
//             You might not have enough system storage available.
//             Close some running applications, and try
//             to organize again.

#define MESSAGE_MEM_ORGANIZE_COMPLETED                      89
//* Define statement owner: DOE, LEE
// Text:       The organization of Translation Memory '%1' completed
//             successfully. %2 segments have been organized.
//             %3 segments were corrupted and had to be skipped.
// HelpTitle:  OpenTM2 - Help
// Help:       The displayed number of segments is stored in the
//             Translation Memory.

#define ERROR_MEM_ORGANIZE_TERMINATED                       90
//* Define statement owner: DOE, LEE
// Text:       The organization of Translation Memory '%1' abnormally
//             terminated. The Translation Memory remains unchanged.
// HelpTitle:  OpenTM2 - Help
// Help:       An abnormal situation occurred that caused
//             the organize process to stop before it was
//             completed.
//             You might not have enough system storage available.
//             Close some running applications, and try
//             to organize again.

#define ERROR_MEM_ORGANIZE_TERM_FORCED                      91
//* Define statement owner: DOE, LEE
// Text:       The organization of Translation Memory '%1' stopped before
//             it was completed. The Translation Memory remains unchanged.
// HelpTitle:  OpenTM2 - Help
// Help:       A close or stop request stopped the organization.
//             Restart the function to get a successful completion.

#define ERROR_MEM_ORGANIZE_TERMFAILED                       92
//* Define statement owner: DOE, LEE
// Text:       Termination of organizing the Translation Memory
//             '%1' failed.
// HelpTitle:  OpenTM2 - Help
// Help:       The termination of the organize process failed due
//             to an internal error.
//             Close and restart your OpenTM2 application.

#define ERROR_MEM_NOT_REPLACED                              93
//* Define statement owner: DOE, LEE
// Text:       In Translation Memory '%1' a request to replace a segment
//             was made, but there is a more recent version of the
//             segment already in the Translation Memory.
// HelpTitle:  OpenTM2 - Help
// Help:       A more recent version of the segment you are
//             trying to replace is already in the Translation Memory.
//             Check if the date and time on your personal computer are
//             correct. If not, correct them and retry. If they are correct,
//             run the Organize function for that Translation Memory.

#define ERROR_MEM_NOSPACE                                   94
//* Define statement owner: DOE, LEE
// Text:       Disk '%2' does not have enough free space to merge or import
//             Translation Memory '%1'.
//             You need %3 bytes free on the target disk to complete this.
// HelpTitle:  OpenTM2 - Help
// Help:       The number of bytes shown is the maximum. You might need
//             less to complete the process.
//             Cancel the process, and
//             make more space available on the disk by deleting
//             unnecessary files before you try again.
//             If you ignore the lack of space, the Translation
//             Memory might not be merged or imported at all.

#define ERROR_MEMIMPORT_TM_EXISTS                           95
//* Define statement owner: DOE, LEE
// Text:       Translation Memory '%1' cannot be imported.
//             A Translation Memory with the same name already exists.
// HelpTitle:  OpenTM2 - Help
// Help:       The same Translation Memory name is in use for a different
//             project. To import the Translation Memory:
//             :ol.
//             :li.Create a new Translation Memory with a different name but
//             the same properties as the Translation Memory that
//             is already in your system.
//             :li.Merge the Translation Memory that is already in your
//             system into the newly created Translation Memory.
//             :li.Rename the Translation Memory name in all the Folders
//             where it appears to the newly-created
//             Translation Memory name.
//             Delete the Translation Memory that is already in your
//             system.
//             :li.Restart the Import process.
//             :eol.

#define ERROR_MEMIMPORT_ABNORMALLY                          96
//* Define statement owner: DOE, LEE
// Text:       The import of Translation Memory '%1'
//             has terminated abnormally.
// HelpTitle:  OpenTM2 - Help
// Help:       An abnormal situation occurred that caused
//             the import process to stop before it was successfully
//             completed. First, close and restart your
//             OpenTM2 application.
//             If the error occurs again, reinstall OpenTM2.

#define ERROR_NO_MEM_NAME                                   97
//* Define statement owner: DOE, LEE
// Text:       No Translation Memory name was specified.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to import data into a Translation Memory you have
//             to specify an existing or a new Translation Memory name.

#define ERROR_MEM_DESTROYED                                 98
//* Define statement owner: DOE, LEE
// Text:       Translation Memory '%1' is damaged and cannot
//             be recovered.
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory cannot be used any longer. Delete it
//             and create it again from a backup copy
//             before you import the data again.

#define ERROR_MEM_CREATE_DISKFULL                           99
//* Define statement owner: DOE, NEW
// Text:       The disk is full. The Translation Memory was not created.
// HelpTitle:  OpenTM2 - Help
// Help:       Make more space on the disk before you create a new
//             Translation Memory, or create it on another disk.
//             If you are organizing a Translation Memory, make
//             more space available on the specified disk.
//* -------------------------------------------------------
//*     Table and List messages
//* -------------------------------------------------------

#define ERROR_NO_SUPPORTFILE_INSTALLED                     100
//* Define statement owner: SOK, LEE
// Text:       There is no support file installed
//             for language '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       There is no support file installed
//             and therefore nothing can be deleted.

#define ERROR_EXCLUSION_LIST_NOT_FOUND                     101
//* Define statement owner: DOE, LEE
// Text:       The exclusion list '%1' cannot be found or read.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified exclusion list is not available
//             in the system or cannot be read.
//             .br
//             If it is a list originally provided by the
//             OpenTM2 system,
//             you should reinstall OpenTM2.
//             If it is a self-created list, you should import
//             a backup copy, or create the list again.

#define ERROR_LANGUAGE_FILE_NOT_FOUND                      102
//* Define statement owner: DOE, LEE
// Text:       The language support file '%1' cannot be found or read.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified language support file is either inactive or
//             damaged. You should reinstall OpenTM2.

#define ERROR_NO_FORMAT_TABLE_AVA                          103
//* Define statement owner: DOE, LEE
// Text:       The markup table '%1' is not available.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified markup table is not available.
//             If it is a table originally provided by the
//             OpenTM2 system,
//             you should reinstall OpenTM2.
//             If it is a self-created table, you should import
//             a backup copy, or create the table again.

#define ERROR_NO_EXCLUSION_LIST_AVA                        104
//* Define statement owner: DOE, LEE
// Text:       The exclusion list '%1' is not available.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified exclusion list is not available.
//             If it is a list originally provided by the
//             OpenTM2 system,
//             you should reinstall OpenTM2.
//             If it is a self-created list, you should import
//             a backup copy, or create the list again.

#define ERROR_EXIT_NOTFOUND                                105
//* Define statement owner: XQG, NEW
// Text:       The user exit DLL '%1' for markup table '%2' was
//             not found.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified markup table user exit was renamed
//             or deleted.

#define NO_NEW_TAGTABLE_SELECTED                           106
//* Define statement owner: THO, LEE
// Text:       You need to specify a markup table name for import.
// HelpTitle:  OpenTM2 - Help
// Help:       The name of the markup table to be created during
//             import is required. It can be up to eight characters
//             long. No extension is required.

#define ERROR_TIMP_CANCEL                                  107
//* Define statement owner: DKS, LEE
// Text:       Do you really want to cancel markup table import?
// HelpTitle:  OpenTM2 - Help
// Help:       Canceling the markup table import process
//             results in no markup table being built.

#define ERROR_TIMP_COMPLETE                                108
//* Define statement owner: THO, LEE
// Text:       Markup table import has been completed successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.

#define ERROR_TEX_CANCEL                                   109
//* Define statement owner: DKS, LEE
// Text:       Do you really want to cancel markup table export?
// HelpTitle:  OpenTM2 - Help
// Help:       Cancelling the markup table export process
//             results in no markup table in external format being built.

#define ERROR_TEX_COMPLETE                                 110
//* Define statement owner: DKS, LEE
// Text:       Markup table export has been completed successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.

#define ERROR_NO_TABLE_SELECTED                            111
//* Define statement owner: THO, LEE
// Text:       No markup table has been selected for export.
// HelpTitle:  OpenTM2 - Help
// Help:       Select an existing markup table for export.

#define ERROR_INVALID_FORMAT                               112
//* Define statement owner: THO, LEE
// Text:       The external format of the markup table
//             is not valid. No markup table can be created.
// HelpTitle:  OpenTM2 - Help
// Help:       Your file does not conform to the defined SGML markup table
//             syntax. No markup table can be created. Check your
//             syntax.

#define ERROR_TABLE_EMPTY                                  113
//* Define statement owner: THO, LEE
// Text:       The selected markup table is empty.
//             No external format is created.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected markup table does not contain any data and
//             cannot be exported.

#define WARNING_DELETE_TAGTABLE                            114
//* Define statement owner: THO, NEW
// Text:       Are you sure that you want to delete markup table '%1'?
// HelpTitle:  OpenTM2 - Help
// Help:       Delete the markup table if you are sure you do not need it
//             any more.
//             .br
//             Hint:
//             .br
//             You might want to export the markup table before deleting
//             it to save its data.

#define ERROR_TABLE_NOT_DELETED                            115
//* Define statement owner: THO, LEE
// Text:       The markup table '%1' is in use at the moment and cannot be
//             deleted.
// HelpTitle:  OpenTM2 - Help
// Help:       Wait until the selected markup table is not being used
//             before you restart this function.

#define ERROR_SGML_TAG                                     116
//* Define statement owner: DKS, LEE
// Text:       There is an SGML syntax error in the file '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that the SGML syntax in the markup table is
//             correct.

#define ERROR_NO_LANG_SUPPORT_INSTALLED                    117
//* Define statement owner: DOE, NEW
// Text:       There are no language support files installed.
// HelpTitle:  OpenTM2 - Help
// Help:       No language support files are installed.
//             Run 'Display language list' on the 'Utilities'
//             pull-down to see which languages are available from the
//             installation diskettes. Close OpenTM2 and
//             install the language support files you need.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      no valid enabled language found in SOURCE.LNG file

#define ERROR_DIALOG_NOT_FINISHED                          118
//* Define statement owner: SOK, LEE
// Text:       Open and delete is not active until the
//             dialog is finished.
// HelpTitle:  OpenTM2 - Help
// Help:       To open the user defined abbreviations for
//             a given language or to delete the support files
//             the language list has to be displayed completely.

#define ERROR_NO_LANGUAGE_SELECTED                         119
//* Define statement owner: SOK, LEE
// Text:       No language has been selected.
// HelpTitle:  OpenTM2 - Help
// Help:       Select the language you want to delete.
//*
//* -------------------------------------------------------
//*     Folder messages
//* -------------------------------------------------------

#define ERROR_FOLPROP_CHANGE_OF_EDITOR                     120
//* Define statement owner: XQG, LEE
// Text:       Changing the editor means you need to analyze all
//             folder documents again. Any translations done in this
//             folder will be lost.\n
//             Do you want to change the editor?
// HelpTitle:  OpenTM2 - Help
// Help:       All the affected
//             documents need to be reanalyzed
//             if the editor is changed, because you cannot work on a
//             document with different editors at one time.
//             If you change the editor the translated
//             documents are deleted.
//             If you want to keep the documents as they are,
//             do not change the editor and leave the folder
//             properties dialog.

#define ERROR_FOLPROP_FOLDER_IS_OPEN                       121
//* Define statement owner: XQG, LEE
// Text:       You cannot change the editor while the folder is open.
//             Close the document list of this folder and try again.
// HelpTitle:  OpenTM2 - Help
// Help:       The folder editor cannot be changed if the document list
//             for this folder is open or a document of this folder
//             is in use.
//             Leave the folder properties dialog, and close the document
//             list window and any open Translation Environment windows for
//             this folder. Then open the folder properties dialog again to
//             change the translation editor.

#define ERROR_DELETE_FOLDER                                122
//* Define statement owner: XQG, LEE
// Text:       You cannot delete the folder '%1' because it is open.
// HelpTitle:  OpenTM2 - Help
// Help:       A folder cannot be deleted while it is open.
//             First close the folder by closing its document list,
//             and then try again.

#define ERROR_FOLEXP_DELSEL_OPTION                         123
//* Define statement owner: XQG, LEE
// Text:       The 'Delete folder' option cannot be used in conjunction with
//             the 'Select documents' option.
// HelpTitle:  OpenTM2 - Help
// Help:       Your folder might contain other documents.
//             In order not to lose these, the 'Delete
//             folder' option cannot be used when only single documents
//             are exported. You can, of course, delete the entire
//             folder.
//             Remove the mark from either 'Delete folder'
//             or 'Select documents' and start the export process again.

#define QUERY_DELETE_FOLDER                                124
//* Define statement owner: XQG, LEE
// Text:       Are you sure that you want to delete folder '%1' and all
//             the documents it contains?
// HelpTitle:  OpenTM2 - Help
// Help:       By deleting a folder you delete all the documents it contains.

#define INFO_DIC_SELECTED                                  125
//* Define statement owner: XQG, LEE
// Text:       The requested dictionary is already selected.
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary entries must be unique in the list
//             of selected dictionaries. A previously selected
//             dictionary cannot be selected again.

#define INFO_DIC_MAXSELECTED                               126
//* Define statement owner: XQG, LEE
// Text:       The allowed maximum number of dictionaries (10)
//             is already selected.
// HelpTitle:  OpenTM2 - Help
// Help:       The number of dictionaries that can be
//             associated with a folder, document or
//             opened for dictionary lookup
//             is limited to 10.

#define ERROR_XLATE_INPUTPARAMS                            127
//* Define statement owner: JOR, LEE
// Text:       An incorrect number of parameters was specified.
//             To invoke the program:\n
//               EQFXLATE  folder  infile  outfile\n
//             where\n
//               folder   specifies the folder name,\n
//               infile   specifies the fully-qualified input file name,\n
//               outfile  is the fully-qualified output file name.
// HelpTitle:  OpenTM2 - Help
// Help:       You tried to access OpenTM2 but you
//             specified the wrong number of parameters.
//             Please ensure that you have provided the folder and
//             the input and output file names.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_FIELD_INPUT                                  128
//* Define statement owner: XQG, LEE
// Text:       One or more fields are empty or filled with incorrect data.
// HelpTitle:  OpenTM2 - Help
// Help:       The cursor is positioned at the first field that is
//             empty or contains wrong data.
//             Supply or correct the input data.

#define ERROR_EXPFOLDER_NOT_DELETED                        129
//* Define statement owner: XQG, LEE
// Text:       The exported folder '%1' is in use at the moment and cannot
//             be deleted.
// HelpTitle:  OpenTM2 - Help
// Help:       Wait until the selected exported folder is not used
//             any more before you restart this function.

#define WARNING_DELETE_EXPORTED_FOLDER                     130
//* Define statement owner: XQG, LEE
// Text:       Are you sure that you want to delete the exported
//             folder '%1'?
// HelpTitle:  OpenTM2 - Help
// Help:       If you choose to delete the exported folder, you will not
//             delete the actual
//             folder but only the exported copy of it.

#define ERROR_CREATE_SUBDIR                                131
//* Define statement owner: XQG, LEE
// Text:       The folder cannot be created due to file-handling errors.
// HelpTitle:  OpenTM2 - Help
// Help:       Either the disk is full, or a file with the name used
//             by OpenTM2 for folder directories exists
//             in the OpenTM2
//             system directory, or the disk is write-protected.
// Add.Info:   message may be obsolete

#define ERROR_FORMAT_NOTFOUND                              132
//* Define statement owner: XQG, LEE
// Text:       The markup table '%1' was not found.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified markup table was renamed or deleted.
//             If you are analyzing a document, end the
//             Analysis dialog, select 'Properties...' from the 'File'
//             pull-down,
//             and select a different format on the properties window.

#define ERROR_MEMORY_NOTFOUND                              133
//* Define statement owner: XQG, LEE
// Text:       Translation Memory '%1' was not found.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified Translation Memory might have been renamed,
//             deleted or is stored on a secondary drive or LAN drive
//             not accessible any more.

#define ERROR_FOLEXP_FOLDER_EXISTS                         134
//* Define statement owner: XQG, NEW
// Text:       There is already an exported folder '%1' on drive '%2'.
//             If you continue, the new export will overwrite the
//             exported folder.
// HelpTitle:  OpenTM2 - Help
// Help:       An exported folder with the given name exists
//             on the export target drive. Either this exported
//             folder is from a previous export to this drive or
//             another user has exported
//             a folder with the same name to the target drive.
//             :p.
//             Select OK if you are sure that the existing exported
//             folder may be overwritten by the current export.
//             :p.
//             Cancel the folder export operation if you don't want
//             to overwrite the existing exported folder.

#define ERROR_DIC_NOTFOUND                                 135
//* Define statement owner: XQG, LEE
// Text:       The dictionary '%1' was not found.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified dictionary might have been renamed or deleted.

#define MESSAGE_FOLIMP_TERMINATED                          136
//* Define statement owner: XQG, LEE
// Text:       The import of folder '%1' to drive '%2' terminated
//             abnormally due to a previously reported error
//             condition. Some parts of the folder might have been imported.
// HelpTitle:  OpenTM2 - Help
// Help:       Either a stop or close request was issued, or another
//             abnormal situation occurred during the import process.
//             Correct the condition that caused the error and retry
//             the folder import function.
// Cause:      User Cancel or previously reported error condition.
// Add.Info:   '%1' = folder name, '%2' = target drive

#define MESSAGE_FOLIMP_COMPLETED                           137
//* Define statement owner: XQG, LEE
// Text:       The folder '%1' was imported successfully to drive '%2'.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
// Add.Info:   '%1' = folder name, '%2' = target drive

#define ERROR_FOLIMP_FOLDEREXISTS                          138
//* Define statement owner: XQG, LEE
// Text:       A folder with the same name already exists.
//             Import is stopped.
//             Although these folders have the same
//             names, their contents could be different.
// HelpTitle:  OpenTM2 - Help
// Help:       The import of another folder with the same name is not
//             allowed, since the two folders could contain different
//             documents, or might have different folder properties.
// Cause:      Unique folder IDs mismatch.

#define ERROR_EDITOR_NOT_FOUND                             139
//* Define statement owner: XQG, LEE
// Text:       The editor '%1' specified
//             in the folder could not be found.
// HelpTitle:  OpenTM2 - Help
// Help:       The properties for the specified editor could not be
//             found. Either the editor properties have been deleted
//             or this is an imported folder and the editor specified
//             is not installed.

#define ERROR_FOLIMP_NOPROPFILE                            140
//* Define statement owner: XQG, LEE
// Text:       The folder properties found during import are either
//             missing or incorrect.
// HelpTitle:  OpenTM2 - Help
// Help:       The imported folder does not contain folder
//             properties, or the ones it does contain
//             are from another folder.
//             Either the previously exported folder file is damaged or
//             its name has been changed.
//             Rename the exported folder file to its original
//             name and retry the import of the folder.
// Cause:      No or wrong property file found in folder package

#define ERROR_CREATE_FOLDER                                141
//* Define statement owner: XQG, LEE
// Text:       A new folder cannot be created.
// HelpTitle:  OpenTM2 - Help
// Help:       You cannot create a new folder due to errors reported
//             before this message. Check the messages that were
//             displayed previously.

#define ERROR_CREATE_FOLDERLIST                            142
//* Define statement owner: XQG, LEE
// Text:       A new folder list cannot be created.
// HelpTitle:  OpenTM2 - Help
// Help:       A new folder list cannot be created
//             because OpenTM2 allows only one folder list.

#define ERROR_FOLDER_EXISTS                                143
//* Define statement owner: XQG, LEE
// Text:       The folder '%1' already exists and is open.
// HelpTitle:  OpenTM2 - Help
// Help:       Try another folder name.

#define ERROR_DESCRIPTION_MISSING                          144
//* Define statement owner: XQG, LEE
// Text:       You have not specified a folder description.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       A folder description is not required,
//             but it makes it easier to identify
//             the folders.
//             Continue if you do not want to specify a description.
// Type:       MB_YESNO

#define ERROR_NODICT_SELECTED                              145
//* Define statement owner: XQG, LEE
// Text:       No folder dictionary has been selected.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       If you do not select a dictionary now, no dictionary
//             window will be displayed in translation environment for
//             this folder. However, you can select a dictionary later
//             by changing the folder properties.
// Type:       MB_YESNO

#define ERROR_NEWFOLDER_EXISTS                             146
//* Define statement owner: XQG, LEE
// Text:       A folder with the name '%1' already exists. Specify a new name.
// HelpTitle:  OpenTM2 - Help
// Help:       Give the folder a name that does not exist already.
// Type:       MB_CANCEL
// Cause:      There is already a folder with the name specified
// Cause:      in the folder name entry field of the New Folder dialog.
// Cause:      in order to avoid confusion, folder names
// Cause:      need to be unique.
// Cause:      therefore, please specify another one.
// Add.Info:   name of existing folder

#define ERROR_FOLIMP_NOSPACE                               147
//* Define statement owner: XQG, LEE
// Text:       There is insufficient space on disk '%2' to import
//             folder '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Make more space available on the disk by deleting
//             unnecessary files, or import the folder to another disk.
// Cause:      Insufficient disk space to unload the package into
// Cause:      the import directory on the target drive.
// Add.Info:   '%1' = name of imported folder, '%2' = target drive

#define ERROR_FOLEXP_NOSPACE                               148
//* Define statement owner: XQG, LEE
// Text:       Disk '%1' is full. The export function cannot continue.
// HelpTitle:  OpenTM2 - Help
// Help:       Make more space available on the disk by deleting
//             unnecessary files or export the folder to another disk.
// Cause:      Disk full condition while exporting files.
// Add.Info:   '%1' = target drive

#define ERROR_FOLEXP_NEWDISK                               149
//* Define statement owner: XQG, LEE
// Text:       The diskette in drive '%1' is full. Insert an empty diskette
//             and press Enter to continue.
// HelpTitle:  OpenTM2 - Help
// Help:       Insert an empty, formatted diskette in the specified drive and
//             continue.
// Cause:      Disk full condition while exporting files.
// Add.Info:   '%1' = target drive

#define ERROR_FOLEXP_EXPORTDISK                            150
//* Define statement owner: XQG, LEE
// Text:       The diskette in drive '%1' is from the current export set.
//             Insert an empty diskette and continue.
// HelpTitle:  OpenTM2 - Help
// Help:       Either you have not yet removed the last diskette, or you
//             inserted a diskette which has already been used within this
//             export process.
// Cause:      Disk has not been removed. Disk is from current export set.
// Add.Info:   '%1' = target drive

#define ERROR_XLATE_FOLDER_NOT_EXIST                       151
//* Define statement owner: JOR, LEE
// Text:       The specified folder '%1' does not exist or is
//             not accessible.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that the specified folder exists
//             and that its name is spelled correctly.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define MESSAGE_FOLEXP_TERMINATED                          152
//* Define statement owner: XQG, LEE
// Text:       The export of folder '%1' to drive '%2' has terminated
//             abnormally.
// HelpTitle:  OpenTM2 - Help
// Help:       The export process stopped before it was successfully
//             completed. Either you issued a stop or close request, or
//             another abnormal situation occurred during the export process.
//             Try exporting the folder again.
// Cause:      User Cancel or previously reported error condition
// Add.Info:   '%1' = folder name, '%2' = target drive

#define MESSAGE_FOLEXP_COMPLETED                           153
//* Define statement owner: XQG, LEE
// Text:       The export of folder '%1' to drive '%2' completed successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
// Add.Info:   '%1' = folder name, '%2' = target drive

#define MESSAGE_FOLEXP_FIRSTDISK                           154
//* Define statement owner: XQG, LEE
// Text:       Reinsert the first diskette of the export set into drive '%1'
//             and continue.
// HelpTitle:  OpenTM2 - Help
// Help:       The first diskette is required again to supply administrative
//             information about the exported folder.
// Cause:      The first diskette is required to rewrite the file list.
// Add.Info:   '%1' = target drive

#define ERROR_FOLEXP_NOTFIRSTDISK                          155
//* Define statement owner: XQG, LEE
// Text:       This is not the first diskette of the export set. Insert
//             the correct diskette into drive '%1' and continue.
// HelpTitle:  OpenTM2 - Help
// Help:       The diskette in the specified drive is not the first
//             diskette used in this export.
//             The first diskette is required again to supply administrative
//             information about the exported folder.
// Cause:      This is not the first diskette of the export set.
// Add.Info:   '%1' = target drive

#define MESSAGE_FOLIMP_NEWDISK                             156
//* Define statement owner: XQG, LEE
// Text:       Insert the export disk '%2' in drive '%1' and continue.
// HelpTitle:  OpenTM2 - Help
// Help:       Insert the requested disk into the specified drive and
//             continue.
//             The order of diskettes is not arbitrary. The sequence of
//             importing a folder is determined by the order in which
//             a folder was exported.
// Cause:      Current disk processed completely and package not yet complete.
// Add.Info:   '%1' = target drive, '%2' = disk number

#define ERROR_FOLIMP_WRONGDISK                             157
//* Define statement owner: XQG, LEE
// Text:       This disk is not the correct one. Insert the export
//             disk '%2' in drive '%1' and continue.
// HelpTitle:  OpenTM2 - Help
// Help:       Insert the requested disk in the specified drive and
//             continue.
//             The order of diskettes is not arbitrary. The sequence of
//             importing a folder is determined by the order in which
//             a folder was exported.
// Cause:      User selected OK on MESSAGE_FOLIMP_NEWDISK, but the drive
// Cause:      does not contain the correct disk.
// Add.Info:   '%1' = target drive, '%2' = disk number

#define ERROR_FOLEXP_CANCEL                                158
//* Define statement owner: XQG, LEE
// Text:       Do you really want to cancel the export of folder '%1'?
// HelpTitle:  OpenTM2 - Help
// Help:       If you cancel the export of this folder
//             no files are exported.

#define ERROR_FOLIMP_CANCEL                                159
//* Define statement owner: XQG, LEE
// Text:       Do you really want to cancel the import of folder '%1'?
// HelpTitle:  OpenTM2 - Help
// Help:       If you cancel the import process the folder is
//             only partially imported. This could prevent all of the
//             documents that you want to translate from being
//             available.
//* -------------------------------------------------------
//*     Document messages
//* -------------------------------------------------------

#define NOT_DELETED                                        160
//* Define statement owner: DOE, LEE
// Text:       The document '%1' cannot be overwritten.
// HelpTitle:  OpenTM2 - Help
// Help:       You specified that the document is to be overwritten.
//             Due to a system error the document cannot be overwritten.
//             Close OpenTM2 and try again.

#define WARNING_DELETE_DOCUMENT                            161
//* Define statement owner: DOE, LEE
// Text:       Do you really want to delete document '%1'?
// HelpTitle:  OpenTM2 - Help
// Help:       Deleting a document means that both its original and its
//             translated version are deleted. However, the translated
//             segments remain stored within the appropriate Translation
//             Memory.

#define ERROR_MORE_THAN_ONE_SELECTED                       162
//* Define statement owner: DOE, LEE
// Text:       More than one document is selected.
// HelpTitle:  OpenTM2 - Help
// Help:       You can select only one document at a time for this action.

#define ERROR_DOCIMP_CANCEL                                163
//* Define statement owner: XQG, LEE
// Text:       Do you really want to cancel document import?
// HelpTitle:  OpenTM2 - Help
// Help:       If you cancel the document import process,
//             not all the documents will be imported into the
//             selected folder.

#define ERROR_NO_SNOMATCH_FOR_DOC                          164
//* Define statement owner: XQG, NEW
// Text:       There is no untranslated segments file for document '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       You have asked to export untranslated segments files of the
//             documents. For the referenced document no untranslated
//             segments file exists, probably because the document has been
//             analyzed without the 'Create file containing untranslated
//             segments' options.
//             :p.
//             To create the untranslated segments file, cancel the
//             document export and analyze the document with the above option.

#define ERROR_NOT_ANALYZED                                 165
//* Define statement owner: JOR, LEE
// Text:       The document cannot be translated
//             because it has not been analyzed yet.
// HelpTitle:  OpenTM2 - Help
// Help:       Before translation, a document needs to be
//             analyzed (select 'Analyze...' from the 'File' pull-down).

#define ERROR_EXP_FILE_EXISTS_ALREADY                      166
//* Define statement owner: XQG, NEW
// Text:       There is already an exported file of document '%1'
//             on drive '%2'.\n
//             Do you want to replace it?
// HelpTitle:  OpenTM2 - Help
// Help:       You have asked to export a document to the
//             specified drive in OpenTM2 format.
//             However,
//             this exported document exists already.
//             Either replace the file, or exit without
//             replacing it.

#define ERROR_DELETE_DOCUMENT                              167
//* Define statement owner: DOE, LEE
// Text:       The document is in use and cannot be deleted.
// HelpTitle:  OpenTM2 - Help
// Help:       The document is either open or is in use at the moment by
//             another process, such as Word Count or Text Analysis. Wait
//             until this process has finished and try again.

#define ERROR_DOCEXP_CANCEL                                168
//* Define statement owner: CHM
// Text:       Do you really want to cancel document export?
// HelpTitle:  OpenTM2 - Help
// Help:       If you cancel the document export process,
//             not all the documents will be exported from the
//             selected folder.

#define ERROR_DOCIMP_TO_MANY_DOC_IN_FLD                    169
//* Define statement owner: CHM
// Text:       You reached the maximum number (%1) of documents
//             allowed in folder '%2'. Import terminated.
// HelpTitle:  OpenTM2 - Help
// Help:       Create another folder for your documents, or
//             delete no longer needed documents from the
//             selected folder.

#define ERROR_DOC_NOT_ANALYZED                             170
//* Define statement owner: XQG, NEW
// Text:       The document %1 cannot be used for global
//             find and change
//             because it has not been analyzed yet.
//             Continue with next document?
// HelpTitle:  OpenTM2 - Help
// Help:       Before a document can be used for global find
//             and change operations, it needs to be
//             analyzed (select "Analyze..." from the "File" pull-down).

#define INFO_DOC_NO_MORE_HIT                               171
//* Define statement owner: XQG, NEW
// Text:       Search complete. No more occurrences
//             of "%1" found.
// HelpTitle:  OpenTM2 - Help
// Help:       The string being searched was not found anymore
//             in the rest of the document(s).

#define INFO_DOC_NO_HIT                                    172
//* Define statement owner: XQG, NEW
// Text:       Search complete. No occurrence of "%1" found.
// HelpTitle:  OpenTM2 - Help
// Help:       The string being searched was not found
//             in any of the documents being searched.

#define INFO_DOC_CHANGED                                   173
//* Define statement owner: XQG, NEW
// Text:       Global change completed. %1 occurrences of "%2"
//             changed to "%3".
// HelpTitle:  OpenTM2 - Help
// Help:       The search string has been replaced
//             the given number of times.

#define ERROR_FOLFIND_SAVE_SEGFILE                         174
//* Define statement owner: XQG, NEW
// Text:       The document "%1" could not be saved to disk.\n
//             Do you want to continue and skip this document?
//* Displayed Buttons: OK CANCEL */
// HelpTitle:  OpenTM2 - Help
// Help:       The system could not save the specified file.
//             Ensure that there is enough space on the specified disk.
//             If necessary, make more space available, or choose
//             another disk.
//             .br
//             If you continue, the document is skipped, and the next
//             document is processed. Leaving this function stops
//             the current process.

#define ERROR_FOLFIND_DOC_INUSE                            175
//* Define statement owner: XQG, NEW
// Text:       The document "%1" is in use
//             by another process. It will be ignored in Global Find
//             and Change.\n
//             Do you want to continue anyway?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       The document is in use by another process.
//             You can either skip
//             this document, or stop Global Find and Change and wait
//             until the running processes are finished before
//             you try again.

#define ERROR_FOLFIND_DOCLOAD                              176
//* Define statement owner: XQG, NEW
// Text:       The document "%1" could not be accessed by the
//             global find and change process.\n
//             Do you want to continue with the next document?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       An error occurred while processing one of the documents.
//             The specified document might not exist or could be
//             in use by another process.
//             .br
//             If you continue, the document where the error occurred
//             is skipped and the next document (if there is one)
//             is processed.

#define INTER_NO_INTER_FILE                                177
//* Define statement owner: XJR, NEW
// Text:       The specified file '%1' is not an Interleaf file.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to analyze is not an Interleaf
//             file and hence could not be analyzed.
//             Please check the file type and specify the correct
//             markup table for analysis.
// Add.Info:   '%1'  file to be analyzed

#define ERROR_FOLFIND_DOC_STILLINUSE                       178
//* Define statement owner: XQG, NEW
// Text:       The document "%1" is in still open or in use
//             by another process. Close the document or stop the
//             process before you continue.
//* Displayed Buttons: CANCEL */
// HelpTitle:  OpenTM2 - Help
// Help:       The document is open or in use by another process.
//             Before you continue using the Global Find
//             and Change window you have to close the document
//             or stop the process

#define ERROR_MAX_READONLY_TM                              179
//* Define statement owner: XJR
// Text:       You have already selected the maximum number of
//             Translation Memory databases.
// HelpTitle:  OpenTM2 - Help
// Help:       The number of Translation Memory
//             databases
//             that can be used for search-only Translation Memory
//             databases is 4.
//             Since you are already using the maximum number of
//             Translation Memory databases
//             allowed this Translation Memory
//             is not added to the list.
//* -------------------------------------------------------
//*     Dictionary Messages for Artemis
//* -------------------------------------------------------

#define NO_TROJA_FORMAT                                    180
//* Define statement owner: DKS, LEE
// Text:       Files in OpenTM2 format cannot be processed.
// HelpTitle:  OpenTM2 - Help
// Help:       This function has not been implemented yet. If you want to
//             keep the dictionaries in OpenTM2 format,
//             use Folder 'Import.../Export...' instead.

#define NO_NEW_DICTIONARY_SELECTED                         181
//* Define statement owner: DKS, LEE
// Text:       You have not specified a dictionary name.
// HelpTitle:  OpenTM2 - Help
// Help:       The name of the dictionary that is to be created during
//             dictionary import is needed. It can be up to eight characters
//             long - no extension is required.

#define INVALID_OS2_FILENAME                               182
//* Define statement owner: DKS, LEE
// Text:       The file name specification is not valid.
// HelpTitle:  OpenTM2 - Help
// Help:       The name of the file can be up to eight characters long
//             and the extension up to three.

#define PROTECTED_DICT_AND_NO_TERM                         183
//* Define statement owner: XQG, NEW
// Text:       The term '%1' was not found in the selected
//             dictionaries and the term cannot be added
//             as all selected dictionaries are protected.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected term was not found in the dictionaries
//             being searched.
//             :p.
//             As no unprotected dictionaries are selected the term
//             cannot be added to a dictionary. Either unprotect
//             one of the selected dictionaries or add an unprotected
//             dictionary to the selected ones to add the term.

#define ERROR_DIMP_CANCEL                                  184
//* Define statement owner: JOR, DKS, LEE
// Text:       Do you really want to cancel the dictionary import process?
// HelpTitle:  OpenTM2 - Help
// Help:       Cancelling dictionary import results in
//             a partially built dictionary, which can be used for
//             other tasks (lookup or edit). However,
//             terms will be missing.

#define ERROR_NO_EXPORT_NAME                               185
//* Define statement owner: XQG, NEW
// Text:       You have not specified a name for the output file.
// HelpTitle:  OpenTM2 - Help
// Help:       The name of the SGML file that is to be created during
//             dictionary export is needed.

#define INVALID_NAME                                       186
//* Define statement owner: DKS, LEE
// Text:       The dictionary name does not follow the naming conventions.
// HelpTitle:  OpenTM2 - Help
// Help:       A dictionary name can have up to
//             eight alphanumeric characters. Do not include a file
//             extension.

#define ERROR_DIMP_INVSTYLE                                187
//* Define statement owner: DKS, LEE
// Text:       The tag attribute style '%1' is not valid.
//             The error occurred near the entry '%2'.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       The tag attribute syntax is incorrect. Tag attributes occur
//             in user-defined tags (for example, ID= ) in
//             dictionary files. Either stop
//             processing and correct the attribute (if you are not sure
//             about the SGML syntax, refer to the chapter on dictionaries
//             in the OpenTM2 book)
//             or continue processing, in which case the entire entry with
//             the attribute syntax error is skipped. The entry is not added
//             to the dictionary.

#define ERROR_DIMP_INVTAG                                  188
//* Define statement owner: JOR, DKS, LEE
// Text:       The SGML tag '%1' used in the SGML-based file is not valid.
//             The error occurred near the entry '%2'.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       The SGML tag syntax is incorrect. This can happen if start
//             and end tags do not match or contain an error, or if the tag
//             has been positioned incorrectly (not assigned to the right
//             level in a dictionary file) or left out altogether.
//             Either stop processing and correct the syntax
//             (if you are not sure about
//             the SGML syntax, refer to the chapter on dictionaries
//             in the OpenTM2 book).
//             or continue processing, in which case the entire entry with
//             the tag syntax error is skipped. The entry is not added
//             to the dictionary.

#define ERROR_DIMP_NOTINMAP                                189
//* Define statement owner: DKS, LEE
// Text:       The dictionary field "%1" with a value of "%2" does not correspond 
//             to the dictionary structure. Data in
//             this field will be lost.\n
//             Do you want to ignore this error and continue?
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary entry contains a field that is not included
//             in the dictionary structure and so it cannot be added to the
//             entry. Some information is lost if you continue. To avoid
//             losing information, include all entry fields that can
//             occur in any entry in the dictionary structure (maptable).

#define ERROR_DIMP_DATATOOBIG                              190
//* Define statement owner: LUE DKS
// Text:       The dictionary entry %1 you are trying to import is too big.
// HelpTitle:  OpenTM2 - Help
// Help:       The allowed size for a dictionary entry is up to 60000 
//             characters.
//             Try to shorten the dictionary entry and repeat the import.

#define ERROR_DIMP_COMPLETE                                191
//* Define statement owner: JOR, DKS, NEW
// Text:       Importing into dictionary '%1' has been successfully
//             completed. The dictionary contains %2 entries.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you may continue working.

#define ERROR_NOLONGNAMESUPPORT                            192
//* Define statement owner: XQG
// Text:       Drive %1 does not support long file names.
//             Do you want to export the folder using the
//             folder short name '%2'?
// HelpTitle:  OpenTM2 - Help
// Help:       The target drive for the folder being exported
//             does not support long file names. You can either
//             export the folder using the folder short name or
//             can cancel this message and select a different drive
//             as target for the folder export.

#define INVALID_FILE_FOR_IMPORT                            193
//* Define statement owner: XQG, NEW
// Text:       The format of the dictionary file is incorrect.
// HelpTitle:  OpenTM2 - Help
// Help:       You are trying to import a file which is not an SGML-
//             based dictionary file.

#define ERROR_DIMP_INVMAP                                  194
//* Define statement owner: DKS, LEE
// Text:       The dictionary cannot be created because of an error in
//             the maptable.
// HelpTitle:  OpenTM2 - Help
// Help:       The maptable section determines which entry fields the
//             dictionary contains and must be correct before a
//             dictionary can be created.
//             Ensure that all entry fields have correct levels assigned to
//             them (for example: entry, homonym, sense, and target).
//             Also ensure that the SGML syntax is correct (that all start
//             and end tags match).
//             If you are not sure about
//             the SGML syntax, refer to the chapter on dictionaries in the
//             OpenTM2 book.

#define MERGE_DICTIONARIES                                 195
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' already exists.\n
//             Do you want to merge the file '%2' into this dictionary?
// HelpTitle:  OpenTM2 - Help
// Help:       The selected dictionary exists already.
//             Its structure is applied to the file when they are merged.
//             Ensure that this is the dictionary structure you want for
//             this file.

#define ERROR_DEX_CANCEL                                   196
//* Define statement owner: DKS, LEE
// Text:       Do you really want to cancel the dictionary export process?
// HelpTitle:  OpenTM2 - Help
// Help:       Canceling the dictionary export function results in
//             a partially built SGML file that can be imported,
//             although some terms might be missing.

#define ERROR_DEX_COMPLETE                                 197
//* Define statement owner: DKS, LEE
// Text:       Dictionary '%1' has been exported successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.

#define ERROR_SUBFOLDER_EXISTS                             198
//* Define statement owner: XQG, LEE
// Text:       A subfolder with the name '%1' already exists
//             within the current parent folder. Specify a new name.
// HelpTitle:  OpenTM2 - Help
// Help:       Give the subfolder a name that does not exist already
//             in the current parent folder.
// Type:       MB_CANCEL
// Add.Info:   name of existing subfolder

#define ERROR_SEARCH_ENTRY                                 199
//* Define statement owner: DKS, NEW
// Text:       An error occurred while retrieving entry '%1' from
//             the dictionary. Do you want to skip it?
// HelpTitle:  OpenTM2 - Help
// Help:       The entry is damaged and hence cannot be retrieved.
//             Skipping the entry enables further processing. If you
//             opt not to continue, processing will stop.

#define ERROR_WRITING_TO_SGML_FILE                         200
//* Define statement owner: DKS, LEE
// Text:       An error occurred while writing to the dictionary export file.
// HelpTitle:  OpenTM2 - Help
// Help:       Check available space on the drive where the dictionary export
//             file (SGML file) is being built.

#define ERROR_DICT_CORRUPT                                 201
//* Define statement owner: DKS, LEE
// Text:       The dictionary you are trying to export is damaged.
// HelpTitle:  OpenTM2 - Help
// Help:       Select 'Organize' from the 'File'
//             pull-down
//             to reorganize the dictionary before
//             you try to export it again.

#define FILE_EXISTS                                        202
//* Define statement owner: DKS, LEE
// Text:       The file '%1' already exists.\n
//             Do you want to overwrite it?
// HelpTitle:  OpenTM2 - Help
// Help:       If you do not specify a new file name, the file
//             is overwritten by the new data.

#define ERROR_WRITING                                      203
//* Define statement owner: DKS, LEE
// Text:       Not all of the data could be written.
//             Check the drive '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Check the amount of space left on the specified
//             disk drive. Make space on this disk drive and restart
//             the function, or select another disk drive
//             if the function allows for it.
//             If you want to export a dictionary or a Translation Memory
//             database to a diskette and exceed maximum
//             diskette capacity use the folder 'Export...' option
//             on the 'File' pull-down.
//             For this create a new folder and associate the
//             dictionary or Translation Memory with this folder. Then
//             select folder export.

#define ERROR_SUBFOLDERNAME_REQUIRED                       204
//* Define statement owner: XQG
// Text:       Please specify the name of the new subfolder.
// HelpTitle:  OpenTM2 - Help
// Help:       Give the subfolder a name that does not exist already
//             in the current parent folder. The name may have up to
//             256 characters.
// Type:       MB_CANCEL

#define DIFFERENT_PROFILES                                 205
//* Define statement owner: DKS, LEE
// Text:       The dictionary entry structures are not identical.
//             Merging dictionaries will lead to a loss of information.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       The structures of the dictionaries you want to merge are not
//             identical. The entry structure of the dictionary
//             into which you are merging
//             will be used. This structure could contain fewer entry
//             fields, and, therefore, merging can lead to a loss of data.

#define WARNING_DELETE_DICTIONARY                          206
//* Define statement owner: XQG, LEE
// Text:       Are you sure that you want to delete dictionary '%1',
//             which is located locally on your system?
// HelpTitle:  OpenTM2 - Help
// Help:       Delete the dictionary if you are sure you do not need it
//             any more.
//             .br
//             Hint:
//             .br
//             You might want to export the dictionary before deleting
//             it to save its data.

#define ERROR_DICT_NOT_DELETED                             207
//* Define statement owner: XQG, LEE
// Text:       Dictionary '%1' is in use at the moment and cannot be
//             deleted.
// HelpTitle:  OpenTM2 - Help
// Help:       Wait until the selected dictionary is not being used
//             any longer before you restart this function.

#define ERROR_DORG_CANCEL                                  208
//* Define statement owner: DKS, LEE
// Text:       Do you really want to cancel the dictionary organizing process?
// HelpTitle:  OpenTM2 - Help
// Help:       If you cancel this function, no newly organized dictionary is
//             created and the dictionary is still damaged.

#define ERROR_DORG_COMPLETE                                209
//* Define statement owner: DKS, LEE
// Text:       Dictionary '%1' has been successfully organized.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.

#define ERROR_ENTRYFIELD_NOT_DELETED                       210
//* Define statement owner: DKS, NEW
// Text:       The selected entry field cannot be deleted.
// HelpTitle:  OpenTM2 - Help
// Help:       The two entry fields 'headword' and 'translation' cannot
//             be deleted. A dictionary must have at least these two entry
//             fields.

#define ERROR_DIMP_INVHEADER                               211
//* Define statement owner: DKS, LEE
// Text:       There is an error in the dictionary header section.
//             Dictionary cannot be created.
// HelpTitle:  OpenTM2 - Help
// Help:       Check the header section of your SGML dictionary. There
//             might be a tag error or important tags could be missing.
//             The header section is required and cannot be left out.

#define ERROR_DEL_COPYRIGHTED                              212
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' that you want to delete is copyrighted.\n
//             Are you sure that you want to delete this dictionary?
// HelpTitle:  OpenTM2 - Help
// Help:       Copyrighted dictionaries cannot be modified
//             or exported. Because you cannot make a copy of the
//             dictionary (in SGML form, for instance) deleting it implies
//             that all of the data will be lost.

#define ERROR_NO_FILE_NAME                                 213
//* Define statement owner: DKS, LEE
// Text:       You have not specified the new dictionary name.
// HelpTitle:  OpenTM2 - Help
// Help:       The new dictionary name can have up to eight
//             characters and should not include an extension.

#define ERROR_DICTIONARY_EXISTS                            214
//* Define statement owner: DKS, LEE
// Text:       A dictionary named '%1' already exists.
//             Select a new name.
// HelpTitle:  OpenTM2 - Help
// Help:       When creating a new dictionary the name should be unique.
//             The dictionary name can have up to eight characters
//             and should not include an extension.

#define ERROR_DORG_COMPLETE2                               215
//* Define statement owner: DKS, LEE
// Text:       The organization of dictionary '%1' completed
//             successfully. %2 terms have been organized.
//             %3 terms were corrupted and had to be skipped.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.

#define ERROR_FIELDNAME_EMPTY                              216
//* Define statement owner: DKS, LEE
// Text:       The dictionary entry field name is either empty or
//             begins with a blank.
// HelpTitle:  OpenTM2 - Help
// Help:       Either you have not entered a dictionary field name or
//             the field name you have entered begins with a blank.
//             Dictionary field names can be any number of alpha-
//             numeric characters or special characters (including
//             blanks) but cannot begin with blanks. It is
//             a good idea to use short field names, because they fit
//             better on the dictionary display dialog panels.

#define NO_SPACE_FOR_EXPORT                                217
//* Define statement owner: DKS, LEE
// Text:       There is not enough space on disk '%1' to entirely export
//             the selected dictionary.\n
//             Do you still want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       The specified disk drive is full. Make space on this disk
//             drive and restart the function, or select another disk drive
//             if the function allows for this. If you continue, only part
//             of the dictionary is exported.

#define ERROR_PROP_CHANGE_DENIED                           218
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' is copyrighted and none of its features
//             can be changed.
// HelpTitle:  OpenTM2 - Help
// Help:       Copyrighted dictionaries cannot be modified in any way.
//             This also includes features such as the name of dictionary
//             entry fields and what information is to be displayed
//             on which panels.

#define ERROR_MAPTABLE_MISSING                             219
//* Define statement owner: DKS, LEE
// Text:       The dictionary file '%1' you are importing does not contain
//             a maptable showing the dictionary entry structure.
// HelpTitle:  OpenTM2 - Help
// Help:       The SGML dictionary file you are importing does not contain
//             a maptable section. The maptable section defines which
//             entry fields the dictionary must contain and it needs to be
//             included in every SGML dictionary file. For detailed
//             information, refer to the chapter on dictionaries in the
//             OpenTM2 book.

#define ERROR_PROP_ACCESS_DENIED                           220
//* Define statement owner: DKS, LEE
// Text:       The system is unable to access the properties belonging to
//             the dictionary '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Either the dictionary properties file cannot be found
//             or there is not enough space on the system disk drive
//             to access the property file.

#define NO_SPACE_FOR_IMPORT                                221
//* Define statement owner: DKS, NEW
// Text:       Disk '%2' is %3 bytes short on space to import
//             the dictionary '%1'.\n
//             Do you want to start importing nevertheless?
// HelpTitle:  OpenTM2 - Help
// Help:       The specified disk drive is full. Make space on this disk
//             drive and restart the function, or select another disk drive
//             if the function allows for this. Continue if you know that
//             you only want
//             to import part of the dictionary.

#define NO_SPACE_FOR_ORGANIZE                              222
//* Define statement owner: DKS, LEE
// Text:       Disk '%2' is short of space to organize
//             the dictionary '%1'.\n
//             Do you want to start organizing anyway?
// HelpTitle:  OpenTM2 - Help
// Help:       Make more space available on the disk by deleting
//             unnecessary files before you restart this function.
//             If you start organizing anyway, the process will stop
//             when the disk gets full.
// Type:       EQF_QUERY
// Severity:   YES NO
// Cause:      MemCheckSpaceForFile returns that disk space is short
// Add.Info:   '%1' = dict name, '%2' = disk drive

#define ERROR_MAPTABLE_TOO_LONG                            223
//* Define statement owner: DKS, LEE
// Text:       You have defined too many dictionary entry fields (more
//             than 38) in the maptable section of the SGML dictionary file.
// HelpTitle:  OpenTM2 - Help
// Help:       The maptable section determines which
//             entry fields the dictionary is to contain and must
//             be included in every SGML dictionary file. However, a
//             maptable cannot contain more than 38 entry fields.
//             Check the SGML maptable section. For detailed
//             information, refer to the chapter on dictionaries in the
//             OpenTM2 book.

#define ERROR_CLOSING_PROPS                                224
//* Define statement owner: DKS, LEE
// Text:       The system is unable to close the properties belonging to
//             the dictionary '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Check if there is enough space on the system disk drive,
//             because the system is unable to close the dictionary
//             properties.

#define ERROR_FILE_EXP_SIZE                                225
//* Define statement owner: LUE, LEE
// Text:       The markup table '%1' you want to export is empty.
// HelpTitle:  OpenTM2 - Help
// Help:       Empty markup tables cannot be exported.
//             The markup table was not created correctly and
//             it does not contain any data.

#define QUERY_DELETE_SUBFOLDER                             226
//* Define statement owner: XQG
// Text:       Are you sure that you want to delete subfolder
//             '%1' and all subfolders and documents it contains?
// HelpTitle:  OpenTM2 - Help
// Help:       Delete the subfolder if you are sure that
//             you do not need it any more.
//             All documents and all subfolders within the subfolder
//             will be deleted.



#define ERROR_FILE_IMP_SIZE                                227
//* Define statement owner: DKS, LEE
// Text:       The file '%1' you want to import is empty.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified file is empty.

#define NO_SPACE_FOR_CREATION                              228
//* Define statement owner: DKS, LEE
// Text:       There is not enough space on your '%1' disk to create
//             a new dictionary.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified disk drive is full. Make space on this disk
//             drive and restart the function, or select another disk drive
//             if the function allows for this.

#define ERROR_EQF_PATH_INVALID                             229
//* Define statement owner: DKS, LEE
// Text:       You are not authorized to work with system directories.
//             The file name '%1' is not valid.
// HelpTitle:  OpenTM2 - Help
// Help:       EQF is a system directory. Select another directory.

#define ERROR_PROCESS_STOPPED                              230
//* Define statement owner: DKS, LEE
// Text:       Processing stopped because of a system error.
//             Check the available memory.
// HelpTitle:  OpenTM2 - Help
// Help:       Your job was interrupted before completion. No files
//             were created.
//             Ensure that you have enough disk drive space.

#define NO_SPACE_FOR_TAGIMPORT                             231
//* Define statement owner: DKS, LEE
// Text:       There is not enough space on your '%1' disk to import
//             the selected markup table.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified disk drive is full. Make space on this disk
//             drive and restart the function, or select another disk drive
//             if the function allows for this.

#define FOLDER_MERGE                                       232
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' already exists.\n
//             Do you want to merge the folder dictionary into this
//             dictionary?
// HelpTitle:  OpenTM2 - Help
// Help:       A dictionary exists with the same name as the dictionary in
//             the folder you are importing. Merging these two dictionaries
//             means that you use the dictionary structure of the destination
//             dictionary. This could lead to a loss of data if the
//             dictionary structures differ. If you do not want to merge,
//             this dictionary is not imported. If you want to
//             use the folder dictionary, export the existing
//             dictionary and reimport it with a different name. Delete
//             the existing dictionary and restart the folder import
//             process.

#define ERROR_NO_SOURCELANG                                233
//* Define statement owner: XQG, NEW
// Text:       You have not specified a source language.
// HelpTitle:  OpenTM2 - Help
// Help:       The source language is required to work with
//             the folder.

#define ERROR_NO_TARGETLANG                                234
//* Define statement owner: XQG, NEW
// Text:       You have not specified a target language.
// HelpTitle:  OpenTM2 - Help
// Help:       The target language is required to work with
//             the folder.

#define ERROR_RENUMBER                                     235
//* Define statement owner: DKS, LEE
// Text:       The dictionary or Translation Memory '%1'
//             needs to be organized; it is
//             either damaged or it was created with
//             an older version of OpenTM2.\n
//             Please use the organize function to
//             organize the dictionary or Translation Memory.
// HelpTitle:  OpenTM2 - Help
// Help:       Your dictionary or Translation Memory is either
//             damaged or
//             it was created with an older version of OpenTM2.
//             :p.
//             Try organizing the dictionary or Translation
//             Memory
//             or reimport the SGML copy of the file.

#define ERROR_DICT_LOCKED                                  236
//* Define statement owner: DKS, LEE
// Text:       The selected dictionary or Translation Memory '%1'
//             is in use at the moment by another process or
//             by another user. The function you have selected
//             is stopped.
// HelpTitle:  OpenTM2 - Help
// Help:       If the resource is in use by another process
//             either wait until the other process has been completed
//             or interrupt it (cancel processing). Then
//             restart the last function you selected.
//             :p.
//             If the resource is in use by another user
//             wait until the resource is not in use
//             anymore. Then restart the last function you
//             selected.

#define ERROR_NO_SOURCE_LANG_SELECTED                      237
//* Define statement owner: DKS, LEE
// Text:       You have not specified a source language.
// HelpTitle:  OpenTM2 - Help
// Help:       Without a specified source language, no dictionary work can
//             be carried out.

#define ERROR_INV_PASSWORD                                 238
//* Define statement owner: DKS, LEE
// Text:       The password you have entered is not valid.
// HelpTitle:  OpenTM2 - Help
// Help:       A password can be any combination of alphanumeric
//             characters. The password must be at least four
//             but not more than eight characters long.

#define ERROR_FOLDERDICT_PROTECTED                         239
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' is protected. It cannot be merged with
//             non-protected dictionaries. Importing is stopped.
// HelpTitle:  OpenTM2 - Help
// Help:       Protected dictionaries are read-only dictionaries and
//             merging with non-protected dictionaries is not permitted.
//             Therefore, the import process was stopped.

#define ERROR_SYSTDICT_COPYRIGHTED                         240
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' available on the system is copyrighted.
// HelpTitle:  OpenTM2 - Help
// Help:       Copyrighted dictionaries cannot be changed.

#define ERROR_OVERWRITE_ENTRY                              241
//* Define statement owner: DKS, XQG, LEE
// Text:       The dictionary entry\n
//             '%1'\n
//             already exists.\n
//             Do you want to replace it?
// HelpTitle:  OpenTM2 - Help
// Help:       The existing dictionary entry is replaced by the new
//             entry. Data in the old entry that is not contained in the
//             new entry is lost.

#define ERROR_DICT_EXISTS_ON_OTHERDRIVE                    242
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' in the folder you have chosen to
//             import already exists on another drive.
//             Importing is not permitted.
// HelpTitle:  OpenTM2 - Help
// Help:       A dictionary exists with the same name as the dictionary
//             in the folder you are importing. It cannot be copied or
//             merged into the existing dictionary.

#define ERROR_NO_PASSWORD_ENTERED                          243
//* Define statement owner: DKS, LEE
// Text:       No password has been entered.
// HelpTitle:  OpenTM2 - Help
// Help:       A password can have any combination of alphanumeric
//             characters. The password must be at least four
//             but not more than eight characters long.

#define ERRMSG_WRONG_PASSWORD                              244
//* Define statement owner: DKS, LEE
// Text:       The password entered is not the one associated with
//             this object.
// HelpTitle:  OpenTM2 - Help
// Help:       The password entered was incorrect and access authorization
//             has not been granted.

#define ERROR_TROJADICT_PROTECTED                          245
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' you want to merge a folder dictionary
//             into is protected. The folder dictionary is not imported.
// HelpTitle:  OpenTM2 - Help
// Help:       Protected dictionaries are read-only dictionaries and
//             merging with other data is not permitted. If you wish
//             to import the folder dictionary, delete the existing protected
//             dictionary and reimport the folder.

#define ERROR_PASSWORDS_NOT_INDENTICAL                     246
//* Define statement owner: DKS, LEE
// Text:       The two passwords do not match.
// HelpTitle:  OpenTM2 - Help
// Help:       The two passwords entered need to be identical before
//             being accepted as a valid password.

#define ERROR_EXPORTDICT_PROTECTED                         247
//* Define statement owner: DKS, NEW
// Text:       The dictionary '%1' you want to export into SGML format
//             is protected and so it cannot be exported.
// HelpTitle:  OpenTM2 - Help
// Help:       Protected dictionaries are read-only dictionaries and
//             exporting is not permitted. Provided the password is known,
//             the protection flag for local dictionaries can be reset
//             on the properties dialog.
//             The protection flag for shared dictionaries however can only be
//             reset by the owner of the dictionary.

#define ERROR_ADDTODICT_PROTECTED                          248
//* Define statement owner: XQG, NEW
// Text:       The dictionary '%1' is protected. It cannot be used to add
//             found terms.
// HelpTitle:  OpenTM2 - Help
// Help:       Protected dictionaries are read-only dictionaries and
//             may not be changed.
//             Select an unprotected dictionary as output dictionary.

#define ERROR_SOURCE_LANG_MISSING                          249
//* Define statement owner: DKS, NEW
// Text:       No source language has been specified in the header section
//             of the SGML dictionary file '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       No dictionary work can be carried out if no source language
//             has been specified.  The source language belongs in the
//             header section of the SGML file between the start tag
//             &lt.source&gt. and the end tag &lt./source&gt.. Run
//             'Display language list' on the 'Utilities' pull-down
//             to see whether the required source language support
//             file is active and to check nomenclature.
//             For detailed information, refer to the chapter on
//             dictionaries in the OpenTM2 book.
//* -------------------------------------------------------
//*     Dictionary Lookup Messages
//* ------------------------------------------------------- */

#define ERROR_HEADWORD_CHANGED                             250
//* Define statement owner: XQG, LEE
// Text:       The term '%1' cannot be deleted because the headword has been
//             changed to '%2'.
// HelpTitle:  OpenTM2 - Help
// Help:       You have changed the headword of the term.
//             To delete the entry whose data is displayed, restore the
//             headword entry field to its original value and request the
//             delete function again.
//             To delete the entry whose headword is in the headword entry
//             field, leave the edit dialog and restart edit for the entry
//             to delete the entry.

#define WARNING_EDITDICT_PROTECTED                         251
//* Define statement owner: DKS, LEE
// Text:       The dictionary containing this entry is protected and
//             changes to this entry cannot be written to the
//             dictionary. Select another target dictionary.
// Cause:      User starts to change the data of one of the dictionary
// Cause:      fields or deletes or adds templates.
// HelpTitle:  OpenTM2 - Help
// Help:       Protected dictionaries are read-only and data cannot
//             be changed. You can either reset the protection flag on
//             the dictionary properties dialog and then save the changes
//             to the dictionary or select another dictionary listed below
//             to save to. If there are no other dictionaries listed,
//             leave this dialog and create a new one.

#define ERROR_EDITDICT_PROTECTED                           252
//* Define statement owner: XQG, LEE
// Text:       The selected target dictionary is protected and
//             cannot be used to save this dictionary entry.
//             Select another dictionary if there is one available.
// Cause:      User tries to save a dictionary entry into a protected
// Cause:      dictionary.
// HelpTitle:  OpenTM2 - Help
// Help:       Protected dictionaries are read-only and data cannot be
//             changed. If there are other dictionaries available, use
//             one of these. If there are no other dictionaries, you might
//             want to create a new one. However, the present
//             changes to the dictionary entries cannot be saved and are lost.

#define QUERY_EDITDICT_CONTINUE                            253
//* Define statement owner: XQG, LEE
// Text:       Do you want to continue processing terms marked
//             for dictionaries?
// HelpTitle:  OpenTM2 - Help
// Help:       The edit of a dictionary entry has been canceled.
//             Confirm the cancellation to continue processing the next term
//             marked for the dictionary, or
//             return to the lists dialog.

#define ERROR_ASDENTRY_CORRUPT                             254
//* Define statement owner: XQG, LEE
// Text:       The dictionary entry being processed is
//             damaged.
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary entry is damaged. Reorganize
//             the dictionary before you continue.

#define WARNING_DEL_OWN_REM_DICT                           255
//* Define statement owner: DKS, LEE
// Text:       As the LAN administrator or dictionary owner
//             you are authorized to delete the shared dictionary '%1'.
//             Are you sure that you want to delete it?
// HelpTitle:  OpenTM2 - Help
// Help:       Delete the dictionary if you are sure that no one needs
//             it any more. Remember that the dictionary is shared
//             and other users have access to it.
//             .br
//             Hint:
//             .br
//             You can export the dictionary before deleting
//             it to save its data.

#define WARNING_DEL_REM_DICT                               256
//* Define statement owner: DKS, NEW
// Text:       You are not the owner of the shared dictionary '%1'
//             and so are not authorized to physically delete it.
//             Deleting will prevent you alone from accessing it.
//             Are you sure that you want to delete it?
// HelpTitle:  OpenTM2 - Help
// Help:       A shared dictionary may only be deleted
//             by its owner. Deleting it as a non-owner merely implies
//             disallowing access and removing the dictionary from the
//             dictionary list window.

#define ERROR_IMPORTDICT_COPYRIGHTED                        257
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' being imported with the folder
//             is copyrighted and cannot be merged into the existing
//             non-copyrighted dictionary.
// HelpTitle:  OpenTM2 - Help
// Help:       It is not possible to merge the copyrighted dictionary
//             included in the exported folder into the existing
//             dictionary with the same name which is not copyrighted.


#define INFO_RESTART_REQUIRED                              258
//* Define statement owner: XQG
// Text:       You have to restart OpenTM2 in order
//             to activate the new mode for the folder and document
//             list windows.
// HelpTitle:  OpenTM2 - Help
// Help:       The changed mode for the folder list window and the
//             document list windows will be activated the next time
//             OpenTM2 is started.

#define WARNING_NO_LOCALE                                  259
//* Define statement owner: Bruno
// Text:       Codepage %1 couldn't be set during
//             DBCS conversion, document may be corrupt.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_SHAREDMEM_EXISTS                             260
//* Define statement owner: XQG
// Text:       There is already a shared Translation Memory with the name '%1'
//             on drive %2. Please change the Translation Memory name or
//             connect to the existing shared memory.
// HelpTitle:  OpenTM2 - Help
// Help:       A shared Translation Memory with the specified name exists already
//             on the given drive. Either specify another name for the
//             Translation Memory being created or connect the existine memory.
//             To access the shared Translation Memory leave the current window
//             and use 'Connect shared resources' to connect the shared
//             Translation Memory.

#define ERROR_SHAREDDICT_EXISTS                            261
//* Define statement owner: XQG
// Text:       There is already a shared dictionary with the name '%1'
//             on drive %2. Either change the dictionary name or
//             select another target drive.
// HelpTitle:  OpenTM2 - Help
// Help:       A shared dictionary with the specified name exists already
//             on the target drive. Either specify another name for the
//             dictionary being created or select another drive.
//             To access the shared dictionary leave the current window
//             and use 'Connect shared resources' to connect the shared
//             dictionary.


#define ERROR_LST_DISPLAY_ERROR                            262
//* Define statement owner: XQG
// Text:       Display of data failed. Either the list is too large
//             or the system is short on resources.
//             before saving it to disk.
// HelpTitle:  OpenTM2 - Help
// Help:       The list is too large to be displayed in the
//             edit window (Win95 and Win98 only support up to 64KB in an
//             edit window) or the system is short on system resources.
// Cause:      edit control reports failure


#define ERROR_IMP_FOLDER_EXISTS                            263
//* Define statement owner: XQG, LEE
// Text:       There is already a different folder named '%1'.
//             If you continue the imported folder will be merged
//             into the existing one.
// HelpTitle:  OpenTM2 - Help
// Help:       Either cancel the import and select a different name
//             for the imported folder or continue and let the imported
//             folder being merged into the existing one.


#define EQF_DICT_LOCKED                                    264
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' is currently
//             in use by another process and cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       Cancel the other process or wait until it is finished
//             and then repeat the function you wanted to carry out.

#define EQF_DICT_ENTRY_LOCKED                              265
//* Define statement owner: DKS, LEE
// Text:       The dictionary entry '%1' is currently locked by another
//             user and cannot be modified.
// HelpTitle:  OpenTM2 - Help
// Help:       Only one user at a time can make changes
//             to an entry in a shared dictionary. Wait until
//             the entry has been freed before making changes to it.

#define MESSAGE_DIC_CREATE_COMPLETED                       266
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' was successfully created.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
//*       The message is self-explanatory.

#define WARNING_PROTECT_REM_DICT                           267
//* Define statement owner: DKS, LEE
// Text:       You are not the owner of the shared dictionary '%1'
//             and are not authorized to protect it with a password.
// HelpTitle:  OpenTM2 - Help
// Help:       A shared dictionary can only be protected
//             against change by its owner or the LAN administrator.

#define ERROR_NO_REPORT_LAYOUT                             268
//* Define statement owner: XMH
// Text:       No report layout specified.
//             Specify one of the following report layouts:
//             "Standard", "Standard and Group-Summary" or
//             "Shrunk to Groups".
// HelpTitle:  OpenTM2 - Help
// Help:       Specify one of the following report layouts:
//             "Standard", "Standard and Group-Summary" or
//             "Shrunk to Groups".

#define ERROR_WRONG_REPORT_LAYOUT                          269
//* Define statement owner: XMH
// Text:       Wrong report layout specified.
//             Specify one of the following report layouts:
//             "Standard", "Standard and Group-Summary" or
//             "Shrunk to Groups".
// HelpTitle:  OpenTM2 - Help
// Help:       Specify one of the following report layouts:
//             "Standard", "Standard and Group-Summary" or
//             "Shrunk to Groups".

#define ERROR_DICT_NOT_FOUND                               270
//* Define statement owner: DKS, NEW
// Text:       The dictionary '%1' could not be found.
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary was either deleted by an operating system
//             command outside the OpenTM2 environment
//             or it was created as a shared dictionary and then deleted
//             by its owner. It can no longer be accessed.
//             'Delete' on the 'File'
//             pull-down will remove it from the Dictionary List window.

#define ERROR_DIMP_HALFCOMPLETE                            271
//* Define statement owner: DKS, LEE
// Text:       The dictionary import process was stopped early.
//             The dictionary does not contain all of the entries.
// HelpTitle:  OpenTM2 - Help
// Help:       The import process was stopped prematurely, either
//             because you chose to cancel the process or because
//             an error occurred. The dictionary does not contain all
//             of the entries but you can work with it.

#define ERROR_DEX_HALFCOMPLETE                             272
//* Define statement owner: DKS, LEE
// Text:       The dictionary export process was stopped at entry
//             '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       The export process was stopped prematurely
//             because you chose to cancel the process. Not all of the
//             entries were exported, but what was exported
//             forms a valid SGML dictionary file.

#define ERROR_DICT_PROPS_INCOMPATIBLE                      273
//* Define statement owner: DKS, LEE
// Text:       The properties belonging to the shared dictionary
//             '%1' have changed.
//             The old properties will be replaced by the new ones.
// HelpTitle:  OpenTM2 - Help
// Help:       The shared dictionary no longer has the same properties
//             related to it. This can happen when the owner of the shared
//             dictionary deletes it, and then a new dictionary with the
//             same name is created. The contents of the
//             dictionary also might have changed.

#define ERROR_DORG_FAILED                                  274
//* Define statement owner: DKS, NEW
// Text:       The organize process carried out on dictionary
//             '%1' has not terminated successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary organize process was not successful
//             for any one of the following reasons.
//             :ul.
//             :li.The dictionary is corrupt and can no longer be organized.
//             Delete the dictionary as you can no longer work with it and
//             import a backup copy.
//             :li.There is not enough disk space. In this case, make room on
//             the disk and restart the process.
//             :li.A dictionary file is read-only. Restart the organize
//             process.
//             :li.The dictionary organize termination process did not run
//             successfully. Restart the organize process.
//             :li.The organize process was deliberately terminated
//             by the user.
//             :eul.

#define ERROR_WRITE_DENIED_MSG                             275
//* Define statement owner: DKS, LEE, NEW
// Text:       Access to '%1' failed. Either there is not enough
//             disk space, or the file, directory, or drive is
//             read-only.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified file cannot be accessed for one of the
//             following reasons:
//             :ul.
//             :li.The disk on which it is located is full.
//             :li.The disk on which the file is to reside is
//             write-protected.
//             :eul.
//             If the disk is full,
//             make space by deleting files that are no longer required,
//             and repeat the function.
//             If you want to gain access, contact your network
//             administrator for authorization, and try again.
// Cause:      A DOS call returned ACCESS_DENIED, NETWORK_ACCESS_DENIED or
// Cause:      SHARING_VIOLATION
// Add.Info:   file name

#define ERROR_MAP_INVSTYLE                                 276
//* Define statement owner: DKS, LEE
// Text:       There is an SGML syntax error in the maptable section
//             of the dictionary you are importing.
//             Please check the tag '%1'.
//             The dictionary cannot be created.
// HelpTitle:  OpenTM2 - Help
// Help:       The maptable section elicits which entry fields the dictionary
//             is to contain and must be correct before a dictionary can be
//             created.
//             Check to see that all entry fields have been assigned to
//             their correct levels
//             (that is, entry, homonym, sense, and target),
//             and that the SGML syntax is correct (that is, that start
//             and end tags correlate and that tag attributes are correct).
//             If you are not sure about
//             the SGML syntax, refer to the chapter on dictionaries in the
//             OpenTM2 book.

#define ERROR_DEL_GREYEDOUT_DICT                           277
//* Define statement owner: DKS, NEW
// Text:       Dictionary '%1' is not accessible. Only the files
//             that can be accessed will be deleted. Are you sure
//             you want to delete the dictionary?
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary is greyed out because it is not
//             accessible. The reason for this can be twofold.
//             Firstly, the drive on which it is located may have been deleted
//             manually. Deleting the dictionary in this case means
//             that parts of the dictionary will be physically deleted.
//             If you allow access to this drive again after deleting
//             the dictionary, it will not be accessible as parts will
//             have been deleted.
//             Secondly, the drive may have been removed
//             via the 'Configure drives...' option on
//             the 'Utilities' pull-down. Again, deleting will
//             physically remove parts of the dictionary. If the drive
//             is added to the list of configured drives again, the
//             dictionary will not be accessible.

#define ERROR_TAGIMP_INVSTYLE                              278
//* Define statement owner: LUE, NEW
// Text:       Invalid tag attribute style: '%1'.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       The tag attribute syntax is incorrect. Tag attributes occur
//             in user defined tags (i.e. the ID= plus numeric data), in
//             specified additional characteristics for
//             format tags, and in markup tables (e.g. WIDTH= in the
//             BOOKMASTER &colon.fig tag).
//             Either stop
//             processing and correct the attribute. If you are not sure
//             about the SGML syntax, refer to the chapter on
//             markup tables in the OpenTM2 book.
//             .br
//             Or continue processing, in which case the entire entry with
//             the attribute syntax error is skipped. The entry is not added
//             to the markup table.

#define ERROR_TAGIMP_INVTAG                                279
//* Define statement owner: LUE, NEW
// Text:       Invalid SGML tag name '%1' used in SGML-based file.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       The SGML tag syntax is incorrect. This can happen if start
//             and end tag do not match or contain an error or if the tag
//             has been positioned incorrectly or left out altogether
//             Either stop processing and correct the syntax.
//             If you are not sure about
//             the SGML syntax, refer to the chapter on
//             markup tables in the OpenTM2 book.
//             .br
//             Or continue processing, in which case the entire entry with
//             the SGML tag syntax error is skipped. The entry is not added
//             to the markup table.
//* -------------------------------------------------------
//*     List Messages for Artemis
//* ------------------------------------------------------- */

#define ERROR_DEL_PROPS                                    280
//* Define statement owner: DKS, NEW
// Text:       The properties belonging to dictionary '%1' cannot
//             be read as they are corrupt. The properties will
//             be deleted and the dictionary name removed from
//             the dictionary list window.
// HelpTitle:  OpenTM2 - Help
// Help:       A dictionary can only be accessed with its associated
//             properties. If these cannot be read, the dictionary
//             can never be accessed (although it exits) as it cannot be
//             located.
//             Create a new dictionary remembering to give it a different name
//             and import the SGML backup copy of the 'lost' dictionary.

#define ERROR_DICT_DESCR_MISSING                           281
//* Define statement owner: DKS, NEW
// Text:       You have not specified a dictionary description.\n
//             Do you want to add a description
//             and then continue?
// HelpTitle:  OpenTM2 - Help
// Help:       A dictionary description is not required,
//             but it makes it easier to identify dictionaries.
// Type:       MB_YESNO

#define ERROR_WRONG_STATISTICS_TYPE                        282
//* Define statement owner: XMH
// Text:       Wrong statistics type specified.
//             When using "Calculating Report" specify either "Standard"
//             or "Advanced" as a statistics type.
// HelpTitle:  OpenTM2 - Help
// Help:       When using "Calculating Report" specify either "Standard"
//             or "Advanced" as a statistics type.

#define ERROR_FIELDNAME_TOO_LONG                           283
//* Define statement owner: DKS, NEW
// Text:       A dictionary entry field name may only be up to 29 characters
//             long.
// HelpTitle:  OpenTM2 - Help
// Help:       The name allotted to a dictionary entry field may not exceed
//             29 characters.

#define ERROR_FIELDNAME_TRUNCATED                          284
//* Define statement owner: DKS, NEW
// Text:       The dictionary entry field name '%1' is too long and will
//             be truncated. Do you want to continue importing your
//             SGML dictionary file?
// HelpTitle:  OpenTM2 - Help
// Help:       A dictionary entry field name may not exceed 29 characters.
//             If you wish to continue processing, the field name will be
//             truncated at the 29th character. If you would rather shorten
//             the name however, stop processing and change the name in the
//             maptable section at the beginning of the SGML file. Then
//             re-import the file.

#define ERROR_PROP_WRITE_MSG                               285
//* Define statement owner: DKS, LEE
// Text:       Dictionary properties on drive '%1' could not be created.
//             Either there is not enough disk space, or the drive or
//             directory is read-only.
// HelpTitle:  OpenTM2 - Help
// Help:       Check that there is enough space on the specified drive
//             and that the drive is not write protected. Restart the
//             function.

#define ERROR_DICT_ACCESS_DENIED_MSG                       286
//* Define statement owner: DKS, NEW
// Text:       Dictionary '%1' cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       You are either trying to access a dictionary on a drive
//             that is currently not accessible because it was deleted
//             manually or a dictionary file has been deleted.

#define ERROR_DICT_DIFF_SOURCE_LANG                        287
//* Define statement owner: DKS, NEW
// Text:       The source languages of the two dictionaries
//             you want to merge are different.
//             The source language of dictionary '%1' you want to
//             merge to is '%2', whereas the source language of the
//             dictionary you want to add is '%3'.\n
//             Are you sure that you want to merge them?
// HelpTitle:  OpenTM2 - Help
// Help:       Merging two dictionaries with different source
//             languages is only sometimes helpful.
//             For instance, when working with American and British English.
//             Otherwise, it can lead to inconsistency and should be avoided.

#define ERROR_EXCEEDING_VALUE                               288
//* Define statement owner: XMH
// Text:       Either COLUMN or CATEGORY exceeds the maximum value
//             The maximum value for column is 10.
//             The maximum value for category is 3.
// HelpTitle:  OpenTM2 - Help
// Help:       The maximum value for column is 10.
//             The maximum value for category is 3.

#define ERROR_LANG_NOTINSTALLED                            289
//* Define statement owner: XQG, NEW
// Text:       There is no language support installed for the source
//             language %1 of the imported dictionary %2.
// HelpTitle:  OpenTM2 - Help
// Help:       No dictionary work can be carried out if no source language
//             has been specified or no support for the source language
//             has been installed.
//             Either select another source language from the list
//             of installed source languages or cancel the import
//             and install the support for the source language
//             before re-installing the dictionary.

#define ERROR_SOURCELANG_NOTINSTALLED                      290
//* Define statement owner: XQG, NEW
// Text:       There is no language support installed for the
//             source language %1 stored in the folder properties.
// HelpTitle:  OpenTM2 - Help
// Help:       The folder properties can not be completed if no source
//             language has been specified or no support for the
//             source language has been installed.
//             Either select another language from the list
//             of installed source languages or cancel the property
//             dialog and install the support for the source language
//             first.

#define ERROR_TARGETLANG_NOTINSTALLED                      291
//* Define statement owner: XQG, NEW
// Text:       There is no language support installed for the
//             target language %1 stored in the folder properties.
// HelpTitle:  OpenTM2 - Help
// Help:       The folder properties can not be completed if no target
//             language has been specified or no support for the
//             target language has been installed.
//             Either select another language from the list
//             of installed target languages or cancel the property
//             dialog and install the support for the target language
//             first.

#define ERROR_UNACCEPTABLE_REP_TYPE                         292
//* Define statement owner: XMH
// Text:       The combination BASE_TYP and FACT_TYP is not supported
//             as a report type.
//             Specify any combination of BASE_TYP, FACT_TYP and SUM_TYP.
// HelpTitle:  OpenTM2 - Help
// Help:       Specify any combination of BASE_TYP, FACT_TYP and SUM_TYP.

#define ERROR_NO_REP_TYPE                                   293
//* Define statement owner: XMH
// Text:       No report type specified.
//             Specify BASE_TYP, FACT_TYP and SUM_TYP.
//             The combination BASE_TYP and FACT_TYP is not supported.
// HelpTitle:  OpenTM2 - Help
// Help:       Specify BASE_TYP, FACT_TYP and SUM_TYP.
//             The combination BASE_TYP and FACT_TYP is not supported.

#define ERROR_NO_OUTPUT_FILE_FORMAT                         294
//* Define statement owner: XMH
// Text:       No output file format specified.
//             Specify the output file format ASCII, HTML or RTF.
// HelpTitle:  OpenTM2 - Help
// Help:       Specify the output file format ASCII, HTML or RTF.

#define ERROR_WRONG_OUTPUT_FILE_FORMAT                      295
//* Define statement owner: XMH
// Text:       Wrong output file format %1 specified.
//             Specify the output file format ASCII, HTML or RTF.
// HelpTitle:  OpenTM2 - Help
// Help:       Specify the output file format ASCII, HTML or RTF.

#define ERROR_NO_COUNTING_TYPE                              296
//* Define statement owner: XMH
// Text:       No counting report type specified.
//             Specify the counting report type "Source Words",
//             "Target Words", "Segments" or "Modified Words".
// HelpTitle:  OpenTM2 - Help
// Help:       Specify the counting report type "Source Words",
//             "Target Words", "Segments" or "Modified Words".

#define ERROR_NO_TARGETDRIVE_SELECTED                       297
//* Define statement owner: XQG, LEE
// Text:       No target drive has been selected.
// HelpTitle:  OpenTM2 - Help
// Help:       When you want to export a folder you need to specify
//             the drive to which you want to export the folder.

#define ERROR_WRONG_COUNTING_TYPE                           298
//* Define statement owner: XMH
// Text:       Wrong counting report type specified.
//             Specify the counting report type "Source Words",
//             "Target Words", "Segments" or "Modified Words".
// HelpTitle:  OpenTM2 - Help
// Help:       Specify the counting report type "Source Words",
//             "Target Words", "Segments" or "Modified Words".

#define ERROR_NO_BOOLEAN_VALUE                              299
//* Define statement owner: XMH
// Text:       No boolean value for %1 specified.
//             Use the boolean values TRUE or FALSE.
// HelpTitle:  OpenTM2 - Help
// Help:       Use the boolean values TRUE or FALSE.

#define ERROR_WRONG_PAY_STANDARD                            300
//* Define statement owner: XMH
// Text:       Wrong pay standard specified.
//             Specify either 1, 10 or 250 for a pay standard by
//             word, line or page respectively.
// HelpTitle:  OpenTM2 - Help
// Help:       Specify either 1, 10 or 250 for a pay standard by
//             word, line or page respectively.

#define ERROR_WRONG_SECURITY_OPTION                         301
//* Define statement owner: XMH
// Text:       Wrong security option specified.
//             Specify a combination of PLAUS_OPT, LOST_OPT
//             and LIST_OPT.
// HelpTitle:  OpenTM2 - Help
// Help:       Specify a combination of PLAUS_OPT, LOST_OPT
//             and LIST_OPT.

#define ERROR_WRONG_LAYOUT                                  302
//* Define statement owner: XMH
// Text:       Wrong report layout specified.
//             Specify the report layout as "Standard",
//             "Standard and Group-Summary" or "Shrunk to Groups".
// HelpTitle:  OpenTM2 - Help
// Help:       Specify the report layout as "Standard",
//             "Standard and Group-Summary" or "Shrunk to Groups".

#define ERROR_STATISTICS_TYPE_NOT_ALLOWED                   303
//* Define statement owner: XMH
// Text:       Statistics type is only allowed with "Counting Report".
// HelpTitle:  OpenTM2 - Help
// Help:       Statistics type is only allowed with "Counting Report".

#define ERROR_EX_PROPOSAL                                   304
//* Define statement owner: XMH
// Text:       Existing Proposal is only allowed with "Counting Report".
// HelpTitle:  OpenTM2 - Help
// Help:       Existing Proposal is only allowed with "Counting Report".

#define ERROR_NO_VALID_CURRENCY                             305
//* Define statement owner: XMH
// Text:       The currency %1 is not valid
// HelpTitle:  OpenTM2 - Help
// Help:       The currency %1 is not valid

#define ERROR_MUPROP_NO_TABLE_MAME                         306
//* Define statement owner: XQG
// Text:       You need to specify a name for the markup table.
// HelpTitle:  OpenTM2 - Help
// Help:       The name of the markup table is required.
//             It can be up to eight characters long.

#define ERROR_MUPROP_NAME_INVALID                           307
//* Define statement owner: XQG
// Text:       The name '%1' is not valid for a markup table.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to create the markup table correct the name.
//             The name can have up to eight alphanumeric characters.

#define ERROR_MUPROP_NAME_EXISTS                            308
//* Define statement owner: XQG
// Text:       There is already a markup table with the  name '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Specify a different name as there is already a markup table
//             with this name.
//             The name can have up to eight alphanumeric characters.

#define ERROR_MUPROP_TABLE_IS_EMPTY                         309
//* Define statement owner: XQG
// Text:       The markup table is empty and cannot be saved.
// HelpTitle:  OpenTM2 - Help
// Help:       No tags have been defined for this markup table
//             and there is no user exit specified. Either define
//             some tags or specify an user exit before saving the
//             markup table.

#define ERROR_MUPROP_DESCRNAME_INUSE                       310
// Text:       Descriptive name '%1' already in use for
//             markup table '%2'.
// HelpTitle:  OpenTM2 - Help
// Help:       The descriptive name must be a unique identifier
//             for the markup table. The specified descriptive name
//             is already used by another markup table. Choose a
//             different descriptive name.

#define ERROR_MUPROP_SAMESUBSTCHAR                         311
//* Define statement owner: XQG
// Text:       Same character used for single and multiple substitution.
// HelpTitle:  OpenTM2 - Help
// Help:       The characters used as place holder for the
//             substitution of single and multiple characters must be
//             unique. Choose a different character to continue.

#define ERROR_MUPROP_NO_TAG_MAME                           312
//* Define statement owner: XQG
// Text:       No string specified for tag
// HelpTitle:  OpenTM2 - Help
// Help:       The tag string cannot be empty. Enter the tag characters
//             in the tag string entry field.

#define ERROR_MUPROP_NO_TAG_DELIM                          313
//* Define statement owner: XQG
// Text:       Tag has no end delimiter and no length
// HelpTitle:  OpenTM2 - Help
// Help:       Specify end delimiter characters for the tag or
//             enter a tag length to continue.

#define ERROR_MUPROP_NO_ATTR_MAME                          314
//* Define statement owner: XQG
// Text:       No string specified for attribute
// HelpTitle:  OpenTM2 - Help
// Help:       The attribute string cannot be empty. Enter the
//             attribute characters in the attribute string entry field.


#define ERROR_ENTRYFIELD_NOT_SELECTED                      315
//* Define statement owner: DKS
// Text:       No entry field has been selected.
// HelpTitle:  OpenTM2 - Help
// Help:       You need to select an entry field.


#define ERROR_MUPROP_NO_ATTR_DELIM                         316
//* Define statement owner: XQG
// Text:       Attribute has no end delimiter and no length
// HelpTitle:  OpenTM2 - Help
// Help:       Specify end delimiter characters for the attribute or
//             enter a attribute length to continue.


#define ERROR_MUPROP_REFRESH_FAILED                        317
//* Define statement owner: XQG
// Text:       Test of markup table failed
// HelpTitle:  OpenTM2 - Help
// Help:       Due to a previously reported error condition
//             the selected test document could not be analyzed
//             using this markup table.

#define MESSAGE_MULT_MEM_ORGANIZE_COMPLETED                318
//* Define statement owner: XQG
// Text:       The organization of the Translation Memory databases %1
//             completed successfully. %2 segments have been organized.
//             %3 segments were corrupted and had to be skipped.
// HelpTitle:  OpenTM2 - Help
// Help:       The displayed number of segments is stored in the
//             Translation Memory.


#define ERROR_INVALID_FUZZYMATCH_VALUE                     319
//* Define statement owner: XQG, NEW
// Text:       Please specify a number in the range from 5 to 99.
// HelpTitle:  OpenTM2 - Help
// Help:       The fuzzy match overlap ratio is a numerical value
//             in the range from 5 to 99.

#define ERROR_DORG_COMPLETE_MUL                            320
//* Define statement owner: XQG,NEW
// Text:       The organization of the dictionaries %1 completed
//             successfully. %2 terms have been organized.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.

#define ERROR_DEX_COMPLETE_MUL                            321
//* Define statement owner: XQG, NEW
// Text:       Dictionaries %1 have been exported successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.

#define MESSAGE_MEM_EXP_COMPLETED_MUL                     322
//* Define statement owner: XQG, NEW
// Text:       The export of the Translation Memory databases %1 completed
//             successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       The export of the Translation Memory databases has been
//             successfully completed.

#define MESSAGE_FOLEXP_COMPLETED_MUL                       323
//* Define statement owner: XQG, NEW
// Text:       The export of the folders %1 to drive '%2' completed
//             successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
// Add.Info:   '%1' = folder name, '%2' = target drive

#define MESSAGE_MEMIMP_COMPLETED_MUL                       324
//* Define statement owner: DOE, TAF, LEE
// Text:       The import of Translation Memory databases %1
//             completed successfully.
//             %2 segments have been imported. %3 segments were
//             not valid or they are obsolete.
// HelpTitle:  OpenTM2 - Help
// Help:       Segments need to have valid syntax to be imported.
//             Segments that have a more recent equivalent in the
//             Translation Memory are also not imported.

#define MESSAGE_FOLIMP_COMPLETED_MUL                       325
//* Define statement owner: XQG, LEE
// Text:       The folders %1 have been imported successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
// Add.Info:   '%1' = list of folder names

#define ERROR_DIMP_COMPLETE_MUL                            326
//* Define statement owner: XQG, NEW
// Text:       Importing into the dictionaries %1 has been
//             successfully completed.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you may continue working.

#define MESSAGE_MEM_LOAD_COMPLETED_MUL                     327
//* Define statement owner: XQG, NEW
// Text:       The import of the Translation Memory
//             databases completed successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       The import of the selected files
//             has been completed.

#define ERROR_INVALID_OLD_SHIPMENT_NUMBER                  328
//* Define statement owner: XQG
// Text:       The existing shipment number "%1" is invalid.
//             Select a numeric shipment number from the
//             shipment number dropdown.
// HelpTitle:  OpenTM2 - Help
// Help:       The shipment number specified for this folder
//             is not a valid shipment number. Valid shipment
//             numbers are numeric and in the range from 1 to 99.

#define WARNING_DECREASED_SHIPMENT_NUMBER                  329
//* Define statement owner: XQG
// Text:       The selected shipment number is smaller than the active
//             one. Are you sure that you want to decrease the
//             shipment number?.
// HelpTitle:  OpenTM2 - Help
// Help:       The shipment number specified for this folder
//             is smaller than the current one. Select 'OK' if you
//             are sure that the new shipment is correct.
//* -------------------------------------------------------
//*     File export and import messages
//* -------------------------------------------------------

#define NO_EXPORTPATH                                      330
//* Define statement owner: XQG, LEE
// Text:       No path is specified for the documents.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to export the document,
//             you need to specify its destination. Enter a specific
//             path in the path entry field.
//             .br
//             The file names of the
//             original document, the translated document, and the
//             file with no matching segments are identical.
//             Therefore it is important to specify a unique path for
//             each of them.

#define WARNING_LARGEINCREMENT_SHIPMENT_NUMBER             331
//* Define statement owner: XQG
// Text:       The selected shipment number '%1' is not the next shipment
//             from shipment '%2'. Are you sure that you want to use the
//             selected shipment number?.
// HelpTitle:  OpenTM2 - Help
// Help:       The shipment number specified for this folder
//             is not the next possible one. Select 'OK' if you
//             are sure that the new shipment is correct.

#define WRONG_EXPORTDIR                                    332
//* Define statement owner: XQG, LEE
// Text:       An incorrect path is specified for the documents.
// HelpTitle:  OpenTM2 - Help
// Help:       The directory does not exist or has not been specified
//             correctly. The path must begin and end with
//             a backslash, and the path name can be any combination
//             of alphanumeric characters. If you do not want to specify
//             a subdirectory, the export to the chosen root directory must
//             be indicated by a single backslash.
//             .br
//             Correct the information in the path field and continue.

#define ERROR_MTMQS_LOAD_FAILED                            333
//* Define statement owner: GQ
// Text:       Load of MT interface failed. Please install the IBM WebSphere
//             MQ Client software and retry.
// HelpTitle:  OpenTM2 - Help
// Help:       Translation Manager was not able to load the interface
//             to the MT server. Please ensure that the IBM WebSphere MQ client
//             has been installed correctly.

#define ALL_IMPORTED                                       334
//* Define statement owner: DOE, LEE
// Text:       All selected files have been successfully imported to the
//             specified folder.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.

#define NOT_ALL_IMPORTED                                   335
//* Define statement owner: XQG, LEE
// Text:       Not all files were successfully imported.
// HelpTitle:  OpenTM2 - Help
// Help:       Because of some errors, which were reported prior
//             to this message, not all files have been successfully
//             imported.
//             Follow the instructions in the previously displayed messages.

#define ALL_EXPORTED                                       336
//* Define statement owner: DOE, LEE
// Text:       The selected document(s) has been successfully exported
//             to the specified directory.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.

#define NOT_ALL_EXPORTED                                   337
//* Define statement owner: XQG, LEE
// Text:       Not all documents have been successfully exported.
// HelpTitle:  OpenTM2 - Help
// Help:       Because of some errors, which were reported prior
//             to this message, not all files have been successfully
//             exported.
//             Follow the instructions in the previously displayed messages.

#define CHECKONE                                           338
//* Define statement owner: XQG, LEE
// Text:       Select either the translation, the original, or
//             the untranslated segments check box, or any combination
//             of them.
// HelpTitle:  OpenTM2 - Help
// Help:       When exporting documents you need to specify whether you
//             want to export either the translated or the original version
//             or the untranslated segments for the document or any
//             combination of these documents.
//             Because the file names are the same,
//             you need to specify distinct paths for the documents
//             when you want to export more than one document type.

#define ERROR_EXPORT_TO_SYSTEM                             339
//* Define statement owner: DOE, LEE
// Text:       The specified path is reserved.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified path is reserved for OpenTM2
//             system files.
//             Please, specify another path.

#define ERROR_DOCEXP_NO_REVMARK                            340
//* Define statement owner: XQG, LEE
// Text:       The revision mark name is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       Select a revision mark file name from the drop down
//             combination box.
//             To create new revision marks, enter a new name and
//             press the revision mark edit button. Enter all
//             revision mark data into the appropriate fields of
//             the revision mark dialog.

#define ERROR_SAMEPATH                                     341
//* Define statement owner: DOE, LEE
// Text:       The paths for the translated document, the original document,
//             or the untranslated segments file are the same.
// HelpTitle:  OpenTM2 - Help
// Help:       You need to specify unique paths
//             when you want to export more than one document.
//             The file names of the original the translated document, and the
//             untranslated segments file are the same.
//             Correct one of the paths and try again.

#define ERROR_NOTARGETFILE                                 342
//* Define statement owner: DOE, LEE
// Text:       The translated document %1 cannot be found.
// HelpTitle:  OpenTM2 - Help
// Help:       Either the document has not been translated yet, or the
//             translated version is no longer accessible.

#define ERROR_DOCEXP_REVMARK_NOT_EXIST                     343
//* Define statement owner: XQG, LEE
// Text:       The specified revision mark file does not exist.
// HelpTitle:  OpenTM2 - Help
// Help:       To create new revision marks, press the revision
//             mark edit button. Enter all revision mark data
//             into the appropriate fields of the revision mark
//             dialog.

#define ERROR_DOCEXP_REVMARK_CORRUPT                       344
//* Define statement owner: XQG, LEE
// Text:       The selected revision mark file is damaged.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected revision mark file does not have
//             the correct format or is damaged.
//             Use the revision mark dialog to create a new
//             revision mark file or select another revision
//             mark file.

#define ERROR_REVMARK_NO_NAME                              345
//* Define statement owner: XQG, LEE
// Text:       The revision mark name is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       You need to enter the name of the revision marks
//             before saving them to disk.

#define ERROR_REVMARK_NAME_INVALID                         346
//* Define statement owner: XQG, LEE
// Text:       The name '%1' is incorrect.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to save the revision marks to disk,
//             correct the name.
//             The name can have up to
//             eight alphanumeric characters. Do not specify a file
//             extension.

#define ERROR_REVMARK_CHANGED                              347
//* Define statement owner: XQG, LEE
// Text:       Revision marks have been changed.\n
//             Do you want to save the changes before leaving the dialog?
// HelpTitle:  OpenTM2 - Help
// Help:       The revision mark data has been changed. Either save
//             the changed revision marks before performing the
//             requested function, discard the changes, or return
//             to the dialog.

#define WARNING_REVMARK_EXISTS                             348
//* Define statement owner: XQG, LEE
// Text:       The revision mark file '%1' already exists.\n
//             Do you want to overwrite it?
// HelpTitle:  OpenTM2 - Help
// Help:       If you do not specify a new revision mark name,
//             the file is overwritten with
//             the new data.

#define QUERY_DELETEMTMEM                                  349
//* Define statement owner: XQG NEW
// Text:       The MT translation memory exists already.\n
//             Do you want to delete all segments contained
//             in the Translation Memory before filling
//             it with the MT proposals?
// HelpTitle:  OpenTM2 - Help
// Help:       The machine Translation Memory exists already and
//             may contain data.
//             .br
//             Either allow to delete all segments stored in this
//             Translation Memory so that the Translation Memory
//             will contain only the segments added during the
//             merge process or continue without deleting the data
//             contained in the Translation Memory.
//* -------------------------------------------------------
//*     Property messages
//* -------------------------------------------------------

#define ERROR_MUPROP_USEREXIT_INVALID                      350
//* Define statement owner: XQG
// Text:       The name '%1' is not valid for a user exit.
// HelpTitle:  OpenTM2 - Help
// Help:       Correct the name of the user exit.
//             The name can have up to eight alphanumeric characters.


#define ERROR_PROPERTY_ACCESS                              351
//* Define statement owner: DOE, LEE
// Text:       The property '%1' cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       Because of a system error, an internal file cannot be
//             accessed. Close the workbench and try again.
//             If the error continues, reinstall the workbench.

#define ERROR_CREATING_PROPS                               352
//* Define statement owner: DKS, LEE
#define ERROR_CREATE_PROP                                  352
#define ERROR_CREATE_PROPERTIES                            352
//* Define statement owner: DOE
// Text:       The properties of '%1' could not be created.
// HelpTitle:  OpenTM2 - Help
// Help:       The properties could not be created due to file-handling
//             errors.
//             Ensure that there is enough space available on your
//             OpenTM2 system drive.

#define ERROR_SAVING_PROPS                                 353
//* Define statement owner: DKS, LEE
#define ERROR_WRITE_PROPERTIES                             353
//* Define statement owner: DOE
// Text:       Unable to write the properties '%1' to disk.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that there is enough space available on your
//             OpenTM2 system drive.

#define ERROR_OPENING_PROPS                                354
//* Define statement owner: DKS
#define ERROR_OPEN_PROPERTIES                              354
//* Define statement owner: DOE, LEE
// Text:       The properties for '%1' cannot be opened.
// HelpTitle:  OpenTM2 - Help
// Help:       Either the properties file cannot be found
//             or there is not enough space on the system disk drive
//             to access the property file.

#define ERROR_ACCESS_SYSTEMPROPERTIES                      355
//* Define statement owner: DOE, LEE
// Text:       Write access to the system properties was denied.
// HelpTitle:  OpenTM2 - Help
// Help:       The system properties might be in use by another process.
//             You can either cancel some running processes, or wait until
//             they are finished and try again.
//*     Restarting the
//*     workbench takes previously saved values into account.

#define ERROR_READ_SYSTEMPROPERTIES                        356
//* Define statement owner: DOE, LEE
// Text:       The system properties could not be read.
// HelpTitle:  OpenTM2 - Help
// Help:       The system properties file is an internal control file. It
//             might be damaged. Try to restart the OpenTM2
//             workbench. If the
//             error continues, reinstall OpenTM2.

#define ERROR_PROP_ALREADYEXISTS                           357
//* Define statement owner: SOK, LEE
// Text:       The properties for '%1' already exist.
// HelpTitle:  OpenTM2 - Help
// Help:       The properties could not be created because they
//             already exist. Choose a different name.

#define ERROR_PROP_INVALIDCLASS                            358
//* Define statement owner: SOK, LEE
// Text:       A component of OpenTM2 could not be
//             created.
// HelpTitle:  OpenTM2 - Help
// Help:       Check the disk space on the system drive. If necessary, make
//             space on this drive and restart OpenTM2.
//             If the error continues, reinstall OpenTM2.

#define ERROR_PROP_INVALIDFILE                             359
//* Define statement owner: SOK, LEE
// Text:       The file '%1' is damaged.
// HelpTitle:  OpenTM2 - Help
// HELP:       An important component of OpenTM2 could not
//             be opened.
//             Reinstall OpenTM2.

#define ERROR_PROP_DEFAULT                                 360
//* Define statement owner: SOK, LEE
// Text:       An internal programming error has occurred.
// HelpTitle:  OpenTM2 - Help
// Help:       Try the function again. If the error continues, reinstall
//             OpenTM2.

#define ERROR_PROP_OBJECTBUSY                              361
//* Define statement owner: SOK, LEE
// Text:       The file '%1' is already in use by another process.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified property file is being used by another
//             process. Wait until this process has finished before you
//             try again.

#define ERROR_GENERAL_PROP_ERROR_MSG                       362
//* Define statement owner: SOK, NEW
// Text:       The file '%1' is damaged.
// HelpTitle:  OpenTM2 - Help
// Help:       Restart OpenTM2. If the error continues,
//             reinstall OpenTM2.

#define ERROR_EXISTTAGTABLE_DATE                          363
//* Define statement owner: XQG
// Text:       The existing markup table '%1' is
//             newer (%2) than your system date.
//             The markup table has not been imported.
// HelpTitle:  OpenTM2 - Help
// Help:       The existing markup table is newer than
//             the system date of your machine. Either adjust
//             the system date or delete the markup table
//             manually before repeating the folder import.

#define ERROR_IMPTAGTABLE_DATE                           364
//* Define statement owner: XQG
// Text:       The markup table '%1' imported with the
//             folder is newer (%2) than your system date.
//             The markup table has not been imported.
// HelpTitle:  OpenTM2 - Help
// Help:       The markup table contained in the imported
//             folder is newer than the system data of your
//             Adjust the system date if it is not correct
//             and repeat the folder import or
//             ignore the markup table being imported.

#define ERROR_EXISTUSEREXIT_DATE                          365
//* Define statement owner: XQG
// Text:       The existing markup table user exit '%1' is
//             newer (%2) than your system date.
//             The user exit not been imported.
// HelpTitle:  OpenTM2 - Help
// Help:       The existing user exit is newer than
//             the system date of your machine. Either adjust
//             the system date or delete the user exit
//             manually before repeating the folder import.

#define ERROR_IMPUSEREXIT_DATE                            366
//* Define statement owner: XQG
// Text:       The markup table user exit '%1' imported
//             with the folder is newer (%2) than your
//             system date. The user exit has not been imported.
// HelpTitle:  OpenTM2 - Help
// Help:       The markup table user exit contained in the imported
//             folder is newer than the system date of your
//             machine. Adjust the system date if it is not correct
//             and repeat the folder import or
//             ignore the markup table user exit being imported.


#define ERROR_MUPROP_USEREXIT_LOADERROR                    367
//* Define statement owner: XQG
// Text:       The user exit '%1' does not exist or
//             can not be loaded.
// HelpTitle:  OpenTM2 - Help
// Help:       Specify the name of an existing user exit.
//             The name can have up to eight alphanumeric characters
//             and the user exit has to be stored in the \EQF\WIN
//             directory and must be a valid Windows DLL.

#define WARNUNG_NO_TARGETLANG_SET                         368
//* Define statement owner: XQG
// Text:       Select the default target language.
// HelpTitle:  OpenTM2 - Help
// Help:       Select the default target language
//             suitable for your environment.
//             If no or the wrong language has been
//             selected code page conversions performed
//             by OpenTM2 may result in
//             corrupted characters.

#define ERROR_MEM_DB_FULL_W_SEG                            369
//* Define statement owner: XQG
// Text:       Translation Memory '%1' is full. Import stopped
//             at segment %2.
// HelpTitle:  OpenTM2 - Help
// Help:       Due to the internal structure of Translation Memory databases
//             their size is limited. Memory import stopped at the
//             referenced segment. This segment is the last one imported.
//             Split the imported file at this position and import
//             the rest of the file into a new TranslationMemory

#define ERROR_WRITE_PROFILE                                370
//* Define statement owner: XQG
// Text:       Save of profile '%1' failed.
// HelpTitle:  OpenTM2 - Help
// Help:       Profile could not be saved. Probably the profile
//             is write-protected.


#define ERROR_NEW_NAME_MISSING                             371
//* Define statement owner: .............
// Text:       Missing new name for object
// HelpTitle:  OpenTM2 - Help
// Help:       You have not entered a new name for
//             the object. Please type-in a new name.

//* -------------------------------------------------------
//* Error returned from WideCharToMultiByte or MultiByteToWideChar
//* ERROR_INSUFFICIENT_BUFFER (122), ERROR_INVALID_FLAGS (1004)
//* ERROR_INVALID_PARAMETER(87), ERROR_NO_UNICODE_TRANSLATION(1113)
//* -------------------------------------------------------

#define ERROR_DATA_CONVERSION                             372
//* Define statement owner: XJR
// Text:       An error occurred during data conversion.
//             Data cannot be converted, error return is '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       The data may be invalid, the language support may be missing
//             or the used parameters may be invalid.

#define ERR_RTF_BRACEMISMATCHERR                           373
//* Define statement owner: XQG
// Text:       The number of curly braces ("{","}") in
//             segment %1 has been changed.
//             This may prevent processing of the
//             document using your word processor.
//             Do you want to continue the export anyway?
// HelpTitle:  OpenTM2 - Help
// Help:       The number of curly braces has been changed
//             in the referenced segment. The curly braces are
//             vital tagging information of RTF. If the
//             number of curly braces is modified the
//             document will probably not load anymore in
//             your word processor.
//             :p.
//             Use the source of the segment to determine the
//             correct number and position of the curly braces
//             and correct the translation accordingly.

#define NO_VALID_FORMAT_WITH_SEG                           374
//* Define statement owner: XQG
// Text:       Segment %1 in file '%2' contains data that cannot be processed.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified file is not in the format the process expects.
//             Retry the function. If the problem continues select another
//             file. If you are analyzing a document, end the Analysis dialog.
//             Select the correct markup table and restart the analyze
//             process.


#define ERROR_LANG_NOTSUPPORTED                            375
//* Define statement owner: XQG, NEW
// Text:       Language "%1" is not supported by your operating system and
//             cannot be selected as default target language.
//             Select a different default target language.
// HelpTitle:  OpenTM2 - Help
// Help:       There is no support installed for the selected language.
//             Select a different language or install the missing language
//             support using the regional settings window of the control panel.

#define ERROR_DELETE_FOLDER_FAILED                        376
//* Define statement owner: XQG
// Text:       Folder '%1' could not be deleted completely
//             due to a previously reported error condition.
// HelpTitle:  OpenTM2 - Help
// Help:       Due to a previously reported error condition
//             the selected folder was only deleted partially.
//             Correct the reported error condition and repeat
//             the delete to remove the folder completely.


#define TA_MARKUPTABLE_INVALID                             377
//* Define statement owner: XJR
// Text:       Markup table is invalid or incomplete or not the
//             latest version.
// HelpTitle:  OpenTM2 - Help
// Help:       Check whether the most recent version is
//             in your installation.

#define ERROR_FOLDER_SHORTNAME_EXISTS                      378
//* Define statement owner: XQG
// Text:       The name '%1' is used as internal short name for
//             folder '%2' and cannot be used as folder name.
//             Specify another name.
// HelpTitle:  OpenTM2 - Help
// Help:       Try another folder name as the specified name
//             is already used as short folder name for another folder.

#define ERROR_DEX_TERMCORRUPTED                            379
//* Define statement owner: XQG
// Text:       The dictionary entry '%1' is corrupted (return code=%2).
//             Do you want to skip this entry and continue the export process?
// HelpTitle:  OpenTM2 - Help
// Help:       You can either continue exporting, in which case the entry
//             is skipped, or you can
//             cancel the export process.

#define ERROR_DEX_INCOMPLETE                               380
//* Define statement owner: XQG
// Text:       Dictionary '%1' has not been exported completely. Some
//             corrupted entries have been skipped.
// HelpTitle:  OpenTM2 - Help
// Help:       Not all entries of the dicitonary could be exported.


#define ERROR_DATA_CONVERSION_EXINFO                      381
//* Define statement owner: XQG
// Text:       An error occurred during data conversion.
//             The character code %1 could not be converted from
//             codepage %2 to Unicode, the error code is '%3'.
// HelpTitle:  OpenTM2 - Help
// Help:       The data may be invalid, the language support may be missing
//             or the used parameters may be invalid.

#define ERROR_INVALID_LOOKUP_FUZZYMATCH_VALUE                     382
//* Define statement owner: XQG, XJR
// Text:       Please specify a number in the range from 5 to 50.
// HelpTitle:  OpenTM2 - Help
// Help:       The fuzzy match overlap ratio is a numerical value
//             in the range from 5 to 50.

#define MESSAGE_MULFOLIMP_COMPLETED                           383
//* Define statement owner: XQG
// Text:       The folders %1 were imported successfully to drive '%2'.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
// Add.Info:   '%1' = list of folder names, '%2' = target drive

#define ERROR_INV_EXPRESSION                                 384
//* Define statement owner: XQG
// Text:       The search expression is not supported. Please change and retry.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified search expression syntax is not supported.
//             The following types of expressions may be used:
//             string1 AND string2
//             string1 OR string2
//             NOT string1

#define WARNING_DIMP_FORMAT                                385
//* Define statement owner: XRJ
// Text:       The format specified in the imported file is %1, but the format
//             selected in the import dialog is %2.\n
//             If you continue, the format as defined
//             in the imported file is used.
// HelpTitle:  OpenTM2 - Help
// Help:       If you continue, the format information from the
//             imported file is used.

#define WARNING_DIMP_CODEPAGE                              386
//* Define statement owner: XRJ
// Text:       The codepage specified in the imported file is %1, but the codepage
//             defined by the default target language of OpenTM2
//             is %2. \n
//             If you continue, the codepage as defined
//             in the imported file is used.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to change the default target language, you need to cancel
//             the import process.
//             Select cancel if the codepage in the imported file is wrong.

#define ERROR_DIMP_UTF16WRONG                              387
//* Define statement owner: XRJ
// Text:       The codepage specified in the imported file is Unicode,
//             but the file is not a valid Unicode file.
//             Import process is canceled.
// HelpTitle:  OpenTM2 - Help
// Help:       The codepage specification in the import file may have been
//             changed. The BOM at the begin of the file may be missing.

#define NO_VALID_ASCIIANSIFORMAT                             388
//* Define statement owner: XJR
// Text:       The file seems to be a Unicode file, not a %1 file.  \n
//             Import process is canceled.
// HelpTitle:  OpenTM2 - Help
// Help:       Please check the format specified in the
//             Codepage identifier of the file.
//             The file contains a Unicode BOM.

#define NO_VALID_ASCIIANSICP                               389
//* Define statement owner: XJR
// Text:       The codepage %1 given in the importfile is not supported.\n
//             Import process is canceled.
// HelpTitle:  OpenTM2 - Help
// Help:       Please check the codepage identifier in the file.


//* -------------------------------------------------------
//* Messages for Translation
//* -------------------------------------------------------

#define ERROR_NEWER_RICHED_NEEDED                          390
//* Define statement owner: XJR
// Text:       A newer Richedit control is needed
// HelpTitle:  OpenTM2 - Help
// Help:       Please install a more recent Richedit control.
//             At least Version 3.0 of the RICHED20.DLL is necessary.

#define ERROR_START_EDITOR                                 391
//* Define statement owner: DOE, LEE
// Text:       The translation editor '%1' cannot be started.
// HelpTitle:  OpenTM2 - Help
// Help:       The translation editor cannot be started.
//             Close OpenTM2 and try again.

#define ERROR_ONLY_ONE_TRANSLATION                         392
//* Define statement owner: DOE, LEE
// Text:       Only one translation session is allowed.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to translate another document, close the
//             active document and open a new one.

#define ITM_FILENAMES_ARE_EQUAL                            393
//* Define statement owner: XJR, LEE
// Text:       File names to be aligned are the same.
//             They will be ignored.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       The specified file names are the same and hence ignored.
//             The message allows you either to continue or to stop the
//             alignment process.
// Type:       EQF_ERROR
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : file name specified

#define ITM_WRONGCMDLINE                                   394
//* Define statement owner: XJR
// Text:       The specified command does not have the correct
//             syntax.\n
//             The wrong parameter is: '%1' \n
//             The correct syntax is:  \n
//             EQFITM  /ME[M]=memdb   \n
//                     /FI[LES]=flist \n
//                     /MA[RKUP]=markup \n
//                     [/SG[MLMEM]=mem] \n
//                     [/TY[PE]=NOANA | NOCONF |         \n
//                              NOTM | VISUAL | PREPARE] \n
//                     /TG[TLNG]=tgtlng \n
//                     [/SR[CLNG]=srclng] \n
//                     [[/SRCSTARTPATH|/SS]=startpath] \n
//                     [[/TGTSTARTPATH|/TS]=startpath] \n
//                     [[/SGMLFORMAT|/SF]=Ansi|ASCII|Unicode]\n
// HelpTitle:  OpenTM2 - Help
// Help:       The correct syntax of the command line is follows:
//             .br
//              EQFITM  /ME[M]=memdb
//             .br
//                      /FI[LES]=flist
//             .br
//                      /MA[RKUP]=markup
//             .br
//                      [/SG[MLMEM]=mem]
//             .br
//                      [/TY[PE]=type]
//             .br
//                      /TG[TLNG]=tgtlng
//             .br
//                      [/SR[CLNG]=srclng]
//             .br
//                      [[/SRCSTARTPATH|/SS]=startpath] \n
//             .br
//                      [[/TGTSTARTPATH|/TS]=startpath] \n
//             .br
//                      [[/SGMLFORMAT|/SF]=Ansi|ASCII|Unicode]\n
//             .br
//             For information about the parameters, refer to the
//             OpenTM2 documentation.
// Add.Info:   '%1'  wrong parameter

#define ITM_MANDCMDLINE                                    395
//* Define statement owner: XJR
// Text:       A required command line parameter is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       The command line parameters that are required for the ITM
//             generation are:
//             .br
//              /FI[LES]=
//              /ME[M]=
//              /MA[RKUP]=
//              /TG[TLNG]=

#define ITM_NUMOFFILES                                     396
//* Define statement owner: XJR, LEE
// Text:       The number of files specified should be an even number.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified number of files is not an even
//             number.
//             Update the list or the command line entry so that the
//             pairs of files to be aligned are available.

#define ITM_LING_SUPPORT_MISSING                           397
//* Define statement owner: XJR, NEW
// Text:       The linguistic support file for the specified
//             target language '%1' is missing.
//             You can proceed, but the results may not be as
//             good as expected.\n
//             Do you want to proceed?
// HelpTitle:  OpenTM2 - Help
// Help:       The linguistic support file for the target
//             language as specified in the Translation Memory
//             is not available. Therefore the target file could
//             not be analyzed/counted
//             using the correct language support.
//             Continue if you do not want to install the linguistic
//             support first.
// Type:       MB_YESNO
// Add.Info:   Target Language

#define ITM_EMPTYCMDLINE                                   398
//* Define statement owner: XJR, NEW
// Text:       The specified command line is empty\n
//             The correct syntax is:\n
//              EQFITM  /ME[M]=memdb   \n
//                      /FI[LES]=flist \n
//                      /MA[RKUP]=markup \n
//                      [/SG[MLMEM]=mem] \n
//                      [/TY[PE]=type]   \n
//                      /TG[TLNG]=tgtlng \n
//                      [/SR[CLNG]=srclng] \n
//                      [[/SRCSTARTPATH|/SS]=startpath] \n
//                      [[/TGTSTARTPATH|/TS]=startpath] \n
//                      [[/SGMLFORMAT|/SF]=Ansi|ASCII|Unicode]\n
//             For information about the parameters, refer to the
//             OpenTM2 documentation.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ITM_TM_NEEDS_ORGANIZE                              399
//* Define statement owner: XJR, NEW
// Text:       The dictionary or Translation Memory '%1'
//             needs to be organized; it is
//             either damaged or it was created with
//             an older version of OpenTM2.\n
//             Please use the organize function to
//             organize the dictionary or Translation Memory.
// HelpTitle:  OpenTM2 - Help
// Help:       Your dictionary or Translation Memory is either
//             damaged or
//             it was created with an older version of OpenTM2.
//             :p.
//             Try organizing the dictionary or Translation
//             Memory
//             or reimport the SGML copy of the file.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1'  Translation Memory name
//* -------------------------------------------------------
//* Messages for Text Analysis
//* -------------------------------------------------------

#define ERROR_START_THREAD                                 400
//* Define statement owner: LEE
// Text:       Text analysis could not be started.
// HelpTitle:  OpenTM2 - Help
// Help:       The system could not start text analysis.
//             Too many processes might be running at the moment.
//             You can either cancel some of these running processes,
//             or wait until they have finished and try again.

#define ERROR_TACOMPLETE                                   401
//* Define statement owner: NEW
// Text:       Text analysis completed.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.

#define ERROR_TA_DICTACT                                   402
//* Define statement owner: LEE
// Text:       The dictionary '%1' could not be activated.
//             Text analysis is cancelled.
//* Displayed Buttons: CANCEL
// HelpTitle:  OpenTM2 - Help
// Help:       The specified dictionary could not be activated for one
//             of the following reasons:
//             :ul.
//             :li.There is no dictionary with the specified name.
//             Correct the name and try again.
//             :li.The dictionary you wanted to work with has been
//             created with an older version of OpenTM2,
//             or it is damaged. Reorganize the dictionary and try again.
//             :eul.

#define ERROR_TA_NOMORPH                                   403
//* Define statement owner: LEE
// Text:       No language support file has been selected for this text
//             analysis process.\n
//             Do you want to continue?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       If you continue, this text analysis process continues
//             without looking for new terms and terms found. No lists
//             are created. Leave this function if you want to select
//             one or more
//             dictionaries and then start text analysis again.
// Add.Info:   message may be obsolete

#define ERROR_TA_SEGMENT                                   404
//* Define statement owner: LEE
// Text:       The analysis of document '%1' failed. It will be ignored.\n
//             Do you want to continue analyzing the remaining documents?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       An error occurred while the text was being divided
//             into segments.
//             Select 'Yes' to continue. The erroneous document is ignored
//             during analysis.
//             Select 'No' to stop analysis. Possible causes for the error
//             include:
//             :ul.
//             :li.You specified the wrong markup table or
//             the wrong language support file.
//             :li.Either or both of these files contain errors. If
//             it is a self-created markup table is erroneous, import
//             a backup copy, or create the table again.
//             Check the contents of the SGML copy of
//             the table.
//             :li.If the table or support file was supplied by
//             OpenTM2, reinstall OpenTM2.
//             If the error continues, call your IBM
//             representative.
//             :li.The file you selected
//             for analysis might not be a text file and, therefore, not
//             intended for translation (for example, it is an executable
//             file).
//             :eul.

#define ERROR_TA_SAVE_SEGFILE                              405
//* Define statement owner: DOE, LEE
// Text:       The document '%1' could not be saved to disk.\n
//             Do you want to continue and skip this document?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       The system could not save the specified file.
//             Ensure that there is enough space on the specified disk.
//             If necessary, make more space available, or choose
//             another disk.
//             .br
//             If you continue, the document is skipped, and the next
//             document is analyzed. Leaving this function stops
//             the analysis process.

#define ERROR_TA_FILEWRITE                                 406
//* Define statement owner: LEE
// Text:       Not enough space on disk to save file '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Make more space available on the disk by deleting
//             unnecessary files or export the folder to another disk.

#define ERROR_TA_ACC_FILE                                  407
//* Define statement owner: LEE
// Text:       The table '%1' could not be accessed.
//* Displayed Buttons: (OK) CANCEL
// HelpTitle:  OpenTM2 - Help
// Help:       The specified table has either
//             been deleted or is damaged and so cannot be accessed.
//             Reinstall OpenTM2.

#define ERROR_TA_DISKFULL_DOC_EXPORT                       408
//* Define statement owner: LUE, LEE
// Text:       There is not enough space on the disk to export
//             the document '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified disk drive is full. Make space on this disk
//             drive and restart the function, or select another disk drive
//             if possible.

#define ERROR_TA_SOURCEFILE                                409
//* Define statement owner: DOE, LEE
// Text:       The document '%1' could not be accessed by the
//             analysis process.\n
//             Do you want to continue with text analysis?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       An error occurred while processing one of the source
//             documents. The specified document might not exist or could be
//             in use by another process.
//             .br
//             If you continue, the document where the error occurred
//             is skipped and the next document (if there is one)
//             is analyzed. Leaving this function stops the analysis
//             process.

#define ERROR_CANCELTA                                     410
//* Define statement owner: LEE
// Text:       Do you really want to cancel text analysis?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       When you cancel the analysis process only the files already
//             analyzed are saved. Neither the file being processed,
//             nor the following files are saved. Therefore, they will not be
//             available for translation.

#define INFO_NO_NOMATCH_SEGMENTS                           411
//* Define statement owner: XQG, LEE
// Text:       No untranslated segments were found.
// HelpTitle:  OpenTM2 - Help
// Help:       You requested creation of a file containing
//             untranslated segments, but no untranslated segments
//             were contained in the documents being analyzed.

#define ITM_COMPLETE                                       412
//* Define statement owner: XJR, LEE
// Text:       The initial Translation Memory was successfully built.\n
//               Segments to be translated: %1\n
//               Aligned Segments:          %2
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
// Add.Info:   '%1' = number of segments to be translated,
// Add.Info:   '%2' = aligned segments
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message

#define ERROR_NODOCS_ANALYSIS                              413
//* Define statement owner: DOE, LEE
// Text:       No documents are selected for analysis.
// HelpTitle:  OpenTM2 - Help
// Help:       To perform an analysis you need to select the
//             individual documents or the folder to be analyzed.

#define ERROR_NO_INDIC_SELECTED                            414
//* Define statement owner: DOE, LEE
// Text:       No dictionary is selected for analysis.
// HelpTitle:  OpenTM2 - Help
// Help:       To perform an analysis, you must select one or more
//             dictionaries to be searched during the analysis process.
//             Terms of the document that are not contained in the
//             dictionaries are added to the New Terms List. Terms that
//             are contained in the dictionary are added to the Found
//             Terms List.

#define ERROR_NO_INMDB_SELECTED                            415
//* Define statement owner: DOE, LEE
// Text:       No Translation Memory is selected.
// HelpTitle:  OpenTM2 - Help
// Help:       To perform an analysis, you must select one or more
//             Translation Memory databases to be searched during the analysis
//             process.
//             .br
//             The selected Translation Memory databases are checked
//             to see if they
//             already contain a translation for the segments.
//             If they do, these translations
//             are added to the Translation Memory folder.

#define ERROR_TA_ACC_TAGTABLE                              416
//* Define statement owner: XQG, LEE
// Text:       The markup table '%1' could not be accessed.
//* Displayed Buttons: CANCEL
// HelpTitle:  OpenTM2 - Help
// Help:       Check whether the markup table specified in the
//             folder properties of this folder is still valid.

#define ERROR_NO_LUVALUES_SAVED                            417
//* Define statement owner: DOE, LEE
// Text:       The defined settings cannot be saved.
// HelpTitle:  OpenTM2 - Help
// Help:       There might not be enough space on the selected disk to
//             run the analysis process. If you continue, the settings you
//             defined during this dialog
//             are not saved. Cancel the analysis process, and create more
//             space on the selected disk, or choose another disk.
//             .br
//             Another possibility is that the document you want to analyze,
//             or the folder that contains this document, is in use. Either
//             cancel the running processes, or wait
//             until they are finished and try again.

#define ERROR_TA_COMPL_ERR                                 418
//* Define statement owner: DOE, LEE
// Text:       Not all files have been successfully analyzed.
// HelpTitle:  OpenTM2 - Help
// Help:       The Analysis process is complete, but some files
//             remain unanalyzed. They could not be read, possibly because
//             they are non-text files, such as
//             executable files, which are not
//             intended for translation.

#define ERROR_CANCELARCHTM                                 419
//* Define statement owner: XQG NEW
// Text:       Do you really want to cancel the build of the archive
//             Translation Memory?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       When you cancel the build archive Translation Memory
//             process only the segments already processed have been
//             added to the archive Translation Memory.
//             Therefore, they will not be available for translation.

#define ERROR_DOC_TOUCHED                                  420
//* Define statement owner: LEE
// Text:       Translation of the document '%1' has already
//             started.\n
//             Do you want to overwrite the target document?
//* Displayed Buttons: YES NO CANCEL */
// HelpTitle:  OpenTM2 - Help
// Help:       The translation of the document has already been started.
//             Text Analysis overwrites the existing target document
//             when segmentation is done. Therefore the existing translation
//             will be lost. However, previously translated segments are
//             saved in the Translation Memory you used before.

#define ERROR_DOC_INUSE                                    421
//* Define statement owner: THO, LEE
// Text:       The document '%1' is in use
//             by another process. It will be ignored in Text Analysis.\n
//             Do you want to continue anyway?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       The document is in use by another process.
//             You can either skip
//             this document, or stop Text Analysis and wait until the
//             running processes are finished before you try again.

#define ERROR_TM_NOT_EXIST                                 422
//* Define statement owner: DOE, LEE
//*Text:       The Translation Memory '%1' specified in the folder properties
//*            of folder '%2' does not exist.
// Text:       The Translation Memory '%1' specified to be used with
//             this document/this folder does not exist.
// HelpTitle:  OpenTM2 - Help
//*Help:       Use the folder property dialog to specify an existing
//*             Translation Memory.
// Help:       Use either the folder property or the document property
//             dialog to specify an existing Translation Memory.

#define ERROR_CANNOT_LOAD_USER_EXIT                        423
//* Define statement owner: DOE, LEE
// Text:       The file '%1' that contains the user exits
//             for analysis could not be loaded.
// HelpTitle:  OpenTM2 - Help
// Help:       You should reinstall OpenTM2.
//             If the error continues, contact your system administrator.

#define ERROR_TA_TMERROR                                   424
//* Define statement owner: DOE, LEE
// Text:       The Translation Memory has returned an error.
//             Text Analysis is stopped. Reorganize the
//             Translation Memory before you work with it again.
// HelpTitle:  OpenTM2 - Help
// Help:       Your Translation Memory might be damaged.
//             Reorganize it before you work with it again.
//             .br
//             The Translation Memory might be in use by another process.
//             Wait until this process has finished and try again.

#define ERROR_TA_LPASSOC                                   425
//* Define statement owner: LEE
// Text:       The association of dictionaries failed.
//             Text Analysis is stopped.
// HelpTitle:  OpenTM2 - Help
// Help:       One or more dictionaries does not exist or cannot be found
//             with the specified file name.
//             .br
//             Some dictionaries might be damaged,
//             or they have been created with an older version of
//             OpenTM2,
//             in which case you should reorganize them and try again.

#define ERROR_TA_LPDICTOPEN                                426
//* Define statement owner: LEE
// Text:       The dictionary '%1' could not be opened.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified dictionary could not be opened for one
//             of the following reasons:
//             :ul.
//             :li.There is no dictionary with the specified name.
//             Correct the name of the dictionary.
//             :li.The network drive where the dictionary
//             resides is not connected.
//             :li.The dictionary you wanted to work with has been
//             created with an older version of OpenTM2,
//             or it is damaged. Reorganize the dictionary and try again.
//             :eul.

#define ERROR_TA_TERM_TOOLONG                              427
//* Define statement owner: LEE
// Text:       A term in the text is too long to be inserted in
//             the Terms List and is being skipped.\n
//             Do you want to continue the process?
// HelpTitle:  OpenTM2 - Help
// Help:       A term in the text was longer than 255
//             characters and could not be inserted in the
//             Terms Lists. If you continue, this term is
//             skipped. If you leave this function, no list is
//             created.

#define ERROR_TA_LPPROC                                    428
//* Define statement owner: LEE
// Text:       Errors prevented processing the terms in the
//             current segment '%1'.\n
//             Do you want to continue anyway?
// HelpTitle:  OpenTM2 - Help
// Help:       Errors prevented the complete processing of the terms
//             in the last segment. The current segment is incomplete.
//             You can continue, or stop this process.
// Add.Info:   process and condition leading to this message is still being
// Add.Info:   worked on....

#define ERROR_TA_ASDERR                                    429
//* Define statement owner: LEE
// Text:       The dictionary '%1' cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary you selected might be damaged, and
//             you should reorganize it. If the error continues,
//             reimport a back-up version.
//             .br
//             You might have specified the wrong language support file,
//             or the one you specified is inactive or damaged.
//             You should reinstall OpenTM2.

#define ERROR_TM_FOR_DOC_NOT_EXIST                         430
//* Define statement owner: JOR, LEE
// Text:       The Translation Memory '%1' specified in the document
//             properties of document '%2' does not exist.
// HelpTitle:  OpenTM2 - Help
// Help:       Use the document property dialog to specify an existing
//             Translation Memory.

#define ERROR_NO_NTL_SELECTED                              431
//* Define statement owner: LEE
// Text:       The file name for the New Terms list is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       Select a file name, or specify a new one,
//             to create a list of new terms.

#define ERROR_NO_FTL_SELECTED                              432
//* Define statement owner: LEE
// Text:       The file name for the Found Terms list is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       Select a file name, or specify a new one,
//             to create a list of found terms.

#define ERROR_NO_LIST_SELECTED                             433
//* Define statement owner: XQG, LEE
// Text:       Lists Creation was requested but neither the
//             New Terms List nor the Found Terms List nor
//             'Add terms to dictionary' were selected.
// HelpTitle:  Terms Lists creation
// Help:       Within the List settings dialog you can choose to create
//             a New Terms List or a Found Terms List or to add terms
//             to a dictionary. You can also choose any combination of
//             these choices.

#define ERROR_LIST_WRITING                                 434
//* Define statement owner: LEE
// Text:       The Terms List '%1' could not be written to disk.
// HelpTitle:  OpenTM2 - Help
// Help:       Errors prevented the successful creation of the Terms
//             List file.
//             Check whether there is enough space available on the
//             specified disk. Delete some unnecessary files
//             before you try again.

#define ERROR_MAX_DICTS                                    435
//* Define statement owner: LEE
// Text:       You have already selected the maximum number of
//             dictionaries or Translation Memory databases.
// HelpTitle:  OpenTM2 - Help
// Help:       The number of dictionaries and Translation Memory
//             databases
//             that can be used for text analysis is limited to 10.
//             Since you are already using the maximum of dictionaries
//             or Translation Memory databases
//             allowed this dictionary or Translation Memory
//             is not added to the list.

#define ERROR_DIALOG_VALUES_NOT_SAVED                      436
//* Define statement owner: LEE
// Text:       The defined settings cannot be saved.
// HelpTitle:  OpenTM2 - Help
// Help:       There might not be enough space on the selected disk to
//             run the analysis process. If you continue, the settings you
//             defined during this dialog
//             are not saved. Cancel the analysis process and create more
//             space on the selected disk, or choose another disk.
//             .br
//             Another possibility is that the document you want to analyze,
//             or the folder that contains this document, is in use. Either
//             cancel the running processes or wait
//             until they are finished and try again.

#define NTL_FILE_EXISTS                                    437
//* Define statement owner: LEE
// Text:       The New Terms List '%1' already exists.\n
//             Do you want to overwrite it?
// HelpTitle:  OpenTM2 - Help
// Help:       If you do not specify a new file name, the file
//             is overwritten with the new data.

#define FTL_FILE_EXISTS                                    438
//* Define statement owner: LEE
// Text:       The Found Terms List '%1' already exists.\n
//             Do you want to overwrite it?
// HelpTitle:  OpenTM2 - Help
// Help:       If you do not specify a new file name, the file
//             is overwritten with the new data.

#define ERROR_OPENING_CONTEXT_FILE                         439
//* Define statement owner: DOE, LEE
// Text:       The document '%1' could not be opened.
// HelpTitle:  OpenTM2 - Help
// Help:       The number of open files that are
//             allowed might need to be increased.
// Add.Info:   message may be obsolete

#define ERROR_READING_CONTEXT                              440
//* Define statement owner: DOE, LEE
// Text:       The file '%1' contains data that cannot be processed.
//             Text Analysis is stopped.
// HelpTitle:  OpenTM2 - Help
// Help:       Check the document to be analyzed to ensure that it is a
//             text file. Non-text files, for example, executable files,
//             are not intended for translation.

#define ERROR_WRITING_TAG_TO_LIST_FILE                     441
//* Define statement owner: DOE, LEE
// Text:       An error occurred while writing the tag '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       A file I/O error occurred while trying to write to
//             an output file.
// Add.Info:   message may be obsolete

#define INVALID_CHARACTER_IN_FILENAME                      442
//* Define statement owner: DOE, LEE
// Text:       The file '%1' contains a character that is not valid.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter the file name again, using only valid file name
//             characters.
//             .br
//             File names can have
//             up to eight alphanumeric characters,
//             followed by an extension of up to three
//             characters.

#define ERROR_NO_LIST_SELECTION                            443
//* Define statement owner: XQG, LEE
// Text:       You did not select New Terms List nor Found
//             Terms List nor 'Add terms to dictionary'.
// HelpTitle:  Terms Lists creation
// Help:       Within the List settings dialog you can choose to create
//             a New Terms List or a Found Terms List or to add terms
//             to a dictionary. You can also choose any combination of
//             these choices. You have to select at least one of the
//             choices to complete the list creation settings.

#define ERROR_COULDNT_LOAD_TAGFILE                         444
//* Define statement owner: DOE, LEE
// Text:       The internal table '%1' needed for list processing could not
//             be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       Close your OpenTM2 workbench and try again.
//             If the error continues,
//             reinstall OpenTM2,
//             or call your system administrator.

#define ERROR_OUTPUT_EQUAL_TO_EXCLUSION                    445
//* Define statement owner: DOE, LEE
// Text:       An output dictionary cannot be used as
//             an exclusion dictionary.
// HelpTitle:  OpenTM2 - Help
// Help:       Change either the output or the exclusion dictionary.
// Add.Info:   message may be obsolete

#define ERROR_EXCLUSION_IN_INPUTS                          446
//* Define statement owner: XQG
// Text:       The dictionary '%1' selected as an exclusion
//             dictionary cannot be used as an input dictionary.
// HelpTitle:  OpenTM2 - Help
// Help:       An exclusion dictionary cannot be used as
//             the dictionary to be searched for found or new terms.
//             Change either the exclusion dictionary or the
//             dictionary to be searched.

#define ERROR_OUTDICT_IN_INPUTS                            447
//* Define statement owner: XQG, NEW
// Text:       Dictionary '%1' cannot be used as input dictionary
//             and as an output dictionary at the same time.
// HelpTitle:  OpenTM2 - Help
// Help:       A dictionary selected as output dictionary for
//             found terms cannot be a dictionary to be searched
//             for at the same time.
//             Change either the output dictionary or remove
//             the dictionary from the dictionaries to be searched.

#define ERROR_EXCLUSION_IS_OUTPUT                          448
//* Define statement owner: XQG
// Text:       The dictionary '%1' selected as an output
//             dictionary cannot be used as an exclusion dictionary.
// HelpTitle:  OpenTM2 - Help
// Help:       An dictionary cannot be used as an exclusion
//             dictionary and as an output dictionary at the same
//             time.
//             :p.
//             Either change the exclusion dictionary or the
//             output dictionary.

#define ERROR_COPYRIGHTDICT_IN_INPUT                       449
//* Define statement owner: XQG, NEW
// Text:       The dictionary '%1' is copyrighted and
//             cannot be used as input dictionary for
//             found term lists or as an output dictionary.
// HelpTitle:  OpenTM2 - Help
// Help:       A copyrighted dictionary cannot be used as
//             input dictionary for a found terms list.
//             Also it could not be used to fill a dictionary with
//             found terms.
//             :p.
//             Either remove the dictionary from the list of
//             input dictionaries or deselect the found term list
//             checkbox or the output dictionary checkbox.
//* -------------------------------------------------------
//*     Messages for Utilities such as UtlError etc. ( DOS_ERROR messages ) */
//* -------------------------------------------------------

#define ERROR_NOT_READY_MSG                                450
//* Define statement owner: XQG, LEE
// Text:       Drive '%1' is not ready.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that the diskette is inserted correctly.
// Add.Info:   drive letter

#define ERROR_DISK_CHANGE_MSG                              451
//* Define statement owner: XQG, LEE
// Text:       Insert a diskette in drive '%1' and try again.
// HelpTitle:  OpenTM2 - Help
// Help:       Insert the diskette for the specified drive and try again.
// Add.Info:   drive letter

#define ERROR_SECTOR_NOT_FOUND_MSG                         452
//* Define statement owner: XQG, LEE
// Text:       An error occurred during read or write access to drive '%1'.
//             A specific sector could not be found.
// HelpTitle:  OpenTM2 - Help
// Help:       The program tried to read data from, or write it to, a disk
//             or diskette. However, it could not find a specific sector.
//             Your disk or diskette could be damaged.
//             Check the specified drive by running the DOS function CHKDSK
//             on it outside the OpenTM2 environment.
// Add.Info:   drive letter

#define ERROR_PATH_NOT_FOUND_MSG                           453
//* Define statement owner: XQG, LEE
// Text:       The path '%1' is not valid or cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       The drive or directory you specified either does not exist
//             or it contains an error.
//             .br
//             Directory specifications can have
//             up to eight alphanumeric characters,
//             followed by an extension of up to three
//             characters.
//             .br
//             Correct the name or names and try again.

#define ERROR_GENERAL_DOS_ERROR_MSG                        454
//* Define statement owner: XQG, LEE
// Text:       An unexpected file handling error occured. The error code is %1.
// HelpTitle:  OpenTM2 - Help
// Help:       An unexpected error occurred during file I/O.

#define ERROR_NETWORK_ACCESS_DENIED_MSG                    455
//* Define statement owner: DKS, NEW
// Text:       Network access denied.
// HelpTitle:  OpenTM2 - Help
// Help:       You are trying to access a drive that is currently not
//             accessible because it was either deleted manually or via
//             the 'Configure drives...' option
//             on the 'Utilities' pull-down.

#define ERROR_ACCESS_DENIED_MSG                            456
//* Define statement owner: XQG, LEE
// Text:       Access to '%1' failed because the file is
//             in use or because the file, directory, or drive is
//             read-only.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified file cannot be accessed for one of the
//             following reasons:
//             :ul.
//             :li.The file is in use by another process. Wait until
//             the process is finished and try again.
//             :li.The file name is the same as the name of a directory.
//             :li.The disk to which you are writing the file is
//             write-protected.
//             :li.The file's mode is read-only.
//             :eul.
//             Check the file name, the disk specification,
//             and the file mode. If necessary, correct the file name and
//             disk specification.
//             Furthermore,
//             if you are authorized to do so, remove the
//             write protection or the read-only attribute, and try again.
// Cause:      A DOS call returned ACCESS_DENIED, NETWORK_ACCESS_DENIED or
// Cause:      SHARING_VIOLATION
// Add.Info:   file name

#define ERROR_DISK_FULL_MSG                                457
//* Define statement owner: XQG, LEE
// Text:       Disk '%1' is full.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified disk is full. If this disk is a diskette,
//             insert an empty one, and retry the current function.
//             If this disk is a fixed disk, create space by deleting
//             unnecessary files, and try again.
// Add.Info:   drive name

#define INFO_ARCHTM_COMPLETED                              458
//* Define statement owner: XQG NEW
// Text:       Building of archive Translation Memory '%1'
//             completed successfully. %2 segments have been added.
// HelpTitle:  OpenTM2 - Help
// Help:       The archive Translation Memory was created and filled
//             with alreaydy translated segments from the selected
//             documents.

#define ERROR_ARCHTM_TERMINATED                            459
//* Define statement owner: XQG NEW
// Text:       Building archive Translation Memory '%1'
//             terminated abnormally.
// HelpTitle:  OpenTM2 - Help
// Help:       The build archvie Translation Memory process failed
//             due to an error reported in previous messages.
//             Check these previously displayed error messages.
//             .br
//             The archive Translation Memory database has been build only
//             partially. You can continue working with the partially
//             builded version, but some data could be missing.

#define ERROR_ARCHTM_NOSEGSFOUND                           460
//* Define statement owner: XQG NEW
// Text:       Building archive Translation Memory '%1'
//             terminated. No translated segments found.
// HelpTitle:  OpenTM2 - Help
// Help:       The build archvie Translation Memory process has
//             completed but there no translated segments found
//             in the selected documents. Only documents which have
//             been translated will contain translated segments which
//             can be used to fill an archive Translation Memory.

#define ERROR_PROPERTY_LANG_DATA                           461
//* Define statement owner: DOE, NEW
// Text:       The language '%1' is not valid or the associated language
//             file is missing or corrupted.
// HelpTitle:  OpenTM2 - Help
// Help:       Error associated with a Translation Memory database:
//             :p.
//             Export the Translation Memory database and import it again
//             (valid source languages are displayed for selection).
//             :p.
//             Error associated with a dictionary:
//             :p.
//             The dictionary cannot be opened as the language support
//             file cannot be accessed. The 'Dictionary Properties'
//             dialog displays the dictionary source language.
//             .br
//             Compare this with the languages listed on the
//              'Display language list'
//             dialog on the 'Utilities' pull-down.
//             .br
//             If the nomenclature
//             differs or if the language has not been installed, re-install
//             the language.
//             .br
//             You can also choose to re-import the SGML dictionary file.
//             .br
//             To
//             do this, edit the language between the SGML tags &lt.source&gt.
//             and &lt./source&gt. to fit the language listed on the 'Display
//             language list' and select 'Import...'.
//             :p.
//             Error associated with Language List window:
//             :p.
//             This error is the result of an installation or programming
//             error. Try re-installing OpenTM2
//             to solve the problem.

#define ERROR_NETWORK_WRITE_FAULT                          462
//* Define statement owner: XQG, NEW
// Text:       Error writing to server disk '%1.
// HelpTitle:  OpenTM2 - Help
// Help:       A write operation to the specified server disk failed.
//             Probably the disk is full.
//             Check the free space on this disk, create space
//             by deleting unnecessary files, and try again.
//             If the problem continues, contact your LAN administrator.
// Add.Info:   drive name

#define QUERY_ARCHTM_EXISTS                                463
//* Define statement owner: XQG NEW
// Text:       The Translation Memory selected for the archive
//             function exists already.\n
//             Do you want to delete all segments contained
//             in the Translation Memory before filling
//             it with the translated segments?
// HelpTitle:  OpenTM2 - Help
// Help:       The selected Translation Memory exists already and
//             may contain data.
//             .br
//             Either allow to delete all segments stored in this
//             Translation Memory so that the Translation Memory
//             will contain only the segments added during the
//             build process or continue without deleting the data
//             contained in the Translation Memory.

#define NO_VALID_UNICODEFORMAT                              464
//* Define statement owner: XJR
// Text:       The file '%1' seems not to be a Unicode file.  \n
//             Would you like to proceed?
// HelpTitle:  OpenTM2 - Help
// Help:       Please check the format you have selected.

#define ERROR_PROFILE_NOSAVEALLOWED                        465
//* Define statement owner: XQG
// Text:       Profile '%1' is protected and may not be used in
//             save operations.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected profile is a protected system profile. It is not
//             possible to save profile data using the name of a system
//             profile.

#define ERROR_MEM_DIFFERENT_SOURCE_LANG_IMPORT             466
//* Define statement owner: XQG
// Text:       The source language of the imported memory is different 
//             from the source lanuage of the target memory.
//             The source language of the imported memory
//             is '%1' and the source language of the target 
//             memory is '%2'.\n
//             Are you sure that you want to continue the import?
// HelpTitle:  OpenTM2 - Help
// Help:       Merging two Translation Memory databases with different source,
//             but identical target, languages is only sometimes helpful.
//             For instance,
//             when translating a document from American English to German,
//             you might want to consult the British English to German
//             Translation Memory on the same subject. Otherwise, basing
//             a translation on
//             two unrelated
//             source languages (for example, Spanish and French) will not
//             result in consistent translation proposals.


#define ERROR_COUNT_WINDOW_OPEN                            467
//* Define statement owner: XQG
// Text:       There is already an open count window for this folder.
//             Close this window before starting a new word count.
// HelpTitle:  OpenTM2 - Help
// Help:       OpenTM2 supports only one word count result window
//             per folder. Please close the existing count window and retry.

#define ERROR_CALC_WINDOW_OPEN                             468
//* Define statement owner: XQG
// Text:       There is already an open counting report window for this folder.
//             Close this window before starting a new counting report.
// HelpTitle:  OpenTM2 - Help
// Help:       OpenTM2 supports only one counting report window
//             per folder. Please close the existing window and retry.

#define ERROR_SAME_WILDCARDS                              469
//* Define statement owner: XQG
// Text:       Same character used for single and multiple wildcard.
// HelpTitle:  OpenTM2 - Help
// Help:       The wildcard for single characters is the same as 
//             the wildcard for multiple charatcers\n
//             Choose different wildcard characters before continuing.

#define ERROR_FOLIMP_CONTROLLEDFOLDEREXISTS               470
//* Define statement owner: XQG
// Text:       The folder %1 exists already.
//             Although this folder has the same name as the imported
//             folder it is a different folder.
//             Do you want to continue the folder import?
// HelpTitle:  OpenTM2 - Help
// Help:       The import of another folder with the same name is not
//             allowed, since the two folders could contain different
//             documents, or might have different folder properties.
// Cause:      Unique folder IDs mismatch.


#define ERROR_FOLIMP_CONTROLLEDFOLDERMERGE               471
//* Define statement owner: XQG
// Text:       There is already a controlled folder named %1.
//             Do you want to merge the imported folder into 
//             this folder?
// HelpTitle:  OpenTM2 - Help
// Help:       The merge of the folder into an existing controlled
//             folder has to be confirmed. You also need the 
//             password of the controlled folder to continue.


#define ERROR_REPPATH_NOT_EXIST                                472
//* Define statement owner: 
// Text:       The path specified for the report output file
//             does not exist or can not be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       Correct the path name and try again.

#define ERROR_XSL_NOT_FOUND                                    473
//* Define statement XQG
// Text:       The XSL style sheet %1 is missing. Please contact 
//             IBM and report this error or try another output
//             format.
// HelpTitle:  OpenTM2 - Help
// Help:       A style sheet required for the conversion of the
//             calculation report is missing. Contact IBM to report
//             the missing style sheet. You can however try a different
//             report putput format.

#define ERROR_XML_CONVERSION                                  474
//* Define statement owner: XQG
// Text:       An error occured while converting %2 using conversion
//             style sheet %3, the error text is\n
//             "%1".
// HelpTitle:  OpenTM2 - Help
// Help:       A conversion error occured. Please retry the operation.
//             Report the prblem if the error persists.

#define INFO_HISTLOG_CORRECTED                            475
//* Define statement owner: XQG
// Text:       The file containing the folder history information
//             was damaged and has been automatically corrected.\n
//             \n
//             The defective entries were located in the period from %1 to %2.\n
//             \n
//             The original file containing the damaged history information 
//             has been saved to %3.\n
//             \n
//             Please send this file to %4 for further investigations.
// HelpTitle:  OpenTM2 - Help
// Help:       The file containing the folder history is damaged.
//             Please note the conditions that have led to that error,
//             and contact your IBM representative.

#define ERROR_USEREXIT_INUSE                              476
//* Define statement owner: THO, LEE
// Text:       The user exit '%1' is currently in use and
//             could not be updated with a newer version.\n
//             The update will be performed automatically when
//             OpenTM2 is restarted.
// HelpTitle:  OpenTM2  - Help
// Help:       Restart OpenTM2 and repeat the import.

#define ERROR_NO_REPLLIST_SELECTED                         477
//* Define statement owner: LEE
// Text:       The file name for the replacement list is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       Select the name of an existing replacemet list.

#define ERROR_IN_REPLLIST                                 478
//* Define statement owner: LEE
// Text:       The load of replacement list %1 failed.\n
//             The reason is\n
//             %2
// HelpTitle:  OpenTM2 - Help
// Help:       Replacement list could not be loaded.


#define ERROR_REDREPORT_DOCMISMATCH                      479
//* Define statement owner: XQG 
// Text:       The documents selected for the redundancy report do not match
//             the documents prepared for redundancy counting during analysis.\n
//              \n
//             The report will be done for the prepared documents.\n 
//             \n
//             (%1)
// HelpTitle:  OpenTM2 - Help
// Help:       The documents which were prepared during analysis for redundancy counting
//             are not the one which have been selected for the redundancy report. 
//             Either some documents haven not been prepared or more documents have
//             been prepared but are not selected for the redundancy report.
//             The report will be created for the documents which have been prepared
//             for redunancy counting.
//*
//* -------------------------------------------------------
//*     Continuation of Translation Memory messages
//* -------------------------------------------------------

#define ERROR_MEM_MERGE_INITFAILED                         480
//* Define statement owner: DOE, LEE
// Text:       Merging Translation Memory
//             '%1' into Translation Memory '%2' failed during initialization.
// HelpTitle:  OpenTM2 - Help
// Help:       The initialization process for merge failed due
//             to an internal error.
//             Either there is not enough memory available, or
//             the system resources are used up.
//             Close some running applications and try again.

#define MESSAGE_MEM_MERGE_COMPLETED                        481
//* Define statement owner: DOE, LEE
// Text:       Merging Translation Memory '%1' into Translation Memory '%2'
//             completed successfully. %3 segments have been merged.
//             %4 segments were obsolete.
// HelpTitle:  OpenTM2 - Help
// Help:       Obsolete segments have a more recent equivalent already listed
//             in the Translation Memory and are therefore not merged.

#define ERROR_MEM_MERGE_TERMFAILED                         482
//* Define statement owner: DOE, LEE
// Text:       Termination of merging Translation Memory
//             '%1' into Translation Memory '%2' failed.
// HelpTitle:  OpenTM2 - Help
// Help:       The termination process for merge failed due to an error
//             reported in previous messages.
//             Check these previously displayed error messages.
//             .br
//             The Translation Memory databases have been merged only
//             partially. You can continue working with the partially merged
//             version, but some data could be missing.

#define ERROR_MEM_MERGE_TERMINATED                         483
//* Define statement owner: DOE, LEE
// Text:       Merging Translation Memory '%1' into Translation
//             Memory '%2' terminated abnormally.
// HelpTitle:  OpenTM2 - Help
// Help:       The merging process failed due to an error
//             reported in previous messages.
//             Check these previously displayed error messages.
//             .br
//             The Translation Memory databases have been merged only
//             partially. You can continue working with the partially
//             merged version, but some data could be missing.

#define ERROR_MEM_MERGE_TERM_FORCED                        484
//* Define statement owner: DOE, LEE
// Text:       Do you really want to cancel merging Translation Memory
//             '%1' into Translation Memory '%2'?
// HelpTitle:  OpenTM2 - Help
// Help:       If you cancel the merge process,
//             the Translation Memory databases are only partially merged.
//             You can continue working with the partially merged version,
//             but some data could be missing.
//             .br
//             Otherwise confirm to continue with the merge process.

#define ERROR_MEM_IMPORT_TERM_FORCED                       485
//* Define statement owner: DOE, LEE
// Text:       The import of Translation Memory '%1'
//             was stopped before completion.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to work with a completely imported version of
//             this Translation Memory, try importing again to get a
//             successful completion.
//             .br
//             If you cancel the import process,
//             this Translation Memory is only partially imported.
//             You can continue working with the partially imported version,
//             but some data could be missing.

#define ERROR_MEM_IMPORT_CORR_BACKLEVEL                    486
//* Define statement owner: DOE, TAF, LEE
// Text:       Importing cannot be performed because
//             Translation Memory '%1' is damaged, or it was created with
//             an older version of OpenTM2.\n
//             Do you want to start organizing
//             the Translation Memory '%1' now?
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory in your system is either damaged or
//             it was created with an older version of OpenTM2.
//             Organize the Translation Memory in your system before
//             you restart importing.

#define QUERY_MEM_FOLIMP_MERGE                             487
//* Define statement owner: XQG
// Text:       The Translation Memory '%1' already exists.\n
//             Do you want to merge the folder Translation Memory into this
//             Translation Memory?
// HelpTitle:  OpenTM2 - Help
// Help:       A Translation Memory exists with the same name as a
//             Translation Memory in the folder you are importing.
//             If you do not want to merge, this Translation Memory
//             is not imported. If you want to use the Translation Memory
//             in the folder dictionary, export the existing
//             Translation Memory and reimport it with a different name.
//             Delete the existing Translation Memory and restart the
//             folder import process.

#define ERROR_MEM_NOT_REPLACED_CONFIRM                     488
//* Define statement owner: DOE, LEE, LEE
// Text:       In Translation Memory '%1' a request to replace a segment
//             was made, but there is a more recent version of the
//             segment already in the Translation Memory.\n
//             Do you want to continue without displaying this
//             confirmation message for each segment?
// HelpTitle:  OpenTM2 - Help
// Help:       A more recent version of the segment you are
//             trying to replace is already in the Translation Memory.
//             More recent segments are not replaced.
//             Ensure that the date and time on your personal computer are
//             correct. If not, correct them and retry.
//             If you continue without displaying the confirmation message
//             for each segment, the process continues and you will not
//             be told when a more recent segment exists in the
//             Translation Memory.
//             Select 'Cancel' to stop the process.
// Type:       EQF_QUERY
// Severity:   YES NO CANCEL
// Cause:      TmReplace (or TmtReplace) returns ERROR_MEM_NOT_REPLACED
// Add.Info:   '%1' = TM name

#define QUERY_IMPORT_MERGE_CONFIRMATION                    489
//* Define statement owner: DOE, TAF, LEE
// Text:       A Translation Memory with the same name already exists.
//             Although these Translation Memory databases have the same
//             names, their contents could be different.\n
//             Do you really want to import the specified
//             Translation Memory and merge it with the existing one?
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory to be imported and the
//             existing one have the same name. However, their properties and
//             therefore their contents could be different
//             (for example, their source and target languages could be
//             different). Importing the specified Translation
//             Memory means that it will be merged with the existing one.
//             .br
//             Note that if
//             you merge Translation Memory databases with
//             different properties, and possibly different source and target
//             languages, the Translation Memory proposals you
//             get may be inconsistent.
// Type:       EQF_QUERY
// Severity:   YESNO

#define ERROR_MEM_DIFFERENT_SOURCE_LANG                    490
//* Define statement owner: DOE, LEE
// Text:       The source languages of the Translation Memory databases
//             you want to merge are different.
//             The source language
//             of '%1' is '%3', and the source language of '%2' is '%4'.\n
//             Are you sure that you want to merge them?
// HelpTitle:  OpenTM2 - Help
// Help:       Merging two Translation Memory databases with different source,
//             but identical target, languages is only sometimes helpful.
//             For instance,
//             when translating a document from American English to German,
//             you might want to consult the British English to German
//             Translation Memory on the same subject. Otherwise, basing
//             a translation on
//             two unrelated
//             source languages (for example, Spanish and French) will not
//             result in consistent translation proposals.

#define ERROR_MEM_DIFFERENT_TARGET_LANG                    491
//* Define statement owner: DOE, LEE
// Text:       The target languages of the Translation Memory databases
//             you want to merge are different.
//             The target language
//             of '%1' is '%3', and the target language of '%2' is '%4'.\n
//             Are you sure that you want to merge them?
// HelpTitle:  OpenTM2 - Help
// Help:       If you continue, the merged Translation
//             Memory will contain translations in both languages.
//             This means that you will get Translation Memory proposals
//             in both languages during the translation of a document.
//             .br
//             Merging two Translation Memory databases with different target,
//             but identical source, languages is only sometimes helpful.
//             For instance, you might find it helpful to display
//             a previously translated version of a specific document,
//             even though this could have been created in a different
//             target language. Thus, the English translation of a German
//             document
//             could be helpful in creating the French
//             translation.
//             .br
//             However, using two closely related target languages (like
//             British English and American English) is likely to be
//             unhelpful.

#define MEM_LOAD_MERGE_CONFIRMATION                        492
//* Define statement owner: DOE, LEE
// Text:       The Translation Memory '%1' already exists.\n
//             Do you want to merge the file '%2' into this Translation
//             Memory?
// HelpTitle:  OpenTM2 - Help
// Help:       The selected Translation Memory already exists, but
//             the properties and
//             therefore the contents of the existing Translation Memory
//             and the file to be imported may be completely different.
//             For example, their source and target languages could be
//             different. Importing the specified file means that it will
//             be merged with the existing Translation Memory.
//             .br
//             Note that
//             when you merge Translation Memory databases with
//             different properties or different source and target
//             languages, it can lead to inconsistent Translation Memory
//             proposals.
// Type:       EQF_QUERY
// Severity:   YESNO

#define QUERY_MEM_DELETE_GREYEDOUT                         493
//* Define statement owner: DOE, NEW
// Text:       Translation Memory '%1' is not accessible. Only the files
//             that can be accessed will be deleted. Are you sure
//             you want to delete the Translation Memory?
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory is greyed out because it is not
//             accessible. The reason for this can be twofold.
//             Firstly, the drive on which it is located may have been deleted
//             manually. Deleting the Translation Memory in this case means
//             that parts of the Translation Memory will be physically
//             deleted. If you allow access to this drive again after deleting
//             the Translation Memory, it will not be accessible as parts will
//             have been deleted.
//             Secondly, the drive may have been removed
//             via the 'Configure drives...' option
//             on the 'Utilities' pull-down. Again, deleting will
//             physically remove parts of the Translation Memory. If the drive
//             is added to the list of configured drives again, the
//             Translation Memory will not be accessible.
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      grayed out TM should not be deleted
// Add.Info:   '%1' = TM name

#define ERROR_MEM_DESCRIPTION_MISSING                      494
//* Define statement owner: DOE, NEW
// Text:       You have not specified a Translation Memory description.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       A Translation Memory description is not required,
//             but it makes it easier to identify
//             the Translation Memory databases.
//             Continue if you do not want to specify a description.
// Type:       MB_QUERY
// Severity:   YESNO
// Cause:      no description specified on TM new dialog

#define ERROR_MEM_TAG_TABLE_TOO_LARGE                      495
//* Define statement owner: DOE, NEW
// Text:       The markup table '%1' is too large. The Translation Memory
//             could not be created.
// HelpTitle:  OpenTM2 - Help
// Help:       Make sure that you have the correct markup table installed
//             and the table contains only the tags you really need.
//             Export the markup table, delete the tags you do not need,
//             and import the table again.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      (size of tag table + size of noise list) list is greater
// Cause:      than the maximal buffer size of CREATE_IN structure

#define ERROR_MEM_INVALID_SGML                             496
//* Define statement owner: DOE, NEW
// Text:       There is an SGML syntax error in the file '%1'.
//             Segment number %2 was the last one successfully processed.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that the SGML syntax in the file
//             to be imported is correct or that the file to be
//             imported is indeed a Translation Memory file.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      invalid SGML in Translation Memory to be imported.
// Add.Info:   '%1' = SGML TM name

#define ERROR_MEM_INVALID_LNG_FILE                         497
//* Define statement owner: DOE, NEW
// Text:       File '%1' is corrupted.
// HelpTitle:  OpenTM2 - Help
// Help:       An internal OpenTM2 file is corrupted.
//             Reinstall OpenTM2.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      error in SOURCE.LNG or TARGET.LNG
// Add.Info:   '%1' = SOURCE.LNG or TARGET.LNG

#define ERROR_MEM_SEGMENT_TOO_LARGE_IMP                    498
//* Define statement owner: DOE, NEW
// Text:       The segment %1 in file '%2' is too large.
//             Skip that segment, or stop the
//             import process.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       You can either continue importing, in which case the segment
//             is skipped, or you can
//             cancel the import process and reduce the size of the
//             translation. Then try to import the segment again.
// Type:       EQF_ERROR
// Severity:   YES, NO
// Cause:      source, translation and secondary key exceeds size of
// Cause:      bufdata in structure SEGMENT
// Add.Info:   '%1' = segment number
// Add.Info:   '%2' = TM to be imported

#define ERROR_MEM_SEGMENT_TOO_LARGE                        499
//* Define statement owner: DOE, NEW
// Text:       The segment is too large. It won't be saved in the
//             Translation Memory.
// HelpTitle:  OpenTM2 - Help
// Help:       The source and the translation of the segment exceeds
//             the maximum size. Reduce the size of the translation.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      source, translation and secondary key exceeds size of
// Cause:      bufdata in structure SEGMENT
//* -------------------------------------------------------
//*     Messages for EQF services
//* -------------------------------------------------------

#define ERROR_XML_LOADERROR                                500
//* Define statement owner: XQG
// Text:       An error occured while loading XML file %1\n
//             The error text is "%2",\n
//             the error location is %3.
// HelpTitle:  OpenTM2 - Help
// Help:       A conversion error occured. Please retry the operation.
//             Report the prblem if the error persists.


#define EQFRS_NO_INIT                                      501
//* Define statement owner: JOR, LEE
// Text:       The workbench services are not initialized.
// HelpTitle:  OpenTM2 - Help
// Help:       The workbench services, that is, the Translation Memory and
//             the Dictionary services, could not be activated. If there
//             are other programs running, you can either cancel them, or
//             wait until they are finished, and try again.

#define EQFRS_ALREADY_INIT                                 502
//* Define statement owner: JOR, LEE
// Text:       Translation Memory is already initialized.
// HelpTitle:  OpenTM2 - Help
// Help:       A previously opened Translation Memory could not be closed.
//             Therefore no new one can be opened. Close
//             OpenTM2 and restart it.

#define ERROR_START_MT                                     503
//* Define statement owner: JOR, LEE
// Text:       The Machine Translation subsystem '%1' could
//             not be started.\n
//             Do you want to continue without using it?
// HelpTitle:  OpenTM2 - Help
// Help:       The specified dynamic link library (DLL) describing the
//             Machine Translation subsystem could not
//             be found or started.
//             Ensure that the specified DLL is in the
//             access path for DLLs and that it contains the
//             necessary entry points.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : MT DLL name

#define EQFS_MTERROR                                       504
//* Define statement owner: JOR, LEE
// Text:       The Machine Translation subsystem returned
//             the following error:\n
//             %1
// HelpTitle:  OpenTM2 - Help
// Help:       Please look up the returned error in the Machine
//             Translation subsystem documentation.
//*
// Type:       EQF_CANCEL
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define EQFRS_INVALID_CMD                                  505
//* Define statement owner: JOR, LEE
// Text:       The issued command is not valid.
// HelpTitle:  OpenTM2 - Help
// Help:       Check the requested command again. If the problem
//             continues, contact your IBM representative.

#define ERROR_XSLT_CONV_LOAD                               506
//* Define statement owner: XQG, NEW
// Text:       The XSLT conversion DLL '%1' could not be loaded or
//             is corrupted.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified XSLT conversion DLL could not be found, one or
//             prerequisite DLLs are missing or the DLL is corrupted.

#define MESSAGE_MEM_LOAD_COMPLETED2                         507
//* Define statement owner: XQG
// Text:       The import of file '%2' into Translation Memory '%1'
//             was completed successfully.\n
//             %3 segments have been imported.
//             %4 segments were not valid or they are obsolete.\n
//             \n
//             The list of invalid segments can be found in %5.
// HelpTitle:  OpenTM2 - Help
// Help:       Segments need to have valid syntax to be imported.
//             Segments that have a more recent equivalent in the
//             Translation Memory are also not imported.

#define EQFRS_TM_NOT_ACTIVE                                508
//* Define statement owner: JOR, LEE
// Text:       Translation Memory could not be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected Translation Memory could not be
//             activated as it is in use by another process.
//             Wait until this process has been completed,
//             and then try accessing the Translation Memory again.

#define ERROR_PROP_FROM_RO_MEMORY                          509
//* Define statement owner: XJR
// Text:       The currently selected proposal cannot
//             be deleted because it is retrieved from
//             a search-only Translation Memory.
// HelpTitle:  OpenTM2 - Help
// Help:       You asked to delete a proposal which is
//             retrieved from a search-only Translation Memory
//             associated with the folder.
//             Only proposals which are part of the
//             document/folder Translation Memory can
//             be deleted.

#define EQFRS_ENTRY_NOT_AVAIL                              510
//* Define statement owner: JOR, LEE
// Text:       The requested copy from the dictionary or Translation Memory
//             window is not available.
// HelpTitle:  OpenTM2 - Help
// Help:       You asked to copy a dictionary match or a Translation Memory
//             proposal that is not available.
//             Check your selection and try again.

#define EQFRS_SYSTEM_ERR                                   511
//* Define statement owner: JOR, LEE
// Text:       A system error occurred (system return code = %1).
// HelpTitle:  OpenTM2 - Help

#define ERROR_MEMIMP_ERROR                                 512
//* Define statement owner: XQG
// Text:       Memory import error:\n
//             %1
// HelpTitle:  OpenTM2 - Help
// Help:       Import of memory failed

#define WARNING_MEMIMP_ASCIIORANSI                         513 
//* Define statement owner: .............
// Text:       The memory being imported seems to be in the 
//             format %1. The selected import format is however %2.\n
//             Do you want to continue the import using the selected import format?
// HelpTitle:  OpenTM2 - Help
// Help:       The selected import format  does not match the type
//             of the imported file. You can either continue the import
//             using he incorrect format or you can cancel the import
//             and select the correct import format.

#define ERROR_ADDENDAORABBREV_NOTPOSSIBLE                  514
//* Define statement owner: XJR
// Text:       The new term(s) cannot be added to the
//             addendum or abbreviation list.
//             Please check whether your terms are valid.
// HelpTitle:  OpenTM2 - Help
// Help:       Only terms consisting of one word are valid.
// Type:       EQF_ERROR
// Severity:   CANCEL


#define EQFS_TM_NOT_FOUND                                  515
//* Define statement owner: JOR, LEE
// Text:       Translation Memory '%1' could not be found.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified Translation Memory was not found.
//             Check whether it really exists and whether it is
//             properly accessed.

#define EQFRS_SEG_NOT_FOUND                                516
//* Define statement owner: JOR, LEE
// Text:       The segment was not found in Translation Memory '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       The requested segment was not found in the Translation
//             Memory. The requested action could not be performed
//             on the segment.

#define ERROR_NOADD_SUPPORT                                517
//* Define statement owner: XQG
// Text:       The language support for language '%1' does not
//             not support addendum dictionaries.
//             The request to add the word to the addendum
//             or to edit the addendum dictionary is ignored.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified language support does not support
//             addendum dictionaries.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1'  language name

#define ERROR_NOABBR_SUPPORT                               518
//* Define statement owner: XQG
// Text:       The language support for language '%1' does not
//             not support abbreviation term dictionaries.
//             The request to add the word to the abbreviation terms
//             or to edit the abbreviation terms is ignored.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified language support does not support
//             abbreviation term dictionaries.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1'  language name

#define ERROR_NOABBR_DICT                                  519
//* Define statement owner: XQG
// Text:       The specified abbreviation term dictionary '%1' could not
//             be accessed.
//             The request to add the word to the abbreviation terms
//             or to edit the abbreviation terms is ignored.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified abbreviation term dictionary for the
//             source language could not be accessed.
//             Check that the language support file
//             referred to with your abbreviation is properly installed.
//             If it is not, reinstall the specified language
//             support file.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1'  abbreviation name

#define ERROR_DOC_LOCKED                                   520
//* Define statement owner: JOR, LEE
// Text:       The selected document '%1' is in use at the moment
//             by another process. The function you have selected
//             to be carried out on '%1' is stopped.
// HelpTitle:  OpenTM2 - Help
// Help:       Either wait until the other process has been completed
//             or stop it prematurely. Then
//             restart the last function you selected.

#define ERROR_FOLDER_LOCKED                                521
//* Define statement owner: JOR, LEE
// Text:       The selected folder '%1' is in use at the moment
//             by another process. The function you have selected
//             to be carried out on '%1' is stopped.
// HelpTitle:  OpenTM2 - Help
// Help:       Either wait until the other process has been completed
//             or stop it prematurely (cancel processing). Then
//             restart the last function you selected.

#define EQFRS_AREA_TOO_SMALL                               522
//* Define statement owner: JOR, NEW
// Text:       The passed data area is too small to hold all of the data.
// HelpTitle:  OpenTM2 - Help
// Help:       To get all the information, increase the area size and try
//             again.
// Type:       EQF_INFO
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   None

#define EQFRS_NOMORPH_DICT                                 523
//* Define statement owner: JOR, TAF, LEE
// Text:       The specified language support file '%1' is not
//             available or only accessible if the corresponding
//             operating system is installed.
// HelpTitle:  OpenTM2 - Help
// Help:       The requested language support file for the
//             source language could not be accessed.
//             Check that it is properly installed on your
//             system. If it is not, reinstall the specified language
//             support file.
//             For certain languages (e.g. Japanese) it is required
//             that the corresponding operating system is installed.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1'  dictionary name

#define EQFRS_NOADDENDA_DICT                               524
//* Define statement owner: JOR, TAF, LEE
// Text:       The specified addendum dictionary '%1' could not
//             be accessed.
//             The request to add the word to the addendum is ignored.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified addendum dictionary for the target
//             language could not be accessed.
//             Check that the language support file
//             referred to with your addendum is properly installed.
//             If it is not, reinstall the specified language
//             support file.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1'  addenda name
//* -------------------------------------------------------
//*     Messages for accessing TOLSTOY/DAM
//* -------------------------------------------------------

#define EQFS_DA_ERROR                                      525
//* Define statement owner: XQG, JOR, LEE
// Text:       An error occurred while accessing the language support files.
// HelpTitle:  OpenTM2 - Help
// Help:       You are trying to access a language support file
//             that is damaged. Use the dictionary properties to check
//             which source language support files are needed. Run
//             'Display language list...' on the 'Utilities' pull-down
//             to see whether the required source language support
//             files are active. If not, reinstall the language
//             required.

#define EQFS_BAD_DICT                                      526
//* Define statement owner: JOR, LEE
// Text:       The language support files could not be opened.
// HelpTitle:  OpenTM2 - Help
// Help:       You are trying to open language support files
//             that are not accessible. Use dictionary properties to check
//             which source language support files are needed. Run
//             'Display language list...' on the 'Utilities' pull-down
//             to see whether the required source language support
//             files are active. If not, reinstall the language
//             required.

#define EQFS_DICT_MEM_ERR                                  527
//* Define statement owner: JOR, LEE
// Text:       The language support files cannot be accessed due to
//             memory shortage.
// HelpTitle:  OpenTM2 - Help
// Help:       The system does not have enough memory available to access
//             the language support files. Close some running processes
//             in order to free memory.

#define EQFS_BAD_DICT_NAME                                 528
//* Define statement owner: JOR, LEE
// Text:       The language support file names are incorrect.
// HelpTitle:  OpenTM2 - Help
// Help:       Use dictionary properties to check
//             which source language support files are needed for
//             the dictionary you are trying to access.
//             Run 'Display language list...' on the 'Utilities' pull-down
//             to see whether the required source language support
//             files are active. If not, reinstall the language
//             required.

#define EQFS_FILE_ACC_DENIED                               529
//* Define statement owner: JOR, LEE
// Text:       The language support files cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       Use dictionary properties to check
//             which source language support files are needed for
//             the dictionary you are trying to access.
//             Run 'Display language list...' on the 'Utilities' pull-down
//             to see whether the required source language support
//             files are active. If not, reinstall the language
//             required.

#define EQFS_FILE_NOT_FOUND                                530
//* Define statement owner: DKS, LEE
// Text:       The language support files cannot be found.
// HelpTitle:  OpenTM2 - Help
// Help:       You are trying to access a language support file
//             that cannot be found. Use dictionary properties to check
//             which source language support files are needed. Run
//             'Display language list...' on the 'Utilities' pull-down
//             to see whether the required source language support
//             files are active. If not, reinstall the language
//             required by the dictionary.

#define EQFS_FILE_OPEN_FAILED                              531
//* Define statement owner: JOR, NEW
// Text:       Dictionary or Translation Memory '%1' cannot be opened.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected dictionary or Translation Memory cannot be opened,
//             because files may either be not accessible or corrupted or, if
//             the dictionary or Translation Memory is
//             shared, are on a network drive which is currently no accessible.
//             If a local dictionary or Translation Memory is not accessible,
//             files may have been deleted in which case it can only be
//             rebuild by importing a backup copy of the file.
//             If the dictionary or Translation Memory is corrupted,
//             try reorganizing it. For shared dictionaries or Translation
//             Memory databases located on network
//             drives check if the network is up and running, if the machine
//             supplying the drive is started and connected to the network
//             and that the drive is still attached.

#define EQFS_FILE_SHARING_VIOL                             532
//* Define statement owner: JOR, LEE
// Text:       Language support files cannot be shared.
// HelpTitle:  OpenTM2 - Help
// Help:       Run 'Display language list...' on the 'Utilities' pull-down
//             to see whether the required source language support
//             files are active. If not, reinstall the language
//             required.

#define EQFS_DICT_WRITE_ERR                                533
//* Define statement owner: XQG, NEW
// Text:       You cannot write to the selected dictionary.
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary is either write-protected, or
//             the disk on which it resides is full, or
//             it is not accessible.
//             .br
//             Check if there is enough disk space available and
//             whether the drive on which the dictionary resides is
//             accessible. Restart the function.

#define EQFS_DICT_ALREADY_OPEN                             534
//* Define statement owner: JOR, LEE
// Text:       The dictionary '%1' is already opened.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified dictionary is being used by another
//             user or process.
//             Wait until it is not used any more, and try again.

#define EQFS_TOO_MANY_DICTS                                535
//* Define statement owner: JOR, LEE
// Text:       You have reached the limit for the number of dictionaries
//             that can be opened (10).
// HelpTitle:  OpenTM2 - Help
// Help:       You have exceeded the limit of open dictionaries for
//             the active program. A maximum of ten dictionaries
//             can be selected at a time.
//             .br
//             Close some dictionaries associated with the open folder,
//             and then try again.

#define EQFS_DICT_ASSOC                                    536
//* Define statement owner: JOR, LEE
// Text:       The selected dictionaries for the automatic dictionary
//             lookup have different levels and cannot be used together.
// HelpTitle:  OpenTM2 - Help
// Help:       One of the selected dictionaries is back level
//             and cannot be used together with the others.
//             Organize the dictionaries and try again.
// Add.Info:   message may be obsolete

#define EQFS_DICT_START                                    537
//* Define statement owner: JOR, LEE
// Text:       The dictionary could not be started or did not
//             respond to system requests.
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary could not be started, because
//             of memory shortage.
//             End some other applications, and close the translation
//             environment. Then try again.

#define EQFS_DICT_GENERAL_ERROR                            538
//* Define statement owner: DKS, LEE
//* this is a general dictionary error
// Text:       An error occurred while accessing the dictionary or
//             Translation Memory %1. \n
//             The error code was: %2
// HelpTitle:  OpenTM2 - Help
// Help:       The selected dictionary or Translation Memory cannot
//             be opened or accessed.
//             The reason for this might be one of the following:
//             :ul.
//             :li.Dictionary or Translation Memory is damaged
//             :li.Disk containing dictionary or Translation Memory is full
//             :li.Dictionary or Translation Memory is in exclusive use by
//             another process
//             :li.Source language support file missing
//             :eul.
//             If the dictionary or Translation Memory is damaged,
//             reorganize it by selecting
//             'Organize' from the 'File' pull-down to repair it.
//             You can also rebuild it by importing the corresponding
//             SGML file. You do this by selecting 'Import...' from the
//             'File' pull-down and selecting the SGML file. If the
//             source language support file cannot be accessed, select
//             'Display language list...' from the 'Utilities' pull-down
//             to see whether the required source language support
//             file is active. If not, reinstall the language
//             required.

#define ERROR_DICT_CORRUPTED                               539
//* Define statement owner: DKS, LEE
// Text:       The dictionary '%1' is damaged.\n
//             Do you want to keep the organized copy?
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary organize process has to be stopped because your
//             dictionary is damaged.
//             You can keep the organized copy, which contains everything
//             up to the erroneous entry. You can work with the dictionary,
//             but some data might be lost, depending on where the error
//             occurred.
//             .br
//             Or you can keep the original unorganized copy of the
//             dictionary, in which case the dictionary remains damaged and
//             cannot be used (this might be useful for error tracking).

#define ERROR_BAD_LANG_NAME                                540
//* Define statement owner: XQG, NEW
// Text:       The language support file name %1 specified in the
//             SGML file is incorrect.
// HelpTitle:  OpenTM2 - Help
// Help:       The source language specified in the header section
//             of the SGML file being imported is no source
//             language supported by OpenTM2.
//             Run 'Display language list...' on the 'Utilities' pull-down
//             to see whether the required source language support
//             files are active. If not, reinstall the language
//             required or change the source language specified in
//             the SGML file.
//* -------------------------------------------------------
//*     Messages for ARTEMIS Work with Dicts (Lookup, Select, Edit)
//*     by SOTEC (Hermann)
//* -------------------------------------------------------

#define WARNING_MEMIMP_WRONGFORMAT                         541 
//* Define statement owner: .............
// Text:       The memory being imported seems to be in the 
//             format %1. The selected import format is however %2.\n
//             Do you want to import the memory in %3 format?
// HelpTitle:  OpenTM2 - Help
// Help:       The selected import format does not match the type
//             of the imported file. Select 'Yes' to import the
//             memory in the proposed format, select 'No' to 
//             import the memory in the selected format or
//             select 'Cancel' to cancel the memory import.

#define WD_START_DISPLAY                                   542
//* Define statement owner: XQG, LEE
//* source of problems: system limits
// Text:       The Lookup Entry dialog cannot be started.
// HelpTitle:  OpenTM2 - Help
// Help:       OpenTM2 could not start this dialog due to a
//             shortage of system resources. Close all
//             windows you do not need at the moment and retry.

#define WD_START_EDIT                                      543
//* Define statement owner: XQG, LEE
//* source of problems: system limits
// Text:       The Edit Entry in Dictionary dialog cannot be started.
// HelpTitle:  OpenTM2 - Help
// Help:       OpenTM2 could not start this dialog
//             due to a shortage of system resources. Close
//             all windows you do not need at the moment and retry.

#define XLIFF_DOCUMENT_DOES_NOT_EXIST                     544
//* Define statement owner: .............
// Text:       Document %1 does not exist in folder %2. 
//             The XLIFF folder import can only import documents
//             which exist already in the folder.\n
//             The import is terminated.
// HelpTitle:  OpenTM2 - Help
// Help:       The XLIFF file contains a document which is not 
//             contained in the current folder. Either this document
//             has been deleted or the folder is not the one from
//             which the XLIFF file had been exported.
//             Ensure that the XLIFF file is imported into the
//             original folder.

#define XLIFF_FOLDER_DOES_NOT_EXIST                     545
//* Define statement owner: .............
// Text:       The folder %1 to which the imported XLIFF documents  
//             belong does not exist. 
//             The XLIFF folder import can only import into
//             existing OpenTM2 folders.\n
//             The import is terminated.
// HelpTitle:  OpenTM2 - Help
// Help:       The XLIFF file contains the name of a folder 
//             which does not exist in OpenTM2. 
//             The XLIFF can only be imported into the original 
//             folder from which it has been exportet from.

#define WD_ASD_WRD_NT_FND                                  546
//* Define statement owner: XQG, LEE
// Text:       The term\n
//              '%1'\n
//             was not found.\n
//             Do you want to add the term to the dictionary?
// HelpTitle:  OpenTM2 - Help
// Help:       The specified term was not found in the dictionary.
//             When you choose to add the term to the dictionary, you
//             are prompted with the Dictionary Edit dialog.
//             If you do not want to add this term, you can
//             continue with the current function.

#define WD_ASD_BEG_NT_FND                                  547
//* Define statement owner: XQG, TAF, LEE
//* applies to substring search
// Text:       No terms were found.
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary contains no terms that begin with
//             the specified string. Select another string
//             and try again.

#define ERROR_DLUP_NO_FILTER_MATCH                         548
//* Define statement owner: XQG, NEW
// Text:       Term not found due to filter condition.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified term is contained in the
//             dictionary but the active filter
//             prevents a match.
//             :p.
//             Deactivate the filter or change the
//             filter condition to process the selected
//             term.

#define ERROR_DLUP_LOAD_FILTER                             549
//* Define statement owner: XQG
// Text:       Filter '%1' could not be activated.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified filter could not be activated.
//             Check the filter using the filter edit
//             pushbutton in the filter group.

#define WD_DELETE_ENTRY                                    550
//* Define statement owner: XQG, TAF, LEE
// Text:       Do you want to remove the complete entry?
// HelpTitle:  OpenTM2 - Help
// Help:       This entry will be removed completely from the
//             specified dictionary. Ensure that none of the homonym's
//             usages will be needed any further.

#define WD_SAVE_ON_CLOSE                                   551
//* Define statement owner: JOR, LEE
// Text:       You have made changes to the dictionary entry.\n
//             Do you want to save them?
// HelpTitle:  OpenTM2 - Help
// Help:       If you save the changes the entry is updated in
//             the dictionary.

#define XLIFF_NO_FOLDER_NAME                              552
//* Define statement owner: .............
// Text:       There is no folder information contained in 
//             the XLIFF file. Import cannot continue.
// HelpTitle:  OpenTM2 - Help
// Help:       The XLIFF file does not contain any information
//             about the OpenTM2 folder it has been exported
//             from. Either the  XLIFF file has  not been exported
//             by OpenTM2 or the editor used for the 
//             translation has removed the folder information.

#define XLIFF_DIFF_FOLDER                                553
//* Define statement owner: .............
// Text:       XLIFF folders can only be imported into the folder
//             from which the folder was exported from. The
//             XLIFF folder was exported from folder %1 and 
//             cannot be imported into folder %2. 
// HelpTitle:  OpenTM2 - Help
// Help:       XLIFF folders can only be imported into the folder
//             from which the folder was exported from. Ensure
//             that the correct folder is selected before you
//             continue the folder import.

#define WD_NO_SELECTION                                    554
//* Define statement owner: JOR, LEE
// Text:       There is no text selected for copying.
// HelpTitle:  OpenTM2 - Help
// Help:       Make a selection with the keyboard or mouse,
//             and try again.

#define WD_CLIPBOARD_EMPTY                                 555
//* Define statement owner: JOR, LEE
// Text:       There is nothing to paste.
// HelpTitle:  OpenTM2 - Help
// Help:       First select something to be copied, and then try again.

#define WD_NO_EF_SPECIFIED                                 556
//* Define statement owner: JOR, LEE
// Text:       There is no entry field selected for pasting or copying.
// HelpTitle:  OpenTM2 - Help
// Help:       First select one of the entry fields, and then try again.

#define WD_PASTE_NO_SUCCESS                                557
//* Define statement owner: JOR, LEE
// Text:       The paste operation was not successful.
// HelpTitle:  OpenTM2 - Help
// Help:       Pasting has caused an overflow of the text limit
//             set for the entry fields. A maximum of 128 bytes are
//             allowed for single-line entry fields, and a maximum
//             of 512 bytes for multi-line entry fields.

#define WD_STRUCT_DIFFERENT                                558
//* Define statement owner: JOR, LEE
// Text:       The target dictionary could contain different fields.
//             Data could be lost in the target dictionary.\n
//             Do you want to save it anyway?
// HelpTitle:  OpenTM2 - Help
// Help:       The target dictionary you want to save this entry to
//             could contain different fields. Compare the fields of the
//             dictionary you looked the word up in with the fields of
//             the dictionary you want to save the entry to, and decide
//             whether you still want to save it. Fields not contained
//             in the target dictionary
//             are ignored, and additional fields
//             are filled with blanks.

#define ERROR_MEM_EXPORT_DATACORRUPT                       559
//* Define statement owner: XQG
// Text:       Not all characters contained in the memory can be exported
//             correctly in the %1 format and will be corrupted.\n
//             \n
//             Do you want to continue the export?
// HelpTitle:  OpenTM2 - Help
// Help:       Some characters in the memory can not be correctly converted
//             into the choosen export format. When the export is continued, these
//             characters will be corrupted.
//             To avoid this corruption stop the export and choose
//             SGML (Unicode) as export format or selecte the correct
//             default target language in the Systen Preferences.

#define ERROR_TA_EMPTYFILE                                560
//* Define statement owner: XQG
// Text:       The document '%1' has a file size of zero bytes
//             and cannot be analyzed.\n
//             Do you want to continue with text analysis?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       The document has a length of zero bytes and cannot
//             be analyzed.
//             .br
//             Please check the document source and re-import the
//             document to solve this problem.
//             .br
//             If you continue, the document where the error occurred
//             is skipped and the next document (if there is one)
//             is analyzed. Leaving this function stops the analysis
//             process.

#define ERROR_TA_PROFILE_LOAD                             561
//* Define statement owner: XQG
// Text:       The analyis profile %1 does not exist or cannot be loaded.\n
//             Please select an existing profile or restore the
//             missing analysis profile.
// HelpTitle:  OpenTM2 - Help
// Help:       The referenced analysis profile cannot be loaded.
//             Either the profile name is incorrect or the profile
//             has been damaged.

#define ERROR_TA_PROFILE_INVNAME                           562
//* Define statement owner: XQG
// Text:       The specified name for the profile is not valid.\n
//             A valid profile name has up to 40 characters and 
//             uses the characters "1" to "9", "A" to "Z", 
//             "_", and "-" only. 
// HelpTitle:  OpenTM2 - Help
// Help:       Correct the profile name and try again.

#define ERROR_TA_UNDEFINEDERR                              563
//* Define statement owner: LEE
// Text:       The analysis of document '%1' failed. 
// HelpTitle:  OpenTM2 - Help
// Help:       An error occurred while the text was being analyzed.
//             Possible causes for the error
//             include:
//             :ul.
//             :li.You specified the wrong markup table or
//             the wrong language support file.
//             :li.Either or both of these files contain errors. If
//             it is a self-created markup table is erroneous, import
//             a backup copy, or create the table again.
//             Check the contents of the SGML copy of
//             the table.
//             :li.The document contains characters which cannot 
//             be processed using the selected markup able.
//             :li.If the table or support file was supplied by
//             OpenTM2, reinstall OpenTM2.
//             If the error continues, call your IBM
//             representative.
//             :li.The file you selected
//             for analysis might not be a text file and, therefore, not
//             intended for translation (for example, it is an executable
//             file).
//             :eul.

#define QUERY_HISTLOG_DELETE                               564
//* Define statement owner: .............
// Text:       The history log file contains vital information for the
//             creation of calculation reports. When you delete the file
//             all translation efforts stored in the history log will be
//             lost\n
//             Are you sure that you want to delete the history log file?
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_NO_USER_ID                                  565
//* Define statement owner: .............
// Text:       Enter your user ID or your name
// HelpTitle:  OpenTM2 - Help
// Help:       For the confirmation of the delete the user ID or
//             the user name is necessary

#define ERROR_NO_CONFIRM                                 566
//* Define statement owner: .............
// Text:       Enter the confirmation text characters.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to confirm the delete of the history log
//             you have to enter the 6 confirmation text characters. 

#define ERROR_CONFIRM_MISMATCH                           567
//* Define statement owner: .............
// Text:       The confirmation text is not correct
// HelpTitle:  OpenTM2 - Help
// Help:       Enter the 6 characters of the confirmation text exactly
//             as shown. Do not enter blanks and ensure that the characters
//             are in lowercase of uppercase as shown. 

#define INFO_HISTLOG_DELETED                             568
//* Define statement owner: .............
// Text:       The historylog file of the folder has been deleted.
// HelpTitle:  OpenTM2 - Help
// Help:       The history log file has been deleted. The folder is now
//             read for new translation projects.

#define ERROR_TAGIMP_DATATOOBIG                            569
//* Define statement owner: LUE DKS
// Text:       The markup table you are trying to import is too big.
// HelpTitle:  OpenTM2 - Help
// Help:       The markup table limit is about 2000 tags and attributes.
//             Try to shorten your list of markup tags.


#define WARNING_DELETE_PROFILE                            570
//* Define statement owner: XQG
// Text:       Do you really want to delete profile '%1'?
// HelpTitle:  OpenTM2 - Help
// Help:       Only delete the profile when it is not required
//             anymore.

#define ERROR_FS_DOC_NOT_ANALYZED                          571
//* Define statement owner: XQG, NEW
// Text:       The document %1 cannot be used for fuzzy
//             segment search because it has not been analyzed yet.
//             Continue with next document?
// HelpTitle:  OpenTM2 - Help
// Help:       Before a document can be used for fuzzy segment
//             search operations, it needs to be
//             analyzed (select "Analyze..." from the "File" pull-down).

#define INFO_FS_DOC_NO_HIT                                572
//* Define statement owner: XQG, NEW
// Text:       Search complete. No segments with fuzzy matches
//             matching the selected class(es) were found.
// HelpTitle:  OpenTM2 - Help
// Help:       There were no fuzzy matches with the selected 
//             number of different words.


#define ERROR_FS_DOCLOAD                                   573
//* Define statement owner: XQG, NEW
// Text:       The document "%1" could not be accessed by the
//             fuzzy segment search.\n
//             Do you want to continue with the next document?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       An error occurred while processing one of the documents.
//             The specified document might not exist or could be
//             in use by another process.
//             .br
//             If you continue, the document where the error occurred
//             is skipped and the next document (if there is one)
//             is processed.

#define INFO_FS_SEARCH_COMPLETE                           574
//* Define statement owner: XQG, NEW
// Text:       Search complete. %1 segments with fuzzy matches
//             matching the selected class(es) have been found.
// HelpTitle:  OpenTM2 - Help



#define EQFQUOT_QUOTE_MISMATCH                             575
// Text:       The number of quotes in the segment has been changed.\n
//             As quotes are used in this document format as delimiter
//             for translatable text, this might corrupt the document
//             format.\n
//             Do you want to save the segment anyway?
// HelpTitle:  OpenTM2 - Help
// Help:       The number of quotes has been changed.
//             :p.
//             Use the source of the segment to determine the
//             correct number and position of the quotes
//             and correct the translation accordingly.

#define ERROR_FILE_OPEN_FAILED                              576
//* Define statement owner: LEE
// Text:       Open of file '%1' failed.
// HelpTitle:  OpenTM2 - Help
// Help:       The referenced file could not be accessed, either it
//             does not exist or it is locked by another program.
//             \n.
//             Check the file name and verfiy that no other program
//             is currently accessing this file.

#define ERROR_OPEN_TM_PROPERTIES                           577
//* Define statement owner: DOE, NEW
// Text:       The properties of the Translation Memory '%1' could not be
//             opened.
// HelpTitle:  OpenTM2 - Help
// Help:       An internal control file could not be opened.
//             Either the file does not exist or it is damaged.
//             Restart OpenTM2
//             and the OpenTM2 Server (if installed).
//             If the problem continues, delete the Translation Memory
//             and import a backup copy.
//             If the problem continues,
//             contact your IBM representative for further help.
// Cause:      Open of property file for Translation Memory failed
// Type:       EQF_ERROR
// Severity:   Cancel
// Add.Info:   '%1' = TM name

#define QUERY_DO_RESTORE                                  578
// Text:       Do you really want to restore all previously 
//             removed documents?
// HelpTitle:  OpenTM2 - Help
// Help:       When you continue all documents which were 
//             removed from the folder will be restored.

#define ERROR_TM_FILE_NOT_FOUND                            579
//* Define statement owner: DOE, NEW
// Text:       The Translation Memory '%1' could not be found.
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory was either deleted by an operating
//             command outside the OpenTM2 environment,
//             or it was created as a shared Translation Memory and then
//             deleted by its owner. It can no longer be accessed.
//             'Delete' on the 'File' pull-down will remove it from the
//             Translation Memory List window.
// Type:       EQF_ERROR
// Severity:   Cancel
// Add.Info:   '%1' = TM name

#define INFO_RESTORE_COMPLETE                             580
// Text:       Restore complete. %1 documents have been restored.
// HelpTitle:  OpenTM2 - Help
// Help:       The restore function completed successfully.

#define INFO_REMOVE_COMPLETE                             581
// Text:       Remove complete. %1 documents have been removed.
// HelpTitle:  OpenTM2 - Help
// Help:       The removed function completed successfully.
//             You can restore the document using the restore function.

#define ERROR_TM_OPENED_EXCLUSIVELY                        582
#define ERROR_TM_OPENED_SHARED                             582
//* Define statement owner: MHO, TAF, LEE
// Text:       Translation Memory '%1' is in use by another process.
// HelpTitle:  OpenTM2 - Help
// Help:       You wanted to work with a Translation Memory that
//             is in use by another process.
//             Wait until this process has been completed, and try again.

#define WARNING_DELETE_REM_INC_MEMORY                      583
//* Define statement owner: DOE, NEW
// Text:       You are not the owner of the shared Translation Memory '%1'
//             and so are not authorized to physically delete it. Only
//             access to the Translation Memory will be undermined.
//             Do you want to undermine the access and remove it
//             from your Translation Memory list window?
// HelpTitle:  OpenTM2 - Help
// Help:       A shared Translation Memory situated on the network may only
//             be deleted by its owner.
//             Deleting it as a non-owner merely implies
//             disallowing access and removing the Translation Memory
//             from the Translation Memory list window. It may me connected
//             again later on.

#define WARNING_DEL_OWN_REM_TM                             584
//* Define statement owner: DKS, LEE
// Text:       As the LAN administrator or Translation Memory owner
//             you are authorized to delete the shared Translation
//             Memory '%1'.
//             Are you sure that you want to delete it?
// HelpTitle:  OpenTM2 - Help
// Help:       Delete the Translation Memory if you are sure that no
//             one needs it any more. Remember that the Translation
//             Memory is shared and other users have access to it.
//             .br
//             Hint:
//             .br
//             You can export the Translation Memory before deleting
//             it to save its data.

#define ERROR_PROP_EXIST                                   585
//* Define statement owner: MHO, LEE
// Text:       Translation Memory '%1' could not be created.
// HelpTitle:  OpenTM2 - Help
// Help:       Try to create a new Translation Memory with another name.
//             If you need further help, please contact your IBM
//             representative.
//*
// Cause:      Property file for Translation Memory '%1'
// Cause:      already exists.

#define ERROR_PROP_WRITE                                   586
//* Define statement owner: MHO, LEE
// Text:       Translation Memory '%1' could not be created.
// HelpTitle:  OpenTM2 - Help
// Help:       Try to create a new Translation Memory with another name.
//             If you need further help, please contact your IBM
//             representative.

#define ERROR_CANCELEXPORTSEGS                              587
//* Define statement owner: XQG NEW
// Text:       Do you really want to cancel the eport of segments?
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       When you cancel the export segments 
//             process the output file may contain incomplete data.


#define ERROR_EXPORTSEGS_NOSEGSFOUND                       588
//* Define statement owner: XQG NEW
// Text:       Exporting segments 
//             terminated. No segments were located inside
//             the specified tag groups.
// HelpTitle:  OpenTM2 - Help
// Help:       The export segments process has been
//             completed but there no segments located
//             inside the specified tag groups.

#define INFO_EXPORTSEGS_COMPLETED                          589
//* Define statement owner: XQG NEW
// Text:       Export of segments to file  '%1'
//             has been completed successfully. %2 segments have 
//             written to the file.
// HelpTitle:  OpenTM2 - Help
// Help:       The segments inside the specified tag groups have
//             been exported to the output file.

//* -------------------------------------------------------
//*     Messages for WordCount
//*
//* -------------------------------------------------------

#define ERROR_WODCA                                        590
//* Define statement owner: DOE, TAF, LEE
// Text:       The document '%1' could not be counted.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       An error occurred while counting the specified document.
//             You can skip this document, and continue counting
//             the rest of the
//             selected documents. Or you can leave this
//             counting function.

#define ERROR_WODCA_DISK_FULL                              591
//* Define statement owner: DOE, LEE
// Text:       The disk '%1' is full.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       The disk to which the word count result file should be
//             written is full. If you continue with counting, the
//             counting results are not written to the output file on
//             the disk, but they are still displayed on the screen.

#define ERROR_WODCA_WRITE                                  592
//* Define statement owner: DOE, LEE
// Text:       The word count results could not be written to file '%1'.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       The disk to which the word count result file should be
//             written is full. If you continue with counting, the
//             counting results are not written to the output file on
//             the disk, but they are still displayed on the screen.

#define ERROR_WODCA_FILE_NOT_ANALYZED                      593
//* Define statement owner: DOE, NEW
// Text:       The document file '%1' cannot be counted,
//             because it has not been analyzed.\n
//             Do you want to continue with the next document file?
// HelpTitle:  OpenTM2 - Help
// Help:       The selected document file has not been analyzed, and so the
//             words of this document file cannot
//             be counted. Either analyze the document before
//             starting Word Count again,
//             or skip this document and continue counting
//             with the next one.
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      Document is not analyzed. All documents to be counted have to
// Cause:      analyzed also when only the Original is counted.
// Add.Info:   '%1' = document name

#define ERROR_WODCA_NO_DOCTYPE_SELECTED                    594
//* Define statement owner: DOE, LEE
// Text:       No document type is selected.
// HelpTitle:  OpenTM2 - Help
// Help:       To specify the type of the document to be counted, select
//             one of the types displayed in the Document type list box.

#define WODCA_LING_SUPPORT                                 595
//* Define statement owner: XJR, NEW
// Text:       The linguistic support for the
//             language '%1' could not be activated.
// HelpTitle:  OpenTM2 - Help
// Help:       The linguistic support file for the source
//             or target
//             language as specified in the Translation Memory
//             is not accessible. Therefore the file could
//             not be counted using the correct language support.
//             Please check if you the appropriate language file
//             is installed and if you have sufficient memory
//             free.
// Type:       MB_CANCEL
// Add.Info:   Target Language

#define ERROR_WODCA_DOC_IN_USE                             596
//* Define statement owner: DOE, NEW
// Text:       The document '%1' is in use
//             by another process. It will be ignored in Word Count.\n
//             Do you want to continue anyway?
// HelpTitle:  OpenTM2 - Help
// Help:       The document is in use by another process.
//             You can either skip
//             this document, or stop Word Count and wait until the
//             running processes are finished before you try again.
// Type:       MB_YESNO
// Cause:      document is in use and can't be counted
// Add.Info:   '%1' = name of document

#define MESSAGE_RPT_NO_HISTLOG_AVAIL                       597
//* Define statement owner: SEK, NEW
// Text:       There is no history data available
//             for the folder '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       For the selected folder there was no history data found.
//             History data  for the reporting is collected starting
//             from the first document import action on.

#define MESSAGE_RPT_EMPTY_FOLDER                           598
//* Define statement owner: SEK, NEW
// Text:       The folder '%1' is empty.\n
//             The selected report could no be created.
// HelpTitle:  OpenTM2 - Help
// Help:       For the selected report at least one document must be
//             in the folder. Create this report again after you imported
//             one or more documents.

#define ERROR_EXPORTSEGS_TERMINATED                         599
//* Define statement owner: XQG NEW
// Text:       Exporting of segments to file '%1'
//             terminated abnormally.
// HelpTitle:  OpenTM2 - Help
// Help:       The segment export process failed
//             due to an error reported in previous messages.

//* -------------------------------------------------------
//*     Translation processor messages
//* -------------------------------------------------------

#define TB_CHANGESEGMENT                                   600
//* Define statement owner: XJR, LEE
// Text:       You have changed the active segment.\n
//             Do you want to save it?
// HelpTitle:  OpenTM2 - Help
// Help:       You have changed the active segment and left it without
//             saving. You can either save the changes in the Translation
//             Memory, discard them, or return to the segment.
//*
// Type:       EQF_QUERY
// Severity:   YESNOCANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define TB_NOTINACTSEG                                     601
//* Define statement owner: XJR, LEE
// Text:       The requested command is only allowed in the active segment.
// HelpTitle:  OpenTM2 - Help
// Help:       You left the active segment. Return to the active
//             segment by placing the cursor there, and try again.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define TB_NOSEGACT                                        602
//* Define statement owner: XJR, LEE
// Text:       No segment is activated.
// HelpTitle:  OpenTM2 - Help
// Help:       You have not activated a segment. Make a segment active
//             and try again.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define TB_DOCXLATED                                       603
//* Define statement owner: XJR, LEE
// Text:       The translation for document '%1' is completed.
// HelpTitle:  OpenTM2 - Help
// Help:       The document does not contain any more segments to be
//             translated. It is also
//             marked in the document list as translated.
// Type:       EQF_INFO
// Severity:   OK
// Cause:      see error message
// Add.Info:   '%1'  - document name

#define TB_NOVALIDABBR                                     604
//* Define statement owner: XJR, LEE
// Text:       You did not specify an abbreviation for
//             shrink or compact style.\n
//             Do you really want to set it?
// HelpTitle:  OpenTM2 - Help
// Help:       The strings for the style abbreviations
//             are empty. Enter one non-blank
//             character.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   none

#define TB_AUTOSTOP                                        605
//* Define statement owner: XJR, LEE
// Text:       There is no or more than one exact translation available
//             for this segment.
//             Automatic substitution is stopped.
// HelpTitle:  OpenTM2 - Help
// Help:       If there is a fuzzy match available, you might want to
//             continue with it, and change it appropriately.
//             .br
//             If there is more than one exact match available, choose
//             the correct one.
//             .br
//             Afterwards you can continue with the
//             automatic substitution of segments that are already translated.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No parameter

#define TB_SEGEMPTY                                        606
//* Define statement owner: XJR, TAF, LEE
// Text:       The translated segment is empty.\n
//             Do you want to save it anyway?
// HelpTitle:  OpenTM2 - Help
// Help:       You can save the empty segment in the Translation Memory.
//             That way it is not lost, and you can return to it later
//             to fill it in.
//*
// Type:       EQF_WARNING
// Severity:   YES NO CANCEL
// Cause:      see error message
// Add.Info:   No parameter

#define TB_NOVALIDFOCUS                                    607
//* Define statement owner: XJR
// Text:       Your line number for the active segment is
//             not in the valid range.
//             Only numbers between 1 and 20 are allowed.
// HelpTitle:  Set active segment line number
// Help:       Your line number is not valid.
//             Try a number in the valid range.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_ADDABBREVIATION                                 608
//* Define statement owner: XJR, NEW
// Text:       Do you really want to add '%1' to your
//             user abbreviation list?
// HelpTitle:  OpenTM2 - Help
// Help:       You selected to add an abbreviation to
//             your user abbreviation list.
//             Continue if you are sure to add this abbreviation.
//             Please use the display Abbreviation list to modify
//             (edit/delete/update) any already stored list.
//             :NOTE.Please analyse/align the file again to activate
//             this change in your segmented file.
// Type:       MB_YESNO

#define TB_ACTNEWTENV                                      609
//* Define statement owner: XJR
// Text:       The selected translation environment
//             could not be activated.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected translation environment
//             could not be activated.
//             A reason might be that the document is
//             in use or that the translation memory
//             or dictionary cannot be accessed.
//             Please reopen the specific document again.

#define TB_NOMATCH                                         610
//* Define statement owner: XJR, LEE
// Text:       No match was found.
// HelpTitle:  OpenTM2 - Help
// Help:       There is no further match available that corresponds
//             to the search criteria.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No parameter

#define TB_TOOLONG                                         611
//* Define statement owner: XJR, LEE
// Text:       The line is too long.
// HelpTitle:  OpenTM2 - Help
// Help:       The maximum size of a line or sentence is
//             2047 bytes. Since your line is too long, the request
//             is ignored. Shorten your line, and try again.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No parameter

#define TB_QUIT                                            612
//* Define statement owner: XJR, TAF, LEE
// Text:       Do you really want to close the Translation Environment?
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Environment will be closed. The currently
//             active document file is saved.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : document name

#define TB_NOSEGCLEAR                                      613
//* Define statement owner: XJR, LEE
// Text:       The active segment has no bookmark.
// HelpTitle:  OpenTM2 - Help
// Help:       You wanted to reset a bookmark, but the active
//             segment has no bookmark set. The request is ignored.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_NOSEGMARK                                       614
//* Define statement owner: XJR, LEE
// Text:       No bookmark is available in the current document.
// HelpTitle:  OpenTM2 - Help
// Help:       You searched for a bookmark but the active document file
//             has no bookmark set.
//             .br
//             The request is ignored.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_NOSEGJOINED                                     615
//* Define statement owner: XJR, LEE
// Text:       You cannot split a segment unless it has been joined
//             before.
// HelpTitle:  OpenTM2 - Help
// Help:       You can only split an active segment
//             if it has been joined before.
//             .br
//             The request is ignored.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_JOINEDSEG                                       616
//* Define statement owner: XJR, LEE
// Text:       The active segment will be split as follows:\n
//             Segment 1:\n
//                %1\n
//             Segment 2:\n
//                %2\n
//             Do you really want to split it?
// HelpTitle:  OpenTM2 - Help
// Help:       You can accept the splitting of the active segment as
//             indicated, or continue working.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : segment 1
// Add.Info:   '%2' : segment 2

#define TB_JOINSEG                                         617
//* Define statement owner: XJR, LEE
// Text:       The following two segments will be joined:\n
//             Segment 1:\n
//                %1\n
//             Segment 2:\n
//                %2\n
//             Do you really want to join them?
// HelpTitle:  OpenTM2 - Help
// Help:       You requested to join two segments.
//             Combining the two segments deletes previous
//             translations of the original segments in the Translation
//             Memory.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : segment 1
// Add.Info:   '%2' : segment 2

#define TB_JOINEDSEGTOOLONG                                618
//* Define statement owner: XJR, LEE
// Text:       The segments are not joined because the joined segment would
//             be too long.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       The maximum size of a line or sentence is
//             2047 bytes. After joining the segments, your line will be
//             too long. The request is ignored.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   NONE

#define TB_DICTENTRYTOOLONG                                619
//* Define statement owner: XJR, TAF, LEE
// Text:       The selected word or the marked area is too long.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       Either you marked a block that is too long or
//             the word the cursor is positioned on
//             is too long. The length is limited to 255 characters.
//             .br
//             Your request is ignored.
//             Mark a shorter block, and try again.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_DICTENTRYEMPTY                                  620
//* Define statement owner: XJR, LEE
// Text:       There is no mark available, or the cursor is not
//             positioned on a word or term.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       Mark a term, or position the cursor on a word,
//             and try again.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_FUZZYNOTCHANGED                                 621
//* Define statement owner: XJR, TAF, LEE
// Text:       You copied a fuzzy match proposal that might not match exactly.\n
//             Are you sure that the following is the correct
//             translation?:\n
//                '%1'
//*
// HelpTitle:  OpenTM2 - Help
// Help:       You copied a proposal marked with [f] , that is,
//             an already existing translation that was used for a
//             similar but not identical original. 
//             Ensure that the translation is
//             appropriate for this segment, and, if necessary, correct it.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : segment

#define TB_MARKSET                                         622
//* Define statement owner: XJR, LEE
// Text:       There is already a marked block in another document.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       You tried to mark a block but there is already
//             another marked block.
//             Unmark the first block, and try again.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_MARKOUTSIDESEG                                  623
//* Define statement owner: XJR, LEE
// Text:       There is already a marked block in another segment.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       Another marked block already exists outside the current
//             segment. You are not allowed to mark blocks over segment
//             boundaries. First unmark the other block, and then try again.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_KEY_RESERVED_FOR_OS2                            624
//* Define statement owner: XJR, LEE
// Text:       The key '%1' is reserved for the operating system and
//             cannot be assigned to an editor function.\n
//             Do you want to assign another key to the selected function?
//*
// HelpTitle:  OpenTM2 - Help
// Help:       The key you tried to assign is reserved
//             for an operating system function. It cannot be assigned to
//             any editor function. Try another key.
//*
// Type:       EQF_WARNING
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : name of key

#define TB_KEY_RESERVED_FOR_TP                             625
//* Define statement owner: XJR, LEE
// Text:       The key '%1' is reserved for the copy of dictionary or
//             Translation Memory proposals and cannot be
//             assigned to any other editor function.\n
//             Do you want to assign another key to the selected function?
//*
// HelpTitle:  OpenTM2 - Help
// Help:       The key you tried to assign is reserved for copying
//             Translation Memory proposals and dictionary
//             terms. It cannot be assigned to any other editor function.
//             Try another key.
//*
// Type:       EQF_WARNING
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : name of key

#define TB_KEY_IN_USE                                      626
//* Define statement owner: XJR, LEE
// Text:       The key '%1' is already in use for function '%2'.\n
//             Do you want to assign the key to the new function?
//*
// HelpTitle:  OpenTM2 - Help
// Help:       The key you tried to assign is already assigned to another
//             function. You can assign the key to
//             the new function. The old function will lose the key
//             assignment. If you want to keep your original settings,
//             try another key.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : name of key
// Add.Info:   '%2' : name of function

#define TB_KEY_IS_CHARACTER                                627
//* Define statement owner: XJR, LEE
// Text:       The key '%1' is a character key and should not be
//             assigned to an editor function.\n
//             Do you want to assign another key to the selected function?
//*
// HelpTitle:  OpenTM2 - Help
// Help:       The key you tried to assign is a character key.
//             Character keys should be used to type textual information,
//             but should not be used for editor functions.
//             Try another key.
//*
// Type:       EQF_WARNING
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : name of key

#define TB_NOMARKACTIVE                                    628
//* Define statement owner: XJR, LEE
// Text:       There is no mark active in the current document.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       You tried to unmark or to cut and paste an area,
//             but no mark is active in the current document.
//             Mark an area first.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_NOCLIPBOARDDATA                                 629
//* Define statement owner: XJR, TAF, LEE
// Text:       There is no data available to be pasted.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       You tried to paste an area,
//             but there is no data available to be pasted.
//             First select something to be pasted, and then try again.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_MARKMOVENOTVALID                                630
//* Define statement owner: XJR, TAF, LEE
// Text:       Moving a marked area is only valid within the current segment.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       You tried to move a marked area from outside the current
//             segment into the active segment.
//             Moving marked areas only works within the active
//             segment.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_KEY_ACCELERATOR                                 631
//* Define statement owner: XJR, LEE
// Text:       The key '%1' is already assigned to an accelerator key
//             and is therefore ignored.\n
//             Do you want to assign another key to the selected function?
//*
// HelpTitle:  OpenTM2 - Help
// Help:       You pressed a key that is used as an accelerator key within
//             OpenTM2. You can either assign a different key,
//             or cancel your request.
//*
// Type:       EQF_WARNING
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1'   key name

#define TB_MARKSEG                                         632
//* Define statement owner: XJR, LEE
// Text:       The segment\n
//               '%1'\n
//             is marked.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       The specified segment is marked, and you can continue working.
//*
// Type:       EQF_INFO
// Severity:   OK
// Cause:      see error message
// Add.Info:   No parameter

#define TB_CLEARMARK                                       633
//* Define statement owner: XJR, LEE
// Text:       The segment\n
//               '%1'\n
//             is no longer marked.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       The specified segment has been unmarked, and you can continue
//             working.
//*
// Type:       EQF_INFO
// Severity:   OK
// Cause:      see error message
// Add.Info:   '%1' : segment

#define TB_NOVALIDMARGIN                                   634
//* Define statement owner: XJR, LEE
// Text:       The specified margin %1 is either not a valid number or
//             it is outside of the allowed range (10 to 950).
//*
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that the number for margin is
//             within the allowed range (10 to 950).
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1' : specified right margin

#define TB_ENDSPELLCHECK                                   635
//* Define statement owner: XJR, LEE
// Text:       No more misspelled words were found.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       No further misspelled words were detected
//             in the given area, and you can continue working.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message

#define TB_ERRSPELLCHECK                                   636
//* Define statement owner: XJR, LEE
// Text:       No spell checking is possible.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       Check whether the target language is specified correctly.
//             Activate the Translation Memory List window, and
//             select 'Details' from the 'View' pull-down. If the
//             Translation Memory does not contain the right target
//             language you should create it again, specifying the right
//             target language. Then merge the
//             old Translation Memory into the new one.
//             .br
//             You can also check whether the appropriate language support
//             file for the specified target language is active (select
//             'Display language list...' from the 'Utilities' pull-down).
//             If it is not active, reinstall it.
//             .br
//             Furthermore, you might not have enough space available to run
//             this spell check function. Close some applications you do not
//             need at the moment, and try again.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message

#define TB_IGNOREERRSPELLCHECK                             637
//* Define statement owner: XJR, LEE
// Text:       The temp add List is full.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       Since the list of words to be ignored is full,
//             no other words can be added to this list.
//             You can continue using the words
//             already contained in the list.
//             This temporary list will be cleared when you enter the
//             Translation Environment the next time.
//*
// Type:       EQF_INFO
// Severity:   OK
// Cause:      see error message

#define TB_DELPROTECT                                      638
//* Define statement owner: XJR, LEE
// Text:       Do you want to delete the protected characters, too?
//*
// HelpTitle:  OpenTM2 - Help
// Help:       The characters to be deleted contain protected
//             information, for example, tags and variables.
//             Ensure that the tagging
//             information is still correct after deletion.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message

#define TB_SEGDELETE                                       639
//* Define statement owner: XJR, LEE
// Text:       Do you really want to delete the segment '%1'?
// HelpTitle:  OpenTM2 - Help
//*
// Help:       The complete segment will be deleted, including all protected
//             characters in the segment.
//             .br
//             In order to guarantee positioning at this segment
//             a &colon.NONE. tag will be inserted, if you delete this
//             segment.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message

#define TB_EOFSTILLUNTRANS                                 640
//* Define statement owner: XJR, TAF, LEE
// Text:       The last segment of the file has been translated, but there are
//             still untranslated segments in the translation. You can leave
//             the translation now or continue with the next
//             untranslated segment.\n
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       You have reached the end of your document file, but
//             its translation has not yet been completed.
//             You can continue
//             translating the remaining untranslated segments, or
//             leave the translation.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message

#define TB_SOURCEUNCHANGED                                 641
//* Define statement owner: XJR, LEE
// Text:       The original segment has not been changed.\n
//             Do you want to save it?
// HelpTitle:  OpenTM2 - Help
// Help:       Since you have not translated anything, you are in
//             the process of saving the original as translation.
//             Ensure that this is the correct translation
//             for your original document.
//*
// Type:       EQF_QUERY
// Severity:   YESNOCANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define TB_NOFURTHERMATCH                                  642
//* Define statement owner: XJR, LEE
// Text:       No further match was found.
// HelpTitle:  OpenTM2 - Help
// Help:       There is no further match available in
//             the specified range.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No parameter

#define TB_NOSEGSCRATCH                                    643
//* Define statement owner: XJR, LEE
// Text:       No segment was found in the current document that
//             fulfills the requested criteria.
//*
// HelpTitle:  OpenTM2 - Help
// Help:       You asked to find a segment that
//             fulfills one of the following criteria:
//             :ul.
//             :li.Translated from scratch
//             :li.Translation Memory proposal copied, but modified
//             :li.Translation Memory proposal copied without changes
//             :eul.
//             There is no such segment in the
//             current document.
//             .br
//             The request is ignored.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   none

#define TB_TAGCHANGED                                      644
//* Define statement owner: XJR, LEE
// Text:       You have changed the number or contents of the tags.\n
//             Do you want to save the segment?
// HelpTitle:  OpenTM2 - Help
// Help:       You have changed the number or the contents of the tags.
//             Ensure that the changes will still provide correct layout
//             information.
//             .br
//             You can either save the changes in the Translation
//             Memory or return to the segment.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   No additional parameter

#define TB_NOVALIDLINENUM                                  645
//* Define statement owner: XJR, LEE
// Text:       The value %1 you specified is not a valid line number.
//             It is either too large, too small, or it contains non-numeric
//             characters.
// HelpTitle:  OpenTM2 - Help
// Help:       The line number should be a number between 1
//             and the last line of the file.
//             .br
//             Correct the number and try again.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1'  data converted to the line number

#define TB_LINEINFILE                                      646
//* Define statement owner: XJR, LEE
// Text:       Current line: %1 of %2
// HelpTitle:  OpenTM2 - Help
// Help:       The cursor is positioned at the specified line in the
//             file.
//*
// Type:       EQF_INFO
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1'  cursor line
// Add.Info:   '%2'  max. number of lines in the file

#define TB_INVABBREVIATION                                 647
//* Define statement owner: XQG, NEW
// Text:       The term '%1' is no valid abbreviation and could not
//             be added to the abbreviation list.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected term forms no valid abbreviation
//             for the active source language.
//             :p.The problem may be caused by invalid characters
//             in the term, terms consisting of more than one word,
//             or punctuation characters within the term.
// Type:       MB_CANCEL

#define TB_NOUPDATEDINFILE                                  648
//* Define statement owner: XJR
// Text:       No (further) updated segment has been found in the document.
// HelpTitle:  OpenTM2 - Help
// Help:       All translated segments in this file (or all except one)
//             has been
//             translated by automatic substitution during analysis.
// Type:       MB_OK

#define TB_LANG_NOT_SUPPORTED_BY_EDITOR                    649
//* Define statement owner: XJR
// Text:       The selected source or target language '%1' or special
//             editing features for this language are not
//             supported by the chosen editor. Please select
//             the appropriate editor.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected editor does not support some language specific
//             editing features.
//             For certain languages (e.g. Thai) it is required that the
//             editor supports TrueType fonts for supporting diacritics,
//             for others right-to-left writing order (Hebrew) is required.
//             Please change your editor in the document or folder properties.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1'  source or target language name

#define ERR_RTF_CANTDETECTPICSIZE                          650
//* Define statement owner: XTK
// Text:       TM can't detect the size of an embedded picture. Later export of the document will fail.
// HelpTitle:  OpenTM2 - Help
// Help:       Please contact technical support.

#define ERR_FILEOPERATION                                  651
//* Define statement owner: XTK
// Text:       File operation error. Projectfolder contents are corrupt.
// HelpTitle:  OpenTM2 - Help
// Help:       One or more files in the TM project folder are missing or corrupt. This may occur for example,
//             if your system trapped down while TM was processing files or if your filesystem is damaged due
//             to other reasons. In some cases reanalyzing the source document may help.

#define ERR_PPT_OLDFORMAT                                  652
//* Define statement owner: XTK
// Text:       This document was analyzed with an older version of the Powerpoint filter.
//             You must reanalyze, before exporting it.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define LANG_NOT_SUPPORTED_BY_MT                           653
//* Define statement owner: XQG
// Text:       The source or target language '%1' of document
//             '%2' is not supported by the machine
//             translation server.
// HelpTitle:  OpenTM2 - Help
// Help:       The source language or the target language of
//             the document is not supported by the machine
//             translation server. The document will be skipped.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1' source/target lang, %2 docname

#define TB_LFCHANGE_NOT_ALLOWED                            654
//* Define statement owner: XJR
// Text:       The selected action would change the number
//             of linefeeds in the segment.
//             As this is not allowed, the action is ignored.
// HelpTitle:  OpenTM2 - Help
// Help:       The markup table of the document
//             does not allow linefeeds to be changed.
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      CRLF within MRI documents should not be changed


#define TB_NOVALIDSEGNUM                                  655
//* Define statement owner: XQG
// Text:       The value %1 you specified is not a valid segment number.
//             It is either too large, too small, or it contains non-numeric
//             characters.
// HelpTitle:  OpenTM2 - Help
// Help:       The line number should be a number between 1
//             and the last segment number the file.
//             .br
//             Correct the number and try again.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   '%1'  data converted to the segment number

#define ERROR_IN_STARTSTOPFILE                             656
//* Define statement owner: XQG
// Text:       The load of start/stop list %1 failed.\n
//             The reason is\n
//             %2
// HelpTitle:  OpenTM2 - Help
// Help:       The file containing the list of
//             start/stop tags could not be loaded.

#define NO_VALID_STARTSTOP_TAGS                           657
//* Define statement owner: XQG
// Text:       The file %1 does not contain any valid 
//             start and stop tag definitions.
// HelpTitle:  OpenTM2 - Help
// Help:       The file containing the start and stop
//             tag definitions has to be encoded in
//             ASCII. Each text line of the file contains
//             a start tag and a stop tag separated by a 
//             comma. The tags may be enclosed in 
//             double-quotes.

#define MESSAGE_FOLIMP_COMPL_USEREXITINUSE                658
//* Define statement owner: XQG, LEE
// Text:       The import of folder '%1' to drive '%2' has been completed.\n
//             One or more of the markup tables could not be updated with newer versions
//             as the markup tables were in use.\n
//             Please restart OpenTM2
//             before working with the imported folder. On restart OpenTM2 will
//             update the markup tables automatically.
// HelpTitle:  OpenTM2 - Help
// Help:       Either a stop or close request was issued, or another
//             abnormal situation occurred during the import process.
//             Correct the condition that caused the error and retry
//             the folder import function.
// Cause:      User Cancel or previously reported error condition.
// Add.Info:   '%1' = folder name, '%2' = target drive

#define TB_MACHINENOTCHANGED                               659
//* Define statement owner: 
// Text:       You copied a machine match proposal and did not change it.\n
//             Are you sure that the following is the correct
//             translation?:\n
//                '%1'
//*
// HelpTitle:  OpenTM2 - Help
// Help:       You copied a proposal marked with [m] , that is,
//             an automatically translated proposal.
//             Ensure that the translation is
//             appropriate for this segment, and, if necessary, correct it.
//*
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      see error message
// Add.Info:   '%1' : segment

//* -------------------------------------------------------
//*     Continuation of Messages for SERVER Operations
//*                see message number 589
//* -------------------------------------------------------

#define ERROR_MARKUPTABLEINFO                              660
//* Define statement owner: XQG
// Text:       The markup table information file %1 in the folder is corrupted.\n
// HelpTitle:  OpenTM2 - Help
// Help:       The markup table information file contained in the folder
//             is corrupted and the markup table contained in the folder
//             can't be imported.


#define ERROR_PLUGINREQUIRED                              661
//* Define statement owner: XQG
// Text:       No markup table plugin for markup table %1 found. 
//             Please install a markup table plugin supporting this markup table 
//             and repeat the folder import.
// HelpTitle:  OpenTM2 - Help
// Help:       The referenced markup table plugin is required for the import of the markup
//             tables contained in the folder.

#define ERROR_MEMORYINFO                                   662
//* Define statement owner: XQG
// Text:       The memory information file %1 in the folder is corrupted.\n
// HelpTitle:  OpenTM2 - Help
// Help:       The memory information file contained in the folder
//             is corrupted and the memory contained in the folder
//             can't be imported.


#define ERROR_MEMPLUGINREQUIRED                              663
//* Define statement owner: XQG
// Text:       For the import of the folder the memory plugin %1 is
//             required. Please install this plugin and repeat the folder
//             import.
// HelpTitle:  OpenTM2 - Help
// Help:       The referenced memory plugin is required for the import of the memory 
//             contained in the folder.

#define ERROR_FILE_EXISTS_ALREADY_BATCH                    664
// Text:       The file '%1' exists already and could not be replaced as the overwrite option had not been specified.
// HelpTitle:  OpenTM2 - Help
// Help:       You have asked to write to the specified file. However,
//             this file exists already.
//             Either replace the file, or exit without
//             replacing it.

#define ERROR_MEMORY_IMPORT_XLIFF                          665
// Text:       The memory cannot be imported because it is not an OpenTM2 generated XLIFF file for machine translation.
// HelpTitle:  OpenTM2 - Help
// Help:       You are trying to import a file which was not
//             generated by OpenTM2 for machine translation 
//             processing.  The file does not contain the   
//             expected information to create memory entries.

#define MESSAGE_MEM_LOAD_COMPLETED3                         666
//* Define statement owner: DOE, TAF, LEE
// Text:       The import of file '%2' into Translation Memory '%1' was completed successfully.\n\n
//             %3 segments have been imported.\n 
//             %5 segments have been imported with markup table OTMUTF8.\n
//             %4 segments were not valid and not imported.\n
// HelpTitle:  OpenTM2 - Help
// Help:       Segments need to have valid syntax to be imported.
//             Segments that have a more recent equivalent in the
//             Translation Memory are also not imported.
//             Segments which use an unknown markup table were 
//             imported, but were assigned the generic OTMUTF8 markup table.

#define ERROR_DIMP_UTF8WRONG                               667
//* Define statement owner: XRJ
// Text:       The codepage specified in the imported file is UTF-8,
//             but the file is not a valid UTF-8 file.
//             Import process is canceled.
// HelpTitle:  OpenTM2 - Help
// Help:       The codepage specification in the import file may have been
//             changed. The BOM at the begin of the file may be missing.

#define VIETNAMESE_KEYBOARD_INFO                           668
// Text:       Use only the Microsoft Keyboard input method when translating into Vietnamese.
//             \nDo not use the UniKey input method.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_INVALID_BATCHLIST                           669
// Text:       The current batch find & replace list does not contain valid entries. Please
//             ensure that the list contains at least one search entry.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_DICT_PID_NL_PRODUCT                          670
// Text:       Dictionary entries cannot be selected by their PID value because none of the 
//             selected dictionaries define the "NL Product" field.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
#define INFO_DICTPID_SELECTED                              671
// Text:       The requested dictionary PID is already selected.
// HelpTitle:  OpenTM2 - Help
// Help:       The dictionary PID entries must be unique in the list
//             of selected dictionary PID values.
//             A previously selected
//             dictionary PID value cannot be selected again.
#define ERROR_DICTPID_MAXSELECTED                          672
// Text:       The allowed maximum length (400) of dictionary PID values
//             is already selected.
// HelpTitle:  OpenTM2 - Help
// Help:       The number of dictionary PID values that can be
//             associated with a folder is limited to
//             400 characters.

#define FUNC_ADDMATCHSEGID_NOIDSPECS                      673
//* Define statement owner: XQG
// Text:       Missing input specs for match segment ID creation
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_DIFFERENT_TARGET_LANG                       674
// Text:       The selected documents use different target languages ('%1' and '%2').
//             Please ensure that only documents having the same target language 
//             are selected before restarting the spellcheck function.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_NO_SPELL_SUPPORT_FOR_TARGET_LANG             675
// Text:       No spellcheck support for target language '%1' is available.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_INVALID_SHIPMENT_NUMBER                      676
//* Define statement owner: XQG
// Text:       The shipment number "%1" is invalid.
//             The shipment number is either a numeric value in the range from 1 to 99
//             to select a specific shipment
//             or one of the keywords "Single Shipments" or "All Shipments".
// HelpTitle:  OpenTM2 - Help
// Help:       The shipment number specified 
//             is not a valid shipment number. Valid shipment
//             numbers are numeric and in the range from 1 to 99 or 
//             one of the keywords "Single Shipments" or "All Shipments".

#define WARNING_DOCEXP_WITHTRACKID                         677
//* Define statement owner: DAW
// Text:       Exporting a document with TVT tracking IDs should only be done
//             when the document will be used to build the product for use
//             in a SmartTVT testing environment.  Extra zero-length Unicode
//             characters are added around the translatable text in the exported
//             file.\n
//             \nThis option should -ONLY- be used when you are instructed to use this option.\n
//             \nSelect "OK" to add tracking IDs to the exported file.
//             \nSelect "Cancel" to cancel exporting these documents.
// HelpTitle:  OpenTM2 - Help
// Help:       Exporting a document with TVT tracking IDs should only be done
//             when the document will be used to build the product for use
//             in a SmartTVT testing environment.  Extra zero-length Unicode
//             characters are added around the translatable text in the exported
//             file.
//             .br
//             This option should -ONLY- be used when you are instructed to use this option.

#define INFO_FS_EXPORT_COMPLETE                           678
//* Define statement owner: XQG, NEW
// Text:       The search results have been exported to file %1.
// HelpTitle:  OpenTM2 - Help

#define ERROR_FS_INVALID_CLASS                              679
// Text:       The specified search class %1 is not valid. Only the numbers from 0 to 6 are valid.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_FS_INVALID_MODE                               680
// Text:       The specified search mode %1 is not valid. 
//             \nValid modes are 
//             \n  0 (for up to and including seleced class)
//             \n  1 (for seleced class and higher)
//             \n  2 (for only selected class)
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_FS_MISSING_OUTPUT_NAME                       681
// Text:       No output file name has been specified.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_TA_MTOPTIONS                                682
// Text:       MT output options error:\n
//             %1
// HelpTitle:  OpenTM2 - Help
// Help:       There was an error in the specified MT output options. Please correct
//             the error and retry.

#define FUNC_INVALID_MEMORY_HANDLE  683
// Text:       The specified memory handle is not invalid.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define RESET_WORDCOUNT_INFO                  684
// Text:       The wordcount information in the folder history log has been reset.\n
//             \nRe-analyze the folder with automatic substitution to remove any word count information 
//             contained in the documents.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_FOLDER_FILES_EXIST                           685
//* Define statement owner: XQG, NEW
// Text:       Error creating folder '%1'. File or directory
//             exists.
// HelpTitle:  OpenTM2 - Help
// Help:       The folder can not be created because a folder with
//             the same name already exists on the selected drive.

#define QUERY_FOL_DELETE_GREYEDOUT                         686
//* Define statement owner: DOE, NEW
// Text:       Folder '%1' is not accessible. Only the files
//             that can be accessed will be deleted. Are you sure
//             you want to delete the Folder?
// HelpTitle:  OpenTM2 - Help
// Help:       The folder is greyed out because it is not
//             accessible. The reason for this can be threefold.
//             Firstly, the drive on which it is located may have been deleted
//             manually. Deleting the folder in this case means
//             that parts of the folder will be physically
//             deleted. If you allow access to this drive again after deleting
//             the folder it will not be accessible as parts will
//             have been deleted.
//             Secondly, the drive may have been removed
//             via the 'Configure drives...' option
//             on the 'Utilities' pull-down
//             either locally or on the server. Again, deleting will
//             physically remove parts of the folder. If the drive
//             is added to the list of configured drives again, the
//             folder will not be accessible.
//             Lastly, the server administrator may have removed a drive
//             for good. In this case, delete the folder as future
//             access to the folder is not possible.
// Type:       EQF_QUERY
// Severity:   YESNO
// Cause:      grayed out or inaccessible folder is to be deleted
// Add.Info:   '%1' = folder name

#define ERROR_TOO_MUCH_DOC_IN_FOLDER                       687
//* Define statement owner: SOK
// Text:       The folder can not be exported , because there are too many
//             documents in the folder.
// HelpTitle:  OpenTM2 - Help
// Help:       The folder you want to export contains too many documents
//             for export. Use the select document function.

#define FREE_0688_xxxxxxxxxxxxxxxxxxxxx                    688
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0689_xxxxxxxxxxxxxxxxxxxxx                    689
//* -------------------------------------------------------
//*     Continuation of Messages for EQF list handler
//* -------------------------------------------------------
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_LST_CREATE_PM                                690
//* Define statement owner: XQG, TAF, LEE
// Text:       The list window could not be created.
//             The Presentation Manager return code is %1.
// HelpTitle:  OpenTM2 - Help
// Help:       The list window could not be created due to
//             system problems (for example, a shortage of system resources).
//             End all programs that you do not currently need and try
//             again. If the problem continues, contact your
//             IBM representative.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      WinCreateStdWindow of list instance window failed.
// Add.Info:   Error code returned by WinGetlastError

#define ERROR_LST_CREATE_INSTANCE                          691
//* Define statement owner: XQG, LEE
// Text:       The list window could not be created.
// HelpTitle:  OpenTM2 - Help
// Help:       The creation of the list window failed
//             due to some errors that were reported prior to
//             this message.
//             Inspect the previously displayed messages.
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      WinCreateStdWindow of list instance window was cancelled
// Cause:      by the list instance itself

#define WARNING_LST_DELETE_LIST                            692
//* Define statement owner: XQG, LEE
// Text:       Are you sure that you want to delete the list '%1'?
// HelpTitle:  OpenTM2 - Help
// Help:       Hint:
//             .br
//             You can export the list before deleting
//             it to save its data.

#define ERROR_LST_IN_USE                                   693
//* Define statement owner: XQG, TAF, LEE
// Text:       List '%1' is in use by another process.
// HelpTitle:  OpenTM2 - Help
// Help:       Wait until all running OpenTM2 processes
//             have stopped,
//             and retry the function. If the problem continues, close
//             OpenTM2, and try again.

#define WARNING_LST_NO_TERMS_MARKED                        694
//* Define statement owner: XQG, LEE
// Text:       No terms have been marked for processing.
// HelpTitle:  OpenTM2 - Help
// Help:       Before terms of a list can be processed they
//             have to be marked using the mark pushbuttons.

#define ERROR_LSTEXP_NOSPACE                               695
//* Define statement owner: XQG, LEE
// Text:       There is not enough space on disk '%1' to export
//             list '%2'.
// HelpTitle:  OpenTM2 - Help
// Help:       Make more space available on the disk by deleting
//             unnecessary files or export the list to another disk.
// Add.Info:   '%1' = target drive, '%2' = name of exported list

#define ERROR_LST_LIST_UPDATED                             696
//* Define statement owner: XQG, LEE
// Text:       List '%1' has been changed.\n
//             Do you want to save the changes before leaving the dialog?
// HelpTitle:  OpenTM2 - Help
// Help:       Either save
//             the changed list before performing the requested function,
//             discard the changes, or return to the dialog.

#define ERROR_LST_SAVE_NO_NAME                             697
//* Define statement owner: XQG, LEE
// Text:       No name was specified for the list.
// HelpTitle:  OpenTM2 - Help
// Help:       To save a list you need to specify a name for it.

#define ERROR_LST_NAME_INVALID                             698
//* Define statement owner: XQG, LEE
// Text:       Name '%1' is not a valid name for a list.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to create the list, either correct
//             the name or specify another one. A list name can have
//             up to eight alphanumeric characters.

#define WARNING_LST_FILE_EXISTS                            699
//* Define statement owner: XQG, LEE
// Text:       The list '%1' already exists.\n
//             Do you want to overwrite it?
// HelpTitle:  OpenTM2 - Help
// Help:       If you do not specify a new list name, the contents of
//             the list will be overwritten by the new data.

#define ERROR_TOO_MUCH_DOC_SELECTED                        700
//* Define statement owner: SOK
// Text:       There are too many documents selected.
//             Selected %1.\n
//             Allowed  %2.\n
// HelpTitle:  OpenTM2 - Help
// Help:       Deselect the left over documents and export the folder.
//             Export the other documents using the export document dialog
//             in OpenTM2 format and import into another
//             folder.

#define ERROR_LST_TOO_LARGE                                701
//* Define statement owner: XQG, LEE
// Text:       List '%1' is too large. Reduce the list size
//             before saving it to disk.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified list is too large to be
//             saved to disk. Remove some of the terms
//             before you save the list.
// Cause:      overall size of list would exceed 64KB
// Add.Info:   '%1' = name of list

#define ERROR_LST_NOSPACE                                  702
//* Define statement owner: XQG, LEE
// Text:       There is not enough space on disk '%2' to save list '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Make more space available on this disk by deleting
//             unnecessary files, or save it to another disk.

#define ERROR_LST_PRINT                                    703
//* Define statement owner: XQG, LEE
// Text:       The printing of list '%1' failed.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that the print spooler is up and running, and that there
//             is enough space on the fixed disk used by the print spooler.

#define ERROR_LST_MARK_NO_NAME                             704
//* Define statement owner: XQG, LEE
// Text:       No exclusion list name is specified.
// HelpTitle:  OpenTM2 - Help
// Help:       You need to select an exclusion list before you can
//             mark terms for an exclusion list.

#define QUERY_LST_EXCL_CREATE                              705
//* Define statement owner: XQG, LEE
// Text:       The exclusion list '%1' does not exist.\n
//             Do you want to create it?
// HelpTitle:  OpenTM2 - Help
// Help:       The specified exclusion list does not exist. You can
//             either create it, or leave this function.

#define INFO_LST_LIST_SAVED                                706
//* Define statement owner: XQG, TAF, LEE
// Text:       The list '%1' has been saved to disk.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified list has been successfully saved to
//             disk. All markings and all selections in the
//             list are available when you open the list again.

#define INFO_LST_MARKS_PROCESSED                           707
//* Define statement owner: XQG, TAF, LEE
// Text:       All markings set in list '%1' have been processed
//             and the changed list has been written to disk.
// HelpTitle:  OpenTM2 - Help
// Help:       The markings in the specified list have been
//             successfully processed and the terms processed
//             have been removed from the list.
//             The changed list has been written to disk.

#define INFO_LST_EXPORTED_INT                              708
//* Define statement owner: XQG, LEE
// Text:       The list '%1' has been successfully exported
//             to drive '%2'.
// HelpTitle:  OpenTM2 - Help
// Help:       The list has been successfully exported in
//             OpenTM2 format to the drive you specified,
//             and you can continue working.

#define INFO_LST_IMPORTED                                  709
//* Define statement owner: XQG, NEW
// Text:       Importing into list '%1' has been successfully
//             completed.
// HelpTitle:  OpenTM2 - Help
// Help:       The list has been successfully imported
//             from the drive in the format you specified, and you
//             can continue working.

#define ERROR_LST_NO_IMPORT_LIST                           710
//* Define statement owner: XQG, TAF, LEE
// Text:       Specify the name of the importing list.
// HelpTitle:  OpenTM2 - Help
// Help:       The name of the list that is to be created during
//             list import is needed. It can have up to eight
//             alphanumeric characters, and
//             no extension is allowed.

#define ERROR_LSTIMP_NOSPACE                               711
//* Define statement owner: XQG, TAF, LEE
// Text:       There is not enough space on disk '%1' to import
//             the file '%2'.
// HelpTitle:  OpenTM2 - Help
// Help:       Make more space available on the disk by deleting
//             unnecessary files, and try again.
// Add.Info:   '%1' = target drive, '%2' = name of imported file

#define FREE_0712_xxxxxxxxxxxxxxxxxxxxx                    712
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_LST_LBFULL                                   713
//* Define statement owner: XQG, TAF, LEE
// Text:       Not all of the terms of the terminology list can be displayed
//             in the terminology list box.
// HelpTitle:  OpenTM2 - Help
// Help:       The terminology list contains more terms than the list box can
//             display at one time. First process the displayed
//             terms. Once the terms have been processed, the remaining
//             terms are displayed in the list box.

#define INFO_ALL_TERMS_PROCESSED                           714
//* Define statement owner: XQG, LEE
// Text:       All markings set in list '%1' have been processed
//             and the resulting list is now empty. The empty
//             list has been removed and the dialog is ended.
// HelpTitle:  OpenTM2 - Help
// Help:       The markings in the specified list have been
//             successfully processed and the terms processed
//             have been removed from the list. Because the list
//             is empty now, it has been removed
//             and the list dialog is stopped.

#define INFO_LST_EXPORTED_EXT                              715
//* Define statement owner: XQG, LEE
// Text:       The list '%1' has been successfully exported to
//             file '%2'.
// HelpTitle:  OpenTM2 - Help
// Help:       The list has been successfully exported
//             to the file in the format you specified, and you can
//             continue working.

#define FREE_0716_xxxxxxxxxxxxxxxxxxxxx                    716
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0717_xxxxxxxxxxxxxxxxxxxxx                    717
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0718_xxxxxxxxxxxxxxxxxxxxx                    718
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0719_xxxxxxxxxxxxxxxxxxxxx                    719
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0720_xxxxxxxxxxxxxxxxxxxxx                    720
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0721_xxxxxxxxxxxxxxxxxxxxx                    721
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0722_xxxxxxxxxxxxxxxxxxxxx                    722
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0723_xxxxxxxxxxxxxxxxxxxxx                    723
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0724_xxxxxxxxxxxxxxxxxxxxx                    724
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0725_xxxxxxxxxxxxxxxxxxxxx                    725
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0726_xxxxxxxxxxxxxxxxxxxxx                    726
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0727_xxxxxxxxxxxxxxxxxxxxx                    727
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0728_xxxxxxxxxxxxxxxxxxxxx                    728
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0729_xxxxxxxxxxxxxxxxxxxxx                    729
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
//* -------------------------------------------------------
//*     Messages for FILTER-Funktion
//* -------------------------------------------------------

#define ERROR_NO_ENTRYFIELD                                730
//* Define statement owner: XQG, LEE
// Text:       You have not selected a dictionary entry field to be
//             associated with the selected operator.
// HelpTitle:  OpenTM2 - Help
// Help:       A dictionary entry field must be selected before an operator
//             can be chosen.

#define ERROR_NO_FILTER_SELECTED                           731
//* Define statement owner: XQG, LEE
// Text:       You have not selected a filter to save the filter condition
//             to.
// HelpTitle:  OpenTM2 - Help
// Help:       You cannot save the filter condition before you choose
//             a filter to which you want to save the filter condition.

#define WARNING_FILTER_EXISTS                              732
//* Define statement owner: XQG, LEE
// Text:       The filter '%1' exists already.\n
//             Do you want to replace it?
// HelpTitle:  OpenTM2 - Help
// Help:       You have asked to save the filter with the specified name.
//             However, this filter exists already.
//             Either replace this filter,
//             or exit this function.

#define ERROR_INVALID_OP                                   733
//* Define statement owner: XQG, LEE
// Text:       The operator that you entered is not valid.
// HelpTitle:  OpenTM2 - Help
// Help:       You have entered an operator that is not in the list of valid
//             operators on the filter dialog. Only the operators listed
//             on the filter dialog can be used for setting up filter
//             conditions.

#define ERROR_BRACKET_MISSING                              734
//* Define statement owner: XQG, LEE
// Text:       A closing bracket is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       You have left out the closing bracket in a bracketed
//             condition.

#define ERROR_SYNTAX_INCORRECT                             735
//* Define statement owner: XQG, LEE
// Text:       You are attempting to set up a condition which
//             has led to a syntax error.
// HelpTitle:  OpenTM2 - Help
// Help:       Check to see if an open bracket is missing.
//             Make sure that you are following the correct syntax.

#define ERROR_BETWEEN_OPERATOR                             736
//* Define statement owner: XQG, LEE
// Text:       Correct syntax has not been followed for the
//             BETWEEN operator.
// HelpTitle:  OpenTM2 - Help
// Help:       The syntax required when using the BETWEEN operator is:
//             .br
//             BETWEEN operand AND operand.

#define ERROR_QUOTE_MISSING                                737
//* Define statement owner: XQG, LEE
// Text:       A required end quote is missing in the specified string.
// HelpTitle:  OpenTM2 - Help
// Help:       The syntax required when representing data is 'data'.

#define ERROR_FIELDNAME_INVALID                            738
//* Define statement owner: XQG, LEE
// Text:       You need to use a dictionary field name that is listed
//             in the Available fields list box on the filter dialog.
// HelpTitle:  OpenTM2 - Help
// Help:       When setting up a filter condition, use the dictionary entry
//             fields in the Available field list box.

#define ERROR_EF_EMPTY                                     739
//* Define statement owner: XQG, LEE
// Text:       You need to enter data with which to associate the
//             dictionary entry field and the selected operator.
// HelpTitle:  OpenTM2 - Help
// Help:       You have not entered any data in the entry field. If you want
//             to specify an empty data string, use the operator '='.

#define WARNING_DELETE_FILTER                              740
//* Define statement owner: XQG, LEE
// Text:       Are you sure that you want to delete Filter '%1'?
// HelpTitle:  OpenTM2 - Help
// Help:       Either delete this filter, or leave this function.

#define ERROR_FILTERNAME_INVALID                           741
//* Define statement owner: XQG, LEE
// Text:       The name '%1' is not a valid filter name.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to create a new filter, either correct
//             the name or specify another one. A filter name
//             can have up to eight alphanumeric characters.

#define ERROR_NO_FILTER_STATEMENTS                         742
//* Define statement owner: XQG, LEE
// Text:       You need to specify some selected fields, a WHERE condition,
//             or both.
// HelpTitle:  OpenTM2 - Help
// Help:       To set up a filter, either the SELECT
//             entry field must contain selected fields, or
//             the WHERE condition must be specified.
//             You can also set up a filter using a
//             combination of SELECT and WHERE.

#define ERROR_FIELDS_NOT_IN_DICT                           743
//* Define statement owner: XQG, LEE
// Text:       The selected filter uses fields that are not
//             part of the dictionary. Do you want to
//             continue anyway?
// HelpTitle:  OpenTM2 - Help
// Help:       There are dictionary fields used in the
//             SELECT or WHERE part of the filter that are not
//             part of the dictionary being processed.
//             :p.
//             Either the filter was created for a dictionary
//             with a different field layout, or the dictionary
//             field names have been changed.
//             :p.
//             Using this filter can lead to unpredictable results.
//* ------------------------------------------------------- */
//*     QDPR messages
//* ------------------------------------------------------- */

#define QDPR_INIT_FAILURE                                  744
//* Define statement owner: MHO, LEE
// Text:       An error occurred during the initialization of
//             the print process.
// HelpTitle:  OpenTM2 - Help
// Help:       Note the conditions that have led to that error,
//             and contact your IBM representative.

#define QDPR_LOAD_DICTIONARY                               745
//* Define statement owner: MHO, LEE
// Text:       The dictionary '%1' could not be loaded properly.
// HelpTitle:  OpenTM2 - Help
// Help:       While trying to load the dictionary an
//             unrecoverable error occurred. Printing cannot
//             continue.
//             :p.
//             Check that the dictionary is set up
//             properly and start the print process again.

#define QDPR_READ_DICTIONARY                               746
//* Define statement owner: MHO, LEE
// Text:       An error occurred while trying to read entries from
//             dictionary '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Printing cannot continue.
//             :p.
//             Check that the dictionary is set up
//             properly and start the print process again.

#define ERROR_INCOMPLETE_CONDITION                         747
//* Define statement owner: XQG, LEE
// Text:       The specified condition is not complete.
// HelpTitle:  OpenTM2 - Help
// Help:       You set up a condition in the WHERE
//             entry field that is not complete.
//             :p.
//             Normally the WHERE condition ends with OR, AND, or
//             NOT but no condition is following.
//             :p.
//             Add the missing condition or remove the last
//             operator.

#define QDPR_NO_ENTRIES_PROCESSED                          748
//* Define statement owner: MHO, LEE
// Text:       No entries were printed from the dictionary '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Possible causes for no entries being printed include:
//             :ul.
//             :li.Your dictionary contains no entries.
//             :li.You selected a filter through which none of the dictionary
//             entries could pass.
//             :eul.
//             :p.
//             The print output might only contain the
//             <dictfront> and the <dictback> part (if specified).
//             :p.
//             If the print output was to be directed to a file and
//             nothing has been written to the file (not even the
//             <dictfront> or the <dictback> part) then the file is
//             deleted.
//             :p.
//             Empty print output directed to a printer can result
//             in an empty page being ejected from the printer.

#define QDPR_NO_FORMAT_SELECTED                            749
//* Define statement owner: MHO, LEE
// Text:       You need to select a format.
// HelpTitle:  OpenTM2 - Help
// Help:       You did not select a format from the format
//             group. Printing cannot be started without
//             specifying a format.
//             :p.
//             Select a format and start the print process again.

#define QDPR_SYER_MISSING_FLTFIELDNAME                     750
//* Define statement owner: XQG
// Text:       The field name '%2' in the format information file
//             '%1' is not selected in the filter '%3'.
//             Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       A '<var name='xxx'>'
//             tag given in the format file
//             contains a field name that is contained
//             in the dictionary but is excluded
//             by the chosen filter.
//             :p.
//             Selecting field names in the format file which are
//             not selected in the filter generates empty lines
//             in the printout.
//             :p.
//             To avoid empty lines either select the referenced
//             field name in the SELECT part of the chosen filter
//             or choose another filter.
// Type:       EQF_QUERY
// Severity:   YESNO

#define QDPR_SCANNING_FORMAT_FILE                          751
//* Define statement owner: MHO, LEE
// Text:       An error occurred while scanning the format
//             information file '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       While OpenTM2 was trying to evaluate
//             the specifications made in the named format
//             information file an error occurred.
//             :p.
//             Ensure that the file exists and that it conforms to
//             the rules for writing format information files.

#define QDPR_NO_VALID_FILE_SELECTED                        752
//* Define statement owner: XQG
// Text:       The file name '%1' you selected for the output file
//             is not valid
//             or it does not exist, or the file cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that the file name is valid. It can have up to
//             eight alphanumeric characters.
//             :p.
//             If the file name is correct, check whether the file cannot be
//             accessed due to disk errors or because it is being used by
//             another process. Wait until the file can
//             be accessed and start the print process again.

#define QDPR_DEST_FILE_ALREADY_EXISTS                      753
//* Define statement owner: MHO, LEE
// Text:       The print destination file '%1' already exists.
//             Do you want to append the print output to the
//             file?
// HelpTitle:  OpenTM2 - Help
// Help:       The file you selected to receive the print
//             output already exists on your disk. A positive response
//             will append the print output to the file
//             and a negative will replace the file.
//             If you do not want to append
//             or replace the file, you return to the
//             previous dialog to select a new file name or
//             to cancel printing.

#define QDPR_REALLY_STOP_PROCESSING                        754
//* Define statement owner: MHO, LEE
// Text:       Do you really want to stop the dictionary print
//             process?
// HelpTitle:  OpenTM2 - Help
// Help:       Stopping the dictionary print process can
//             result in not printing remaining dictionary data.

#define QDPR_ERROR_WRITE_TO_DEST_FILE                      755
//* Define statement owner: MHO, LEE
// Text:       An error occurred while trying to write to the print
//             destination file '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       Another process may be using
//             the file, or the disk you have chosen as destination
//             is full.
//             :p.
//             Check that the file is not locked by another
//             process and that the disk is not full.

#define QDPR_ERROR_WRITE_TO_PRINTER                        756
//* Define statement owner: MHO, LEE
// Text:       An error occurred while trying to write to the printer
//             you selected.
// HelpTitle:  OpenTM2 - Help
// Help:       Check that the printer is online, that there
//             is enough paper, and that the printer is not busy
//             working with other print jobs.

#define QDPR_SYER_SAME_TAG_TWICE                           757
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The '%3' tag on line %2 is specified more than once.
// HelpTitle:  OpenTM2 - Help
// Help:       The tag given in the message is specified more than once
//             in the format information file.
//             :p.
//             Remove the extra tag specification.

#define QDPR_SYER_WRONG_TAG_ORDER                          758
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The tags on line '%2' are in the wrong order.
// HelpTitle:  OpenTM2 - Help
// Help:       The order of the tags is not correct. The tags
//             need to be in the following order:
//             :ol.
//             :li.'<description>' and '</description>' tag
//             :li.'<dictfront>' and '</dictfront>' tag
//             :li.'<pagehead>' and '</pagehead>' tag
//             :li.'<entryprt>' and '</entryprt>' tag
//             :li.'<pagefoot>' and '</pagefoot>' tag
//             :li.'<dictback>' and '</dictback>' tag
//             :li.'<set>' tags can be anywhere in the format information
//             file except within another tag, for example
//             '<entryprt <set sysname=$PAGE_NO VALUE=10>>' is not allowed)
//             :li.'<var>' tags with attribute 'name='xxx'' can only
//             occur between the
//             '<pagehead>' and '</pagehead>', '<entryprt>' and
//             '</entryprt>'
//             and
//             '<pagefoot>' and '</pagefoot>' tags
//             :li.'<var>' tags with attribute 'sysname=$abc' can only
//             occur between the '<dictfront>' and '</dictfront>',
//             '<pagehead>' and '</pagehead>', '<entryprt>' and
//             '</entryprt>' and
//             '<pagefoot>' and '</pagefoot>', '<dictback>' and
//             '</dictback>' tags
//             :li.'<repeat>' and '</repeat>' tags can only occur between
//             the '<entryprt>' and '</entryprt>' tag
//             :li.'<comment>' and '</comment>' tag can be anywhere in the
//             format information file (except within another tag, for example
//             <entryprt <comment>COMMENT</comment>> is not allowed)
//             :eol.
//             :p.
//             Correct the error in the format
//             information file.

#define QDPR_SYER_NO_ENTRY_TAG_FOUND                       759
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The file is missing an '<entryprt>' tag.
// HelpTitle:  OpenTM2 - Help
// Help:       Add an '<entryprt>' tag to the
//             format information file, or delete the file named in the
//             message, because it is not a format information file.

#define QDPR_SYER_TAG_NOT_CLOSED                           760
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The end tag '%4' for the '%3' tag is missing in line %2.
// HelpTitle:  OpenTM2 - Help
// Help:       The tag given in the message is not closed by its
//             matching end tag.
//             Add the end tag to the right place
//             in the format information file.
//             You can also check that all multi-word variable names
//             are enclosed in apostrophes (name='source term').

#define QDPR_SYER_MISSING_START_TAG                        761
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The start tag '%4' for the '%3' tag is missing in line %2.
// HelpTitle:  OpenTM2 - Help
// Help:       There is no start tag for the tag given in the message.
//             Add the start tag to the appropriate position
//             in the format information file.

#define QDPR_SYER_NO_VALID_REPEAT_NAME                     762
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The level or field name '%4' for the '%3' tag
//             on line %2 is not valid.
// HelpTitle:  OpenTM2 - Help
// Help:       You specified a level or field name that is not
//             known to OpenTM2.
//             :p.
//             The valid level names for the '<repeat>' tag are:
//             :ul.
//             :li.name='xxx'
//             .br
//             Specifies that field ('xxx') which is to be checked for
//             changes.
//             :li.level=ENTRY
//             Specifies that all fields on the entry level shall be checked
//             for changes.
//             :li.level=HOM
//             Specifies that all fields on the hom level shall be checked
//             for changes.
//             :li.level=SENSE
//             Specifies that all fields on the sense level shall be checked
//             for changes.
//             :li.level=TARGET
//             Specifies that all fields on the target level shall be checked
//             for changes.
//             :eul.
//             Example: '<repeat level=HOM>'
//             :p.
//             Specify one of the above system variables.

#define QDPR_SYER_NO_VALID_ATTRIBUTE                       763
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The '%3' tag in line %2 contains the attribute
//             '%4' that is not valid for '%5'.
// HelpTitle:  OpenTM2 - Help
// Help:       Attributes can be specified for the tag given in the message.
//             The tag now contains information that
//             is not a valid attribute or system variable.
//             :p.
//             Valid attributes and system variables
//             for the '<var>' tag are:
//             :ul.
//             :li.name='xxx'
//             .br
//             Specifies the field ('xxx') to be taken from the
//             dictionary entry.
//             :li.sysname=$FILENAME
//             .br
//             Substitute this '<var>' tag with the file name to which
//             printing is done.
//             :li.sysname=$DICTNAME
//             .br
//             Substitute this '<var>' tag with the dictionary name which
//             has been selected for printing.
//             :li.sysname=$DATE
//             .br
//             Substitute this '<var>' tag with the current date.
//             :li.sysname=$TIME
//             .br
//             Substitute this '<var>' tag with the current time.
//             :li.sysname=$PAGE_NO
//             .br
//             Substitute this '<var>' tag with the current page number.
//             :li.sysname=$PAGE_EJECT
//             .br
//             At this position make a page eject.
//             :li.format=format
//             .br
//             This specifies format information for the $DATE or $TIME
//             system variables.
//             .br
//             Valid format information for 'sysname=$DATE' is:
//             :ul.
//             :li.'mm/dd/yy' specifies a date like 05/16/92
//             :li.'mm/dd/yyyy' specifies a date like 05/16/1992
//             :li.'yy/mm/dd' specifies a date like 92/05/16
//             :li.'yyyy/mm/dd' specifies a date like 1992/05/16
//             :li.'dd.mm.yy' specifies a date like 16.05.92
//             :li.'dd.mm.yyyy' specifies a date like 16.05.1992
//             :eul.
//             :p.
//             Example: '<var sysname=$DATE format='dd.mm.yy'>'
//             :p.
//             Valid format information for 'sysname=$TIME' is:
//             :ul.
//             :li.'hh&colon.mm&colon.ss' specifies a time like
//             16&colon.44&colon.37
//             :li.'hh&colon.mm' specifies a time like 16&colon.44
//             :li.'hh&colon.mm&colon.ssPM' specifies a time like
//             4&colon.44&colon.37pm
//             :li.'hh&colon.mmPM' specifies a time like 4&colon.44pm
//             :eul.
//             :p.
//             Example: '<var sysname=$TIME format='hh&colon.mm&colon.ss'>'
//             :li.$FIRST_ON_PAGE
//             .br
//             Use the data from the specified field as it has been
//             when the current page was started.
//             .br
//             This attribute is not valid for all 'sysname=XXX'
//             system variables.
//             :li.$LAST_ON_PAGE
//             .br
//             Use the data from the specified field as it has been
//             when the current page is finished.
//             .br
//             This attribute is only valid for '<var>' tags used in the
//             '<pagefoot>' tag.
//             .br
//             This attribute is not valid for all 'sysname=XXX'
//             system variables.
//             :li.$MAX=n
//             .br
//             Specifies the maximum amount of characters to be printed
//             from the specified field. Data longer than 'n' will be
//             truncated.
//             .br
//             If both $MIN and $MAX are specified the value for 'n' in
//             $MIN must be lower or equal than the value for 'n' in $MAX.
//             :li.$MIN=n
//             .br
//             Specifies the minimum amount of characters to be printed
//             from the specified field. Data shorter than 'n' will be
//             filled up with blanks.
//             .br
//             If both $MIN and $MAX are specified the value for 'n' in
//             $MIN must be lower or equal than the value for 'n' in $MAX.
//             :li.$RIGHT
//             .br
//             Right justify the data from the specified field.
//             :li.$LEFT
//             .br
//             Left justify the data from the specified field.
//             :li.$SAME_ENTRY_AGAIN
//             .br
//             Before printing data from the specified field, the
//             total entry is reset to its first template and then
//             the data is extracted from the specified field.
//             .br
//             This attribute is not valid for all 'sysname=XXX'
//             system variables.
//             :li.$NO_DISPLAY
//             .br
//             Do not print data from that field. Mostly used in
//             conjunction with $SAME_ENTRY_AGAIN when only the
//             entry shall be reset but no data shall be printed.
//             .br
//             This attribute is not valid for all 'sysname=XXX'
//             system variables.
//             :eul.
//             :p.
//             Valid attributes and system variables
//             for the '<set>' tag are:
//             :ul.
//             :li.sysname=$LINE_LENGTH
//             .br
//             This specifies the number of characters to be put on one
//             line (default is 80).
//             :li.sysname=$PAGE_LENGTH
//             .br
//             This specifies the number of lines to be put on a page
//             including the data from the '<pagehead>' and '<pagefoot>'
//             tags (default is 72).
//             :li.sysname=$PAGE_NO
//             .br
//             This specifies the page number which shall be the start page
//             number of the print output (default is 1).
//             :li.sysname=$PAGE_EJECT
//             .br
//             This specifies the string that shall be used as the page
//             eject string (default is '').
//             :li.value='yyy'
//             .br
//             This specifies the values for the above system variables.
//             It must always be set together with the corresponding value:
//             .br
//             for $LINE_LENGTH it is the number of characters, for
//             $PAGE_LENGTH it is the number of lines, for $PAGE_NO it
//             is the page number to start with and for $PAGE_EJECT
//             it is the string to be used as page eject string.
//             :eul.
//             :p.
//             Valid attributes for the '<repeat>' tag are:
//             :ul.
//             :li.name='xxx'
//             .br
//             Specifies that field ('xxx') which is to be checked for
//             changes.
//             :li.level=ENTRY
//             Specifies that all fields on the entry level shall be checked
//             for changes.
//             :li.level=HOM
//             Specifies that all fields on the hom level shall be checked
//             for changes.
//             :li.level=SENSE
//             Specifies that all fields on the sense level shall be checked
//             for changes.
//             :li.level=TARGET
//             Specifies that all fields on the target level shall be checked
//             for changes.
//             :eul.
//             :p.
//             Please specify one of the above valid attributes
//             and/or system variables.

#define QDPR_SYER_NO_VITAL_ATTRIBUTE                       764
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The '%3' tag is missing the attribute '%4'
//             for the system variable '%5' in line %2.
// HelpTitle:  OpenTM2 - Help
// Help:       The tag given in the message is missing the required attribute
//             as indicated in the message.
//             :p.
//             Valid attributes for the '<var>' tag are:
//             :ul.
//             :li.format=format
//             .br
//             This is to be supplied if 'sysname=$DATE' or
//             'sysname=$TIME' are used in the '<var>' tag.
//             :eul.
//             :p.
//             The vital attributes for the '<set>' tag are:
//             .br
//             :ul.
//             :li.value=
//             .br
//             This has always got to be specified.
//             :eul.
//             :p.
//             Please supply the one of the above vital attributes.

#define QDPR_SYER_NO_NUM_VALUE                             765
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The attribute '%4' in the '%3' tag in line %2 needs
//             a numeric value after the '=' operator.
// HelpTitle:  OpenTM2 - Help
// Help:       The attribute in the message
//             requires numerical information.
//             This numerical information is specified with
//             the '=' operator (for example '<var name='headword' $MAX=10>').
//             :p.
//             Add a correct numerical value to the attribute.

#define QDPR_SYER_NO_VALID_SYSNAME                         766
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The system variable '%4' for the '%3' tag
//             on line %2 is not valid.
// HelpTitle:  OpenTM2 - Help
// Help:       You specified a system variable that is not
//             known to OpenTM2.
//             :p.
//             The valid system variable for the '<set>' tag are:
//             :ul.
//             :li.sysname=$LINE_LENGTH
//             This specifies the number of characters to be put on one
//             line (default is 80).
//             :li.sysname=$PAGE_LENGTH
//             .br
//             This specifies the number of lines to be put on a page
//             including the data from the '<pagehead>' and '<pagefoot>'
//             tags (default is 72).
//             :li.sysname=$PAGE_NO
//             .br
//             This specifies the page number which shall be the start page
//             number of the print output (default is 1).
//             :li.sysname=$PAGE_EJECT
//             .br
//             This specifies the string that shall be used as the page
//             eject string (default is '').
//             :eul.
//             Example: '<set sysname=$PAGE_NO value=10>'
//             :p.
//             The valid system variable for the '<var>' tag are:
//             :ul.
//             :li.sysname=$FILENAME
//             .br
//             Substitute this '<var>' tag with the file name to which
//             printing is done.
//             :li.sysname=$DICTNAME
//             .br
//             Substitute this '<var>' tag with the dictionary name which
//             has been selected for printing.
//             :li.sysname=$DATE
//             .br
//             Substitute this '<var>' tag with the current date.
//             :li.sysname=$TIME
//             .br
//             Substitute this '<var>' tag with the current time.
//             :li.sysname=$PAGE_NO
//             .br
//             Substitute this '<var>' tag with the current page number.
//             :li.sysname=$PAGE_EJECT
//             .br
//             At this position make a page eject.
//             :eul.
//             Example: '<var sysname=$PAGE_NO>'
//             :p.
//             Please specify one of the above system variables.

#define QDPR_SYER_SAME_ATTRIBUTE_TWICE                     767
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             '%3' tag in line %2 contains the attribute
//             '%4' twice.
// HelpTitle:  OpenTM2 - Help
// Help:       The tag given in the message contains one of its
//             valid attributes twice.
//             :p.
//             Specify the appropriate attribute only once.

#define QDPR_SYER_NO_DATE_TIME_FORMAT                      768
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The attribute '%4' for the
//             system variable '%3' in the '<var>' tag
//             in line %2 contains an
//             invalid format specification.
// HelpTitle:  OpenTM2 - Help
// Help:       The format information specified in the
//             system variable is not valid.
//             :p.
//             Valid format information for 'sysname=$DATE' is:
//             :ul.
//             :li.'mm/dd/yy' specifies a date like 05/16/92
//             :li.'mm/dd/yyyy' specifies a date like 05/16/1992
//             :li.'yy/mm/dd' specifies a date like 92/05/16
//             :li.'yyyy/mm/dd' specifies a date like 1992/05/16
//             :li.'dd.mm.yy' specifies a date like 16.05.92
//             :li.'dd.mm.yyyy' specifies a date like 16.05.1992
//             :eul.
//             :p.
//             Example: '<var sysname=$DATE format='dd.mm.yy'>'
//             :p.
//             Valid format information for 'sysname=$TIME' is:
//             :ul.
//             :li.'hh&colon.mm&colon.ss' specifies a time like
//             16&colon.44&colon.37
//             :li.'hh&colon.mm' specifies a time like 16&colon.44
//             :li.'hh&colon.mm&colon.ssPM' specifies a time like
//             4&colon.44&colon.37pm
//             :li.'hh&colon.mmPM' specifies a time like 4&colon.44pm
//             :eul.
//             :p.
//             Example: '<var sysname=$TIME format='hh&colon.mm&colon.ss'>'
//             :p.
//             Please choose the right format specification.

#define QDPR_SYER_WRONG_FIELDNAME                          769
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The '%3' tag in line %2 contains the field name '%5' that
//             is not specified for dictionary '%4'.
// HelpTitle:  OpenTM2 - Help
// Help:       The '<var name='xxx'>' or '<repeat name='yyy'>'
//             tag given in the
//             message contains a field name that could not
//             be found in the dictionary you selected for
//             printing. Probably the print format file is
//             designed for another dictionary.
//             :p.
//             Ensure that all field names used in the print
//             format file are
//             contained in the dictionary you want to print.
//             :p.
//             In order to correct the print format file
//             do the following:
//             :p.
//             Use your favourite text editor and open the
//             print format file as a plain text file.
//             The print format files are located in the
//             directory :hp2.\EQF\PRTFORM:ehp2. on your primary
//             OpenTM2 drive and have a file
//             extension of :hp2..FRM:ehp2.. Goto the line
//             in error and correct either the field name
//             specified in the :hp2.name=:ehp2. attribute or
//             remove the reference to the field completely.
//             Save the corrected print format file as a plain
//             text file and retry the dictionary print function.

#define QDPR_SYER_EXCL_ATTR_IN_SAME_TAG                    770
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.\n
//             The '%3' tag on line %2 contains attributes that exclude
//             each other.
// HelpTitle:  OpenTM2 - Help
// Help:       For the tag given in the message attributes can
//             be specified. The tag now contains attributes that
//             cannot be specified in the tag if its excluding
//             attribute is also specified.
//             The following attributes are excluding attributes:
//             :ul.
//             :li.$FIRST_ON_PAGE and $LAST_ON_PAGE
//             :li.$RIGHT and $LEFT
//             :eul.
//             :p.
//             Remove one of the excluding attributes.

#define QDPR_SYER_LINELENGTH_TO_LARGE                      771
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The $LINE_LENGTH definition in the '%3' tag on line
//             %2 exceeds the maximum length (255 characters).
// HelpTitle:  OpenTM2 - Help
// Help:       The 'sysname=$LINE_LENGTH' definition in the '<set>'
//             tag exceeds the allowed maximum length of 255
//             characters per line.
//             :p.
//             Correct the value given for 'sysname=$LINE_LENGTH'.

#define QDPR_SYER_PAGELENGTH_TO_LARGE                      772
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The $PAGE_LENGTH definition in the '%3' tag on line
//             %2 exceeds the maximum length.
// HelpTitle:  OpenTM2 - Help
// Help:       The 'sysname=$PAGE_LENGTH' definition in the '<set>'
//             tag exceeds the allowed maximum length of 200
//             lines per page.
//             :p.
//             Correct the value given for 'sysname=$PAGE_LENGTH'.

#define QDPR_SYER_MANY_SYSVAR                              773
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The system variable '%3' on line %2 has already
//             been set.
// HelpTitle:  OpenTM2 - Help
// Help:       The system variable given in the message has
//             already been set somewhere above in the
//             format information file. It can only be specified
//             once.
//             :p.
//             Remove one of the two system variables from the
//             format information file.

#define QDPR_SYER_TOO_LONG_PAGEHEAD                        774
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The text in the page header uses more lines than
//             a page will contain.
// HelpTitle:  OpenTM2 - Help
// Help:       The text between the '<pagehead>' and '</pagehead>'
//             tags uses more lines than 'sysname=$PAGE_LENGTH'
//             specifies (or is set to the default value of 72).
//             :p.
//             You can specify another value for 'sysname=$PAGE_LENGTH'
//             in the '<set>' tag, or you can join or delete lines in the
//             page header.

#define QDPR_SYER_TOO_LONG_PAGEHEADFOOT                    775
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The text in the page header and in the page footer
//             uses more lines than a page will contain.
// HelpTitle:  OpenTM2 - Help
// Help:       The text between the '<pagehead>' and '</pagehead>'
//             and the '<pagefoot>' and '</pagefoot>'
//             tags uses more lines than 'sysname=$PAGE_LENGTH'
//             specifies (or is set to the default value of 72).
//             :p.
//             You can either specify another value for 'sysname=$PAGE_LENGTH'
//             in the '<set>' tag, or you can join or delete lines in the
//             page header and page footer.

#define QDPR_SYER_MIN_GREATER_MAX                          776
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The '%3' tag in line %2 contains a '%4' attribute that
//             is greater than the '%5' attribute also defined.
// HelpTitle:  OpenTM2 - Help
// Help:       You specified a '$MAX=' and '$MIN=' attribute, but the
//             value for the '$MAX=' attribute is less than the
//             value for the '$MIN=' attribute.
//             :p.
//             Specify a smaller value for the '$MIN=' attribute
//             or a larger value for the '$MAX=' attribute.

#define QDPR_SYER_NO_END_TAG_CHAR                          777
//* Define statement owner: MHO, LEE
// Text:       There is a syntax error in the format information
//             file '%1'.
//             The '%3' tag in line %2 is missing the '%4'
//             end-tag character.
// HelpTitle:  OpenTM2 - Help
// Help:       Each tag that you specify closes with an
//             end-tag character. The tag mentioned in the
//             message is missing its end-tag character.
//             :p.
//             Add the end-tag character to the appropriate
//             tag.

#define QDPR_PRINT_SUCCESSFULLY_ENDED                      778
//* Define statement owner: MHO, LEE
// Text:       The dictionary '%1' printed successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred and you can continue working.

#define QDPR_ERROR_OPEN_DEST_FILE                          779
//* Define statement owner: MHO, LEE
// Text:       The file '%1' selected as print destination
//             could not be opened.
// HelpTitle:  OpenTM2 - Help
// Help:       While trying to open the print destination
//             an error occurred.
//             :p.
//             Ensure that the file is not accessed by
//             another process and that the path to the file exists.

#define QDPR_ERROR_OPEN_PRINTER                            780
//* Define statement owner: MHO, LEE
// Text:       The selected printer could not be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       While trying to access the printer
//             you selected an error occurred.
//             :p.
//             Ensure that the printer is defined in your
//             system and that the printer is accessible.
//* -------------------------------------------------------
//*     Folder messages
//* ------------------------------------------------------- */

#define WARNING_NO_PROP_CHANGE_ALLOWED                     781
//* Define statement owner: XQG, LEE
// Text:       The properties of the folder cannot be changed
//             while files of this folder are being analyzed or
//             while the folder is open.
// HelpTitle:  OpenTM2 - Help
// Help:       To change the folder properties, close
//             the folder properties dialog, wait
//             until the analyze function has been completed
//             or close the document list if it is open, and
//             activate the folder properties dialog again.

#define QDPR_PRINT_COPYRIGHT_DICT                          782
//* Define statement owner: MHO, LEE
// Text:       The dictionary '%1' that you selected for printing is
//             copyrighted and cannot be printed.
// HelpTitle:  OpenTM2 - Help
// Help:       Because the dictionary you selected for printing is
//             copyrighted, it cannot be printed or written to a file.
//             :p.
//             Select another dictionary for printing.

#define QDPR_PRINT_PROTECTED_DICT                          783
//* Define statement owner: MHO, LEE
// Text:       The dictionary '%1' you selected for printing is
//             protected and cannot be printed to a file.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to print the dictionary to a file, you need
//             to remove the protection for the dictionary first.
//             If you do not want to remove its protection, you
//             can choose the printer as print destination.

#define SOURCE_LANG_DIFFERENT                              784
//* Define statement owner: XQG, LEE
// Text:       The source languages of the selected dictionaries do
//             not match.\n
//             The source language of dictionary '%1' is '%2', and
//             the source language of the first selected
//             dictionary is '%3'.
// HelpTitle:  OpenTM2 - Help
// Help:       Selecting dictionaries with different
//             source languages can cause problems in
//             the dictionary lookup functions.

#define FOL_FOLDER_ACCESS_ERROR                            785
//* Define statement owner: XQG, NEW
// Text:       The folder '%1' located on drive '%2' cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       The folder can not be accessed because
//             one of the following reasons:
//             :ol.
//             :li.The drive where this folder is located
//             cannot be accessed.
//             :li.The folder or some of the folder files
//             have been deleted manually.
//             :eol.
//             :p.
//             Ensure that the drive for the folder
//             is ready and configured as OpenTM2 drives.
//             Check this with the 'Configure drives...'
//             option in the 'Utilities' pull-down.
//             :p.
//             If the drive is ready and the folder still
//             cannot be accessed you have to delete the
//             the folder and create a new one.
//* -------------------------------------------------------
//*     QPRU messages
//* ------------------------------------------------------- */

#define QPRU_NO_QUEUE_PRINTER                              786
//* Define statement owner: MHO, NEW
// Text:       No printer is attached to default application
//             queue '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to use the print option you must
//             attach a printer to your default application queue.

#define QPRU_ERROR_OPEN_PRINTER                            787
//* Define statement owner: MHO, NEW
// Text:       The printer you attached to your default application
//             queue could not be opened.
// HelpTitle:  OpenTM2 - Help
// Help:       The printer you attached to your default application
//             queue could not be opened.
//             :p.
//             Please check that the printer and the queue are
//             installed correctly.

#define QPRU_NO_QUEUE_SELECTED                             788
//* Define statement owner: MHO, NEW
// Text:       No default application queue is selected for
//             printing.
// HelpTitle:  OpenTM2 - Help
// Help:       In your system no default application queue is
//             selected for printing to.
//             :p.
//             Please install one of your queues as default application
//             queue and attach a printer to it.

#define QPRU_ERROR_QUEUE_INSTALL                           789
//* Define statement owner: MHO, NEW
// Text:       Default application queue '%1' is installed
//             improperly.
// HelpTitle:  OpenTM2 - Help
// Help:       The queue you selected as default application queue
//             is improperly installed.
//             :p.
//             Please check that you selected a printer driver of a
//             printer that has been installed in your system.

#define QPRU_NO_PRINTERS_INSTALLED                         790
//* Define statement owner: MHO, NEW
// Text:       There are no printers installed in your system.
// HelpTitle:  OpenTM2 - Help
// Help:       You didn't install any printers in your system.
//             :p.
//             Please install at least one printer in your system
//             in order to work with the print option.
//* -------------------------------------------------------
//*     Display Language List messages
//* ------------------------------------------------------- */

#define WARNING_DELETE_LANGUAGE                            791
//* Define statement owner: SOK, LEE
// Text:       Are you sure that you want to delete the language
//             support for '%1'?
// HelpTitle:  OpenTM2 - Help
// Help:       If the language support file is
//             deleted, you cannot analyze or open
//             documents with this language as the source language.

#define ERROR_SUPPORTFILE_NOT_DELETED                      792
//* Define statement owner: SOK, NEW
// Text:       The language support file '%1' is in
//             use and cannot be deleted.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected language support file is in use by another
//             process. Close and restart your OpenTM2
//             application and do again.

#define FREE_0793_xxxxxxxxxxxxxxxxxxxxx                    793
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0794_xxxxxxxxxxxxxxxxxxxxx                    794
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define FREE_0795_xxxxxxxxxxxxxxxxxxxxx                    795
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR_INIT_DYNAMIC_DIALOG                          796
//* Define statement owner: XQG
// Text:       The selected dialog could not be initialized.
// HelpTitle:  OpenTM2 - Help
// Help:       Please check that the prerequisites for the
//             selected function are available and correctly
//             installed.

#define ITM_STARTPATHNOTEQUAL                              797
//* Define statement owner: XJR
// Text:       Either source and target startpath must be
//             specified or none of both.
// HelpTitle:  OpenTM2 - Help
// Help:       Please check that either both startpaths are specified
//             or no startpath is specified at all.

#define ERROR_OPEN_NOTESDB                                798
//* Define statement owner: XQG
// Text:       The selected database could not be opened.
// HelpTitle:  OpenTM2 - Help
// Help:       Please ensure that the database is available, either locally
//             or on the specified server.

#define FREE_0799_xxxxxxxxxxxxxxxxxxxxx                    799
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!


//**********************************************************************/
//* DDE MESSAGES                                                       */
//**********************************************************************/

#define DDE_NOTASK                                         800
//* Define statement owner: XJR, LEE
// Text:       No task is specified in the command line.
// HelpTitle:  OpenTM2 - Help
// Help:       The command line does not contain a task
//             or the task is specified incorrectly.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define DDE_WRONGTASK                                      801
//* Define statement owner: XJR, LEE
// Text:       The task specified in the command line is not correct.
// HelpTitle:  OpenTM2 - Help
// Help:       Check that the task you specified is really valid.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define DDE_WRONGCMDLINE                                   802
//* Define statement owner: XJR, LEE
// Text:       The parameter '%1' is specified incorrectly.
// HelpTitle:  OpenTM2 - Help
// Help:       Check the syntax for the parameter that was
//             specified.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define DDE_MANDPARAMISSING                                803
//* Define statement owner: XJR, NEW
// Text:       The required parameter '%1' is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       A parameter is missing that is required
//             for the task specified.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define DDE_NOTIMPLEMENTED                                 804
//* Define statement owner: XJR, LEE
// Text:       The task is not implemented.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified task is not supported yet.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define DDE_WRONGBATCH                                     805
//* Define statement owner: XJR, LEE
// Text:       The parameter '%1'
//             is either not allowed or incompletely
//             specified for the requested task.
// HelpTitle:  OpenTM2 - Help
// Help:       Check which parameters are allowed for the
//             requested task and how
//             they have to be specified.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define DDE_ERRFILELIST                                    806
//* Define statement owner: XJR, LEE
// Text:       The list of files referred\n
//             to in the '%1' statement\n
//             is incorrect.
// HelpTitle:  OpenTM2 - Help
// Help:       There is a syntax error in the list of files.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define DDE_FILE_NO_OVERWRITE                              807
//* Define statement owner: XJR, NEW
// Text:       The file already exists.\n
//             Please specify the option\n
//                  /OVERWRITE=YES\n
//             if you want to overwrite the file.
// HelpTitle:  OpenTM2 - Help
// Help:       No confirmation flag is set in the command line and so
//             the file cannot be overwritten.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define DDE_DOC_NOT_IN_FOLDR                               808
//* Define statement owner: XQG
// Text:       The document '%1' is not contained in folder '%2'.
//             The current batch mode function is cancelled.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified document was not found in the given folder.
//             Correct the name of the document or of the folder and
//             restart the batch mode function.

#define ERROR_DIC_SELECTED                                 809
//* Define statement owner: XQG
// Text:       Dictionary %1 has already been specified.
// HelpTitle:  OpenTM2 - Help
// Help:       The referenced dictionary has already been specified
//             for this folder.

#define ERROR_CMD_TMNOTFOUND                               810
//* Define statement owner: XQG
// Text:       EQFCMD cannot access OpenTM2.\n
//             Please check whether OpenTM2 is running.
// HelpTitle:  OpenTM2 - Help
// Help:       EQFCMD tried to establish a connection to
//             OpenTM2 but there was no
//             OpenTM2 session found.
//             Start OpenTM2 before retrying the
//             batch command.

#define ERROR_CMD_COMPLETED                                811
//* Define statement owner: XQG
// Text:       Task successful completed.
// HelpTitle:  OpenTM2 - Help
// Help:       The batch command has been completed
//             successfully.

#define ERROR_CMD_ERROR                                    812
//* Define statement owner: XQG
// Text:       Error in processing:\n \n
//             %1
// HelpTitle:  OpenTM2 - Help
// Help:       The batch command failed for the
//             listed reason.

#define ERROR_CMD_NOTSUPPORTED                             813
//* Define statement owner: XQG
// Text:       Item not supported
// HelpTitle:  OpenTM2 - Help
// Help:       The item is not supported by EQFCMD,

#define ERROR_CMD_TASKLISTWRONG                            814
//* Define statement owner: XQG
// Text:       Error in Tasklist specification
// HelpTitle:  OpenTM2 - Help
// Help:       The tasklist has been specified incorrect.

#define FREE_0815_xxxxxxxxxxxxxxxxxxxxx                    815
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
//**********************************************************************/
//* WP MESSAGES                                                        */
//**********************************************************************/

#define WP_NO_WPFILE                                       816
//* Define statement owner: JOR, NEW
// Text:       The specified file '%1' is not a WordPerfect 5.x file.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to analyze is not a WordPerfect
//             5.x file and hence could not be analyzed.
//             Please check the file type and specify the correct
//             markup table for analysis.
// Add.Info:   '%1'  file to be analyzed

#define WP_WRONG_FILETYPE                                  817
//* Define statement owner: JOR, NEW
// Text:       The specified file '%1' is not a WordPerfect file.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to analyze is not a WordPerfect
//             file and hence could not be analyzed.
//             The file belongs to the WordPerfect family
//             (e.g. PlanPerfect, WordPerfect Office, etc.) but
//             probable there is no need to translate it.
// Add.Info:   '%1'  file to be analyzed

#define WP_FILE_ENCRYPTED                                  818
//* Define statement owner: JOR, NEW
// Text:       The specified file '%1' is a password protected
//             WordPerfect file.
// HelpTitle:  OpenTM2 - Help
// Help:       Please unprotect the file first using your
//             WordPerfect system and try again.
// Add.Info:   '%1'  file to be analyzed

#define WP_UNEXPECTED                                      819
//* Define statement owner: JOR, NEW
// Text:       Syntax error in the file '%1'.
//             File could not be encoded/decoded.
// HelpTitle:  OpenTM2 - Help
// Help:       Please correct the error and try again.
// Add.Info:   '%1'  file to be analyzed

#define WP_NO_WP6FILE                                      820
//* Define statement owner: JOR, NEW
// Text:       The specified file '%1' is not a WordPerfect 6.x file.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to analyze is not a WordPerfect
//             6.x file and hence could not be analyzed.
//             Please check the file type and specify the correct
//             markup table for analysis.
// Add.Info:   '%1'  file to be analyzed

#define BATCH_NOTASK                                         821
//* Define statement owner: XQG
// Text:       No task is specified in the command line. Use the /task=
//             parameter to specify the task.\n
//             \n
//             Valid tasks are : %1
// HelpTitle:  OpenTM2 - Help
// Help:       The command line does not contain a task
//             or the task is specified incorrectly.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define BATCH_WRONGTASK                                     822
//* Define statement owner: XQG
// Text:       The task specified in the command line is not correct.\n
//             \n
//             Valid tasks are : %1
// HelpTitle:  OpenTM2 - Help
// Help:       Check that the task you specified is really valid.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define BATCH_WRONGVALUE                                   823
//* Define statement owner: XJR, LEE
// Text:       The value '%1' specified for parameter %2 is
//             invalid.\n
//             \n
//             Valid values for this parameter are: %3
// HelpTitle:  OpenTM2 - Help
// Help:       Check the syntax for the parameter that was
//             specified.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define BATCH_WRONGPARM                                   824
//* Define statement owner: XJR, LEE
// Text:       The parameter '%1' is not valid for this task.\n
//             \n
//             Valid parameters for this task are: %2
// HelpTitle:  OpenTM2 - Help
// Help:       Check the syntax for the parameter that was
//             specified.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL

#define FREE_0825_xxxxxxxxxxxxxxxxxxxxx                    825
//* Define statement owner: .............
// Text:       !! No message text defined !!
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
//**********************************************************************/
//* AmiPro Messages                                                    */
//**********************************************************************/

#define ERROR_AMI_GLOSSARY                                 826
//* Define statement owner: LUE
// Text:       The file '%1' can not be segmented.
//             It is a glossary file.
// HelpTitle:  OpenTM2 - Help
// Help:       You tried to analyze a glossary file.
//             As glossary files don't contain any text
//             to be translated the analysis process will
//             be canceled.

#define ERROR_NO_AMI_FORMAT_EXPORT                         827
//* Define statement owner: LUE
// Text:       The file '%1' can not be exported by
//             the Ami Pro analysis support program.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to export was
//             not recognized as an Ami Pro document
//             or you changed internal format information
//             during translation in the translation environment.
//             In the latter case, analyze and translate the
//             document again using the same Translation Memory
//             database as before and correct the changed format
//             information.
//             Internal Ami Pro format information may
//             never be changed by the user.

#define ERROR_NO_AMI_FORMAT_ANALYSE                        828
//* Define statement owner: LUE
// Text:       The file '%1' can not be analyzed by
//             the Ami Pro analysis support program.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to analyze was
//             not recognized as an Ami Pro document
//             by text analysis.
//             Please check the following:
//             Is the document an Ami Pro file?
//             Is the document an Ami Pro glossary data file?
//             These files cannot be analyzed by the Ami Pro
//             analysis support program.
//**********************************************************************/
//* Word Messages                                                      */
//**********************************************************************/

#define WW_NO_WWFILE                                       829
//* Define statement owner: LUE
// Text:       The specified file '%1' is not
//             an MS Word for Windows 2.0 file.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to process is not a
//             MS Word for Windows 2.0 file and hence could
//             not be analyzed/exported.
//             Please check the file type and specify
//             the correct markup table for analysis.

#define WW_DOCUMENT_TEMPLATE                               830
//* Define statement owner: LUE
// Text:       The specified file '%1' is not an MS Word
//             for Windows document, but a template file.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to process is not an
//             MS Word for Windows document, but a template file
//             and hence cannot be translated. Please check
//             if you want to translate the document that is
//             associated to this template and import the correct
//             file and analyze it.

#define WW_COMPLEX_FORMAT                                  831
//* Define statement owner: LUE
// Text:       The specified file '%1' is a fast saved document.
//             Translation Manager can only process full saved
//             documents.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to analyze has been fast saved
//             or autosaved and so can not be processed by the
//             analysis function. Please save the file correctly
//             first in MS Word for Windows and then try again.

#define WW_FILE_ENCRYPTED                                  832
//* Define statement owner: LUE
// Text:       The specified file '%1' is a password
//             protected MS Word for Windows file.
// HelpTitle:  OpenTM2 - Help
// Help:       Please unprotect the file first in
//             MS Word for Windows, save the file,
//             and then try again.

#define WW_NOMORE_PAR_PROPS                                833
//* Define statement owner: LUE
// Text:       The last %1 characters from file '%2' can not be
//             analyzed and will be lost. Analysis will continue
//             nonetheless.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified number of characters cannot be analyzed.
//             Compare the analysed file against the printout to
//             determine what text is missing. To restore the
//             entire file you can try to:
//             :ol.
//             :li.open the source file in MS Word for Windows
//             :li.save it
//             :li.import the file to Translation Manager
//             :li.try to analyze it again
//             :eol.

#define WW_NO_PLCFCHPX                                     834
//* Define statement owner: LUE
// Text:       An error occurred during exporting file '%1'.
//             You may not be able to open the file again in
//             MS Word for Windows. Export will continue nonetheless.
// HelpTitle:  OpenTM2 - Help
// Help:       One reason for not being able to export a file
//             is changes made to tags. Check if you have inserted
//             or deleted any tags either by comparing Translation
//             Memory entries or by opening and saving the file
//             in MS Word for Windows, importing it into
//             Translation Manager, analysing it and then exporting
//             it without translating. If you are able to export
//             the file you have obviously made changes to tags
//             during translation.

#define WW_NO_PLCFPAPX                                     835
//* Define statement owner: LUE
// Text:       An error occurred during exporting file '%1'.
//             You may not be able to open the file again in
//             MS Word for Windows. Export will continue nonetheless.
// HelpTitle:  OpenTM2 - Help
// Help:       One reason for not being able to export a file
//             is changes made to tags. Check if you have inserted
//             or deleted any tags either by comparing Translation
//             Memory entries or by opening and saving the file
//             in MS Word for Windows, importing it into
//             Translation Manager, analysing it and then exporting
//             it without translating. If you are able to export
//             the file you have obviously made changes to tags
//             during translation.
//             The tags you should have a special look at are:
//             [section&colon.&per.&per.&per.] and
//             [paragraph&colon.&per.&per.&per.].

#define WW_NO_PICT                                         836
//* Define statement owner: LUE
// Text:       You may have lost a graphic or an embedded object
//             in file '%1'. Export will continue nonetheless.
// HelpTitle:  OpenTM2 - Help
// Help:       The export function tried to export a picture or
//             an embedded object but couldn't find the reference
//             to it. Perhaps you have added a tag for a picture or
//             an embedded or an error occurred in the export
//             function. When export is complete, open the
//             file in MS Word for Windows and check if a picture,
//             graphic, equation, or any other embedded object is
//             missing. If so, you should open the source file and
//             copy the missing object into the translated file.

#define WW_BKF_NOT_BKL_WARNING                             837
//* Define statement owner: LUE
// Text:       The analyze or export function cannot process
//             the bookmarks defined in file '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you are trying to analyze or export contains
//             bookmarks. But there is a discrepancy between
//             number of starting and ending bookmark references.
//             The bookmarks will be lost.

#define WW_MACRO_ERROR_PARSING                             838
//* Define statement owner: LUE
// Text:       The file '%1' you are trying to analyze
//             contains macros, which cannot be processed
//             by the analyze function.
// HelpTitle:  OpenTM2 - Help
// Help:       Open the document in MS Word for Windows and
//             get rid of saving the macros in the document.
//             Then try analyze again.

#define WW_FILE_NO_READ_WRITE_ERROR                        839
//* Define statement owner: LUE
// Text:       The file '%1' you tried to analyze or export
//             can not be processed.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that the file name is valid. It can have up to
//             eight alphanumeric characters.
//             :p.
//             If the file name is correct, check whether the file cannot be
//             accessed due to disk errors or because it is being used by
//             another process or because it is write protected.
//             Wait until the file can be accessed and start the
//             analyze or export function again.

#define WW_ERROR_NO_WIN_INSTALLED                          840
//* Define statement owner: LUE
//* Message occurs only under OS/2
// Text:       You need to have Win/OS2 installed in order
//             to be able to process the file '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to be able to process documents
//             from MS Word for Windows 6.0 OpenTM2
//             needs to start a small Windows program.
//             The start of the Windows program failed because
//             OpenTM2 was not able to found Win/OS2
//             installed on your PC or PS/2.

#define WW_ERROR_READING_WORD_FROM_WIN                     841
//* Define statement owner: LUE
//* Message occurs only under OS/2
// Text:       The file '%1' you tried to analyze or export
//             can not be processed.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to be able to process documents
//             from MS Word for Windows 6.0 OpenTM2
//             needs to start a small Windows program.
//             OpenTM2 received a timeout that the
//             file can not be parsed. Please ensure that your
//             Win/OS2 is installed correctly, and that the
//             path to your WINOS2.COM is set.
//             A last chance may be, that you tried to
//             process a very large file which may need
//             a few minutes in order to be processed.
//             You may try analysis or export on a faster
//             machine.

#define WW_ERROR_START_WIN                                 842
//* Define statement owner: LUE
//* Message occurs only under OS/2
// Text:       In order to process the file '%1' OpenTM2
//             tried to start a Windows program, but was not successful.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to be able to process documents
//             from MS Word for Windows 6.0 OpenTM2
//             needs to start a small Windows program.
//             The start of the Windows program failed. Please ensure
//             that your path to WINOS2.COM is correct and that
//             Win/OS2 is installed correctly.

#define WW_NO_WW6FILE                                      843
//* Define statement owner: LUE
// Text:       The specified file '%1' is not
//             an MS Word for Windows 6.0 file.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to process is not a
//             MS Word for Windows 6.0 file and hence could
//             not be analyzed/exported.
//             Please check the file type and specify
//             the correct markup table for analysis.

#define ERROR_MIFCHECK_TEXT_TAG                            844
//* Define statement owner: ROB
// Text:       Relevant markup information was changed.
//             The segment can not be saved!\n
//             Should it be automatically corrected?\n
//             Press Yes to perform an automatic correction.
//             You will remain in this segment for verification\n
//             - or -\n
//             press No to stay in this segment and
//             perform your own correction.
// HelpTitle:  OpenTM2 - Help
// Help:       You changed some markup information in the active segment.
//             If this would be saved, FrameMaker would not be able to
//             load this document.
//             Therefore the save is inhibited and an automatic
//             correction could be requested.
//             A final check is then still possible.

#define ERROR_MIFCHECK_TEXT_VAR                            845
//* Define statement owner: ROB
// Text:       You have added, changed, or deleted one or more \xhh.
//             character(s)!\n
//             Press OK to accept the segment as it is now and save it\n
//             - or -\n
//             press Cancel to stay at this segment to review and/or modify
//             it.
// HelpTitle:  OpenTM2 - Help
// Help:       The \xhh. characters represent special characters which are
//             not to be found on your keyboard.
//             Because they usually have to remain in the translation,
//             this warning is shown, if you deleted, changed, or
//             inserted such a character.
//             If you checked your change, you may save the segment
//             nevertheless.

#define ERR_RTF_BRACEMISMATCH                              846
//* Define statement owner: XQG
// Text:       The number of curly braces ("{","}") has been
//             changed. This may prevent processing of the
//             document using your word processor.
//             Do you want to save the segment anyway?
// HelpTitle:  OpenTM2 - Help
// Help:       The number of curly braces has been changed
//             in the active segment. The curly braces are
//             vital tagging information of RTF. If the
//             number of curly braces is modified the
//             document will probably not load anymore in
//             your word processor.
//             :p.
//             Use the source of the segment to determine the
//             correct number and position of the curly braces
//             and correct the translation accordingly.
//**********************************************************************/
//* Lotus Messages                                                     */
//**********************************************************************/

#define ERR_NO_LOTUS_FILE                                  847
//* Define statement owner: LUE
// Text:       The specified file '%1' is not saved in the
//             WK1 format of Lotus 1-2-3.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to analyze is not a
//             Lotus 1-2-3 file and hence could not be analyzed.
//             Please check the file type and specify
//             the correct markup table for analysis.

#define ERR_LOTUS_EXPORT                                   848
//* Define statement owner: LUE
// Text:       The file '%1' can not be exported by
//             the Lotus 1-2-3 export support program.
// HelpTitle:  OpenTM2 - Help
// Help:       An error occurred during the export of
//             the specified file. One reason for this
//             error may be that a tag has been changed
//             during translation. Please open the document
//             again in the Translation Editor and compare
//             the translation with the sources in the
//             Translation Memory. If any tag has been
//             changed, change it back to what is was before.

#define ERR_LOTUS_HEAD_FOOT_MAX                            849
//* Define statement owner: LUE
// Text:       The translation of the header or footer
//             exceeds the maximum allowed size for
//             headers or footers. The export function will
//             continue nonetheless.
// HelpTitle:  OpenTM2 - Help
// Help:       The translation of the header or footer is
//             too long. When the export function has finished
//             you should open the document in Lotus 1-2-3
//             and shorten the text of the header or footer.

#define ERR_LOTUS_LABEL_MAX                                850
//* Define statement owner: LUE
// Text:       The translation in cell %1:%2 exceeds
//             the maximum allowed length for cells.
//             The export function will continue nonetheless.
// HelpTitle:  OpenTM2 - Help
// Help:       The translation in the specified cell is too
//             long. When the export function has finished you
//             should open the document in Lotus 1-2-3 and
//             shorten the text of the specified cell.
//**********************************************************************/
//* Delete machine [m] translation from TM                             */
//**********************************************************************/

#define DMM_WRONGCMDLINE                                   851
//* Define statement owner: MHO, NEW
// Text:       The specified command line has an incorrect
//             syntax.\n
//             The wrong parameter is: '%1'\n
//             The correct syntax is:\n
//              EQFDMM  /ME[M]=memdb\n
//                      [/TY[PE]=NOCONF]\n
//             where\n
//                memdb   is the Translation Memory database name
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Add.Info:   '%1'  wrong parameter

#define DMM_MANDCMDLINE                                    852
//* Define statement owner: MHO, NEW
// Text:       The Translation Memory name as mandatory parameter
//             is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define DMM_COMPLETE                                       853
//* Define statement owner: MHO, NEW
// Text:       Delete [m]-flagged matches successfully finished.\n
//               Segments processed:        %1\n
//               Segments deleted:          %2
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Add.Info:   '%1' = number of segments that were processed,
// Add.Info:   '%2' = number of segments that were deleted
// Type:       EQF_INFO
// Severity:   OK
// Cause:      see error message

#define DMM_EMPTYCMDLINE                                   854
//* Define statement owner: MHO, NEW
// Text:       No command line parameters are given.\n
//             The correct syntax is:\n
//              EQFDMM  /ME[M]=memdb\n
//                      [/TY[PE]=NOCONF]\n
//             where\n
//                memdb   is the Translation Memory database name
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Type:       EQF_ERROR
// Severity:   CANCEL

#define QDPR_NO_FILTER_SELECTED                            855
//* Define statement owner: XQG
// Text:       You need to select a filter if you check
//             the filter checkbox.
// HelpTitle:  OpenTM2 - Help
// Help:       You did not select a filter from the filter
//             group but checked the filter checkbox.
//             Printing using a filter cannot be started without
//             specifying a filter.
//             :p.
//             Either select a filter or deselect the filter
//             checkbox start the print process again.

#define ERROR_FILT_DOESNOTEXIST                            856
//* Define statement owner: XQG, NEW
// Text:       There is no filter with the given name %1.
// HelpTitle:  OpenTM2 - Help
// Help:       You entered the name of a filter but there is no
//             filter with this name.
//             The selected function cannot be started without
//             specifying a filter.
//             :p.
//             Either select an existing filter or create
//             the filter using the edit pushbutton and the filter
//             dialog or deselect the filter checkbox.

#define REVM_WRONGCMDLINE                                  857
//* Define statement owner: Matthew Erbele
// Text:       The specified command line has an incorrect
//             syntax.\n
//             The wrong parameter is: '%1'\n
//             The correct syntax is:\n
//              EQFREVM  /ME[M]=memdb\n
//                          /RE[V]=memdb2\n
//                         [/TY[PE]=NOCONF]\n
//             where\n
//                memdb and memdb2\n
//             are the names of two different
//             Translation Memory databases.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Add.Info:   '%1'  wrong parameter

#define REVM_MANDCMDLINE                                   858
//* Define statement owner: Matthew Erbele
// Text:       Two Translation Memory names are required
//             as mandatory parameters\n
//             The correct syntax is:\n
//              EQFREVM  /ME[M]=memdb\n
//                          /RE[V]=memdb2\n
//                         [/TY[PE]=NOCONF]\n
//             where\n
//                memdb and memdb2\n
//             are the names of two different
//             Translation Memory databases.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define REVM_COMPLETE                                      859
//* Define statement owner: Matthew Erbele
// Text:       Reverse Translation Memory successfully finished.\n
//               Segments processed:        %1\n
//               Segments reversed:          %2
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Add.Info:   '%1' = number of segments that were processed,
// Add.Info:   '%2' = number of segments that were reversed
// Type:       EQF_INFO
// Severity:   OK
// Cause:      see error message

#define REVM_EMPTYCMDLINE                                  860
//* Define statement owner: Matthew Erbele
// Text:       No command line parameters were given.\n
//             The correct syntax is:\n
//              EQFREVM  /ME[M]=memdb\n
//                          /RE[V]=memdb2\n
//                         [/TY[PE]=NOCONF]\n
//             where\n
//                memdb and memdb2\n
//             are the names of two different
//             Translation Memory databases.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Type:       EQF_ERROR
// Severity:   CANCEL
//**********************************************************************/
//* Change machine [m] translation from TM                             */
//**********************************************************************/

#define CMM_WRONGCMDLINE                                   861
//* Define statement owner: ROB
// Text:       The specified command line has an incorrect
//             syntax.\n
//             The wrong parameter is: '%1'\n
//             The correct syntax is:\n
//              EQFCMM  /ME[M]=memdb\n
//                      [/CL[EAR]=YES | NO]\n
//                      [/SE[T]=YES | NO]\n
//                      [/TY[PE]=NOCONF]\n
//             where\n
//                memdb   is the Translation Memory database name
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Add.Info:   '%1'  wrong parameter

#define CMM_MANDCMDLINE                                    862
//* Define statement owner: ROB
// Text:       The Translation Memory name as mandatory parameter
//             is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define CMM_COMPLETE                                       863
//* Define statement owner: ROB
// Text:       Change [m]-flagged matches successfully finished.\n
//               Segments processed:        %1\n
//               Segments cleared:          %2\n
//               Segments set:              %3
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Add.Info:   '%1' = number of segments that were processed,
// Add.Info:   '%2' = number of segments that were cleared
// Add.Info:   '%3' = number of segments that were set
// Type:       EQF_INFO
// Severity:   OK
// Cause:      see error message

#define CMM_EMPTYCMDLINE                                   864
//* Define statement owner: ROB
// Text:       No command line parameters are given.\n
//             The correct syntax is:\n
//              EQFCMM  /ME[M]=memdb\n
//                      [/CL[EAR]=YES | NO]\n
//                      [/SE[T]=YES | NO]\n
//                      [/TY[PE]=NOCONF]\n
//             where\n
//                memdb   is the Translation Memory database name
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Type:       EQF_ERROR
// Severity:   CANCEL

#define ITM_MARKTWO                                        865
//* Define statement owner: XJR
// Text:       Mark the source and its translation in the
//             target document which you want to connect.
// HelpTitle:  OpenTM2 - Help
// Help:       Use the mouse or the corresponding menu item
//             to mark exactly one segment in each document.

#define ITM_MARKANCHOR                                     866
//* Define statement owner: XJR
// Text:       The connection to be deleted is either not marked
//             or not marked correctly.
// HelpTitle:  OpenTM2 - Help
// Help:       Either one connected segment in one document must be
//             marked or the source and its connected target
//             must be marked.

#define ITM_MARKEXACTONE                                   867
//* Define statement owner: XJR
// Text:       Mark exactly one segment. Only one segment can be
//             crossed out at a time.
// HelpTitle:  OpenTM2 - Help
// Help:       Either no segment is marked or in both documents
//             one segment is marked.

#define ITM_NOFOUND                                        868
//* Define statement owner: XJR
// Text:       No further match is found in specified direction.
// HelpTitle:  OpenTM2 - Help
// Help:       There is no further alignment available that
//             corresponds to the search criteria.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL

#define ITM_STARTPATHINVALID                               869
//* Define statement owner: XJR
// Text:       The specified start path is not part of
//             the fully qualified filename '%1'. Please
//             correct either the filename or the start path.
// HelpTitle:  OpenTM2 - Help
// Help:       All source and all target files must be
//             located in a subdirectory of
//             the specified start path.
//* -------------------------------------------------------
//*     Continuation of Translation Memory messages
//* -------------------------------------------------------

#define ERROR_MEM_DIFFERENT_MARKUP                         870
//* Define statement owner: DOE, NEW
// Text:       The markup of the Translation Memory databases
//             you want to merge are different.
//             The markup language
//             of '%1' is '%3', and the markup of '%2' is '%4'.\n
//             Are you sure that you want to merge them?
// HelpTitle:  OpenTM2 - Help
// Help:       If you continue, the merged Translation
//             Memory will contain translations with different markups.
//             .br
//             Merging two Translation Memory
//             databases with different markup,
//             will influence the search for translations when using the
//             merged Translation Memory.
//             :ul.
//             :li.You will get translations with wrong markups.
//             :li.You will get fuzzy instead of exact matches.
//             :eul.
// Type:       EQF_QUERY
// Severity:   YES NO
// Add.Info:   '%1' = source TM
// Add.Info:   '%2' = target TM
// Add.Info:   '%3' = markup of source TM
// Add.Info:   '%4' = markup of target TM
//* -------------------------------------------------------
//*     Continuation of Folder messages (part 3)
//* -------------------------------------------------------

#define ERROR_EXPORT_OPEN_FOLDER                           871
//* Define statement owner: XQG, LEE
// Text:       You cannot export the folder '%1' because it is open.
// HelpTitle:  OpenTM2 - Help
// Help:       A folder cannot be exported while it is open.
//             First close the folder by closing its document list,
//             and then try again.

#define ERROR_IMPORT_OPEN_FOLDER                           872
//* Define statement owner: XQG, LEE
// Text:       You cannot import the folder '%1' because it is open.
// HelpTitle:  OpenTM2 - Help
// Help:       A folder cannot be imported while it is open.
//             First close the folder by closing its document list,
//             and then try again.
//* -------------------------------------------------------
//*     Continuation of Workbench messages
//* -------------------------------------------------------

#define ERROR_NOT_ENHANCEDMODE                             873
//* Define statement owner: XQG, NEW
// Text:       OpenTM2 can only run in Windows enhanced
//             mode.
// HelpTitle:  OpenTM2 - Help
// Help:       OpenTM2 requires Windows enhanced mode.
//             It cannot be run in standard mode. Stop Windows
//             and restart Windows in enhanced mode, or,
//             for OS/2 users, set the Windows mode for the
//             Win-OS/2 session to enhanced.

#define INFO_LANG_ACTIVATED                                874
//* Define statement owner: XQG, NEW
// Text:       The new panel language '%1' has been set. Stop
//             and restart OpenTM2 to activate the
//             new language.
// HelpTitle:  OpenTM2 - Help
// Help:       The panel language of OpenTM2 has been
//             changed, but the change will only be in effect
//             once OpenTM2 has been restarted.

#define ERROR_OPEN_FOL_PROPERTIES                          875
//* Define statement owner: XQG, NEW
// Text:       The properties of the folder '%1' could not be
//             opened.
// HelpTitle:  OpenTM2 - Help
// Help:       An internal control file could not be opened.
//             Either the file does not exist or it is damaged.
//             Restart OpenTM2.
//             If the problem continues, delete the folder
//             and import a backup copy.
//             If the problem continues,
//             contact your IBM representative for further help.
// Cause:      Open of property file for folder failed
// Type:       EQF_ERROR
// Severity:   Cancel
// Add.Info:   '%1' = folder name

#define ERROR_FOLEXP_ACC_TAGTABLE                          876
//* Define statement owner: XQG, NEW
// Text:       The markup table '%1' specified for the folder
//             or one of its documents could not be found.
//             Do you want to continue the folder export?
//* Displayed Buttons: YESNO
// HelpTitle:  OpenTM2 - Help
// Help:       Check whether the markup table specified in the
//             folder properties of this folder or in the
//             document properties of the documents of this
//             folder is still valid.
//             If you continue the folder export
//             the referenced markup table will not be part
//             of the exported folder.

#define ERROR_NO_SOURCE_LANGUAGE                           877
//* Define statement owner: JOR, NEW
// Text:       No source language selected.
//* Displayed Buttons: CANCEL
// HelpTitle:  OpenTM2 - Help
// Help:       Select a source language to create a new Translation
//             Memory.
//             The source language is mandatory for a Translation
//             Memory to provide the linguistic support during
//             finding the best matching proposals.
//             .br
//             If no source language is available please run
//             the 'Display language list' on the 'Utilities'
//             pull-down to see which languages are available from the
//             installation diskettes. Close OpenTM2 and
//             install the language support files you need.
// Type:       EQF_ERROR
// Severity:   CANCEL

#define ERROR_TM_FILES_EXIST                               878
//* Define statement owner: JOR, NEW
// Text:       A Translation Memory or dictionary data file already
//             exists under the same name.\n
//             Do you want to overwrite it?
//* Displayed Buttons: YESNO
// HelpTitle:  OpenTM2 - Help
// Help:       A Translation Memory or dictionary file with the
//             specified name already exists.
//             The reason for this might be two folded.
//             It either might be one for which the property
//             file could not be accessed, or (in the case of
//             a network drive) a Translation
//             Memory or dictionary created by another user.
// Type:       EQF_ERROR
// Severity:   YESNO

#define ERROR_TM_FOR_FOL_NOT_EXIST                         879
//* Define statement owner: XQG, NEW
// Text:       The Translation Memory '%1' specified in the folder
//             properties of folder '%2' does not exist.
// HelpTitle:  OpenTM2 - Help
// Help:       Use the folder property dialog to specify an existing
//             Translation Memory.

#define ERROR_NO_TARGET_LANGUAGE                           880
//* Define statement owner: JOR, NEW
// Text:       No target language selected.
//* Displayed Buttons: CANCEL
// HelpTitle:  OpenTM2 - Help
// Help:       Select the target language used for the translation
//             of the material to be imported.
//             .br
//             The source language used is the one defined in the
//             Translation Memory.
//             .br
//             The target language is mandatory
//             to provide the linguistic support during
//             finding the best matching proposals.
// Type:       EQF_ERROR
// Severity:   CANCEL

#define ERROR_NO_MARKUP                                    881
//* Define statement owner: JOR, NEW
// Text:       No markup selected.
//* Displayed Buttons: CANCEL
// HelpTitle:  OpenTM2 - Help
// Help:       Select the markup table which matches the
//             layout information for the material to be
//             imported.
//             .br
//             The markup is mandatory to find out which
//             of the information will be treated as text
//             and which as layout. The layout will be
//             ignored for finding the best matching proposals.
// Type:       EQF_ERROR
// Severity:   CANCEL

#define ERROR_PRINTER_NOT_READY                            882
//* Define statement owner: JOR, NEW
// Text:       Printer not ready
//* Displayed Buttons: CANCEL
// HelpTitle:  OpenTM2 - Help
// Help:       The printer attached to 'LPT1' is not
//             ready.
//             .br
//             This might be caused by the following reasons:
//             :ul.
//             :li.The printer is offline
//             :li.No paper is available
//             :li.The printer is not attached to 'LPT1'
//             :eul.
// Type:       EQF_ERROR
// Severity:   CANCEL

#define ERROR_PRINTING_COMPLETED                           883
//* Define statement owner: JOR, NEW
// Text:       Printing successfully completed
//* Displayed Buttons: OK
// HelpTitle:  OpenTM2 - Help
// Help:       The printing of the key settings,
//             the file or list was successfully completed.
// Type:       EQF_INFO
// Severity:   OK

#define WW_TRY_SAVE                                        884
//* Define statement owner: LUE
// Text:       The specified file '%1' can not be processed
//             by the analysis or export functions.
// HelpTitle:  OpenTM2 - Help
// Help:       The file you tried to analyze or export is
//             in error. Please open and save the source
//             document in MS Word for Windows 2.0, import
//             it into OpenTM2, and then try the
//             function again.
//             :p.
//             Note:
//             :p.
//             If the error occurred during document export
//             you have to analyze and translate it again
//             before exporting. Use the previously created
//             Translation Memory for translation.

#define ERROR_DELETE_MAINFOLDER                            885
//* Define statement owner: JOR
// Text:       The folder MAIN cannot be deleted.
// HelpTitle:  OpenTM2 - Help
// Help:       The folder MAIN is your default working folder
//             and therefore cannot be deleted.

#define ITM_SAVETOALI                                      886
//* Define statement owner: XJR
// Text:       The current alignments are successfully saved
//             for later continuation.
// HelpTitle:  OpenTM2 - Help
// Help:       The current file pair is now prepared. In your next
//             ITM session it will be in the filelist and is marked
//             with a 'p' in the first column.
//*
// Type:       EQF_INFO
// Severity:   OK

#define ITM_DELETEALI                                      887
//* Define statement owner: XJR
// Text:       Are you sure that you want to delete the prepared
//             alignments?
// HelpTitle:  OpenTM2 - Help
// Help:       The current file pair is already prepared.
//             This command will delete your prepared alignments.
//*
// Type:       EQF_QUERY
// Severity:   YES NO

#define ITM_PARAMNOTMATCH                                  888
//* Define statement owner: XJR
// Text:       The parameter "%1" stored with the prepared alignments
//             is different to your current specifications.
//             Do you want to align the files again with
//             the new specifications?
// HelpTitle:  OpenTM2 - Help
// Help:       You can also delete the old prepared alignments
//             and their parameter specifications using the 'Delete'
//             pushbutton in the dialog.
//             You cannot keep the prepared alignments and change
//             the parameter specifications.
//*
// Type:       EQF_QUERY
// Severity:   YES NO

#define ITM_ALICORRUPT                                     889
//* Define statement owner: XJR
// Text:       The file containing the prepared alignments is corrupted.
//             The files are aligned again.
// HelpTitle:  OpenTM2 - Help
// Help:       If user changes in the visualization of the alignments
//             have been saved, they are lost now.
//*
// Type:       EQF_WARNING
// Severity:   OK

#define ITM_FILEUPDATE                                     890
//* Define statement owner: XJR
// Text:       The file "%1" stored with the prepared alignments
//             has been updated.
//             Thus the prepared alignments are canceled.
//             The files are aligned again.
// HelpTitle:  OpenTM2 - Help
// Help:       If user changes in the visualization of the alignments
//             have been saved, they are lost now.
//*
// Type:       EQF_CANCEL
// Severity:   CANCEL

#define ITM_QUITCHANGES                                    891
//* Define statement owner: XJR
// Text:       Your alignment changes will be lost. \n
//             Do you really want to quit ITM?
// HelpTitle:  OpenTM2 - Help
// Help:       After selecting No, you can either save your current
//             alignment for later continuation or fill the alignment
//             in the Translation Memory.
//*
// Type:       EQF_QUERY
// Severity:   YES NO

#define ITM_TOOMANYFILES                                   892
//* Define statement owner: XJR
// Text:       There are too many files specified to be aligned.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified number of files must be reduced.
//             Update the list or the command line entry so that
//             less file pairs are specified in one ITM call.
//*
// Type:        EQF_CANCEL
// Severity:    CANCEL

#define ITM_MAXALI                                         893
//* Define statement owner: XJR
// Text:       The maximum number of continuation files already
//             exists. Hence this alignment can not be saved for further
//             continuation.
// HelpTitle:  OpenTM2 - Help
// Help:       Please delete continuation files by either writing
//             their contents to the memory or by deleting the file pair
//             in the filelist listbox in the dialog.
//*
// Type:        EQF_CANCEL
// Severity:    CANCEL

#define TMPROP_DEL_MSG                                     894
//* Define statement owner: JOR, NEW
// Text:       You selected to delete the translation \n
//               "%2"\n
//             from the Translation Memory.\n
//             Its source is\n
//               "%1".\n
//             Do you really want to delete the selected proposal?
// HelpTitle:  OpenTM2 - Help
// Help:       If you continue, this sentence and its translation
//             will be deleted from the Translation Memory.
//             .br
//             Please have in mind, that this is a valid translation
//             and was used in a previous translation. If you delete it,
//             there will be no translation available any more for this
//             specific source sentence.
// Type:       EQF_QUERY
// Severity:   YES NO

#define ERROR_TMORG_CANCEL                                 895
//* Define statement owner: XQG
// Text:       Do you really want to cancel the Translation Memory
//             organizing process?
// HelpTitle:  OpenTM2 - Help
// Help:       If you cancel this function, no newly organized Translation
//             Memory is created and the corrupted Translation Memory is
//             still damaged.

#define ERROR_WP_NOT_IN_ACTSEG                             896
//* Define statement owner: XJR, NEW
// Text:       The insertion point is not in the active segment.
//             The requested action is ignored.
// HelpTitle:  OpenTM2 - Help
// Help:       Move the insertion point into the active segment
//             and try again.
//             You can achieve this using the 'Go To Active Segment'
//             function from the Translate pull-down.

#define ERROR_INFO_TM_MAXSELECTED                          897
//* Define statement owner: XJR, NEW
// Text:       The allowed maximum number of Translation
//             Memory databases (10)
//             is already selected.
// HelpTitle:  OpenTM2 - Help
// Help:       The number of Translation Memory databases that can be
//             associated with a folder or document
//             is limited to 10.

#define ERROR_INFO_TM_SELECTED                             898
//* Define statement owner: XJR, NEW
// Text:       The requested Translation Memory is already
//             selected.
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory entries must be unique
//             in the list
//             of selected Translation Memory databases.
//             A previously selected Translation Memory
//             cannot be selected again.

#define ERROR_WP_NO_DICTIONARY_SELECTED                    899
//* Define statement owner: XJR, NEW
// Text:       You have not selected any dictionary to be
//             associated with the file during preparation.
// HelpTitle:  OpenTM2 - Help
// Help:       You have not selected any dictionary during the
//             preparation of the file.
//             Please prepare the file again and select a
//             dictionary to be consulted.

#define ERROR_WP_NO_DICTTERM_FOUND                         900
//* Define statement owner: XJR, NEW
// Text:       No dictionary term was found in the active sentence
// HelpTitle:  OpenTM2 - Help
// Help:       No matching term out of the associated dictionary
//             or list of dictionaries was found in the active sentence.

#define ERROR_NO_TM_ACTIVATED                              901
//* Define statement owner: XJR
// Text:       No Translation Memory is active.
//             Please use the settings dialog to activate one.
// HelpTitle:  OpenTM2 - Help
// Help:       The translation support requires a Translation
//             Memory.
//*
// Type:       EQF_INFO
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_PREPARED                                     902
//* Define statement owner: XJR
// Text:       The file is already prepared for translation.
//             Your request is canceled.
// HelpTitle:  OpenTM2 - Help
// Help:       An already prepared file cannot be prepared
//             again.
//             You can immediately continue translation.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_NOT_PREPARED                                 903
//* Define statement owner: XJR
// Text:       The active document is not prepared for
//             translation.
//             Please select 'Prepare file' to prepare it.
// HelpTitle:  OpenTM2 - Help
// Help:       Your request is cancelled because it is
//             only possible on a prepared file.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_NAMES_EQUAL                                  904
//* Define statement owner: XJR
// Text:       The file names of source and target file are equal.
//             Please enter another extension for the target file.
// HelpTitle:  OpenTM2 - Help
// Help:       The target file must have a different name as the
//             source file.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_OTHERDOC_PREPARED                            905
//* Define statement owner: XJR
// Text:       Currently another document is activated for translation.
//             Please save it and try again.
// HelpTitle:  OpenTM2 - Help
// Help:       Only one prepared document can be open
//             in WordPerfect at a time.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_NOT_ON_DICTENTRY                             906
//* Define statement owner: XJR
// Text:       The cursor is not positioned on a
//             dictionary entry.
// HelpTitle:  OpenTM2 - Help
// Help:       The current word is not available in the
//             dictionary.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_THIRDPARTY_DELETE                            907
//* Define statement owner: XJR
// Text:       You are not allowed to delete information added
//             for use by translation support.
// HelpTitle:  OpenTM2 - Help
// Help:       The information for translation support is
//             deleted when you finish your translation.
//*
// Type:       EQF_WARNING
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_NO_FILE_LOADED                               908
//* Define statement owner: XJR
// Text:       No file is loaded.
//             Please open the file which you want to translate.
// HelpTitle:  OpenTM2 - Help
// Help:       The translation support requires that the file
//             is loaded by WordPerfect.
//*
// Type:       EQF_INFO
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_FORMAT_NOT_SUPPORTED                         909
//* Define statement owner: XJR
// Text:       The file version is currently not supported.
// HelpTitle:  OpenTM2 - Help
// Help:       To use translation support convert the file
//             to WP 6.0 format and try again.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_CONVERSION                                   910
//* Define statement owner: XJR
// Text:       An error occurred during conversion of the
//             file. Please check whether the file is
//             not corrupted.
// HelpTitle:  OpenTM2 - Help
// Help:       Your request is cancelled because the file
//             cannot be converted to a version supported.
//*
// Type:       EQF_ERROR
// Severity:   CANCEL
// Cause:      see error message
// Add.Info:   No additional parameter

#define ERROR_TM_FILE_LANG_DIFFERENT                       911
//* Define statement owner: XJR, NEW
// Text:       The source language of the selected Translation
//             Memory does not match the selected source language
//             for the file.\n
//             The source language of the Translation Memory is '%1', and
//             the source language for the file is '%2'.
// HelpTitle:  OpenTM2 - Help
// Help:       Select the appropriate matching Translation Memory,
//             otherwise you will not get any matching proposals.
// Add.Info:   %1: src language of TM
//             %2: src language selected for file

#define ERROR_TMM_PW_LAST                                  912
//* Define statement owner: LUE, NEW
// Text:       This was you're last attempt at entering
//             the correct password. Your chances are up!
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory Maintenance
//             function is password protected.
//             You are only given three
//             attempts at entering the valid password.

#define ERROR_TMM_PW_UNCORRECT                             913
//* Define statement owner: LUE, NEW
// Text:       The password you have entered is
//             not correct.
// HelpTitle:  OpenTM2 - Help
// Help:       The Translation Memory Maintenance
//             function is password protected.
//             You are only given three
//             attempts at entering the valid password.

#define ERROR_TMM_EDATE_EARLY                              914
//* Define statement owner: LUE, NEW
// Text:       You have specified a time-frame where
//             the end date is dated earlier than the
//             starting date.
// HelpTitle:  OpenTM2 - Help
// Help:       Check the time-frame you have entered.
//             The end date should be more recent than
//             the starting date.

#define ERROR_TMM_START_DATE                               915
//* Define statement owner: LUE, NEW
// Text:       You have specified a date where the
//             starting date is later than the current date.
// HelpTitle:  OpenTM2 - Help
// Help:       Set your starting date back in time.

#define ERROR_TMM_NO_ENTRY                                 916
//* Define statement owner: LUE, NEW
// Text:       No matches were found in the Translation
//             Memory.
// HelpTitle:  OpenTM2 - Help
// Help:       The search query was unsuccessful. No
//             Translation Memory matches were found.
//             Try a new query.

#define ERROR_TMM_QDOC_UNCORRECT                           917
//* Define statement owner: LUE, NEW
// Text:       The entered file name is not valid.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that the name you enter is not longer than
//             eight characters in length and contains alphanumeric
//             characters.
//             .br
//             Correct the name and try again.

#define ERROR_TMM_NO_QDOC_ENTERED                          918
//* Define statement owner: LUE, NEW
// Text:       You have not entered a file name.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that the name you enter is no longer than
//             eight characters in length and contains alphanumeric
//             characters.
//             .br
//             Correct the name and try again. If you do not want to
//             enter a document name, cancel the dialog.

#define ERROR_TMM_LOAD_TOO_SMALL                           919
//* Define statement owner: LUE, NEW, XJR
// Text:       Either you have not entered a number at all
//             or the number you have entered is smaller
//             than 5 or exceeds 99999.
// HelpTitle:  OpenTM2 - Help
// Help:       The number you enter is the number of Translation
//             Memory segments that are loaded into the Translation
//             Memory editor at any one time. This number is not
//             allowed to be smaller than 5 or to exceed 99999.

#define ERROR_TMM_EMPTY                                    920
//* Define statement owner: LUE, NEW
// Text:       The Translation Memory you have selected
//             is empty.
// HelpTitle:  OpenTM2 - Help
// Help:       You cannot open any empty Translation Memory
//             for editing purposes.

#define WARNING_TMM_DELETE_ALL                             921
//* Define statement owner: LUE, NEW
// Text:       Are you sure you want to delete all segments?
// HelpTitle:  OpenTM2 - Help
// Help:       If you select to delete all of the segments
//             displayed in the Translation Memory editor
//             they will be physically deleted from the
//             Translation Memory. When all segments currently
//             loaded in the editor are deleted, and the
//             Translation Memory is not empty, you have the
//             option to either return back to the query dialog
//             or to load the next collection of entries into the
//             editor. If there is no next collection of entries
//             to load, the previous will be loaded if it has not
//             already been deleted.

#define ERROR_TMM_LAST_DELETED                             922
//* Define statement owner: LUE, NEW
// Text:       You have deleted the last translation segment
//             displayed in the editor.
// HelpTitle:  OpenTM2 - Help
// Help:       There are no more segments left to display in the
//             editor. You are returned to the query dialog.

#define WARNING_TMM_DEL_NEXT                               923
//* Define statement owner: LUE, NEW
// Text:       You have not loaded all of the entries that match
//             your query into the editor.
//             Do you want to display the next collection of
//             entries?
// HelpTitle:  OpenTM2 - Help
// Help:       There are still entries left in the Translation
//             Memory that match your query. As you previously
//             selected to delete all entries displayed, do you
//             want to load the next lot of entries to delete
//             them too. If you select not to, you will be
//             returned to the query dialog.

#define WARNING_TMM_DEL_PREV                               924
//* Define statement owner: LUE, NEW
// Text:       The previous collection of entries displayed in the
//             editor also matches your query. Do you want to display
//             this previous collection of entries?
// HelpTitle:  OpenTM2 - Help
// Help:       There are still entries left in the Translation
//             Memory that match your query. As you previously
//             selected to delete all entries displayed, do you
//             want to load the previous lot of entries to delete
//             them too. If you select not to, you will be
//             returned to the query dialog.

#define WARNING_TMM_DEL_EMPTY_TM                           925
//* Define statement owner: LUE, NEW
// Text:       You have deleted all of the segments contained in the selected
//             range of the Translation Memory. You are returned to
//             the main OpenTM2 window.
// HelpTitle:  OpenTM2 - Help
// Help:       There are no more segments left in the selected range
//             of the Translation Memory.
//             You are returned to the main OpenTM2
//             window.

#define ERROR_TMM_DELERR                                   926
//* Define statement owner: XQG
// Text:       Delete of segment failed. Organize the
//             Translation Memory before you retry the
//             operation.
// HelpTitle:  OpenTM2 - Help
// Help:       The delete of a Translation Memory segment
//             failed.
//             .br
//             Probably the Translation Memory is
//             corrupted or contains other inconsistencies.
//             .br
//             Organize the Translation Memory before retrying
//             to repeat the failed function.

#define WARNING_TMM_DELETE_SEG                             927
//* Define statement owner: XQG
// Text:       Are you sure you want to delete the current segment?
// HelpTitle:  OpenTM2 - Help
// Help:       If you select to delete the current segment
//             it will be physically deleted from the
//             Translation Memory.
//* -------------------------------------------------------
//*     Messages for folder rename function
//* -------------------------------------------------------

#define ERROR_INVALID_FOLDER_NAME                          928
//* Define statement owner: XQG
// Text:       The folder name '%1' does not follow the naming
//             conventions.
// HelpTitle:  OpenTM2 - Help
// Help:       A folder name can have up to
//             eight alphanumeric characters. Do not include a file
//             extension.

#define FOLDER_MERGE_CONFIRMATION                          929
//* Define statement owner: XQG
// Text:       The folder '%1' already exists.\n
//             Do you want to merge the imported folder '%2'
//             into this folder?
// HelpTitle:  OpenTM2 - Help
// Help:       There is already a folder with the given name.
//             Importing the folder using the given name means that
//             it will be merged with the existing folder.
// Type:       EQF_QUERY
// Severity:   YESNO

#define ERROR_MISSING_FOLDER_NAME                          930
//* Define statement owner: XQG
// Text:       The folder name is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       Select a folder name from the drop down
//             combination box to merge the imported folder into
//             the selected one or enter a new folder name
//             in the entry field of the drop down combination
//             box to create a new folder.

#define WARNING_NO_NUMERICAL_VALUE                         931
//* Define statement owner: LUE, NEW
// Text:       Not a numerical value, or number less then zero: %1!
// HelpTitle:  OpenTM2 - Help
// Help:       Please type in a numerical value greater or equal zero
//             to have an consistent pay resp. complexity factor.

#define ERROR_MT_MACHINE                                   932
//* Define statement owner: LUE, NEW
// Text:       Error from MT Machine: %1!
// HelpTitle:  OpenTM2 - Help
// Help:       Error from MT Engine, please refer to User Guide

#define WARNING_SHOWTRANS_NOT_SUPPORTED                     933
//* Define statement owner: XJR, NEW
// Text:       The Show Translation function is not supported
//             for the selected document.
// HelpTitle:  OpenTM2 - Help
// Help:       The Show Translation function is only available
//             for selected document types. For the active document
//             it is not available.

#define MESSAGE_RPT_PROFILE_LOAD                            934
//* Define statement owner: DOE, LEE
// Text:       Profile  '%1' was loaded successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
//*       The message is self-explanatory.

#define MESSAGE_RPT_PROFILE_SAVE                            935
//* Define statement owner: DOE, LEE
// Text:       Profile '%1' was saved successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
//*       The message is self-explanatory.


#define ERROR_PROFILE_LOAD                                   936
//* Define statement owner: DOE, LEE
// Text:       The profile '%1' could not be loaded.
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       An error occurred while trying to access the profile.
//             Check if the profile exists.
#define EEA_WRONGCMDLINE                                      937
//* Define statement owner: Guenther Jornitz
// Text:       The specified command line has an incorrect
//             syntax.\n
//             The wrong parameter is: '%1'\n
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Add.Info:   '%1'  wrong parameter

#define EEA_MANDCMDLINE                                   938
//* Define statement owner: Guenther Jornitz
// Text:       A mandatory commandline parameter is missing.\n
//             Please look at the documentation for more information.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define EEA_COMPLETE                                      939
//* Define statement owner: Guenther Jornitz
// Text:       Analysis of provided file/files finished successful.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define EEA_EMPTYCMDLINE                                  940
//* Define statement owner: Guenther Jornitz
// Text:       No command line parameters were given.\n
//             Please look at the documentation for more information.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!
// Type:       EQF_ERROR
// Severity:   CANCEL

#define EEA_NUMOFFILES                                     941
//* Define statement owner: Guenther Jornitz
// Text:       The number of files specified does not match the requested
//             number.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define EEA_LEXPATH_NOT_FOUND                              942
//* Define statement owner: Guenther Jornitz
// Text:       The LEXPATH system variable was not found or is
//             not accessible.
// HelpTitle:  OpenTM2 - Help
// Help:       This system variable is used to access the Easy
//             English dictionary and all other components.
//             Please correct the setup or reinstall
//             the Easy English system .

#define EEA_PROF_NOT_FOUND                                  943
//* Define statement owner: Guenther Jornitz
// Text:       The specified profile or the standard profile could
//             not be found or accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       Please ensure that you specified an existing profile.
//             If the problem persists, please
//             note the conditions that have led to that error,
//             and contact your IBM representative.


#define EEA_TERMFILE_NOT_FOUND                              944
//* Define statement owner: Guenther Jornitz
// Text:       The specified termfile could
//             not be found or accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       Please ensure that you specified an existing termfile.
//             If the problem persists, please
//             note the conditions that have led to that error,
//             and contact your IBM representative.


#define EEA_DLSFILE_NOT_FOUND                              945
//* Define statement owner: Guenther Jornitz
// Text:       The specified dictionary file could
//             not be found or accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       Please ensure that you specified an existing dictionary
//             file.
//             If the problem persists, please
//             note the conditions that have led to that error,
//             and contact your IBM representative.

#define EEA_PROF_WRONG_FORMAT                              946
//* Define statement owner: Guenther Jornitz
// Text:       The specified profile has a wrong format.
// HelpTitle:  OpenTM2 - Help
// Help:       Please correct the error and try again.


#define EEA_INPUT_FILE_NOT_ACCESS                          947
//* Define statement owner: Guenther Jornitz
// Text:       The specified input file cannot be accessed.
// HelpTitle:  OpenTM2 - Help
// Help:       Please ensure that you specified an existing input
//             file.
//             If the problem persists, please
//             note the conditions that have led to that error,
//             and contact your IBM representative.

#define EEA_ENDED_BY_USERREQUEST                           948
//* Define statement owner: Guenther Jornitz
// Text:       The function was stopped by the user.
// HelpTitle:  OpenTM2 - Help
// Help:       The user requested to stop the EEA Analysis or
//             the new term collection.

#define EEA_EXES_NOT_ACCESS                                949
//* Define statement owner: Guenther Jornitz
// Text:       The requested function is not accessible.
// HelpTitle:  OpenTM2 - Help
// Help:       Either the Easy English function or the
//             ETERM function is not available or accessible.
//             Please correct the setup or reinstall
//             the Easy English system .

#define ERROR_RPT_PROFILE_INVALID                          950
//* Define statement owner: DOE, LEE
// Text:       Profile name '%1'  invalid.
// HelpTitle:  OpenTM2 - Help
// Help:       Please insert a valid profile name.
//*       The message is self-explanatory.

#define EEA_WRONG_HANDLE                                  951
//* Define statement owner: JOR
// Text:       The passed handle is invalid
// Help:       The passed handle for the last used data
//             is invalid. Please correct the error and
//             retry the command.

#define EEA_NO_FURTHER_EXCEPTION                           952
//* Define statement owner: JOR
// Text:       No further exception found.
// Help:       No further exception was found in result list.
//             Please go back and correct the previous
//             exceptions.

#define EEA_NO_EXCEPTION_AVAILABLE                         953
//* Define statement owner: JOR
// Text:       No exception available.
// Help:       No exception available in the result list. Please
//             check the clarity and if necessary re-analyze
//             the complete file again.

#define ERROR_BROWSER_NOT_LOADED                         954
//* Define statement owner: JOR
// Text:       The browser control to display the file could not
//             be loaded.
// Help:       The browser control used for displaying the file
//             in WYSIWYG could not be loaded.
//             Please ensure that your system fulfills the prereqs.

#define WARNING_DOCUMENT_CHANGED                         955
//* Define statement owner: JOR
// Text:       The document %1 was changed.
//             Do you want to save your changes?
// Help:       You modified the currently active document.
//             Before loading a new document, you have to decide if
//             you want to save your changes or quit them.

#define EEA_SENTENCE_NO_ERROR                            956
//* Define statement owner: JOR
// Text:       No EEA problems found for the active sentence.
// Help:       The analyzed sentence did not return any problems.

#define EEA_SENTENCE_ERROR                               957
//* Define statement owner: JOR
// Text:       The following EEA problem(s) are found: \n%1.
// Help:       The analyzed sentence did return the following problems.
//             Please have a look at the problems and modify them.


#define MESSAGE_RPT_NO_REDUND_AVAIL                       958
//* Define statement owner: SEK, NEW
// Text:       There is no redundancy data available
//             for the folder '%1'.
// HelpTitle:  OpenTM2 - Help
// Help:       For the selected folder there was no redundancy data found.
//             Do an analysis with the option prepare redundancy Report checked.


#define MESSAGE_RPT_CONSISTENT_VER                                 959
//* Define statement owner: DOE, LEE
// Text:       Consistent Versions used.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
//*       The message is self-explanatory.


#define MESSAGE_RPT_INCONSISTENT_VER                               960
//* Define statement owner: DOE, LEE
// Text:       Inconsistent Versions used. Check History Log!
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you can continue working.
//*       The message is self-explanatory.

#define ERROR_WORDPROINIT_FAILED                         961
//* Define statement owner: XQG
// Text:       Access to WordPro support failed!
// HelpTitle:  OpenTM2 - Help
// Help:       One or more of the WordPro support files could not be accessed.
//             Please re-install the WordPro support files before you
//             retry the function.

#define ERROR_WORDPROSAVE_FAILED                         962
//* Define statement owner: XQG
// Text:       Could not save WordPro document %1 to disk drive %2!
// HelpTitle:  OpenTM2 - Help
// Help:       The support for WordProp documents was not able to save
//             the document to the referenced disk. Most likely this
//             error is caused by insufficient disk space.
//             Make room on the disk and restart the process.

#define ERROR_WORDPROOPEN_FAILED                         963
//* Define statement owner: XQG
// Text:       Could not open WordPro document %1!
// HelpTitle:  OpenTM2 - Help
// Help:       The specified WordProp document could not be openend.
//             Either the document is damaged or it is no WordPro document
//             at all. Ensure that the document is in WordPro format.


#define ERROR_OLE_WRAPPER_1                        964
//* Define statement owner: XQG
// Text:       Error Accessing OleWrapper with function %1!
// HelpTitle:  OpenTM2 - Help
// Help:       The specified Ole function could not be accessed.
//             Ensure that the used application is installed on the system.



#define ERROR_CALL_RTF_PARSER                        965
//* Define statement owner: XQG
// Text:       Error Accessing RTF Parser with function %1!
// HelpTitle:  OpenTM2 - Help
// Help:       The RTF parser could not be accessed.
//             Ensure that the RTF parser is installed on the system.


#define ERROR_CALL_WORD_APPLICATION                        966
//* Define statement owner: XQG
// Text:       Error Accessing MS Word!
// HelpTitle:  OpenTM2 - Help
// Help:       MS Word could not be accessed to convert from DOC to RTF format.
//             Ensure that the MS Word is installed on the system.



#define ERROR_TLEXT_TEST_MESSAGE                        967
//* Define statement owner: XQG
// Text:       TransLexis test message
// HelpTitle:  TransLexis - Help
// Help:       Hallo Kandel


#define TQM_ACC_TO_USERPROF_FAILED                         968
//* Define statement owner: TQM
// Text:       Access to the TQM user profile failed (RC = %1).
// HelpTitle:  OpenTM2 - Help
// Help:       The TQM dataset containing the user profile
//             data could not be accessed for one of the
//             the following reasons:
//             :ol.
//             :li.Insufficient system resources (e.g. main storage).
//             :li.Missing or corrupted TQM user profile.
//             :li.General system failure.
//             :eol.
//             Follow these steps one by one until
//             the dataset can be accessed:
//             :ol.
//             :li.Try to restart TQM.
//             :li.End other applications (e.g Communications Manager)
//             and restart TQM.
//             :li.Reboot your workstation and restart TQM.
//             :li.Reinstall TQM and try to restart it.
//             :li.Call your IBM representative.
//             :eol.

#define TQM_USER_NAME_NOT_UNIQUE                           969
//* Define statement owner: TQM
// Text:       User %1 exists already.
// HelpTitle:  OpenTM2 - Help
// Help:       A user with that name exists already.
//             To change the profile of this user,
//             highlight the user in the 'User Profile List'
//             and select 'Properties' from the 'File'
//             menu.

#define TQM_USER_ID_NOT_UNIQUE                             970
//* Define statement owner: TQM
// Text:       User ID %1 has already been used for user %2.
// HelpTitle:  OpenTM2 - Help
// Help:       The user ID must be unique within the system.
//             The same user ID cannot be used for more than
//             one user. Choose another user ID, and continue.

#define TQM_RESET_PW_REQUIRED                              971
//* Define statement owner: TQM
// Text:       For new users a reset password is required.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to allow new users to log on to TQM
//             a reset password has to be specified here.

#define TQM_USER_CLASS_REQUIRED                            972
//* Define statement owner: TQM
// Text:       A user class is required.
// HelpTitle:  OpenTM2 - Help
// Help:       Select a user class from the drop-down list

#define TQM_USER_NAME_REQUIRED                             973
//* Define statement owner: TQM
// Text:       User name is required.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter the name of the user into the 'Name'
//             box.
//             :p.
//             The name can have up to 30 characters, and must
//             be unique.

#define TQM_USER_ID_REQUIRED                               974
//* Define statement owner: TQM
// Text:       User ID is required.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter an identifier (ID) for the user in the 'User ID'
//             box.
//             :p.
//             The name can have up to 8 characters, and must
//             be unique.


#define TQM_LOGON_USER_ID_REQUIRED                         975
//* Define statement owner: TQM
// Text:       User ID is required.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter your user ID into the 'User ID'
//             box

#define TQM_USER_ID_NOT_FOUND                              976
//* Define statement owner: TQM
// Text:       User ID %1 not found in TQM user table.
// HelpTitle:  OpenTM2 - Help
// Help:       The user ID referenced is not known to TQM.
//             :p.
//             Correct the user ID if it is mispelled.
//             Contact your system administrator in all
//             other cases.

#define TQM_ACCESS_REVOKED                                 977
//* Define statement owner: TQM
// Text:       User ID %1 has been revoked.
// HelpTitle:  OpenTM2 - Help
// Help:       The user ID has been revoked.
//             :p.
//             Contact your system administrator.

#define TQM_PASSWORD_REQUIRED                              978
//* Define statement owner: TQM
// Text:       Password is required.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter the password in the 'Password' box
//             to complete your logon to TQM.

#define TQM_PASSWORD_INVALID                               979
//* Define statement owner: TQM
// Text:       The password is invalid.
// HelpTitle:  OpenTM2 - Help
// Help:       The password you have entered is not the
//             correct one for the current user ID.
//             :p.
//             Correct the password and retry. Keep in mind
//             that the user ID is revoked after 5 invalid
//             logon attempts.

#define TQM_USER_ID_IS_REVOKED                             980
//* Define statement owner: TQM
// Text:       The user ID is revoked due to too many
//             invalid logon attempts.
// HelpTitle:  OpenTM2 - Help
// Help:       Contact your system administrator to reset
//             your user ID.

#define TQM_NEW_PASSWORD_MISMATCH                          981
//* Define statement owner: TQM
// Text:       The new password and the confirmation
//             password do not match.
// HelpTitle:  OpenTM2 - Help
// Help:       To change your password, enter
//             the new password in the 'New password'
//             box, and enter the same password
//             again in the 'Confirmation' box.

#define TQM_PASSWORD_CHANGED                               982
//* Define statement owner: TQM
// Text:       The password has been changed.
// HelpTitle:  OpenTM2 - Help
// Help:       The change of password was successful. Use
//             the new password from now on for logging on

#define TQM_PASSWORD_CHANGE_ERROR                          983
//* Define statement owner: TQM
// Text:       Change of password failed.
// HelpTitle:  OpenTM2 - Help
// Help:       Due to a previously reported error condition,
//             the change of password could not be completed.

#define TQM_PASSWORD_CHANGE_REQUIRED                       984
//* Define statement owner: TQM
// Text:       To complete your logon, change your password.
// HelpTitle:  OpenTM2 - Help
// Help:       Your user ID has been reset by the system
//             administrator.
//             :p.
//             To complete the reset, you must change the
//             reset password.
//             :p.
//             Enter the new password in the 'New password'
//             box, and enter the same password
//             in the 'Confirmation' box.

#define TQM_LOGGED_ON                                      985
//* Define statement owner: TQM
// Text:       Logon to TQM was successful.
// HelpTitle:  OpenTM2 - Help
// Help:       You have successfully logged on to TQM.

#define TQM_USER_LOCKED                                    986
//* Define statement owner: JOR
// Text:       The selected user %1 is currently active.
//             The function you have selected
//             to be carried out on %1 will be stopped.
// HelpTitle:  OpenTM2 - Help
// Help:       Logoff and logon with under a different been completed
//             or terminate it prematurely. Then
//             restart the last function you selected.

#define QUERY_DELETE_USER                                  987
//* Define statement owner: TQM
// Text:       Are you sure that you want to delete user %1?
// HelpTitle:  OpenTM2 - Help
// Help:       By deleting a user you delete the user profile
//             for the user.

#define QUERY_DELETE_EVAL                                  988
//* Define statement owner: TQM
// Text:       Are you sure that you want to delete evaluation
//             profile %1?
// HelpTitle:  OpenTM2 - Help
// Help:       none

#define TQM_EVAL_NAME_NOT_UNIQUE                           989
//* Define statement owner: TQM
// Text:       There is already an evaluation profile %1.
// HelpTitle:  OpenTM2 - Help
// Help:       The referenced evaluation profile exists.
//             already. Select another name for the new
//             evaluation profile or use 'Properties...' from
//             the file pulldown to work on the existing
//             evaluation profile.

#define TQM_EVAL_UNIT_REQUIRED                             990
//* Define statement owner: TQM
// Text:       Unit is a mandatory parameter.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter the unit of measurement in the unit
//             entryfield.

#define TQM_EVAL_WORDS_REQUIRED                            991
//* Define statement owner: TQM
// Text:       Words per unit is a mandatory parameter.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter the number of words per unit in the
//             words per unit unit entryfield.

#define TQM_EVAL_WORDS_INVALID                             992
//* Define statement owner: TQM
// Text:       Words per unit must be numeric.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter the number of words per unit in the
//             words per unit unit entryfield. Only
//             digits are allowed.

#define TQM_EVAL_LIMIT_REQUIRED                            993
//* Define statement owner: TQM
// Text:       Limit for sample is a mandatory parameter.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter the limit for the sample.

#define TQM_EVAL_LIMIT_INVALID                             994
//* Define statement owner: TQM
// Text:       Limit for sample must be numeric.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter the limit for the sample. Use digits
//             only.

#define TQM_EVAL_FACTOR_INVALID                            995
//* Define statement owner: TQM
// Text:       The severity factor must be numeric.
// HelpTitle:  OpenTM2 - Help
// Help:       Only digits and the decimal sign are
//             allowed for the severity factor.

#define TQM_EVAL_NAME_REQUIRED                             996
//* Define statement owner: TQM
// Text:       Evaluation profile name is a mandatory parameter.
// HelpTitle:  OpenTM2 - Help
// Help:       Enter the name of the evaluation profile.

#define TQM_MANDATORY_VALUE_MISSING                        997
//* Define statement owner: TQM
// Text:       A value is required
// HelpTitle:  OpenTM2 - Help
// Help:       No value has been entered in a mandatory
//             entryfield.

#define TQM_NO_NUMERIC_VALUE                               998
//* Define statement owner: TQM
// Text:       Specified value is not numeric.
// HelpTitle:  OpenTM2 - Help
// Help:       You have entered non-numeric data in a field
//             for numeric data. Only the digits '0' to '9'
//             are allowed in this entryfield.

#define QUERY_DELETE_DOCUMENT                              999
//* Define statement owner: TQM
// Text:       Are you sure that you want to delete
//             project %1?
// HelpTitle:  OpenTM2 - Help
// Help:       none

#define TQM_CHAPTERS_OUT_OF_RANGE                         1000
//* Define statement owner: TQM
// Text:       Number of chapters is invalid.
//             Specifiy a value between 1 and 200.
// HelpTitle:  OpenTM2 - Help
// Help:       The number of chapters in the main part of a
//             project must be in the range from 1 to 200.

#define TQM_PROJID_MISSING                                1001
//* Define statement owner: TQM
// Text:       Project ID is a mandantory parameter.
// HelpTitle:  OpenTM2 - Help
// Help:       The project ID is the unique identifier of
//             the project.
//             The project ID may consist of
//             any combination of up to eight alphanumeric characters.

#define TQM_PROJID_IN_USE                                 1002
//* Define statement owner: TQM
// Text:       Project ID %1 is already in use.
// HelpTitle:  OpenTM2 - Help
// Help:       The referenced project ID has already been used
//             as identifier for another project.
//             :p.
//             Use a different name as identifier for this project.
//             :p.
//             The project ID may consist of
//             any combination of up to eight alphanumeric characters.

#define TQM_TOO_MANY_CHAPTERS                             1003
//* Define statement owner: TQM
// Text:       You have specified too many preface/backmatter
//             chapters. The allowed maximum of chapters is
//             10.
// HelpTitle:  OpenTM2 - Help
// Help:       Delete some of the chapters in the chapter MLE
//             so that the count of chapters does not exceed 10.

#define TQM_NO_NUMBER_SYMB                                1004
//* Define statement owner: TQM
// Text:       Missing symbol for chapter number insertion (%1)
//             in chapter name template "%2".
// HelpTitle:  OpenTM2 - Help
// Help:       The chapter name template must contain the symbol
//             %1 which will be substitued with the actual
//             chapter number.
//             :p.
//             Add the mising symbol to the chapter name template.

#define QUERY_DELETE_VEND                                 1005
//* Define statement owner: TQM
// Text:       Are you sure that you want to delete translation company %1
//             and all its projects ?
// HelpTitle:  OpenTM2 - Help
// Help:       Deleting the specified translation company will delete the
//             translation company and all projects belonging to this translation company.
//             Furthermore all samples and results of proofreading
//             for these projects will be deleted.

#define TQM_NOEVAL_LOADED                                 1006
//* Define statement owner: TQM
// Text:       Neither the evaluation profile for PUB nor
//             the evaluation profile for MRI could be
//             loaded. Check the TQM settings.
// HelpTitle:  OpenTM2 - Help
// Help:       None of the default evaluation profiles could be loaded.
//             :p.
//             Either the default evaluation profile names have not
//             been
//             set in the 'Settings' dialog or the evaluation profiles
//             specified in the settings are corrupted or have been
//             deleted.
//             :p.
//             Check the evaluation profiles specified in the 'Settings'
//             dialog before retrying.

#define TQM_NO_LAYOUT                                     1007
//* Define statement owner: TQM
// Text:       There are no physical units or no logical units
//             defined for the project yet. Enter the number of
//             words into the words entryfield and use the
//             Project Layout window
//             to define the physical layout before continuing.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that standard units and physical units
//             are defined for the project.
//             :p.
//             Standard units are entered indirectly by specifying
//             the nunmber of words of the project in the words
//             entry field.
//             :p.
//             Physical units are defined in the Project Layout
//             Window.
//             :p.
//             Use the project layout button to activate
//             the project layout dialog.
//             :p.
//             Enter the physical units into the approbriate
//             fields and save the project layout before
//             trying to compute a sample.

#define TQM_SAMPLE_SIZE_IN_ERROR                          1008
//* Define statement owner: TQM
// Text:       The manual sample size
//             is too small to achieve the targetted risks.
// HelpTitle:  OpenTM2 - Help
// Help:       The sample size is too small to achieve
//             the given risk values.
//             :p.
//             Enlarge the sample size or let TQM compute
//             a sample size for you.

#define TQM_PROJEVAL_NOT_LOADED                           1009
//* Define statement owner: TQM
// Text:       The evaluation profile %1 used by
//             project %2 could not be loaded.
// HelpTitle:  OpenTM2 - Help
// Help:       The evaluation profile which was active when the
//             project was created could not be accessed.
//             :p.
//             Without evaluation profile no sample can be computed.
//             :p.
//             Check the evaluation profiles specified in the 'Settings'
//             dialog or contact your administrator.

#define TQM_PRINTFILENAME_MISSING                         1010
//* Define statement owner: TQM
// Text:       The filename for the print file is a mandantory
//             parameter.
// HelpTitle:  OpenTM2 - Help
// Help:       The filename for the print file is required if
//             file output has been selected.
//             :p.
//             Enter the name for the printout in the filename
//             entry field. The name must follow the OS/2 conventions
//             for filenames.

#define TQM_PRINTFILENAME_INVALID                         1011
//* Define statement owner: TQM
// Text:       The filename for the print file is no valid
//             OS/2 filename.
// HelpTitle:  OpenTM2 - Help
//             Correct the name for the printout in the filename
//             entry field. The name must follow the OS/2 conventions
//             for filenames.
// Help:       !! No help text defined !!

#define TQM_PRINT_SAMPLE_COMPLETE                         1012
//* Define statement owner: TQM
// Text:       Print/Export of Q-Control work file has been
//             completed successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you may continue working.

#define TQM_COMPANY_MISSING                               1013
//* Define statement owner: TQM
// Text:       No translation company selected for project.
// HelpTitle:  OpenTM2 - Help
// Help:       No translation company has been selected in
//             the Transl. company dropdown list.
//             :p.
//             Select one of the companys listed in the
//             company dropdown list.
//             :p.
//             If the company dropdown list is empty, cancel
//             the project profile window and create a
//             company profile using the 'New' option in
//             the 'File' pulldown of the 'translation company list' window.

#define TQM_NO_UNITS                                      1014
//* Define statement owner: TQM
// Text:       There are no physical units defined for the project yet.
//             Enter the number of words into the words entryfield
//             before continuing.
// HelpTitle:  OpenTM2 - Help
// Help:       Ensure that standard units
//             are defined for the project.
//             :p.
//             Standard units are entered indirectly by specifying
//             the nunmber of words of the project in the words
//             entry field.
//             :p.
//             Enter the physical units into the approbriate
//             field.

#define TQM_NEW_PASSWORD_IS_SAME                          1015
//* Define statement owner: TQM
// Text:       The new password must be different from
//             the old password.
// HelpTitle:  OpenTM2 - Help
// Help:       You may not use your old password or the reset
//             password as new password. Choose another password
//             and enter it the new password entryfield and
//             in the password confirmation field.

#define TQM_NO_SAMPLES                                    1016
//* Define statement owner: TQM
// Text:       No samples have been saved for project.
// HelpTitle:  OpenTM2 - Help
// Help:       The results of proofreading can't be entered
//             for this project, as no samples have been saved
//             for the project.
//             :p.
//             Choose Samples from the Project Profile Properties
//             window to compute and save samples.

#define TQM_ERRNUM_INVALID                                1017
//* Define statement owner: TQM
// Text:       Error number is invalid.
// HelpTitle:  OpenTM2 - Help
// Help:       The marked area contains an invalid error number.
//             :p.
//             Error numbers consist of three digits and may be
//             prefixed by a repetition count which must be
//             seperated from the error number by '*' or 'x'.
//             :p.
//             Samples for valid error numbers:
//             :sl.
//             :li.101
//             :li.301
//             :li.2*101
//             :li.5x301
//             :esl.

#define TQM_ERRNUM_UNKNOWN                                1018
//* Define statement owner: TQM
// Text:       No error with the given number.
// HelpTitle:  OpenTM2 - Help
// Help:       The error number specified is not defined in the
//             active evaluation profile.
//             :p.
//             Check the printout of the sample for the
//             errors defined.

#define TQM_PAGES_READ_INVALID                            1019
//* Define statement owner: TQM
// Text:       Read units must in the range from 1 to size
//             of sample.
// HelpTitle:  OpenTM2 - Help
// Help:       The units actually read before the proofreading was
//             aborted must be in the range from 1 up to the
//             size of the sample.

#define TQM_PRINT_RESULT_COMPLETE                         1020
//* Define statement owner: TQM
// Text:       Print/Export of the Q-Control work file with
//             proofreading results has been completed
//             successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you may continue working.

#define TQM_RESULTS_EXIST                                 1021
//* Define statement owner: TQM
// Text:       There are already results stored for check %1 of project
//             %2. Do you want to overwrite these results?
// HelpTitle:  OpenTM2 - Help
// Help:       Results have been entered and stored for the
//             referenced check of the project.
//             :p.
//             These results will be overwritten with the new
//             results if your continue.

#define TQM_PHYS_SAMPLE_SIZE_INVALID                      1022
//* Define statement owner: TQM
// Text:       The physical sample size must be in the range
//             1 to %1 physical units.
// HelpTitle:  OpenTM2 - Help
// Help:       Correct the sample size in physical units.
//             Valid is the range from one physical unit
//             up to the size of the project in physical units.

#define TQM_STD_SAMPLE_SIZE_INVALID                       1023
//* Define statement owner: TQM
// Text:       The standard sample size must be in the range
//             1 to %1 standard units.
// HelpTitle:  OpenTM2 - Help
// Help:       Correct the sample size in standard units.
//             Valid is the range from one standard unit
//             up to the size of the project in standard units.

#define TQM_TOO_FEW_UNITS_SELECTED                        1024
//* Define statement owner: TQM
// Text:       Not all units of the sample have been selected.
//             Do you want to use %1 units as new sample size?
// HelpTitle:  OpenTM2 - Help
// Help:       Either allow reduction of the sample size or
//             select more units in the sample units listbox
//             until the required sample size is reached or
//             use the Redo pushbutton to let the system select
//             the remaining units.

#define TQM_NO_UNITS_SELECTED                             1025
//* Define statement owner: TQM
// Text:       No units for the sample have been selected.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to save the sample
//             select units in the sample units listbox
//             until the sample size has been reached or
//             use the Redo pushbutton to let the system select
//             the sample units.

#define TQM_REPORT_NO_NAME                                1026
//* Define statement owner: TQM
// Text:       Missing report name.
// HelpTitle:  Report name missing
// Help:       You have to enter the name of the report
//             before saving the report to disk.

#define TQM_REPORT_NAME_INVALID                           1027
//* Define statement owner: TQM
// Text:       Name %1 is incorrect.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to save the report to disk
//             correct the name according to the following
//             conventions:
//             The name may be composed of up to
//             eight alphanumeric characters. Do not specify a file
//             extension.

#define TQM_REPORT_CHANGED                                1028
//* Define statement owner: TQM
// Text:       Report has been changed.\n
//             Do you want to save the changes before leaving the dialog?
// HelpTitle:  OpenTM2 - Help
// Help:       The report data has been changed. Either save
//             the changed report before performing the
//             requested function, discard the changes, or return
//             to the dialog.

#define TQM_REPORT_EXISTS                                 1029
//* Define statement owner: TQM
// Text:       The report %1 already exists.\n
//             Do you want to overwrite it?
// HelpTitle:  Report exists
// Help:       If you do not specify a new report name
//             the content of the file will be overwritten by
//             the new data.

#define TQM_REPORT_NO_COLS                                1030
//* Define statement owner: TQM
// Text:       No columns selected for report.
// HelpTitle:  OpenTM2 - Help
// Help:       You have to select one or more columns
//             in order to define a report.

#define TQM_REPORT_CANCEL                                 1031
//* Define statement owner: TQM
// Text:       Do you really want to cancel report processing?
// HelpTitle:  OpenTM2 - Help
// Help:       Cancelling the report generation process
//             results in no report being produced.

#define TQM_REPORT_COMPLETE                               1032
//* Define statement owner: TQM
// Text:       Report %1 has been created successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       No errors occurred, and you may continue working.

#define TQM_REPPAGE_CHANGED                               1033
//* Define statement owner: TQM
// Text:       Report page layout has been changed.\n
//             Do you want to discard the changes?
// HelpTitle:  OpenTM2 - Help
// Help:       The report page layout has been changed.
//             Discard the changes or return
//             to the dialog.

#define TQM_REP_OPEN_TEMP_ERROR                           1034
//* Define statement owner: TQM
// Text:       Error opening temporary file %1 (RC=%2).
// HelpTitle:  OpenTM2 - Help
// Help:       An error occured while the referenced
//             temporary file was opened.

#define TQM_REP_READ_PROJ_ERROR                           1035
//* Define statement owner: TQM
// Text:       Error reading project profile %1 (RC=%2).
// HelpTitle:  OpenTM2 - Help
// Help:       An error occured while trying to read
//             the referenced project profile.

#define TQM_REP_READ_VEND_ERROR                           1036
//* Define statement owner: TQM
// Text:       Error reading translation company profile %1 (RC=%2).
// HelpTitle:  OpenTM2 - Help
// Help:       An error occured while trying to read
//             the referenced translation company profile.

#define TQM_REP_WRITE_TEMP_ERROR                          1037
//* Define statement owner: TQM
// Text:       Error writing to temporary file %1 (RC=%2).
// HelpTitle:  OpenTM2 - Help
// Help:       An error occured while trying to write
//             data to a temporary report file.
//             Maybe the TQM system drive has no more free
//             space left.

#define TQM_REP_OPEN_PRINT_ERROR                          1038
//* Define statement owner: TQM
// Text:       Error opening print output file or destination (RC=%1).
// HelpTitle:  OpenTM2 - Help
// Help:       An error occured while trying to open
//             the print output file or the printer.

#define TQM_REP_READ_TEMP_ERROR                           1039
//* Define statement owner: TQM
// Text:       Error reading from temporary file %1 (RC=%2).
// HelpTitle:  OpenTM2 - Help
// Help:       An error occured while trying to read
//             data from a temporary report file.

#define ERROR_AFC_NOT_ALLOWED_1                        1040
//* Define statement owner: XQG
// Text:       Action '%1' not allowed by AFC folder
// HelpTitle:  OpenTM2 - Help
// Help:       Action not allowed by AFC folder ask for password.

#define TQM_REP_REPORT_LOAD_ERROR                       1041
//* Define statement owner: TQM
// Text:       Error loading report definition %1.
// HelpTitle:  OpenTM2 - Help
// Help:       An error occured while loading the selected
//             report definition. The selected file may
//             be no TQM report definition file or the file
//             may have been corrupted.

#define TQM_NEW_CUSTOMER_CONFIRMATION                   1042
//* Define statement owner: TQM
// Text:       %1 is a new customer. Do you want to add it to
//             the list of customer names?
// HelpTitle:  OpenTM2 - Help
// Help:       You have entered the name of a customer which
//             is not yet in the list of customer names.
//             Confirm this message if you want to add the new
//             name.

#define TQM_NEW_MANAGER_CONFIRMATION                   1043
//* Define statement owner: TQM
// Text:       %1 is a new manager. Do you want to add it to
//             the list of manager names?
// HelpTitle:  OpenTM2 - Help
// Help:       You have entered the name of a manager which
//             is not yet in the list of manager names.
//             Confirm this message if you want to add the new
//             name.

#define TQM_NEW_READER_CONFIRMATION                    1044
//* Define statement owner: TQM
// Text:       %1 is the name of a new quality company. Do you want
//             to add it to the list of quality company names?
// HelpTitle:  OpenTM2 - Help
// Help:       You have entered the name of a customer which
//             is not yet in the list of customer names.
//             Confirm this message if you want to add the new
//             name.

#define TQM_EVAL_RISK_REQUIRED                          1045
//* Define statement owner: TQM
// Text:       Please specify a risk value in the
//             range 0 to 10.
// HelpTitle:  OpenTM2 - Help
// Help:       The risk values are required to compute the size of the
//             sample. The lower the risk the bigger the sample. The
//             valid range for risks is 0 to 10 (%).

#define TQM_EVAL_RISK_INVALID                           1046
//* Define statement owner: TQM
// Text:       The specified risk value is not valid. Please
//             specify a risk value in the range 0 to 10.
// HelpTitle:  OpenTM2 - Help
// Help:       The risk values are required to compute the size of the
//             sample. The lower the risk the bigger the sample. The
//             valid range for risks is 0 to 10 (%).

#define TQM_QCONTROL_READ_ERROR                         1047
//* Define statement owner: TQM
// Text:       The file %1 is no valid Q-Control work file.
//             Please select a valid Q-Control work file for import.
// HelpTitle:  OpenTM2 - Help
// Help:       The selected file is no valid Q-Control work file or
//             the Q-Control work file has been corrupted.

#define TQM_QCONTROL_INVERRDATA_ERROR                    1048
//* Define statement owner: TQM
// Text:       The error data for page %1 in the Q_Control work file
//             is not valid.
//             Please correct the value an re-import the file.
// HelpTitle:  OpenTM2 - Help
// Help:       The error values for the referenced page seems
//             to be incorrect. Please correct the data and
//             re-import the file.

#define TQM_QCONTROL_PROJID_MISMATCH                    1049
//* Define statement owner: TQM
// Text:       The project ID in the Q-Control work file (%1)
//             does not match the ID of the current project.
// HelpTitle:  OpenTM2 - Help
// Help:       The project ID in the Q-Control work file is different
//             from the ID of the project currently open.
//             Please ensure that the Q-Control work file for the
//             currently active project is imported.

#define TQM_ERRNUM_INFILE_INVALID                         1050
//* Define statement owner: TQM
// Text:       The error specification for page %1 is invalid.
// HelpTitle:  OpenTM2 - Help
// Help:       The error specification for the given page
//             contains an invalid error number.
//             :p.
//             Error numbers consist of three digits and may be
//             prefixed by a repetition count which must be
//             seperated from the error number by '*' or 'x'.
//             :p.
//             Samples for valid error numbers:
//             :sl.
//             :li.101
//             :li.301
//             :li.2*101
//             :li.5x301
//             :esl.

#define TQM_ERRNUM_INFILE_UNKNOWN                                1051
//* Define statement owner: TQM
// Text:       No error with the given error number for page %1.
// HelpTitle:  OpenTM2 - Help
// Help:       The error number specified for the given page
//             is not defined in the
//             active evaluation profile.
//             :p.
//             Check the printout of the sample for the
//             errors defined.

#define TQM_QCONTROL_READ_OK                                  1052
//* Define statement owner: TQM
// Text:       The Q-Control work file %1 has been imported
//             successfully.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified Q-Control work file has been imported.
//             To save the results use the :hp2.Save results:ehp2.
//             button.

#define TQM_EXEMPTIONSUCCESS                                  1053
//* Define statement owner: TQM
// Text:       The Q-Control exemption was successfull.
// HelpTitle:  OpenTM2 - Help
// Help:       The Q-Control exemption succeeded as the
//             translation company has 5 or more accepted
//             projects in a row and the draw was for the
//             Q-Control exemption.

#define TQM_DRAWFAILED                                  1054
//* Define statement owner: TQM
// Text:       The draw for Q-Control exemption was not successfull.
// HelpTitle:  OpenTM2 - Help
// Help:       No Q-Control exemption because the draw
//             for Q-Control exemption failed.
//             The translation company has 5 or more 5 accepted
//             projects in a row but there is a chance of 20%
//             against Q-Control exemption.

#define TQM_NOTENOUGHPROJECTS                              1055
//* Define statement owner: TQM
// Text:       Not enough accepted projects of translation company.
// HelpTitle:  OpenTM2 - Help
// Help:       No Q-Control exemption because there are
//             less than 5 accepted projects in a row of the
//             selected translation company.

#define TQM_NOVENDORDATA_LOADED                            1056
//* Define statement owner: TQM
// Text:       Access to translation company profile failed.
// HelpTitle:  OpenTM2 - Help
// Help:       No Q-Control exemption check could be performed
//             as the access to the translation company profile
//             failed.
//             :p.
//             Please check the translation company profile.

#define TQM_CHANGEWORDS_WARNING                         1057
//* Define statement owner: TQM
// Text:       If you change the number of words any existing
//             sample and project layout gets obsolete and will
//             will be deleted. Do you want to continue?
// HelpTitle:  OpenTM2 - Help
// Help:       If the number of words of a project is changed
//             any existing project layout and any existing sample
//             gets obsolete. If you continue the system will delete
//             the layout and the sample data and you have to
//             re-enter the data.

#define TQM_EVAL_WVALUE_REQUIRED                          1058
//* Define statement owner: TQM
// Text:       Please specify a value for this factor in the
//             range of 1 to 100.
// HelpTitle:  OpenTM2 - Help
// Help:       The factors are required to compute the quality
//             compound index.
//             The valid range for these factors is 1 to 100.

#define TQM_EVAL_WVALUE_INVALID                           1059
//* Define statement owner: TQM
// Text:       The specified factor is not valid. Please
//             specify a factor in the range 1 to 100.
// HelpTitle:  OpenTM2 - Help
// Help:       The factors are required to compute the quality
//             compound index.
//             The valid range for these factors is 1 to 100.

#define TQM_EVAL_TARGET_REQUIRED                          1060
//* Define statement owner: TQM
// Text:       Please specify a value for the quality target
//             in the range from 1.0 to 9.9
// HelpTitle:  OpenTM2 - Help
// Help:       The quality target defines the minimu score which
//             has to be reached by a project to be accepted.
//             The valid range for this target is 1.0 up to 9.9

#define TQM_EVAL_TARGET_INVALID                           1061
//* Define statement owner: TQM
// Text:       The specified quality target is not valid. Please
//             specify a value for the quality target
//             in the range from 1.0 to 9.9
// HelpTitle:  OpenTM2 - Help
// Help:       The quality target defines the minimu score which
//             has to be reached by a project to be accepted.
//             The valid range for this target is 1.0 up to 9.9

#define TQM_EVAL_PROB_REQUIRED                          1062
//* Define statement owner: TQM
// Text:       Please specify a value for the probability (in %%) of a
//             project being q-control exemption eligible. The
//             valid range is 50 to 99 (%%).
// HelpTitle:  OpenTM2 - Help
// Help:       The probability for a project being q-control
//             exemption eligible defines chance for the project
//             during the draw fro q-control exemption.
//             The valid range is 50 to 99 (%).

#define TQM_EVAL_PROB_INVALID                           1063
//* Define statement owner: TQM
// Text:       The specified quality value is not valid. Please
//             specify a value for the probability (in %%) of a
//             project being q-control exemption eligible. The
//             valid range is 50 to 99 (%%).
// HelpTitle:  OpenTM2 - Help
// Help:       The probability for a project being q-control
//             exemption eligible defines chance for the project
//             during the draw fro q-control exemption.
//             The valid range is 50 to 99 (%).

#define TQM_REPORT_DELETE_CONF                           1064
//* Define statement owner: TQM
// Text:       Are you sure that you want to delete report %1?
// HelpTitle:  OpenTM2 - Help
// Help:       If you continue the referenced report will be
//             deleted.

#define CLBFILT_OP_REQUIRED                              1065
//* Define statement owner: XQG
// Text:       Please select one of the condition operators.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to specify a complete condition a condition
//             operator is required. Please select one of the condition
//             operators from the condition dropdown list.

#define CLBFILT_COL_REQUIRED                             1066
//* Define statement owner: XQG
// Text:       Please select a column for the condition.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to specify a complete condition a column
//             has to be selected from the column dropdown list.

#define CLBFILT_VAL_REQUIRED                             1067
//* Define statement owner: XQG
// Text:       Please enter a value to complete the condition.
// HelpTitle:  OpenTM2 - Help
// Help:       In order to specify a complete condition a value
//             has to be entered which will be checked against the
//             the actual column data. This value may be part of a name
//             (e.g.  :hp1.*smith*:ehp1. in conjunction with the Like
//             operator). The value field can be empty when using the
//             Equal or the Not Equal condition to check for empty
//             column data.

#define TQM_FINALRESULT_QUERY                             1068
//* Define statement owner: TQM
// Text:       Are you sure that the final results have been entered?\n
//             \n
//             If you continue the results are frozen and will be
//             used for reports and q-control exemption checks.
// HelpTitle:  OpenTM2 - Help
// Help:       If you continue the entered results are frozen and cannot be
//             changed anymore but will be used fro reoport creation and
//             q-control exemption checks.

#define ERROR_TMM_SEARCHSEGNUM                            1069
//* Define statement owner:
// Text:       The segment number to search for is invalid.
// HelpTitle:  OpenTM2 - Help
// Help:       You have specified a value that isn't really
//             a number.

#define ERROR_TMM_SHOWPMSEGS                              1070
//* Define statement owner:
// Text:       The number of surrounding segments is invalid.
// HelpTitle:  OpenTM2 - Help
// Help:       The number of surrounding segments must be in the
//             range from 0 to 5.



#define AFC_PASSWORD_INVALID                              1071
//* Define statement owner:
// Text:       The password is invalid.
// HelpTitle:  OpenTM2 - Help
// Help:       The password you have entered is not the
//             correct one.
//             :p.
//             Correct the password and retry.




#define ERROR_CALL_POWERPOINT_APPL                         1072
//* Define statement owner: XQG
// Text:       Error Accessing MS Powerpoint!
// HelpTitle:  OpenTM2 - Help
// Help:       MS Powerpoint could not be accessed to convert ppt format.
//             Ensure that the MS Powerpoint is installed on the system.

#define ERROR_OLE_WRAPPER_RC                               1073
//* Define statement owner: bt
// Text:       Error Accessing OleWrapper with function %1 (RC=%2)!
// HelpTitle:  OpenTM2 - Help
// Help:       The specified Ole function could not be accessed.
//             Ensure that the used application is installed on the system.

#define ERROR_KILL_HANGING_APP                             1074
//* Define statement owner: bt
// Text:       Couldn't shutdown hanging application. Please close all programs \n
//             and restart the system
// HelpTitle:  OpenTM2 - Help
// Help:       A hanging background application couldn't shut down.

#define WARNING_RTF_BRACEMISMATCH                          1075
//* Define statement owner: bt
// Text:       Brace mismatch detected - current segment is skipped.
// HelpTitle:  OpenTM2 - Help
// Help:       Brace mismatch detected - current segment is skipped.

#define TQM_QCONTROLEXEMPT_EXISTS                          1076
//* Define statement owner: TQM
// Text:       Q-Control exemption for this project already done. Using
//             results of last Q-Control exemption check.
// HelpTitle:  OpenTM2 - Help
// Help:       A Q-Control exemption check has already been performed for
//             this project. It is not possible to redo the check. The
//             results of the existing check are applied to the project.

#define TA_MANDPASSWORD                                    1077
//* Define statement owner: WL
// Text:       The password as mandatory parameter is missing.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define TA_PASSWORDTOOLONG                                 1078
//* Define statement owner: WL
// Text:       The password is too long (please use only 6 chars).
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!


#define INFO_CLOSE_OLEAPP                                  1079
//* Define statement owner: bt
// Text:       Press the OK button to close %1.
// HelpTitle:  OpenTM2 - Help
// Help:       Press OK to close application.

#define TQM_SAMPLE_EXISTS                                  1080
//* Define statement owner: TQM
// Text:        One/More Samples are already saved for this project. \n
//              You can only edit the actual sample. Please make sure \n
//              that you have entered all neccessary information for \n
//              the existing sample.
//              \n Would you like to proceed?
// HelpTitle:  OpenTM2 - Help
// Help:       One/More Samples are already saved for this project. You can only edit the actual sample.
//             Please make sure that you have entered all neccessary information for the existing sample.


#define TQM_NONSTANDARD_PROJECT                                  1081
//* Define statement owner: TQM
// Text:        For NonStandard projects it is not possible to define \n
//              Error Categories and/or formulas
// HelpTitle:  OpenTM2 - Help
// Help:       For NonStandard projects it is not possible to define \n
//             Error Categories and/or formulas


#define ERROR_REGISTER_CF                                        1082
//* Define statement owner: cv
// Text:       Unable to register custom clipboard format.\n
//             Copy/move or drag/drop operations will be disabled.
// HelpTitle:  OpenTM2 - Help
// Help:       This does not affect the functionality of TM/2,\n
//             you may continue working.

#define ERROR_EXPORT_INT                                        1083
//* Define statement owner: cv
// Text:       Preparation of the documents failed.\n
//             Copy/move or drag/drop operation is canceled.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!


#define ERROR_IMPORT_INT                                        1084
//* Define statement owner: cv
// Text:       Creation of the documents failed.\n
//             Copy/move or drag/drop operation is canceled.\n
//             Please try again, clipboard data is still available\n
//             using popup menu.
// HelpTitle:  OpenTM2 - Help
// Help:       !! No help text defined !!

#define ERROR__DELDOC_INVALID_DOC                                1085
// Text:       The document '%1' does not exist and could not be removed.
//* Displayed Buttons: YES NO */
// HelpTitle:  OpenTM2 - Help
// Help:       The referenced document does not exist in the current 
//             folder. Either the wrong folder has been selected for the
//             remove documents function, or the document has already
//             been removed or the wrong file containing the document
//             names has been used.\n
//             Select the correct folder, the correct file or adjust
//             the contents of the list file and repeat the function.

#define ERROR_MEMORY_EXISTS                                    1086
//* Define statement owner: DOE, LEE
// Text:       A Translation Memory with the name '%1' exists already.
// HelpTitle:  OpenTM2 - Help
// Help:       If you want to create the Translation Memory, specify
//             another name.

#define ERROR_LIST_FILE_ACCESS                                 1087
// Text:       Could not access the file '%1' containing the list of files names.
// HelpTitle:  OpenTM2 - Help
// Help:       The specified file containing the list of file names does
//             not exist or could not be opened.

#define ERROR_PLUGIN_EXPIRED                                   1088
// Text:       Some plugin has expired.  Please reference the log file for details.
// HelpTitle:  OpenTM2 - Help
// Help:       Some plugin has expired.  It cannot be loaded by OpenTM2, and 
//             plugin cannot be used.

#define ERROR_PLUGIN_INVALID                                   1089
// Text:       OpenTM2 needs at least a memory plugin, a dictionary plugin and a markup table plugin. \n The plugin(s) 
//             '%1' is (are) missing. Please open "Utilities" -> "Plugin Mananger" and install the missing plugin(s).
// HelpTitle:  OpenTM2 - Help
// Help:       The tool need at least a memory plugin, a dictionary plugin and a markup table plugin, or the 
//             tool cannot work.