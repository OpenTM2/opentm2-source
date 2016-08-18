/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef OtmMorphPlugin_h__
#define OtmMorphPlugin_h__

#include "OtmPlugin.h"
#include "OtmMorph.h"

/*! \brief Abstract base-class for plugins handling morphology
*/
class __declspec(dllexport) OtmMorphPlugin: public OtmPlugin
{
public:

	/*! \brief constructor	 */
	OtmMorphPlugin(void) 
	{
		pluginType = eMorphType;
	};

	/*! \brief destructor	 */
	~OtmMorphPlugin(void) {};

	
	/*! \brief check if the plug-in is usable.	 */
	virtual bool isUsable()
	{
		return OtmPlugin::isUsable();
	}

	/*! \brief get the name of the plug-in	 */
	virtual const char* getName() = 0;

	/*! \brief get the short description of the plug-in	 */
	virtual const char* getShortDescription() = 0;

	/*! \brief get the long description of the plug-in	 */
	virtual const char* getLongDescription() = 0;

	/*! \brief get the version of the plug-in	 */
	virtual const char* getVersion() = 0;

	/*! \brief get the supplier of the plug-in	 */
	virtual const char* getSupplier() = 0;

	/*! \enum eRc Possible return values of OtmMemory and OtmMemoryPlugin methods	*/
	enum eRc 
	{
		eSuccess = 0,
		eUnknown,
		eNotSupported,
		eNotFound,
		eParameterError,
	};

	/*! \brief open a morph checking object for a specified language
		\param vLanguage pointer to the language name
		\returns pointer to the morph check object or NULL if failed.
	 */
	virtual OtmMorph* openMorph(
		const char* pszLanguage
		) = 0;
	
	/*! \brief structure for morph checking info	 */
	typedef struct _MORPHINFO 
	{
		char szName[50];
		char szDescription[256];
		char szLanguage[50];
		bool bEnabled;
	}MORPHINFO, *PMORPHINFO;

	/*! \brief get the information of the specified language
		\param vLanguage pointer to the language name
		\vInfo pointer to the structure that restores the information
		\returns 0 means success
	 */
	virtual int getMorphInfo(
		const char* pszLanguage, 
		PMORPHINFO pInfo
		) = 0;

	 /*! \brief get the language support list of the plugin
      \ that is supported by the MorphFactory class.
      \param vLanguageList reference to the vector that contains all the languages supported by the MorphFactory class
      \returns 0 means success, other value means error!
   */
	virtual int getLanguageList(
		vector<string>& vLanguageList
		) = 0;

	/*! \brief check if a specific language is supported by this plugin
	  \param vName pointer to the language
	  \returns 1 when the language is supported and 0 if it is not supported
    */
   virtual int isSupported(
	  const char* pszLanguage
	  ) = 0;

};


#endif // OtmMorphPlugin_h__

