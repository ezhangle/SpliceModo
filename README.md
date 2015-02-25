Fabric Splice for The Foundry Modo
===================================
A Fabric Splice integration for Modo.

Fabric Splice for Modo allows you to make use of the Fabric Core inside of Modo and use KL and the Fabric graph to perform computations inside of Modo using a custom Mesh item, a custom Schematic node etc.

Repository Status
=================

This repository will be maintained and kept up to date by a third party to match the latest Modo / Fabric Core.

Supported platforms
===================

To date all platforms for supported by Modo (windows, linux, osx) are supported.

Building
========

A scons (http://www.scons.org/) build script is provided. Fabric Splice for Modo depends on
* A static build of boost (http://www.boost.org/), version 1.55 or higher.
* A dynamic build of Fabric Core (matching the latest version).
* The SpliceAPI repository checked out one level above (http://github.com/fabric-engine/SpliceAPI)
* The Modo SDK

Fabric Splice for Modo requires a certain folder structure to build properly. You will need to have the SpliceAPI cloned as well on your drive, as such:

    SpliceAPI
    Applications/SpliceModo

You can use the bash script below to clone the repositories accordingly:

    git clone git@github.com:fabric-engine/SpliceAPI.git
    mkdir Applications
    cd Applications
    git clone git@github.com:fabric-engine/SpliceModo.git
    cd SpliceModo
    scons

To inform scons where to find the Fabric Core includes as well as the thirdparty libraries, you need to set the following environment variables:

* FABRIC_BUILD_OS: Should be the type of OS you are building for (Windows, Darwin, Linux)
* FABRIC_BUILD_ARCH: The architecture you are building for (x86, x86_64)
* FABRIC_BUILD_TYPE: The optimization type (Release, Debug)
* FABRIC_SPLICE_VERSION: Refers to the version you want to build. Typically the name of the branch (for example 1.13.0)
* FABRIC_DIR: Should point to Fabric Engine's installation folder.
* BOOST_DIR: Should point to the boost root folder (containing boost/ (includes) and lib/ for the static libraries).
* MODO_INCLUDE_DIR: The include folder of the MODO installation.
* MODO_LIB_DIR: The library folder of the MODO installation.
* MODO_VERSION: The Modo version to use. (for example: 901)

The temporary files will be built into the *.build* folder, while the structured output files will be placed in the *.stage* folder.

To perform a build you can just run

    scons all -j8

To clean the build you can run

    scons clean

License
==========

The license used for this DCC integration can be found in the root folder of this repository.
