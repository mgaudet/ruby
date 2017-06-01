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

#include "ruby/env/RubyMethod.hpp"
#include "env/ConcreteFE.hpp"
#include "ilgen/IlGeneratorMethodDetails.hpp"
#include "control/CompileMethod.hpp"
#include "control/Options.hpp"
#include "control/Options_inlines.hpp"
#include "control/CompilationQueue.hpp" 
#include "control/CompilationRequest.hpp" 
#include "control/CompilationStateManager.hpp"
#include "env/IO.hpp"
#include "env/VMHeaders.hpp"
#include "ruby/version.h"
#include "ruby/config.h"
#include "ruby/thread.h"
#include "env/CompilerEnv.hpp"
#include "env/RawAllocator.hpp"
#include "ras/DebugCounter.hpp"
#include "compile/Compilation.hpp"
#include "AtomicSupport.hpp"
#include <string>
#include <thread>
#include <chrono> 
#include <memory>

extern void setupCodeCacheParameters(int32_t *, OMR::CodeCacheCodeGenCallbacks *callBacks, int32_t *numHelpers, int32_t *CCPreLoadedCodeSize);
typedef VALUE (*jit_method_t)(rb_thread_t*);

#ifdef TR_HOST_POWER
extern "C" VALUE compiledCodeDispatch(rb_thread_t *th, jit_method_t code, void *pseudoTOC);
#endif

extern TR_RuntimeHelperTable runtimeHelpers;

#if defined(TR_HOST_POWER) && !defined(__LITTLE_ENDIAN__)
//Big-Endian POWER.
//Helper Address is stored in a function descriptor consisting of [address, TOC, envp]
//Load out Helper address from this function descriptor.
#define helperAddress(x) (*(void**)(x))
#else
//Non-POWER or Little-Endian POWER.
//Load out Helper Address direclty, no function descriptor used.
#define helperAddress(x) (x)
#endif

