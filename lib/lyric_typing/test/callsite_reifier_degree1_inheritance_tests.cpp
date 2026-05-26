#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <tempo_test/tempo_test.h>

#include "base_typing_fixture.h"

class CallsiteReifierDegree1Inheritance : public BaseTypingFixture {};

TEST_F(CallsiteReifierDegree1Inheritance, InvokeOverrideMethodOnDirectSubclassOfBaseClass)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    // define base class Base
    lyric_assembler::ClassSymbol *BaseClass;
    TU_ASSIGN_OR_RAISE (BaseClass, rootBlock->declareClass(
        "Base", false, {}, false, lyric_object::DeriveType::Any, true));
    TU_RAISE_IF_NOT_OK (BaseClass->finalizeClass(ObjectType));
    lyric_assembler::CallSymbol *BaseMethodCall;
    TU_ASSIGN_OR_RAISE (BaseMethodCall, BaseClass->declareMethod("Method", false));
    lyric_assembler::ParameterPack baseParameters;
    baseParameters.listParameters = {
        {0, "p0", {}, IntType, lyric_object::PlacementType::List, false},
    };
    TU_RAISE_IF_STATUS (BaseMethodCall->defineCall(baseParameters, IntType));
    TU_RAISE_IF_STATUS (BaseMethodCall->finalizeCall());

    // define subclass Sub from Base
    lyric_assembler::ClassSymbol *SubClass;
    TU_ASSIGN_OR_RAISE (SubClass, rootBlock->declareClass(
        "Sub", false, {}, false, lyric_object::DeriveType::Any, true));
    TU_RAISE_IF_NOT_OK (SubClass->finalizeClass(BaseClass->getTypeDef()));
    lyric_assembler::CallSymbol *SubMethodCall;
    TU_ASSIGN_OR_RAISE (SubMethodCall, SubClass->declareMethod("Method", false));
    lyric_assembler::ParameterPack subParameters;
    subParameters.listParameters = {
            {0, "p0", {}, IntType, lyric_object::PlacementType::List, false},
        };
    TU_RAISE_IF_STATUS (SubMethodCall->defineCall(subParameters, IntType));
    TU_RAISE_IF_STATUS (SubMethodCall->finalizeCall());

    // receiver type is Sub
    auto receiverType = SubClass->getTypeDef();

    // prepare method
    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    TU_RAISE_IF_NOT_OK (SubClass->prepareMethod("Method", receiverType, callable));

    // initialize reifier
    lyric_typing::CallsiteReifier reifier(typeSystem.get());
    ASSERT_THAT (reifier.initialize(receiverType, callable.get()), tempo_test::IsOk());

    // reify p0
    ASSERT_THAT (reifier.reifyNextArgument(IntType), tempo_test::IsOk());

    // reify result
    auto reifyReturnResult = reifier.reifyResult(IntType);
    ASSERT_THAT (reifyReturnResult, tempo_test::IsResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);
}

TEST_F(CallsiteReifierDegree1Inheritance, InvokeBaseMethodOnDirectSubclassOfBaseClass)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    // define base class Base
    lyric_assembler::ClassSymbol *BaseClass;
    TU_ASSIGN_OR_RAISE (BaseClass, rootBlock->declareClass(
        "Base", false, {}, false, lyric_object::DeriveType::Any, true));
    TU_RAISE_IF_NOT_OK (BaseClass->finalizeClass(ObjectType));
    lyric_assembler::CallSymbol *BaseMethodCall;
    TU_ASSIGN_OR_RAISE (BaseMethodCall, BaseClass->declareMethod("Method", false));
    lyric_assembler::ParameterPack baseParameters;
    baseParameters.listParameters = {
        {0, "p0", {}, IntType, lyric_object::PlacementType::List, false},
    };
    TU_RAISE_IF_STATUS (BaseMethodCall->defineCall(baseParameters, IntType));
    TU_RAISE_IF_STATUS (BaseMethodCall->finalizeCall());

    // define subclass Sub from Base
    lyric_assembler::ClassSymbol *SubClass;
    TU_ASSIGN_OR_RAISE (SubClass, rootBlock->declareClass(
        "Sub", false, {}, false, lyric_object::DeriveType::Any, true));
    TU_RAISE_IF_NOT_OK (SubClass->finalizeClass(BaseClass->getTypeDef()));

    // receiver type is Sub
    auto receiverType = SubClass->getTypeDef();

    // prepare method
    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    TU_RAISE_IF_NOT_OK (SubClass->prepareMethod("Method", receiverType, callable));

    // initialize reifier
    lyric_typing::CallsiteReifier reifier(typeSystem.get());
    ASSERT_THAT (reifier.initialize(receiverType, callable.get()), tempo_test::IsOk());

    // reify p0
    ASSERT_THAT (reifier.reifyNextArgument(IntType), tempo_test::IsOk());

    // reify result
    auto reifyReturnResult = reifier.reifyResult(IntType);
    ASSERT_THAT (reifyReturnResult, tempo_test::IsResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);
}

