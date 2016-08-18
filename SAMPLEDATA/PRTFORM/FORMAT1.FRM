<comment>
  This format information file is designed to print each headword and
  its translation in the dictionaries SAMPLE1 and SAMPLE2.
</comment>

<description>Format for SAMPLE1 and SAMPLE2</description>

<comment>

     Between <entryprt> and </entryprt> you specify what you want to print.
     The names following "var name=" have to be the same names that are
     defined as entry field names in the dictionary you are printing.
     To see which entry field names are in a dictionary, select 'Properties'
     from the File pulldown.
     The dictionaries SAMPLE1 and SAMPLE2 are defined with the entryfields:
                        Headword
                        Part of Speech
                        Abbrev./Fullform
                        Definition
                        Synonym
                        Other Related Terms
                        Translation
                        Company/Subject Code

 - With the instruction "var name=Headword" the headword will be printed.
 - With the instruction "var name=Translation" the translation for the
   headword is printed. If there is more than one translation, only the first
   translation is printed.
 - The "==========" in the example below is used as a separator line between
   the entries on your printout.

</comment>

<entryprt>
=======================================
<var name='Source term'>
  <var name='NL term'>
</entryprt>
