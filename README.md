flexcore {#mainpage}
========

A generic C++ library for dataflow programming.

Usage
-----
See the [getting started](GETTING_STARTED.md) page.

Installation
------------
Flexcore uses a cmake based build system.
After cloning run:

    $ git submodule init
    $ git submodule update

To compile and install:

    $ mkdir build && cd build
    $ cmake -DENABLE_TESTS=NO ..
    $ make install

The installation location can be customized in the usual cmake way:

    cmake -DCMAKE_INSTALL_PREFIX=<prefix>

On 32-bit platforms libraries will be placed in <prefix>/lib32, and on 64-bit
platforms in <prefix>/lib64.

To override the library dir suffix specify the variable -DLIB_SUFFIX= in the
call to cmake. The libraries will then be installed in
<prefix>/lib${LIB_SUFFIX}

To use flexcore in a cmake based project check the [usage](USING.md) document.

Dependencies
------------
- C++14 compatible compiler (tested with GCC-5.2, Clang-3.6)
- Boost (>=1.54) - thread/log/system libraries

External libraries that are included as git submodules, are located under
/src/3rdparty. Currently, flexcore includes the following libraries:

- adobe stlab forest library - A generic C++ library for graph representation
- cereal - A C++11 library for serialization

