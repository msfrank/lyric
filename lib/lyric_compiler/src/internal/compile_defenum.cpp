
#include <absl/strings/match.h>

#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_block.h>
#include <lyric_compiler/internal/compile_call.h>
#include <lyric_compiler/internal/compile_defenum.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

static tempo_utils::Result<std::string>
compile_defenum_val(
    lyric_assembler::EnumSymbol *baseEnum,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (baseEnum != nullptr);
    TU_ASSERT(walker.isValid());
    auto *enumBlock = baseEnum->enumBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // get val name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get val type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec valSpec;
    TU_ASSIGN_OR_RETURN (valSpec, typeSystem->parseAssignable(enumBlock, typeNode));
    lyric_common::TypeDef valType;
    TU_ASSIGN_OR_RETURN (valType, typeSystem->resolveAssignable(enumBlock, valSpec));

    // determine the access type
    lyric_object::AccessType accessType = lyric_object::AccessType::Public;
    if (absl::StartsWith(identifier, "__")) {
        accessType = lyric_object::AccessType::Private;
    } else if (absl::StartsWith(identifier, "_")) {
        accessType = lyric_object::AccessType::Protected;
    }

    lyric_assembler::FieldSymbol *fieldSymbol;
    TU_ASSIGN_OR_RETURN (fieldSymbol, baseEnum->declareMember(identifier, valType, false, accessType));

    TU_LOG_INFO << "declared val member " << identifier << " for " << baseEnum->getSymbolUrl();

    // compile the member initializer if specified
    if (walker.numChildren() > 0) {
        auto defaultInit = walker.getChild(0);
        TU_RETURN_IF_NOT_OK (lyric_compiler::internal::compile_member_initializer(
            fieldSymbol, defaultInit, moduleEntry));
    }

    return identifier;
}

static tempo_utils::Status
compile_defenum_def(
    lyric_assembler::EnumSymbol *baseEnum,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (baseEnum != nullptr);
    TU_ASSERT(walker.isValid());
    auto *enumBlock = baseEnum->enumBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    if (walker.numChildren() < 2)
        enumBlock->throwSyntaxError(walker, "invalid def");

    // get method name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // determine the access level
    lyric_object::AccessType access = lyric_object::AccessType::Public;
    if (absl::StartsWith(identifier, "__")) {
        access = lyric_object::AccessType::Private;
    } else if (absl::StartsWith(identifier, "_")) {
        access = lyric_object::AccessType::Protected;
    }

    // parse the return type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(enumBlock, typeNode));

    // parse the parameter list
    auto pack = walker.getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(enumBlock, pack));

    // declare the method
    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, baseEnum->declareMethod(identifier, access));

    auto *resolver = callSymbol->callResolver();

    // resolve the parameter pack
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(resolver, packSpec));

    // resolve the return type
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(resolver, returnSpec));

    // compile list parameter initializers
    for (const auto &p : parameterPack.listParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                enumBlock, entry->second, p, {}, moduleEntry));
            callSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    // compile named parameter initializers
    for (const auto &p : parameterPack.namedParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                enumBlock, entry->second, p, {}, moduleEntry));
            callSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    // compile the method body
    auto body = walker.getChild(1);
    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, returnType));
    lyric_common::TypeDef bodyType;
    TU_ASSIGN_OR_RETURN (bodyType, lyric_compiler::internal::compile_block(
        procHandle->procBlock(), body, moduleEntry));

    // add return instruction
    TU_RETURN_IF_NOT_OK (procHandle->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN));

    // validate that body returns the expected type
    bool isReturnable;
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, bodyType));
    if (!isReturnable)
        return enumBlock->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "def body does not match return type {}", returnType.toString());

    // validate that each exit returns the expected type
    for (auto it = procHandle->exitTypesBegin(); it != procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, *it));
        if (!isReturnable)
            return enumBlock->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "def body does not match return type {}", returnType.toString());
    }

    // return control to the caller
    return {};
}

