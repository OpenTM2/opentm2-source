//+----------------------------------------------------------------------------+
//|EQFPLGMG.CPP     OTM  Plugin Manager Parser function                        |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
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
#pragma once

#define DOWNLOAD_STR                                  "Downloading %s"
#define PROGRESS_START_STR                            "Current=%d, Total=%d Start..."
#define PROGRESS_END_STR                              "Current=%d, Total=%d End."
#define PM_TAB_NAME_1_STR                             "Installed"
#define PM_TAB_NAME_2_STR                             "Available"
#define PM_TAB_NAME_3_STR                             "Updates"
#define PM_TAB_NAME_4_STR                             "Settings"
#define PM_TAB_NAME_5_STR                             "Help"
#define INSTALL_LST_COLUMN_1_STR                      "Plugin"
#define INSTALL_LST_COLUMN_2_STR                      "Type"
#define INSTALL_LST_COLUMN_3_STR                      "Installed Version"
#define INSTALL_LST_COLUMN_4_STR                      "Updates?"
#define INSTALL_LST_COLUMN_5_STR                      "Description"
#define AVAILBLE_LST_COLUMN_1_STR                     "Plugin"
#define AVAILBLE_LST_COLUMN_2_STR                     "Type"
#define AVAILBLE_LST_COLUMN_3_STR                     "Available Version"
#define AVAILBLE_LST_COLUMN_4_STR                     "Available Date"
#define AVAILBLE_LST_COLUMN_5_STR                     "Description"
#define AVAILBLE_LST_COLUMN_6_STR                     "Severity"
#define AVAILBLE_LST_COLUMN_7_STR                     "Impact on OpenTM2 Assets"
#define AVAILBLE_LST_COLUMN_8_STR                     "Action to be Taken After Installation"
#define UPDATE_LST_COLUMN_1_STR                       "Plugin"
#define UPDATE_LST_COLUMN_2_STR                       "Type"
#define UPDATE_LST_COLUMN_3_STR                       "Installed Version"
#define UPDATE_LST_COLUMN_4_STR                       "Available Version"
#define UPDATE_LST_COLUMN_5_STR                       "Available Date"
#define UPDATE_LST_COLUMN_6_STR                       "Description"
#define UPDATE_LST_COLUMN_7_STR                       "Severity"
#define UPDATE_LST_COLUMN_8_STR                       "Impact on OpenTM2 Assets"
#define UPDATE_LST_COLUMN_9_STR                       "Action to be Taken After Installation"
#define PLUGIN_TYPE_MEMORY_STR                        "Translation Memory"
#define PLUGIN_TYPE_DICT_STR                          "Dictionary"
#define PLUGIN_TYPE_MARKUP_STR                        "Markup"
#define PLUGIN_TYPE_SHARED_MEMORY_STR                 "Shared Translation Memory"
#define PLUGIN_TYPE_DOCUMENT_STR                      "Document"
#define PLUGIN_TYPE_UNDEFINED_STR                     "Undefined"
#define PLUGIN_TYPE_SPELL_STR                         "Spell Check"
#define PLUGIN_TYPE_MORGH_STR                         "Morphologic Functionality"
#define PLUGIN_TYPE_TOOL_STR                          "Tools"
#define TITLE_SET_KEEK_PKG_STR                        "Keep the downloaded package in \\OTM\\PLUGINS\\Downloads"
#define YES_STR                                       "Yes"

// Info
#define INFO_SAVE_SUCCESS_STR                         "Save successfully"
#define INFO_NEW_VERSION_STR                          "There are new or updated plugins available.\r\n\r\nWould you like to open the Plugin Manager?"
#define INFO_TASK_END_SUCCESS_STR                     "All tasks have been finished successfully."
#define INFO_TASK_END_FAIL_STR                        "All tasks have been finished, but some of them failed, please check."
#define INFO_CONNECT_SUCCESS_STR                      "Test connection successfully."
#define INFO_NEW_VERSION_FOUND_STR                    "There are new or updated plugins available.  Please select the corresponding tab."
#define INFO_NO_NEW_FOUND_STR                         "There are no new or updated plugins available."
#define INFO_SETTING_CHANGED_STR                      "It was detected that some setting has changed, please click Yes to save and continue, \r\nor No to abandon this operation."
#define INFO_FREQUENCY_CHANGED_STR                    "Save successfully.\r\n\r\nIt was detected that the frequency setting has changed, do you want to check for updates right now?"

// Warning
#define WARNING_AVT_OPENED_STR                        "Auto version up dialog has already been opened."
#define WARNING_TRD_PATH_SFW_STR                      "To install %s, you need install the following third part software by yourself as well, otherwise the plugin may not work normally.\r\n%s"
#define WARNING_PEND_UPT_STR                          "The plugin installation cannot be completed while OpenTM2 is running.\r\nPlease manually close OpenTM2 when it is convenient.\r\nThe plugin installation will be completed the next time OpenTM2 is started."
#define WARNING_DELETE_CONFIRM_STR                    "Are you sure you want to delete the selected plugins?"
#define WARNING_AUTO_CHECK_RUNNING_STR                "Auto checking is running now, please try to open the window later."
#define WARNING_NO_ITEM_SELECTED                      "No item is selected, please check."
#define DEPN_STR                                      "%s: download address is %s."

// Error
#define ERROR_NOT_DEFINED_A_STR                       "Undefined error found, please contact with the administrator." // 0
#define ERROR_WRONG_PARAM_A_STR                       "Wrong parameter." // 1001
#define ERROR_MALLOC_SIZE_A_STR                       "Failed to malloc the size for the string." // 1002
#define ERROR_OTM_CREATE_FOLDER_A_STR                 "Failed to create the folder." // 1003
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
#define ERROR_LOAD_ENTRY_POINT_A_STR                  "Load entry point failed."
#define ERROR_WRONG_PLUGIN_NAME_A_STR                 "The plugin name is wrong." // 2001
#define ERROR_GET_PLUGIN_PATH_A_STR                   "Cannot get the path of the plugin." // 2002
#define ERROR_PLUGIN_NOT_FOUND_A_STR                  "Failed to find the plugin." // 2003
#define ERROR_PLUGIN_NO_ARRTI_B_STR                   "Plugin %s has no attribute." // 2004
#define ERROR_UPDATE_PLUGIN_B_STR                     "Failed to update the plugin." // 2005
#define ERROR_DEREGISTER_PLUGIN_B_STR                 "Cannot deregister plugin %s." // 2006
#define ERROR_REMOVE_PLUGIN_B_STR                     "Plugin %s cannot be removed." // 2007
#define ERROR_CANNOT_FIND_PLUGIN_B_STR                "Cannot find the plugin %s." // 2008
#define ERROR_CANNOT_GET_URL_B_STR                    "Install plugin %s failed, cannot get url." // 2009
#define ERROR_EMPTY_FILE                              "The file you opened should not be empty, please check."
#define ERROR_EMPTY_LOC_CONFIG_STR                    "The local xml config %s is empty."
#define ERROR_CANNOT_FIND_ATTRI_STR                   "Cannot find the attribute of the key."
#define ERROR_PLUGIN_UPDATE_FAILED_STR                "Update plugin %s failed, cannot find such plugin.(%d)"
#define ERROR_REMOVE_CRITICAL_PLUGIN_STR              "%s is the basic plugin of OpenTM2, please don't remove them all.\r\nPlease reselected."
#define ERROR_FIXPACK_PARENT_NOT_CHECKED              "Fixpack cannot be installed before the new version, please check."
