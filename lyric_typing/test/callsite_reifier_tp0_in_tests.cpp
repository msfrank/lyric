#include <gtest/gtest.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_fixture.h"

class CallsiteReifierTP0In : public BaseFixture {};

TEST_F(CallsiteReifierTP0In, UnaryFunctionGivenT_P0takesT_returnsBool)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

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

    lyric_object::Parameter p0;
    p0.index = 0;
    p0.typeDef = templateHandle->getPlaceholder(0);
    p0.placement = lyric_object::PlacementType::List;

    // simulate the function f[T](p0: T): Bool
    lyric_typing::CallsiteReifier reifier({p0}, {}, templateUrl, templateHandle->getTemplateParameters(),
        {}, m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize().isOk());
    ASSERT_TRUE (reifier.isValid());

    // apply Int argument
    ASSERT_TRUE (reifier.reifyNextArgument(IntType).isOk());

    // result type should be Bool
    auto reifyReturnResult = reifier.reifyResult(BoolType);
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (BoolType, resultType);

    // verify 1 argument has been reified
    ASSERT_EQ (1, reifier.numArguments());
}