TEST_F(CallsiteReifierDegree1Inheritance, InvokeBaseMethodOnParameterizedDirectSubclassOfGenericBaseClass)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);

    // define base class Base[T]
    lyric_assembler::ClassSymbol *BaseClass;
    TU_ASSIGN_OR_RAISE (BaseClass, rootBlock->declareClass("Base", false,
        {{"T", 0, AnyType, lyric_object::VarianceType::Invariant, lyric_object::BoundType::Extends}},
        false, lyric_object::DeriveType::Any, true));
    TU_RAISE_IF_NOT_OK (BaseClass->finalizeClass(ObjectType));
    auto *BaseTemplate = BaseClass->classTemplate();
    auto BaseTType = BaseTemplate->getPlaceholder(0);
    lyric_assembler::CallSymbol *BaseMethodCall;
    TU_ASSIGN_OR_RAISE (BaseMethodCall, BaseClass->declareMethod("Method", false));
    lyric_assembler::ParameterPack baseParameters;
    baseParameters.listParameters = {
        {0, "p0", {}, BaseTType, lyric_object::PlacementType::List, false},
    };
    TU_RAISE_IF_STATUS (BaseMethodCall->defineCall(baseParameters, BaseTType));
    TU_RAISE_IF_STATUS (BaseMethodCall->finalizeCall());

    // define subclass Sub from Base[Int]
    lyric_assembler::ClassSymbol *SubClass;
    TU_ASSIGN_OR_RAISE (SubClass, rootBlock->declareClass(
        "Sub", false, {}, false, lyric_object::DeriveType::Any, true));
    lyric_common::TypeDef superclassType;
    TU_ASSIGN_OR_RAISE (superclassType, lyric_common::TypeDef::forConcrete(BaseClass->getSymbolUrl(), {IntType}));
    TU_RAISE_IF_NOT_OK (SubClass->finalizeClass(superclassType));

    // receiver type is Sub
    auto receiverType = SubClass->getTypeDef();

    // prepare method
    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    TU_RAISE_IF_NOT_OK (SubClass->prepareMethod("Method", receiverType, callable));

    // initialize reifier
    lyric_typing::CallsiteReifier reifier(typeSystem.get());
    ASSERT_THAT (reifier.initialize(receiverType, callable.get()), tempo_test::IsOk());

    // reify p0
    ASSERT_THAT (reifier.reifyNextArgument(IntType), tempo_test::IsOk());

    // reify result
    auto reifyReturnResult = reifier.reifyResult(IntType);
    ASSERT_THAT (reifyReturnResult, tempo_test::IsResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);
}

