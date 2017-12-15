//+----------------------------------------------------------------------------+
//|CommStr.h     OTM  Plugin Manager function                                  |
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

#pragma once

#define MAX_TIMESTAMP_LEN                             100
#define MAX_LEN_MSG                                   20000

#define ERROR_CANNOT_GET_REGISTRY_INFO                10001
#define ERROR_CANNOT_GET_APP_PATH                     10002
#define ERROR_CANNOT_INITIAL_OTM                      10003
#define ERROR_GET_REG_VALUE                           10004
#define ERROR_NO_MORE_SPACE                           10005

#define MAX_OTM_VER_LEN                               80

#define LINE_BREAK_STR                                "\n"

#define GET_TOOL_INFO_NAME                            "OtmGetToolInfo"

#define KEY_ENVIRONMENT                               "Environment"
#define ENV_PATH_STR                                  "PATH"
#define ENV_LOCPATH_STR                               "LOCPATH"

#define WIN_NT_STR                                    "Microsoft Windows NT 4.0"
#define WIN_95_STR                                    "Microsoft Windows 95"
#define WIN_98_STR                                    "Microsoft Windows 98"
#define WIN_ME_STR                                    "Microsoft Windows Me"
#define WIN_2000_STR                                  "Microsoft Windows 2000"
#define WIN_XP_STR                                    "Microsoft Windows XP"
#define WIN_XP_64_STR                                 "Microsoft Windows XP Professional x64 Edition"
#define WIN_2003_STR                                  "Microsoft Windows Server 2003"
#define WIN_2003_R2_STR                               "Microsoft Windows Server 2003 R2"
#define WIN_VISTA_STR                                 "Microsoft Windows Vista"
#define WIN_2008_SEV_STR                              "Microsoft Windows Server 2008"
#define WIN_2007_STR                                  "Microsoft Windows 7"
#define WIN_2008_R2_STR                               "Microsoft Windows Server 2008 R2"
#define UNKNOW_SYS_STR                                "Unknow system"

#define SYS_TYPE_64_STR                               "64-bit Operating System"
#define SYS_TYPE_32_STR                               "32-bit Operating System"

#define REG_KEY_LOCAL_MACHINE_STR                     "HKEY_LOCAL_MACHINE"
#define REG_KEY_CURRENT_USER_STR                      "HKEY_CURRENT_USER"
#define REG_KEY_WOW_6432_NODE_STR                     "Wow6432Node"
#define REG_KEY_SYS_ENV                               "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"
#define REG_KEY_SOFTWARE_STR                          "SOFTWARE"
#define REG_KEY_UNINSATALL_STR                        "Microsoft\\Windows\\CurrentVersion\\Uninstall"
#define REG_KEY_VIRTUAL_STR                           "Classes\\VirtualStore\\MACHINE"
#define REG_KEY_DEFAULT_STR                           "(default)"

#define APP_GET_TOOL_INFO_STR                         "Get Tool Info"

#define CMD_HEAD_1                                    "%s: List all OpenTM2-related system settings and information. The output is saved to %s\n"
#define CMD_HEAD_2                                    "%s.exe    : List vital OpenTM2-related environment settings.\n"
#define CMD_HEAD_3                                    "Version               : %s\n"
#define CMD_HEAD_4                                    "Copyright             : 1990-2014, International Business Machines Corporation and others. All rights reserved.\n"
#define CMD_HEAD_5                                    "Purpose               : List all OpenTM2-related system information\n"
#define CMD_HEAD_6                                    "Syntax format         : %s [/CMD] [/UI] [/DETAILS]\n"
#define CMD_HEAD_7                                    "Options and parameters: \n"
#define CMD_HEAD_8                                    "    /CMD        Show the tool by command mode.(DEFAULT) \n"
#define CMD_HEAD_9                                    "    /UI         Show the tool by UI mode. \n"
#define CMD_HEAD_10                                   "    /DETAILS    Show the details of the PLUGINS-directory. \n"

