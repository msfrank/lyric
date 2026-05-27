#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/struct_symbol.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

#include "base_assembler_fixture.h"
#include "lyric_assembler/class_symbol.h"

class ClassSymbol : public BaseAssemblerFixture {};

TEST_F (ClassSymbol, PreparePrivateMethodFromInheritedReceiver)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *block = objectRoot->rootBlock();

    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);

    lyric_assembler::ClassSymbol *BaseClass;
    TU_ASSIGN_OR_RAISE (BaseClass, block->declareClass("Base", false, {}));
    TU_RAISE_IF_NOT_OK (BaseClass->finalizeClass(ObjectType));

    TU_RAISE_IF_STATUS (BaseClass->declareMethod("Method", /* isHidden= */ true));

    lyric_assembler::ClassSymbol *SubClass;
    TU_ASSIGN_OR_RAISE (SubClass, block->declareClass("Sub", false, {}));
    TU_RAISE_IF_NOT_OK (SubClass->finalizeClass(BaseClass->getTypeDef()));

    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    ASSERT_THAT (SubClass->prepareMethod("Method", SubClass->getTypeDef(), callable, true), tempo_test::IsOk());
}
