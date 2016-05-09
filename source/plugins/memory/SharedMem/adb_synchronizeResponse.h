//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

        #ifndef ADB_SYNCHRONIZERESPONSE_H
        #define ADB_SYNCHRONIZERESPONSE_H

       /**
        * adb_synchronizeResponse.h
        *
        * This file was auto-generated from WSDL
        * by the Apache Axis2/Java version: 1.6.4  Built on : Dec 28, 2015 (10:04:10 GMT)
        */

       /**
        *  adb_synchronizeResponse class
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
        

        typedef struct adb_synchronizeResponse adb_synchronizeResponse_t;

        
        

        /******************************* Create and Free functions *********************************/

        /**
         * Constructor for creating adb_synchronizeResponse_t
         * @param env pointer to environment struct
         * @return newly created adb_synchronizeResponse_t object
         */
        adb_synchronizeResponse_t* AXIS2_CALL
        adb_synchronizeResponse_create(
            const axutil_env_t *env );

        /**
         * Wrapper for the "free" function, will invoke the extension mapper instead
         * @param  _synchronizeResponse adb_synchronizeResponse_t object to free
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_synchronizeResponse_free (
            adb_synchronizeResponse_t* _synchronizeResponse,
            const axutil_env_t *env);

        /**
         * Free adb_synchronizeResponse_t object
         * @param  _synchronizeResponse adb_synchronizeResponse_t object to free
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_synchronizeResponse_free_obj (
            adb_synchronizeResponse_t* _synchronizeResponse,
            const axutil_env_t *env);



        /********************************** Getters and Setters **************************************/
        
        

        /**
         * Getter for return. 
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_synchronizeResponse_get_return(
            adb_synchronizeResponse_t* _synchronizeResponse,
            const axutil_env_t *env);

        /**
         * Setter for return.
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @param arg_return axis2_char_t*
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_synchronizeResponse_set_return(
            adb_synchronizeResponse_t* _synchronizeResponse,
            const axutil_env_t *env,
            const axis2_char_t*  arg_return);

        /**
         * Resetter for return
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_synchronizeResponse_reset_return(
            adb_synchronizeResponse_t* _synchronizeResponse,
            const axutil_env_t *env);

        


        /******************************* Checking and Setting NIL values *********************************/
        

        /**
         * NOTE: set_nil is only available for nillable properties
         */

        

        /**
         * Check whether return is nill
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @return AXIS2_TRUE if the element is nil or AXIS2_FALSE otherwise
         */
        axis2_bool_t AXIS2_CALL
        adb_synchronizeResponse_is_return_nil(
                adb_synchronizeResponse_t* _synchronizeResponse,
                const axutil_env_t *env);


        
        /**
         * Set return to nill (currently the same as reset)
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_synchronizeResponse_set_return_nil(
                adb_synchronizeResponse_t* _synchronizeResponse,
                const axutil_env_t *env);
        

        /**************************** Serialize and Deserialize functions ***************************/
        /*********** These functions are for use only inside the generated code *********************/

        
        /**
         * Wrapper for the deserialization function, will invoke the extension mapper instead
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @param dp_parent double pointer to the parent node to deserialize
         * @param dp_is_early_node_valid double pointer to a flag (is_early_node_valid?)
         * @param dont_care_minoccurs Dont set errors on validating minoccurs, 
         *              (Parent will order this in a case of choice)
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_synchronizeResponse_deserialize(
            adb_synchronizeResponse_t* _synchronizeResponse,
            const axutil_env_t *env,
            axiom_node_t** dp_parent,
            axis2_bool_t *dp_is_early_node_valid,
            axis2_bool_t dont_care_minoccurs);

        /**
         * Deserialize an XML to adb objects
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @param dp_parent double pointer to the parent node to deserialize
         * @param dp_is_early_node_valid double pointer to a flag (is_early_node_valid?)
         * @param dont_care_minoccurs Dont set errors on validating minoccurs,
         *              (Parent will order this in a case of choice)
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axis2_status_t AXIS2_CALL
        adb_synchronizeResponse_deserialize_obj(
            adb_synchronizeResponse_t* _synchronizeResponse,
            const axutil_env_t *env,
            axiom_node_t** dp_parent,
            axis2_bool_t *dp_is_early_node_valid,
            axis2_bool_t dont_care_minoccurs);
                            
            

       /**
         * Declare namespace in the most parent node 
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @param parent_element parent element
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index pointer to an int which contain the next namespace index
         */
       void AXIS2_CALL
       adb_synchronizeResponse_declare_parent_namespaces(
                    adb_synchronizeResponse_t* _synchronizeResponse,
                    const axutil_env_t *env, axiom_element_t *parent_element,
                    axutil_hash_t *namespaces, int *next_ns_index);

        

        /**
         * Wrapper for the serialization function, will invoke the extension mapper instead
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @param synchronizeResponse_om_node node to serialize from
         * @param synchronizeResponse_om_element parent element to serialize from
         * @param tag_closed whether the parent tag is closed or not
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index an int which contain the next namespace index
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axiom_node_t* AXIS2_CALL
        adb_synchronizeResponse_serialize(
            adb_synchronizeResponse_t* _synchronizeResponse,
            const axutil_env_t *env,
            axiom_node_t* synchronizeResponse_om_node, axiom_element_t *synchronizeResponse_om_element, int tag_closed, axutil_hash_t *namespaces, int *next_ns_index);

        /**
         * Serialize to an XML from the adb objects
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @param synchronizeResponse_om_node node to serialize from
         * @param synchronizeResponse_om_element parent element to serialize from
         * @param tag_closed whether the parent tag is closed or not
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index an int which contain the next namespace index
         * @return AXIS2_SUCCESS on success, else AXIS2_FAILURE
         */
        axiom_node_t* AXIS2_CALL
        adb_synchronizeResponse_serialize_obj(
            adb_synchronizeResponse_t* _synchronizeResponse,
            const axutil_env_t *env,
            axiom_node_t* synchronizeResponse_om_node, axiom_element_t *synchronizeResponse_om_element, int tag_closed, axutil_hash_t *namespaces, int *next_ns_index);

        /**
         * Check whether the adb_synchronizeResponse is a particle class (E.g. group, inner sequence)
         * @return whether this is a particle class.
         */
        axis2_bool_t AXIS2_CALL
        adb_synchronizeResponse_is_particle();

        /******************************* Alternatives for Create and Free functions *********************************/

        

        /**
         * Constructor for creating adb_synchronizeResponse_t
         * @param env pointer to environment struct
         * @param _return axis2_char_t*
         * @return newly created adb_synchronizeResponse_t object
         */
        adb_synchronizeResponse_t* AXIS2_CALL
        adb_synchronizeResponse_create_with_values(
            const axutil_env_t *env,
                axis2_char_t* _return);

        


                /**
                 * Free adb_synchronizeResponse_t object and return the property value.
                 * You can use this to free the adb object as returning the property value. If there are
                 * many properties, it will only return the first property. Other properties will get freed with the adb object.
                 * @param  _synchronizeResponse adb_synchronizeResponse_t object to free
                 * @param env pointer to environment struct
                 * @return the property value holded by the ADB object, if there are many properties only returns the first.
                 */
                axis2_char_t* AXIS2_CALL
                adb_synchronizeResponse_free_popping_value(
                        adb_synchronizeResponse_t* _synchronizeResponse,
                        const axutil_env_t *env);
            

        /******************************* get the value by the property number  *********************************/
        /************NOTE: This method is introduced to resolve a problem in unwrapping mode *******************/

        
        

        /**
         * Getter for return by property number (1)
         * @param  _synchronizeResponse adb_synchronizeResponse_t object
         * @param env pointer to environment struct
         * @return axis2_char_t*
         */
        axis2_char_t* AXIS2_CALL
        adb_synchronizeResponse_get_property1(
            adb_synchronizeResponse_t* _synchronizeResponse,
            const axutil_env_t *env);

    
     #ifdef __cplusplus
     }
     #endif

     #endif /* ADB_SYNCHRONIZERESPONSE_H */
    