#define RPT_HEAD_1                                    "* ================================================================================================ *\n"
#define RPT_HEAD_2                                    "* == Report created by       : %s.exe                                               == *\n"
#define RPT_HEAD_3                                    "* == Purpose of report       : List vital OpenTM2-related environment settings                  == *\n"
#define RPT_HEAD_4                                    "* == Date and time of report : %s                                             == *\n"
#define RPT_HEAD_5                                    "* == Location of report      : %s                                                       == *\n"
#define RPT_HEAD_6                                    "* == Options                 : %-10s                                                       == *\n"

#define FIRST_PATH_TITLE_STR                          "[Part 1] Windows Operating System related information:\n"
#define SECOND_PATH_TITLE_STR                         "[Part 2] Network and hardware/software related information:\n"
#define THREE_PATH_TITLE_STR                          "[Part 3] OpenTM2 related information:\n"

#define WIN_INFO_FORMAT_STR                           "   1.1 Windows Version: %s\n"
#define SYS_ENV_TITLE_STR                             "   1.2 System Environment:\n"
#define USR_ENV_TITLE_STR                             "   1.3 User Environment:\n"
#define REG_INFO_TITLE_STR                            "   1.4 All registry entries of OpenTM2:\n"
#define DRIVE_INFO_TITLE_STR                          "   2.1 Drive(s):\n"
#define LAN_INFO_TITLE_STR                            "   2.2 Lan Drive(s): %s\n"
#define MEM_INFO_TITLE_STR                            "   2.3 Memory Status:\n"
#define OTM_VER_FORMAT_STR                            "   3.1 OpenTM2 Version: %s\n"
#define PLUGIN_INFO_STR                               "   3.2 OpenTM2 plugins: \n"
#define WIN_FLD_TITLE_STR                             "   3.3 Listing of the files contained in %s:\n"
#define PLUGIN_FLD_TITLE_STR                          "   3.4 Listing of the files contained in %s:\n"

#define DRIVE_SIZE_FORMAT_STR                         "     - %c: Total MB = %-9.0f, Free MB = %-9.0f, Access time = %lu [ms]\n"
#define DRIVE_NOSIZE_AVAIL_STR                        "     - %c: No size info available, Access time = %lu [ms]\n"
#define DRIVE_LIST_TIME_FORMAT_STR                    "     - Time needed to get the list of available drives = %lu [ms]\n"
#define SYS_TYPE_STR                                  "       System Type    : %s\n"
#define PATH_FORMAT_STR                               "       PATH = %s\n"
#define LOCPATH_FORMAT_STR                            "       LOCPATH = %s\n"

#define INSTALLED_PHY_MEM_STR                         "     - Installed Physical Memory(RAM): %9.1f (GB)\n"
#define TOTAL_PHY_MEM_STR                             "     - Total Physical Memory:          %9.1f (GB)\n"
#define AVAIL_PHY_MEM_STR                             "     - Available Physical Memory:      %9.1f (GB)\n"
#define TOTAL_VIR_MEM_STR                             "     - Total Virtual Memory:           %9.1f (GB)\n"
#define AVAIL_VIR_MEM_STR                             "     - Available Virtual Memory:       %9.1f (GB)\n"

#define ERROR_WRONG_PARAM                             "Some of the parameter you input is wrong, please check."
#define ERROR_CANNOT_GET_REGISTRY_INFO_STR            "Could not access OpenTM2 registry information!" // 3005
#define ERROR_CANNOT_GET_APP_PATH_STR                 "Failed to get the path of the application."
#define ERROR_CANNOT_INITIAL_OTM_STR                  "Failed to initilize the OpenTM2."

#define COMPLETE_SUCCESS_STR                          "The OpenTM2 related information was successfully collected, and the report %s was stored to %s.\n"
#define COMPLETE_FAIL_STR                             "Collect the OpenTM2 related information with Failure(%d).\n"

#define ERROR_INFO_MISSING_STR                        " ERROR==>   Missing %s entries.\n"
