#$perl
#--------------------------------------------------------------------#
#  Rename GIF references in the DITA files.                          #
#                                                                    #
#  Parameters:                                                       #
#      1st:  Input directory containing DITA files.                  #
#      2nd:  Output directory to updated DITA files.                 #
#      3rd:  Conversion file (old->new names).                       #
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
  print "\n           3rd parm:   Control file\n";
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
  print "\n           3rd parm:   Control file\n";
  exit $ERROR ; 
}

if ( defined $ARGV[2] ) {
  $ControlFile=$ARGV[2];
  if ((-e $ControlFile)!=$TRUE) {
    print "\n***ERROR.  Control file does not exist: $ControlFile\n";
    exit $ERROR ; 
  }
} else {
  print "\n***ERROR.  Control file was not specified.\n\n";
  print "\n           1st parm:   Input DITA directory\n";
  print "\n           2nd parm:   Output DITA directory\n";
  print "\n           3rd parm:   Control file\n";
  exit $ERROR ; 
}

if ( defined $ARGV[3] ) {
  print "\n***ERROR.  Too many parameters were specified.\n\n";
  print "\n           1st parm:   Input DITA directory\n";
  print "\n           2nd parm:   Output DITA directory\n";
  print "\n           3rd parm:   Control file\n";
  exit $ERROR ; 
}


#--------------------------------------------------------------------
#  Create output directory.
#--------------------------------------------------------------------
system("md $OutDir");

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

system("DIR /B /S $InPath\\* > DITADOC.TMP");
open(FILE_LIST,"DITADOC.TMP"); 

#--------------------------------------------------------------------
#  Processs each file, one at a time.
#--------------------------------------------------------------------
while( $FileName = <FILE_LIST> ) {
  chomp($FileName);
  if ( index($FileName,".gif")>0 ) { next; }
  if ( index($FileName,".zip")>0 ) { next; }

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
  #  Convert old file names to new file names on each line.
  #--------------------------------------------------------------------
  while( $InLine = <IN_FILE> ) {

    for( $i=index($InLine,".gif") ; $i>=0 ; $i=index($InLine,".gif",$i+1) ) {
      $j=rindex($InLine,"\"",$i);
      if ( $j<0 ) {
        $j=rindex($InLine,"\'",$i);
      }
      if ( $j<0 ) {
        print "\n***ERROR.  Bad GIF.   [$InLine]\n";
        next;
      }
      $OldStart=$j+1;
      $OldEnd=$i+3;
      $OldName=substr($InLine,$OldStart,$OldEnd-$OldStart+1);

      $Found=0;
      open(CONTROL2,$ControlFile);
      while( $CLine = <CONTROL2> ) {
        $j=index($CLine,",");
        if ( $j<=0 ) {
          next;
        }
        $Name1=substr($CLine,0,$j);
        chomp($Name1);
        if ( $Name1 eq $OldName ) {
          $Found=1;
          $Name2=substr($CLine,$j+1);
          chomp($Name2);

          $InLine=substr($InLine,0,$OldStart)."gifs/".$Name2.substr($InLine,$OldEnd+1);
          $i=index($InLine,".gif",$OldStart);

          print "      $Name1  ==>  $Name2\n";
          last;
        }
      }
      close(CONTROL2);
      if ( $Found==0 ) {
        print "\n***ERROR:  GIF file not in control file:  $OldName\n";
      }

    }
    print OUT_FILE $InLine;
  }
  close(IN_FILE);
  close(OUT_FILE);
}

close(FILE_LIST);
system("del DITADOC.TMP"); 
print("\n\n");

exit $ERROR ; 
