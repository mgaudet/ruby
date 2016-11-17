require 'test/unit'


class JITModuleTest < Test::Unit::TestCase 

   def addone(x)
      return x+1
   end

   def test_jit_control
      am = method(:addone)
      # No testing occurs in this module unless the jit exists. 
      if RubyVM::JIT::exists?
         assert_equal(false,RubyVM::JIT::compiled?(am) )
         assert_equal(true, RubyVM::JIT::compile(am) )
         assert_equal(true, RubyVM::JIT::compiled?(am) )
      end
   end

end

