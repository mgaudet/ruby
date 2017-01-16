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
 * The JIT options string sourced from the command line.
 *
 * set_jit_options ensures it's prefixed with -Xjit:, as that
 * is what the OMR options processor expects
 */
static char * jit_options_string = NULL;

/**
 * Copies the provided option into a self allocated buffer
 *
 * FIXME: tiny leak: Should probably be freed. 
 */
void 
set_jit_options(const char * options) {
   int optlen = 0; 
   const char * format = "%s"; 

   if (!options) return; 

   optlen = strlen(options) + 6 /* -Xjit: */ + 1 /* NULL */;
   jit_options_string = malloc(optlen); 

   if (strncmp(options, "-Xjit", 5) != 0) {
      format = "-Xjit:%s"; 
   } 

   snprintf(jit_options_string, optlen, format,  options);

   if (getenv("TRACE_JIT_OPTIONS")) 
      fprintf(stderr, "[TRACE_JIT_OPTIONS] set options to  %s\n", jit_options_string); 
   return;
}

/**
 * Return the options string to feed to jit_init. 
 *
 * Preferrs environment variables. 
 *
 */
char *
get_jit_options() {
   char *env_jit_options = getenv("OMR_JIT_OPTIONS");
   if (env_jit_options) {
      if (jit_options_string) {
         fprintf(stderr, "Warning: JIT options already set on command line. Preferring env var (%s) to command line (%s)\n", env_jit_options, jit_options_string);
      }

      set_jit_options(env_jit_options);
   }

   if (getenv("TRACE_JIT_OPTIONS"))
      fprintf(stderr, "[TRACE_JIT_OPTIONS] Returning %s\n", jit_options_string);
   return jit_options_string;
}

