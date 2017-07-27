#$perl
#--------------------------------------------------------------------#
#  Get all of the GIF references from the DITA files.                #
#                                                                    #
#  Parameters:                                                       #
#      1st:  Input directory containing IBM DITA files.              #
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
  print "\n           1st parm:   Input DITA directory\n";
  print "\n           2nd parm:   Output file\n";
  exit $ERROR ; 
}

if ( defined $ARGV[2] ) {
  print "\n***ERROR.  Too many parameters were specified.\n\n";
  print "\n           1st parm:   Input DITA directory\n";
  print "\n           2nd parm:   Output file\n";
  exit $ERROR ; 
}


#--------------------------------------------------------------------
#  Get list of files to process.
#--------------------------------------------------------------------

system("DIR /B /S $InPath\\*.dita > DITADOC.TMP");
open(FILE_LIST,"DITADOC.TMP"); 


#--------------------------------------------------------------------
#  Processs each file, one at a time.
#--------------------------------------------------------------------
open(OUT_FILE,">$OutFile");

while( $FileName = <FILE_LIST> ) {
    chomp($FileName);
    $InFile=$FileName;

    if ( ((-e $InFile)!=$TRUE) ||
         ((-f $InFile)!=$TRUE) ) {
      next;
    }

    open(IN_FILE,$InFile);
    $i=rindex($InFile,'\\');
    $file=substr($InFile,$i+1);

    #--------------------------------------------------------------------
    #  Convert tags different between IBM and Open Toolkit.
    #--------------------------------------------------------------------
    $Found=0;
    while( $InLine = <IN_FILE> ) {
      for( $i=index($InLine,".gif") ; $i>=0 ; $i=index($InLine,".gif",$i+1) ) {
        $j=rindex($InLine,"\"",$i);
        if ( $j<0 ) {
          $j=rindex($InLine,"\'",$i);
        }
        if ( $j>=0 ) {
           $gif=substr($InLine,$j+1,($i+3)-($j+1)+1);
           print OUT_FILE "$file;$gif\n";
        } else {
          print "\n***ERROR.  Bad GIF.   [$InLine]\n";
        }
      }
    }
    close(IN_FILE); 

    next;
}
close(OUT_FILE); 
close(FILE_LIST); 
system("del DITADOC.TMP"); 

exit $ERROR ; 
