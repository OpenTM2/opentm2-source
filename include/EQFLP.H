// Copyright (c) 1999-2012, International Business Machines
// Corporation and others.  All rights reserved.
//
//+----------------------------------------------------------------------------+
//|EQFLP.H                                                                     |
//+----------------------------------------------------------------------------+
//|Author: Gerhard Queck (QSoft)                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Include file for list processors                               |
//|                                                                            |
//+----------------------------------------------------------------------------+

/**********************************************************************/
/* Command Values for List Processors                                 */
/**********************************************************************/
typedef enum _LISTPROCCMD
{
   LST_INIT_CMD = 1,                   // do any necessary initialization
   LST_RETTYPENAME_CMD,                // return name of list type processed
   LST_FILLINNAMES_CMD,                // fill listbox with list names
   LST_FILLINDETAILS_CMD,              // fill listbox with list details
   LST_NEW_CMD,                        // create a list
   LST_OPEN_CMD,                       // open a list
   LST_DELETE_CMD,                     // delete a list
   LST_PRINT_CMD,                      // print a list
   LST_EXPORT_CMD,                     // export a list
   LST_IMPORT_CMD,                     // import a list
   LST_LAST_CMD                        // dummy command, indicates last command
} LISTPROCCMD;

/**********************************************************************/
/* Return Values for List Processors                                  */
/**********************************************************************/
typedef enum _LISTPROCRC
{
   LST_OK_RC,                          // function completed successfully
   LST_CMD_FAILED,                     // execution of command failed
   LST_CMD_NOT_PROCESSED_RC            // command has not been processed
} LISTPROCRC;

#define THAI_STR    "Thai"     //@@ for now here
