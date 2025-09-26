
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/defstruct_utils.h>
#include <lyric_parser/ast_attrs.h>

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_compiler::declare_struct_default_init(
    lyric_assembler::StructSymbol *structSymbol,
    const std::string &allocatorTrap,
    lyric_assembler::SymbolCache *symbolCache)
{
    TU_ASSERT (structSymbol != nullptr);

    // declare the constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, structSymbol->declareCtor(
        lyric_object::kCtorSpecialSymbol, /* isHidden= */ false, allocatorTrap));

    lyric_assembler::ParameterPack parameterPack;
    absl::flat_hash_map<std::string,lyric_common::SymbolUrl> paramInitializers;
    tu_uint8 currparam = 0;

    for (auto it = structSymbol->membersBegin();
        it != structSymbol->membersEnd();
        it++)
    {
        const auto &memberName = it->first;
        const auto &fieldRef = it->second;

        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(fieldRef.symbolUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid struct field {}", fieldRef.symbolUrl.toString());
        auto *fieldSymbol = cast_symbol_to_field(symbol);
        auto fieldInitializerUrl = fieldSymbol->getInitializer();

        lyric_assembler::Parameter p;
        p.index = currparam++;
        p.name = memberName;
        p.label = memberName;
        p.isVariable = false;
        p.placement = lyric_object::PlacementType::Invalid;
        p.typeDef = fieldSymbol->getTypeDef();

        if (fieldInitializerUrl.isValid()) {
            if (!symbolCache->hasSymbol(fieldInitializerUrl))
                return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                    "missing field initializer {}", fieldInitializerUrl.toString());
            p.placement = lyric_object::PlacementType::NamedOpt;
            paramInitializers[memberName] = fieldInitializerUrl;
            parameterPack.namedParameters.push_back(p);
        } else {
            p.placement = lyric_object::PlacementType::Named;
            parameterPack.namedParameters.push_back(p);
        }
    }

    // define constructor
    TU_RETURN_IF_STATUS(ctorSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));

    // set the initializer for each NamedOpt param
    for (const auto &entry : paramInitializers) {
        TU_RETURN_IF_NOT_OK (ctorSymbol->putInitializer(entry.first, entry.second));
    }

    return ctorSymbol;
}

tempo_utils::Status
lyric_compiler::define_struct_default_init(
    const DefStruct *defstruct,
    const std::string &allocatorTrap,
    lyric_assembler::SymbolCache *symbolCache,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (defstruct != nullptr);

    auto *structSymbol = defstruct->structSymbol;
    TU_ASSERT (structSymbol != nullptr);
    auto *ctorSymbol = defstruct->defaultCtor;
    TU_ASSERT (ctorSymbol != nullptr);

    // verify that super ctor takes no arguments
    auto *superStruct = structSymbol->superStruct();
    auto superCtorUrl = superStruct->getCtor(lyric_object::kCtorSpecialSymbol);
    lyric_assembler::AbstractSymbol *superCtorSymbol;
    TU_ASSIGN_OR_RETURN (superCtorSymbol, symbolCache->getOrImportSymbol(superCtorUrl));
    auto *superCtorCall = cast_symbol_to_call(superCtorSymbol);
    if (superCtorCall->numParameters() > 0)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "super struct {} is not default-constructable", superCtorUrl.toString());

    auto *procHandle = ctorSymbol->callProc();
    auto *ctorBlock = procHandle->procBlock();
    auto *procBuilder = procHandle->procCode();
    auto *fragment = procBuilder->rootFragment();

    // find the superstruct ctor
    lyric_assembler::ConstructableInvoker superCtor;
    TU_RETURN_IF_NOT_OK (superStruct->prepareCtor(lyric_object::kCtorSpecialSymbol, superCtor));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(superCtor));

    // load the uninitialized struct onto the top of the stack
    TU_RETURN_IF_NOT_OK (fragment->loadThis());

    // call super ctor
    TU_RETURN_IF_STATUS (superCtor.invoke(ctorBlock, reifier, fragment, /* flags= */ 0));

    for (auto it = structSymbol->membersBegin();
         it != structSymbol->membersEnd();
         it++)
    {
        const auto &memberName = it->first;
        const auto &fieldRef = it->second;

        // resolve argument binding
        lyric_assembler::DataReference argRef;
        TU_ASSIGN_OR_RETURN (argRef, ctorBlock->resolveReference(memberName));

        // load $this onto the top of the stack
        TU_RETURN_IF_NOT_OK (fragment->loadThis());

        // load argument value
        TU_RETURN_IF_NOT_OK (fragment->loadRef(argRef));

        // store default value in struct field
        TU_RETURN_IF_NOT_OK (fragment->storeRef(fieldRef, /* initialStore= */ true));

        // mark member as initialized
        TU_RETURN_IF_NOT_OK (structSymbol->setMemberInitialized(memberName));
    }

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    if (!structSymbol->isCompletelyInitialized())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "struct {} is not completely initialized", structSymbol->getSymbolUrl().toString());

    TU_LOG_V << "defined default ctor " << ctorSymbol->getSymbolUrl()
        << " for " << structSymbol->getSymbolUrl();

    return {};
}