#define initHelper(NAME) \
   runtimeHelpers.setAddress(RubyHelper_##NAME, helperAddress((void*)vm->jit->callbacks.NAME##_f))


/**
 * The ruby compilation state manager manages the state related to a 
 * _ruby_ compilation. Inheritence is not used here to ensure the 
 * initialization order is controlled
 */
struct RubyCompilationStateManager : public TR_Uncopyable
   {
   RubyCompilationStateManager(rb_iseq_t* iseq_p, std::string name_p, TR_Hotness optLevel_p)
      : iseq(iseq_p),
        name(name_p),
        optLevel(optLevel_p),
        mb(iseq, name.c_str()),
        compilee(mb), 
        details(&compilee),
        stateManager(NULL, details, optLevel) 
      {
      }

   std::string to_string()
      {
      return name; 
      }

   uint8_t* getStartPC()
      { 
      TR_ASSERT_FATAL(stateManager.getState() == CompilationStateManager::COMPLETED, "Asking for a PC  before it's available %s", name.c_str());
      return stateManager.getStartPC(); 
      }

   rb_iseq_t* iseq; 
   std::string name;
   TR_Hotness optLevel;
   RubyMethodBlock mb;
   ResolvedRubyMethod compilee;
   TR::IlGeneratorMethodDetails details;
   CompilationStateManager stateManager;
   };


static void
initializeAllHelpers(struct rb_vm_struct *vm, TR_RubyJitConfig *jitConfig)
   {
   initializeJitRuntimeHelperTable(false);

#if defined(TR_HOST_POWER)
   jitConfig->setInterpreterTOC(((size_t*)vm->jit->callbacks.rb_funcallv_f)[1]);
#endif

   initHelper(rb_funcallv);
   initHelper(vm_send);
   initHelper(vm_send_without_block);
   initHelper(vm_setconstant);
   initHelper(vm_getspecial);
   initHelper(lep_svar_set);
   initHelper(vm_getivar);
   initHelper(vm_setivar);
   initHelper(vm_opt_plus);
   initHelper(vm_opt_minus);
   initHelper(vm_opt_mult);
   initHelper(vm_opt_div);
   initHelper(vm_opt_mod);
   initHelper(vm_opt_eq);
   initHelper(vm_opt_neq);
   initHelper(vm_opt_lt);
   initHelper(vm_opt_le);
   initHelper(vm_opt_gt);
   initHelper(vm_opt_ge);
   initHelper(vm_opt_ltlt);
   initHelper(vm_opt_not);
   initHelper(vm_opt_aref);
   initHelper(vm_opt_aset);
   initHelper(vm_opt_length);
   initHelper(vm_opt_size);
   initHelper(vm_opt_empty_p);
   initHelper(vm_opt_succ);
   initHelper(rb_ary_new_capa);
   initHelper(rb_ary_new_from_values);
   initHelper(vm_expandarray);
   initHelper(rb_ary_resurrect);
   initHelper(vm_concatarray);
   initHelper(vm_splatarray);
   initHelper(rb_range_new);
   initHelper(rb_hash_new);
   initHelper(rb_hash_aset);
   initHelper(vm_trace);
   initHelper(rb_str_new);
   initHelper(rb_str_new_cstr);
   initHelper(rb_str_resurrect);
   initHelper(vm_get_ev_const);
   initHelper(vm_check_if_namespace);
   initHelper(rb_const_set);
   initHelper(rb_gvar_get);
   initHelper(rb_gvar_set);
   initHelper(rb_iseq_add_mark_object);
   initHelper(vm_setinlinecache);
   initHelper(vm_throw);
   initHelper(rb_threadptr_execute_interrupts);
   initHelper(rb_obj_as_string);
   initHelper(rb_str_append);
   initHelper(rb_vm_ep_local_ep);
   initHelper(vm_get_cbase);
   initHelper(vm_get_const_base);
   initHelper(rb_vm_get_cref);
   initHelper(vm_get_cvar_base);
   initHelper(rb_cvar_get);
   initHelper(rb_cvar_set);
   initHelper(vm_checkmatch);
   initHelper(rb_ary_tmp_new);
   initHelper(rb_ary_store);
   initHelper(rb_reg_new_ary);
   initHelper(vm_opt_regexpmatch1);
   initHelper(vm_opt_regexpmatch2);
   initHelper(vm_defined);
   initHelper(vm_invokesuper);
   initHelper(vm_invokeblock);
   initHelper(rb_method_entry);
   initHelper(rb_class_of);
   initHelper(vm_send_woblock_jit_inline_frame);
   initHelper(vm_send_woblock_inlineable_guard);
   initHelper(rb_bug);
   initHelper(vm_exec_core);
#ifdef OMR_JIT_PROFILING
   initHelper(ruby_omr_is_valid_object);
#endif
   initHelper(rb_class2name);
   initHelper(vm_opt_aref_with);
   initHelper(vm_opt_aset_with);
   initHelper(rb_vm_env_write); 
   initHelper(vm_jit_stack_check); 
   initHelper(rb_str_freeze);
   initHelper(rb_ivar_set);
   initHelper(vm_compute_case_dest); 
   initHelper(vm_getinstancevariable);
   initHelper(vm_setinstancevariable);
   }

static void
initializeCodeCache(TR::CodeCacheManager &codeCacheManager)
   {
   TR::CodeCacheConfig &codeCacheConfig = codeCacheManager.codeCacheConfig();
   // setupCodeCacheParameters must stay before
   // TR_RubyCodeCacheManager::initialize() because it needs trampolineCodeSize
   setupCodeCacheParameters(&codeCacheConfig._trampolineCodeSize,
                            &codeCacheConfig._mccCallbacks,
                            &codeCacheConfig._numOfRuntimeHelpers,
                            &codeCacheConfig._CCPreLoadedCodeSize);
   codeCacheConfig._needsMethodTrampolines = true;
   codeCacheConfig._trampolineSpacePercentage = 5;
   codeCacheConfig._allowedToGrowCache = true;
   codeCacheConfig._lowCodeCacheThreshold = 0;
   codeCacheConfig._verboseCodeCache = false;
   codeCacheConfig._verbosePerformance = false;
   codeCacheConfig._verboseReclamation = false;
   codeCacheConfig._doSanityChecks = false;
   codeCacheConfig._codeCacheTotalKB = 128*1024;
   codeCacheConfig._codeCacheKB = 512;
   codeCacheConfig._codeCachePadKB = 0;
   codeCacheConfig._codeCacheAlignment = 32;
   codeCacheConfig._codeCacheFreeBlockRecylingEnabled = true;
   codeCacheConfig._largeCodePageSize = 0;
   codeCacheConfig._largeCodePageFlags = 0;
   codeCacheConfig._maxNumberOfCodeCaches = 256;
   codeCacheConfig._canChangeNumCodeCaches = true;
   codeCacheConfig._emitElfObject = TR::Options::getCmdLineOptions()->getOption(TR_PerfTool);

   codeCacheManager.initialize(true, 1);
   }

int jitInit(struct rb_vm_struct *vm, char *options)
   {

   // Create a bootstrap raw allocator.
   //
   TR::RawAllocator rawAllocator;

   try
      {
      // Allocate the host environment structure
      //
      TR::Compiler = new (rawAllocator) TR::CompilerEnv(rawAllocator, TR::PersistentAllocatorKit(rawAllocator));
      }
   catch (const std::bad_alloc& ba)
      {
      return -1;
      }

   TR::Compiler->initialize();

   // --------------------------------------------------------------------------

   static TR_RubyFE fe(vm); // singleton object

   initializeAllHelpers(vm, fe.jitConfig());

   if (commonJitInit(fe, options) < 0)
      return -1;

   initializeCodeCache(fe.codeCacheManager());

   vm->jit->default_count = TR::Options::getCmdLineOptions()->getInitialCount();

   if (TR::Options::getCmdLineOptions()->getOption(TR_EnableRubyTieredCompilation))
      {
      vm->jit->options |= TIERED_COMPILATION;
      }

   if (TR::Options::getCmdLineOptions()->getOption(TR_EnableRubyCodeCacheReclamation))
      {
      vm->jit->options |= CODE_CACHE_RECLAMATION;
      }

   // Disable async compilation by default until all the bugs have been 
   // ironed out, but allow it to be turned on and delivered for now. 
   if (getenv("ENABLE_ASYNC_COMPILATION") == NULL) 
      {
      TR::Options::getCmdLineOptions()->setOption(TR_DisableAsyncCompilation);
      }

   return 0;
   }


static void accumulateAndPrintDebugCounters(TR_RubyFE& fe)
   {
   TR_Debug *debug = TR::Options::getDebug();
   if (debug)
      {
      TR::DebugCounterGroup *counters;
      counters = fe.getPersistentInfo()->getStaticCounters();
      if (counters)
         {
         counters->accumulate();
         debug->printDebugCounters(counters, "Static debug counters");
         }
      counters = fe.getPersistentInfo()->getDynamicCounters();
      if (counters)
         {
         counters->accumulate();
         debug->printDebugCounters(counters, "Dynamic debug counters");
         }
      }
   }

/**
 * Dumps information relevant to a crash.
 */
void jitCrashReport() 
   {
   auto &fe = TR_RubyFE::singleton();

   // Dump the debug counters: Can be very useful to 
   // investigate failures that are suspected to be 
   // related to the entry switch! 
   accumulateAndPrintDebugCounters(fe);
   }

int jitTerminate(void *)
   {
   auto &fe = TR_RubyFE::singleton();
   accumulateAndPrintDebugCounters(fe);


   TR::CodeCacheManager &codeCacheManager = fe.codeCacheManager();
   codeCacheManager.destroy();

   return 0;
   }


VALUE updateInfo(uint8_t* startPC, rb_iseq_t* iseq, TR_Hotness optLevel)
   {
   if (startPC)
      {
      if (getenv("NO_PUBLISH")) 
         {
         static bool warning = false; 
         if (!warning)
            { 
            fprintf(stderr, "blacklisting iseq for investigation purposes\n");
            warning = true; 
            }
         iseq->jit.state = ISEQ_JIT_STATE_BLACKLISTED;
         return Qfalse; 
         }

      iseq_jit_body_info *body_info =  ALLOC(iseq_jit_body_info);
      assert(body_info && "Failed to allocate body_info");

      body_info->opt_level = optLevel;
      body_info->startPC = startPC;

      /* Set up body to prepare for recompilation. 
       */
      body_info->recomp_count = TR::Options::getCmdLineOptions()->getInitialCount();
      body_info->invoke_count = 0;
      body_info->prev = NULL;
      body_info->next = iseq->jit.body_info;

      if (iseq->jit.body_info) {
         iseq->jit.body_info->prev = body_info;
      }

      iseq->jit.body_info = body_info;
      iseq->jit.u.code  = body_info->startPC;

      // It will be important the state transition happens last 
      // when doing asynchronous compilation. Flush previous 
      // writes here.
      VM_AtomicSupport::writeBarrier(); 

     
      iseq->jit.state = ISEQ_JIT_STATE_JITTED;
      return Qtrue; 
      }
   else 
      {
      if (iseq->jit.state == ISEQ_JIT_STATE_JITTED) {
            /* unable to recompile - never attempt again */
            iseq->jit.state = ISEQ_JIT_STATE_RECOMP_BLACKLISTED;
            return Qtrue;
        }
        else {
            /* unable to compile - never attempt again */
            iseq->jit.state = ISEQ_JIT_STATE_BLACKLISTED;
            return Qfalse;
        }
      }
   }


#define async_trace(...) if (getenv("ASYNC_COMPILATION_TRACE")) { TR_VerboseLog::writeLineLocked(TR_Vlog_INFO, __VA_ARGS__); } 
VALUE compileRubyISeq(rb_iseq_t *iseq, std::string name, TR_Hotness optLevel)
   {
   int32_t rc = 0;
   RubyMethodBlock mb(iseq, name.c_str()); //OK, as lifetime is shorter than param. 
   ResolvedRubyMethod compilee(mb);
   uint8_t* startPC = compileMethod(NULL, compilee, optLevel, rc);
   return updateInfo(startPC, iseq, optLevel); 
   }

static int compilation_thread_started = 0; 
void unblock_compilation_thread(void* arg) { 
   async_trace("Unblock called!, arg address is %p", arg);
   *(int*)arg  = 0; // interrupt compilation thread. 
}

VALUE compileStateManager(std::unique_ptr<RubyCompilationStateManager> manager) 
   {
   // I'm doing this to avoid casting an std::unique_ptr to 
   // void*, which I just know will end in tears. 
   struct LambdaArg {
      LambdaArg(std::unique_ptr<RubyCompilationStateManager> m) : manager(std::move(m)) {} 
      std::unique_ptr<RubyCompilationStateManager> manager; 
   };
   LambdaArg la(std::move(manager));  
   
   auto compileWithoutGVL = [](void* la) -> void* { 
      LambdaArg* lambda_arg = (LambdaArg*)la; 
      async_trace("Compiling State Manager %s", lambda_arg->manager->to_string().c_str());

      lambda_arg->manager->stateManager.compile();
      async_trace("Compiled State Manager %s, PC is %p",
                  lambda_arg->manager->to_string().c_str(),
                  lambda_arg->manager->getStartPC()
                  );
      return NULL;
   };

   if (!TR::Options::getCmdLineOptions()->getOption(TR_DisableAsyncCompilation))
      { 
      async_trace("releasing GVL to compile");
      rb_thread_call_without_gvl(compileWithoutGVL,             /* func */ 
                                 (void*)&la,                    /* func arg */   
                                 unblock_compilation_thread,    /* unblock func */
                                 &compilation_thread_started);  /* unblock arg */

      async_trace("Re-Acquired GVL after compilation");
      }
   else
      { 
      compileWithoutGVL((void*)&la);
      }
   return updateInfo(la.manager->getStartPC(),
                     la.manager->iseq,
                     la.manager->optLevel);
   }



void* vm_compile_thread(void *vm) { 
   async_trace("invoked compile thread loop"); 
   compilation_thread_started = 1;
   while (compilation_thread_started) {
      TR_RubyFE &fe = TR_RubyFE::singleton();
      
      try {
         /* 
          * Why use a tranformer here? 
          *
          * Well. We want the compilation state manager, and all the objects contained therein 
          * to be constructed _while holding the queue lock_. 
          * 
          * This prevents a data race where an unconstructed compilation could miss a shutdown
          * notification,which would cause a crash.
          *
          * However, it's important that this transformer not be called while holding the GVL, 
          * or else there is a possibility of deadlock. 
          *
          */
         auto transformer = [](TR::CompilationRequest req) -> std::unique_ptr<RubyCompilationStateManager>
            {
            return std::unique_ptr<RubyCompilationStateManager>(new RubyCompilationStateManager(req.iseq, req.name, req.optLevel)); 
            };

         std::unique_ptr<RubyCompilationStateManager> manager = fe.getCompilationQueue().popAndTransform(transformer); 
         if (TR::Options::getCmdLineOptions()->getVerboseOption(TR_VerboseOptions))
            {
            TR_VerboseLog::writeLineLocked(TR_Vlog_DISPATCH, "Popped %s for compilation", manager->to_string().c_str()); 
            }

         compileStateManager(std::move(manager));
      } catch (TR::CompilationQueue<TR::CompilationRequest>::EmptyQueue e)  { // Queue is empty. Sleep. 
         rb_thread_wait_for(rb_time_interval(DBL2NUM(0.001)));
         // auto sleepWithoutGVL = [](void*) -> void* { 
         //    std::this_thread::sleep_for(std::chrono::milliseconds(10));
         // };
         // rb_thread_call_without_gvl2(sleepWithoutGVL,               /* func */ 
         //                             NULL,                          /* func arg */   
         //                             unblock_compilation_thread,    /* unblock func */
         //                             &compilation_thread_started);  /* unblock arg */
      }
   }
   async_trace("compilation thread stopped. Returning NULL"); 
   return NULL; 
}

/**
 *  deque compilations
 */
void dequeCompilations(TR_RubyFE& fe, const rb_iseq_t* iseq)
   {
   auto predicate = [iseq](TR::CompilationRequest t) -> bool {
      if (t.iseq == iseq) {
         async_trace("queue filter found %p iseq", iseq); 
         return true;
      }
      return false;
   };
   fe.getCompilationQueue().predicateFilter(predicate); 
   }

/**
 * interrupt compilations
 */
void interruptCompilations(TR_RubyFE& fe, const rb_iseq_t* iseq)
   {
   auto selection_predicate = [iseq](const TR::Compilation* c) -> bool
      { 
      async_trace("is comp: %s", c->isCompilationFor(iseq) ? "true" : "false"); 
      return c->isCompilationFor(iseq); 
      }; 

   auto mutator             = [iseq](TR::Compilation* c)
      {
      async_trace("Interrupting compilation"); 
      c->interruptCompilation();
      }; 

   async_trace("Starting compilation interrupt");
   fe.getCompilationRegistry().mutateSelectedCompilations(selection_predicate, mutator); 
   }

/**
 * Need to unqueue compilations, as well as interrupt any in progress.
 */
void dequeCompilationsAndInterruptExecutingCompilations(const rb_iseq_t* iseq) 
   {
   TR_RubyFE &fe = TR_RubyFE::singleton();
   async_trace("Filtering out %p from queues, and killing in progress compilations", iseq); 

   // Filter the queue first, to avoid starting a new (doomed) compilation
   dequeCompilations(fe,     iseq); 

   async_trace("Dequed compilation, starting interrupt");
   // Then interrupt compilations. 
   interruptCompilations(fe, iseq); 
   }

extern "C"
{
/*
   _____      _                        _
  | ____|_  _| |_ ___ _ __ _ __   __ _| |
  |  _| \ \/ / __/ _ \ '__| '_ \ / _` | |
  | |___ >  <| ||  __/ |  | | | | (_| | |
  |_____/_/\_\\__\___|_|  |_| |_|\__,_|_|

   ___       _             __
  |_ _|_ __ | |_ ___ _ __ / _| __ _  ___ ___
   | || '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
   | || | | | ||  __/ |  |  _| (_| | (_|  __/
  |___|_| |_|\__\___|_|  |_|  \__,_|\___\___|

*/

#include "ruby.h"
#include "method.h"       // for rb_method_*
#include "vm_core.h"      // for rb_iseq_
#include "jit.h"

void jit_init(rb_vm_t *vm, char * options)
   {
   jitInit(vm, options);
   }

void jit_terminate(rb_vm_t *vm)
   {
   jitTerminate(vm);
   }

/**
 * Return Qtrue if execution can proceed when this function returns. Otherwise, 
 * return false, and update the iseq jit states. 
 */
VALUE jit_compile(rb_iseq_t *iseq)
   {
   TR_Hotness optLevel = cold;

   int32_t len =
      RSTRING_LEN(iseq->body->location.path) +
      (sizeof(size_t) * 3) +                // first_lineno: estimate three decimal digits per byte
      RSTRING_LEN(iseq->body->location.label) +
      3;                                    // two colons and a null terminator

   std::string name =   
           std::string((char*) RSTRING_PTR(iseq->body->location.path)) + 
           ":" + 
           std::to_string(FIX2LONG(iseq->body->location.first_lineno)) + 
           ":" 
           + (char* )RSTRING_PTR(iseq->body->location.label);

   // Prevent recompiling more than once:
   // If we have a chain of two body_info structs
   // for a given iseq, then no need to recompile again.
   if (TR::Options::getCmdLineOptions()->getOption(TR_EnableRubyTieredCompilation)
       && iseq->jit.body_info
       && iseq->jit.body_info->next)
      {
      if (TR::Options::getCmdLineOptions()->getVerboseOption(TR_VerboseOptions))
         TR_VerboseLog::writeLineLocked(TR_Vlog_INFO,"%s @ %p already compiled twice, not compiling again",
                                        name.c_str(),
                                        iseq->jit.body_info->startPC);

      return Qtrue;
      }

   if ((iseq->body->param.flags.has_opt
         && feGetEnv("TR_DISABLE_OPTIONAL_ARGUMENTS"))   ||
       iseq->body->param.flags.has_rest   ||
       iseq->body->param.flags.has_post   ||
       iseq->body->param.flags.has_block  ||
       iseq->body->param.flags.has_kw     ||
       iseq->body->param.flags.has_kwrest
       )
      {
      if (TR::Options::getVerboseOption(TR_VerboseOptions))
         {
         TR_VerboseLog::writeLineLocked(TR_Vlog_COMPFAIL,
            "<JIT: %s cannot be translated: complex arguments:"
            " opts %d rest %d post %d block %d keywords %d kwrest %d>\n",
            name.c_str(),
            iseq->body->param.flags.has_opt,
            iseq->body->param.flags.has_rest,
            iseq->body->param.flags.has_post,
            iseq->body->param.flags.has_block,
            iseq->body->param.flags.has_kw,
            iseq->body->param.flags.has_kwrest);
         }
      /* unable to compile - never attempt again */
      iseq->jit.state = ISEQ_JIT_STATE_BLACKLISTED;
      return Qfalse; 
      }

   // If we're not compiling async, block and compile immediately here.
   if (TR::Options::getCmdLineOptions()->getOption(TR_DisableAsyncCompilation))
      {
      auto manager = std::unique_ptr<RubyCompilationStateManager>(new RubyCompilationStateManager(iseq, name, optLevel)); 
      return compileStateManager(std::move(manager));
      }
   else  // Otherwise, queue the compilation. 
      {
      TR_RubyFE &fe = TR_RubyFE::singleton();
      // This state transition must happen before queing, or else 
      // we could trample the ISEQ_JIT_STATE_JITTED setting when
      // the compilation completes under adversarial scheduling of
      // threads.  
      iseq->jit.state = ISEQ_JIT_STATE_QUEUED;
      fe.getCompilationQueue().enqueue(TR::CompilationRequest(iseq,name,optLevel));
      return Qfalse; //Assume we haven't compiled the method between queueing and returning.
      }
   }


VALUE jit_dispatch(rb_thread_t *th, jit_method_t code)
   {
   TR_ASSERT_FATAL(!getenv("NO_PUBLISH"), "Executing unpublished code"); 
#if !defined(TR_HOST_POWER)
   return (*code)(th);
#else
   /* On Power, we need to setup the TOC and the pseudoTOC for the
      jitted code -- compiledCodeDispatch does this for us */
   TR_RubyFE &fe = TR_RubyFE::singleton();
   return compiledCodeDispatch(th, code,
                               fe.jitConfig()->getPseudoTOC());
#endif
   }

/**
 * Invoked when Ruby crashes from the Ruby crash reporter. 
 *
 * Dump information of relevance to a crash!
 */
void jit_crash(void*) 
   {
   jitCrashReport();
   }

/**
 * Return Qtrue if there is a body to execute.
 *
 * Otherwise, update the states for this method. 
 */
VALUE jit_update_state(rb_thread_t* th, const rb_iseq_t* iseq_const)
   { 
   if (iseq_const->jit.state == ISEQ_JIT_STATE_JITTED) 
      return Qtrue; 
   
   if (iseq_const->jit.state == ISEQ_JIT_STATE_QUEUED)
      return Qfalse; 

   if (iseq_const->jit.state == ISEQ_JIT_STATE_RECOMP_BLACKLISTED)
      return Qtrue;

   if (iseq_const->jit.state == ISEQ_JIT_STATE_BLACKLISTED)
      return Qfalse;

   rb_iseq_t* iseq = (rb_iseq_t*)iseq_const; 
   if (iseq_const->jit.state == ISEQ_JIT_STATE_JITTED)
      {
      if (th->vm->jit->options & TIERED_COMPILATION)
         {
         --iseq_const->jit.body_info->recomp_count;
         if (iseq_const->jit.body_info->recomp_count < 0)
            {
            return jit_compile(iseq);
            }
         }
      return Qtrue;
      }

   if (iseq->jit.state == ISEQ_JIT_STATE_ZERO)
      {
      iseq->jit.u.count = th->vm->jit->default_count;
      iseq->jit.state = ISEQ_JIT_STATE_INTERPRETED;
      iseq->jit.body_info = NULL;
      }

   --iseq->jit.u.count;

   if (iseq_const->jit.u.count < 0)
      {
      return jit_compile(iseq);
      }

    return Qfalse; /* not jitted yet */
   }

void jit_create_compilation_thread(rb_vm_t* vm) 
   {
   if (!TR::Options::getCmdLineOptions()->getOption(TR_DisableAsyncCompilation)) 
      {
      const int thread_count = getenv("COMPILATION_THREAD_COUNT")  ? atoi(getenv("COMPILATION_THREAD_COUNT")) : 1; 
      async_trace("Creating compilation %d threads", thread_count); 
      for (int i = 0; i < thread_count; i++) 
         {
         async_trace("Creating compilation thread %d", i); 
         typedef VALUE (*thread_function)(ANYARGS);
         rb_thread_create(reinterpret_cast<thread_function>(vm_compile_thread),vm);
         }
      } 
   else
      {
      async_trace("Declined to create compilation threads -- Option says disabled.");
      }
   }

/**
 * Handle invoked by the garbage collector when an iseq is being freed. We need
 * to ensure that we remove this iseq from the compilation queue, and terminate
 * any compilations that may be in progres.
 */
void jit_iseq_free(const rb_iseq_t* iseq)
   {
   if (TR::Options::getCmdLineOptions()->getVerboseOption(TR_VerboseOptions))
      TR_VerboseLog::writeLineLocked(TR_Vlog_RECLAMATION, "iseq %p reclaimed",iseq);  
   dequeCompilationsAndInterruptExecutingCompilations(iseq); 
   }

} /* extern C */