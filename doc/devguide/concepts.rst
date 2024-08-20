========
Concepts
========

Lyric Programming Language
--------------------------

Lyric is a multi-paradigm programming language with object-oriented and functional features,
taking inspiration from other languages such as Python, Scala, Haskell, and Rust.

Source File
-----------

Lyric source code is represented as UTF-8 encoded text. By convention, Lyric source code files,
or *source files*, have the file extension ``.ly``.

Modules
-------

A Lyric program may consist of multiple source files. Lyric considers each source file to be a
separate code *module*, and allows code to import and use symbols from other modules. Modules
are referred to via a URL syntax, allowing the build system and interpreter to load modules from
various locations in a uniform way. A URL for a module is referred to as a ``ModuleLocation``.