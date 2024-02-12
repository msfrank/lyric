Lyric - Build system and runtime for the Zuri project
=====================================================

Copyright (c) 2024, Michael Frank

Overview
--------

Lyric is a collection of libraries implementing the object and packaging formats,
bytecode interpreter, runtime support, and build system used by the Zuri project.

Prerequisites
-------------

Lyric requires the following dependencies to be installed on the system:
```
CMake version 3.27 or greater:      https://cmake.org
Conan version 2:                    https://conan.io
Tempo:                              https://github.com/msfrank/tempo
```

Lyric also depends on the package recipes from Timbre and expects that the recipes
are exported into the conan2 package cache.

Quick Start
-----------

1. Navigate to the repository root.
1. Build and install Lyric using Conan:
  ```
conan create . --build=missing
  ```
