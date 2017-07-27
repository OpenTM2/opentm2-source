#$perl
#--------------------------------------------------------------------#
#  Rename GIF files for Translator's Reference.                      #
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
    print "\n*** ERROR:  No comma separator.   $Control\n";
    next;
  }
  $FromFile=substr($Control,0,$i);
  $ToFile=substr($Control,$i+1);
  chomp($FromFile);
  chomp($ToFile);
  $FromFile=$FromFile;
  $ToFile=$ToFile;
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

  binmode(IN_FILE);
  binmode(OUT_FILE);
  while( $InLine = <IN_FILE> ) {
    print OUT_FILE $InLine;
  }
  close(IN_FILE);
  close(OUT_FILE);
}

close(CONTROL);
print("\n\n");

exit $ERROR ; 
