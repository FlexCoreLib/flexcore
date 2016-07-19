How to use Flexcore in your project
===================================

There are two supported ways to include flexcore in your own project. Both of
these methods assume you use cmake as your build configuration tool.

git submodule
-------------

Add the flexcore repository as a git submodule of your project.

    $ git submodule add <flexcore repo URL> flexcore

The root CMakeLists.txt of your project should then include the line:

    add_subdirectory(flexcore)

To link flexcore with one of your targets add the commands:

    add_executeable(main main.cpp)
    target_link_libraries(main flexcore)

to CMakeLists.txt. This will add the proper include directories, link flags
and will force building in c++14 mode.

When checking out your project remember to also checkout submodules:

    $ git submodule init && git submodule update --recursive

It is not necessary to integrate flexcore as a git submodule; if you know where
the source tree is located you can call add_subdirectory with the full path to
flexcore sources.

find_package
------------

This is the recommended way of including flexcore. First build and install flexcore.

    $ git clone <flexcore repo URL> flexcore
    $ mkdir -p flexcore/build && cd flexcore/build
    $ cmake .. -DCMAKE_INSTALL_PREFIX=<path prefix>
    $ make install

Once flexcore has been installed, your projects may use flexcore by including
the following in their CMakeLists.txt:

    find_package(flexcore)
    add_executable(main main.cpp)
    target_link_libraries(main flexcore)

If you installed flexcore in a custom location then your cmake invocation will
probably need to include a value for flexcore_DIR (the path to the installed
flexcore-config.cmake).

    $ cmake -Dflexcore_DIR=<path prefix>/lib(32|64)/cmake/flexcore

The prefix is the one specified as CMAKE_INSTALL_PREFIX when installing
flexcore.
