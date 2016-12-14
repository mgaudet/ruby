#!ruby -s
#
require 'mkmf'
require 'rbconfig' 

$VPATH << '$(topdir)' << '$(top_srcdir)'
$INCFLAGS << ' -I$(topdir) -I$(top_srcdir)'

create_makefile 'jit' 
