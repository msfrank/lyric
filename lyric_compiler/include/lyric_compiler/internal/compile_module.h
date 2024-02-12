#ifndef LYRIC_COMPILER_COMPILE_MODULE_H
#define LYRIC_COMPILER_COMPILE_MODULE_H

#include <lyric_compiler/module_entry.h>
#include <lyric_object/lyric_object.h>
#include <lyric_parser/node_walker.h>

namespace lyric_compiler::internal {

    tempo_utils::Result<lyric_object::LyricObject>
    compile_module(
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &entry,
        bool touchExternalSymbols);
}

#endif // LYRIC_COMPILER_COMPILE_MODULE_H
