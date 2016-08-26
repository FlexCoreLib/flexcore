flexcore {#mainpage}
========

A generic C++ library for dataflow programming.

Usage
-----
See the [getting started](docs/GETTING_STARTED.md) page.

Installation
------------
Flexcore uses a cmake based build system.
After cloning run:

    $ git submodule init
    $ git submodule update

To compile and install:

    $ mkdir build && cd build
    $ cmake -DFLEXCORE_ENABLE_TESTS=NO ..
    $ make install

The installation location can be customized in the usual cmake way:

    cmake -DCMAKE_INSTALL_PREFIX=<prefix>

On 32-bit platforms libraries will be placed in <prefix>/lib32, and on 64-bit
platforms in <prefix>/lib64.

To override the library dir suffix specify the variable -DLIB_SUFFIX= in the
call to cmake. The libraries will then be installed in
<prefix>/lib${LIB_SUFFIX}

To use flexcore in a cmake based project check the [usage](docs/USING.md) document.

To access the documentation in doxygen, execute doxygen from top level directory, not from /docs :

`doxygen docs/doxyfile`

Dependencies
------------
- C++14 compatible compiler (tested with GCC-5.2, Clang-3.6)
- Boost (>=1.54) - thread/log/system libraries

External libraries that are included as git submodules, are located under
/src/3rdparty. Currently, flexcore includes the following libraries:

- adobe stlab forest library - A generic C++ library for graph representation
- cereal - A C++11 library for serialization

License
------------
Copyright 2016 Caspar Kielwein

Licensed under the Apache License, Version 2.0 (the "License");  
you may not use this file except in compliance with the License.  
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software  
distributed under the License is distributed on an "AS IS" BASIS,  
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
See the License for the specific language governing permissions and
limitations under the License.  
See LICENSE.txt or http://www.apache.org/licenses/LICENSE-2.0

All files under flexcore/3rdparty are licensed by their respective copyright owners.  
adobe stlab forest is distributed under the Boost Software License, Version 1.0.  
cereal is licensed under the BSD license.
