<comment>
  This format information is designed to print all of the templates for each
  entry in a "flat" (unformated) structure. In order to achieve this a single
  <repeat> tag is used.
</comment>

<description>Format for SAMPLE1 and SAMPLE2</description>

<comment>

     Between "entry" and "/entry" you specify what you want to print.
     The names following 'var name=' have to be the same names that are defined
     as entry field names in the dictionary you are printing. To see which
     entry field names are in a dictionary, select 'Properties' in the File
     pulldown.
     The dictionaries SAMPLE1 and SAMPLE2 are defined with the entryfields:
                        Headword
                        Part of Speech
                        Abbrev./Fullform
                        Definition
                        Synonym
                        Other Related Terms
                        Translation
                        Company/Subject Code

     Non-tagged information that you type between the <entryprt> and </entryprt>
     tags is printed for each iteration. In the example below the "====" is
     used as a separator line between the entries on your printout.

 - With the instruction "var name=Headword" the headword is printed.

 - The "repeat" instruction means that the Part of Speech, Abbrev./Fullform
   and the Translation entries are printed until a different headword is found.

 - If there are no entries for Part of Speech or Abbrev./Fullform in the
   dictionary, an empty line is printed.

</comment>

<entryprt>
=======================================
<var name=Headword>
<repeat name=Headword>
  <var name='Part of Speech'>
  <var name=Abbrev./Fullform>
  <var name=Translation>
</repeat>
</entryprt>
