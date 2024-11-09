#ifndef LYRIC_COMPILER_LAMBDA_UTILS_H
#define LYRIC_COMPILER_LAMBDA_UTILS_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_typing/type_system.h>

#include "defenum_handler.h"
#include "impl_handler.h"
#include "member_handler.h"
#include "method_handler.h"

namespace lyric_compiler {

    tempo_utils::Result<lyric_assembler::CallSymbol *>
    define_lambda_builder(
        lyric_assembler::CallSymbol *lambdaCall,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::FundamentalCache *fundamentalCache,
        lyric_assembler::SymbolCache *symbolCache,
        lyric_typing::TypeSystem *typeSystem);
}

#endif // LYRIC_COMPILER_LAMBDA_UTILS_H
