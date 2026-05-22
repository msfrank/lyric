#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/type_cache.h>

#include "base_typing_fixture.h"

class CompareUnion : public BaseTypingFixture {};

TEST_F(CompareUnion, ComparisonToItselfIsEqual)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto ObjectOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, IntrinsicType}).orElseThrow();
    auto cmp = typeSystem->compareAssignable(ObjectOrIntrinsicType, ObjectOrIntrinsicType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareUnion, ComparisonToWidenedTypeUnionIsEqual)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto RecordType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto ObjectOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, IntrinsicType}).orElseThrow();
    auto ObjectOrRecordOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, RecordType, IntrinsicType}).orElseThrow();
    auto cmp = typeSystem->compareAssignable(ObjectOrRecordOrIntrinsicType, ObjectOrIntrinsicType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareUnion, ComparisonToNarrowedTypeUnionIsDisjoint)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto RecordType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto ObjectOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, IntrinsicType}).orElseThrow();
    auto ObjectOrRecordOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, RecordType, IntrinsicType}).orElseThrow();
    auto cmp = typeSystem->compareAssignable(ObjectOrIntrinsicType, ObjectOrRecordOrIntrinsicType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::DISJOINT, cmp);
}

TEST_F(CompareUnion, ComparisonToSingularMemberIsEqual)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto ObjectOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, IntrinsicType}).orElseThrow();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto cmp = typeSystem->compareAssignable(AnyType, ObjectOrIntrinsicType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}
