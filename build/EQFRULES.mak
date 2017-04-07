#   Copyright Notice:                                                         
#                                                                             
#           Copyright (C) 1990-2015, International Business Machines          
#           Corporation and others. All rights reserved                       
#                                                                             
#                                                                             

#------------------------------------------------------------------------------
# compile and link options                                                    -
#------------------------------------------------------------------------------
.SUFFIXES:  .exe .obj .res .rbj .c .h .id .men .str .dlg .rc .MAK .mdp .def .lib .cpp

#------------------------------------------------------------------------------
# compile statements  for a compile from the BUILDER                          -
#------------------------------------------------------------------------------

{$(_SRC)\api}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\api\$(*B).C
    @echo ---- Compiling $(_SRC)\api\$(*B).C >>$(_ERR)
    @echo ---- _OBJ= $(_OBJ)
    @echo $(_COMPILER)  $(_CL_OPTIONS) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\api\$(*B).c
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\api\$(*B).c

{$(_SRC)\core\analysis}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\core\analysis\$(*B).C
    @echo ---- Compiling $(_SRC)\core\analysis\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\analysis\$(*B).c

{$(_SRC)\core\counting}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\core\counting\$(*B).C
    @echo ---- Compiling $(_SRC)\core\counting\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\counting\$(*B).c

{$(_SRC)\core\dictionary}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\core\dictionary\$(*B).C
    @echo ---- Compiling $(_SRC)\core\dictionary\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\dictionary\$(*B).c

{$(_SRC)\core\dictionary}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\core\dictionary\$(*B).CPP
    @echo ---- Compiling $(_SRC)\core\dictionary\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\dictionary\$(*B).cpp

{$(_SRC)\core\document}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\core\document\$(*B).C
    @echo ---- Compiling $(_SRC)\core\document\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\document\$(*B).c

{$(_SRC)\core\document}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\core\document\$(*B).CPP
    @echo ---- Compiling $(_SRC)\core\document\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\document\$(*B).cpp

{$(_SRC)\core\folder}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\folder\$(*B).c

{$(_SRC)\core\spell}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\  $(_SRC)\core\spell\$(*B).cpp

{$(_SRC)\core\morph}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\  $(_SRC)\core\morph\$(*B).cpp

{$(_SRC)\core\linguistic}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\linguistic\$(*B).cpp

{$(_SRC)\core\linguistic}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\linguistic\$(*B).c

{$(_SRC)\core\lists}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\lists\$(*B).c

{$(_SRC)\core\memory}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\memory\$(*B).c

{$(_SRC)\core\memory}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\  $(_SRC)\core\memory\$(*B).cpp

{$(_SRC)\core\PluginManager}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).Cpp
    @echo ---- Compiling $(_SRC)\$(*B).Cpp >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\PluginManager\$(*B).cpp

{$(_SRC)\core\mt}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\mt\$(*B).c

{$(_SRC)\core\services}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\services\$(*B).c

{$(_SRC)\core\tagtable}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\tagtable\$(*B).c

{$(_SRC)\core\tagtable}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).Cpp
    @echo ---- Compiling $(_SRC)\$(*B).Cpp >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\tagtable\$(*B).cpp

{$(_SRC)\core\utilities}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\utilities\$(*B).c

{$(_SRC)\core\utilities}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\utilities\$(*B).cpp
	
{$(_SRC)\core\workbench}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\workbench\$(*B).c

{$(_SRC)\core\workbench}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\workbench\$(*B).cpp

{$(_SRC)\plugins\spell\OtmSpellHSPlugin}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\spell\OtmSpellHSPlugin\$(*B).cpp

{$(_SRC)\plugins\morph\OtmMorphICUPlugin}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\morph\OtmMorphICUPlugin\$(*B).cpp

{$(_SRC)\plugins\linguistic}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\linguistic\$(*B).cpp

{$(_SRC)\plugins\markup}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\markup\$(*B).c

{$(_SRC)\tools}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_EXE) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\tools\$(*B).c

{$(_SRC)\tools\itm}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\tools\itm\$(*B).c

{$(_SRC)\tools\itm}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\tools\itm\$(*B).cpp

{$(_SRC)\tools\common}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\tools\common\$(*B).c

{$(_SRC)\tools\common}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\tools\common\$(*B).cpp

{$(_SRC)\tools\batch}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\tools\batch\$(*B).c

{$(_SRC)\tools\commandline}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\tools\commandline\$(*B).c

