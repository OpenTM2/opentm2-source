//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+


        /**
         * adb_setLogfileResponse.c
         *
         * This file was auto-generated from WSDL
         * by the Apache Axis2/C version: SNAPSHOT  Built on : Mar 10, 2008 (08:35:52 GMT+00:00)
         */

        #include "adb_setLogfileResponse.h"
        
               /*
                * implmentation of the setLogfileResponse|http://webservices.folt.de/ element
                */
           


        struct adb_setLogfileResponse
        {
            axis2_char_t *property_Type;

            
                axutil_qname_t* qname;
            
        };


       /************************* Private Function prototypes ********************************/
        


       /************************* Function Implmentations ********************************/
        adb_setLogfileResponse_t* AXIS2_CALL
        adb_setLogfileResponse_create(
            const axutil_env_t *env)
        {
            adb_setLogfileResponse_t *_setLogfileResponse = NULL;
            
                axutil_qname_t* qname = NULL;
            
            AXIS2_ENV_CHECK(env, NULL);

            _setLogfileResponse = (adb_setLogfileResponse_t *) AXIS2_MALLOC(env->
                allocator, sizeof(adb_setLogfileResponse_t));

            if(NULL == _setLogfileResponse)
            {
                AXIS2_ERROR_SET(env->error, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
                return NULL;
            }

            memset(_setLogfileResponse, 0, sizeof(adb_setLogfileResponse_t));

            _setLogfileResponse->property_Type = axutil_strdup(env, "adb_setLogfileResponse");
            
                  qname =  axutil_qname_create (env,
                        "setLogfileResponse",
                        "http://webservices.folt.de/",
                        NULL);
                _setLogfileResponse->qname = qname;
            

            return _setLogfileResponse;
        }

        adb_setLogfileResponse_t* AXIS2_CALL
        adb_setLogfileResponse_create_with_values(
            const axutil_env_t *env)
        {
            adb_setLogfileResponse_t* adb_obj = NULL;
            axis2_status_t status = AXIS2_SUCCESS;

            adb_obj = adb_setLogfileResponse_create(env);

            

            return adb_obj;
        }
      
        
                void* AXIS2_CALL
                adb_setLogfileResponse_free_popping_value(
                        adb_setLogfileResponse_t* _setLogfileResponse,
                        const axutil_env_t *env)
                {
                    adb_setLogfileResponse_free(_setLogfileResponse, env);
                    return NULL;
                }
            

        axis2_status_t AXIS2_CALL
        adb_setLogfileResponse_free(
                adb_setLogfileResponse_t* _setLogfileResponse,
                const axutil_env_t *env)
        {
            
            
            return adb_setLogfileResponse_free_obj(
                _setLogfileResponse,
                env);
            
        }

        axis2_status_t AXIS2_CALL
        adb_setLogfileResponse_free_obj(
                adb_setLogfileResponse_t* _setLogfileResponse,
                const axutil_env_t *env)
        {
            

            AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
            AXIS2_PARAM_CHECK(env->error, _setLogfileResponse, AXIS2_FAILURE);

            if (_setLogfileResponse->property_Type != NULL)
            {
              AXIS2_FREE(env->allocator, _setLogfileResponse->property_Type);
            }

            
              if(_setLogfileResponse->qname)
              {
                  axutil_qname_free (_setLogfileResponse->qname, env);
                  _setLogfileResponse->qname = NULL;
              }
            

            if(_setLogfileResponse)
            {
                AXIS2_FREE(env->allocator, _setLogfileResponse);
                _setLogfileResponse = NULL;
            }

            return AXIS2_SUCCESS;
        }


        

        axis2_status_t AXIS2_CALL
        adb_setLogfileResponse_deserialize(
                adb_setLogfileResponse_t* _setLogfileResponse,
                const axutil_env_t *env,
                axiom_node_t **dp_parent,
                axis2_bool_t *dp_is_early_node_valid,
                axis2_bool_t dont_care_minoccurs)
        {
            
            
            return adb_setLogfileResponse_deserialize_obj(
                _setLogfileResponse,
                env,
                dp_parent,
                dp_is_early_node_valid,
                dont_care_minoccurs);
            
        }

        axis2_status_t AXIS2_CALL
        adb_setLogfileResponse_deserialize_obj(
                adb_setLogfileResponse_t* _setLogfileResponse,
                const axutil_env_t *env,
                axiom_node_t **dp_parent,
                axis2_bool_t *dp_is_early_node_valid,
                axis2_bool_t dont_care_minoccurs)
        {
          axiom_node_t *parent = *dp_parent;
          
          axis2_status_t status = AXIS2_SUCCESS;
          
            axutil_qname_t *element_qname = NULL; 
            
            AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
            AXIS2_PARAM_CHECK(env->error, _setLogfileResponse, AXIS2_FAILURE);

            
          return status;
       }

          axis2_bool_t AXIS2_CALL
          adb_setLogfileResponse_is_particle()
          {
            
                 return AXIS2_FALSE;
              
          }


          void AXIS2_CALL
          adb_setLogfileResponse_declare_parent_namespaces(
                    adb_setLogfileResponse_t* _setLogfileResponse,
                    const axutil_env_t *env, axiom_element_t *parent_element,
                    axutil_hash_t *namespaces, int *next_ns_index)
          {
            
                  /* Here this is an empty function, Nothing to declare */
                 
          }

        
        
        axiom_node_t* AXIS2_CALL
        adb_setLogfileResponse_serialize(
                adb_setLogfileResponse_t* _setLogfileResponse,
                const axutil_env_t *env, axiom_node_t *parent, axiom_element_t *parent_element, int parent_tag_closed, axutil_hash_t *namespaces, int *next_ns_index)
        {
            
            
                return adb_setLogfileResponse_serialize_obj(
                    _setLogfileResponse, env, parent, parent_element, parent_tag_closed, namespaces, next_ns_index);
            
        }

        axiom_node_t* AXIS2_CALL
        adb_setLogfileResponse_serialize_obj(
                adb_setLogfileResponse_t* _setLogfileResponse,
                const axutil_env_t *env, axiom_node_t *parent, axiom_element_t *parent_element, int parent_tag_closed, axutil_hash_t *namespaces, int *next_ns_index)
        {
            
            
         
         axiom_node_t* current_node = NULL;
         int tag_closed = 0;
         
                axiom_namespace_t *ns1 = NULL;

                axis2_char_t *qname_uri = NULL;
                axis2_char_t *qname_prefix = NULL;
                axis2_char_t *p_prefix = NULL;
                axis2_bool_t ns_already_defined;
            
            
               axiom_data_source_t *data_source = NULL;
               axutil_stream_t *stream = NULL;

             
                int next_ns_index_value = 0;
            

            AXIS2_ENV_CHECK(env, NULL);
            AXIS2_PARAM_CHECK(env->error, _setLogfileResponse, NULL);
            
             
                    namespaces = axutil_hash_make(env);
                    next_ns_index = &next_ns_index_value;
                     
                           ns1 = axiom_namespace_create (env,
                                             "http://webservices.folt.de/",
                                             "n"); 
                           axutil_hash_set(namespaces, "http://webservices.folt.de/", AXIS2_HASH_KEY_STRING, axutil_strdup(env, "n"));
                       
                     
                    parent_element = axiom_element_create (env, NULL, "setLogfileResponse", ns1 , &parent);
                    
                    
                    axiom_element_set_namespace(parent_element, env, ns1, parent);


            

            return parent;
        }


        

