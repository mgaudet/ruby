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


#ifndef RUBY_COMPILATION_REGISTRY
#define RUBY_COMPILATION_REGISTRY

#include "env/VerboseLog.hpp"
#include "infra/Monitor.hpp"
#include "infra/CriticalSection.hpp"
#include <vector> 
#include <algorithm>

namespace TR { class Compilation; } 

namespace TR { 

/**
 * Provides lookup capabilities around the existing compilations. It is
 * expected that a compilation will be registered during construction, and
 * unregistered as part of destruction. 
 *
 * To avoid races, manipulation must be done by passing functions into the
 * registry, this ensures that the compilation will not be invalidated during
 * manipulation.
 *
 */
class CompilationRegistry
   {
public:
   CompilationRegistry(bool verbose) : 
      _registryMonitor(TR::Monitor::create("CompilationRegistry monitor")),
      _compilations(),
      _verbose(verbose)
   {} 

   void registerCompilation(TR::Compilation* c)
      {  
      OMR::CriticalSection lock(_registryMonitor); 
      _compilations.push_back(c); 
      if (_verbose)
         TR_VerboseLog::writeLineLocked(TR_Vlog_INFO, "COMPILATION_REGSTRY: Registered %p, Registry size %d\n", c, _compilations.size());
      }

   void unregisterCompilation(TR::Compilation* c)
      {  
      OMR::CriticalSection lock(_registryMonitor); 
      _compilations.erase(std::remove(_compilations.begin(), _compilations.end(), c), _compilations.end()); 
      if (_verbose)
         TR_VerboseLog::writeLineLocked(TR_Vlog_INFO, "COMPILATION_REGSTRY: Unregistered %p, Registry size %d\n", c, _compilations.size());
      }

   /**
    * While holding the registry lock, mutate all compilations that match 
    * the predicate using the passed mutator. 
    */
   template <typename CompilationPredicate, typename CompilationMutator>
   void mutateSelectedCompilations(CompilationPredicate pred, CompilationMutator m)
      { 
      OMR::CriticalSection lock(_registryMonitor); 
      if (_verbose)
         TR_VerboseLog::writeLineLocked(TR_Vlog_INFO, "MUTATOR %d compilations\n", _compilations.size());
      for (auto c : _compilations)
         { 
         if (_verbose)
            TR_VerboseLog::writeLineLocked(TR_Vlog_INFO, "MUTATOR CHECKING %p\n", c);
         if (pred(c)) 
            {
            if (_verbose)
               TR_VerboseLog::writeLineLocked(TR_Vlog_INFO, "MUTATOR MUTATING %p\n", c);
            m(c);
            }
         }
      }

private:
   std::vector<TR::Compilation*> _compilations; 
   TR::Monitor* _registryMonitor; 
   bool _verbose;
   };

}
#endif