#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/type_cache.h>

#include "base_typing_fixture.h"

class ComparePlaceholder : public BaseTypingFixture {};

TEST_F(ComparePlaceholder, ComparisonToItselfIsEqual)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
    auto *typeCache = m_objectState->typeCache();

    lyric_common::SymbolUrl entryUrl(lyric_common::SymbolPath({"$entry"}));
    lyric_assembler::BlockHandle rootBlock(m_objectState.get());
    auto proc = std::make_unique<lyric_assembler::ProcHandle>(entryUrl, &rootBlock, m_objectState.get());

    lyric_object::TemplateParameter tp;
    tp.index = 0;
    tp.name = "T";
    tp.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    tp.bound = lyric_object::BoundType::Extends;
    tp.variance = lyric_object::VarianceType::Invariant;

    lyric_common::SymbolUrl templateUrl(lyric_common::SymbolPath({"sym"}));
    lyric_assembler::TemplateHandle *templateHandle;
    TU_ASSIGN_OR_RAISE (templateHandle, typeCache->makeTemplate(templateUrl, {tp}, proc->procBlock()));

    auto placeholder = templateHandle->getPlaceholder(0);

    auto cmp = m_typeSystem->compareAssignable(placeholder, placeholder).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}