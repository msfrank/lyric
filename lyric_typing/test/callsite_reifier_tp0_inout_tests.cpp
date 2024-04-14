#include <gtest/gtest.h>

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_fixture.h"

class CallsiteReifierTP0InOut : public BaseFixture {};

TEST_F(CallsiteReifierTP0InOut, UnaryFunctionGivenT_P0takesT_returnsT)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    auto *typeCache = m_assemblyState->typeCache();

    lyric_common::SymbolUrl entryUrl(lyric_common::SymbolPath({"$entry"}));
    auto proc = std::make_unique<lyric_assembler::ProcHandle>(entryUrl, m_assemblyState.get());

    lyric_object::TemplateParameter tp0;
    tp0.index = 0;
    tp0.name = "T";
    tp0.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    tp0.bound = lyric_object::BoundType::None;
    tp0.variance = lyric_object::VarianceType::Invariant;

    lyric_common::SymbolUrl templateUrl(lyric_common::SymbolPath({"sym"}));
    ASSERT_TRUE (typeCache->makeTemplate(templateUrl, {tp0}, proc->procBlock()).isOk());
    lyric_assembler::TemplateHandle *templateHandle = typeCache->getTemplate(templateUrl);
    ASSERT_TRUE (templateHandle != nullptr);

    lyric_object::Parameter p0;
    p0.index = 0;
    p0.typeDef = templateHandle->getPlaceholder(0);
    p0.placement = lyric_object::PlacementType::List;

    // simulate the function f[T](p0: T): T
    lyric_typing::CallsiteReifier reifier({p0}, {}, templateUrl, templateHandle->getTemplateParameters(),
        {}, m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize().isOk());
    ASSERT_TRUE (reifier.isValid());

    // apply Int argument
    ASSERT_TRUE (reifier.reifyNextArgument(IntType).isOk());

    // result type should be Int
    auto reifyReturnResult = reifier.reifyResult(templateHandle->getPlaceholder(0));
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);

    // verify 1 argument has been reified
    ASSERT_EQ (1, reifier.numArguments());
}

TEST_F(CallsiteReifierTP0InOut, UnaryFunctionGivenT_P0takesCollectionOfT_returnsT)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    auto *typeCache = m_assemblyState->typeCache();

    lyric_common::SymbolUrl entryUrl(lyric_common::SymbolPath({"$entry"}));
    auto proc = std::make_unique<lyric_assembler::ProcHandle>(entryUrl, m_assemblyState.get());
    auto *block = proc->procBlock();

    lyric_object::TemplateParameter tp0;
    tp0.index = 0;
    tp0.name = "T";
    tp0.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    tp0.bound = lyric_object::BoundType::None;
    tp0.variance = lyric_object::VarianceType::Invariant;

    lyric_common::SymbolUrl templateUrl(lyric_common::SymbolPath({"sym"}));
    ASSERT_TRUE (typeCache->makeTemplate(templateUrl, {tp0}, proc->procBlock()).isOk());
    lyric_assembler::TemplateHandle *templateHandle = typeCache->getTemplate(templateUrl);
    ASSERT_TRUE (templateHandle != nullptr);

    auto *symbolCache = m_assemblyState->symbolCache();
    auto *ObjectClass = lyric_assembler::cast_symbol_to_class(
        symbolCache->getSymbol(
            fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Object)));

    lyric_common::SymbolUrl collectionUrl;
    TU_ASSIGN_OR_RAISE (collectionUrl,
        block->declareClass("Collection", ObjectClass, lyric_object::AccessType::Public, {
            {"T", 0, AnyType, lyric_object::VarianceType::Invariant, lyric_object::BoundType::None},
        }));
    auto CollectionOfTType = lyric_common::TypeDef::forConcrete(collectionUrl, {templateHandle->getPlaceholder(0)});

    lyric_object::Parameter p0;
    p0.index = 0;
    p0.typeDef = CollectionOfTType;
    p0.placement = lyric_object::PlacementType::List;

    // simulate the function f[T](p0: Collection[T]): T
    lyric_typing::CallsiteReifier reifier({p0}, {}, templateUrl, templateHandle->getTemplateParameters(),
                                          {}, m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize().isOk());
    ASSERT_TRUE (reifier.isValid());

    // apply Collection[Int] argument
    auto CollectionOfIntType = lyric_common::TypeDef::forConcrete(collectionUrl, {IntType});
    ASSERT_TRUE (reifier.reifyNextArgument(CollectionOfIntType).isOk());

    // result type should be Int
    auto reifyReturnResult = reifier.reifyResult(templateHandle->getPlaceholder(0));
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);

    // verify 1 argument has been reified
    ASSERT_EQ (1, reifier.numArguments());
}

TEST_F(CallsiteReifierTP0InOut, UnaryFunctionGivenT_P0takesUnionOfTandNil_returnsT)
{
    auto *fundamentalCache = m_assemblyState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto NilType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil);

    auto *typeCache = m_assemblyState->typeCache();

    lyric_common::SymbolUrl entryUrl(lyric_common::SymbolPath({"$entry"}));
    auto proc = std::make_unique<lyric_assembler::ProcHandle>(entryUrl, m_assemblyState.get());

    lyric_object::TemplateParameter tp0;
    tp0.index = 0;
    tp0.name = "T";
    tp0.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    tp0.bound = lyric_object::BoundType::None;
    tp0.variance = lyric_object::VarianceType::Invariant;

    lyric_common::SymbolUrl templateUrl(lyric_common::SymbolPath({"sym"}));
    ASSERT_TRUE (typeCache->makeTemplate(templateUrl, {tp0}, proc->procBlock()).isOk());
    lyric_assembler::TemplateHandle *templateHandle = typeCache->getTemplate(templateUrl);
    ASSERT_TRUE (templateHandle != nullptr);

    lyric_object::Parameter p0;
    p0.index = 0;
    p0.typeDef = lyric_common::TypeDef::forUnion({templateHandle->getPlaceholder(0), NilType});
    p0.placement = lyric_object::PlacementType::List;

    // simulate the function f[T](p0: T | Nil): T
    lyric_typing::CallsiteReifier reifier({p0}, {}, templateUrl, templateHandle->getTemplateParameters(),
        {}, m_typeSystem.get());
    ASSERT_TRUE (reifier.initialize().isOk());
    ASSERT_TRUE (reifier.isValid());

    // apply Int argument
    ASSERT_TRUE (reifier.reifyNextArgument(IntType).isOk());

    // result type should be Int
    auto reifyReturnResult = reifier.reifyResult(templateHandle->getPlaceholder(0));
    ASSERT_TRUE (reifyReturnResult.isResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);

    // verify 1 argument has been reified
    ASSERT_EQ (1, reifier.numArguments());
}
