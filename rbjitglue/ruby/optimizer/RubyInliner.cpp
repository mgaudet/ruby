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


#include <algorithm>
#include "env/VMHeaders.hpp" 
#include "env/FEBase.hpp"
#include "il/Block.hpp"
#include "il/ILOpCodes.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"
#include "il/TreeTop.hpp"
#include "il/symbol/ParameterSymbol.hpp"
#include "ilgen/IlGenRequest.hpp"
#include "ilgen/IlGeneratorMethodDetails_inlines.hpp"
#include "optimizer/Inliner.hpp"
#include "optimizer/RubyInliner.hpp"
#include "ruby/env/RubyFE.hpp"
#include "ruby/env/RubyMethod.hpp"
#include "ruby/optimizer/RubyCallInfo.hpp"
#include "ruby/ilgen/IlGeneratorMethodDetails.hpp"
#include "ras/DebugCounter.hpp"
#include "env/GuardedCall.hpp"


#define OPT_DETAILS "O^O DUMB INLINER CALLSITE: "

//Quick helper function.
static TR::TreeTop*
genCall(TR::Compilation* comp, TR_RuntimeHelper helper, TR::ILOpCodes opcode, int32_t num, ...)
   {
   va_list args;
   va_start(args, num);
   TR::Node *callNode = TR::Node::create(opcode, num);
   for (int i = 0; i < num; i++)
      {
      callNode->setAndIncChild(i, va_arg(args, TR::Node *));
      }
   va_end(args);

   TR::SymbolReference * helperSymRef = comp->getSymRefTab()->findOrCreateRuntimeHelper(helper, true, true, false);
   helperSymRef->setEmptyUseDefAliases(comp->getSymRefTab());
   callNode->setSymbolReference(helperSymRef);

   TR::TreeTop* returnTT = TR::TreeTop::create(comp, TR::Node::create(TR::treetop, 1, callNode));
   return returnTT;
   }

bool TR_InlinerBase::tryToGenerateILForMethod (TR::ResolvedMethodSymbol* calleeSymbol, TR::ResolvedMethodSymbol* callerSymbol, TR_CallTarget* calltarget)
   {
   bool ilGenSuccess = false;
   TR::IlGeneratorMethodDetails callee_ilgen_details(calleeSymbol->getResolvedMethod());
   TR::CompileIlGenRequest request(callee_ilgen_details);
   ilGenSuccess  = calleeSymbol->genIL(fe(), comp(), comp()->getSymRefTab(), request);
   return ilGenSuccess;
   }

bool TR_InlinerBase::inlineCallTarget(TR_CallStack *callStack, TR_CallTarget *calltarget, bool inlinefromgraph, TR_PrexArgInfo *argInfo, TR::TreeTop** cursorTreeTop)
   {
   TR_InlinerDelimiter delimiter(tracer(),"TR_InlinerBase::inlineCallTarget");

   if (!comp()->incInlineDepth(calltarget->_calleeSymbol,
                               calltarget->_myCallSite->_callNode->getByteCodeInfo(),
                               calltarget->_myCallSite->_callNode->getSymbolReference()->getCPIndex(),
                               calltarget->_myCallSite->_callNode->getSymbolReference(),
                               !calltarget->_myCallSite->_isIndirectCall,
                               argInfo))
      {
      return false;
      }



   bool successful = inlineCallTarget2(callStack, calltarget, cursorTreeTop, inlinefromgraph, 99);

   // if inlining fails, we need to tell decInlineDepth to remove elements that
   // we added during inlineCallTarget2
   comp()->decInlineDepth(!successful);

   return successful;
   }

void TR_InlinerBase::getBorderFrequencies(int32_t &hotBorderFrequency, int32_t &coldBorderFrequency, TR_ResolvedMethod * calleeResolvedMethod, TR::Node *callNode)
   {
   hotBorderFrequency = 2500;
   coldBorderFrequency = 0;
   return;
   }

int32_t TR_InlinerBase::scaleSizeBasedOnBlockFrequency(int32_t bytecodeSize, int32_t frequency, int32_t borderFrequency, TR_ResolvedMethod * calleeResolvedMethod, TR::Node *callNode, int32_t coldBorderFrequency)
   {
   int32_t maxFrequency = MAX_BLOCK_COUNT + MAX_COLD_BLOCK_COUNT;
   bytecodeSize = (int)((float)bytecodeSize * (float)(maxFrequency-borderFrequency)/(float)maxFrequency);
              if (bytecodeSize < 10) bytecodeSize = 10;

   return bytecodeSize;

   }