tempo_utils::Result<lyric_compiler::Constructor>
lyric_compiler::declare_struct_init(
    const lyric_parser::ArchetypeNode *initNode,
    lyric_assembler::StructSymbol *structSymbol,
    const std::string &allocatorTrap,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (initNode != nullptr);
    TU_ASSERT (structSymbol != nullptr);
    auto *structBlock = structSymbol->structBlock();

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
    TU_ASSIGN_OR_RETURN (ctorSymbol, structSymbol->declareCtor(identifier, isHidden, allocatorTrap));

    lyric_typing::PackSpec packSpec;
    lyric_assembler::ParameterPack parameterPack;

    // compile the parameter list if specified, otherwise default to an empty list
    if (initNode != nullptr) {
        auto *packNode = initNode->getChild(0);
        TU_ASSERT (packNode != nullptr);
        TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(structBlock, packNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(structBlock, packSpec));
    }

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, ctorSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));

    Constructor constructor;
    constructor.superSymbol = structSymbol->superStruct();
    constructor.callSymbol = ctorSymbol;
    constructor.procHandle = procHandle;
    return constructor;
}

tempo_utils::Result<lyric_compiler::Member>
lyric_compiler::declare_struct_member(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::StructSymbol *structSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (structSymbol != nullptr);
    auto *structBlock = structSymbol->structBlock();

    // get member name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get member type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec memberSpec;
    TU_ASSIGN_OR_RETURN (memberSpec, typeSystem->parseAssignable(structBlock, typeNode->getArchetypeNode()));
    lyric_common::TypeDef memberType;
    TU_ASSIGN_OR_RETURN (memberType, typeSystem->resolveAssignable(structBlock, memberSpec));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    Member member;

    TU_ASSIGN_OR_RETURN (member.fieldSymbol, structSymbol->declareMember(identifier, memberType, isHidden));

    TU_LOG_V << "declared member " << identifier << " for " << structSymbol->getSymbolUrl();

    // define the initializer if specified
    if (node->numChildren() > 0) {
        TU_ASSIGN_OR_RETURN (member.initializerHandle, member.fieldSymbol->defineInitializer());
    }

    return member;
}

tempo_utils::Status
lyric_compiler::default_initialize_struct_members(
    lyric_assembler::StructSymbol *structSymbol,
    lyric_assembler::CallSymbol *ctorSymbol,
    lyric_assembler::SymbolCache *symbolCache,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (structSymbol != nullptr);
    TU_ASSERT (ctorSymbol != nullptr);
    TU_ASSERT (symbolCache != nullptr);
    TU_ASSERT (typeSystem != nullptr);
    auto *procHandle = ctorSymbol->callProc();
    auto *ctorBlock = procHandle->procBlock();
    auto *procBuilder = procHandle->procCode();
    auto *fragment = procBuilder->rootFragment();

    // default-initialize all members
    for (auto it = structSymbol->membersBegin();
        it != structSymbol->membersEnd();
        it++)
    {
        auto &memberName = it->first;
        auto &fieldRef = it->second;

        // skip members which have been initialized already
        if (structSymbol->isMemberInitialized(memberName))
            continue;

        lyric_assembler::AbstractSymbol *symbol;

        // resolve the member binding
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(fieldRef.symbolUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid struct field {}", fieldRef.symbolUrl.toString());

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

        // store default value in struct field
        TU_RETURN_IF_NOT_OK (fragment->storeRef(fieldRef, /* initialStore= */ true));

        // mark member as initialized
        TU_RETURN_IF_NOT_OK (structSymbol->setMemberInitialized(memberName));
    }

    return {};
}

tempo_utils::Result<lyric_compiler::Method>
lyric_compiler::declare_struct_method(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::StructSymbol *structSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (structSymbol != nullptr);
    auto *structBlock = structSymbol->structBlock();

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
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(structBlock, typeNode->getArchetypeNode()));

    // parse the parameter list
    auto *packNode = node->getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(structBlock, packNode->getArchetypeNode()));

    // if method is generic, then parse the template parameter list
    lyric_typing::TemplateSpec templateSpec;
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(structBlock, genericNode->getArchetypeNode()));
    }

    Method method;

    // declare the method
    TU_ASSIGN_OR_RETURN (method.callSymbol, structSymbol->declareMethod(identifier, isHidden));

    TU_LOG_V << "declared method " << identifier << " for " << structSymbol->getSymbolUrl();

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
lyric_compiler::declare_struct_impl(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::StructSymbol *structSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (structSymbol != nullptr);
    auto *structBlock = structSymbol->structBlock();

    // parse the impl type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(structBlock, typeNode->getArchetypeNode()));

    // resolve the impl type
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(structBlock, implSpec));

    Impl impl;

    // declare the impl
    TU_ASSIGN_OR_RETURN (impl.implHandle, structSymbol->declareImpl(implType));

    TU_LOG_V << "declared impl " << implType << " for " << structSymbol->getSymbolUrl();

    return impl;
}
