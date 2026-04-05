#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/struct_symbol.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

#include "base_assembler_fixture.h"

class StructSymbol : public BaseAssemblerFixture {};

TEST_F (StructSymbol, WriteStruct)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *globalNs = objectRoot->globalNamespace();
    auto *block = globalNs->namespaceBlock();

    auto RecordType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record);
    lyric_assembler::StructSymbol *superStruct;
    TU_ASSIGN_OR_RAISE (superStruct, block->resolveStruct(RecordType));

    auto declareStructResult = block->declareStruct("test", superStruct, false);
    ASSERT_THAT (declareStructResult, tempo_test::IsResult());
    auto *structSymbol = declareStructResult.getResult();

    TU_RAISE_IF_NOT_OK (globalNs->putTarget(structSymbol->getSymbolUrl()));

    lyric_object::LyricObject object;
    ASSERT_THAT (writeObjectWithEmptyEntry(object), tempo_test::IsOk());

    ASSERT_EQ (1, object.numStructs());
    auto walker = object.getStruct(0);
    ASSERT_TRUE (walker.isValid());
    ASSERT_EQ (structSymbol->getSymbolUrl().getSymbolPath(), walker.getSymbolPath());
}