static tempo_utils::Status
compile_defenum_base_init(
    lyric_assembler::EnumSymbol *baseEnum,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (baseEnum != nullptr);
    TU_ASSERT(walker.isValid());
    auto *baseBlock = baseEnum->enumBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    if (walker.numChildren() < 1)
        baseBlock->throwSyntaxError(walker, "invalid init");

    // parse the parameter list
    auto pack = walker.getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(baseBlock, pack));

    // check for initializers
    if (!packSpec.initializers.empty()) {
        for (const auto &p: packSpec.listParameterSpec) {
            if (!p.init.isEmpty())
                baseBlock->throwSyntaxError(p.init.getValue(),
                    "list parameter '{}' has unexpected initializer", p.name);
        }
        for (const auto &p : packSpec.namedParameterSpec) {
            if (!p.init.isEmpty())
                baseBlock->throwSyntaxError(p.init.getValue(),
                    "named parameter '{}' has unexpected initializer", p.name);
        }
    }

    // check for ctx parameters
    if (!packSpec.ctxParameterSpec.empty()) {
        auto &p = packSpec.ctxParameterSpec.front();
        baseBlock->throwSyntaxError(p.node, "unexpected ctx parameter '{}'", p.name);
    }

    // check for rest parameter
    if (!packSpec.restParameterSpec.isEmpty()) {
        auto p = packSpec.restParameterSpec.getValue();
        baseBlock->throwSyntaxError(p.node, "unexpected rest parameter");
    }

    // declare the base constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, baseEnum->declareCtor(lyric_object::AccessType::Public));

    auto *resolver = ctorSymbol->callResolver();

    // resolve the parameter pack
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(resolver, packSpec));

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, ctorSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));

    auto *code = procHandle->procCode();
    auto *ctorBlock = procHandle->procBlock();

    // find the superenum ctor
    lyric_assembler::ConstructableInvoker superCtor;
    TU_RETURN_IF_NOT_OK (baseEnum->superEnum()->prepareCtor(superCtor));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(superCtor));

    // load the uninitialized class onto the top of the stack
    TU_RETURN_IF_NOT_OK (code->loadSynthetic(lyric_assembler::SyntheticType::THIS));

    // call super ctor
    TU_RETURN_IF_STATUS (superCtor.invoke(ctorBlock, reifier, 0));

    // compile the constructor body if it exists
    if (walker.numChildren() == 2) {
        auto body = walker.getChild(1);
        TU_RETURN_IF_STATUS (lyric_compiler::internal::compile_block(ctorBlock, body, moduleEntry));
    }

    TU_LOG_INFO << "declared ctor " << ctorSymbol->getSymbolUrl() << " for " << baseEnum->getSymbolUrl();

    // add return instruction
    TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_RETURN));

    if (!baseEnum->isCompletelyInitialized())
        baseBlock->throwAssemblerInvariant("enum {} is not completely initialized",
            baseEnum->getSymbolUrl().toString());

    return {};
}

static tempo_utils::Status
compile_defenum_base_default_init(
    lyric_assembler::EnumSymbol *baseEnum,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (baseEnum != nullptr);
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *baseBlock = baseEnum->enumBlock();

    // declare the constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, baseEnum->declareCtor(lyric_object::AccessType::Public));

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, ctorSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));

    auto *code = procHandle->procCode();
    auto *ctorBlock = procHandle->procBlock();

    // find the superenum ctor
    lyric_assembler::ConstructableInvoker superCtor;
    TU_RETURN_IF_NOT_OK (baseEnum->superEnum()->prepareCtor(superCtor));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(superCtor));

    // load the uninitialized enum onto the top of the stack
    TU_RETURN_IF_NOT_OK (code->loadSynthetic(lyric_assembler::SyntheticType::THIS));

    // call super ctor
    TU_RETURN_IF_STATUS (superCtor.invoke(ctorBlock, reifier, 0));

    TU_LOG_INFO << "declared ctor " << ctorSymbol->getSymbolUrl() << " for " << baseEnum->getSymbolUrl();

    // add return instruction
    TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_RETURN));

    if (!baseEnum->isCompletelyInitialized())
        baseBlock->throwAssemblerInvariant("enum {} is not completely initialized",
            baseEnum->getSymbolUrl().toString());

    return {};
}

