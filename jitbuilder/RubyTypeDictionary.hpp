#include "ilgen/TypeDictionary.hpp" 
#include "ruby.h"
#include "vm_core.h"

namespace Ruby { 

   struct rubyTypes { 
      TR::IlType* VALUE; 
      TR::IlType* prb_thread_t;
   };

   class TypeDictionary : public TR::TypeDictionary  { 
      rubyTypes types;
   public:
      TypeDictionary() : TR::TypeDictionary() { 
         types.VALUE            = toIlType<VALUE>(); 
         types.prb_thread_t     = toIlType<void*>(); //FIXME: NEED TO DEFINE STRUCT
      }

      rubyTypes &getTypes() { return types; } 

   }; 


} /* namespace ruby */
