/* -*- mode: c; style: ruby; -*-
 *
 * Licensed Materials - Property of IBM
 * "Restricted Materials of IBM"
 *
 * (c) Copyright IBM Corp. 2014 All Rights Reserved
 */

#include "jit.h" 
/* for dlopen and dlsym */
/* FIXME: do this more portably */
#include <dlfcn.h>
#include <assert.h> 

/* Forward declare functions */
VALUE rb_reg_new_ary(VALUE ary, int options);

#define JIT_DEFAULT_COUNT 1000

/**
 * Returns the name of the JIT DLL to be loaded. 
 */
extern const char * get_jit_dll_name();

/**
 * Return the options string to feed to jit_init. 
 */
extern char * get_jit_options(); 

/* Forward declare.  */ 
void verify_jit_callbacks(jit_callbacks_t *callbacks); 
void vm_jit_stack_check(rb_thread_t*, rb_control_frame_t * cfp); 

void
vm_jit_init(rb_vm_t *vm, jit_globals_t globals)
{
    char const  *dll_name     = NULL; 
    char        *jit_options  = NULL;
    char        *error        = NULL;
    char *handle = 0;
    rb_jit_t *jit = 0;
    int rc = 0;

    dll_name = get_jit_dll_name(); 
    if (!dll_name)
        goto not_enabled;
        
    jit_options = get_jit_options();
    if (!jit_options)
        goto not_enabled;

    /* FIXME: do this more portably */
    handle = dlopen(dll_name, RTLD_LAZY | RTLD_LOCAL);

    if (!handle) {
	error = dlerror();
	fprintf(stderr, "vm_jit_init: unable to load %s: %s\n", dll_name, error);
	goto load_problem;
    }

    jit = ruby_mimmalloc(sizeof(*jit)); /* FIXME: mimmalloc or ALLOC? */
    if (!jit) {
	fprintf(stderr, "[FATAL] failed to allocated memory (jit_init)\n");
	exit(EXIT_FAILURE);
    }

    /* vm->jit interface */
    jit->dll_handle  = handle;
    jit->default_count = JIT_DEFAULT_COUNT;
    jit->init_f        = dlsym(handle, "jit_init"); 
    jit->terminate_f = dlsym(handle, "jit_terminate");
    jit->compile_f   = dlsym(handle, "jit_compile");
    jit->dispatch_f  = dlsym(handle, "jit_dispatch");

    if (!jit->init_f      ||
	!jit->terminate_f ||
	!jit->compile_f   ||
        !jit->dispatch_f   ) {
	fprintf(stderr, "vm_jit_init: missing symbols in %s: init %p, terminate %p, compile %p, dispatch %p\n",
                dll_name,
                jit->init_f,
                jit->terminate_f,
                jit->compile_f,
                jit->dispatch_f);
	goto symbol_problem;
    }

    /*
     * Initialze callbacks
     */ 
    memset(&(jit->callbacks), 0, sizeof(jit->callbacks)); 
#ifdef JIT_INTERFACE
    #include "vm_jit_initializations.inc" 
#endif

    
    jit->callbacks.rb_id2name_f             = rb_id2name;
    jit->callbacks.rb_funcallv_f            = rb_funcallv;
    jit->callbacks.vm_send_f                = vm_send;
    jit->callbacks.vm_send_without_block_f  = vm_send_without_block;
    jit->callbacks.lep_svar_set_f           = lep_svar_set;
    jit->callbacks.vm_getivar_f             = vm_getivar;
    jit->callbacks.vm_setivar_f             = vm_setivar;
    jit->callbacks.rb_ary_new_capa_f        = rb_ary_new_capa;
    jit->callbacks.rb_ary_new_from_values_f = rb_ary_new_from_values;
    jit->callbacks.vm_expandarray_f         = vm_expandarray;
    jit->callbacks.rb_ary_resurrect_f       = rb_ary_resurrect;
    jit->callbacks.rb_range_new_f           = rb_range_new;
    jit->callbacks.rb_str_new_f             = rb_str_new;
    jit->callbacks.rb_hash_new_f            = rb_hash_new;
    jit->callbacks.rb_hash_aset_f           = rb_hash_aset;
    jit->callbacks.rb_str_new_cstr_f        = rb_str_new_cstr;
    jit->callbacks.rb_str_resurrect_f       = rb_str_resurrect;
    jit->callbacks.vm_get_ev_const_f        = vm_get_ev_const;
    jit->callbacks.vm_check_if_namespace_f  = vm_check_if_namespace;
    jit->callbacks.rb_const_set_f           = rb_const_set;
    jit->callbacks.rb_gvar_get_f            = rb_gvar_get;
    jit->callbacks.rb_gvar_set_f            = rb_gvar_set;
    jit->callbacks.rb_iseq_add_mark_object_f= rb_iseq_add_mark_object;
    jit->callbacks.rb_obj_as_string_f       = rb_obj_as_string;
    jit->callbacks.rb_str_append_f          = rb_str_append;
    jit->callbacks.rb_vm_ep_local_ep_f      = rb_vm_ep_local_ep;
    jit->callbacks.vm_get_cbase_f           = vm_get_cbase;
    jit->callbacks.rb_vm_get_cref_f         = rb_vm_get_cref;
    jit->callbacks.vm_get_const_base_f      = vm_get_const_base;
    jit->callbacks.vm_get_cvar_base_f       = vm_get_cvar_base;
    jit->callbacks.rb_cvar_get_f            = rb_cvar_get;
    jit->callbacks.rb_cvar_set_f            = rb_cvar_set;
    jit->callbacks.rb_ary_tmp_new_f         = rb_ary_tmp_new;
    jit->callbacks.rb_ary_store_f           = rb_ary_store;
    jit->callbacks.rb_reg_new_ary_f         = rb_reg_new_ary;
    jit->callbacks.vm_invokesuper_f         = vm_invokesuper;
    jit->callbacks.vm_invokeblock_f         = vm_invoke_block_wrapper;
    jit->callbacks.rb_class_of_f            = rb_class_of;
    jit->callbacks.rb_method_entry_f        = method_entry_get_without_cache;
    jit->callbacks.rb_bug_f                 = rb_bug;
    jit->callbacks.vm_exec_core_f           = vm_exec_core;
    jit->callbacks.rb_class2name_f          = rb_class2name;
    /*
    jit->callbacks.vm_send_woblock_jit_inline_frame_f = vm_send_woblock_jit_inline_frame;
    jit->callbacks.vm_send_woblock_inlineable_guard_f = vm_send_woblock_inlineable_guard;
    */
    jit->callbacks.rb_threadptr_execute_interrupts_f  = rb_threadptr_execute_interrupts;
#ifdef OMR_RUBY_VALID_CLASS
    jit->callbacks.ruby_omr_is_valid_object_f                   = rb_omr_is_valid_object;
#endif
    jit->callbacks.rb_vm_env_write_f        = rb_vm_env_write; 
    jit->callbacks.vm_jit_stack_check_f     = vm_jit_stack_check; 

    verify_jit_callbacks(&jit->callbacks); 
    
    /* Initialize Globals */
    jit->globals = globals;

    vm->jit = jit;
    
    /* Initialize options bit to 0 */
    jit->options = 0;

    /* Initialize the JIT */
    rc = (jit->init_f)(vm, jit_options);
    if (rc != 0)
       {
       fprintf(stderr, "vm_jit_init: non-zero return for jit_init_f.\n"); 
	goto init_problem;
       }

    return;

  init_problem:
    vm->jit = 0;
    /* fall through */

  symbol_problem:
    /* need to free jit */
    ruby_mimfree(jit);
    jit = 0;
    /* fall through */

  load_problem:
  not_enabled:
    assert(!vm->jit && "jit shouldn't have been allocated/published in this case");
    return;
}