TEST_F(CallsiteReifierDegree1Inheritance, InvokeBaseMethodOnGenericDirectSubclassOfGenericBaseClass)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);

    // define base class Base[T]
    lyric_assembler::ClassSymbol *BaseClass;
    TU_ASSIGN_OR_RAISE (BaseClass, rootBlock->declareClass("Base", false,
        {{"T", 0, AnyType, lyric_object::VarianceType::Invariant, lyric_object::BoundType::Extends}},
        false, lyric_object::DeriveType::Any, true));
    TU_RAISE_IF_NOT_OK (BaseClass->finalizeClass(ObjectType));
    auto *BaseTemplate = BaseClass->classTemplate();
    auto BaseTType = BaseTemplate->getPlaceholder(0);
    lyric_assembler::CallSymbol *BaseMethodCall;
    TU_ASSIGN_OR_RAISE (BaseMethodCall, BaseClass->declareMethod("Method", false));
    lyric_assembler::ParameterPack baseParameters;
    baseParameters.listParameters = {
        {0, "p0", {}, BaseTType, lyric_object::PlacementType::List, false},
    };
    TU_RAISE_IF_STATUS (BaseMethodCall->defineCall(baseParameters, BaseTType));
    TU_RAISE_IF_STATUS (BaseMethodCall->finalizeCall());

    // define subclass Sub[U] from Base[U]
    lyric_assembler::ClassSymbol *SubClass;
    TU_ASSIGN_OR_RAISE (SubClass, rootBlock->declareClass("Sub", false,
        {{"U", 0, AnyType, lyric_object::VarianceType::Invariant, lyric_object::BoundType::Extends}},
        false, lyric_object::DeriveType::Any, true));
    auto *SubTemplate = SubClass->classTemplate();
    auto SubUType = SubTemplate->getPlaceholder(0);
    lyric_common::TypeDef superclassType;
    TU_ASSIGN_OR_RAISE (superclassType, lyric_common::TypeDef::forConcrete(BaseClass->getSymbolUrl(), {SubUType}));
    TU_RAISE_IF_NOT_OK (SubClass->finalizeClass(superclassType));

    // receiver type is Sub[Int]
    lyric_common::TypeDef receiverType;
    TU_ASSIGN_OR_RAISE (receiverType, lyric_common::TypeDef::forConcrete(SubClass->getSymbolUrl(), {IntType}));

    // prepare method
    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    TU_RAISE_IF_NOT_OK (SubClass->prepareMethod("Method", receiverType, callable));

    // initialize reifier
    lyric_typing::CallsiteReifier reifier(typeSystem.get());
    ASSERT_THAT (reifier.initialize(receiverType, callable.get()), tempo_test::IsOk());

    // reify p0
    ASSERT_THAT (reifier.reifyNextArgument(IntType), tempo_test::IsOk());

    // reify result
    auto reifyReturnResult = reifier.reifyResult(IntType);
    ASSERT_THAT (reifyReturnResult, tempo_test::IsResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);
}

TEST_F(CallsiteReifierDegree1Inheritance, InvokeBaseMethodOnGenericDirectSubclassOfBaseClass)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);

    // define base class Base
    lyric_assembler::ClassSymbol *BaseClass;
    TU_ASSIGN_OR_RAISE (BaseClass, rootBlock->declareClass("Base", false, {}, false, lyric_object::DeriveType::Any, true));
    TU_RAISE_IF_NOT_OK (BaseClass->finalizeClass(ObjectType));
    lyric_assembler::CallSymbol *BaseMethodCall;
    TU_ASSIGN_OR_RAISE (BaseMethodCall, BaseClass->declareMethod("Method", false));
    lyric_assembler::ParameterPack baseParameters;
    baseParameters.listParameters = {
        {0, "p0", {}, IntType, lyric_object::PlacementType::List, false},
    };
    TU_RAISE_IF_STATUS (BaseMethodCall->defineCall(baseParameters, IntType));
    TU_RAISE_IF_STATUS (BaseMethodCall->finalizeCall());

    // define subclass Sub[T] from Base
    lyric_assembler::ClassSymbol *SubClass;
    TU_ASSIGN_OR_RAISE (SubClass, rootBlock->declareClass("Sub", false,
        {{"T", 0, AnyType, lyric_object::VarianceType::Invariant, lyric_object::BoundType::Extends}},
        false, lyric_object::DeriveType::Any, true));
    auto superclassType = BaseClass->getTypeDef();
    TU_RAISE_IF_NOT_OK (SubClass->finalizeClass(superclassType));

    // receiver type is Sub[Object]
    lyric_common::TypeDef receiverType;
    TU_ASSIGN_OR_RAISE (receiverType, lyric_common::TypeDef::forConcrete(SubClass->getSymbolUrl(), {ObjectType}));

    // prepare method
    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    TU_RAISE_IF_NOT_OK (SubClass->prepareMethod("Method", receiverType, callable));

    // initialize reifier
    lyric_typing::CallsiteReifier reifier(typeSystem.get());
    ASSERT_THAT (reifier.initialize(receiverType, callable.get()), tempo_test::IsOk());

    // reify p0
    ASSERT_THAT (reifier.reifyNextArgument(IntType), tempo_test::IsOk());

    // reify result
    auto reifyReturnResult = reifier.reifyResult(IntType);
    ASSERT_THAT (reifyReturnResult, tempo_test::IsResult());
    auto resultType = reifyReturnResult.getResult();
    ASSERT_EQ (IntType, resultType);
}
