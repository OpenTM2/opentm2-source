#$perl
#--------------------------------------------------------------------#
#  Get all of the titles from the DITA files.                        #
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
      $i=index($InLine,"<title>");
      if ( $i >= 0 ) {
        $j=index($InLine,"</title>");
        if ( $j > $i ) {
          $title=substr($InLine,$i+7,$j-($i+7));

          $i=index($title,"<!--");
          if ( $i>=0 ) {
            $j=index($title,"-->",$i);
            if ( $j>$i) {
              if ($i==0){ $title=substr($title,$j+3); }
            }
          }
   keyword_again:
          $i=index($title,"<keyword ");
          if ( $i>=0 ) {
            $j=index($title,">",$i);
            if ( $j>$i) {
              $keyword=substr($title,$i,$j-$i+1);
              if ( index($keyword,"tm4w")>0 ) { $title=substr($title,0,$i)."OpenTM2".substr($title,$j+1); goto keyword_again;}
              if ( index($keyword,"xlmems")>0 ) { $title=substr($title,0,$i)."Translation Memory databases".substr($title,$j+1); goto keyword_again;}
              if ( index($keyword,"xlmem")>0 ) { $title=substr($title,0,$i)."Translation Memory".substr($title,$j+1); goto keyword_again;}
              if ( index($keyword,"ITM")>0 ) { $title=substr($title,0,$i)."Initial Translation Memory".substr($title,$j+1); goto keyword_again;}
              print "KEY=$keyword\n";
            }
          }
          print OUT_FILE "$file,$title,\n";
          $Found=1;
          last;
        }
      }
    }
    if ( $Found == 0 ) {
       print OUT_FILE "$file,,\n";
    }
    close(IN_FILE); 

    next;
}
close(OUT_FILE); 
close(FILE_LIST); 
system("del DITADOC.TMP"); 

exit $ERROR ; 
