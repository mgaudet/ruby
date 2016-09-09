This branch contains a modified version of the Ruby VM that builds the 
[Ruby + OMR Technology Preview](https://github.com/rubyomr-preview/rubyomr-preview) ,
*JIT component only*. 

# What's Ruby

Ruby is the interpreted scripting language for quick and easy object-oriented
programming.  It has many features to process text files and to do system
management tasks (as in Perl).  It is simple, straight-forward, and
extensible.

## Ruby + OMR Technology Preview

This is an experimental version of Ruby integrated with the [Eclipse OMR
Project](https://github.com/eclipse/omr) compiler technology.
Eclipse OMR is an open source C/C++ collection of language runtime technologies.

Visit [Ruby + OMR Technology Preview](https://github.com/rubyomr-preview/rubyomr-preview)
to get more details about this technology preview.

Our current Ruby + OMR changes are based on Ruby 2.2. We have created
a branch named ruby_2_2_omr, the default branch in this fork, containing a single
commit to show the changes needed to incorporate Eclipse OMR into Ruby.

The branch ruby_2_2_omr_jitonly contains the changes required to activate the JIT only. 

## Building Ruby + OMR

Simplified steps to build Ruby + OMR.  See more detailed instructions below to
modify the install location, etc.
```
$ git clone https://github.com/rubyomr-preview/ruby.git --branch ruby_2_2_omr --recursive 
$ cd ruby
$ autoconf
$ ./configure SPEC=<specname> --with-omr-jit
$ make
$ make install
```
Since the Ruby + OMR code has only been tested on Linux x86-64, Linux PPC-LE-64, Linux PPC-BE-64
and Linux 390-64 the acceptable values for `<specname>` are:
```
1. linux_x86-64
2. linux_ppc-64_le_gcc
3. linux_ppc-64
4. linux_390-64
```

## Running with the JIT compiler

Use the environment variable `OMR_JIT_OPTIONS` to pass options; Be sure to start the variable
with `-Xjit:` to activate the JIT.  

Some options of interest: 

| `-Xjit:` option        | Description                                                          | 
|------------------------|----------------------------------------------------------------------| 
| `count=`_N_.           | How many times a method needs to be invoked before it is compiled.   | 
| `verbose`              |  Outputs compilation decisions to `stdout`                           |
| `vlog=`_file_          |  Redirect compilation decision output to _file_                      |
| `tracefull,log=`_file_ | Produce a compilation log at _file_, suffixed with PID               |

### Running without installing

If you haven't run `make install`, the dynamic loader will complain. Tell it where to find 
`librbjit` by pointing `LD_LIBRARY_PATH` to this directory.

So, running `make test` without installing: 

    LD_LIBRARY_PATH=$PWD OMR_JIT_OPTIONS=-Xjit:count=0 make test

## Features of Ruby

*   Simple Syntax
*   **Normal** Object-Oriented features(ex. class, method calls)
*   **Advanced** Object-Oriented features(ex. Mix-in, Singleton-method)
*   Operator Overloading
*   Exception Handling
*   Iterators and Closures
*   Garbage Collection
*   Dynamic Loading of Object files(on some architecture)
*   Highly Portable (works on many Unix-like/POSIX compatible platforms as
    well as Windows, Mac OS X, BeOS etc.) cf.
    http://bugs.ruby-lang.org/projects/ruby-trunk/wiki/SupportedPlatforms


## How to get Ruby

For a complete list of ways to install Ruby, including using third party tools
like rvm, see:

http://www.ruby-lang.org/en/downloads/

The Ruby distribution files can be found in the following FTP site:

ftp://ftp.ruby-lang.org/pub/ruby/

The trunk of the Ruby source tree can be checked out with the following
command:

    $ svn co http://svn.ruby-lang.org/repos/ruby/trunk/ ruby

Or if you are using git then use the following command:

    $ git clone git://github.com/ruby/ruby.git

There are some other branches under development.  Try the following command
and see the list of branches:

    $ svn ls http://svn.ruby-lang.org/repos/ruby/branches/

Or if you are using git then use the following command:

    $ git ls-remote git://github.com/ruby/ruby.git

## Ruby home-page

The URL of the Ruby home-page is:

http://www.ruby-lang.org/

## Mailing list

There is a mailing list to talk about Ruby. To subscribe this list, please
send the following phrase

    subscribe

in the mail body (not subject) to the address
<mailto:ruby-talk-request@ruby-lang.org>.

## How to compile and install

This is what you need to do to compile and install Ruby:

1.  If you want to use Microsoft Visual C++ to compile ruby, read
    win32/README.win32 instead of this document.

2.  If `./configure` does not exist or is older than configure.in, run
    autoconf to (re)generate configure.

3.  Run `./configure`, which will generate config.h and Makefile.

    Some C compiler flags may be added by default depending on your
    environment.  Specify `optflags=..` and `warnflags=..` as necessary to
    override them.

4.  Edit `defines.h` if you need. Usually this step will not be needed.

5.  Remove comment mark(`#`) before the module names from `ext/Setup` (or add
    module names if not present), if you want to link modules statically.

    If you don't want to compile non static extension modules (probably on
    architectures which do not allow dynamic loading), remove comment mark
    from the line "`#option nodynamic`" in `ext/Setup`.

    Usually this step will not be needed.

6.  Run `make`.

7.  Optionally, run '`make check`' to check whether the compiled Ruby
    interpreter works well. If you see the message "`check succeeded`", your
    ruby works as it should (hopefully).

8.  Run '`make install`'

    This command will create following directories and install files onto
    them.

    *   `${DESTDIR}${prefix}/bin`
    *   `${DESTDIR}${prefix}/include/ruby-${MAJOR}.${MINOR}.${TEENY}`
    *   `${DESTDIR}${prefix}/include/ruby-${MAJOR}.${MINOR}.${TEENY}/${PLATFOR
        M}`
    *   `${DESTDIR}${prefix}/lib`
    *   `${DESTDIR}${prefix}/lib/ruby`
    *   `${DESTDIR}${prefix}/lib/ruby/${MAJOR}.${MINOR}.${TEENY}`
    *   `${DESTDIR}${prefix}/lib/ruby/${MAJOR}.${MINOR}.${TEENY}/${PLATFORM}`
    *   `${DESTDIR}${prefix}/lib/ruby/site_ruby`
    *   `${DESTDIR}${prefix}/lib/ruby/site_ruby/${MAJOR}.${MINOR}.${TEENY}`
    *   `${DESTDIR}${prefix}/lib/ruby/site_ruby/${MAJOR}.${MINOR}.${TEENY}/${P
        LATFORM}`
    *   `${DESTDIR}${prefix}/lib/ruby/vendor_ruby`
    *   `${DESTDIR}${prefix}/lib/ruby/vendor_ruby/${MAJOR}.${MINOR}.${TEENY}`
    *   `${DESTDIR}${prefix}/lib/ruby/vendor_ruby/${MAJOR}.${MINOR}.${TEENY}/$
        {PLATFORM}`
    *   `${DESTDIR}${prefix}/lib/ruby/gems/${MAJOR}.${MINOR}.${TEENY}`
    *   `${DESTDIR}${prefix}/share/man/man1`
    *   `${DESTDIR}${prefix}/share/ri/${MAJOR}.${MINOR}.${TEENY}/system`


    If Ruby's API version is '*x.y.z*', the `${MAJOR}` is '*x*', the
    `${MINOR}` is '*y*', and the `${TEENY}` is '*z*'.

    **NOTE**: teeny of the API version may be different from one of Ruby's
    program version

    You may have to be a super user to install ruby.


If you fail to compile ruby, please send the detailed error report with the
error log and machine/OS type, to help others.

Some extension libraries may not get compiled because of lack of necessary
external libraries and/or headers, then you will need to run '`make
distclean-ext`' to remove old configuration after installing them in such
case.

## Copying

See the file `COPYING`.

## Feedback

Questions about the Ruby language can be asked on the Ruby-Talk mailing list
(http://www.ruby-lang.org/en/community/mailing-lists) or on websites like
(http://stackoverflow.com).

Bug reports should be filed at http://bugs.ruby-lang.org

## The Author

Ruby was originally designed and developed by Yukihiro Matsumoto (Matz) in
1995.

<mailto:matz@ruby-lang.org>
