#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_fixture.h"
#include "lyric_assembler/class_symbol.h"

class CallsiteReifier : public BaseFixture {};

TEST_F(CallsiteReifier, NullaryFunction_takesNoArguments_returnsBool)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

    // simulate the function f(): Bool
    lyric_typing::CallsiteReifier reifier({}, {}, m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize().isOk());
    ASSERT_TRUE (reifier.isValid());

    // result type should be Bool
    auto reifyReturnResult = reifier.reifyResult(BoolType);
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (BoolType, resultType);

    // verify 0 arguments have been reified
    ASSERT_EQ (0, reifier.numArguments());
}

TEST_F(CallsiteReifier, UnaryFunction_P0takesInt_returnsInt)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    lyric_object::Parameter p0;
    p0.index = 0;
    p0.typeDef = IntType;
    p0.placement = lyric_object::PlacementType::List;

    // simulate the function f(p0: Int): Int
    lyric_typing::CallsiteReifier reifier({p0}, {}, m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize().isOk());
    ASSERT_TRUE (reifier.isValid());

    // apply Int argument
    ASSERT_TRUE (reifier.reifyNextArgument(IntType).isOk());

    // result type should be Int
    auto reifyReturnResult = reifier.reifyResult(IntType);
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);

    // verify 1 argument has been reified
    ASSERT_EQ (1, reifier.numArguments());
}

TEST_F(CallsiteReifier, BinaryFunction_P0takesInt_P1takesFloat_returnsInt)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto FloatType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    lyric_object::Parameter p0;
    p0.index = 0;
    p0.typeDef = IntType;
    p0.placement = lyric_object::PlacementType::List;

    lyric_object::Parameter p1;
    p1.index = 0;
    p1.typeDef = FloatType;
    p1.placement = lyric_object::PlacementType::List;

    // simulate the function f(p0: Int, p1: Float): Int
    lyric_typing::CallsiteReifier reifier({p0, p1}, {}, m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize().isOk());
    ASSERT_TRUE (reifier.isValid());

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
    ASSERT_EQ (2, reifier.numArguments());
}