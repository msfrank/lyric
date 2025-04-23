#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_typing_fixture.h"
#include "test_callable.h"

class CallsiteReifierTP0In : public BaseTypingFixture {};

TEST_F(CallsiteReifierTP0In, UnaryFunctionGivenT_P0takesT_returnsBool)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    auto *typeCache = m_objectState->typeCache();

    lyric_common::SymbolUrl entryUrl(lyric_common::SymbolPath({"$entry"}));
    lyric_assembler::BlockHandle rootBlock(m_objectState.get());
    auto proc = std::make_unique<lyric_assembler::ProcHandle>(entryUrl, &rootBlock, m_objectState.get());

    lyric_object::TemplateParameter tp;
    tp.index = 0;
    tp.name = "T";
    tp.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    tp.bound = lyric_object::BoundType::None;
    tp.variance = lyric_object::VarianceType::Invariant;

    lyric_common::SymbolUrl templateUrl(lyric_common::SymbolPath({"sym"}));
    lyric_assembler::TemplateHandle *templateHandle;
    TU_ASSIGN_OR_RAISE (templateHandle, typeCache->makeTemplate(templateUrl, {tp}, proc->procBlock()));

    lyric_assembler::Parameter p0;
    p0.index = 0;
    p0.typeDef = templateHandle->getPlaceholder(0);
    p0.placement = lyric_object::PlacementType::List;

    lyric_assembler::CallableInvoker invoker;
    auto callable = std::unique_ptr<TestCallable>(new TestCallable({p0}, {}, {}, templateHandle));
    ASSERT_TRUE (invoker.initialize(std::move(callable)).isOk());

    // simulate the function f[T](p0: T): Bool
    lyric_typing::CallsiteReifier reifier(m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize(invoker).isOk());

    // apply Int argument
    ASSERT_TRUE (reifier.reifyNextArgument(IntType).isOk());

    // result type should be Bool
    auto reifyReturnResult = reifier.reifyResult(BoolType);
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (BoolType, resultType);

    // verify 1 argument has been reified
    ASSERT_EQ (1, reifier.numReifiedArguments());
}