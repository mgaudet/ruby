#include "ruby/ruby.h"
#include "internal.h"
#include "vm_core.h" 
#include "iseq.h" 

/*
 * call-seq:
 *      RubyVM::JIT::exists? -> boolean
 *
 * Returns true if a JIT is loaded and actitve
 */
VALUE
vm_jit_exists_p()
{
#if defined(JIT_INTERFACE)

   if (get_jit()) return Qtrue;
#endif
   return Qfalse;
}

VALUE iseqw_s_of(VALUE klass, VALUE body);

/*
 * call-seq:
 *    RubyVM::JIT::compiled?(method)   -> boolean 
 *
 * Returns true if the code associated with this method has been jitted. 
 */
VALUE
vm_jit_compiled_p(VALUE klass, VALUE method)
{
#if defined(JIT_INTERFACE)
   VALUE iseq = iseqw_s_of(klass,method);
   if (!NIL_P(iseq) && rb_iseqw_to_iseq(iseq)->jit.state == ISEQ_JIT_STATE_JITTED)
      return Qtrue; 
#endif

   return Qfalse; 


}

extern VALUE vm_jit(rb_jit_t*, rb_iseq_t *);
/*
 * call-seq:
 *    RubyVM::JIT::compile   -> boolean 
 *
 * Attempts to compile the method, and returns true if successful. 
 *
 */
VALUE
vm_jit_compile_method(VALUE klass, VALUE method)
{
#if defined(JIT_INTERFACE)
   VALUE iseq; 
   iseq = iseqw_s_of(klass,method);

   if (!NIL_P(iseq)) {
      // Discards const qualifier of iseq here. 
      // Will have to deal with this eventually.
      return vm_jit(get_jit(), (rb_iseq_t*)rb_iseqw_to_iseq(iseq));  
   }
#endif
   return Qfalse; 
}

VALUE rb_cJIT; 

void Init_jit(void)
{ 
   rb_cJIT = rb_define_class_under(rb_cRubyVM, "JIT", rb_cObject);    
   rb_define_singleton_method(rb_cJIT, "exists?",   vm_jit_exists_p, 0); 
   rb_define_singleton_method(rb_cJIT, "compiled?", vm_jit_compiled_p, 1); 
   rb_define_singleton_method(rb_cJIT, "compile",   vm_jit_compile_method, 1); 
}

