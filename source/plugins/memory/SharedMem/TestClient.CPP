//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#include "axis2_stub_OpenTMSWebServiceImplementationService.h"

#include <stdio.h>
#include <axiom.h>
#include <axis2_util.h>
#include <axiom_soap.h>
#include <axis2_client.h>

#define CREATECMD "{\"method\":\"create\",\"user-id\":\"klemens35\",\"password\":\"ccf4532\",\"name\":\"firstsstm\",\"parameter\":\"url=localhost;user=Klemens;password=dferc;port=1342\",\"user-id-list\":\"karl34;frith25;maria23\"}"

#include <axis2_client.h>

FILE *hfLog = NULL;
char szLogFile[512];
char szScriptFile[512];
char szUrl[512];
char szLineBuffer[32000];

int synchronize_test(const axutil_env_t *env, axis2_stub_t *stub, char *pszCommand, FILE *hfLog );

int main(int argc, char **argv)
{
    FILE *hfScript = NULL;

    /** the core variables */
    axutil_env_t *env = NULL;
    axis2_char_t *client_home = NULL;
    axis2_char_t *endpoint_uri = NULL;
    axis2_stub_t *stub = NULL;

    // check command line parameters
    argc--, argv++;
    szScriptFile[0] = '\0';
    szLogFile[0] = '\0';
    szUrl[0] = '\0';
    while ( argc )
    {
	    PSZ pszParm = argv[0];
	    if ( (*pszParm == '-') || (*pszParm == '/') )
	    {
	      if ( _strnicmp( pszParm+1, "logfile=", 8 ) == 0 )
	      {
	        strcpy( szLogFile, pszParm+9 );
	      }
	      else if ( _strnicmp( pszParm+1, "log=", 4 ) == 0 )
	      {
	        strcpy( szLogFile, pszParm+5 );
	      }
	      else if ( _strnicmp( pszParm+1, "url=", 4 ) == 0 )
	      {
	        strcpy( szUrl, pszParm+5 );
	      }
        else
        {
	        printf( "WARNING==> unknown option \'%s\' is ignored\n", pszParm );
	      } /* endif */
	    }
	    else if ( szScriptFile[0] == '\0' )
	    {
	      strcpy( szScriptFile, pszParm );
	    }
	    else
	    {
	      printf( "WARNING==> superfluos command line parameter \'%s\' is ignored\n", pszParm );
	    } /* endif */
	    argc--;
	    argv++;
	  } /* endwhile */

    // use default url if non given
    if ( szUrl[0] == '\0' )
    {
      strcpy( szUrl, "http://localhost:8090/openTMS" );
      printf( "Info: Using devault endpoint URL %s\n", szUrl );
    } /* endif */

    // stop if no script file has been specified
    if ( szScriptFile[0] == '\0' )
    {
      printf( "Error: %s\n", "No script file has been specified" );
      return( -1 );
    } /* end */       

    // open script file 
    hfScript = fopen( szScriptFile, "r" );
    if ( szScriptFile[0] == '\0' )
    {
      printf( "Error: Could not open script file %s\n", szScriptFile );
      return( -2 );
    } /* end */       

    // open out log file
    if ( szLogFile[0] != '\0' )
    {
      time_t curTime = 0;
      hfLog = fopen( szLogFile, "a" );
      time( &curTime );
      if ( hfLog ) 
      {
        printf( "Info: Writing log info into log file %s\n", szLogFile );
        fprintf( hfLog, "TestClient run started %s\n", asctime( localtime( &curTime ) ) );
        fprintf( hfLog, "   script= %s\n", szScriptFile );
        fprintf( hfLog, "   url= %s\n", szUrl );
      }
      else
      {
        printf( "Error: Could not open log file %s\n", szLogFile );
      } /* endif */         
    } /* endif */       

    /* create the stub */
    endpoint_uri = szUrl;

    env = axutil_env_create_all("alltest.log", AXIS2_LOG_LEVEL_TRACE);
    if ( env == NULL )
    {
      if ( hfLog ) fprintf( hfLog, "Error: call to axutil_env_create_all failed\n", "" );
      printf( "Error: call to axutil_env_create_all failed\n", "" );
    } /* end */       

    client_home = AXIS2_GETENV("AXIS2C_HOME");
    if ( client_home == NULL )
    {
      if ( hfLog ) fprintf( hfLog, "Error: AXIS2C_HOME environment variable not set\n", "" );
      printf( "Error: AXIS2C_HOME environment variable not set\n", "" );
    }
    else
    {
      if ( hfLog ) fprintf( hfLog, "    AXIS2C_HOME is set to %s\n", client_home );
    } /* end */       

    stub = axis2_stub_create_OpenTMSWebServiceImplementationService(env, client_home, endpoint_uri);
    if ( stub == NULL )
    {
      if ( hfLog ) fprintf( hfLog, "Error: could not create OpenTMSWebService stuff\n", "" );
      printf( "Error: could not create OpenTMSWebService stuff\n", "" );
    } /* end */       

    // send script lines to web service
    if( stub != NULL )
    {
      do
      {
        szLineBuffer[0] = 0;
        fgets( szLineBuffer, sizeof(szLineBuffer), hfScript );

        int iLen = strlen( szLineBuffer );
        if ( iLen != 0 )
        {
          if ( (iLen != 0) &&  (szLineBuffer[iLen-1] == '\n') )  { szLineBuffer[iLen-1] = 0; iLen--; }
          if ( (iLen != 0) &&  (szLineBuffer[iLen-1] == '\r') )  { szLineBuffer[iLen-1] = 0; iLen--; }

          if ( hfLog ) fprintf( hfLog, "Input Line: %s\n", szLineBuffer );
          printf( "Input Line: %s\n", szLineBuffer );

          if ( szLineBuffer[0] != '*' )
          {
            synchronize_test( env, stub, szLineBuffer, hfLog );
          } /* endif */
        } /* end */           
      } while ( !feof( hfScript ) );/* end */         
        /* call the method that actually invoke the service written below */
    }
    if ( hfLog ) fclose( hfLog );
    return 0;
}

int synchronize_test(const axutil_env_t *env, axis2_stub_t *stub, char *pszCommand, FILE *hfLog )
{

    adb_synchronize_t *synch;
    axis2_char_t *command = CREATECMD;

    adb_synchronizeResponse_t *response;
    axis2_char_t* returnedResponse;

    /* Build the request adb */

    synch = adb_synchronize_create(env);
    if( synch == NULL)
    {
      if ( hfLog ) fprintf( hfLog, "Error: %s\n", "adb_synchronize_create failed" );
      printf( "Error: %s\n", "adb_synchronize_create failed" );
      return -1;
    }
    adb_synchronize_set_parameters( synch, env, pszCommand );

    /* Finished building the request, time to call the service */
    if ( hfLog ) fprintf( hfLog, "Sending command to web service\ncmd=%s\n", pszCommand );
    response = axis2_stub_op_OpenTMSWebServiceImplementationService_synchronize( stub, env, synch );

    /* Extract the response */
    if(response)
    {
        returnedResponse = adb_synchronizeResponse_get_return(response, env);
        if ( hfLog ) fprintf( hfLog, "Response returned by web service\nresp=%s\n", returnedResponse );
        printf( "Response: %s\n", returnedResponse );
        return 0;
    }
    else
    {
        if ( hfLog ) fprintf( hfLog, "Error: %s\n", "No repsonse from web service" );
        printf( "Error: %s\n", "No repsonse from web service" );
    }
    return -1;
}
