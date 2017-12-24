libxcustomtitle
===============

An LD\_PRELOAD hack to change X window titles.

Copyright (C) 2015, 2017 Scott Zeid.  Released under the X11 License.  
<http://code.s.zeid.me/libxcustomtitle>


Usage
-----

Simply build libxcustomtitle (see below), set the `LD_PRELOAD` environment
variable to the path to the compiled library, and set `X_CUSTOM_TITLE` to the
desired window title.

    LD_PRELOAD=path/to/libxcustomtitle[32].so X_CUSTOM_TITLE=<title> \
     <X program> [arguments [...]]

For convenience, a `run` script is provided that looks in the directory in which
itself is located for the library:

    path/to/libxcustomtitle/run <title> <program> [arguments [...]]


Building
--------

1.  Make sure you're on GNU/Linux (yes, the GNU part matters here) and have
    the glibc and SDL 1.2 development packages installed, as well as GNU Make.
    
        fedora$ sudo yum install glibc-devel libX11-devel make

2.  In the repository's root directory, run `make`.


Cross-compiling
---------------

To cross-compile a 32-bit library on a 64-bit machine, run `make 32`.  You
can also run `make 64 32` to make both the 64-bit and 32-bit versions at the
same time.  Make sure you have the 32-bit development packages installed:

    fedora$ sudo yum install glibc-devel.i686 libX11-devel.i686

You can override the compiler and linker with the `CC` and `LD` variables:

    make CC=... LD=... [...]
