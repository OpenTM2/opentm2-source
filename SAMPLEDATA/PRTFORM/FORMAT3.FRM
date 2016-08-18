<comment>
  This format information file is designed to be formatted with BookMaster
  before you print it. If you print it without formatting, you will see
  only the tags in the file, and not the result they produce.
  This file prints all of the templates for each dictionary entry.
  The BookMaster tags are designed to make your output resemble
  dictionary entries.
  Supply the information below for "Author's Name" and "Address".
</comment>


<description>Format for SAMPLE1 and SAMPLE2</description>

<dictfront>
:userdoc.
:prolog.
:docprof hyphen=yes headnum=no.
:title.
Extract from the dictionary <var sysname=$DICTNAME>
:etitle.
:date.
<var sysname=$DATE format=dd.mm.yyyy>
:author.
"Author's Name"
:address.
"Address"
:eaddress.
:eprolog.
:frontm.
:tipage.

.*================== BEGIN BODY ========================================
.*======================================================================
:body.

</dictfront>

<entryprt>
.**************************************
:h2.<var name=Headword>
The term :hp1.<var name=Headword>:ehp1. has the following meanings:
:ul.
<repeat name=Headword>
:li.<var name=Translation>
</repeat>
:eul.
</entryprt>

<dictback>
.*================== END BODY ====================
:backm.
:euserdoc.
</dictback>