/**
 * Ensure that something hasn't changed the children's layout we expect.
 *
 * Expecting 4 children: 
 *
 *     RubyHelper_vm_send_without_block 
 *     -- thread
 *     -- call_info
 *     -- call cache
 *     -- reciever. 
 *    
 * We need this layout because we're going to go poking about inside the values of the nodes
 * and it's best we dereference memory we trust :D 
 */
void
verify_children(TR::Node* node) 
   {
   TR_ASSERT( (node->getNumChildren() == 4) &&
               node->getSecondChild() &&
               node->getSecondChild()->getOpCodeValue() == TR::aconst &&
               node->getThirdChild() &&
               node->getThirdChild()->getOpCodeValue() == TR::aconst,
               "Unexpected children in hierarchy of vm_send_without_block when creating callsite target.");
   }


static int
jit_callable_class_p(const rb_callable_method_entry_t* me)
{
    VALUE klass = me->defined_class;
    if (!klass) return FALSE;
    switch (RB_BUILTIN_TYPE(klass)) {
      case T_ICLASS:
	if (!RB_TYPE_P(RCLASS_SUPER(klass), T_MODULE)) break;
      case T_MODULE:
	return TRUE;
    }
    while (klass) {
	if (klass == rb_cBasicObject) {
	    return TRUE;
	}
	klass = RCLASS_SUPER(klass);
    }
    return FALSE;
}




/**
 * Returns an enum value that indicates whether or not we will be able to inline. 
 */
