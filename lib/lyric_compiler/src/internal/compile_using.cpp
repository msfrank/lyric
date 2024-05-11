
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/internal/compile_using.h>
#include <lyric_parser/ast_attrs.h>

tempo_utils::Status
lyric_compiler::internal::compile_using(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstUsingClass, 1);

    // determine the using location if specified
    lyric_common::AssemblyLocation usingLocation;
    if (walker.hasAttr(lyric_parser::kLyricAstAssemblyLocation)) {
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstAssemblyLocation, usingLocation);
    }

    // import instances into the current block
    for (int i = 0; i < walker.numChildren(); i++) {
        auto symbolRef = walker.getChild(i);
        moduleEntry.checkClassOrThrow(symbolRef, lyric_schema::kLyricAstSymbolRefClass);

        lyric_common::SymbolPath symbolPath;
        moduleEntry.parseAttrOrThrow(symbolRef, lyric_parser::kLyricAstSymbolPath, symbolPath);
        lyric_common::SymbolUrl usingUrl(usingLocation, symbolPath);

        auto status = block->useSymbol(usingUrl);
        if (status.notOk())
            return status;
    }

    return CompilerStatus::ok();
}