static tempo_utils::Status
compile_defenum_case_init(
    lyric_assembler::EnumSymbol *caseEnum,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (caseEnum != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *caseBlock = caseEnum->enumBlock();

    // declare the constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, caseEnum->declareCtor(lyric_object::AccessType::Public));

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, ctorSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));

    auto *code = procHandle->procCode();
    auto *ctorBlock = procHandle->procBlock();

    // find the superenum ctor
    lyric_assembler::ConstructableInvoker superCtor;
    TU_RETURN_IF_NOT_OK (caseEnum->superEnum()->prepareCtor(superCtor));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(superCtor));

    // load the uninitialized enum onto the top of the stack
    TU_RETURN_IF_NOT_OK (code->loadSynthetic(lyric_assembler::SyntheticType::THIS));

    // place arguments
    TU_RETURN_IF_NOT_OK (lyric_compiler::internal::compile_placement(
        superCtor.getConstructable(), ctorBlock, ctorBlock, reifier, walker, moduleEntry));

    // call super ctor
    TU_RETURN_IF_STATUS (superCtor.invoke(ctorBlock, reifier, 0));

    TU_LOG_INFO << "declared ctor " << ctorSymbol->getSymbolUrl() << " for " << caseEnum->getSymbolUrl();

    // add return instruction
    TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_RETURN));

    if (!caseEnum->isCompletelyInitialized())
        caseBlock->throwAssemblerInvariant("enum {} is not completely initialized",
            caseEnum->getSymbolUrl().toString());

    return {};
}

static tempo_utils::Status
compile_defenum_case(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::EnumSymbol *baseEnum,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT(walker.isValid());

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstCaseClass, 1);

    // get case call
    auto caseCall = walker.getChild(0);
    moduleEntry.checkClassOrThrow(caseCall, lyric_schema::kLyricAstCallClass);

    // get case name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(caseCall, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_assembler::EnumSymbol *caseEnum;
    TU_ASSIGN_OR_RETURN (caseEnum, block->declareEnum(
        identifier, baseEnum, lyric_object::AccessType::Public, lyric_object::DeriveType::Final));

    TU_LOG_INFO << "declared enum case " << caseEnum->getSymbolUrl() << " from " << baseEnum->getSymbolUrl();

    // add case to set of sealed subtypes
    auto status = baseEnum->putSealedType(caseEnum->getAssignableType());
    if (status.notOk())
        return status;

    // compile constructor
    return compile_defenum_case_init(caseEnum, caseCall, moduleEntry);
}

static tempo_utils::Status
compile_defenum_impl_def(
    lyric_assembler::ImplHandle *implHandle,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (implHandle != nullptr);
    TU_ASSERT(walker.isValid());
    auto *implBlock = implHandle->implBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 2);

    // get method name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // parse the return type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(implBlock, typeNode));

    // parse the parameter list
    auto pack = walker.getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(implBlock, pack));

    // check for initializers
    if (!packSpec.initializers.empty()) {
        for (const auto &p : packSpec.listParameterSpec) {
            if (!p.init.isEmpty())
                implBlock->throwSyntaxError(p.init.getValue(),
                    "list parameter '{}' has unexpected initializer", p.name);
        }
        for (const auto &p : packSpec.namedParameterSpec) {
            if (!p.init.isEmpty())
                implBlock->throwSyntaxError(p.init.getValue(),
                    "named parameter '{}' has unexpected initializer", p.name);
        }
    }

    // resolve the parameter pack
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(implBlock, packSpec));

    // resolve the return type
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(implBlock, returnSpec));

    // define the extension
    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, implHandle->defineExtension(identifier, parameterPack, returnType));

    // compile the method body
    auto body = walker.getChild(1);
    lyric_common::TypeDef bodyType;
    TU_ASSIGN_OR_RETURN (bodyType, lyric_compiler::internal::compile_block(
        procHandle->procBlock(), body, moduleEntry));

    // add return instruction
    TU_RETURN_IF_NOT_OK (procHandle->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN));

    // validate that body returns the expected type
    bool isReturnable;
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, bodyType));
    if (!isReturnable)
        return implBlock->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", returnType.toString());

    // validate that each exit returns the expected type
    for (auto it = procHandle->exitTypesBegin(); it != procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, *it));
        if (!isReturnable)
            return implBlock->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", returnType.toString());
    }

    // return control to the caller
    return {};
}