int
TR_InlinerBase::checkInlineableWithoutInitialCalleeSymbol (TR_CallSite* callSite, TR::Compilation* comp)
   {
   TR::Node* node = callSite->_callNode;

   if(node->getSymbolReference()
      != comp->getSymRefTab()->getSymRef(RubyHelper_vm_send_without_block))
      return Ruby_unsupported_calltype;

   // Ensure call looks like how we need. 
   verify_children(node); 

   CALL_INFO  ci = reinterpret_cast<CALL_INFO> (node->getSecondChild()->getAddress());
   /*
    * splat args require a call to vm_caller_setup_arg_splat. 
    */
   if (ci->flag & VM_CALL_ARGS_SPLAT)
      { 
      TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/splattedArgs"));
      return Ruby_unsupported_calltype;
      }

   /* 
    * Tailcalls require special support. 
    */ 
   if (ci->flag & VM_CALL_TAILCALL)
      { 
      TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/tailcall"));
      return Ruby_unsupported_calltype;
      }

   /*
    * Keyword args require a call to vm_caller_setup_arg_kw 
    */
   if (ci->flag & VM_CALL_KWARG)
      { 
      TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/keyword"));
      return Ruby_unsupported_calltype;
      }


   CALL_CACHE cc = reinterpret_cast<CALL_CACHE>(node->getThirdChild()->getAddress());
   TR_ASSERT_FATAL(TR_RubyFE::instance()->getJitInterface()->globals.initialized == 1, "using uninitialzied globals"); 

   // No point inlining if it's never going to run! 
   if (cc->method_state != *TR_RubyFE::instance()->getJitInterface()->globals.ruby_vm_global_method_state_ptr)
      {
      TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/method_state_changed"));
      return Ruby_unsupported_calltype;
      }

   const rb_callable_method_entry_t *me = cc->me; 
   if(!me)
      {
      TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/missing_method_entry"));
      return Ruby_missing_method_entry;
      }

   // Low bit set. 
   // 
   // Empirically this always indicates that ->def is pointing to garbage. What I don't understand is _why_. 
   // 
   // I've asked on Ruby-core: http://blade.nagaokaut.ac.jp/cgi-bin/scat.rb/ruby/ruby-core/80924
   if (me->flags & 0x1)
      {
      TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/unsupported_method_entry_flag/low_bit_set/%d",me->flags));
      return Ruby_unsupported_method_entry_flag;
      }

   if (!me->def)
      {
      TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/missing_Def"));
      return Ruby_non_iseq_method;
      }

   /*
    * Wed Jun 03 10:35:45 2015  Koichi Sasada  <ko1@atdot.net>
    *
    *     * method.h: split rb_method_definition_t::flag to several flags.
    *
    *       `flag' contains several categories of attributes and it makes us
    *       confusion (at least, I had confused).
    *
    *       * rb_method_visibility_t (flags::visi)
    *         * NOEX_UNDEF     -> METHOD_VISI_UNDEF     = 0
    *         * NOEX_PUBLIC    -> METHOD_VISI_PUBLIC    = 1
    *         * NOEX_PRIVATE   -> METHOD_VISI_PRIVATE   = 2
    *         * NOEX_PROTECTED -> METHOD_VISI_PROTECTED = 3
    *       * NOEX_SAFE(flag)  -> safe (flags::safe, 3 bits)
    *       * NOEX_BASIC       -> basic (flags::basic, 1 bit)
    *       * NOEX_MODFUNC     -> rb_scope_visibility_t in CREF
    *       * NOEX_SUPER       -> MISSING_SUPER (enum missing_reason)
    *       * NOEX_VCALL       -> MISSING_VCALL (enum missing_reason)
    *       * NOEX_RESPONDS    -> BOUND_RESPONDS (macro)
    */

   // This logic is inspired heavily by vm_call_method
   //
   // Essentially, we break if we hit vm_call_method_each_type, because
   // these are the cases where we needn't be concerned with having to
   // issue a warning. 
   //
   // FIXME: Need to investigate others later. 
   switch (METHOD_ENTRY_VISI(cc->me)) 
      {
      case METHOD_VISI_PUBLIC:
         break; // OK for inlining.
      case METHOD_VISI_PRIVATE:
         if (ci->flag & VM_CALL_FCALL)
            TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/private"));
            break; // Ok for inlining
      default: 
            {
            TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/unsupported_method_entry_flag/0x%lx", me->flags));
            return Ruby_unsupported_method_entry_flag;
            }
      }

   /* A couple of different checks probably make sense here. 
    *
    * cc->me->def->type == VM_METHOD_TYPE_ISEQ
    * ci->call == vm_call_iseq_setup 
    *
    * from inside vm_call_iseq_setup: 
    *
    *     const rb_iseq_t *iseq = def_iseq_ptr(cc->me->def);
    *     const int param_size = iseq->body->param.size;
    *     const int local_size = iseq->body->local_table_size;
    *
    */

   /*
    * In an async compilation world, this code is racy (even under GVL) 
    * becuase GC can happen; so disabling this until we can run 
    * dramatically more of the inlining under the GVL than just this. 
    *
    * char methodName[64];
    * char klassName[64];

    * snprintf(methodName, 64, "%s", GVLGuardedCall(TR_RubyFE::instance()->getJitInterface()->vm_functions.rb_id2name_f,ci->mid));
    * snprintf(klassName, 64, "%s", GVLGuardedCall(TR_RubyFE::instance()->getJitInterface()->vm_functions.rb_class2name_f,me->defined_class));
    */ 

   //If this isn't a proper Ruby method, don't inline.
   //
   //FIXME: Should really dig into the remaining missing types to have a 
   //       better view, ie, VM_METHOD_TYPE_BMETHOD or others. 
   //    
   switch(me->def->type)
      {
      case VM_METHOD_TYPE_ISEQ:
         break;
      case VM_METHOD_TYPE_CFUNC:
         // See note above
         //TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/inlining_cfunc/%s/%s", klassName,methodName));
         TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/inlining_cfunc"));
         return Ruby_inlining_cfunc;
      default:
         // See note above
         //TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/inlining_non_iseq_method/%s/%s", klassName,methodName));
         TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/inlining_non_iseq_method"));
         return Ruby_non_iseq_method;
      }

   // FIXME: Async Compilation makes this, and all references via this pointer below racy.
   // Need a runtime assumption guarded accessor that will abort inlining should the 
   // iseq be freed.  
   const rb_iseq_t *iseq_callee = GVLGuardedCall(TR_RubyFE::instance()->getJitInterface()->vm_functions.def_iseq_ptr_f, me->def);

   //Check if the callee has optional arguments.
   //
   // This will be inlinable at some point. 
   bool check_arg_opts     = (iseq_callee->body->param.flags.has_opt    != 0);
   bool check_arg_rest     = (iseq_callee->body->param.flags.has_rest   != 0);
   bool check_arg_post_len = (iseq_callee->body->param.flags.has_post   != 0);
   bool check_arg_block    = (iseq_callee->body->param.flags.has_block  != 0);
   bool check_arg_keywords = (iseq_callee->body->param.flags.has_kw     != 0);
   bool check_arg_kwrest   = (iseq_callee->body->param.flags.has_kwrest != 0);

   if ( check_arg_opts      ||
        check_arg_rest      ||
        check_arg_post_len  ||
        check_arg_block     ||
        check_arg_keywords ||
        check_arg_kwrest
      )
      {
      if (check_arg_kwrest)
         TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/has_opt_args/arg_kwrest"));
      if (check_arg_opts)
         TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/has_opt_args/arg_opts"));
      if (check_arg_rest)
         TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/has_opt_args/arg_rest"));
      if (check_arg_post_len)
         TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/has_opt_args/arg_post_len"));
      if (check_arg_block)
         TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/has_opt_args/arg_block"));
      if (check_arg_keywords)
         TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/has_opt_args/arg_keywords"));

      return Ruby_has_opt_args;
      }

   if(iseq_callee->body->catch_table != 0)
      {
      TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/notInlineable/non_zero_catchtable"));
      return Ruby_non_zero_catchtable;
      }

   //Ok everything looks good.
   TR::DebugCounter::incStaticDebugCounter(comp, TR::DebugCounter::debugCounterName(comp, "ruby.callSites/send_without_block/inlineable"));
   return InlineableTarget;
   }

