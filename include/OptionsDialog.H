//------------------------------------------------------------------------------
//  OptionsDialog.H - Public header file the options dialog interface
//------------------------------------------------------------------------------
//  Copyright Notice:                                                         
//                                                                            
//      Copyright (C) 1990-2012, International Business Machines              
//      Corporation and others. All rights reserved                           
//------------------------------------------------------------------------------

/*! \brief Callback function called by the options dialog to verify the input data

    This callback function is called by the options dialog to verify the data typed-in
    by the user. The function should return TRUE when the input data is correct and FALSE
    when the input data is not correct. When returning FALSE the message text buffer has to be filled
    and the field index field should indicate which data field is in error

    \param pvData pointer to a vector containing the input data per option field
    \param piIndex index of the field in error
    \param pszErrorBuffer pointer to a buffer for the error text
    \param iBufSize size of the error buffer in number of characters
    \param lHandle handle iBufSize size of the error buffer in number of characters
    \param pfOK pointer to callers OK flag

  	\returns TRUW when input data is correct and FALSE when the user has to correct the data
*/

typedef BOOL (APIENTRY *PFN_OPTIONSDIALOGCHECKDATA)
( 
  std::vector<std::string> *pvData,
  int *piIndex,
  char *pszErrorBuffer,
  int iBufSize,
  long lHandle,
  BOOL *pfOK
);


/*! \brief Shows a dialog window for the input of options

    The option dialog presents input fields for the input of option data.
    The number of option fields, the initial values, and the field names are
    defined by the caller. 

    \param hwndOwner Window handle of the window owning the option dialog
    \param sID ID to be used for the option dialog
    \param Label pointer to a vector containing the strings for the option field names
    \param Data pointer to a vector containing the option strings and (on exit) receives the user input
    \param Descr pointer to a vector containing the strings for the option field descriptions
    \param strTitle options dialog title bar text
    \param pfnCheckData pointer to a callback function checking the user input or NULL
    \param lHandle handle passed to pfnCheckData callback
  	\returns TRUE when user input data is correct and FALSE when the user has to correct the data
*/
__declspec(dllexport) BOOL UtlOptionsDlg
(
  HWND              hwndOwner,        // owner window for this dialog box
  SHORT             sID,              // ID to be used for options dialog
  std::vector<std::string> *pvLabel,  // Vector with labels for option fields 
  std::vector<std::string> *pvData,   // Vector with data for option fields  (on return filled with user supplied data)
  std::vector<std::string> *pvDescr,  // Vector with descriptions for option fields
  std::string strTitle,               // title bar text  
  PFN_OPTIONSDIALOGCHECKDATA pfnCheckData, // input data checking callback
  long    lHandle                    // handle passed to the callback function
);
