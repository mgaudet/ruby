/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2000, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 *******************************************************************************/


#ifndef RUBY_COMPILATION_REQUEST
#define RUBY_COMPILATION_REQUEST

#include <string>

namespace TR { 
/**
 * A request to compile a method. 
 */
struct CompilationRequest
   { 
   CompilationRequest() : 
      iseq(NULL), name(), optLevel(cold) {}

   CompilationRequest(rb_iseq_t* i, std::string n, TR_Hotness o) :
      iseq(i), name(n), optLevel(o) {} 

   rb_iseq_t *iseq;     ///< The iseq to be compiled. 
   std::string name;    ///< Name of the method  
   TR_Hotness optLevel; ///< optLevel -- I believe this is mostly ignored right now. 

   /**
    * Return an std::string representation. 
    */
   const std::string& to_string() 
      {
      /* For now, just return the name */ 
      return name;
      }
   };
}
#endif
