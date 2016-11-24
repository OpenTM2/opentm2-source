#!/usr/bin/python
#! \file
# Description:
#
# Copyright Notice:
#	Copyright (C) 1990-2016, International Business Machines
#   Corporation and others. All rights reserved
#

import sys, traceback
import xml.dom.minidom as minidom

class BasePart(object):
    """base class of Command or Parameter"""
    def __init__(self, name, description):
        self.name = name.strip()
        self.id = self.name.lower()
        self.description = description.strip()

    def setAttributes(self, el):
        el.setAttribute("name", self.name)
        el.setAttribute("id", self.id)
        el.setAttribute("description", self.description)

class Command(BasePart):
    """Represents a command"""
    def __init__(self, command, description, parameters):
        super(Command, self).__init__(command, description)
        self.command = command.strip();
        self.tagname = "command"
        self.parameters = parameters
        
    def toXmlElement(self, doc):
        el = doc.createElement(self.tagname)
        self.setAttributes(el)
        for p in self.parameters:
	    if p.type != "hsession":
		pEl = p.toXmlElement(doc)
		el.appendChild(pEl)
        return el
        
    def setAttributes(self, el):
        super(Command, self).setAttributes(el)
        el.setAttribute("command", self.command)

class Parameter(BasePart):
    """Represents a parameter"""
    def __init__(self, type, name, description, optional):
        super(Parameter, self).__init__(name, description)        
        self.optional = optional
        self.type = type
        self.tagname = "parameter"
        
    def appendDescription(self, str):
        self.description = "\n".join((self.description, str))
        
    def toXmlElement(self, doc):
        el = doc.createElement(self.tagname)
        self.setAttributes(el)
        return el
        
    def setAttributes(self, el):
        super(Parameter, self).setAttributes(el)
        if self.optional:
            el.setAttribute("optional", "true")
        if self.type != "psz":
	    el.setAttribute("type", self.type)
   
def parseScripterBlock(file):
     line = file.readline()
     commands = []
     cmdDesc = []
     while line:
         line = line.strip()
         if line.startswith("/*@ADDTOSCRIPTER*/"):
            #ignore __declspec(dllexport) line
            line = file.readline()
            c = parseHeader(line, cmdDesc, file)
            if c is not None:
                commands.append(c)

         line = file.readline()
     return commands
     
def parseHeader(line, cmdDesc, file):
    line = file.readline()
    while line:
        line = line.strip()
        
        if line.lower().startswith("ushort eqf") or\
           line.lower().startswith("unsigned short eqf"):
            
            if line.lower().startswith("ushort eqf"):
                c = parseCommand(line[6:], cmdDesc, file)
                
            elif line.lower().startswith("unsigned short eqf"):
                c = parseCommand(line[14:], cmdDesc, file)
                
            if c:
                return c
  
        line = file.readline()
    return None

def parseCommand(line, comments, file):
    #get command, remove "ushort " (=6 letters) at beginning
    #cmd = line[6:].strip()
    cmd = line[:].strip()
    line = file.readline()
    if not line:
        return None
    line = line.strip()
    if line != "(":
        print "Malformed command starting with \"%s\"" % line
        return None
    line = file.readline()
    paras = []
    while line:
        line = line.strip()
        if line[:2] == ");":
            break
        elif line[:2] == "//":
            paras[-1].appendDescription(line[2:].strip())
        else:
            p = parseParameter(line)
            if p:
                paras.append(p)
        line = file.readline()
    return Command(cmd, '\n'.join(comments), paras)
        
def parseParameter(line):
    parts = line.split(None, 2)
    if len(parts) < 2:
        print "Malformed line \"%s\"" % line
        return None
    name = parts[1][0:-1] if parts[1][-1] == "," else parts[1]
    comment = ["maned", ""]
    if len(parts) > 2 and parts[2][:2] == "//":
        if parts[2][2:].strip().lower().find("@ignore") != -1:
            return None
        
        #### For @API and @Scripter Begin
        desc = parts[2][2:]
        api_index = parts[2].strip().lower().find('@api:')
        scrip_index = parts[2].strip().lower().find('@scripter:')
        if api_index!=-1 and scrip_index!=-1:
            if api_index < scrip_index:
                desc = ''.join([ parts[2][2:api_index].rstrip(),
                                  parts[2][scrip_index+len('@scripter:'):].lstrip()
                                 ]  )         
            else:
                desc = ''.join([ parts[2][2:scrip_index].rstrip(),  
                                  parts[2][scrip_index+len('@scripter:'):api_index].lstrip()
                                 ]  )  
        elif api_index!=-1:
            return None       
        elif scrip_index!=-1:
            desc = ''.join([ parts[2][2:scrip_index].rstrip(),  
                             parts[2][scrip_index+len('@scripter:'):].lstrip()  
                            ]  )  
        ### For @API and @Scripter End
        
        comment = desc.strip().split(":", 1)
        if len(comment) < 2:
            print "Malformed comment \"%s\"" % comment
            return None
    
    optional = True if comment[0] == "opt" else False
    return Parameter(parts[0].lower(), name, comment[1], optional)

def createDom(doc, cmds):
    api = doc.createElement("api")
    for c in cmds:
        cel = c.toXmlElement(doc)
        api.appendChild(cel)
    doc.documentElement.appendChild(api)

def removeOldApiBlock(doc):
    root = doc.documentElement
    apiEls = root.getElementsByTagName("api")
    for e in apiEls: #remove old
        root.removeChild(e)
        
def removeTextNodes(mainnode):        
    skey = lambda x: getattr(x, "tagName", None)
    mainnode.childNodes = sorted([removeTextNodes(n) for n in mainnode.childNodes if n.nodeType != n.TEXT_NODE],
      cmp=lambda x, y: cmp(skey(y), skey(x)))
    return mainnode
      

def getOpenTM2Version():
    fin = open(r'..\include\EQFSERNO.H','r')
    for line in fin:
        if line.find('#define STR_DRIVER_LEVEL_NUMBER') != -1:
            items = line.strip('\n').split(' ')
            totalItems = len(items)
            if totalItems<3:
			    continue
            version = items[totalItems-1].strip('"')
            print version
            fout = open(r'..\openTM2ScripterGUI\resources\OpenTM2Version.info','w')
            fout.writelines(version)
            fout.close()
    fin.close()
	
if __name__ == "__main__":
    # get OpenTM2 version
    getOpenTM2Version()
	# run parse commands
    if len(sys.argv) < 3:
        print "Usage: %s [COMMANDS_XML] [HEADER_FILE]" % sys.argv[0]
        sys.exit()
    
    try:
        header = open(sys.argv[2])
        cmdDoc = minidom.parse(sys.argv[1])
        
        cmds = parseScripterBlock(header)
        removeOldApiBlock(cmdDoc)
        createDom(cmdDoc, cmds)
        writer = open(sys.argv[1], 'w')
        removeTextNodes(cmdDoc.documentElement) #clears newlines and blanks
        writer.write(cmdDoc.documentElement.toprettyxml("  ", "\n", "utf-8"))
        writer.close()
        header.close()
        print "commands.xml build done."
    except:
        print "Couldn't read file"
        traceback.print_exc(file=sys.stdout)