static tempo_utils::Status
compile_defenum_impl(
    lyric_assembler::EnumSymbol *enumSymbol,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (enumSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *enumBlock = enumSymbol->enumBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // parse the impl type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(enumBlock, typeNode));

    // resolve the impl type
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(enumBlock, implSpec));

    // declare the enum impl
    lyric_assembler::ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, enumSymbol->declareImpl(implType));

    // compile each impl def
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
        switch (childId) {
            case lyric_schema::LyricAstId::Def:
                TU_RETURN_IF_NOT_OK (compile_defenum_impl_def(implHandle, child, moduleEntry));
                break;
            default:
                enumBlock->throwSyntaxError(child, "expected impl def");
        }
    }

    return {};
}

tempo_utils::Status
lyric_compiler::internal::compile_defenum(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry,
    lyric_assembler::EnumSymbol **enumSymbolPtr)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();

    // generate the enum name from the identifier
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_parser::NodeWalker init;
    std::vector<lyric_parser::NodeWalker> cases;
    std::vector<lyric_parser::NodeWalker> vals;
    std::vector<lyric_parser::NodeWalker> defs;
    std::vector<lyric_parser::NodeWalker> impls;

    // make initial pass over class body
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
        switch (childId) {
            case lyric_schema::LyricAstId::Init:
                if (init.isValid())
                    block->throwSyntaxError(child, "duplicate init");
                init = child;
                break;
            case lyric_schema::LyricAstId::Case:
                cases.emplace_back(child);
                break;
            case lyric_schema::LyricAstId::Val:
                vals.emplace_back(child);
                break;
            case lyric_schema::LyricAstId::Def:
                defs.emplace_back(child);
                break;
            case lyric_schema::LyricAstId::Impl:
                impls.emplace_back(child);
                break;
            default:
                block->throwSyntaxError(child, "expected enum body");
        }
    }

    // the super enum of the base is Category
    auto fundamentalCategory = state->fundamentalCache()->getFundamentalUrl(
        lyric_assembler::FundamentalSymbol::Category);
    lyric_assembler::AbstractSymbol *categorySym;
    TU_ASSIGN_OR_RETURN (categorySym, state->symbolCache()->getOrImportSymbol(fundamentalCategory));
    if (categorySym->getSymbolType() != lyric_assembler::SymbolType::ENUM)
        block->throwAssemblerInvariant("invalid enum symbol {}", fundamentalCategory.toString());
    auto *categoryEnum = cast_symbol_to_enum(categorySym);

    // declare the base enum
    lyric_assembler::EnumSymbol *baseEnum;
    TU_ASSIGN_OR_RETURN (baseEnum, block->declareEnum(
        identifier, categoryEnum, lyric_object::AccessType::Public, lyric_object::DeriveType::Sealed));

    TU_LOG_INFO << "declared base enum " << baseEnum->getSymbolUrl();

    std::vector<std::string> members;
    tempo_utils::Status status;

    // compile members first
    for (const auto &val : vals) {
        auto valResult = compile_defenum_val(baseEnum, val, moduleEntry);
        if (valResult.isStatus())
            return valResult.getStatus();
        members.push_back(valResult.getResult());
    }

    // then compile constructor
    if (init.isValid()) {
        status = compile_defenum_base_init(baseEnum, init, moduleEntry);
        if (status.notOk())
            return status;
    } else {
        status = compile_defenum_base_default_init(baseEnum, moduleEntry);
        if (status.notOk())
            return status;
    }

    // then compile methods
    for (const auto &def : defs) {
        status = compile_defenum_def(baseEnum, def, moduleEntry);
        if (status.notOk())
            return status;
    }

    // then compile cases
    for (const auto &case_ : cases) {
        status = compile_defenum_case(block, case_, baseEnum, moduleEntry);
        if (status.notOk())
            return status;
    }

    // compile impls last
    for (const auto &impl : impls) {
        status = compile_defenum_impl(baseEnum, impl, moduleEntry);
        if (!status.isOk())
            return status;
    }

    if (enumSymbolPtr != nullptr) {
        *enumSymbolPtr = baseEnum;
    }

    return {};
}
