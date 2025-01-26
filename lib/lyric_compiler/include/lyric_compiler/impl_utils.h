#ifndef LYRIC_COMPILER_IMPL_UTILS_H
#define LYRIC_COMPILER_IMPL_UTILS_H

#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>

namespace lyric_compiler {

    tempo_utils::Result<lyric_assembler::ImplHandle *> resolve_impl_handle(
        const lyric_assembler::ImplReference &implRef,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::SymbolCache *symbolCache);

    tempo_utils::Status prepare_impl_action(
        const std::string &actionName,
        const lyric_assembler::ImplReference &implRef,
        lyric_assembler::CallableInvoker &invoker,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::SymbolCache *symbolCache);
}

#endif // LYRIC_COMPILER_IMPL_UTILS_H
