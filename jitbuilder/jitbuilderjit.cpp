#include <assert.h> 
#include "jit.h"                        // RubyVM jit header
#include "ruby/ruby.h"                  // Ruby types
#include "vm_core.h"                    // Ruby Core. 

#include "ilgen/MethodBuilder.hpp"      // Method Builder type
#include "RubyTypeDictionary.hpp"     // TypeDictionary
#include "Jit.hpp"                      // JitBuilder header

#include <string>

namespace Ruby { 

class ByteCodeBuilder : public TR::MethodBuilder 
   {
   // Name is a member to ensure it has a long enough lifetime to survive 
   // through compilation. 
   std::string _name; 
public: 
   ByteCodeBuilder(Ruby::TypeDictionary* dict, const rb_iseq_t* iseq, rb_vm_t* vm) :
      TR::MethodBuilder(dict),
      _name(computeName(iseq))
      {

      // Definitions of the Ruby function. 
      DefineFile(__FILE__); 
      DefineLine(LINETOSTR(__LINE__));
      DefineName(_name.c_str()); 
      /*
       * jit_method_t is defined as
       * 
       * typedef VALUE (*jit_method_t)(rb_thread_t*);
       */
      DefineReturnType(dict->getTypes().VALUE);
      DefineParameter("th", dict->getTypes().prb_thread_t);

      auto* callbacks = &(vm->jit->callbacks); 
      auto* globals   = &(vm->jit->callbacks); 
      assert(callbacks); 
      assert(globals); 
      // Common definitions usedfor building ruby functions. 
      DefineFunction("vm_exec_core", "vm_exec.c", "48",
                    (void*)callbacks->vm_exec_core_f,
                    dict->getTypes().VALUE,
                    2,
                    dict->getTypes().prb_thread_t,
                    dict->getTypes().VALUE);

      }


   /**
    * Build the IL for the function
    */
   virtual bool buildIL()
      {
      Return(Call("vm_exec_core", 2, Load("th"), ConstAddress(0)));
      return true; 
      }; 

private:

   std::string computeName(const rb_iseq_t* iseq) 
      {
      // TR_ASSERT(iseq->body, "iseq didn't have a body...."); 
      // TR_ASSERT(iseq->body->location, "body didn't have a location..."); 
      std::string path(RSTRING_PTR(iseq->body->location.path));
      std::string line = std::to_string(FIX2LONG(iseq->body->location.first_lineno)); 
      std::string label(RSTRING_PTR(iseq->body->location.label));
      
      return path + ":" + line + ":" + label;
      } 

   };

} /* namespace Ruby */


/*
 *
 * External interface
 *
 *
 */
extern "C" {

static rb_vm_t* ruby_jit_vm = 0;  
// Returns non-zero if an error occurs. 
int jit_init(rb_vm_t *vm, char* options) {
   bool ret = initializeJit();
   fprintf(stderr, "Initialized  jitbuilderjit, rc %d\n", ret);
   ruby_jit_vm = vm; 
   return ret ? 0 : 1;
}


/**
 * Terminate the JIT for the given VM
 */
void jit_terminate(rb_vm_t *vm) {
   shutdownJit();
}

/**
 * Compile a given instruction sequence
 */
void* jit_compile(const rb_iseq_t *iseq) {
   uint8_t* startPC;
   Ruby::TypeDictionary types; 
   Ruby::ByteCodeBuilder builder(&types, iseq, ruby_jit_vm);
   
   auto rc = compileMethodBuilder(&builder, &startPC); 
   fprintf(stderr, "Compiled method builder: rc %d, startPC %p", rc, startPC); 
   if (rc == 0) { 
      // Compilation was successful! 
      iseq_jit_body_info *body_info = ALLOC(iseq_jit_body_info);
      assert(body_info); 

      body_info->opt_level = 0;
      body_info->startPC = startPC;

      return body_info;
   }
   return NULL;
}

/*
 * JIT crash Handler
 */
void jit_crash(void) {
   fprintf(stderr, "JIT CRASH HANDLER INVOKED!\n");
}

/** Dispatch to compiled code. */
VALUE jit_dispatch(rb_thread_t *th, jit_method_t code) {
   fprintf(stderr, "About to dispatch to %p with argument %p\n", code, th);
   return code(th); 
}

} /* extern C */
