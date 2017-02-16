require 'test/unit'

class ListenerTest < Test::Unit::TestCase

   def test_class_definition
      pre_stats = RubyVM::listener_stats

      ListenerTest.class_eval do
         def new_method
         end
      end

      post_stats = RubyVM::listener_stats

      assert_equal(pre_stats[:DEFINE_CLASS] + 1, post_stats[:DEFINE_CLASS])
   end


   def test_bop_redefinition
      pre_stats = RubyVM::listener_stats


      String.class_eval do
         # Try not to break other methods.
         alias_method :old_plus, :+
         def +(s)
            old_plus(s)
         end
      end

      post_stats = RubyVM::listener_stats


      assert_equal(pre_stats[:BOP_REDEFINITION] + 1, post_stats[:BOP_REDEFINITION])
   end

end
