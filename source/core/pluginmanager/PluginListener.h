/*! \file 
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#pragma once

#include <vector>
#include "OtmPlugin.h"

//-----------------------------------------------------
// Plugin Listener Class			
//-----------------------------------------------------
class PluginListener
{
public:

/*! \enum eNotifyRegRc
	Possible notifyreturnvalues of registerPlugin()- and deregisterPlugin()-methods.
*/
	enum eNotifcationType
	{
    eStarted,         /*!< a new plugin has been started */
		eAboutToStop,			/*!< plugin is about to be stopped, but it is still active and can process requests */
		eStopped,	      	/*!< plugin has been stopped, plugin methods can't be used anymore */
	};
	virtual ~PluginListener();		// Destructor
  virtual void Notify( eNotifcationType eNotifcation, const char *pszPluginName, OtmPlugin::ePluginType eType, bool fForce );
protected:
//constructor is protected because this class is abstract, it’s only meant to be inherited!
	PluginListener();
private: 
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	PluginListener(const PluginListener& yRef);	
	PluginListener& operator=(const PluginListener& yRef);	
};