/**
 * Ensure that all members of the callback struct have been assigned. 
 */
void 
verify_jit_callbacks(jit_callbacks_t *callbacks)
{
   /* Treat the array as a buffer of void pointers 
    */
   unsigned int i = 0;
   void **buffer = (void**)callbacks; 
   for (; i < sizeof(jit_callbacks_t) / sizeof(void*); i++) {
      if (buffer[i] == NULL) {
	fprintf(stderr, "[FATAL] Un-initialized callback at index %d\n", i);
	exit(EXIT_FAILURE);
      }
   }
   return;
}


/**
 * Destroy a JIT.
 */ 
void
vm_jit_destroy(rb_vm_t *vm)
{
    if (!vm->jit)
	return;

    /* Tell the JIT that we're terminating */
    (vm->jit->terminate_f)(vm);

    /* FIXME: do proper error handling on the dlclose. */
    /* FIXME: write this more portably */
    dlclose(vm->jit->dll_handle);
    ruby_mimfree(vm->jit);
    vm->jit = 0;
    /* fprintf(stderr, "vm_jit_destroy: unloaded the jit dll\n"); */
}

/**
 * If called before leaving a JIT frame this will enforce the same invariant 
 * as YARV's `leave` opcode. 
 */
void 
vm_jit_stack_check(rb_thread_t* th, rb_control_frame_t * cfp)
   {
   const VALUE *const bp = vm_base_ptr(cfp);
   if (cfp->sp != vm_base_ptr(cfp)) 
      {
      rb_bug("JIT Stack consistency error (sp: %"PRIdPTRDIFF", bp: %"PRIdPTRDIFF")",
             VM_SP_CNT(th, cfp->sp), VM_SP_CNT(th, bp));
      }
   }

