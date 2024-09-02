
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/internal/write_existentials.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_existential(
    const ExistentialSymbol *existentialSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (existentialSymbol != nullptr);

    auto existentialUrl = existentialSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(existentialUrl, existentialSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

//    // if existential is an imported symbol then we are done
//    if (existentialSymbol->isImported())
//        return {};

    return {};
}
