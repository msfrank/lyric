#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_typing/overload_reifier.h>
#include <tempo_test/tempo_test.h>

#include "base_typing_fixture.h"

class OverloadReifier : public BaseTypingFixture {};

TEST_F(OverloadReifier, NullaryFunctionReturningConcreteType)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    auto IdeaType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Idea);

    lyric_assembler::ConceptSymbol *FooConcept;
    TU_ASSIGN_OR_RAISE (FooConcept, rootBlock->declareConcept("Foo", false, {}));
    TU_RAISE_IF_NOT_OK (FooConcept->finalizeConcept(IdeaType));

    lyric_assembler::ActionSymbol *OverloadAction;
    TU_ASSIGN_OR_RAISE (OverloadAction, FooConcept->declareAction("Overload", false));

    // declare extension Overload(): Bool
    lyric_assembler::ParameterPack actionParameters;
    TU_RAISE_IF_NOT_OK (OverloadAction->defineAction(actionParameters, BoolType));

    lyric_typing::OverloadReifier reifier(typeSystem.get());
    ASSERT_THAT (reifier.initialize(OverloadAction), tempo_test::IsOk());

    auto reifyParametersResult = reifier.reifyParameters({});
    ASSERT_THAT (reifyParametersResult, tempo_test::IsResult());
    auto reifiedParameters = reifyParametersResult.getResult();

    // reified parameter pack should be empty
    ASSERT_THAT (reifiedParameters.listParameters, ::testing::IsEmpty());
    ASSERT_THAT (reifiedParameters.namedParameters, ::testing::IsEmpty());
    ASSERT_TRUE (reifiedParameters.restParameter.isEmpty());

    // result type should be Bool
    auto reifyReturnResult = reifier.reifyResult(BoolType);
    ASSERT_THAT (reifyReturnResult, tempo_test::IsResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (BoolType, resultType);
}

TEST_F(OverloadReifier, UnaryFunctionReceivingConcreteTypeReturningConcreteType)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    auto IdeaType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Idea);

    lyric_assembler::ConceptSymbol *FooConcept;
    TU_ASSIGN_OR_RAISE (FooConcept, rootBlock->declareConcept("Foo", false, {}));
    TU_RAISE_IF_NOT_OK (FooConcept->finalizeConcept(IdeaType));

    lyric_assembler::ActionSymbol *OverloadAction;
    TU_ASSIGN_OR_RAISE (OverloadAction, FooConcept->declareAction("Overload", false));

    // declare extension Overload(Bool): Bool
    lyric_assembler::Parameter base0{0, "x","", BoolType, lyric_object::PlacementType::List, false};
    lyric_assembler::ParameterPack actionParameters;
    actionParameters.listParameters.push_back(base0);
    TU_RAISE_IF_NOT_OK (OverloadAction->defineAction(actionParameters, BoolType));

    lyric_typing::OverloadReifier reifier(typeSystem.get());
    ASSERT_THAT (reifier.initialize(OverloadAction), tempo_test::IsOk());

    lyric_assembler::Parameter overload0{0, "x","", BoolType, lyric_object::PlacementType::List, false};
    lyric_assembler::ParameterPack overloadParameters;
    overloadParameters.listParameters.push_back(overload0);
    auto reifyParametersResult = reifier.reifyParameters(overloadParameters);
    ASSERT_THAT (reifyParametersResult, tempo_test::IsResult());
    auto reifiedParameters = reifyParametersResult.getResult();

    // reified parameter pack should contain 1 list parameter
    ASSERT_THAT (reifiedParameters.listParameters, ::testing::SizeIs(1));
    auto reified0 = reifiedParameters.listParameters.at(0);
    ASSERT_EQ (reified0.typeDef, overload0.typeDef);
    ASSERT_THAT (reifiedParameters.namedParameters, ::testing::IsEmpty());
    ASSERT_TRUE (reifiedParameters.restParameter.isEmpty());

    // result type should be Bool
    auto reifyReturnResult = reifier.reifyResult(BoolType);
    ASSERT_THAT (reifyReturnResult, tempo_test::IsResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (BoolType, resultType);
}

TEST_F(OverloadReifier, UnaryFunctionReceivingPlaceholderTypeReturningPlaceholderType)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    auto IdeaType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Idea);

    std::vector<lyric_object::TemplateParameter> templateParameters;
    lyric_object::TemplateParameter tp0{"T", 0, AnyType, lyric_object::VarianceType::Invariant, lyric_object::BoundType::Extends, false};
    templateParameters.push_back(tp0);

    lyric_assembler::ConceptSymbol *FooConcept;
    TU_ASSIGN_OR_RAISE (FooConcept, rootBlock->declareConcept("Foo", false, templateParameters));
    TU_RAISE_IF_NOT_OK (FooConcept->finalizeConcept(IdeaType));

    auto *templateHandle = FooConcept->conceptTemplate();

    lyric_assembler::ActionSymbol *OverloadAction;
    TU_ASSIGN_OR_RAISE (OverloadAction, FooConcept->declareAction("Overload", false));

    // declare extension Overload(Bool): Bool
    lyric_assembler::Parameter base0{0, "x","", templateHandle->getPlaceholder(0), lyric_object::PlacementType::List, false};
    lyric_assembler::ParameterPack actionParameters;
    actionParameters.listParameters.push_back(base0);
    TU_RAISE_IF_NOT_OK (OverloadAction->defineAction(actionParameters, templateHandle->getPlaceholder(0)));

    lyric_typing::OverloadReifier reifier(typeSystem.get());
    ASSERT_THAT (reifier.initialize(OverloadAction), tempo_test::IsOk());

    lyric_assembler::Parameter overload0{0, "x","", BoolType, lyric_object::PlacementType::List, false};
    lyric_assembler::ParameterPack overloadParameters;
    overloadParameters.listParameters.push_back(overload0);
    auto reifyParametersResult = reifier.reifyParameters(overloadParameters);
    ASSERT_THAT (reifyParametersResult, tempo_test::IsResult());
    auto reifiedParameters = reifyParametersResult.getResult();

    // reified parameter pack should contain 1 list parameter
    ASSERT_THAT (reifiedParameters.listParameters, ::testing::SizeIs(1));
    auto reified0 = reifiedParameters.listParameters.at(0);
    ASSERT_EQ (reified0.typeDef, overload0.typeDef);
    ASSERT_THAT (reifiedParameters.namedParameters, ::testing::IsEmpty());
    ASSERT_TRUE (reifiedParameters.restParameter.isEmpty());

    // result type should be Bool
    auto reifyReturnResult = reifier.reifyResult(BoolType);
    ASSERT_THAT (reifyReturnResult, tempo_test::IsResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (BoolType, resultType);
}
