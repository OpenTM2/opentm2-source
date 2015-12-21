//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+


        #ifndef ADB_TRANSLATE_H
        #define ADB_TRANSLATE_H

       /**
        * adb_translate.h
        *
        * This file was auto-generated from WSDL
        * by the Apache Axis2/Java version: 1.6.1  Built on : Aug 31, 2011 (12:23:23 CEST)
        */

       /**
        *  adb_translate class
        */

        

        #include <stdio.h>
        #include <axiom.h>
        #include <axis2_util.h>
        #include <axiom_soap.h>
        #include <axis2_client.h>

        #include "axis2_extension_mapper.h"

        #ifdef __cplusplus
        extern "C"
        {
        #endif

        #define ADB_DEFAULT_DIGIT_LIMIT 1024
        #define ADB_DEFAULT_NAMESPACE_PREFIX_LIMIT 64
        

        typedef struct adb_translate adb_translate_t;

        
        

        /******************************* Create and Free functions *********************************/

        /**
         * Constructor for creating adb_translate_t
         * @param env pointer to environment struct
         * @return newly created adb_translate_t object
         */
        adb_translate_t* AXIS2_CALL
        adb_translate_create(
            const axutil_env_t *env );

        /**
         * Wrapper for the "free" function, will invoke the extension mapper instead
         * @param  _translate adb_translate_t object to free
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_free (
            adb_translate_t* _translate,
            const axutil_env_t *env);

        /**
         * Free adb_translate_t object
         * @param  _translate adb_translate_t object to free
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_free_obj (
            adb_translate_t* _translate,
            const axutil_env_t *env);



        /********************************** Getters and Setters **************************************/
        
        

        /**
         * Getter for sourceSegment. 
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_translate_get_sourceSegment(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        /**
         * Setter for sourceSegment.
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param arg_sourceSegment axis2_char_t*
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_sourceSegment(
            adb_translate_t* _translate,
            const axutil_env_t *env,
            const axis2_char_t*  arg_sourceSegment);

        /**
         * Resetter for sourceSegment
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_reset_sourceSegment(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        
        

        /**
         * Getter for sourceLanguage. 
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_translate_get_sourceLanguage(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        /**
         * Setter for sourceLanguage.
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param arg_sourceLanguage axis2_char_t*
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_sourceLanguage(
            adb_translate_t* _translate,
            const axutil_env_t *env,
            const axis2_char_t*  arg_sourceLanguage);

        /**
         * Resetter for sourceLanguage
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_reset_sourceLanguage(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        
        

        /**
         * Getter for targetLanguage. 
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_translate_get_targetLanguage(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        /**
         * Setter for targetLanguage.
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param arg_targetLanguage axis2_char_t*
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_targetLanguage(
            adb_translate_t* _translate,
            const axutil_env_t *env,
            const axis2_char_t*  arg_targetLanguage);

        /**
         * Resetter for targetLanguage
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_reset_targetLanguage(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        
        

        /**
         * Getter for datasource. 
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_translate_get_datasource(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        /**
         * Setter for datasource.
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param arg_datasource axis2_char_t*
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_datasource(
            adb_translate_t* _translate,
            const axutil_env_t *env,
            const axis2_char_t*  arg_datasource);

        /**
         * Resetter for datasource
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_reset_datasource(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        
        

        /**
         * Getter for matchSimilarity. 
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return int
         */
        int AXIS2_CALL
        adb_translate_get_matchSimilarity(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        /**
         * Setter for matchSimilarity.
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param arg_matchSimilarity int
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_matchSimilarity(
            adb_translate_t* _translate,
            const axutil_env_t *env,
            const int  arg_matchSimilarity);

        /**
         * Resetter for matchSimilarity
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_reset_matchSimilarity(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        
        

        /**
         * Getter for type. 
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_translate_get_type(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        /**
         * Setter for type.
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param arg_type axis2_char_t*
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_type(
            adb_translate_t* _translate,
            const axutil_env_t *env,
            const axis2_char_t*  arg_type);

        /**
         * Resetter for type
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_reset_type(
            adb_translate_t* _translate,
            const axutil_env_t *env);

        


        /******************************* Checking and Setting NIL values *********************************/
        

        /**
         * NOTE: set_nil is only available for nillable properties
         */

        

        /**
         * Check whether sourceSegment is nill
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_TRUE if the element is nil or AXIS2_FALSE otherwise
         */
        axis2_bool_t AXIS2_CALL
        adb_translate_is_sourceSegment_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);


        
        /**
         * Set sourceSegment to nill (currently the same as reset)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_sourceSegment_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);
        

        /**
         * Check whether sourceLanguage is nill
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_TRUE if the element is nil or AXIS2_FALSE otherwise
         */
        axis2_bool_t AXIS2_CALL
        adb_translate_is_sourceLanguage_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);


        
        /**
         * Set sourceLanguage to nill (currently the same as reset)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_sourceLanguage_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);
        

        /**
         * Check whether targetLanguage is nill
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_TRUE if the element is nil or AXIS2_FALSE otherwise
         */
        axis2_bool_t AXIS2_CALL
        adb_translate_is_targetLanguage_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);


        
        /**
         * Set targetLanguage to nill (currently the same as reset)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_targetLanguage_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);
        

        /**
         * Check whether datasource is nill
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_TRUE if the element is nil or AXIS2_FALSE otherwise
         */
        axis2_bool_t AXIS2_CALL
        adb_translate_is_datasource_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);


        
        /**
         * Set datasource to nill (currently the same as reset)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_datasource_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);
        

        /**
         * Check whether matchSimilarity is nill
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_TRUE if the element is nil or AXIS2_FALSE otherwise
         */
        axis2_bool_t AXIS2_CALL
        adb_translate_is_matchSimilarity_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);


        
        /**
         * Set matchSimilarity to nill (currently the same as reset)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_matchSimilarity_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);
        

        /**
         * Check whether type is nill
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_TRUE if the element is nil or AXIS2_FALSE otherwise
         */
        axis2_bool_t AXIS2_CALL
        adb_translate_is_type_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);


        
        /**
         * Set type to nill (currently the same as reset)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_set_type_nil(
                adb_translate_t* _translate,
                const axutil_env_t *env);
        

        /**************************** Serialize and Deserialize functions ***************************/
        /*********** These functions are for use only inside the generated code *********************/

        
        /**
         * Wrapper for the deserialization function, will invoke the extension mapper instead
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param dp_parent double pointer to the parent node to deserialize
         * @param dp_is_early_node_valid double pointer to a flag (is_early_node_valid?)
         * @param dont_care_minoccurs Dont set errors on validating minoccurs, 
         *              (Parent will order this in a case of choice)
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_deserialize(
            adb_translate_t* _translate,
            const axutil_env_t *env,
            axiom_node_t** dp_parent,
            axis2_bool_t *dp_is_early_node_valid,
            axis2_bool_t dont_care_minoccurs);

        /**
         * Deserialize an XML to adb objects
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param dp_parent double pointer to the parent node to deserialize
         * @param dp_is_early_node_valid double pointer to a flag (is_early_node_valid?)
         * @param dont_care_minoccurs Dont set errors on validating minoccurs,
         *              (Parent will order this in a case of choice)
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_translate_deserialize_obj(
            adb_translate_t* _translate,
            const axutil_env_t *env,
            axiom_node_t** dp_parent,
            axis2_bool_t *dp_is_early_node_valid,
            axis2_bool_t dont_care_minoccurs);
                            
            

       /**
         * Declare namespace in the most parent node 
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param parent_element parent element
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index pointer to an int which contain the next namespace index
         */
       void AXIS2_CALL
       adb_translate_declare_parent_namespaces(
                    adb_translate_t* _translate,
                    const axutil_env_t *env, axiom_element_t *parent_element,
                    axutil_hash_t *namespaces, int *next_ns_index);

        

        /**
         * Wrapper for the serialization function, will invoke the extension mapper instead
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param translate_om_node node to serialize from
         * @param translate_om_element parent element to serialize from
         * @param tag_closed whether the parent tag is closed or not
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index an int which contain the next namespace index
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axiom_node_t* AXIS2_CALL
        adb_translate_serialize(
            adb_translate_t* _translate,
            const axutil_env_t *env,
            axiom_node_t* translate_om_node, axiom_element_t *translate_om_element, int tag_closed, axutil_hash_t *namespaces, int *next_ns_index);

        /**
         * Serialize to an XML from the adb objects
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @param translate_om_node node to serialize from
         * @param translate_om_element parent element to serialize from
         * @param tag_closed whether the parent tag is closed or not
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index an int which contain the next namespace index
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axiom_node_t* AXIS2_CALL
        adb_translate_serialize_obj(
            adb_translate_t* _translate,
            const axutil_env_t *env,
            axiom_node_t* translate_om_node, axiom_element_t *translate_om_element, int tag_closed, axutil_hash_t *namespaces, int *next_ns_index);

        /**
         * Check whether the adb_translate is a particle class (E.g. group, inner sequence)
         * @return whether this is a particle class.
         */
        axis2_bool_t AXIS2_CALL
        adb_translate_is_particle();

        /******************************* Alternatives for Create and Free functions *********************************/

        

        /**
         * Constructor for creating adb_translate_t
         * @param env pointer to environment struct
         * @param _sourceSegment axis2_char_t*
         * @param _sourceLanguage axis2_char_t*
         * @param _targetLanguage axis2_char_t*
         * @param _datasource axis2_char_t*
         * @param _matchSimilarity int
         * @param _type axis2_char_t*
         * @return newly created adb_translate_t object
         */
        adb_translate_t* AXIS2_CALL
        adb_translate_create_with_values(
            const axutil_env_t *env,
                axis2_char_t* _sourceSegment,
                axis2_char_t* _sourceLanguage,
                axis2_char_t* _targetLanguage,
                axis2_char_t* _datasource,
                int _matchSimilarity,
                axis2_char_t* _type);

        


                /**
                 * Free adb_translate_t object and return the property value.
                 * You can use this to free the adb object as returning the property value. If there are
                 * many properties, it will only return the first property. Other properties will get freed with the adb object.
                 * @param  _translate adb_translate_t object to free
                 * @param env pointer to environment struct
                 * @return the property value holded by the ADB object, if there are many properties only returns the first.
                 */
                axis2_char_t* AXIS2_CALL
                adb_translate_free_popping_value(
                        adb_translate_t* _translate,
                        const axutil_env_t *env);
            

        /******************************* get the value by the property number  *********************************/
        /************NOTE: This method is introduced to resolve a problem in unwrapping mode *******************/

        
        

        /**
         * Getter for sourceSegment by property number (1)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_translate_get_property1(
            adb_translate_t* _translate,
            const axutil_env_t *env);

    
        

        /**
         * Getter for sourceLanguage by property number (2)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_translate_get_property2(
            adb_translate_t* _translate,
            const axutil_env_t *env);

    
        

        /**
         * Getter for targetLanguage by property number (3)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_translate_get_property3(
            adb_translate_t* _translate,
            const axutil_env_t *env);

    
        

        /**
         * Getter for datasource by property number (4)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_translate_get_property4(
            adb_translate_t* _translate,
            const axutil_env_t *env);

    
        

        /**
         * Getter for matchSimilarity by property number (5)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return int
         */
        int AXIS2_CALL
        adb_translate_get_property5(
            adb_translate_t* _translate,
            const axutil_env_t *env);

    
        

        /**
         * Getter for type by property number (6)
         * @param  _translate adb_translate_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_translate_get_property6(
            adb_translate_t* _translate,
            const axutil_env_t *env);

    
     #ifdef __cplusplus
     }
     #endif

     #endif /* ADB_TRANSLATE_H */
    

