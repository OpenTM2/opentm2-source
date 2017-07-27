#$perl
#--------------------------------------------------------------------#
#  Get list of DITA files used by the DITAMAP file.                  #
#                                                                    #
#  Parameters:                                                       #
#      1st:  Input DITAMAP file.                                     #
#      2nd:  Output file.                                            #
#                                                                    #
#   6/09/17  DAW   Created.                                          #
#--------------------------------------------------------------------#

$ERROR=9;
$OK=0;
$TRUE=1;
$FALSE=0; 
$GenericName=0;


#--------------------------------------------------------------------
#  Get passed parameters and set processing variables.    
#--------------------------------------------------------------------

if ( defined $ARGV[0] ) {
  $InFile=$ARGV[0];
  if ( (index($InFile,'*')>=0) ||
       (index($InFile,'?')>=0) ) {
    print "\n***ERROR.  Input map file contains wildcard characters: $InFile\n";
    exit $ERROR ; 
  }
  if ((-e $InFile)!=$TRUE) {
    print "\n***ERROR.  Input map file does not exist: $InFile\n";
    exit $ERROR ; 
  }
} else {
  print "\n***ERROR.  No parameters specified:\n\n";
  print "\n           1st parm:   Input DITA map file\n";
  print "\n           2nd parm:   Output file\n";
  exit $ERROR ; 
}

if ( defined $ARGV[1] ) {
  $OutFile=$ARGV[1];
  if ( (index($OutFile,'*')>=0) ||
       (index($OutFile,'?')>=0) ) {
    print "\n***ERROR.  Output file contains wildcard characters: $OutFile\n";
    exit $ERROR ; 
  }
} else {
  print "\n***ERROR.  Output file was not specified.\n\n";
  print "\n           1st parm:   Input DITA map file\n";
  print "\n           2nd parm:   Output file\n";
  exit $ERROR ; 
}

if ( defined $ARGV[2] ) {
  print "\n***ERROR.  Too many parameters were specified.\n\n";
  print "\n           1st parm:   Input DITA map file\n";
  print "\n           2nd parm:   Output file\n";
  exit $ERROR ; 
}

#--------------------------------------------------------------------
#  Process input map file.
#--------------------------------------------------------------------
open(OUT_FILE,">$OutFile");

open(IN_FILE,$InFile);

#--------------------------------------------------------------------
#  Convert tags different between IBM and Open Toolkit.
#--------------------------------------------------------------------
$navnest=0;
$chapfile="";
$chaptitle="";
$file="";
$title="";
while( $InLine = <IN_FILE> ) {
  #--------------------------------------------------------------------
  #  TOPICREF
  #--------------------------------------------------------------------
#    print OUT_FILE "1-[$chaptitle]\n" ;
  if (index($InLine,"<topicref ")>=0) {
    $navnest=$navnest+1;
    $file="";
    $title="";
    $i=index($InLine,' href="');
    if ($i>0) {
      $j=index($InLine,qq("),$i+7);
      if ($j>0) {
        $file=substr($InLine,$i+7,$j-($i+7));
      }
    }
    $i=index($InLine,' navtitle="');
    if ($i>0) {
      $j=index($InLine,'"',$i+11);
      if ($j>0) {
        $title=substr($InLine,$i+11,$j-($i+11));
        while( index($title,"&lt;") >= 0 ) {
          $i=index($title,"&lt;");
          $title=substr($title,0,$i-1)."<".substr($title,$i+4);
        }
        while( index($title,"&gt;") >= 0 ) {
          $i=index($title,"&gt;");
          $title=substr($title,0,$i-1).">".substr($title,$i+4);
        }
      }
    }
    print OUT_FILE "T;$navnest;$file;$title;$chaptitle;;;\n" ;
  }
  if (index($InLine,"</topicref")>=0) {
    $navnest=$navnest-1;
  }

  #--------------------------------------------------------------------
  #  CHAPTER
  #--------------------------------------------------------------------
#    print OUT_FILE "2-[$chaptitle]\n" ;
  if (index($InLine,'<chapter ')>=0) {
    $chapfile="";
    $chaptitle="";
    $i=index($InLine,' href="');
    if ($i>0) {
      $j=index($InLine,'"',$i+7);
      if ($j>0) {
        $chapfile=substr($InLine,$i+7,$j-($i+7));
      }
    }
    $i=index($InLine,' navtitle="');
    if ($i>0) {
      $j=index($InLine,'"',$i+11);
      if ($j>0) {
        $chaptitle=substr($InLine,$i+11,$j-($i+11));
      }
    }
    print OUT_FILE "C;0;$chapfile;x;$chaptitle;$chapfile;;;\n"
  }

  #--------------------------------------------------------------------
  #  PART   
  #--------------------------------------------------------------------
  if (index($InLine,'<part ')>=0) {
    $partfile="";
    $parttitle="";
    $i=index($InLine,' href="');
    if ($i>0) {
      $j=index($InLine,'"',$i+7);
      if ($j>0) {
        $partfile=substr($InLine,$i+7,$j-($i+7));
      }
    }
    $i=index($InLine,' navtitle="');
    if ($i>0) {
      $j=index($InLine,'"',$i+11);
      if ($j>0) {
        $parttitle=substr($InLine,$i+11,$j-($i+11));
      }
    }
    print OUT_FILE "P;0;$partfile;x;$parttitle;$partfile;;;\n"
  }

  #--------------------------------------------------------------------
  #  IBMFRONTMATTER
  #--------------------------------------------------------------------
  if (index($InLine,"<ibmfrontmatter")>=0) {
    $chapfile="";
    $chaptitle="";
    print OUT_FILE "F;x;x;x;FRONTMATTER;;;\n"
  }

}
close(IN_FILE); 
close(OUT_FILE); 

exit $ERROR ; 
