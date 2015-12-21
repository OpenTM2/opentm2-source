//+----------------------------------------------------------------------------+
//|OtmAutoVerUpStr.h     OTM  Plugin Manager Parser function                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:             Flora Lee                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:        This is module contains some functions which are used   |
//|                    during plugin manager parser                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

// Message
#define AUTO_VER_UP_NAME_STR                          "Auto Version Update"
#define OPENTM2_APP_NAME_STR                          "OpenTM2"
#define AVT_TAB_NAME_1_STR                            "Updates"
#define AVT_TAB_NAME_2_STR                            "Settings"
#define AVT_LST_COLUMN_1_STR                          "Component"
#define AVT_LST_COLUMN_2_STR                          "Current Version"
#define AVT_LST_COLUMN_3_STR                          "Available Version"
#define AVT_LST_COLUMN_4_STR                          "Available Date"
#define AVT_LST_COLUMN_5_STR                          "Description"
#define AVT_LST_COLUMN_6_STR                          "Severity"
#define AVT_LST_COLUMN_7_STR                          "Impact on OpenTM2 Assets"
#define AVT_LST_COLUMN_8_STR                          "Action to be Taken After Installation"
#define TITLE_SET_KEEK_PKG_STR                        "Keep the downloaded package in \\OTM\\Downloads"

// Info
#define DOWNLOAD_STR                                  "Downloading %s"
#define PROGRESS_START_STR                            "Current=%d, Total=%d Start..."
#define PROGRESS_END_STR                              "Current=%d, Total=%d End."
#define INFO_NO_NEW_VERSION_STR                       "There is no new updates available. Do you want to continue to open this panel?"
#define INFO_FOUND_NEW_VERSION_STR                    "OpenTM2 updates are available.\r\n\r\nWould you like to open the Auto Version Update window?"
#define INFO_SAVE_SUCCESS_STR                         "Save successfully"
#define INFO_UPDATE_SUCCESS_STR                       "Update component(s) succeed and do you want to restart the tool?"
#define INFO_OPENTM2_OPEN_STR                         "The new version of OpenTM2 cannot be installed while OpenTM2 is running.\r\nDo you want to close it now?"
#define INFO_OPENTM2_CLOSE_CONFIRM_STR                "Has OpenTM2 closed?"
#define INFO_OPENTM2_CLOSE_RETRY_STR                  "OpenTM2 is still active.\r\n\r\nShould the shutting down of OpenTM2 be retried or should it be cancelled?"
#define INFO_CONNECT_SUCCESS_STR                      "Test connection successfully."
#define INFO_NEW_VERSION_FOUND_STR                    "There are new or updated components available.  Please select the \"Updates\" tab."
#define INFO_NO_NEW_FOUND_STR                         "There are no new or updated components available."
#define INFO_SETTING_CHANGED_STR                      "It was detected that some setting has changed, please click Yes to save and continue, \r\nor No to abandon this operation."
#define INFO_FREQUENCY_CHANGED_STR                    "Save successfully.\r\n\r\nIt was detected that the frequency setting has changed, do you want to check for updates right now?"
#define INFO_NEED_RESTART_STR                         "The update required restarting OpenTM2, do you want to restart it?"

// Warning

// Error
#define ERROR_NOT_DEFINED_A_STR                       "Undefined error found, please contact with the administrator." // 0
#define ERROR_WRONG_PARAM_A_STR                       "Wrong parameter." // 1001
#define ERROR_MALLOC_SIZE_A_STR                       "Failed to malloc the size for the string." // 1002
#define ERROR_OTM_CREATE_FOLDER_A_STR                 "Failed to create the folder."// 1003
#define ERROR_OTM_FILE_NOT_FIND_A_STR                 "Failed to find the file." // 1004
#define ERROR_OTM_OPEN_FILE_A_STR                     "Error occurred when open the file." // 1005
#define ERROR_OTM_READ_FILE_A_STR                     "Read file error." // 1006
#define ERROR_OTM_REMOVE_FILE_B_STR                   "Cannot remove file %s." // 1007
#define ERROR_OTM_REMOVE_FOLDER_B_STR                 "Cannot remove folders %s." // 1008
#define ERROR_EMPTY_FILE_A_STR                        "The file you opened should not be empty, please check." // 1009
#define ERROR_UNZIP_FILE_A_STR                        "Cannot unzip the the package." // 1010
#define ERROR_OPEN_UNZIP_FILE_A_STR                   "Cannot open the download unzip file." // 1011
#define ERROR_WRITE_UNZIP_FILE_A_STR                  "Cannot create unzip file." // 1012
#define ERROR_VERSION_DIFFER_A_STR                    "Wrong version." // 1013
#define ERROR_CREATE_THREAD_A_STR                     "Create thread failed." // 1014
#define ERROR_CURL_INITIAL_A_STR                      "Initial http curl failed." // 1015
#define ERROR_CURL_SETOPT_A_STR                       "Set option for http curl failed." // 1016
#define ERROR_HTTP_DOWNLOAD_A_STR                     "Cannot get the http download file." // 1017
#define ERROR_HTTPS_DOWNLOAD_A_STR                    "Failed to download file from HTTP." // 1018
#define ERROR_SFTP_DOWNLOAD_A_STR                     "Failed to download file from SFTP." // 1019
#define ERROR_OTM_XML_PARSER_A_STR                    "Failed to parse the xml file." // 1020
#define ERROR_CANNOT_FIND_KEY_C_STR                   "The local xml config %s cannot find key %s." // 1021
#define ERROR_OTM_XERCESC_INITIAL_A_STR               "Initial xercesc failed." // 1022
#define ERROR_CANNOT_GET_MODULE_NAME_A_STR            "Cannot get the download module name from the xml file." // 1023
#define ERROR_HTTPS_CONNECT_A_STR                     "Connect to HTTP server failed." // 1024
#define ERROR_SFTP_CONNECT_A_STR                      "Connect to SFTP server failed." // 1025
#define ERROR_AUTO_VER_UP_A_STR                       "Error occurred when doing auto version up." // 3001
#define ERROR_GET_COMP_MODULE_NAME_A_STR              "Cannot get the download module name from the xml file, please contact with the admin." // 3002
#define ERROR_NOT_INSTALL_ALONE_B_STR                 "For %s, it only allowed one version to be upgraded at a time, please select only one version." // 3003
#define ERROR_CANNOT_GET_VERSION_A_STR                "Cannot get the version info of OpenTM2, please start from OpenTM2!" // 3004
#define ERROR_CANNOT_GET_REGISTRY_INFO_A_STR          "Could not access OpenTM2 registry information!" // 3005
#define ERROR_DL_NOT_ZIP_A_STR                        "Current OpenTM2 not support not zip mode download file."
#define ERROR_FIXPACK_PARENT_NOT_CHECKED              "Fixpack cannot be installed before before the new version, please check."

#define ERROR_INSTALL_COMPONENT_STR                   "Install compnent %s failed."
#define ERROR_UPDATE_FAILED_STR                       "Update component(s) failed."
