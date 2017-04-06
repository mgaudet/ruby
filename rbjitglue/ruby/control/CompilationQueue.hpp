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


#ifndef RUBY_COMPILATION_QUEUE
#define RUBY_COMPILATION_QUEUE

#include <deque> 
#include "env/VerboseLog.hpp"
#include "infra/Monitor.hpp"
#include "infra/CriticalSection.hpp"

namespace TR { 

/**
 * A queue of objects to be compiled. 
 *
 * Thread safe insertion and removal required.
 *
 * For RAS purposes, this assumes that T provides a 'to_string()' method, 
 * which returns something that acts like std::string.  
 */
template <typename T> 
class CompilationQueue { 
   public:
   CompilationQueue(bool verbose) :
      _queue(),
      _verbose(verbose),
      _queueMonitor(TR::Monitor::create("CompilationQueueMonitor"))
   {} 
   
   void enqueue(T t) {
      OMR::CriticalSection lock(_queueMonitor); 
      if (_verbose) 
         {
         TR_VerboseLog::writeLineLocked(TR_Vlog_INFO, "QUEUE: Added  %s for compilation (queue size after %d)", t.to_string().c_str(), _queue.size() + 1); 
         }
      _queue.emplace_back(t);
   }

   /**
    * Returns true if an element was popped off the 
    * queue. 
    *
    * If this returns false, it is undefined behaviour
    * to derference the parameter
    */
   bool pop(T& ret) { 
      OMR::CriticalSection lock(_queueMonitor); 
      if (_queue.size() > 0) { 
         ret = _queue.front();
         _queue.pop_front();
         if (_verbose) 
            {
            TR_VerboseLog::writeLineLocked(TR_Vlog_INFO, "QUEUE: popped %s off compilation queue (queue size after %d)", ret.to_string().c_str(), _queue.size()); 
            }
         return true;
      } else { 
         return false;
      }
   }

   /**
    * Remove every element from the queue that matches the provided predicate. 
    *
    * The predicate function returns true if the element is to be removed. The 
    * predicate needs to be compatible with UnaryPredicate from <algorithms>
    *
    * Used to filter out compilations no longer required. 
    */
   template <typename CompilationPredicate> 
   void predicateFilter(CompilationPredicate p) {

      OMR::CriticalSection lock(_queueMonitor); 
      auto size_before = _queue.size(); 
      _queue.erase(std::remove_if(_queue.begin(), _queue.end(), p), _queue.end());
      auto size_after  = _queue.size(); 

      if (_verbose)
         {
         TR_VerboseLog::writeLineLocked(TR_Vlog_INFO, "QUEUE: Filtered %d elements from queue",
                                     size_before - size_after
                                     ); 
         }
   }

   /**
    * Return the size of the queue. 
    */
   size_t size()
      {
      OMR::CriticalSection lock(_queueMonitor); 
      return _queue.size();
      }


   private: 
   std::deque<T> _queue; 
   bool          _verbose; 
   TR::Monitor*   _queueMonitor;
};

}

#endif
