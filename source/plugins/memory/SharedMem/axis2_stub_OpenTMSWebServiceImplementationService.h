//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

        /**
        * axis2_stub_OpenTMSWebServiceImplementationService.h
        *
        * This file was auto-generated from WSDL for "OpenTMSWebServiceImplementationService|http://webservices.folt.de/" service
        * by the Apache Axis2/Java version: 1.6.1  Built on : Aug 31, 2011 (12:22:40 CEST)
        */

        #include <stdio.h>
        #include <axiom.h>
        #include <axutil_utils.h>
        #include <axiom_soap.h>
        #include <axis2_client.h>
        #include <axis2_stub.h>

       
         #include "adb_getMultilingualObject.h"
        
         #include "adb_getMultilingualObjectResponse.h"
        
         #include "adb_getMonolingualObject.h"
        
         #include "adb_getMonolingualObjectResponse.h"
        
         #include "adb_getLogfile.h"
        
         #include "adb_getLogfileResponse.h"
        
         #include "adb_translate.h"
        
         #include "adb_translateResponse.h"
        
         #include "adb_getLanguages.h"
        
         #include "adb_getLanguagesResponse.h"
        
         #include "adb_setLogfile.h"
        
         #include "adb_setLogfileResponse.h"
        
         #include "adb_synchronize.h"
        
         #include "adb_synchronizeResponse.h"
        
         #include "adb_getDataSources.h"
        
         #include "adb_getDataSourcesResponse.h"
        
         #include "adb_shutdown.h"
        
         #include "adb_shutdownResponse.h"
        
         #include "adb_bExistsDataSource.h"
        
         #include "adb_bExistsDataSourceResponse.h"
        

	#ifdef __cplusplus
	extern "C" {
	#endif

        /***************** function prototypes - for header file *************/
        /**
         * axis2_stub_create_OpenTMSWebServiceImplementationService
         * Create and return the stub with services populated
         * @param env Environment ( mandatory)
         * @param client_home Axis2/C home ( mandatory )
         * @param endpoint_uri Service endpoint uri( optional ) - if NULL default picked from WSDL used
         * @return Newly created stub object
         */
        axis2_stub_t* AXIS2_CALL
        axis2_stub_create_OpenTMSWebServiceImplementationService(const axutil_env_t *env,
                                        const axis2_char_t *client_home,
                                        const axis2_char_t *endpoint_uri);
        /**
         * axis2_stub_populate_services_for_OpenTMSWebServiceImplementationService
         * populate the svc in stub with the service and operations
         * @param stub The stub
         * @param env environment ( mandatory)
         */
        void AXIS2_CALL 
        axis2_stub_populate_services_for_OpenTMSWebServiceImplementationService( axis2_stub_t *stub, const axutil_env_t *env);
        /**
         * axis2_stub_get_endpoint_uri_of_OpenTMSWebServiceImplementationService
         * Return the endpoint URI picked from WSDL
         * @param env environment ( mandatory)
         * @return The endpoint picked from WSDL
         */
        axis2_char_t* AXIS2_CALL
        axis2_stub_get_endpoint_uri_of_OpenTMSWebServiceImplementationService(const axutil_env_t *env);


        
            /**
             * Auto generated function declaration
             * for "getMultilingualObject|http://webservices.folt.de/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             * @param _getMultilingualObject of the adb_getMultilingualObject_t*
             *
             * @return adb_getMultilingualObjectResponse_t*
             */

            adb_getMultilingualObjectResponse_t* AXIS2_CALL 
            axis2_stub_op_OpenTMSWebServiceImplementationService_getMultilingualObject( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_getMultilingualObject_t* _getMultilingualObject);
          
            /**
             * Auto generated function declaration
             * for "getMonolingualObject|http://webservices.folt.de/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             * @param _getMonolingualObject of the adb_getMonolingualObject_t*
             *
             * @return adb_getMonolingualObjectResponse_t*
             */

            adb_getMonolingualObjectResponse_t* AXIS2_CALL 
            axis2_stub_op_OpenTMSWebServiceImplementationService_getMonolingualObject( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_getMonolingualObject_t* _getMonolingualObject);
          
            /**
             * Auto generated function declaration
             * for "getLogfile|http://webservices.folt.de/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             * @param _getLogfile of the adb_getLogfile_t*
             *
             * @return adb_getLogfileResponse_t*
             */

            adb_getLogfileResponse_t* AXIS2_CALL 
            axis2_stub_op_OpenTMSWebServiceImplementationService_getLogfile( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_getLogfile_t* _getLogfile);
          
            /**
             * Auto generated function declaration
             * for "translate|http://webservices.folt.de/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             * @param _translate of the adb_translate_t*
             *
             * @return adb_translateResponse_t*
             */

            adb_translateResponse_t* AXIS2_CALL 
            axis2_stub_op_OpenTMSWebServiceImplementationService_translate( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_translate_t* _translate);
          
            /**
             * Auto generated function declaration
             * for "getLanguages|http://webservices.folt.de/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             * @param _getLanguages of the adb_getLanguages_t*
             *
             * @return adb_getLanguagesResponse_t*
             */

            adb_getLanguagesResponse_t* AXIS2_CALL 
            axis2_stub_op_OpenTMSWebServiceImplementationService_getLanguages( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_getLanguages_t* _getLanguages);
          
            /**
             * Auto generated function declaration
             * for "setLogfile|http://webservices.folt.de/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             * @param _setLogfile of the adb_setLogfile_t*
             *
             * @return adb_setLogfileResponse_t*
             */

            adb_setLogfileResponse_t* AXIS2_CALL 
            axis2_stub_op_OpenTMSWebServiceImplementationService_setLogfile( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_setLogfile_t* _setLogfile);
          
            /**
             * Auto generated function declaration
             * for "synchronize|http://webservices.folt.de/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             * @param _synchronize of the adb_synchronize_t*
             *
             * @return adb_synchronizeResponse_t*
             */

            adb_synchronizeResponse_t* AXIS2_CALL 
            axis2_stub_op_OpenTMSWebServiceImplementationService_synchronize( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_synchronize_t* _synchronize);
          
            /**
             * Auto generated function declaration
             * for "getDataSources|http://webservices.folt.de/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             * @param _getDataSources of the adb_getDataSources_t*
             *
             * @return adb_getDataSourcesResponse_t*
             */

            adb_getDataSourcesResponse_t* AXIS2_CALL 
            axis2_stub_op_OpenTMSWebServiceImplementationService_getDataSources( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_getDataSources_t* _getDataSources);
          
            /**
             * Auto generated function declaration
             * for "shutdown|http://webservices.folt.de/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             * @param _shutdown of the adb_shutdown_t*
             *
             * @return adb_shutdownResponse_t*
             */

            adb_shutdownResponse_t* AXIS2_CALL 
            axis2_stub_op_OpenTMSWebServiceImplementationService_shutdown( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_shutdown_t* _shutdown);
          
            /**
             * Auto generated function declaration
             * for "bExistsDataSource|http://webservices.folt.de/" operation.
             * @param stub The stub (axis2_stub_t)
             * @param env environment ( mandatory)
             * @param _bExistsDataSource of the adb_bExistsDataSource_t*
             *
             * @return adb_bExistsDataSourceResponse_t*
             */

            adb_bExistsDataSourceResponse_t* AXIS2_CALL 
            axis2_stub_op_OpenTMSWebServiceImplementationService_bExistsDataSource( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_bExistsDataSource_t* _bExistsDataSource);
          

        /**
         * Auto generated function for asynchronous invocations
         * for "getMultilingualObject|http://webservices.folt.de/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         * @param _getMultilingualObject of the adb_getMultilingualObject_t*
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */


        void AXIS2_CALL
        axis2_stub_start_op_OpenTMSWebServiceImplementationService_getMultilingualObject( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_getMultilingualObject_t* _getMultilingualObject,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_getMultilingualObjectResponse_t* _getMultilingualObjectResponse, void *data),
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

        

        /**
         * Auto generated function for asynchronous invocations
         * for "getMonolingualObject|http://webservices.folt.de/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         * @param _getMonolingualObject of the adb_getMonolingualObject_t*
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */


        void AXIS2_CALL
        axis2_stub_start_op_OpenTMSWebServiceImplementationService_getMonolingualObject( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_getMonolingualObject_t* _getMonolingualObject,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_getMonolingualObjectResponse_t* _getMonolingualObjectResponse, void *data),
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

        

        /**
         * Auto generated function for asynchronous invocations
         * for "getLogfile|http://webservices.folt.de/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         * @param _getLogfile of the adb_getLogfile_t*
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */


        void AXIS2_CALL
        axis2_stub_start_op_OpenTMSWebServiceImplementationService_getLogfile( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_getLogfile_t* _getLogfile,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_getLogfileResponse_t* _getLogfileResponse, void *data),
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

        

        /**
         * Auto generated function for asynchronous invocations
         * for "translate|http://webservices.folt.de/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         * @param _translate of the adb_translate_t*
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */


        void AXIS2_CALL
        axis2_stub_start_op_OpenTMSWebServiceImplementationService_translate( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_translate_t* _translate,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_translateResponse_t* _translateResponse, void *data),
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

        

        /**
         * Auto generated function for asynchronous invocations
         * for "getLanguages|http://webservices.folt.de/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         * @param _getLanguages of the adb_getLanguages_t*
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */


        void AXIS2_CALL
        axis2_stub_start_op_OpenTMSWebServiceImplementationService_getLanguages( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_getLanguages_t* _getLanguages,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_getLanguagesResponse_t* _getLanguagesResponse, void *data),
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

        

        /**
         * Auto generated function for asynchronous invocations
         * for "setLogfile|http://webservices.folt.de/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         * @param _setLogfile of the adb_setLogfile_t*
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */


        void AXIS2_CALL
        axis2_stub_start_op_OpenTMSWebServiceImplementationService_setLogfile( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_setLogfile_t* _setLogfile,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_setLogfileResponse_t* _setLogfileResponse, void *data),
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

        

        /**
         * Auto generated function for asynchronous invocations
         * for "synchronize|http://webservices.folt.de/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         * @param _synchronize of the adb_synchronize_t*
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */


        void AXIS2_CALL
        axis2_stub_start_op_OpenTMSWebServiceImplementationService_synchronize( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_synchronize_t* _synchronize,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_synchronizeResponse_t* _synchronizeResponse, void *data),
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

        

        /**
         * Auto generated function for asynchronous invocations
         * for "getDataSources|http://webservices.folt.de/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         * @param _getDataSources of the adb_getDataSources_t*
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */


        void AXIS2_CALL
        axis2_stub_start_op_OpenTMSWebServiceImplementationService_getDataSources( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_getDataSources_t* _getDataSources,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_getDataSourcesResponse_t* _getDataSourcesResponse, void *data),
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

        

        /**
         * Auto generated function for asynchronous invocations
         * for "shutdown|http://webservices.folt.de/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         * @param _shutdown of the adb_shutdown_t*
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */


        void AXIS2_CALL
        axis2_stub_start_op_OpenTMSWebServiceImplementationService_shutdown( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_shutdown_t* _shutdown,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_shutdownResponse_t* _shutdownResponse, void *data),
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

        

        /**
         * Auto generated function for asynchronous invocations
         * for "bExistsDataSource|http://webservices.folt.de/" operation.
         * @param stub The stub
         * @param env environment ( mandatory)
         * @param _bExistsDataSource of the adb_bExistsDataSource_t*
         * @param user_data user data to be accessed by the callbacks
         * @param on_complete callback to handle on complete
         * @param on_error callback to handle on error
         */


        void AXIS2_CALL
        axis2_stub_start_op_OpenTMSWebServiceImplementationService_bExistsDataSource( axis2_stub_t *stub, const axutil_env_t *env,
                                                  adb_bExistsDataSource_t* _bExistsDataSource,
                                                  void *user_data,
                                                  axis2_status_t ( AXIS2_CALL *on_complete ) (const axutil_env_t *, adb_bExistsDataSourceResponse_t* _bExistsDataSourceResponse, void *data),
                                                  axis2_status_t ( AXIS2_CALL *on_error ) (const axutil_env_t *, int exception, void *data) );

        


    /** we have to reserve some error codes for adb and for custom messages */
    #define AXIS2_STUB_OPENTMSWEBSERVICEIMPLEMENTATIONSERVICE_ERROR_CODES_START (AXIS2_ERROR_LAST + 2000)

    typedef enum 
    {
        AXIS2_STUB_OPENTMSWEBSERVICEIMPLEMENTATIONSERVICE_ERROR_NONE = AXIS2_STUB_OPENTMSWEBSERVICEIMPLEMENTATIONSERVICE_ERROR_CODES_START,
        
        AXIS2_STUB_OPENTMSWEBSERVICEIMPLEMENTATIONSERVICE_ERROR_LAST
    } axis2_stub_OpenTMSWebServiceImplementationService_error_codes;

	#ifdef __cplusplus
	}
	#endif
   

