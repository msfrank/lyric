cmake_minimum_required(VERSION 3.27)

project(lyric)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(IS_DEBUG_BUILD ON)
else()
    set(IS_DEBUG_BUILD OFF)
endif()

#######################################################################
#
# declare options
#
#######################################################################

set(RUNTIME_DISTRIBUTION_ROOT "${CMAKE_INSTALL_PREFIX}"
    CACHE STRING
    "The runtime distribution root directory. Defaults to CMAKE_INSTALL_PREFIX.")
message(STATUS "runtime distribution root is ${RUNTIME_DISTRIBUTION_ROOT}")

option(USE_SANITIZER "Enable sanitizer library." OFF)

set(SANITIZER ""
    CACHE STRING
    "The sanitizer to use. Must be one of 'address', 'thread', 'memory', 'ub', or 'leak'.")

set(SANITIZER_IGNORELIST ""
    CACHE PATH
    "Instruct sanitizer to ignore symbols specified in ignorelist.")

option(USE_PROFILER "Enable gperftools profiler." OFF)

#######################################################################
#
# configure compiler and linker
#
#######################################################################

# configure common compiler flags
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -Wno-deprecated-declarations")

# by default do not specify -O
set (OPTIMIZATION_LEVEL "")

# enable one of the sanitizer libraries if specified
if (USE_SANITIZER)
    if ("${SANITIZER}" STREQUAL "address")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
        set (OPTIMIZATION_LEVEL "-O1")
        message(STATUS "enabling address sanitizer library")
    elseif ("${SANITIZER}" STREQUAL "thread")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
        set (OPTIMIZATION_LEVEL "-O1")
        message(STATUS "enabling thread sanitizer library")
    elseif ("${SANITIZER}" STREQUAL "memory")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory")
        set (OPTIMIZATION_LEVEL "-O1")
        message(STATUS "enabling memory sanitizer library")
    elseif ("${SANITIZER}" STREQUAL "ub")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
        set (OPTIMIZATION_LEVEL "-O1")
        message(STATUS "enabling ub sanitizer library")
    elseif ("${SANITIZER}" STREQUAL "leak")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
        set (OPTIMIZATION_LEVEL "-O1")
        message(STATUS "enabling leak sanitizer library")
    else()
        message(WARNING "skipping unknown sanitizer ${SANITIZER}")
        set (SANITIZER "")
    endif ("${SANITIZER}" STREQUAL "address")
    # if a sanitizer is enabled and an ignorelist is specified, then add it to the cxx flags
    if (SANITIZER)
        if (SANITIZER_IGNORELIST)
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-ignorelist=${SANITIZER_IGNORELIST}")
            message(STATUS "using sanitizer ignorelist ${SANITIZER_IGNORELIST}")
        endif (SANITIZER_IGNORELIST)
    endif (SANITIZER)
else()
    message(STATUS "sanitizer not enabled")
endif (USE_SANITIZER)

#
if (NOT OPTIMIZATION_LEVEL)
    set(OPTIMIZATION_LEVEL "-O0 -fno-eliminate-unused-debug-types")
endif()

# set the optimization level
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTIMIZATION_LEVEL}")

# define the rpath for executable and library targets
if (APPLE)
    set(BIN_RPATH "@executable_path/../lib")
    set(LIB_RPATH "@loader_path")
    set(BOOTSTRAP_RPATH "@loader_path/..")
elseif(WIN32)
    # do nothing
else()
    set(BIN_RPATH "$ORIGIN/../lib")
    set(LIB_RPATH "$ORIGIN/")
    set(BOOTSTRAP_RPATH "$ORIGIN/..")
endif()

#######################################################################
#
# find required and optional dependencies
#
#######################################################################

# find required Tempo dependency
find_package(tempo REQUIRED)

# find required Abseil dependency
find_package(absl REQUIRED)

# find required Boost dependency
find_package(Boost REQUIRED)

# find required Antlr dependency
find_package(antlr REQUIRED)

# find required Flatbuffers dependency
find_package(flatbuffers REQUIRED)

# find required Rocksdb dependency
find_package(rocksdb REQUIRED)

# find required Openssl dependency
find_package(openssl REQUIRED)

# find required Uv dependency
find_package(uv REQUIRED)

# find required Fmt dependency
find_package(fmt REQUIRED)

# find required ICU dependency
find_package(icu REQUIRED)

# find java build tool dependency for antlr
find_package(Java REQUIRED)

# find required google test dependency
find_package(gtest REQUIRED)

