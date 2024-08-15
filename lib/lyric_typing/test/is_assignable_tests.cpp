#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>

#include "base_fixture.h"

class IsAssignable : public BaseFixture {};

TEST_F(IsAssignable, TypeIsAssignableToItself)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    ASSERT_TRUE (m_typeSystem->isAssignable(IntType, IntType).orElseThrow());
}

TEST_F(IsAssignable, TypeIsAssignableToDirectSupertype)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    ASSERT_TRUE (m_typeSystem->isAssignable(IntrinsicType, IntType).orElseThrow());
}

TEST_F(IsAssignable, TypeIsAssignableToSupertype)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    ASSERT_TRUE (m_typeSystem->isAssignable(AnyType, IntType).orElseThrow());
}

TEST_F(IsAssignable, TypeIsNotAssignableToSubtype)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    ASSERT_FALSE (m_typeSystem->isAssignable(IntType, AnyType).orElseThrow());
}