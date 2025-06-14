#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

#include "base_typing_fixture.h"
#include "test_callable.h"
#include "lyric_assembler/object_root.h"
#include "lyric_typing/typing_result.h"

class CallsiteReifierErrorHandling : public BaseTypingFixture {};

TEST_F(CallsiteReifierErrorHandling, ParametricParameterReificationFailsGivenNonParametricArgument)
{
    auto *fundamentalCache = m_objectState->fundamentalCache();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto FloatType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);

    lyric_object::TemplateParameter tp0;
    tp0.index = 0;
    tp0.name = "T";
    tp0.typeDef = AnyType;
    tp0.variance = lyric_object::VarianceType::Invariant;
    tp0.bound = lyric_object::BoundType::None;

    std::vector<lyric_object::TemplateParameter> templateParameters{tp0};

    auto *rootBlock = m_objectRoot->rootBlock();
    auto *ObjectClass = rootBlock->resolveClass(ObjectType).orElseThrow();
    auto *FooClass = rootBlock->declareClass(
        "Foo", ObjectClass, lyric_object::AccessType::Public, templateParameters).orElseThrow();

    lyric_assembler::Parameter p0;
    p0.index = 0;
    p0.typeDef = FooClass->getTypeDef();
    p0.placement = lyric_object::PlacementType::List;

    lyric_assembler::CallableInvoker invoker;
    auto callable = std::unique_ptr<TestCallable>(new TestCallable({p0}, {}, {}));
    ASSERT_THAT (invoker.initialize(std::move(callable)), tempo_test::IsOk());

    // simulate the function f(p0: Foo[T]): Int
    lyric_typing::CallsiteReifier reifier(m_typeSystem.get());
    ASSERT_THAT (reifier.initialize(invoker), tempo_test::IsOk());

    // apply Int argument
    ASSERT_THAT (reifier.reifyNextArgument(IntType), tempo_test::IsCondition(lyric_typing::TypingCondition::kIncompatibleType));
}
