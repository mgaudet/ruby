/* -*- mode: c; style: ruby; -*-
 *
 * Licensed Materials - Property of IBM
 * "Restricted Materials of IBM"
 *
 * (c) Copyright IBM Corp. 2014 All Rights Reserved
 */

#ifndef JIT_H
#define JIT_H

#include "ruby.h"       /* For VALUE */
#include "method.h"     /* For iseq_t */
#include "vm_exec.h"
#include "vm_core.h"

/** 
 * 
 * This struct contains function pointers that are used by the JIT to callback. 
 *
 * The function pointers can be broken into two groups. The first group are
 * generated callbacks; These are callbacks emitted by insns.def, the
 * instruction definition file. 
 *
 * The second segment contains callbacks to VM functionality that may be 
 * required by JITs. 
 */
struct jit_callbacks_struct {

#ifdef JIT_INTERFACE
   /* Include generated callbacks. */
   #include "vm_jit_callbacks.inc" 
#endif


   /* JIT required callbacks. */ 
   const char *         (*rb_id2name_f)                 (ID id);
   VALUE                (*rb_funcallv_f)                (VALUE, ID, int, const VALUE *);
   VALUE                (*vm_send_f)                    (rb_thread_t *th, CALL_INFO ci, CALL_CACHE cc, ISEQ blockiseq, rb_control_frame_t *reg_cfp);
   VALUE                (*vm_send_without_block_f)      (rb_thread_t *th, CALL_INFO ci, CALL_CACHE cc, VALUE recv);
   void                 (*lep_svar_set_f)               (rb_thread_t *th, const VALUE *lep, rb_num_t key, VALUE val);
   VALUE                (*vm_getivar_f)                 (VALUE obj, ID id, IC ic, struct rb_call_cache* cc, int is_attr);
   VALUE                (*vm_setivar_f)                 (VALUE obj, ID id, VALUE val, IC ic, struct rb_call_cache* cc, int is_attr);
   VALUE                (*rb_ary_new_capa_f)            (long n);
   VALUE                (*rb_ary_new_from_values_f)     (long n, const VALUE *elts);
   void                 (*vm_expandarray_f)             (rb_control_frame_t *cfp, VALUE ary, rb_num_t num, int flag);
   VALUE                (*rb_ary_resurrect_f)           (VALUE ary);
   VALUE                (*rb_range_new_f)               (VALUE beg, VALUE end, int exclude_end);
   VALUE                (*rb_str_new_f)                 (const char *ptr, long len);
   VALUE                (*rb_hash_new_f)                ();
   VALUE                (*rb_hash_aset_f)               (VALUE hash, VALUE key, VALUE val);
   VALUE                (*rb_str_new_cstr_f)            (const char *ptr);
   VALUE                (*rb_str_resurrect_f)           (VALUE str);
   VALUE                (*vm_get_ev_const_f)            (rb_thread_t *th,  VALUE orig_klass, ID id, int is_defined);
   void                 (*vm_check_if_namespace_f)      (VALUE klass);
   void                 (*rb_const_set_f)               (VALUE klass, ID id, VALUE val);
   VALUE                (*rb_gvar_get_f)                (struct rb_global_entry *entry);
   VALUE                (*rb_gvar_set_f)                (struct rb_global_entry *entry, VALUE val);
   void                 (*rb_iseq_add_mark_object_f)    (const rb_iseq_t *iseq, VALUE obj);
   VALUE                (*rb_obj_as_string_f)           (VALUE obj);
   VALUE                (*rb_str_append_f)              (VALUE str, VALUE str2);
   const VALUE*         (*rb_vm_ep_local_ep_f)          (const VALUE *ep);
   VALUE                (*vm_get_cbase_f)               (const VALUE *ep);
   rb_cref_t*           (*rb_vm_get_cref_f)             (const VALUE *ep);
   VALUE                (*vm_get_const_base_f)          (const VALUE *ep);
   VALUE                (*vm_get_cvar_base_f)           (const rb_cref_t* cref, rb_control_frame_t *cfp);
   VALUE                (*rb_cvar_get_f)                (VALUE klass, ID id);
   void                 (*rb_cvar_set_f)                (VALUE klass, ID id, VALUE val);
   VALUE                (*rb_ary_tmp_new_f)             (long capa);
   void                 (*rb_ary_store_f)               (VALUE ary, long idx, VALUE val);
   VALUE                (*rb_reg_new_ary_f)             (VALUE ary, int opt);
   VALUE                (*vm_invokesuper_f)             (rb_thread_t *th, CALL_INFO ci, CALL_CACHE cc, ISEQ blockiseq, rb_control_frame_t *cfp);
   VALUE                (*vm_invokeblock_f)             (rb_thread_t *th, CALL_INFO ci);
   VALUE                (*rb_class_of_f)                (VALUE obj);
   rb_method_entry_t *  (*rb_method_entry_f)            (VALUE klass, ID id, VALUE *defined_class_ptr);
   void                 (*rb_bug_f)                     (const char *, ...);
   VALUE                (*vm_exec_core_f)               (rb_thread_t *th, VALUE initial);
   const char *         (*rb_class2name_f)              (VALUE klass); 
   /*
   void                 (*vm_send_woblock_jit_inline_frame_f)(rb_thread_t *th, CALL_INFO ci, VALUE recv);
   VALUE                (*vm_send_woblock_inlineable_guard_f)(CALL_INFO ci, VALUE klass);
   */
   void                 (*rb_threadptr_execute_interrupts_f) (rb_thread_t *th, int blocking_timing);
#ifdef OMR_RUBY_VALID_CLASS
    int                 (*ruby_omr_is_valid_object_f)   (VALUE object);
#endif
    void                (*rb_vm_env_write_f)            (const VALUE*, int, VALUE); 
    void                (*vm_jit_stack_check_f)         (rb_thread_t*, rb_control_frame_t* cfp); 
    VALUE               (*rb_str_freeze_f)              (VALUE);
    VALUE               (*rb_ivar_set_f)                (VALUE, ID, VALUE);
    OFFSET              (*vm_compute_case_dest_f)       (CDHASH,OFFSET,VALUE);  


};

