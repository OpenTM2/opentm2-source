//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+


        #ifndef ADB_GETLANGUAGES_H
        #define ADB_GETLANGUAGES_H

       /**
        * adb_getLanguages.h
        *
        * This file was auto-generated from WSDL
        * by the Apache Axis2/Java version: 1.6.1  Built on : Aug 31, 2011 (12:23:23 CEST)
        */

       /**
        *  adb_getLanguages class
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
        

        typedef struct adb_getLanguages adb_getLanguages_t;

        
        

        /******************************* Create and Free functions *********************************/

        /**
         * Constructor for creating adb_getLanguages_t
         * @param env pointer to environment struct
         * @return newly created adb_getLanguages_t object
         */
        adb_getLanguages_t* AXIS2_CALL
        adb_getLanguages_create(
            const axutil_env_t *env );

        /**
         * Wrapper for the "free" function, will invoke the extension mapper instead
         * @param  _getLanguages adb_getLanguages_t object to free
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getLanguages_free (
            adb_getLanguages_t* _getLanguages,
            const axutil_env_t *env);

        /**
         * Free adb_getLanguages_t object
         * @param  _getLanguages adb_getLanguages_t object to free
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getLanguages_free_obj (
            adb_getLanguages_t* _getLanguages,
            const axutil_env_t *env);



        /********************************** Getters and Setters **************************************/
        


        /******************************* Checking and Setting NIL values *********************************/
        

        /**
         * NOTE: set_nil is only available for nillable properties
         */

        

        /**************************** Serialize and Deserialize functions ***************************/
        /*********** These functions are for use only inside the generated code *********************/

        
        /**
         * Wrapper for the deserialization function, will invoke the extension mapper instead
         * @param  _getLanguages adb_getLanguages_t object
         * @param env pointer to environment struct
         * @param dp_parent double pointer to the parent node to deserialize
         * @param dp_is_early_node_valid double pointer to a flag (is_early_node_valid?)
         * @param dont_care_minoccurs Dont set errors on validating minoccurs, 
         *              (Parent will order this in a case of choice)
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getLanguages_deserialize(
            adb_getLanguages_t* _getLanguages,
            const axutil_env_t *env,
            axiom_node_t** dp_parent,
            axis2_bool_t *dp_is_early_node_valid,
            axis2_bool_t dont_care_minoccurs);

        /**
         * Deserialize an XML to adb objects
         * @param  _getLanguages adb_getLanguages_t object
         * @param env pointer to environment struct
         * @param dp_parent double pointer to the parent node to deserialize
         * @param dp_is_early_node_valid double pointer to a flag (is_early_node_valid?)
         * @param dont_care_minoccurs Dont set errors on validating minoccurs,
         *              (Parent will order this in a case of choice)
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getLanguages_deserialize_obj(
            adb_getLanguages_t* _getLanguages,
            const axutil_env_t *env,
            axiom_node_t** dp_parent,
            axis2_bool_t *dp_is_early_node_valid,
            axis2_bool_t dont_care_minoccurs);
                            
            

       /**
         * Declare namespace in the most parent node 
         * @param  _getLanguages adb_getLanguages_t object
         * @param env pointer to environment struct
         * @param parent_element parent element
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index pointer to an int which contain the next namespace index
         */
       void AXIS2_CALL
       adb_getLanguages_declare_parent_namespaces(
                    adb_getLanguages_t* _getLanguages,
                    const axutil_env_t *env, axiom_element_t *parent_element,
                    axutil_hash_t *namespaces, int *next_ns_index);

        

        /**
         * Wrapper for the serialization function, will invoke the extension mapper instead
         * @param  _getLanguages adb_getLanguages_t object
         * @param env pointer to environment struct
         * @param getLanguages_om_node node to serialize from
         * @param getLanguages_om_element parent element to serialize from
         * @param tag_closed whether the parent tag is closed or not
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index an int which contain the next namespace index
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axiom_node_t* AXIS2_CALL
        adb_getLanguages_serialize(
            adb_getLanguages_t* _getLanguages,
            const axutil_env_t *env,
            axiom_node_t* getLanguages_om_node, axiom_element_t *getLanguages_om_element, int tag_closed, axutil_hash_t *namespaces, int *next_ns_index);

        /**
         * Serialize to an XML from the adb objects
         * @param  _getLanguages adb_getLanguages_t object
         * @param env pointer to environment struct
         * @param getLanguages_om_node node to serialize from
         * @param getLanguages_om_element parent element to serialize from
         * @param tag_closed whether the parent tag is closed or not
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index an int which contain the next namespace index
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axiom_node_t* AXIS2_CALL
        adb_getLanguages_serialize_obj(
            adb_getLanguages_t* _getLanguages,
            const axutil_env_t *env,
            axiom_node_t* getLanguages_om_node, axiom_element_t *getLanguages_om_element, int tag_closed, axutil_hash_t *namespaces, int *next_ns_index);

        /**
         * Check whether the adb_getLanguages is a particle class (E.g. group, inner sequence)
         * @return whether this is a particle class.
         */
        axis2_bool_t AXIS2_CALL
        adb_getLanguages_is_particle();

        /******************************* Alternatives for Create and Free functions *********************************/

        

        /**
         * Constructor for creating adb_getLanguages_t
         * @param env pointer to environment struct
         * @return newly created adb_getLanguages_t object
         */
        adb_getLanguages_t* AXIS2_CALL
        adb_getLanguages_create_with_values(
            const axutil_env_t *env);

        

                /**
                 * Free adb_getLanguages_t object and return the property value.
                 * You can use this to free the adb object as returning the property value. If there are
                 * many properties, it will only return the first property. Other properties will get freed with the adb object.
                 * @param  _getLanguages adb_getLanguages_t object to free
                 * @param env pointer to environment struct
                 * @return the property value holded by the ADB object, if there are many properties only returns the first.
                 */
                void* AXIS2_CALL
                adb_getLanguages_free_popping_value(
                        adb_getLanguages_t* _getLanguages,
                        const axutil_env_t *env);
            

        /******************************* get the value by the property number  *********************************/
        /************NOTE: This method is introduced to resolve a problem in unwrapping mode *******************/

        
     #ifdef __cplusplus
     }
     #endif

     #endif /* ADB_GETLANGUAGES_H */
    

