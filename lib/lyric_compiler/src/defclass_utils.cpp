
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/defclass_utils.h>
#include <lyric_parser/ast_attrs.h>

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_compiler::declare_class_default_init(
    lyric_assembler::ClassSymbol *classSymbol,
    const std::string &allocatorTrap)
{
    TU_ASSERT (classSymbol != nullptr);

    // declare the constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, classSymbol->declareCtor(
        lyric_object::kCtorSpecialSymbol, /* isHidden= */ false, allocatorTrap));

    // define 0-arity constructor
    TU_RETURN_IF_STATUS(ctorSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));

    return ctorSymbol;
}

tempo_utils::Status
lyric_compiler::define_class_default_init(
    const DefClass *defclass,
    const std::string &allocatorTrap,
    lyric_assembler::SymbolCache *symbolCache,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (defclass != nullptr);

    auto *classSymbol = defclass->classSymbol;
    TU_ASSERT (classSymbol != nullptr);
    auto *ctorSymbol = defclass->defaultCtor;
    TU_ASSERT (ctorSymbol != nullptr);

    // verify that super ctor takes no arguments
    auto *superClass = classSymbol->superClass();
    auto superCtorUrl = superClass->getCtor(lyric_object::kCtorSpecialSymbol);
    lyric_assembler::AbstractSymbol *superCtorSymbol;
    TU_ASSIGN_OR_RETURN (superCtorSymbol, symbolCache->getOrImportSymbol(superCtorUrl));
    auto *superCtorCall = cast_symbol_to_call(superCtorSymbol);
    if (superCtorCall->numParameters() > 0)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "super class {} is not default-constructable", superCtorUrl.toString());

    auto *procHandle = ctorSymbol->callProc();
    auto *ctorBlock = procHandle->procBlock();
    auto *fragment = procHandle->procFragment();

    // find the superclass ctor
    lyric_assembler::ConstructableInvoker superCtor;
    TU_RETURN_IF_NOT_OK (superClass->prepareCtor(lyric_object::kCtorSpecialSymbol, superCtor));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(superCtor));

    // load the uninitialized class onto the top of the stack
    TU_RETURN_IF_NOT_OK (fragment->loadThis());

    // call super ctor
    TU_RETURN_IF_STATUS (superCtor.invoke(ctorBlock, reifier, fragment, /* flags= */ 0));

    // default-initialize all members
    for (auto it = defclass->classSymbol->membersBegin(); it != defclass->classSymbol->membersEnd(); it++) {
        auto &memberName = it->first;
        auto &fieldRef = it->second;

        lyric_assembler::AbstractSymbol *symbol;

        // resolve the member binding
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(fieldRef.symbolUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid class field {}", fieldRef.symbolUrl.toString());

        auto *fieldSymbol = cast_symbol_to_field(symbol);

        if (!fieldSymbol->hasInitializer())
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "missing initializer for field {}", memberName);

        auto fieldInitializerUrl = fieldSymbol->getInitializer();

        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(fieldInitializerUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid field initializer {}", fieldInitializerUrl.toString());

        auto *initializerCall = cast_symbol_to_call(symbol);

        // load $this onto the top of the stack
        TU_RETURN_IF_NOT_OK (fragment->loadThis());

        // invoke initializer to place default value onto the top of the stack
        auto callable = std::make_unique<lyric_assembler::FunctionCallable>(
            initializerCall, /* isInlined= */ false);
        lyric_assembler::CallableInvoker invoker;
        TU_RETURN_IF_NOT_OK (invoker.initialize(std::move(callable)));
        lyric_typing::CallsiteReifier initializerReifier(typeSystem);
        TU_RETURN_IF_NOT_OK (initializerReifier.initialize(invoker));
        TU_RETURN_IF_STATUS (invoker.invoke(ctorBlock, initializerReifier, fragment));

        // store default value in class field
        TU_RETURN_IF_NOT_OK (fragment->storeRef(fieldRef, /* initialStore= */ true));

        // mark member as initialized
        TU_RETURN_IF_NOT_OK (defclass->classSymbol->setMemberInitialized(memberName));
    }

    TU_LOG_V << "defined default ctor " << ctorSymbol->getSymbolUrl()
        << " for " << defclass->classSymbol->getSymbolUrl();

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    if (!defclass->classSymbol->isCompletelyInitialized())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "class {} is not completely initialized",
            defclass->classSymbol->getSymbolUrl().toString());

    return {};
}

tempo_utils::Result<lyric_compiler::Constructor>
lyric_compiler::declare_class_init(
    const lyric_parser::ArchetypeNode *initNode,
    lyric_assembler::ClassSymbol *classSymbol,
    const std::string &allocatorTrap,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (initNode != nullptr);
    TU_ASSERT (classSymbol != nullptr);
    auto *classBlock = classSymbol->classBlock();

    std::string identifier;
    if (initNode->hasAttr(lyric_parser::kLyricAstIdentifier)) {
        TU_RETURN_IF_NOT_OK (initNode->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    } else {
        identifier = lyric_object::kCtorSpecialSymbol;
    }

    bool isHidden;
    TU_RETURN_IF_NOT_OK (initNode->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    // declare the constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, classSymbol->declareCtor(identifier, isHidden, allocatorTrap));

    lyric_typing::PackSpec packSpec;
    lyric_assembler::ParameterPack parameterPack;

    // compile the parameter list if specified, otherwise default to an empty list
    if (initNode != nullptr) {
        auto *packNode = initNode->getChild(0);
        TU_ASSERT (packNode != nullptr);
        TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(classBlock, packNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(classBlock, packSpec));
    }

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, ctorSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));

    Constructor constructor;
    constructor.thisSymbol = classSymbol;
    constructor.superSymbol = classSymbol->superClass();
    constructor.callSymbol = ctorSymbol;
    constructor.procHandle = procHandle;
    return constructor;
}

