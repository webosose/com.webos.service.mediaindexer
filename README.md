com.webos.service.mediaindexer
==============================

Summary
-------
Service which detects and indexes media sources

Description
-----------
Service which detects and indexes media sources

How to Build on Linux
=====================

## Dependencies

Below are the tools and libraries (and their minimum versions)
required to build com.webos.service.mediaindexer:

* cmake (version required by openwebos/cmake-modules-webos)
* openwebos/cmake-modules-webos cmake helpers
* gcc >= 8 (std::filesystem)
* make (any version)
* pkg-config 0.26
* glib >= 2.0
* GStreamer >= 1.0 (currently the file meta data extractor can only use GStreamer)
* libupnp >= 1.6 (optional - required for UPnP plugin)
* libmtp >= 1.0 (optional - required for MTP plugin)
* libedit >= 3.0 (optional - required for standalone mode)

Dependencies required to build documentation:

* doxygen
* plantuml (for UML diagram generation)

## Configuraton

In the cmake configuration in ./src/plugins is a define 'STORAGE_DEVS'
that controls the devices storage device pathes for the storage
plugin. The format is

    $ path,name,desc,...

This can be changed from the envrioment variable STORAGE_DEVS,
e. g. if you run the media indexer in the STANDALONE mode.

In the cmake configuration in ./src there is a define
'PARALLEL_META_EXTRACTION' that set the maximum number of parallel
threads for meta data extraction.

## Building

Once you have downloaded the source, enter the following to build it
(after changing into the directory under which it was downloaded):

    $ mkdir BUILD
    $ cd BUILD
    $ cmake ..
    $ make
    $ sudo make install

You can enforce standalone build mode (no dependencies from other
WebOS modules) if you run cmake with -DSTANDALONE=ON.

If you compile with GCC you may have to set CXX=g++-8 for the cmake
environment.

The directory under which the files are installed defaults to
`/usr/local/webos`. You can install them elsewhere by supplying a
value for `WEBOS_INSTALL_ROOT` when invoking `cmake`. For example:

    $ cmake -D WEBOS_INSTALL_ROOT:PATH=$HOME/projects/webosose ..
    $ make
    $ make install

will install the files in subdirectories of `$HOME/projects/webosose`.

Specifying `WEBOS_INSTALL_ROOT` also causes `pkg-config` to look in
that tree first before searching the standard locations. You can
specify additional directories to be searched prior to this one by
setting the `PKG_CONFIG_PATH` environment variable.

If not specified, `WEBOS_INSTALL_ROOT` defaults to `/usr/local/webos`.

To configure for a debug build, enter:

    $ cmake -D CMAKE_BUILD_TYPE:STRING=Debug ..

To see a list of the make targets that `cmake` has generated, enter:

    $ make help

For build inside webOS Yocto use recipe below ./meta-webosose and put
this into your webso-local.conf:

    $ EXTERNALSRC_pn-com.webos.service.mediaindexer = "/<path-to-media-indexer>/media-indexer/"
    $ EXTERNALSRC_BUILD_pn-com.webos.service.mediaindexer = "/<path-to-media-indexer>/media-indexer/build/"
    $ PR_append_pn-com.webos.service.mediaindexer =".local0"

To run valgrind checks you need to use valgrind.supp suppressor
file. Run e. g. like this

    $ valgrind --suppressions=valgrind.supp --log-file=vg.log --leak-check=full --tool=memcheck <path-to-media-indexer>

## Usage

If compiled with LUNA support use LUNA API to enable and run device
detection and media indexing, otherwise an interactive command shell
will be run, type help for usage.

## Uninstalling

From the directory where you originally ran `make install`, enter:

 $ [sudo] make uninstall

You will need to use `sudo` if you did not specify
`WEBOS_INSTALL_ROOT`.

# Copyright and License Information

Copyright (c) 2018 LG Electronics, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

SPDX-License-Identifier: Apache-2.0

