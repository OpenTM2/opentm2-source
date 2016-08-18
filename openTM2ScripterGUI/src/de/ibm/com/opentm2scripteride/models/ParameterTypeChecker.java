/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

package de.ibm.com.opentm2scripteride.models;

//TODO: check methods, maybe adjust them. Especially make sure you can comprehend CheckConditionIf

public class ParameterTypeChecker {

	/*
	 * Function check the syntax of a condition for an if or elseif command
	 * 
	 * @param String input
	 * 
	 * @return boolean true - syntax is ok
	 * 
	 * @return boolean false - syntax is false
	 */
	//@TODO: check, implement !
	//return (checkConditionIf(input) || ((input.substring(0, 0) == "!") && checkConditionIf(input
	//		.substring(1))));
	public static boolean checkConditionIf(String input) {
		return (countChar(input, '(') == countChar(input, ')') 
				&&
				 input.trim().matches("^((([\\(]*((%[^%]+%)|(\\$[^\\$]+\\$)|\\d+)+[\\)]*|[\\(]*\\d+[\\)]*)(\\s)*((==)|(!=)|<|>)(\\s)*)(([\\(]*((%[^%]+%)|(\\$[^\\$]+\\$)|\\d+)+[\\)]*)|[\\(]*\\d+[\\)]*)(\\||&))*" +
				 		"(([\\(]*((%[^%]+%)|(\\$[^\\$]+\\$)|\\d+)+[\\)]*)|[\\(]*\\d+[\\)]*)" +
				 		"(\\s)*((==)|(!=)|<|>)(\\s)*" +
				 		"(([\\(]*((%[^%]+%)|(\\$[^\\$]+\\$)|\\d+)+[\\)]*)|[\\(]*\\d+[\\)]*)$"));
	}

	/**
	 * Function check the syntax of parameter of type default. (String with variables and comments)
	 * 
	 * @param input The string to be checked
	 * @return True if syntax is okay, false otherwise
	 */
	public static boolean checkSimpleAttr(String input) {
		return (countChar(input, '(') == countChar(input, ')') && input
				.matches("^((\\$[^\\$^%]+\\$)|(%[^\\$^%]+%)|[^%^$])+$"));
	}

	/*
	 * Function check the syntax of an comma separated list of simple attributes
	 * 
	 * @param String input
	 * 
	 * @return boolean true - syntax is ok
	 * 
	 * @return boolean false - syntax is false
	 */
	public static boolean checkListComma(String input) {
		boolean test = true;
		String[] tokens = input.split(",");
		if (tokens.length - 1 == countChar(input, ',')) {
			for (int i = 0; i < tokens.length; i++) {
				if (tokens[i].length() == 0 || tokens[i].equals("")
						|| !checkSimpleAttr(tokens[i])) {
					test = false;
				}
			}
		} else
			test = false;
		return test;
	}

	/*
	 * Function check the syntax of a variable name
	 * 
	 * @param String input
	 * 
	 * @return boolean true - syntax is ok
	 * 
	 * @return boolean false - syntax is false
	 */
	public static boolean checkVariableName(String input) {
		return input.matches("^[^$%]+$");
	}

	/*
	 * Function check the syntax of a constant name
	 * 
	 * @param String input
	 * 
	 * @return boolean true - syntax is ok
	 * 
	 * @return boolean false - syntax is false
	 */
	public static boolean checkConstantName(String input) {
		return checkVariableName(input);
	}

	/*
	 * Function check the syntax of a positive integer string (without '+')
	 * 
	 * @param String input
	 * 
	 * @return boolean true - syntax is ok
	 * 
	 * @return boolean false - syntax is false
	 */
	public static boolean checkPositiveIntegerAttr(String input) {
		return input.matches("^(\\([0-9]+\\))|[0-9]+$");
	}

	/*
	 * Function check the syntax of action attribute of a for loop
	 * 
	 * @param String input
	 * 
	 * @return boolean true - syntax is ok
	 * 
	 * @return boolean false - syntax is false
	 */
	public static boolean checkForAction(String input) {
		if (input.matches("^(INC)|(DEC)|((ADD )|(SUB) )[0-9]+$"))
			return true;
		else
			return false;
	}

	/*
	 * This function counts the number of any specific char in a String
	 * 
	 * @param String search String
	 * 
	 * @param char the char you want count
	 * 
	 * @return number of found given chars c in string s as int
	 */
	private static int countChar(String s, char c) {
		return s.replaceAll("[^" + c + "]", "").length();
	}

}
