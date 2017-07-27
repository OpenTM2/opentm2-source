#$perl
#--------------------------------------------------------------------#
#  Convert IBM DITA files to Dita Open Toolkit format.               #
#                                                                    #
#  Parameters:                                                       #
#      1st:  Input directory containing IBM DITA files.              #
#      2nd:  Output directory to contain DITA Open Toolkit files.    #
#      3rd:  Conversion type.  Always use "1".                       #
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
  $InDir=$ARGV[0];
  if ( (index($InDir,'*')>=0) ||
       (index($InDir,'?')>=0) ) {
    print "\n***ERROR.  Input directory contains wildcard characters: $InDir\n";
    exit $ERROR ; 
  }
  if ((-e $InDir)!=$TRUE) {
    print "\n***ERROR.  Input directory does not exist: $InDir\n";
    exit $ERROR ; 
  }
  if ( substr($InDir,length($InDir)-1,1) eq "\\" ) {
    $InPath=substr($InDir,0,length($InDir)-1);
  } else {
    $InPath=$InDir;
  }
} else {
  print "\n***ERROR.  No parameters specified:\n\n";
  print "\n           1st parm:   Input DITA directory\n";
  print "\n           2nd parm:   Output DITA directory\n";
  print "\n           3rd parm:   Conversion type:";
  print "\n                       1=IBM  DITA => Open DITA";
  print "\n                       2=Open DITA => IBM  DITA\n";
  exit $ERROR ; 
}

if ( defined $ARGV[1] ) {
  $OutDir=$ARGV[1];
  if ( (index($OutDir,'*')>=0) ||
       (index($OutDir,'?')>=0) ) {
    print "\n***ERROR.  Output directory contains wildcard characters: $OutDir\n";
    exit $ERROR ; 
  }
  if ((-e $OutDir)==$TRUE) {
    print "\n***WARNING.  Output directory already exist: $OutDir\n";
    print "\n             Do you want to delete this directory now?  (Y/N)\n";
    print "\n                 ==> ";
    $DelDir=<STDIN>;
    chomp($DelDir);
    if ($DelDir eq "y" || $DelDir eq "Y" ) {
      system("rd /s /q $OutDir");
    } else {
      print "\n  User cancelled processing.\n";
      exit $ERROR ; 
    }
  }
  if ( substr($OutDir,length($OutDir)-1,1) eq "\\" ) {
    $OutPath=substr($OutDir,0,length($OutDir)-1);
  } else {
    $OutPath=$OutDir;
  }
  if ( $InDir eq $OutDir ) {
    print "\n***ERROR.  Output directory must be different from input directory: $OutDir\n";
    exit $ERROR ; 
  }
} else {
  print "\n***ERROR.  Output directory was not specified.\n\n";
  print "\n           1st parm:   Input DITA directory\n";
  print "\n           2nd parm:   Output DITA directory\n";
  print "\n           3rd parm:   Conversion type:";
  print "\n                       1=IBM  DITA => Open DITA";
  print "\n                       2=Open DITA => IBM  DITA\n";
  exit $ERROR ; 
}

if ( defined $ARGV[2] ) {
  $ConvertType=$ARGV[2];
  if ( ($ConvertType ne '1') &&
       ($ConvertType ne '2') ) {
    print "\n***ERROR.  Invalid conversion type specified: $ConvertType\n\n";
    print "\n           1st parm:   Input DITA directory\n";
    print "\n           2nd parm:   Output DITA directory\n";
    print "\n           3rd parm:   Conversion type:";
    print "\n                       1=IBM  DITA => Open DITA";
    print "\n                       2=Open DITA => IBM  DITA\n";
    exit $ERROR ; 
  }
} else {
  print "\n***ERROR.  Conversion type was not specified.\n\n";
  print "\n           1st parm:   Input DITA directory\n";
  print "\n           2nd parm:   Output DITA directory\n";
  print "\n           3rd parm:   Conversion type:";
  print "\n                       1=IBM  DITA => Open DITA";
  print "\n                       2=Open DITA => IBM  DITA\n";
  exit $ERROR ; 
}

if ( defined $ARGV[3] ) {
  print "\n***ERROR.  Too many parameters were specified.\n\n";
  print "\n           1st parm:   Input DITA directory\n";
  print "\n           2nd parm:   Output DITA directory\n";
  print "\n           3rd parm:   Conversion type:";
  print "\n                       1=IBM  DITA => Open DITA";
  print "\n                       2=Open DITA => IBM  DITA\n";
  exit $ERROR ; 
}


#--------------------------------------------------------------------
#  Get list of files to process.
#--------------------------------------------------------------------

system("DIR /B /S $InPath\\* > DITADOC.TMP");
open(FILE_LIST,"DITADOC.TMP"); 

system("md $OutDir");
system("md $OutDir\\gifs");

if ( substr($InPath,0,3) eq "..\\" ) {
  $FromDir=substr($InPath,3);
} else {
  $FromDir=$InPath;
}
if ( substr($OutPath,0,3) eq "..\\" ) {
  $ToDir=substr($OutPath,3);
} else {
  $ToDir=$OutPath;
}

