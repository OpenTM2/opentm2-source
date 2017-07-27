#$perl
#--------------------------------------------------------------------#
#  Rename DITA files for Technical Reference.                        #
#  Change all of the references to the DITA files.                   #
#                                                                    #
#  Parameters:                                                       #
#      1st:  Input directory containing IBM DITA files.              #
#      2nd:  Output directory to contain DITA Open Toolkit files.    #
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

open(CONTROL,$ControlFile);

#--------------------------------------------------------------------
#  Processs each file, one at a time.
#--------------------------------------------------------------------
while( $Control = <CONTROL> ) {
  $i=index($Control,",");
  if ( $i<=0 ) {
    print "\n*** ERROR:  No semicolon separator.   $Control\n";
    next;
  }
  $FromFile=substr($Control,0,$i);
  $ToFile=substr($Control,$i+1);
  chomp($FromFile);
  chomp($ToFile);
  $FromFile=$FromFile.".dita";
  $ToFile=$ToFile.".dita";
  $FromPath=$InPath."\\".$FromFile;
  $ToPath=$OutPath."\\".$ToFile;

  if ((-e $FromPath)!=$TRUE) {
    print "\n***ERROR:  From file does not exist: $FromPath\n";
    next;
  }
  if ((-e $ToPath)==$TRUE) {
    print "\n***ERROR:  To file already exists: $ToPath\n";
    next;
  }

  open(IN_FILE,$FromPath);
  open(OUT_FILE,">$ToPath");
  print("C:   $FromFile   ==>   $ToFile\n");

  #--------------------------------------------------------------------
  #  Convert old file names to new file names on each line.
  #--------------------------------------------------------------------
  while( $InLine = <IN_FILE> ) {

    for( $DitaName=index($InLine,"eqfr5mst"); $DitaName>=0 ; $DitaName=index($InLine,"eqfr5mst",$DitaName+1) ) {
      $i=index($InLine,"\"",$DitaName);
      $j=index($InLine,"\'",$DitaName);
      if ( ($i<0) || ($j>0 && $j<$i) ) { $i=$j; }
      $j=index($InLine,"/",$DitaName);
      if ( ($i<0) || ($j>0 && $j<$i) ) { $i=$j; }
      $j=index($InLine,".",$DitaName);
      if ( ($i<0) || ($j>0 && $j<$i) ) { $i=$j; }
      $OldStart=$DitaName;
      $OldEnd=$i-1;
      $OldName=substr($InLine,$OldStart,$OldEnd-$OldStart+1);
#     if ( $OldName eq "common-symbols.dita" ) {
#       next;
#     }

      $Found=0;
      open(CONTROL2,$ControlFile);
      while( $CLine = <CONTROL2> ) {
        $i=index($CLine,",");
        if ( $i<=0 ) {
          next;
        }
        $Name1=substr($CLine,0,$i);
        chomp($Name1);
        if ( $Name1 eq $OldName ) {
          $Found=1;
          $Name2=substr($CLine,$i+1);
          chomp($Name2);

          $InLine=substr($InLine,0,$OldStart).$Name2.substr($InLine,$OldEnd+1);
          last;
        }
      }
      close(CONTROL2);
      if ( $Found==0 ) {
        print "\n***ERROR:  DITA file not in control file:  $OldName\n";
      }

    }
    print OUT_FILE $InLine;
  }
  close(IN_FILE);
  close(OUT_FILE);
}

close(CONTROL);
print("\n\n");

exit $ERROR ; 
