#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_fixture.h"
#include "test_callable.h"

class CallsiteReifier : public BaseFixture {};

TEST_F(CallsiteReifier, NullaryFunction_takesNoArguments_returnsBool)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

    lyric_assembler::CallableInvoker invoker;
    auto callable = std::unique_ptr<TestCallable>(new TestCallable({}, {}, {}));
    ASSERT_TRUE (invoker.initialize(std::move(callable)).isOk());

    // simulate the function f(): Bool
    lyric_typing::CallsiteReifier reifier(m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize(invoker).isOk());

    // result type should be Bool
    auto reifyReturnResult = reifier.reifyResult(BoolType);
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (BoolType, resultType);

    // verify 0 arguments have been reified
    ASSERT_EQ (0, reifier.numReifiedArguments());
}

TEST_F(CallsiteReifier, UnaryFunction_P0takesInt_returnsInt)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    lyric_assembler::Parameter p0;
    p0.index = 0;
    p0.typeDef = IntType;
    p0.placement = lyric_object::PlacementType::List;

    lyric_assembler::CallableInvoker invoker;
    auto callable = std::unique_ptr<TestCallable>(new TestCallable({p0}, {}, {}));
    ASSERT_TRUE (invoker.initialize(std::move(callable)).isOk());

    // simulate the function f(p0: Int): Int
    lyric_typing::CallsiteReifier reifier(m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize(invoker).isOk());

    // apply Int argument
    ASSERT_TRUE (reifier.reifyNextArgument(IntType).isOk());

    // result type should be Int
    auto reifyReturnResult = reifier.reifyResult(IntType);
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);

    // verify 1 argument has been reified
    ASSERT_EQ (1, reifier.numReifiedArguments());
}

TEST_F(CallsiteReifier, BinaryFunction_P0takesInt_P1takesFloat_returnsInt)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
    auto FloatType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    lyric_assembler::Parameter p0;
    p0.index = 0;
    p0.typeDef = IntType;
    p0.placement = lyric_object::PlacementType::List;

    lyric_assembler::Parameter p1;
    p1.index = 1;
    p1.typeDef = FloatType;
    p1.placement = lyric_object::PlacementType::List;

    lyric_assembler::CallableInvoker invoker;
    auto callable = std::unique_ptr<TestCallable>(new TestCallable({p0, p1}, {}, {}));
    ASSERT_TRUE (invoker.initialize(std::move(callable)).isOk());

    // simulate the function f(p0: Int, p1: Float): Int
    lyric_typing::CallsiteReifier reifier(m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize(invoker).isOk());

    // apply Int argument
    ASSERT_TRUE (reifier.reifyNextArgument(IntType).isOk());
    // apply Float argument
    ASSERT_TRUE (reifier.reifyNextArgument(FloatType).isOk());

    // result type should be Int
    auto reifyReturnResult = reifier.reifyResult(IntType);
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);

    // verify 2 arguments have been reified
    ASSERT_EQ (2, reifier.numReifiedArguments());
}