#--------------------------------------------------------------------
#  Processs each file, one at a time.
#--------------------------------------------------------------------
while( $FileName = <FILE_LIST> ) {
    chomp($FileName);
    $InFile=$FileName;
    $i=index($InFile,"\\".$FromDir."\\");
    if ( $i>0 ) {
      $OutFile=substr($InFile,0,$i+1).$ToDir.substr($InFile,$i+length($FromDir)+1);
      print "Out file:  $OutFile\n";
    } else {
      print "Skipping:  $InFile\n";
    }

    if ( ((-e $InFile)!=$TRUE) ||
         ((-f $InFile)!=$TRUE) ) {
      next;
    }

    open(IN_FILE,$InFile);
    open(OUT_FILE,">$OutFile");

    #--------------------------------------------------------------------
    #  No conversion for binary *GIF files.
    #--------------------------------------------------------------------
    $i=index($InFile,".gif");
    $j=index($InFile,".GIF");
    if ( $i>0 || $j>0 ) {
      binmode(IN_FILE);
      binmode(OUT_FILE);
      while( $InLine = <IN_FILE> ) {
        print OUT_FILE $InLine;
      }
    } else {

      #--------------------------------------------------------------------
      #  Convert tags different between IBM and Open Toolkit.
      #--------------------------------------------------------------------
      while( $InLine = <IN_FILE> ) {
     
        if ( $ConvertType eq "1" ) {

          #--------------------------------------------------------------------
          #  Convert <!DOCTYPE> statements.                          
          #--------------------------------------------------------------------
          if ( substr($InLine,0,9) eq "<!DOCTYPE" ) {
           for( ; index($InLine,">")<0 ; ) {
             $InLine=$InLine.<IN_FILE>;
           }
           if ( index($InLine,"ibmbook.dtd") >= 0 ) {
             $InLine="<!DOCTYPE bookmap PUBLIC \"-//OASIS//DTD DITA BookMap//EN\" \"bookmap.dtd\">\n";
           } else {
            if ( index($InLine,"ibm-reference.dtd") >= 0 ) {
               $InLine="<!DOCTYPE topic PUBLIC \"-//OASIS//DTD DITA Topic//EN\" \"topic.dtd\">\n";
            } else {
             if ( index($InLine,"ibm-topic.dtd") >= 0 ) {
                $InLine="<!DOCTYPE topic PUBLIC \"-//OASIS//DTD DITA Topic//EN\" \"topic.dtd\">\n";
             } else {
              if ( index($InLine,"ibm-map.dtd") >= 0 ) {
                 $InLine="<!DOCTYPE map PUBLIC \"-//OASIS//DTD DITA Topic//EN\" \"map.dtd\">\n";
              } else {
                 printf("  WARNING:  Unknown DTD:   $InLine\n");
              }
             }
            }
           }
          }
     
          #--------------------------------------------------------------------
          #  Convert tags beginning with "<ibm".                     
          #--------------------------------------------------------------------
          for( $i=index($InLine,"<ibm",0) ; $i>=0 ; $i=index($InLine,"<ibm",$i+1) ) {
            $Tag=substr($InLine,$i+1,15); 
            if ( substr($Tag,0,8) eq "ibmbook>" ) {
              $InLine=substr($InLine,0,$i+1)."bookmap".substr($InLine,$i+8);
            }
            if ( substr($Tag,0,14) eq "ibmshorttitle>" ) {
              $InLine=substr($InLine,0,$i+1)."booktitlealt".substr($InLine,$i+14);
            }
            if ( substr($InLine,$i+1,3) eq "ibm" ) {
              $InLine=substr($InLine,0,$i+1).substr($InLine,$i+4);
            }
              
          }
     
          #--------------------------------------------------------------------
          #  Convert tags beginning with "</ibm".                     
          #--------------------------------------------------------------------
          for( $i=index($InLine,"</ibm",0) ; $i>=0 ; $i=index($InLine,"</ibm",$i+1) ) {
            $Tag=substr($InLine,$i+2,15); 
            if ( substr($Tag,0,8) eq "ibmbook>" ) {
              $InLine=substr($InLine,0,$i+2)."bookmap".substr($InLine,$i+9);
            }
            if ( substr($Tag,0,14) eq "ibmshorttitle>" ) {
              $InLine=substr($InLine,0,$i+2)."booktitlealt".substr($InLine,$i+15);
            }
            if ( substr($InLine,$i+2,3) eq "ibm" ) {
              $InLine=substr($InLine,0,$i+2).substr($InLine,$i+5);
            }
              
          }
     
          #--------------------------------------------------------------------
          #  Convert special tags with different names.
          #--------------------------------------------------------------------
          if ( index($InLine,"<specialnotices") >= 0 ) { next;}
          if ( index($InLine,"<editionnotices") >= 0 ) { next;}
     
          $i=index($InLine,"<frontbooklists");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+1).substr($InLine,$i+6);
          }
          $i=index($InLine,"</frontbooklists");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+2).substr($InLine,$i+7);
          }
          $i=index($InLine,"<backbooklists");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+1).substr($InLine,$i+5);
          }
          $i=index($InLine,"</backbooklists");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+2).substr($InLine,$i+6);
          }
          $i=index($InLine,"<reference ");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+1)."topic ".substr($InLine,$i+11);
          }
          $i=index($InLine,"</reference>");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+2)."topic".substr($InLine,$i+11);
          }
          $i=index($InLine,"<refbody>");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+1).substr($InLine,$i+4);
          }
          $i=index($InLine,"</refbody>");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+2).substr($InLine,$i+5);
          }
          $i=index($InLine,"<refsyn>");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+1)."section".substr($InLine,$i+7);
          }
          $i=index($InLine,"<refsyn ");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+1)."section".substr($InLine,$i+7);
          }
          $i=index($InLine,"</refsyn>");
          if ( $i >= 0 ) {
             $InLine=substr($InLine,0,$i+2)."section".substr($InLine,$i+8);
          }
     
        } else {
        }
     
        print OUT_FILE $InLine;
      }
    }
    close(IN_FILE); 
    close(OUT_FILE); 

    next;
}
close(FILE_LIST); 
system("del DITADOC.TMP"); 

exit $ERROR ; 
