/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "core\PluginManager\PluginManager.h"
#include "EqfDictionaryPlugin.h"
#include "EqfDictionary.h"
#ifndef CPPTEST
extern "C" {
#endif
#include "OtmDictionaryIF.h"
#ifndef CPPTEST
}
#endif

// the static plugin infos
static char *pszPluginName = "EqfDictionaryPlugin";
static char *pszShortDescription = "EqfDictionaryPlugin";
static char *pszLongDescription	= "This is the standard (EQF) dictionary implementation";
static char *pszVersion = "1.0";
static char *pszSupplier = "International Business Machines Corporation";


EqfDictionaryPlugin::EqfDictionaryPlugin()
{
	name = pszPluginName;
	shortDesc = pszShortDescription;
	longDesc = pszLongDescription;
	version = pszVersion;
	supplier = pszSupplier;
	pluginType = OtmPlugin::eDictionaryType;
	usableState = OtmPlugin::eUsable;
}


EqfDictionaryPlugin::~EqfDictionaryPlugin()
{
}

const char* EqfDictionaryPlugin::getName()
{
	return name.c_str();
}

const char* EqfDictionaryPlugin::getShortDescription()
{
	return shortDesc.c_str();
}

const char* EqfDictionaryPlugin::getLongDescription()
{
	return longDesc.c_str();
}

const char* EqfDictionaryPlugin::getVersion()
{
	return version.c_str();
}

const char* EqfDictionaryPlugin::getSupplier()
{
	return supplier.c_str();
}

OtmDictionary* EqfDictionaryPlugin::CreateDict()
{
	EqfDictionary* pDictionary = new EqfDictionary();
	return (OtmDictionary*) pDictionary;
}

USHORT  EqfDictionaryPlugin::CPP_AsdBegin(USHORT usMaxDicts, PHUCB phUCB)
{
	return AsdBegin(usMaxDicts, phUCB);
}
USHORT  EqfDictionaryPlugin::CPP_AsdEnd(HUCB hUCB)
{
	return AsdEnd(hUCB);
}
USHORT  EqfDictionaryPlugin::CPP_AsdOpen(HUCB hUCB, USHORT usOpenFlags, USHORT usNumDicts, PSZ *ppszDicts, PHDCB phDCB, PUSHORT pusErrDict)
{
	return AsdOpen(hUCB, usOpenFlags, usNumDicts, ppszDicts, phDCB, pusErrDict);
}
USHORT  EqfDictionaryPlugin::CPP_AsdClose(HUCB hUCB, HDCB hDCB)
{
	return AsdClose(hUCB, hDCB);
}

bool EqfDictionaryPlugin::stopPlugin( bool fForce  )
{

  // TODO: check for active objects..
  bool fActiveObjects = false;

  // decline stop if we have active objects
  if ( !fForce && fActiveObjects )
  {
    return( false );
  }

  // TODO: terminate active objects, cleanup, free allocated resources

  // de-register plugin
	PluginManager *pPluginManager = PluginManager::getInstance();
	pPluginManager->deregisterPlugin( (OtmPlugin *)this );

  return( true );
}

extern "C" {
  __declspec(dllexport)
  USHORT registerPlugins()
  {
	  PluginManager::eRegRc eRc = PluginManager::eSuccess;
	  PluginManager *manager = PluginManager::getInstance();
	  EqfDictionaryPlugin* plugin = new EqfDictionaryPlugin();
	  eRc = manager->registerPlugin((OtmPlugin*) plugin);
      USHORT usRC = (USHORT) eRc;
      return usRC;
  }
}

extern "C" {
  __declspec(dllexport)
  unsigned short getPluginInfo( POTMPLUGININFO pPluginInfo )
  {
    strcpy( pPluginInfo->szName, pszPluginName );
    strcpy( pPluginInfo->szShortDescription, pszShortDescription );
    strcpy( pPluginInfo->szLongDescription, pszLongDescription );
    strcpy( pPluginInfo->szVersion, pszVersion );
    strcpy( pPluginInfo->szSupplier, pszSupplier );
	  pPluginInfo->eType = OtmPlugin::eDictionaryType;
    strcpy( pPluginInfo->szDependencies, "" );
    pPluginInfo->iMinOpenTM2Version = -1;
    return( 0 );
  }
}
