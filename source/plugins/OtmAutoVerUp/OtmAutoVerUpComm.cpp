//+----------------------------------------------------------------------------+
//|OtmAutoVerUpComm.cpp     OTM  Plugin Manager Parser function                |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
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

#include "OtmAutoVerUpComm.h"

char * OtmGetMessageFromCode(int nCode)
{
    switch (nCode)
    {
    case ERROR_WRONG_PARAM_A:                            // 1001
        return ERROR_WRONG_PARAM_A_STR;
    case ERROR_MALLOC_SIZE_A:                            // 1002
        return ERROR_MALLOC_SIZE_A_STR;
    case ERROR_OTM_CREATE_FOLDER_A:                      // 1003
        return ERROR_OTM_CREATE_FOLDER_A_STR;
    case ERROR_OTM_FILE_NOT_FIND_A:                      // 1004
        return ERROR_OTM_FILE_NOT_FIND_A_STR;
    case ERROR_OTM_OPEN_FILE_A:                          // 1005
        return ERROR_OTM_OPEN_FILE_A_STR;
    case ERROR_OTM_READ_FILE_A:                          // 1006
        return ERROR_OTM_READ_FILE_A_STR;
    case ERROR_OTM_REMOVE_FILE_B:                        // 1007
        return ERROR_OTM_REMOVE_FILE_B_STR;
    case ERROR_OTM_REMOVE_FOLDER_B:                      // 1008
        return ERROR_OTM_REMOVE_FOLDER_B_STR;
    case ERROR_EMPTY_FILE_A:                             // 1009
        return ERROR_EMPTY_FILE_A_STR;
    case ERROR_UNZIP_FILE_A:                             // 1010
        return ERROR_UNZIP_FILE_A_STR;
    case ERROR_OPEN_UNZIP_FILE_A:                        // 1011
        return ERROR_OPEN_UNZIP_FILE_A_STR;
    case ERROR_WRITE_UNZIP_FILE_A:                       // 1012
        return ERROR_WRITE_UNZIP_FILE_A_STR;
    case ERROR_VERSION_DIFFER_A:                         // 1013
        return ERROR_VERSION_DIFFER_A_STR;
    case ERROR_CREATE_THREAD_A:                          // 1014
        return ERROR_CREATE_THREAD_A_STR;
    case ERROR_CURL_INITIAL_A:                           // 1015
        return ERROR_CURL_INITIAL_A_STR;
    case ERROR_CURL_SETOPT_A:                            // 1016
        return ERROR_CURL_SETOPT_A_STR;
    case ERROR_HTTP_DOWNLOAD_A:                          // 1017
        return ERROR_HTTP_DOWNLOAD_A_STR;
    case ERROR_HTTPS_DOWNLOAD_A:                         // 1018
        return ERROR_HTTPS_DOWNLOAD_A_STR;
    case ERROR_SFTP_DOWNLOAD_A:                          // 1019
        return ERROR_SFTP_DOWNLOAD_A_STR;
    case ERROR_OTM_XML_PARSER_A:                         // 1020
        return ERROR_OTM_XML_PARSER_A_STR;
    case ERROR_CANNOT_FIND_KEY_C:                        // 1021
        return ERROR_CANNOT_FIND_KEY_C_STR;
    case ERROR_OTM_XERCESC_INITIAL_A:                    // 1022
        return ERROR_OTM_XERCESC_INITIAL_A_STR;
    case ERROR_CANNOT_GET_MODULE_NAME_A:                 // 1023
        return ERROR_CANNOT_GET_MODULE_NAME_A_STR;
    case ERROR_HTTPS_CONNECT_A:                          // 1024
        return ERROR_HTTPS_CONNECT_A_STR;
    case ERROR_SFTP_CONNECT_A:                           // 1025
        return ERROR_SFTP_CONNECT_A_STR;
    case ERROR_AUTO_VER_UP_A:                            // 3001
        return ERROR_AUTO_VER_UP_A_STR;
    case ERROR_GET_COMP_MODULE_NAME_A:                   // 3002
        return ERROR_GET_COMP_MODULE_NAME_A_STR;
    case ERROR_NOT_INSTALL_ALONE_B:                      // 3003
        return ERROR_NOT_INSTALL_ALONE_B_STR;
    default:
        return ERROR_NOT_DEFINED_A_STR;
    }
}
