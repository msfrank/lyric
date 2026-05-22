#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>

#include "base_typing_fixture.h"

class IsAssignable : public BaseTypingFixture {};

TEST_F(IsAssignable, TypeIsAssignableToItself)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    ASSERT_TRUE (typeSystem->isAssignable(IntType, IntType).orElseThrow());
}

TEST_F(IsAssignable, TypeIsAssignableToDirectSupertype)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    ASSERT_TRUE (typeSystem->isAssignable(IntrinsicType, IntType).orElseThrow());
}

TEST_F(IsAssignable, TypeIsAssignableToSupertype)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    ASSERT_TRUE (typeSystem->isAssignable(AnyType, IntType).orElseThrow());
}

TEST_F(IsAssignable, TypeIsNotAssignableToSubtype)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    ASSERT_FALSE (typeSystem->isAssignable(IntType, AnyType).orElseThrow());
}