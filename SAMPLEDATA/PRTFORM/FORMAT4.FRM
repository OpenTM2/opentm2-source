<comment>
  This format file prints your dictionary or an extract from your dictionary
in a clearly arranged way.
  A front page informs you about the dictionary printed.
  The part in the middle contains the entries of your dictionary consisting
of the fields you selected. In this example the contents of the entry fields
'Part of Speech, Translation, Synonym, Context and Subject Code' are printed.
  Each page has a header section. In this example the name of the dictionary
and the first term on the page are printed.
  At the bottom of each page the current page number is printed.
</comment>

<description>Print format for Dictprt dictionary</description>


<set sysname=$page_length value='61'>
  <comment>This set command defines the length of the page. Here a
  length of 61 lines per page is chosen.
  </comment>

<set sysname=$page_eject value=''>
  <comment>This set command defines the string that is used to indicate a
  page eject. To learn which sign your printer uses to make a page eject,
  check the appropriate printer manual. Here ASCII 12 (x'0C') is used.
  </comment>

<dictfront>
  <comment>This section defines the layout of the front page.
  </comment>








            Title: Printout of Dictionary <var sysname=$Dictname>
              <comment> A title line is printed and the name of the
              selected dictionary is inserted.
              </comment>

            Date:  <var sysname=$Date format=dd.mm.yyyy>
              <comment> The actual date is printed in the format specified.
              </comment>

            Author: Author's Name
              <comment> Replace the field with your name.
              </comment>

</dictfront>
  <comment>Indicates the end of the front page section.</comment>

<pagehead>
  <comment>Here starts the pagehead section</comment>

<var sysname=$Dictname $Min=45 $Right>
  <comment>This statement specifies where the dictionary name is printed on
  top of the page. Here the $Min and $Right attributes specify that the name
  of the dictionary is written right-justified at column 45.</comment>

<var name=headword $first_on_page>

  <comment>This statement specifies that the first headword on the
  page is printed in the pagehead section.</comment>
===============================================================================

  <comment>This line is printed as a page header separator.</comment>
</pagehead>
  <comment>Here the specifications for the pagehead section end.</comment>
<entryprt>
  <comment>Beginning of the entry section.</comment>

The term "<var name=Headword>" has the following meaning/s:


  <comment>This text is printed and the actual headword is filled in.</comment>
<repeat name=Headword>
  <comment>This specifies that all the fields selected - here they are
  'Part of Speech, Translation, Synonym, Context and Subject Code' - are
  repeated as long as 'Headword' does not change. This means, that all
  the translations of an Headword are printed.</comment>
  Part of Speech: <var name='Part of Speech'>
<repeat name='Part of Speech'>
  <comment>With this statement 'Part of Speech' is printed only if it has
  changed.</comment>
      Translation:    <var name=Translation>
          Synonym:        <var name=Synonym>
          Context:        <var name=Context>
          Subject Code:   <var name='Company/Subject Code'>

</repeat>
  <comment>Here the repetition of 'Part of Speech' section ends.</comment>
</repeat>
  <comment>Here the repetition of the 'Headword' section ends.</comment>
</entryprt>
  <comment>Here the entry section ends. </comment>
<pagefoot>
  <comment>Here the pagefoot section starts.</comment>

======================================================================

                                     - <var sysname=$Page_No> -
  <comment>This statement defines that at the bottom of each page the current
  page number is printed.</comment>

</pagefoot>
  <comment>End of the pagefoot section.</comment>