# link to gperftools profiler library if specified
set (PROFILER_LIBRARIES "")
if (${USE_PROFILER})
    set (PROFILER_LIBRARIES "-lprofiler")
    message(STATUS "enabling gperftools profiler")
else()
    message(STATUS "gperftools profiler not enabled")
endif(${USE_PROFILER})

# include needed CMake features
include(CMakePackageConfigHelpers)
include(CTest)
include(GNUInstallDirs)
include(GoogleTest)

#######################################################################
#
# configure tools and paths
#
#######################################################################

# define path to flatc
set(FLATC ${FLATBUFFERS_FLATC})
message(STATUS "flatc command is ${FLATC}")

# define path to antlr tool
set(ANTLR
    ${Java_JAVA_EXECUTABLE}
    -Xmx500M
    -cp ${ANTLR_TOOL_JAR}
    org.antlr.v4.Tool
    )
message(STATUS "antlr command is ${ANTLR}")

#######################################################################
#
# define build constants
#
#######################################################################

set(FULL_PLATFORM "${CMAKE_SYSTEM_NAME}.${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}")
message(STATUS "full platform is ${FULL_PLATFORM}")

# define individual version components and version string
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1"
    MAJOR_VERSION
    "${LYRIC_PACKAGE_VERSION}")
string(REGEX REPLACE "[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1"
    MINOR_VERSION
    "${LYRIC_PACKAGE_VERSION}")
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1"
    PATCH_VERSION
    "${LYRIC_PACKAGE_VERSION}")
set(FULL_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
message(STATUS "version is ${FULL_VERSION}")

set(BOOTSTRAP_PRELUDE_LOCATION      "dev.zuri.bootstrap:/prelude")

# install paths
set(BOOTSTRAP_DIR_PREFIX            "${CMAKE_INSTALL_LIBDIR}/lyric-bootstrap-${FULL_VERSION}")

# build paths
set(LYRIC_BUILD_OUTPUT_ROOT         "${CMAKE_BINARY_DIR}/output")
set(LYRIC_BUILD_BIN_DIR             "${LYRIC_BUILD_OUTPUT_ROOT}/${CMAKE_INSTALL_BINDIR}")
set(LYRIC_BUILD_LIB_DIR             "${LYRIC_BUILD_OUTPUT_ROOT}/${CMAKE_INSTALL_LIBDIR}")
set(LYRIC_BUILD_BOOTSTRAP_DIR       "${LYRIC_BUILD_OUTPUT_ROOT}/${BOOTSTRAP_DIR_PREFIX}")

# runtime paths
set(LYRIC_RUNTIME_BOOTSTRAP_DIR     "${RUNTIME_DISTRIBUTION_ROOT}/${BOOTSTRAP_DIR_PREFIX}")

#######################################################################
#
# add targets from subdirectories
#
#######################################################################

add_subdirectory(bin)
add_subdirectory(doc)
add_subdirectory(lib)

#######################################################################
#
# declare the package test suite target
#
#######################################################################

set(EMPTY_CPP ${CMAKE_CURRENT_BINARY_DIR}/empty.cpp)
add_custom_command(
    OUTPUT ${EMPTY_CPP}
    COMMAND cmake -E touch ${EMPTY_CPP}
    )

add_executable(lyric_testsuite ${EMPTY_CPP})
target_link_libraries(lyric_testsuite
    PUBLIC gtest::gtest
    PRIVATE
    LyricAnalyzerTestSuite
    LyricArchiverTestSuite
    LyricAssemblerTestSuite
    LyricBootstrapTestSuite
    LyricBuildTestSuite
    LyricCommonTestSuite
    LyricCompilerTestSuite
    LyricObjectTestSuite
    LyricOptimizerTestSuite
    LyricParserTestSuite
    LyricRewriterTestSuite
    LyricSymbolizerTestSuite
    LyricTypingTestSuite
    )
gtest_discover_tests(lyric_testsuite DISCOVERY_TIMEOUT 30)

#######################################################################
#
# create cmake package config
#
#######################################################################

configure_package_config_file(
    cmake/lyric-config.cmake.in
    lyric-config.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/lyric"
    PATH_VARS
        BOOTSTRAP_DIR_PREFIX
    )

write_basic_package_version_file(lyric-config-version.cmake
    VERSION "${FULL_VERSION}"
    COMPATIBILITY SameMajorVersion
    )

install(EXPORT lyric-targets
    COMPONENT Devel
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/lyric"
    NAMESPACE lyric::
    )

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/lyric-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/lyric-config-version.cmake"
    COMPONENT Devel
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/lyric"
    )
