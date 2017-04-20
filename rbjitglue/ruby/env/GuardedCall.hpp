/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2017, 2017
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


#ifndef GUARDED_CALL_HPP
#define GUARDED_CALL_HPP
#include "ruby/thread.h"
#include <functional>

/**
 * This is a function that takes a void pointer, and 
 * invokes it. This is interesting, cause I can now 
 * create a capturing lambda, and use this to invoke it
 * under rb_thread_call_with_gvl
 */
template <typename functionType> 
void* invoker(void* f) 
   {
   auto fn = static_cast<functionType>(f);
   auto res = (*fn)(); 
   return const_cast<void*>(static_cast<const void*>(res)); 
   }

/**
 * A call to function, guarded by rb_thread_call_with_gvl.
 *
 * Single argument variant. One can imagine doing many fancy 
 * things with variadic templates, but for now, all we need is 
 * one.
 */
template <typename FunctionType, typename ArgumentType>
auto GVLGuardedCall(FunctionType function, ArgumentType argument) -> decltype(function(argument))
   {
   /**
    * Capture everything relevant in a lambda. 
    */
   auto fn = [function, argument]() -> decltype(function(argument)) { 
      return function(argument); 
   };

   void * returnval = rb_thread_call_with_gvl(invoker<decltype(&fn)>,
                                              static_cast<void*>(&fn));
   auto typed_returnval = static_cast<decltype(function(argument))>(returnval); 
   return typed_returnval;
   }

#endif