Ruby::InlinerUtil::InlinerUtil(TR::Compilation *comp)
   : OMR_InlinerUtil(comp)
   {}

/**
 * Generates a call to vm_send_woblock_jit_inline_frame, which is 
 * intended to perform all the frame setup code. 
 *
 * FIXME: Should probably be replaced with a tree sequence at somepoint. 
 */   
void
Ruby::InlinerUtil::calleeTreeTopPreMergeActions(TR::ResolvedMethodSymbol *calleeResolvedMethodSymbol, TR_CallTarget* calltarget)
   {
   TR_CallSite* callSite = calltarget->_myCallSite;
   TR::Node* send_without_block_call_Node = callSite->_callNode;

   // Ensure looks like a call! 
   verify_children(send_without_block_call_Node); 

   CALL_INFO  ci = reinterpret_cast<CALL_INFO>(send_without_block_call_Node->getSecondChild()->getAddress());
   CALL_CACHE cc = reinterpret_cast<CALL_CACHE>(send_without_block_call_Node->getThirdChild()->getAddress());

   const rb_iseq_t* iseq = static_cast<ResolvedRubyMethod*>(calltarget->_calleeMethod)->getRubyMethodBlock().iseq();
   TR_ASSERT(iseq, "Didn't find an iseq");
   traceMsg(comp(), "got callee iseq as %p\n",iseq);


   TR::TreeTop * startOfInlinedCall = calleeResolvedMethodSymbol->getFirstTreeTop()->getNextTreeTop();

   //Need Ruby Thread:
   TR::ParameterSymbol *parmSymbol   = comp()->getMethodSymbol()->getParameterList().getListHead()->getData();
   TR::SymbolReference *threadSymRef = comp()->getSymRefTab()->findOrCreateAutoSymbol(comp()->getMethodSymbol(), parmSymbol->getSlot(), parmSymbol->getDataType(), true, false, true, false);

   TR::ILOpCodes callOpCode = TR_RubyFE::SLOTSIZE == 8 ? TR::lcall : TR::icall;

   TR::SymbolReference* receiverTempSymRef = comp()->getSymRefTab()->getRubyInlinedReceiverTempSymRef(callSite);
   TR_ASSERT(receiverTempSymRef != NULL, "NULL receiverTempSymRef, findCallSiteTarget didn't store receiver to a temporary.");

   TR::TreeTop* vm_frame_setup_call_TT = genCall(comp(), RubyHelper_vm_send_woblock_jit_inline_frame, callOpCode, 6,
                                                 TR::Node::createLoad(threadSymRef),
                                                 TR::Node::aconst((uintptr_t)ci), //Can we just anchor and recycle the above child nodes? or different blocks?
                                                 TR::Node::aconst((uintptr_t)cc),
                                                 TR::Node::aconst((uintptr_t)iseq),
                                                 TR::Node::createLoad(receiverTempSymRef),
                                                 TR::Node::aconst((uintptr_t)cc->me)
                                                );

   startOfInlinedCall->insertBefore(vm_frame_setup_call_TT);
   }



