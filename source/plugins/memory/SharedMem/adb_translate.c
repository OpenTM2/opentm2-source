//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+


        /**
         * adb_translate.c
         *
         * This file was auto-generated from WSDL
         * by the Apache Axis2/C version: SNAPSHOT  Built on : Mar 10, 2008 (08:35:52 GMT+00:00)
         */

        #include "adb_translate.h"
        
               /*
                * implmentation of the translate|http://webservices.folt.de/ element
                */
           


        struct adb_translate
        {
            axis2_char_t *property_Type;

            
                axutil_qname_t* qname;
            axis2_char_t* property_sourceSegment;

                
                axis2_bool_t is_valid_sourceSegment;
            axis2_char_t* property_sourceLanguage;

                
                axis2_bool_t is_valid_sourceLanguage;
            axis2_char_t* property_targetLanguage;

                
                axis2_bool_t is_valid_targetLanguage;
            axis2_char_t* property_datasource;

                
                axis2_bool_t is_valid_datasource;
            int property_matchSimilarity;

                
                axis2_bool_t is_valid_matchSimilarity;
            axis2_char_t* property_type;

                
                axis2_bool_t is_valid_type;
            
        };


       /************************* Private Function prototypes ********************************/
        


       /************************* Function Implmentations ********************************/
        adb_translate_t* AXIS2_CALL
        adb_translate_create(
            const axutil_env_t *env)
        {
            adb_translate_t *_translate = NULL;
            
                axutil_qname_t* qname = NULL;
            
            AXIS2_ENV_CHECK(env, NULL);

            _translate = (adb_translate_t *) AXIS2_MALLOC(env->
                allocator, sizeof(adb_translate_t));

            if(NULL == _translate)
            {
                AXIS2_ERROR_SET(env->error, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
                return NULL;
            }

            memset(_translate, 0, sizeof(adb_translate_t));

            _translate->property_Type = axutil_strdup(env, "adb_translate");
            _translate->property_sourceSegment  = NULL;
                  _translate->is_valid_sourceSegment  = AXIS2_FALSE;
            _translate->property_sourceLanguage  = NULL;
                  _translate->is_valid_sourceLanguage  = AXIS2_FALSE;
            _translate->property_targetLanguage  = NULL;
                  _translate->is_valid_targetLanguage  = AXIS2_FALSE;
            _translate->property_datasource  = NULL;
                  _translate->is_valid_datasource  = AXIS2_FALSE;
            _translate->is_valid_matchSimilarity  = AXIS2_FALSE;
            _translate->property_type  = NULL;
                  _translate->is_valid_type  = AXIS2_FALSE;
            
                  qname =  axutil_qname_create (env,
                        "translate",
                        "http://webservices.folt.de/",
                        NULL);
                _translate->qname = qname;
            

            return _translate;
        }

        adb_translate_t* AXIS2_CALL
        adb_translate_create_with_values(
            const axutil_env_t *env,
                axis2_char_t* _sourceSegment,
                axis2_char_t* _sourceLanguage,
                axis2_char_t* _targetLanguage,
                axis2_char_t* _datasource,
                int _matchSimilarity,
                axis2_char_t* _type)
        {
            adb_translate_t* adb_obj = NULL;
            axis2_status_t status = AXIS2_SUCCESS;

            adb_obj = adb_translate_create(env);

            
              status = adb_translate_set_sourceSegment(
                                     adb_obj,
                                     env,
                                     _sourceSegment);
              if(status == AXIS2_FAILURE) {
                  adb_translate_free (adb_obj, env);
                  return NULL;
              }
            
              status = adb_translate_set_sourceLanguage(
                                     adb_obj,
                                     env,
                                     _sourceLanguage);
              if(status == AXIS2_FAILURE) {
                  adb_translate_free (adb_obj, env);
                  return NULL;
              }
            
              status = adb_translate_set_targetLanguage(
                                     adb_obj,
                                     env,
                                     _targetLanguage);
              if(status == AXIS2_FAILURE) {
                  adb_translate_free (adb_obj, env);
                  return NULL;
              }
            
              status = adb_translate_set_datasource(
                                     adb_obj,
                                     env,
                                     _datasource);
              if(status == AXIS2_FAILURE) {
                  adb_translate_free (adb_obj, env);
                  return NULL;
              }
            
              status = adb_translate_set_matchSimilarity(
                                     adb_obj,
                                     env,
                                     _matchSimilarity);
              if(status == AXIS2_FAILURE) {
                  adb_translate_free (adb_obj, env);
                  return NULL;
              }
            
              status = adb_translate_set_type(
                                     adb_obj,
                                     env,
                                     _type);
              if(status == AXIS2_FAILURE) {
                  adb_translate_free (adb_obj, env);
                  return NULL;
              }
            

            return adb_obj;
        }
      
        axis2_char_t* AXIS2_CALL
                adb_translate_free_popping_value(
                        adb_translate_t* _translate,
                        const axutil_env_t *env)
                {
                    axis2_char_t* value;

                    
                    
                    value = _translate->property_sourceSegment;

                    _translate->property_sourceSegment = (axis2_char_t*)NULL;
                    adb_translate_free(_translate, env);

                    return value;
                }
            

        axis2_status_t AXIS2_CALL
        adb_translate_free(
                adb_translate_t* _translate,
                const axutil_env_t *env)
        {
            
            
            return adb_translate_free_obj(
                _translate,
                env);
            
        }

        axis2_status_t AXIS2_CALL
        adb_translate_free_obj(
                adb_translate_t* _translate,
                const axutil_env_t *env)
        {
            

            AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
            AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);

            if (_translate->property_Type != NULL)
            {
              AXIS2_FREE(env->allocator, _translate->property_Type);
            }

            adb_translate_reset_sourceSegment(_translate, env);
            adb_translate_reset_sourceLanguage(_translate, env);
            adb_translate_reset_targetLanguage(_translate, env);
            adb_translate_reset_datasource(_translate, env);
            adb_translate_reset_matchSimilarity(_translate, env);
            adb_translate_reset_type(_translate, env);
            
              if(_translate->qname)
              {
                  axutil_qname_free (_translate->qname, env);
                  _translate->qname = NULL;
              }
            

            if(_translate)
            {
                AXIS2_FREE(env->allocator, _translate);
                _translate = NULL;
            }

            return AXIS2_SUCCESS;
        }


        

        axis2_status_t AXIS2_CALL
        adb_translate_deserialize(
                adb_translate_t* _translate,
                const axutil_env_t *env,
                axiom_node_t **dp_parent,
                axis2_bool_t *dp_is_early_node_valid,
                axis2_bool_t dont_care_minoccurs)
        {
            
            
            return adb_translate_deserialize_obj(
                _translate,
                env,
                dp_parent,
                dp_is_early_node_valid,
                dont_care_minoccurs);
            
        }

        axis2_status_t AXIS2_CALL
        adb_translate_deserialize_obj(
                adb_translate_t* _translate,
                const axutil_env_t *env,
                axiom_node_t **dp_parent,
                axis2_bool_t *dp_is_early_node_valid,
                axis2_bool_t dont_care_minoccurs)
        {
          axiom_node_t *parent = *dp_parent;
          
          axis2_status_t status = AXIS2_SUCCESS;
           
             const axis2_char_t* text_value = NULL;
             axutil_qname_t *qname = NULL;
          
            axutil_qname_t *element_qname = NULL; 
            
               axiom_node_t *first_node = NULL;
               axis2_bool_t is_early_node_valid = AXIS2_TRUE;
               axiom_node_t *current_node = NULL;
               axiom_element_t *current_element = NULL;
            
            AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
            AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);

            
              
              while(parent && axiom_node_get_node_type(parent, env) != AXIOM_ELEMENT)
              {
                  parent = axiom_node_get_next_sibling(parent, env);
              }
              if (NULL == parent)
              {
                /* This should be checked before everything */
                AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, 
                            "Failed in building adb object for translate : "
                            "NULL element can not be passed to deserialize");
                return AXIS2_FAILURE;
              }
              

                    current_element = (axiom_element_t *)axiom_node_get_data_element(parent, env);
                    qname = axiom_element_get_qname(current_element, env, parent);
                    if (axutil_qname_equals(qname, env, _translate-> qname))
                    {
                        
                          first_node = axiom_node_get_first_child(parent, env);
                          
                    }
                    else
                    {
                        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, 
                              "Failed in building adb object for translate : "
                              "Expected %s but returned %s",
                              axutil_qname_to_string(_translate-> qname, env),
                              axutil_qname_to_string(qname, env));
                        
                        return AXIS2_FAILURE;
                    }
                    

                     
                     /*
                      * building sourceSegment element
                      */
                     
                     
                     
                                   current_node = first_node;
                                   is_early_node_valid = AXIS2_FALSE;
                                   
                                   
                                    while(current_node && axiom_node_get_node_type(current_node, env) != AXIOM_ELEMENT)
                                    {
                                        current_node = axiom_node_get_next_sibling(current_node, env);
                                    }
                                    if(current_node != NULL)
                                    {
                                        current_element = (axiom_element_t *)axiom_node_get_data_element(current_node, env);
                                        qname = axiom_element_get_qname(current_element, env, current_node);
                                    }
                                   
                                 element_qname = axutil_qname_create(env, "sourceSegment", NULL, NULL);
                                 

                           if ( 
                                (current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("sourceSegment", axiom_element_get_localname(current_element, env)))))
                           {
                              if( current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("sourceSegment", axiom_element_get_localname(current_element, env))))
                              {
                                is_early_node_valid = AXIS2_TRUE;
                              }
                              
                                 
                                      text_value = axiom_element_get_text(current_element, env, current_node);
                                      if(text_value != NULL)
                                      {
                                            status = adb_translate_set_sourceSegment(_translate, env,
                                                               text_value);
                                      }
                                      
                                 if(AXIS2_FAILURE ==  status)
                                 {
                                     AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "failed in setting the value for sourceSegment ");
                                     if(element_qname)
                                     {
                                         axutil_qname_free(element_qname, env);
                                     }
                                     return AXIS2_FAILURE;
                                 }
                              }
                           
                  if(element_qname)
                  {
                     axutil_qname_free(element_qname, env);
                     element_qname = NULL;
                  }
                 

                     
                     /*
                      * building sourceLanguage element
                      */
                     
                     
                     
                                    /*
                                     * because elements are ordered this works fine
                                     */
                                  
                                   
                                   if(current_node != NULL && is_early_node_valid)
                                   {
                                       current_node = axiom_node_get_next_sibling(current_node, env);
                                       
                                       
                                        while(current_node && axiom_node_get_node_type(current_node, env) != AXIOM_ELEMENT)
                                        {
                                            current_node = axiom_node_get_next_sibling(current_node, env);
                                        }
                                        if(current_node != NULL)
                                        {
                                            current_element = (axiom_element_t *)axiom_node_get_data_element(current_node, env);
                                            qname = axiom_element_get_qname(current_element, env, current_node);
                                        }
                                       
                                   }
                                   is_early_node_valid = AXIS2_FALSE;
                                 
                                 element_qname = axutil_qname_create(env, "sourceLanguage", NULL, NULL);
                                 

                           if ( 
                                (current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("sourceLanguage", axiom_element_get_localname(current_element, env)))))
                           {
                              if( current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("sourceLanguage", axiom_element_get_localname(current_element, env))))
                              {
                                is_early_node_valid = AXIS2_TRUE;
                              }
                              
                                 
                                      text_value = axiom_element_get_text(current_element, env, current_node);
                                      if(text_value != NULL)
                                      {
                                            status = adb_translate_set_sourceLanguage(_translate, env,
                                                               text_value);
                                      }
                                      
                                 if(AXIS2_FAILURE ==  status)
                                 {
                                     AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "failed in setting the value for sourceLanguage ");
                                     if(element_qname)
                                     {
                                         axutil_qname_free(element_qname, env);
                                     }
                                     return AXIS2_FAILURE;
                                 }
                              }
                           
                  if(element_qname)
                  {
                     axutil_qname_free(element_qname, env);
                     element_qname = NULL;
                  }
                 

                     
                     /*
                      * building targetLanguage element
                      */
                     
                     
                     
                                    /*
                                     * because elements are ordered this works fine
                                     */
                                  
                                   
                                   if(current_node != NULL && is_early_node_valid)
                                   {
                                       current_node = axiom_node_get_next_sibling(current_node, env);
                                       
                                       
                                        while(current_node && axiom_node_get_node_type(current_node, env) != AXIOM_ELEMENT)
                                        {
                                            current_node = axiom_node_get_next_sibling(current_node, env);
                                        }
                                        if(current_node != NULL)
                                        {
                                            current_element = (axiom_element_t *)axiom_node_get_data_element(current_node, env);
                                            qname = axiom_element_get_qname(current_element, env, current_node);
                                        }
                                       
                                   }
                                   is_early_node_valid = AXIS2_FALSE;
                                 
                                 element_qname = axutil_qname_create(env, "targetLanguage", NULL, NULL);
                                 

                           if ( 
                                (current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("targetLanguage", axiom_element_get_localname(current_element, env)))))
                           {
                              if( current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("targetLanguage", axiom_element_get_localname(current_element, env))))
                              {
                                is_early_node_valid = AXIS2_TRUE;
                              }
                              
                                 
                                      text_value = axiom_element_get_text(current_element, env, current_node);
                                      if(text_value != NULL)
                                      {
                                            status = adb_translate_set_targetLanguage(_translate, env,
                                                               text_value);
                                      }
                                      
                                 if(AXIS2_FAILURE ==  status)
                                 {
                                     AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "failed in setting the value for targetLanguage ");
                                     if(element_qname)
                                     {
                                         axutil_qname_free(element_qname, env);
                                     }
                                     return AXIS2_FAILURE;
                                 }
                              }
                           
                  if(element_qname)
                  {
                     axutil_qname_free(element_qname, env);
                     element_qname = NULL;
                  }
                 

                     
                     /*
                      * building datasource element
                      */
                     
                     
                     
                                    /*
                                     * because elements are ordered this works fine
                                     */
                                  
                                   
                                   if(current_node != NULL && is_early_node_valid)
                                   {
                                       current_node = axiom_node_get_next_sibling(current_node, env);
                                       
                                       
                                        while(current_node && axiom_node_get_node_type(current_node, env) != AXIOM_ELEMENT)
                                        {
                                            current_node = axiom_node_get_next_sibling(current_node, env);
                                        }
                                        if(current_node != NULL)
                                        {
                                            current_element = (axiom_element_t *)axiom_node_get_data_element(current_node, env);
                                            qname = axiom_element_get_qname(current_element, env, current_node);
                                        }
                                       
                                   }
                                   is_early_node_valid = AXIS2_FALSE;
                                 
                                 element_qname = axutil_qname_create(env, "datasource", NULL, NULL);
                                 

                           if ( 
                                (current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("datasource", axiom_element_get_localname(current_element, env)))))
                           {
                              if( current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("datasource", axiom_element_get_localname(current_element, env))))
                              {
                                is_early_node_valid = AXIS2_TRUE;
                              }
                              
                                 
                                      text_value = axiom_element_get_text(current_element, env, current_node);
                                      if(text_value != NULL)
                                      {
                                            status = adb_translate_set_datasource(_translate, env,
                                                               text_value);
                                      }
                                      
                                 if(AXIS2_FAILURE ==  status)
                                 {
                                     AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "failed in setting the value for datasource ");
                                     if(element_qname)
                                     {
                                         axutil_qname_free(element_qname, env);
                                     }
                                     return AXIS2_FAILURE;
                                 }
                              }
                           
                  if(element_qname)
                  {
                     axutil_qname_free(element_qname, env);
                     element_qname = NULL;
                  }
                 

                     
                     /*
                      * building matchSimilarity element
                      */
                     
                     
                     
                                    /*
                                     * because elements are ordered this works fine
                                     */
                                  
                                   
                                   if(current_node != NULL && is_early_node_valid)
                                   {
                                       current_node = axiom_node_get_next_sibling(current_node, env);
                                       
                                       
                                        while(current_node && axiom_node_get_node_type(current_node, env) != AXIOM_ELEMENT)
                                        {
                                            current_node = axiom_node_get_next_sibling(current_node, env);
                                        }
                                        if(current_node != NULL)
                                        {
                                            current_element = (axiom_element_t *)axiom_node_get_data_element(current_node, env);
                                            qname = axiom_element_get_qname(current_element, env, current_node);
                                        }
                                       
                                   }
                                   is_early_node_valid = AXIS2_FALSE;
                                 
                                 element_qname = axutil_qname_create(env, "matchSimilarity", NULL, NULL);
                                 

                           if ( 
                                (current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("matchSimilarity", axiom_element_get_localname(current_element, env)))))
                           {
                              if( current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("matchSimilarity", axiom_element_get_localname(current_element, env))))
                              {
                                is_early_node_valid = AXIS2_TRUE;
                              }
                              
                                 
                                      text_value = axiom_element_get_text(current_element, env, current_node);
                                      if(text_value != NULL)
                                      {
                                            status = adb_translate_set_matchSimilarity(_translate, env,
                                                                   atoi(text_value));
                                      }
                                      
                                 if(AXIS2_FAILURE ==  status)
                                 {
                                     AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "failed in setting the value for matchSimilarity ");
                                     if(element_qname)
                                     {
                                         axutil_qname_free(element_qname, env);
                                     }
                                     return AXIS2_FAILURE;
                                 }
                              }
                           
                  if(element_qname)
                  {
                     axutil_qname_free(element_qname, env);
                     element_qname = NULL;
                  }
                 

                     
                     /*
                      * building type element
                      */
                     
                     
                     
                                    /*
                                     * because elements are ordered this works fine
                                     */
                                  
                                   
                                   if(current_node != NULL && is_early_node_valid)
                                   {
                                       current_node = axiom_node_get_next_sibling(current_node, env);
                                       
                                       
                                        while(current_node && axiom_node_get_node_type(current_node, env) != AXIOM_ELEMENT)
                                        {
                                            current_node = axiom_node_get_next_sibling(current_node, env);
                                        }
                                        if(current_node != NULL)
                                        {
                                            current_element = (axiom_element_t *)axiom_node_get_data_element(current_node, env);
                                            qname = axiom_element_get_qname(current_element, env, current_node);
                                        }
                                       
                                   }
                                   is_early_node_valid = AXIS2_FALSE;
                                 
                                 element_qname = axutil_qname_create(env, "type", NULL, NULL);
                                 

                           if ( 
                                (current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("type", axiom_element_get_localname(current_element, env)))))
                           {
                              if( current_node   && current_element && (axutil_qname_equals(element_qname, env, qname) || !axutil_strcmp("type", axiom_element_get_localname(current_element, env))))
                              {
                                is_early_node_valid = AXIS2_TRUE;
                              }
                              
                                 
                                      text_value = axiom_element_get_text(current_element, env, current_node);
                                      if(text_value != NULL)
                                      {
                                            status = adb_translate_set_type(_translate, env,
                                                               text_value);
                                      }
                                      
                                 if(AXIS2_FAILURE ==  status)
                                 {
                                     AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "failed in setting the value for type ");
                                     if(element_qname)
                                     {
                                         axutil_qname_free(element_qname, env);
                                     }
                                     return AXIS2_FAILURE;
                                 }
                              }
                           
                  if(element_qname)
                  {
                     axutil_qname_free(element_qname, env);
                     element_qname = NULL;
                  }
                 
          return status;
       }

          axis2_bool_t AXIS2_CALL
          adb_translate_is_particle()
          {
            
                 return AXIS2_FALSE;
              
          }


          void AXIS2_CALL
          adb_translate_declare_parent_namespaces(
                    adb_translate_t* _translate,
                    const axutil_env_t *env, axiom_element_t *parent_element,
                    axutil_hash_t *namespaces, int *next_ns_index)
          {
            
                  /* Here this is an empty function, Nothing to declare */
                 
          }

        
        
        axiom_node_t* AXIS2_CALL
        adb_translate_serialize(
                adb_translate_t* _translate,
                const axutil_env_t *env, axiom_node_t *parent, axiom_element_t *parent_element, int parent_tag_closed, axutil_hash_t *namespaces, int *next_ns_index)
        {
            
            
                return adb_translate_serialize_obj(
                    _translate, env, parent, parent_element, parent_tag_closed, namespaces, next_ns_index);
            
        }

        axiom_node_t* AXIS2_CALL
        adb_translate_serialize_obj(
                adb_translate_t* _translate,
                const axutil_env_t *env, axiom_node_t *parent, axiom_element_t *parent_element, int parent_tag_closed, axutil_hash_t *namespaces, int *next_ns_index)
        {
            
            
         
         axiom_node_t* current_node = NULL;
         int tag_closed = 0;
         
                axiom_namespace_t *ns1 = NULL;

                axis2_char_t *qname_uri = NULL;
                axis2_char_t *qname_prefix = NULL;
                axis2_char_t *p_prefix = NULL;
                axis2_bool_t ns_already_defined;
            
                    axis2_char_t *text_value_1;
                    axis2_char_t *text_value_1_temp;
                    
                    axis2_char_t *text_value_2;
                    axis2_char_t *text_value_2_temp;
                    
                    axis2_char_t *text_value_3;
                    axis2_char_t *text_value_3_temp;
                    
                    axis2_char_t *text_value_4;
                    axis2_char_t *text_value_4_temp;
                    
                    axis2_char_t text_value_5[ADB_DEFAULT_DIGIT_LIMIT];
                    
                    axis2_char_t *text_value_6;
                    axis2_char_t *text_value_6_temp;
                    
               axis2_char_t *start_input_str = NULL;
               axis2_char_t *end_input_str = NULL;
               unsigned int start_input_str_len = 0;
               unsigned int end_input_str_len = 0;
            
            
               axiom_data_source_t *data_source = NULL;
               axutil_stream_t *stream = NULL;

             
                int next_ns_index_value = 0;
            

            AXIS2_ENV_CHECK(env, NULL);
            AXIS2_PARAM_CHECK(env->error, _translate, NULL);
            
             
                    namespaces = axutil_hash_make(env);
                    next_ns_index = &next_ns_index_value;
                     
                           ns1 = axiom_namespace_create (env,
                                             "http://webservices.folt.de/",
                                             "n"); 
                           axutil_hash_set(namespaces, "http://webservices.folt.de/", AXIS2_HASH_KEY_STRING, axutil_strdup(env, "n"));
                       
                     
                    parent_element = axiom_element_create (env, NULL, "translate", ns1 , &parent);
                    
                    
                    axiom_element_set_namespace(parent_element, env, ns1, parent);


            
                    data_source = axiom_data_source_create(env, parent, &current_node);
                    stream = axiom_data_source_get_stream(data_source, env);
                  
                       p_prefix = NULL;
                      

                   if (!_translate->is_valid_sourceSegment)
                   {
                      
                            
                            start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                        (5 + axutil_strlen(p_prefix) + 
                                         axutil_strlen("sourceSegment") + 
                                         axutil_strlen(" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\""))); 
                                        
                            
                            sprintf(start_input_str, "<%s%ssourceSegment xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\"/>",
                                        p_prefix?p_prefix:"",
                                        (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                                        
                            axutil_stream_write(stream, env, start_input_str, axutil_strlen(start_input_str));
                            AXIS2_FREE(env->allocator,start_input_str);
                          
                   }
                   else
                   {
                     start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (4 + axutil_strlen(p_prefix) + 
                                  axutil_strlen("sourceSegment"))); 
                                 
                                 /* axutil_strlen("<:>") + 1 = 4 */
                     end_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (5 + axutil_strlen(p_prefix) + axutil_strlen("sourceSegment")));
                                  /* axutil_strlen("</:>") + 1 = 5 */
                                  
                     

                   
                   
                     
                     /*
                      * parsing sourceSegment element
                      */

                    
                    
                            sprintf(start_input_str, "<%s%ssourceSegment>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                            
                        start_input_str_len = axutil_strlen(start_input_str);
                        sprintf(end_input_str, "</%s%ssourceSegment>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                        end_input_str_len = axutil_strlen(end_input_str);
                    
                           text_value_1 = _translate->property_sourceSegment;
                           
                           axutil_stream_write(stream, env, start_input_str, start_input_str_len);
                           
                            
                           text_value_1_temp = axutil_xml_quote_string(env, text_value_1, AXIS2_TRUE);
                           if (text_value_1_temp)
                           {
                               axutil_stream_write(stream, env, text_value_1_temp, axutil_strlen(text_value_1_temp));
                               AXIS2_FREE(env->allocator, text_value_1_temp);
                           }
                           else
                           {
                               axutil_stream_write(stream, env, text_value_1, axutil_strlen(text_value_1));
                           }
                           
                           axutil_stream_write(stream, env, end_input_str, end_input_str_len);
                           
                     
                     AXIS2_FREE(env->allocator,start_input_str);
                     AXIS2_FREE(env->allocator,end_input_str);
                 } 

                 
                       p_prefix = NULL;
                      

                   if (!_translate->is_valid_sourceLanguage)
                   {
                      
                            
                            start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                        (5 + axutil_strlen(p_prefix) + 
                                         axutil_strlen("sourceLanguage") + 
                                         axutil_strlen(" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\""))); 
                                        
                            
                            sprintf(start_input_str, "<%s%ssourceLanguage xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\"/>",
                                        p_prefix?p_prefix:"",
                                        (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                                        
                            axutil_stream_write(stream, env, start_input_str, axutil_strlen(start_input_str));
                            AXIS2_FREE(env->allocator,start_input_str);
                          
                   }
                   else
                   {
                     start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (4 + axutil_strlen(p_prefix) + 
                                  axutil_strlen("sourceLanguage"))); 
                                 
                                 /* axutil_strlen("<:>") + 1 = 4 */
                     end_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (5 + axutil_strlen(p_prefix) + axutil_strlen("sourceLanguage")));
                                  /* axutil_strlen("</:>") + 1 = 5 */
                                  
                     

                   
                   
                     
                     /*
                      * parsing sourceLanguage element
                      */

                    
                    
                            sprintf(start_input_str, "<%s%ssourceLanguage>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                            
                        start_input_str_len = axutil_strlen(start_input_str);
                        sprintf(end_input_str, "</%s%ssourceLanguage>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                        end_input_str_len = axutil_strlen(end_input_str);
                    
                           text_value_2 = _translate->property_sourceLanguage;
                           
                           axutil_stream_write(stream, env, start_input_str, start_input_str_len);
                           
                            
                           text_value_2_temp = axutil_xml_quote_string(env, text_value_2, AXIS2_TRUE);
                           if (text_value_2_temp)
                           {
                               axutil_stream_write(stream, env, text_value_2_temp, axutil_strlen(text_value_2_temp));
                               AXIS2_FREE(env->allocator, text_value_2_temp);
                           }
                           else
                           {
                               axutil_stream_write(stream, env, text_value_2, axutil_strlen(text_value_2));
                           }
                           
                           axutil_stream_write(stream, env, end_input_str, end_input_str_len);
                           
                     
                     AXIS2_FREE(env->allocator,start_input_str);
                     AXIS2_FREE(env->allocator,end_input_str);
                 } 

                 
                       p_prefix = NULL;
                      

                   if (!_translate->is_valid_targetLanguage)
                   {
                      
                            
                            start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                        (5 + axutil_strlen(p_prefix) + 
                                         axutil_strlen("targetLanguage") + 
                                         axutil_strlen(" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\""))); 
                                        
                            
                            sprintf(start_input_str, "<%s%stargetLanguage xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\"/>",
                                        p_prefix?p_prefix:"",
                                        (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                                        
                            axutil_stream_write(stream, env, start_input_str, axutil_strlen(start_input_str));
                            AXIS2_FREE(env->allocator,start_input_str);
                          
                   }
                   else
                   {
                     start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (4 + axutil_strlen(p_prefix) + 
                                  axutil_strlen("targetLanguage"))); 
                                 
                                 /* axutil_strlen("<:>") + 1 = 4 */
                     end_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (5 + axutil_strlen(p_prefix) + axutil_strlen("targetLanguage")));
                                  /* axutil_strlen("</:>") + 1 = 5 */
                                  
                     

                   
                   
                     
                     /*
                      * parsing targetLanguage element
                      */

                    
                    
                            sprintf(start_input_str, "<%s%stargetLanguage>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                            
                        start_input_str_len = axutil_strlen(start_input_str);
                        sprintf(end_input_str, "</%s%stargetLanguage>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                        end_input_str_len = axutil_strlen(end_input_str);
                    
                           text_value_3 = _translate->property_targetLanguage;
                           
                           axutil_stream_write(stream, env, start_input_str, start_input_str_len);
                           
                            
                           text_value_3_temp = axutil_xml_quote_string(env, text_value_3, AXIS2_TRUE);
                           if (text_value_3_temp)
                           {
                               axutil_stream_write(stream, env, text_value_3_temp, axutil_strlen(text_value_3_temp));
                               AXIS2_FREE(env->allocator, text_value_3_temp);
                           }
                           else
                           {
                               axutil_stream_write(stream, env, text_value_3, axutil_strlen(text_value_3));
                           }
                           
                           axutil_stream_write(stream, env, end_input_str, end_input_str_len);
                           
                     
                     AXIS2_FREE(env->allocator,start_input_str);
                     AXIS2_FREE(env->allocator,end_input_str);
                 } 

                 
                       p_prefix = NULL;
                      

                   if (!_translate->is_valid_datasource)
                   {
                      
                            
                            start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                        (5 + axutil_strlen(p_prefix) + 
                                         axutil_strlen("datasource") + 
                                         axutil_strlen(" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\""))); 
                                        
                            
                            sprintf(start_input_str, "<%s%sdatasource xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\"/>",
                                        p_prefix?p_prefix:"",
                                        (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                                        
                            axutil_stream_write(stream, env, start_input_str, axutil_strlen(start_input_str));
                            AXIS2_FREE(env->allocator,start_input_str);
                          
                   }
                   else
                   {
                     start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (4 + axutil_strlen(p_prefix) + 
                                  axutil_strlen("datasource"))); 
                                 
                                 /* axutil_strlen("<:>") + 1 = 4 */
                     end_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (5 + axutil_strlen(p_prefix) + axutil_strlen("datasource")));
                                  /* axutil_strlen("</:>") + 1 = 5 */
                                  
                     

                   
                   
                     
                     /*
                      * parsing datasource element
                      */

                    
                    
                            sprintf(start_input_str, "<%s%sdatasource>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                            
                        start_input_str_len = axutil_strlen(start_input_str);
                        sprintf(end_input_str, "</%s%sdatasource>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                        end_input_str_len = axutil_strlen(end_input_str);
                    
                           text_value_4 = _translate->property_datasource;
                           
                           axutil_stream_write(stream, env, start_input_str, start_input_str_len);
                           
                            
                           text_value_4_temp = axutil_xml_quote_string(env, text_value_4, AXIS2_TRUE);
                           if (text_value_4_temp)
                           {
                               axutil_stream_write(stream, env, text_value_4_temp, axutil_strlen(text_value_4_temp));
                               AXIS2_FREE(env->allocator, text_value_4_temp);
                           }
                           else
                           {
                               axutil_stream_write(stream, env, text_value_4, axutil_strlen(text_value_4));
                           }
                           
                           axutil_stream_write(stream, env, end_input_str, end_input_str_len);
                           
                     
                     AXIS2_FREE(env->allocator,start_input_str);
                     AXIS2_FREE(env->allocator,end_input_str);
                 } 

                 
                       p_prefix = NULL;
                      

                   if (!_translate->is_valid_matchSimilarity)
                   {
                      
                            
                            start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                        (5 + axutil_strlen(p_prefix) + 
                                         axutil_strlen("matchSimilarity") + 
                                         axutil_strlen(" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\""))); 
                                        
                            
                            sprintf(start_input_str, "<%s%smatchSimilarity xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\"/>",
                                        p_prefix?p_prefix:"",
                                        (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                                        
                            axutil_stream_write(stream, env, start_input_str, axutil_strlen(start_input_str));
                            AXIS2_FREE(env->allocator,start_input_str);
                          
                   }
                   else
                   {
                     start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (4 + axutil_strlen(p_prefix) + 
                                  axutil_strlen("matchSimilarity"))); 
                                 
                                 /* axutil_strlen("<:>") + 1 = 4 */
                     end_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (5 + axutil_strlen(p_prefix) + axutil_strlen("matchSimilarity")));
                                  /* axutil_strlen("</:>") + 1 = 5 */
                                  
                     

                   
                   
                     
                     /*
                      * parsing matchSimilarity element
                      */

                    
                    
                            sprintf(start_input_str, "<%s%smatchSimilarity>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                            
                        start_input_str_len = axutil_strlen(start_input_str);
                        sprintf(end_input_str, "</%s%smatchSimilarity>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                        end_input_str_len = axutil_strlen(end_input_str);
                    
                               sprintf (text_value_5, AXIS2_PRINTF_INT32_FORMAT_SPECIFIER, _translate->property_matchSimilarity);
                             
                           axutil_stream_write(stream, env, start_input_str, start_input_str_len);
                           
                           axutil_stream_write(stream, env, text_value_5, axutil_strlen(text_value_5));
                           
                           axutil_stream_write(stream, env, end_input_str, end_input_str_len);
                           
                     
                     AXIS2_FREE(env->allocator,start_input_str);
                     AXIS2_FREE(env->allocator,end_input_str);
                 } 

                 
                       p_prefix = NULL;
                      

                   if (!_translate->is_valid_type)
                   {
                      
                            
                            start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                        (5 + axutil_strlen(p_prefix) + 
                                         axutil_strlen("type") + 
                                         axutil_strlen(" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\""))); 
                                        
                            
                            sprintf(start_input_str, "<%s%stype xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:nil=\"1\"/>",
                                        p_prefix?p_prefix:"",
                                        (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                                        
                            axutil_stream_write(stream, env, start_input_str, axutil_strlen(start_input_str));
                            AXIS2_FREE(env->allocator,start_input_str);
                          
                   }
                   else
                   {
                     start_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (4 + axutil_strlen(p_prefix) + 
                                  axutil_strlen("type"))); 
                                 
                                 /* axutil_strlen("<:>") + 1 = 4 */
                     end_input_str = (axis2_char_t*)AXIS2_MALLOC(env->allocator, sizeof(axis2_char_t) *
                                 (5 + axutil_strlen(p_prefix) + axutil_strlen("type")));
                                  /* axutil_strlen("</:>") + 1 = 5 */
                                  
                     

                   
                   
                     
                     /*
                      * parsing type element
                      */

                    
                    
                            sprintf(start_input_str, "<%s%stype>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                            
                        start_input_str_len = axutil_strlen(start_input_str);
                        sprintf(end_input_str, "</%s%stype>",
                                 p_prefix?p_prefix:"",
                                 (p_prefix && axutil_strcmp(p_prefix, ""))?":":"");
                        end_input_str_len = axutil_strlen(end_input_str);
                    
                           text_value_6 = _translate->property_type;
                           
                           axutil_stream_write(stream, env, start_input_str, start_input_str_len);
                           
                            
                           text_value_6_temp = axutil_xml_quote_string(env, text_value_6, AXIS2_TRUE);
                           if (text_value_6_temp)
                           {
                               axutil_stream_write(stream, env, text_value_6_temp, axutil_strlen(text_value_6_temp));
                               AXIS2_FREE(env->allocator, text_value_6_temp);
                           }
                           else
                           {
                               axutil_stream_write(stream, env, text_value_6, axutil_strlen(text_value_6));
                           }
                           
                           axutil_stream_write(stream, env, end_input_str, end_input_str_len);
                           
                     
                     AXIS2_FREE(env->allocator,start_input_str);
                     AXIS2_FREE(env->allocator,end_input_str);
                 } 

                 
                   if(namespaces)
                   {
                       axutil_hash_index_t *hi;
                       void *val;
                       for (hi = axutil_hash_first(namespaces, env); hi; hi = axutil_hash_next(env, hi)) 
                       {
                           axutil_hash_this(hi, NULL, NULL, &val);
                           AXIS2_FREE(env->allocator, val);
                       }
                       axutil_hash_free(namespaces, env);
                   }
                

            return parent;
        }


        

            /**
             * Getter for sourceSegment by  Property Number 1
             */
            axis2_char_t* AXIS2_CALL
            adb_translate_get_property1(
                adb_translate_t* _translate,
                const axutil_env_t *env)
            {
                return adb_translate_get_sourceSegment(_translate,
                                             env);
            }

            /**
             * getter for sourceSegment.
             */
            axis2_char_t* AXIS2_CALL
            adb_translate_get_sourceSegment(
                    adb_translate_t* _translate,
                    const axutil_env_t *env)
             {
                
                    AXIS2_ENV_CHECK(env, NULL);
                    AXIS2_PARAM_CHECK(env->error, _translate, NULL);
                  

                return _translate->property_sourceSegment;
             }

            /**
             * setter for sourceSegment
             */
            axis2_status_t AXIS2_CALL
            adb_translate_set_sourceSegment(
                    adb_translate_t* _translate,
                    const axutil_env_t *env,
                    const axis2_char_t*  arg_sourceSegment)
             {
                

                AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
                AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
                
                if(_translate->is_valid_sourceSegment &&
                        arg_sourceSegment == _translate->property_sourceSegment)
                {
                    
                    return AXIS2_SUCCESS; 
                }

                adb_translate_reset_sourceSegment(_translate, env);

                
                if(NULL == arg_sourceSegment)
                {
                    /* We are already done */
                    return AXIS2_SUCCESS;
                }
                _translate->property_sourceSegment = (axis2_char_t *)axutil_strdup(env, arg_sourceSegment);
                        if(NULL == _translate->property_sourceSegment)
                        {
                            AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Error allocating memeory for sourceSegment");
                            return AXIS2_FAILURE;
                        }
                        _translate->is_valid_sourceSegment = AXIS2_TRUE;
                    
                return AXIS2_SUCCESS;
             }

             

           /**
            * resetter for sourceSegment
            */
           axis2_status_t AXIS2_CALL
           adb_translate_reset_sourceSegment(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               int i = 0;
               int count = 0;
               void *element = NULL;

               AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
               

               
            
                
                if(_translate->property_sourceSegment != NULL)
                {
                   
                   
                        AXIS2_FREE(env-> allocator, _translate->property_sourceSegment);
                     _translate->property_sourceSegment = NULL;
                }
            
                
                
                _translate->is_valid_sourceSegment = AXIS2_FALSE; 
               return AXIS2_SUCCESS;
           }

           /**
            * Check whether sourceSegment is nill
            */
           axis2_bool_t AXIS2_CALL
           adb_translate_is_sourceSegment_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               AXIS2_ENV_CHECK(env, AXIS2_TRUE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_TRUE);
               
               return !_translate->is_valid_sourceSegment;
           }

           /**
            * Set sourceSegment to nill (currently the same as reset)
            */
           axis2_status_t AXIS2_CALL
           adb_translate_set_sourceSegment_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               return adb_translate_reset_sourceSegment(_translate, env);
           }

           

            /**
             * Getter for sourceLanguage by  Property Number 2
             */
            axis2_char_t* AXIS2_CALL
            adb_translate_get_property2(
                adb_translate_t* _translate,
                const axutil_env_t *env)
            {
                return adb_translate_get_sourceLanguage(_translate,
                                             env);
            }

            /**
             * getter for sourceLanguage.
             */
            axis2_char_t* AXIS2_CALL
            adb_translate_get_sourceLanguage(
                    adb_translate_t* _translate,
                    const axutil_env_t *env)
             {
                
                    AXIS2_ENV_CHECK(env, NULL);
                    AXIS2_PARAM_CHECK(env->error, _translate, NULL);
                  

                return _translate->property_sourceLanguage;
             }

            /**
             * setter for sourceLanguage
             */
            axis2_status_t AXIS2_CALL
            adb_translate_set_sourceLanguage(
                    adb_translate_t* _translate,
                    const axutil_env_t *env,
                    const axis2_char_t*  arg_sourceLanguage)
             {
                

                AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
                AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
                
                if(_translate->is_valid_sourceLanguage &&
                        arg_sourceLanguage == _translate->property_sourceLanguage)
                {
                    
                    return AXIS2_SUCCESS; 
                }

                adb_translate_reset_sourceLanguage(_translate, env);

                
                if(NULL == arg_sourceLanguage)
                {
                    /* We are already done */
                    return AXIS2_SUCCESS;
                }
                _translate->property_sourceLanguage = (axis2_char_t *)axutil_strdup(env, arg_sourceLanguage);
                        if(NULL == _translate->property_sourceLanguage)
                        {
                            AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Error allocating memeory for sourceLanguage");
                            return AXIS2_FAILURE;
                        }
                        _translate->is_valid_sourceLanguage = AXIS2_TRUE;
                    
                return AXIS2_SUCCESS;
             }

             

           /**
            * resetter for sourceLanguage
            */
           axis2_status_t AXIS2_CALL
           adb_translate_reset_sourceLanguage(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               int i = 0;
               int count = 0;
               void *element = NULL;

               AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
               

               
            
                
                if(_translate->property_sourceLanguage != NULL)
                {
                   
                   
                        AXIS2_FREE(env-> allocator, _translate->property_sourceLanguage);
                     _translate->property_sourceLanguage = NULL;
                }
            
                
                
                _translate->is_valid_sourceLanguage = AXIS2_FALSE; 
               return AXIS2_SUCCESS;
           }

           /**
            * Check whether sourceLanguage is nill
            */
           axis2_bool_t AXIS2_CALL
           adb_translate_is_sourceLanguage_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               AXIS2_ENV_CHECK(env, AXIS2_TRUE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_TRUE);
               
               return !_translate->is_valid_sourceLanguage;
           }

           /**
            * Set sourceLanguage to nill (currently the same as reset)
            */
           axis2_status_t AXIS2_CALL
           adb_translate_set_sourceLanguage_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               return adb_translate_reset_sourceLanguage(_translate, env);
           }

           

            /**
             * Getter for targetLanguage by  Property Number 3
             */
            axis2_char_t* AXIS2_CALL
            adb_translate_get_property3(
                adb_translate_t* _translate,
                const axutil_env_t *env)
            {
                return adb_translate_get_targetLanguage(_translate,
                                             env);
            }

            /**
             * getter for targetLanguage.
             */
            axis2_char_t* AXIS2_CALL
            adb_translate_get_targetLanguage(
                    adb_translate_t* _translate,
                    const axutil_env_t *env)
             {
                
                    AXIS2_ENV_CHECK(env, NULL);
                    AXIS2_PARAM_CHECK(env->error, _translate, NULL);
                  

                return _translate->property_targetLanguage;
             }

            /**
             * setter for targetLanguage
             */
            axis2_status_t AXIS2_CALL
            adb_translate_set_targetLanguage(
                    adb_translate_t* _translate,
                    const axutil_env_t *env,
                    const axis2_char_t*  arg_targetLanguage)
             {
                

                AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
                AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
                
                if(_translate->is_valid_targetLanguage &&
                        arg_targetLanguage == _translate->property_targetLanguage)
                {
                    
                    return AXIS2_SUCCESS; 
                }

                adb_translate_reset_targetLanguage(_translate, env);

                
                if(NULL == arg_targetLanguage)
                {
                    /* We are already done */
                    return AXIS2_SUCCESS;
                }
                _translate->property_targetLanguage = (axis2_char_t *)axutil_strdup(env, arg_targetLanguage);
                        if(NULL == _translate->property_targetLanguage)
                        {
                            AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Error allocating memeory for targetLanguage");
                            return AXIS2_FAILURE;
                        }
                        _translate->is_valid_targetLanguage = AXIS2_TRUE;
                    
                return AXIS2_SUCCESS;
             }

             

           /**
            * resetter for targetLanguage
            */
           axis2_status_t AXIS2_CALL
           adb_translate_reset_targetLanguage(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               int i = 0;
               int count = 0;
               void *element = NULL;

               AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
               

               
            
                
                if(_translate->property_targetLanguage != NULL)
                {
                   
                   
                        AXIS2_FREE(env-> allocator, _translate->property_targetLanguage);
                     _translate->property_targetLanguage = NULL;
                }
            
                
                
                _translate->is_valid_targetLanguage = AXIS2_FALSE; 
               return AXIS2_SUCCESS;
           }

           /**
            * Check whether targetLanguage is nill
            */
           axis2_bool_t AXIS2_CALL
           adb_translate_is_targetLanguage_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               AXIS2_ENV_CHECK(env, AXIS2_TRUE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_TRUE);
               
               return !_translate->is_valid_targetLanguage;
           }

           /**
            * Set targetLanguage to nill (currently the same as reset)
            */
           axis2_status_t AXIS2_CALL
           adb_translate_set_targetLanguage_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               return adb_translate_reset_targetLanguage(_translate, env);
           }

           

            /**
             * Getter for datasource by  Property Number 4
             */
            axis2_char_t* AXIS2_CALL
            adb_translate_get_property4(
                adb_translate_t* _translate,
                const axutil_env_t *env)
            {
                return adb_translate_get_datasource(_translate,
                                             env);
            }

            /**
             * getter for datasource.
             */
            axis2_char_t* AXIS2_CALL
            adb_translate_get_datasource(
                    adb_translate_t* _translate,
                    const axutil_env_t *env)
             {
                
                    AXIS2_ENV_CHECK(env, NULL);
                    AXIS2_PARAM_CHECK(env->error, _translate, NULL);
                  

                return _translate->property_datasource;
             }

            /**
             * setter for datasource
             */
            axis2_status_t AXIS2_CALL
            adb_translate_set_datasource(
                    adb_translate_t* _translate,
                    const axutil_env_t *env,
                    const axis2_char_t*  arg_datasource)
             {
                

                AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
                AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
                
                if(_translate->is_valid_datasource &&
                        arg_datasource == _translate->property_datasource)
                {
                    
                    return AXIS2_SUCCESS; 
                }

                adb_translate_reset_datasource(_translate, env);

                
                if(NULL == arg_datasource)
                {
                    /* We are already done */
                    return AXIS2_SUCCESS;
                }
                _translate->property_datasource = (axis2_char_t *)axutil_strdup(env, arg_datasource);
                        if(NULL == _translate->property_datasource)
                        {
                            AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Error allocating memeory for datasource");
                            return AXIS2_FAILURE;
                        }
                        _translate->is_valid_datasource = AXIS2_TRUE;
                    
                return AXIS2_SUCCESS;
             }

             

           /**
            * resetter for datasource
            */
           axis2_status_t AXIS2_CALL
           adb_translate_reset_datasource(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               int i = 0;
               int count = 0;
               void *element = NULL;

               AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
               

               
            
                
                if(_translate->property_datasource != NULL)
                {
                   
                   
                        AXIS2_FREE(env-> allocator, _translate->property_datasource);
                     _translate->property_datasource = NULL;
                }
            
                
                
                _translate->is_valid_datasource = AXIS2_FALSE; 
               return AXIS2_SUCCESS;
           }

           /**
            * Check whether datasource is nill
            */
           axis2_bool_t AXIS2_CALL
           adb_translate_is_datasource_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               AXIS2_ENV_CHECK(env, AXIS2_TRUE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_TRUE);
               
               return !_translate->is_valid_datasource;
           }

           /**
            * Set datasource to nill (currently the same as reset)
            */
           axis2_status_t AXIS2_CALL
           adb_translate_set_datasource_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               return adb_translate_reset_datasource(_translate, env);
           }

           

            /**
             * Getter for matchSimilarity by  Property Number 5
             */
            int AXIS2_CALL
            adb_translate_get_property5(
                adb_translate_t* _translate,
                const axutil_env_t *env)
            {
                return adb_translate_get_matchSimilarity(_translate,
                                             env);
            }

            /**
             * getter for matchSimilarity.
             */
            int AXIS2_CALL
            adb_translate_get_matchSimilarity(
                    adb_translate_t* _translate,
                    const axutil_env_t *env)
             {
                
                    AXIS2_ENV_CHECK(env, (int)0);
                    AXIS2_PARAM_CHECK(env->error, _translate, (int)0);
                  

                return _translate->property_matchSimilarity;
             }

            /**
             * setter for matchSimilarity
             */
            axis2_status_t AXIS2_CALL
            adb_translate_set_matchSimilarity(
                    adb_translate_t* _translate,
                    const axutil_env_t *env,
                    const int  arg_matchSimilarity)
             {
                

                AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
                AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
                
                if(_translate->is_valid_matchSimilarity &&
                        arg_matchSimilarity == _translate->property_matchSimilarity)
                {
                    
                    return AXIS2_SUCCESS; 
                }

                adb_translate_reset_matchSimilarity(_translate, env);

                _translate->property_matchSimilarity = arg_matchSimilarity;
                        _translate->is_valid_matchSimilarity = AXIS2_TRUE;
                    
                return AXIS2_SUCCESS;
             }

             

           /**
            * resetter for matchSimilarity
            */
           axis2_status_t AXIS2_CALL
           adb_translate_reset_matchSimilarity(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               int i = 0;
               int count = 0;
               void *element = NULL;

               AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
               

               _translate->is_valid_matchSimilarity = AXIS2_FALSE; 
               return AXIS2_SUCCESS;
           }

           /**
            * Check whether matchSimilarity is nill
            */
           axis2_bool_t AXIS2_CALL
           adb_translate_is_matchSimilarity_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               AXIS2_ENV_CHECK(env, AXIS2_TRUE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_TRUE);
               
               return !_translate->is_valid_matchSimilarity;
           }

           /**
            * Set matchSimilarity to nill (currently the same as reset)
            */
           axis2_status_t AXIS2_CALL
           adb_translate_set_matchSimilarity_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               return adb_translate_reset_matchSimilarity(_translate, env);
           }

           

            /**
             * Getter for type by  Property Number 6
             */
            axis2_char_t* AXIS2_CALL
            adb_translate_get_property6(
                adb_translate_t* _translate,
                const axutil_env_t *env)
            {
                return adb_translate_get_type(_translate,
                                             env);
            }

            /**
             * getter for type.
             */
            axis2_char_t* AXIS2_CALL
            adb_translate_get_type(
                    adb_translate_t* _translate,
                    const axutil_env_t *env)
             {
                
                    AXIS2_ENV_CHECK(env, NULL);
                    AXIS2_PARAM_CHECK(env->error, _translate, NULL);
                  

                return _translate->property_type;
             }

            /**
             * setter for type
             */
            axis2_status_t AXIS2_CALL
            adb_translate_set_type(
                    adb_translate_t* _translate,
                    const axutil_env_t *env,
                    const axis2_char_t*  arg_type)
             {
                

                AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
                AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
                
                if(_translate->is_valid_type &&
                        arg_type == _translate->property_type)
                {
                    
                    return AXIS2_SUCCESS; 
                }

                adb_translate_reset_type(_translate, env);

                
                if(NULL == arg_type)
                {
                    /* We are already done */
                    return AXIS2_SUCCESS;
                }
                _translate->property_type = (axis2_char_t *)axutil_strdup(env, arg_type);
                        if(NULL == _translate->property_type)
                        {
                            AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Error allocating memeory for type");
                            return AXIS2_FAILURE;
                        }
                        _translate->is_valid_type = AXIS2_TRUE;
                    
                return AXIS2_SUCCESS;
             }

             

           /**
            * resetter for type
            */
           axis2_status_t AXIS2_CALL
           adb_translate_reset_type(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               int i = 0;
               int count = 0;
               void *element = NULL;

               AXIS2_ENV_CHECK(env, AXIS2_FAILURE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_FAILURE);
               

               
            
                
                if(_translate->property_type != NULL)
                {
                   
                   
                        AXIS2_FREE(env-> allocator, _translate->property_type);
                     _translate->property_type = NULL;
                }
            
                
                
                _translate->is_valid_type = AXIS2_FALSE; 
               return AXIS2_SUCCESS;
           }

           /**
            * Check whether type is nill
            */
           axis2_bool_t AXIS2_CALL
           adb_translate_is_type_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               AXIS2_ENV_CHECK(env, AXIS2_TRUE);
               AXIS2_PARAM_CHECK(env->error, _translate, AXIS2_TRUE);
               
               return !_translate->is_valid_type;
           }

           /**
            * Set type to nill (currently the same as reset)
            */
           axis2_status_t AXIS2_CALL
           adb_translate_set_type_nil(
                   adb_translate_t* _translate,
                   const axutil_env_t *env)
           {
               return adb_translate_reset_type(_translate, env);
           }

           