{$(_SRC)\tools\commandline}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\tools\commandline\$(*B).cpp

{$(_SRC)\core\editor}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\editor\$(*B).c

{$(_SRC)\core\editor}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\editorr\$(*B).cpp

{$(_SRC)}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).C
    @echo ---- Compiling $(_SRC)\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\$(*B).c

{$(_SRC)\plugins\memory\EqfMemoryPlugin}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\memory\EqfMemoryPlugin\$(*B).C
    @echo ---- Compiling $(_SRC)\plugins\memory\EqfMemoryPlugin\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\memory\EqfMemoryPlugin\$(*B).c

{$(_SRC)\core\mfc}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).Cpp
    @echo ---- Compiling $(_SRC)\$(*B).Cpp >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\mfc\$(*B).cpp

{$(_SRC)\tools}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).Cpp
    @echo ---- Compiling $(_SRC)\$(*B).Cpp >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\tools\$(*B).cpp

{$(_SRC)}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).Cpp
    @echo ---- Compiling $(_SRC)\$(*B).Cpp >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\$(*B).cpp

{$(_SRC)\core\counting}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\core\counting\$(*B).CPP
    @echo ---- Compiling $(_SRC)\core\counting\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\counting\$(*B).cpp

{$(_SRC)\core\analysis}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\analysis\$(*B).cpp

{$(_SRC)\plugins\memory}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\memory\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\memory\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\memory\$(*B).cpp

{$(_SRC)\plugins\tools}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\tools\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\tools\$(*B).cpp

{$(_SRC)\plugins\tools\OtmCleanupPlugin}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmCleanupPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmCleanupPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmCleanupPlugin\$(*B).cpp

{$(_SRC)\plugins\tools\OtmToolsLauncherPlugin}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmToolsLauncherPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmToolsLauncherPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmToolsLauncherPlugin\$(*B).cpp

{$(_SRC)\plugins\tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
{$(_SRC)\plugins\memory\EqfMemoryPlugin}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\memory\EqfMemoryPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\memory\EqfMemoryPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\memory\EqfMemoryPlugin\$(*B).cpp

{$(_SRC)\plugins\dictionary}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\dictionary\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\dictionary\$(*B).CPP >>$(_ERR)
    @echo $(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\dictionary\$(*B).cpp
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\dictionary\$(*B).cpp

{$(_SRC)\plugins\markup\OtmMarkupTablePlugin\}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\markup\OtmMarkupTablePlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\markup\OtmMarkupTablePlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\markup\OtmMarkupTablePlugin\$(*B).cpp

{$(_SRC)\plugins\markup\UserMarkupsPlugin\}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\markup\UserMarkupsPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\markup\UserMarkupsPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\markup\UserMarkupsPlugin\$(*B).cpp

{$(_SRC)\plugins\document}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\document\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\document\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\document\$(*B).cpp

{$(_SRC)\core\folder}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\core\folder\$(*B).CPP
    @echo ---- Compiling $(_SRC)\core\folder\$(*B).CPP >>$(_ERR)
    @echo $(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\folder\$(*B).cpp
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\folder\$(*B).cpp

{$(_SRC)\core\services}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\services\$(*B).cpp

{$(_SRC)\core\editor}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\editor\$(*B).cpp


{$(_DEF)}.DEF{$(_LIB)}.lib:
    @echo ---- Creating import library $(_LIB)\$(*B).LIB
    @echo ---- Creating import library $(_LIB)\$(*B).LIB >>$(_ERR)
    @$(_IMPLIBER) /DEF:$(_DEF)\$(*B).$(_DEFEXT) /MACHINE:IX86 /NOLOGO /OUT:$(_LIB)\$(*B).LIB >>$(_ERR)

{$(_RC)}.rc{$(_RES)}.res:
    @echo ---- Compiling resource $(_RC)\$(*B).RC $(_RC_OPT)
    @echo ---- Compiling resource $(_RC)\$(*B).RC $(_RC_OPT) >>$(_ERR)
    @$(_RC_COMPILER) $(_RC_OPT) -fo$(_RES)\$(*B).RES $(_RC)\$(*B).RC >>$(_ERR)

{$(_RES)}.RES{$(_OBJ)}.RBJ:
    CVTRES /NOLOGO /OUT:$(_OBJ)\$(*B).RBJ $(_RES)\$(*B).RES >>$(_ERR)

