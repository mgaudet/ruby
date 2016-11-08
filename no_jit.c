/* -*- mode: c; style: ruby; -*-
*
* Licensed Materials - Property of IBM
* "Restricted Materials of IBM"
*
* (c) Copyright IBM Corp. 2014 All Rights Reserved
*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/**
 * \file no_jit.c 
 *
 * Generic functions for JIT support -- replace
 * these functions with ones customized to your 
 * JIT.
 */

/**
 * Returns the name of the JIT DLL to be loaded. 
 */
const char *
get_jit_dll_name() {
   return NULL;
}

/**
 * Return the options string to feed to jit_init. 
 */
char *
get_jit_options() { 
   return NULL;
}


