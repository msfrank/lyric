
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/defenum_utils.h>
#include <lyric_parser/ast_attrs.h>

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_compiler::declare_enum_init(
    const lyric_parser::ArchetypeNode *initNode,
    lyric_assembler::EnumSymbol *enumSymbol,
    const std::string &allocatorTrap,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (initNode != nullptr);
    TU_ASSERT (enumSymbol != nullptr);
    auto *enumBlock = enumSymbol->enumBlock();

    // declare the constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, enumSymbol->declareCtor(lyric_object::AccessType::Public, allocatorTrap));

    lyric_typing::PackSpec packSpec;
    lyric_assembler::ParameterPack parameterPack;

    // compile the parameter list if specified, otherwise default to an empty list
    if (initNode != nullptr) {
        auto *packNode = initNode->getChild(0);
        TU_ASSERT (packNode != nullptr);
        TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(enumBlock, packNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(enumBlock, packSpec));
    }

    TU_RETURN_IF_STATUS(ctorSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));

    return ctorSymbol;
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_compiler::declare_enum_default_init(
    const DefEnum *defenum,
    lyric_assembler::EnumSymbol *enumSymbol,
    const std::string &allocatorTrap,
    lyric_assembler::SymbolCache *symbolCache,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (defenum != nullptr);
    TU_ASSERT (enumSymbol != nullptr);

    // declare the constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, enumSymbol->declareCtor(lyric_object::AccessType::Public, allocatorTrap));

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, ctorSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));

    auto *ctorBlock = procHandle->procBlock();
    auto *procBuilder = procHandle->procCode();
    auto *fragment = procBuilder->rootFragment();

    // find the superenum ctor
    lyric_assembler::ConstructableInvoker superCtor;
    TU_RETURN_IF_NOT_OK (enumSymbol->superEnum()->prepareCtor(superCtor));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(superCtor));

    // load the uninitialized enum onto the top of the stack
    TU_RETURN_IF_NOT_OK (fragment->loadThis());

    // call super ctor
    TU_RETURN_IF_STATUS (superCtor.invoke(ctorBlock, reifier, fragment, /* flags= */ 0));

    // default-initialize all members
    for (auto it = enumSymbol->membersBegin(); it != enumSymbol->membersEnd(); it++) {
        auto &memberName = it->first;
        auto &fieldRef = it->second;

        lyric_assembler::AbstractSymbol *symbol;

        // resolve the member binding
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(fieldRef.symbolUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid enum field {}", fieldRef.symbolUrl.toString());

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

        // store default value in enum field
        TU_RETURN_IF_NOT_OK (fragment->storeRef(fieldRef, /* initialStore= */ true));

        // mark member as initialized
        TU_RETURN_IF_NOT_OK (enumSymbol->setMemberInitialized(memberName));
    }

    TU_LOG_INFO << "declared ctor " << ctorSymbol->getSymbolUrl() << " for " << enumSymbol->getSymbolUrl();

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    if (!enumSymbol->isCompletelyInitialized())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "enum {} is not completely initialized", enumSymbol->getSymbolUrl().toString());

    return ctorSymbol;
}

tempo_utils::Result<lyric_compiler::Member>
lyric_compiler::declare_enum_member(
    const lyric_parser::ArchetypeNode *node,
    bool isVariable,
    lyric_assembler::EnumSymbol *enumSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (enumSymbol != nullptr);
    auto *enumBlock = enumSymbol->enumBlock();

    // get member name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get member type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec memberSpec;
    TU_ASSIGN_OR_RETURN (memberSpec, typeSystem->parseAssignable(enumBlock, typeNode->getArchetypeNode()));
    lyric_common::TypeDef memberType;
    TU_ASSIGN_OR_RETURN (memberType, typeSystem->resolveAssignable(enumBlock, memberSpec));

    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    Member member;

    TU_ASSIGN_OR_RETURN (member.fieldSymbol, enumSymbol->declareMember(
        identifier, memberType, isVariable, convert_access_type(access)));

    TU_LOG_INFO << "declared member " << identifier << " for " << enumSymbol->getSymbolUrl();

    // define the initializer if specified
    if (node->numChildren() > 0) {
        TU_ASSIGN_OR_RETURN (member.procHandle, member.fieldSymbol->defineInitializer());
    }

    return member;
}

tempo_utils::Result<lyric_compiler::Method>
lyric_compiler::declare_enum_method(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::EnumSymbol *enumSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (enumSymbol != nullptr);
    auto *enumBlock = enumSymbol->enumBlock();

    // get method name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // determine the access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    // parse the return type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(enumBlock, typeNode->getArchetypeNode()));

    // parse the parameter list
    auto *packNode = node->getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(enumBlock, packNode->getArchetypeNode()));

    Method method;

    // declare the method
    TU_ASSIGN_OR_RETURN (method.callSymbol, enumSymbol->declareMethod(
        identifier, convert_access_type(access)));

    TU_LOG_INFO << "declared method " << identifier << " for " << enumSymbol->getSymbolUrl();

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

tempo_utils::Result<lyric_compiler::Impl>
lyric_compiler::declare_enum_impl(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::EnumSymbol *enumSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (enumSymbol != nullptr);
    auto *enumBlock = enumSymbol->enumBlock();

    // parse the impl type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(enumBlock, typeNode->getArchetypeNode()));

    // resolve the impl type
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(enumBlock, implSpec));

    Impl impl;

    // declare the impl
    TU_ASSIGN_OR_RETURN (impl.implHandle, enumSymbol->declareImpl(implType));

    TU_LOG_INFO << "declared impl " << implType << " for " << enumSymbol->getSymbolUrl();

    return impl;
}

tempo_utils::Result<lyric_assembler::EnumSymbol *>
lyric_compiler::declare_enum_case(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::EnumSymbol *enumSymbol,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    // get case name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get case access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    lyric_assembler::EnumSymbol *caseEnum;
    TU_ASSIGN_OR_RETURN (caseEnum, block->declareEnum(
        identifier, enumSymbol, lyric_compiler::convert_access_type(access),
        lyric_object::DeriveType::Final));

    // add case to set of sealed subtypes
    TU_RETURN_IF_NOT_OK (enumSymbol->putSealedType(caseEnum->getTypeDef()));

    return caseEnum;
}