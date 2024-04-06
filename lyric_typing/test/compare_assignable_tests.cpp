#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/type_cache.h>

#include "base_fixture.h"

class CompareAssignable : public BaseFixture {};

TEST_F(CompareAssignable, CompareSingularToItself)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto cmp = m_typeSystem->compareAssignable(IntType, IntType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareAssignable, CompareSingularToDirectSupertype)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto cmp = m_typeSystem->compareAssignable(IntrinsicType, IntType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EXTENDS, cmp);
}

TEST_F(CompareAssignable, CompareSingularToSupertype)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto cmp = m_typeSystem->compareAssignable(AnyType, IntType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EXTENDS, cmp);
}

TEST_F(CompareAssignable, CompareSingularToSubtype)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto cmp = m_typeSystem->compareAssignable(IntType, AnyType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::SUPER, cmp);
}

TEST_F(CompareAssignable, CompareSingularToDisjointType)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto FloatType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
    auto cmp = m_typeSystem->compareAssignable(IntType, FloatType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::DISJOINT, cmp);
}

TEST_F(CompareAssignable, ComparePlaceholderToItself)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto *typeCache = m_assemblyState->typeCache();

    lyric_common::SymbolUrl entryUrl(lyric_common::SymbolPath({"$entry"}));
    auto proc = std::make_unique<lyric_assembler::ProcHandle>(entryUrl, m_assemblyState.get());

    lyric_object::TemplateParameter tp;
    tp.index = 0;
    tp.name = "T";
    tp.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    tp.bound = lyric_object::BoundType::None;
    tp.variance = lyric_object::VarianceType::Invariant;

    lyric_common::SymbolUrl templateUrl(lyric_common::SymbolPath({"sym"}));
    ASSERT_TRUE (typeCache->makeTemplate(templateUrl, {tp}, proc->procBlock()).isOk());
    lyric_assembler::TemplateHandle *templateHandle = typeCache->getTemplate(templateUrl);
    ASSERT_TRUE (templateHandle != nullptr);

    auto placeholder = templateHandle->getPlaceholder(0);

    auto cmp = m_typeSystem->compareAssignable(placeholder, placeholder).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareAssignable, CompareSingularMemberToTypeUnion)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto FloatType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
    auto IntOrFloatType = lyric_common::TypeDef::forUnion({IntType, FloatType});
    auto cmp = m_typeSystem->compareAssignable(IntOrFloatType, IntType).orElseThrow();

    // Int is a direct member of IntOrFloat, so comparison must be equal
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareAssignable, CompareSingularMemberSubtypeToTypeUnion)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto FloatType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
    auto ObjectOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, IntrinsicType});
    auto cmp = m_typeSystem->compareAssignable(ObjectOrIntrinsicType, FloatType).orElseThrow();

    // Int is a subtype of a member of ObjectOrIntrinsic, so comparison must be extends
    ASSERT_EQ (lyric_runtime::TypeComparison::EXTENDS, cmp);
}

TEST_F(CompareAssignable, CompareTypeUnionToTypeUnion)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto ObjectOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, IntrinsicType});
    auto cmp = m_typeSystem->compareAssignable(ObjectOrIntrinsicType, ObjectOrIntrinsicType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareAssignable, CompareTypeUnionToWidenedTypeUnion)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto RecordType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto ObjectOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, IntrinsicType});
    auto ObjectOrRecordOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, RecordType, IntrinsicType});
    auto cmp = m_typeSystem->compareAssignable(ObjectOrRecordOrIntrinsicType, ObjectOrIntrinsicType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareAssignable, CompareTypeUnionToNarrowedTypeUnion)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto RecordType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto ObjectOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, IntrinsicType});
    auto ObjectOrRecordOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, RecordType, IntrinsicType});
    auto cmp = m_typeSystem->compareAssignable(ObjectOrIntrinsicType, ObjectOrRecordOrIntrinsicType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::DISJOINT, cmp);
}

TEST_F(CompareAssignable, CompareTypeUnionToSingularMember)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto ObjectOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, IntrinsicType});
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto cmp = m_typeSystem->compareAssignable(AnyType, ObjectOrIntrinsicType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}