typedef struct jit_callbacks_struct jit_callbacks_t;

typedef enum jit_options_bits {
    TIERED_COMPILATION     = 0x1,
    CODE_CACHE_RECLAMATION = 0x2,
} jit_options_bits;

/** 
 * This struct contains pointers to various global variables 
 * JITed bodies may wish to reference. 
 */
struct jit_globals_struct {
    rb_serial_t         *ruby_vm_global_constant_state_ptr;
    short               *redefined_flag_ptr; /* can't prefix with ruby_vm as a macro fires */
    VALUE               *ruby_rb_mRubyVMFrozenCore_ptr;
    rb_event_flag_t     *ruby_vm_event_flags_ptr; 
};
typedef struct jit_globals_struct jit_globals_t;


/**
 * The JIT emits functions that are best described as taking the 
 * thread as an argument. 
 *
 * These function's aren't called directly to support platform
 * independence, so these are instead passed to a JIT provided
 * dispatch function. 
 */
typedef VALUE (*jit_method_t)(rb_thread_t*);

/**
 * An interface for connecting to a JIT. 
 */
struct rb_jit_struct {
   /** The dlopen handle to the JIT shared library. */
    void     *dll_handle;
   
   /** Default compilation count. */
    int       default_count;

   /** Tiered compilation. */
    int       tiered_compilation;
    
   /** temporary flag for ruby recompilation */
    int       ruby_recomp_available;
    
   /** Used for options bits */
    long      options;
    
   /*
    * vm->jit interface
    */
   
   /** Initialize the JIT for the given VM, and options string. */  
    int   (*init_f)(rb_vm_t *vm, char *options);
   
   /** Terminate the JIT for the given VM */  
    void  (*terminate_f)(rb_vm_t *vm);

   /** Compile a given instruction sequence */  
    void* (*compile_f)(const rb_iseq_t *iseq);

   /** JIT crash Handler  */  
    void (*crash_f)(void);

   /** Dispatch to compiled code. */
    VALUE (*dispatch_f)(rb_thread_t *th, jit_method_t code);

   /*
    * jit->vm interface
    */
    jit_callbacks_t callbacks;
    jit_globals_t   globals;
};
typedef struct rb_jit_struct rb_jit_t;

/**
 * Initialize the JIT for a VM
 */
void  vm_jit_init   (rb_vm_t *vm, jit_globals_t globals);

/**
 * Destroy the JIT for a VM
 */
void  vm_jit_destroy(rb_vm_t *vm);

/**
 * Compile an instruction sequence
 */
void *vm_jit_compile(rb_vm_t *vm, const rb_iseq_t *iseq);

#endif
