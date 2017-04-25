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
   // Already holding GVL -- likely async compilation scenario. 
   if (ruby_thread_has_gvl_p())
      {
      return static_cast<decltype(function(argument))>( function(argument) );
      }
   else
      {
      typedef VALUE (*functype)(ANYARGS);
      auto returnval = rb_thread_value(rb_thread_create(reinterpret_cast<functype>(function), (void*)argument));
      auto typed_returnval = reinterpret_cast<decltype(function(argument))> ( returnval ); 
      return typed_returnval;
      }
   }

#endif
