#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_typing_fixture.h"
#include "test_callable.h"

class CallsiteReifierTP0Out : public BaseTypingFixture {};

TEST_F(CallsiteReifierTP0Out, NullaryFunctionGivenT_IntCallsiteTypeArgument_returnsT)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
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

    lyric_assembler::CallableInvoker invoker;
    auto callable = std::unique_ptr<TestCallable>(new TestCallable({}, {}, {}, templateHandle));
    ASSERT_TRUE (invoker.initialize(std::move(callable)).isOk());

    // simulate the function f[T](): T with the given callsite type arguments [Int]
    lyric_typing::CallsiteReifier reifier(m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize(invoker, {IntType}).isOk());

    // result type should be Int
    auto reifyReturnResult = reifier.reifyResult(templateHandle->getPlaceholder(0));
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);

    // verify 0 arguments have been reified
    ASSERT_EQ (0, reifier.numReifiedArguments());
}