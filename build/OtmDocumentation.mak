# ------------------------------------------------------------------------------
# OtmDocumentation.MAK - Makefile to create OpenTM2 documentation using DoxyGen
# Copyright (c) 2012, International Business Machines
# Corporation and others.  All rights reserved.
# ------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------

build:	$(_DOC)\PluginManager\html\index.html \
		$(_DOC)\PluginManager\PluginManager.chm

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_DOC)\PluginManager\html\index.html:	$(_SRC)\core\PluginManager\OtmPlugin.h \
										$(_SRC)\core\PluginManager\OtmMemoryPlugin.h \
										$(_SRC)\core\PluginManager\OtmMemory.h \
										$(_SRC)\core\PluginManager\OtmDictionaryPlugin.h \
										$(_SRC)\core\PluginManager\OtmDictionary.h \
										$(_SRC)\core\PluginManager\PluginManager.cpp \
										$(_SRC)\core\PluginManager\PluginManager.doxy \
										$(_SRC)\core\PluginManager\PluginManager.h \
										$(_SRC)\core\PluginManager\PluginManagerImpl.cpp \
										$(_SRC)\core\PluginManager\PluginManagerImpl.h
	@echo ---- Running DOXYGEN to generate documentation for PluginManager
	@echo ---- Running DOXYGEN to generate documentation for PluginManager >>$(_ERR)
	doxygen $(_SRC)\core\PluginManager\PluginManager.doxy

# Microsofts Help-Compiler (hhc.exe) exits with RC=1, that's why we use the modifier "-1"
$(_DOC)\PluginManager\PluginManager.chm:	$(_DOC)\PluginManager\html\index.html
	@echo ---- Running HHC (Microsoft help-compiler) to generate compiled documentation for PluginManager
	@echo ---- Running HHC (Microsoft help-compiler) to generate compiled documentation for PluginManager >>$(_ERR)
	-1 hhc $(_DOC)\PluginManager\html\index.hhp
	move $(_DOC)\PluginManager\html\PluginManager.chm $(_DOC)\PluginManager
