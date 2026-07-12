
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/conversion_utils.h>
#include <lyric_typing/impl_selector.h>
#include <lyric_typing/summon_reifier.h>

#include "lyric_compiler/compiler_result.h"

static tempo_utils::Result<lyric_assembler::ActionSymbol *>
resolve_action(
    lyric_assembler::FundamentalSymbol symbol,
    std::string_view actionName,
    lyric_assembler::FundamentalCache *fundamentalCache,
    lyric_assembler::SymbolCache *symbolCache)
{
    auto conceptUrl = fundamentalCache->getFundamentalUrl(symbol);
    auto conceptPath = conceptUrl.getSymbolPath();
    lyric_common::SymbolPath actionPath(conceptPath.getPath(), actionName);
    lyric_common::SymbolUrl actionUrl(conceptUrl.getModuleLocation(), actionPath);
    return symbolCache->getOrImportAction(actionUrl);
}

tempo_utils::Result<bool>
lyric_compiler::convert_operand(
    const lyric_common::TypeDef &fromType,
    const lyric_common::TypeDef &intoType,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment)
{
    TU_ASSERT (block != nullptr);

    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *symbolCache = state->symbolCache();

    lyric_assembler::ActionSymbol *actionSymbol;
    TU_ASSIGN_OR_RETURN (actionSymbol, resolve_action(
        lyric_assembler::FundamentalSymbol::Converter, "Convert", fundamentalCache, symbolCache));

    lyric_typing::SummonReifier summoner(state);

    TU_RETURN_IF_NOT_OK (summoner.initialize(actionSymbol));
    TU_RETURN_IF_NOT_OK (summoner.reifyNextArgument(fromType));
    TU_RETURN_IF_STATUS (summoner.reifyResult(intoType));
    TU_RETURN_IF_NOT_OK (summoner.finalize());

    lyric_typing::ImplSelector selector(&summoner, block);

    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    TU_RETURN_IF_NOT_OK (selector.select(callable, /* allowMissing= */ true));
    if (callable == nullptr)
        return false;

    lyric_typing::CallsiteReifier reifier(state);
    TU_RETURN_IF_NOT_OK (reifier.initialize(callable.get(), selector.getCallsiteArguments()));
    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(fromType));

    TU_RETURN_IF_STATUS (callable->invoke(block, reifier, fragment));
    return true;;
}