tempo_utils::Result<lyric_compiler::Member>
lyric_compiler::declare_class_member(
    const lyric_parser::ArchetypeNode *node,
    bool isVariable,
    lyric_assembler::ClassSymbol *classSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (classSymbol != nullptr);
    auto *classBlock = classSymbol->classBlock();

    // get member name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get member type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec memberSpec;
    TU_ASSIGN_OR_RETURN (memberSpec, typeSystem->parseAssignable(classBlock, typeNode->getArchetypeNode()));
    lyric_common::TypeDef memberType;
    TU_ASSIGN_OR_RETURN (memberType, typeSystem->resolveAssignable(classBlock, memberSpec));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    Member member;

    TU_ASSIGN_OR_RETURN (member.fieldSymbol, classSymbol->declareMember(
        identifier, memberType, isVariable, isHidden));

    TU_LOG_V << "declared member " << identifier << " for " << classSymbol->getSymbolUrl();

    // define the initializer if specified
    if (node->numChildren() > 0) {
        TU_ASSIGN_OR_RETURN (member.initializerHandle, member.fieldSymbol->defineInitializer());
    }

    return member;
}

tempo_utils::Result<lyric_compiler::Method>
lyric_compiler::declare_class_method(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::ClassSymbol *classSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (classSymbol != nullptr);
    auto *classBlock = classSymbol->classBlock();

    // get method name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // determine the access level
    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    // determine if final
    bool noOverride;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstNoOverride, noOverride));
    auto dispatch = noOverride? lyric_assembler::DispatchType::Final : lyric_assembler::DispatchType::Virtual;

    // parse the return type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(classBlock, typeNode->getArchetypeNode()));

    // parse the parameter list
    auto *packNode = node->getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(classBlock, packNode->getArchetypeNode()));

    // if method is generic, then parse the template parameter list
    lyric_typing::TemplateSpec templateSpec;
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(classBlock, genericNode->getArchetypeNode()));
    }

    Method method;
    method.dispatch = dispatch;

    // declare the method
    TU_ASSIGN_OR_RETURN (method.callSymbol, classSymbol->declareMethod(
        identifier, isHidden, dispatch, templateSpec.templateParameters));

    TU_LOG_V << "declared method " << identifier << " for " << classSymbol->getSymbolUrl();

    auto *resolver = method.callSymbol->callResolver();

    // resolve the parameter pack
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(resolver, packSpec));

    // resolve the return type
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(resolver, returnSpec));

    // define the method
    TU_ASSIGN_OR_RETURN (method.procHandle, method.callSymbol->defineCall(parameterPack, returnType));

    return method;
}

tempo_utils::Result<lyric_compiler::Method>
lyric_compiler::declare_class_abstract_method(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::ClassSymbol *classSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (classSymbol != nullptr);
    auto *classBlock = classSymbol->classBlock();

    // get method name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // determine the access level
    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    // parse the return type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(classBlock, typeNode->getArchetypeNode()));

    // parse the parameter list
    auto *packNode = node->getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(classBlock, packNode->getArchetypeNode()));

    // if method is generic, then parse the template parameter list
    lyric_typing::TemplateSpec templateSpec;
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(classBlock, genericNode->getArchetypeNode()));
    }

    Method method;
    method.dispatch = lyric_assembler::DispatchType::Abstract;
    method.procHandle = nullptr;

    // declare the method
    TU_ASSIGN_OR_RETURN (method.callSymbol, classSymbol->declareMethod(
        identifier, isHidden, lyric_assembler::DispatchType::Abstract, templateSpec.templateParameters));

    TU_LOG_V << "declared method " << identifier << " for " << classSymbol->getSymbolUrl();

    auto *resolver = method.callSymbol->callResolver();

    // resolve the parameter pack
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(resolver, packSpec));

    // resolve the return type
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(resolver, returnSpec));

    // define the method
    TU_RETURN_IF_NOT_OK (method.callSymbol->defineAbstract(parameterPack, returnType));

    return method;
}

tempo_utils::Result<lyric_compiler::Impl>
lyric_compiler::declare_class_impl(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::ClassSymbol *classSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (classSymbol != nullptr);
    auto *classBlock = classSymbol->classBlock();

    // parse the impl type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(classBlock, typeNode->getArchetypeNode()));

    // resolve the impl type
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(classBlock, implSpec));

    Impl impl;

    // declare the impl
    TU_ASSIGN_OR_RETURN (impl.implHandle, classSymbol->declareImpl(implType));

    TU_LOG_V << "declared impl " << implType << " for " << classSymbol->getSymbolUrl();

    return impl;
}
