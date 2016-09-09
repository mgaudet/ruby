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
 * \file omr_jit.c 
 *
 * Functions specific to the OMR jit, as opposed to the generic JIT interface
 * code. 
 */

/**
 * Returns the name of the JIT DLL to be loaded. 
 */
const char *
get_jit_dll_name() {
   return "librbjit.so";
}

/**
 * Return the options string to feed to jit_init. 
 *
 */
char *
get_jit_options() { 
   char *jit_options = getenv("OMR_JIT_OPTIONS");
   if (jit_options) {
      if (strncmp(jit_options, "-Xjit", 5) != 0) {
         fprintf(stderr, "[FATAL] invalid OMR_JIT_OPTIONS <[%s]>\n", jit_options);
         exit(EXIT_FAILURE);
      }
   }
   return jit_options;
}


