
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_compiler as a shared library
add_library(lyric_compiler SHARED)
add_library(lyric::lyric_compiler ALIAS lyric_compiler)

set(LYRIC_COMPILER_INCLUDES
    include/lyric_compiler/compiler_result.h
    include/lyric_compiler/lyric_compiler.h
    include/lyric_compiler/module_entry.h
    )
set_target_properties(lyric_compiler PROPERTIES PUBLIC_HEADER "${LYRIC_COMPILER_INCLUDES}")

target_sources(lyric_compiler PRIVATE
    src/compiler_result.cpp
    src/lyric_compiler.cpp
    src/module_entry.cpp

    include/lyric_compiler/internal/compiler_utils.h
    src/internal/compiler_utils.cpp
    include/lyric_compiler/internal/compile_assignment.h
    src/internal/compile_assignment.cpp
    include/lyric_compiler/internal/compile_block.h
    src/internal/compile_block.cpp
    include/lyric_compiler/internal/compile_call.h
    src/internal/compile_call.cpp
    include/lyric_compiler/internal/compile_conditional.h
    src/internal/compile_conditional.cpp
    include/lyric_compiler/internal/compile_constant.h
    src/internal/compile_constant.cpp
    include/lyric_compiler/internal/compile_defclass.h
    src/internal/compile_defclass.cpp
    include/lyric_compiler/internal/compile_defconcept.h
    src/internal/compile_defconcept.cpp
    include/lyric_compiler/internal/compile_defenum.h
    src/internal/compile_defenum.cpp
    include/lyric_compiler/internal/compile_definstance.h
    src/internal/compile_definstance.cpp
    include/lyric_compiler/internal/compile_defstatic.h
    src/internal/compile_defstatic.cpp
    include/lyric_compiler/internal/compile_defstruct.h
    src/internal/compile_defstruct.cpp
    include/lyric_compiler/internal/compile_def.h
    src/internal/compile_def.cpp
    include/lyric_compiler/internal/compile_deref.h
    src/internal/compile_deref.cpp
    include/lyric_compiler/internal/compile_import.h
    src/internal/compile_import.cpp
    include/lyric_compiler/internal/compile_lambda.h
    src/internal/compile_lambda.cpp
    include/lyric_compiler/internal/compile_loop.h
    src/internal/compile_loop.cpp
    include/lyric_compiler/internal/compile_match.h
    src/internal/compile_match.cpp
    include/lyric_compiler/internal/compile_module.h
    src/internal/compile_module.cpp
    include/lyric_compiler/internal/compile_namespace.h
    src/internal/compile_namespace.cpp
    include/lyric_compiler/internal/compile_node.h
    src/internal/compile_node.cpp
    include/lyric_compiler/internal/compile_new.h
    src/internal/compile_new.cpp
    include/lyric_compiler/internal/compile_operator.h
    src/internal/compile_operator.cpp
    include/lyric_compiler/internal/compile_initializer.h
    src/internal/compile_initializer.cpp
    include/lyric_compiler/internal/compile_return.h
    src/internal/compile_return.cpp
    include/lyric_compiler/internal/compile_unwrap.h
    src/internal/compile_unwrap.cpp
    include/lyric_compiler/internal/compile_using.h
    src/internal/compile_using.cpp
    include/lyric_compiler/internal/compile_val.h
    src/internal/compile_val.cpp
    include/lyric_compiler/internal/compile_var.h
    src/internal/compile_var.cpp
    )

# set the library version
set_target_properties(lyric_compiler PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_compiler PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_compiler PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_compiler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(lyric_compiler
    PUBLIC
    lyric::lyric_assembler
    lyric::lyric_importer
    lyric::lyric_parser
    lyric::lyric_object
    lyric::lyric_rewriter
    lyric::lyric_typing
    tempo::tempo_tracing
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    icu::icu
    )

# install targets
install(TARGETS lyric_compiler EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_compiler
    )

# add testing subdirectory
#add_subdirectory(test)
