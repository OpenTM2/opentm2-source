//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+


        #ifndef ADB_GETMULTILINGUALOBJECT_H
        #define ADB_GETMULTILINGUALOBJECT_H

       /**
        * adb_getMultilingualObject.h
        *
        * This file was auto-generated from WSDL
        * by the Apache Axis2/Java version: 1.6.1  Built on : Aug 31, 2011 (12:23:23 CEST)
        */

       /**
        *  adb_getMultilingualObject class
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
        

        typedef struct adb_getMultilingualObject adb_getMultilingualObject_t;

        
        

        /******************************* Create and Free functions *********************************/

        /**
         * Constructor for creating adb_getMultilingualObject_t
         * @param env pointer to environment struct
         * @return newly created adb_getMultilingualObject_t object
         */
        adb_getMultilingualObject_t* AXIS2_CALL
        adb_getMultilingualObject_create(
            const axutil_env_t *env );

        /**
         * Wrapper for the "free" function, will invoke the extension mapper instead
         * @param  _getMultilingualObject adb_getMultilingualObject_t object to free
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getMultilingualObject_free (
            adb_getMultilingualObject_t* _getMultilingualObject,
            const axutil_env_t *env);

        /**
         * Free adb_getMultilingualObject_t object
         * @param  _getMultilingualObject adb_getMultilingualObject_t object to free
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getMultilingualObject_free_obj (
            adb_getMultilingualObject_t* _getMultilingualObject,
            const axutil_env_t *env);



        /********************************** Getters and Setters **************************************/
        
        

        /**
         * Getter for id. 
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_getMultilingualObject_get_id(
            adb_getMultilingualObject_t* _getMultilingualObject,
            const axutil_env_t *env);

        /**
         * Setter for id.
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @param arg_id axis2_char_t*
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getMultilingualObject_set_id(
            adb_getMultilingualObject_t* _getMultilingualObject,
            const axutil_env_t *env,
            const axis2_char_t*  arg_id);

        /**
         * Resetter for id
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getMultilingualObject_reset_id(
            adb_getMultilingualObject_t* _getMultilingualObject,
            const axutil_env_t *env);

        


        /******************************* Checking and Setting NIL values *********************************/
        

        /**
         * NOTE: set_nil is only available for nillable properties
         */

        

        /**
         * Check whether id is nill
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @return AXIS2_TRUE if the element is nil or AXIS2_FALSE otherwise
         */
        axis2_bool_t AXIS2_CALL
        adb_getMultilingualObject_is_id_nil(
                adb_getMultilingualObject_t* _getMultilingualObject,
                const axutil_env_t *env);


        
        /**
         * Set id to nill (currently the same as reset)
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getMultilingualObject_set_id_nil(
                adb_getMultilingualObject_t* _getMultilingualObject,
                const axutil_env_t *env);
        

        /**************************** Serialize and Deserialize functions ***************************/
        /*********** These functions are for use only inside the generated code *********************/

        
        /**
         * Wrapper for the deserialization function, will invoke the extension mapper instead
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @param dp_parent double pointer to the parent node to deserialize
         * @param dp_is_early_node_valid double pointer to a flag (is_early_node_valid?)
         * @param dont_care_minoccurs Dont set errors on validating minoccurs, 
         *              (Parent will order this in a case of choice)
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getMultilingualObject_deserialize(
            adb_getMultilingualObject_t* _getMultilingualObject,
            const axutil_env_t *env,
            axiom_node_t** dp_parent,
            axis2_bool_t *dp_is_early_node_valid,
            axis2_bool_t dont_care_minoccurs);

        /**
         * Deserialize an XML to adb objects
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @param dp_parent double pointer to the parent node to deserialize
         * @param dp_is_early_node_valid double pointer to a flag (is_early_node_valid?)
         * @param dont_care_minoccurs Dont set errors on validating minoccurs,
         *              (Parent will order this in a case of choice)
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_getMultilingualObject_deserialize_obj(
            adb_getMultilingualObject_t* _getMultilingualObject,
            const axutil_env_t *env,
            axiom_node_t** dp_parent,
            axis2_bool_t *dp_is_early_node_valid,
            axis2_bool_t dont_care_minoccurs);
                            
            

       /**
         * Declare namespace in the most parent node 
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @param parent_element parent element
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index pointer to an int which contain the next namespace index
         */
       void AXIS2_CALL
       adb_getMultilingualObject_declare_parent_namespaces(
                    adb_getMultilingualObject_t* _getMultilingualObject,
                    const axutil_env_t *env, axiom_element_t *parent_element,
                    axutil_hash_t *namespaces, int *next_ns_index);

        

        /**
         * Wrapper for the serialization function, will invoke the extension mapper instead
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @param getMultilingualObject_om_node node to serialize from
         * @param getMultilingualObject_om_element parent element to serialize from
         * @param tag_closed whether the parent tag is closed or not
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index an int which contain the next namespace index
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axiom_node_t* AXIS2_CALL
        adb_getMultilingualObject_serialize(
            adb_getMultilingualObject_t* _getMultilingualObject,
            const axutil_env_t *env,
            axiom_node_t* getMultilingualObject_om_node, axiom_element_t *getMultilingualObject_om_element, int tag_closed, axutil_hash_t *namespaces, int *next_ns_index);

        /**
         * Serialize to an XML from the adb objects
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @param getMultilingualObject_om_node node to serialize from
         * @param getMultilingualObject_om_element parent element to serialize from
         * @param tag_closed whether the parent tag is closed or not
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index an int which contain the next namespace index
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axiom_node_t* AXIS2_CALL
        adb_getMultilingualObject_serialize_obj(
            adb_getMultilingualObject_t* _getMultilingualObject,
            const axutil_env_t *env,
            axiom_node_t* getMultilingualObject_om_node, axiom_element_t *getMultilingualObject_om_element, int tag_closed, axutil_hash_t *namespaces, int *next_ns_index);

        /**
         * Check whether the adb_getMultilingualObject is a particle class (E.g. group, inner sequence)
         * @return whether this is a particle class.
         */
        axis2_bool_t AXIS2_CALL
        adb_getMultilingualObject_is_particle();

        /******************************* Alternatives for Create and Free functions *********************************/

        

        /**
         * Constructor for creating adb_getMultilingualObject_t
         * @param env pointer to environment struct
         * @param _id axis2_char_t*
         * @return newly created adb_getMultilingualObject_t object
         */
        adb_getMultilingualObject_t* AXIS2_CALL
        adb_getMultilingualObject_create_with_values(
            const axutil_env_t *env,
                axis2_char_t* _id);

        


                /**
                 * Free adb_getMultilingualObject_t object and return the property value.
                 * You can use this to free the adb object as returning the property value. If there are
                 * many properties, it will only return the first property. Other properties will get freed with the adb object.
                 * @param  _getMultilingualObject adb_getMultilingualObject_t object to free
                 * @param env pointer to environment struct
                 * @return the property value holded by the ADB object, if there are many properties only returns the first.
                 */
                axis2_char_t* AXIS2_CALL
                adb_getMultilingualObject_free_popping_value(
                        adb_getMultilingualObject_t* _getMultilingualObject,
                        const axutil_env_t *env);
            

        /******************************* get the value by the property number  *********************************/
        /************NOTE: This method is introduced to resolve a problem in unwrapping mode *******************/

        
        

        /**
         * Getter for id by property number (1)
         * @param  _getMultilingualObject adb_getMultilingualObject_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_getMultilingualObject_get_property1(
            adb_getMultilingualObject_t* _getMultilingualObject,
            const axutil_env_t *env);

    
     #ifdef __cplusplus
     }
     #endif

     #endif /* ADB_GETMULTILINGUALOBJECT_H */
    

