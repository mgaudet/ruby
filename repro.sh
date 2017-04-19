for i in `seq 1 10`; do RELEASE_GVL=1 ASYNC_COMPILATION_TRACE=1 ./miniruby test.rb -zzz=678 || exit; done; 
