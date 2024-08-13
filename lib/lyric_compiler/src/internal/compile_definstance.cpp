#include <absl/strings/match.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/synthetic_symbol.h>
#include <lyric_compiler/internal/compile_block.h>
#include <lyric_compiler/internal/compile_definstance.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>
#include <tempo_utils/log_stream.h>

static tempo_utils::Result<std::string>
compile_definstance_val(
    lyric_assembler::InstanceSymbol *instanceSymbol,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (instanceSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *instanceBlock = instanceSymbol->instanceBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // get val name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get val type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec valSpec;
    TU_ASSIGN_OR_RETURN (valSpec, typeSystem->parseAssignable(instanceBlock, typeNode));
    lyric_common::TypeDef valType;
    TU_ASSIGN_OR_RETURN (valType, typeSystem->resolveAssignable(instanceBlock, valSpec));

    lyric_object::AccessType accessType = lyric_object::AccessType::Public;
    if (absl::StartsWith(identifier, "__")) {
        accessType = lyric_object::AccessType::Private;
    } else if (absl::StartsWith(identifier, "_")) {
        accessType = lyric_object::AccessType::Protected;
    }

    lyric_assembler::FieldSymbol *fieldSymbol;
    TU_ASSIGN_OR_RETURN (fieldSymbol, instanceSymbol->declareMember(identifier, valType, false, accessType));

    TU_LOG_INFO << "declared val member" << identifier << "for" << instanceSymbol->getSymbolUrl();

    // compile the member initializer if specified
    if (walker.numChildren() > 0) {
        auto defaultInit = walker.getChild(0);
        TU_RETURN_IF_NOT_OK (lyric_compiler::internal::compile_member_initializer(
            fieldSymbol, defaultInit, moduleEntry));
    }

    return identifier;
}

static tempo_utils::Result<std::string>
compile_definstance_var(
    lyric_assembler::InstanceSymbol *instanceSymbol,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (instanceSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *instanceBlock = instanceSymbol->instanceBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // get var name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get var type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec varSpec;
    TU_ASSIGN_OR_RETURN (varSpec, typeSystem->parseAssignable(instanceBlock, typeNode));
    lyric_common::TypeDef varType;
    TU_ASSIGN_OR_RETURN (varType, typeSystem->resolveAssignable(instanceBlock, varSpec));

    lyric_object::AccessType accessType = lyric_object::AccessType::Public;
    if (absl::StartsWith(identifier, "__")) {
        accessType = lyric_object::AccessType::Private;
    } else if (absl::StartsWith(identifier, "_")) {
        accessType = lyric_object::AccessType::Protected;
    }

    lyric_assembler::FieldSymbol *fieldSymbol;
    TU_ASSIGN_OR_RETURN (fieldSymbol, instanceSymbol->declareMember(identifier, varType, true, accessType));

    TU_LOG_INFO << "declared var member" << identifier << "for" << instanceSymbol->getSymbolUrl();

    // compile the member initializer if specified
    if (walker.numChildren() > 0) {
        auto defaultInit = walker.getChild(0);
        TU_RETURN_IF_NOT_OK (lyric_compiler::internal::compile_member_initializer(
            fieldSymbol, defaultInit, moduleEntry));
    }

    return identifier;
}

static tempo_utils::Status
compile_definstance_init(
    lyric_assembler::InstanceSymbol *instanceSymbol,
    const absl::flat_hash_set<std::string> &instanceMemberNames,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (instanceSymbol != nullptr);
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();
    auto *instanceBlock = instanceSymbol->instanceBlock();

    // declare the constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, instanceSymbol->declareCtor(lyric_object::AccessType::Public));

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, ctorSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));

    auto *code = procHandle->procCode();
    auto *ctorBlock = procHandle->procBlock();

    // find the superinstance ctor
    lyric_assembler::ConstructableInvoker superCtor;
    TU_RETURN_IF_NOT_OK (instanceSymbol->superInstance()->prepareCtor(superCtor));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(superCtor));

    // load the uninitialized instance onto the top of the stack
    TU_RETURN_IF_NOT_OK (code->loadSynthetic(lyric_assembler::SyntheticType::THIS));

    // call super ctor
    TU_RETURN_IF_STATUS (superCtor.invoke(ctorBlock, reifier, 0));

    // if any members are uninitialized, then try to default-initialize them
    for (const auto &memberName : instanceMemberNames) {

        // skip members which have been initialized already
        if (instanceSymbol->isMemberInitialized(memberName))
            continue;

        lyric_assembler::AbstractSymbol *symbol;

        // resolve the member binding
        auto maybeMember = instanceSymbol->getMember(memberName);
        if (maybeMember.isEmpty())
            instanceBlock->throwAssemblerInvariant("missing instance member {}", memberName);
        auto fieldRef = maybeMember.getValue();
        TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(fieldRef.symbolUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            instanceBlock->throwAssemblerInvariant("invalid instance field {}", fieldRef.symbolUrl.toString());
        auto *fieldSymbol = cast_symbol_to_field(symbol);
        auto fieldInitializerUrl = fieldSymbol->getInitializer();
        if (!fieldInitializerUrl.isValid())
            instanceBlock->throwAssemblerInvariant("missing field initializer {}", fieldInitializerUrl.toString());
        TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(fieldInitializerUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
            instanceBlock->throwAssemblerInvariant("invalid field initializer {}", fieldInitializerUrl.toString());
        auto *initializerCall = cast_symbol_to_call(symbol);

        // load $this onto the top of the stack
        TU_RETURN_IF_NOT_OK (code->loadSynthetic(lyric_assembler::SyntheticType::THIS));

        // invoke initializer to place default value onto the top of the stack
        auto callable = std::make_unique<lyric_assembler::FunctionCallable>(initializerCall);
        lyric_assembler::CallableInvoker invoker;
        TU_RETURN_IF_NOT_OK (invoker.initialize(std::move(callable)));
        lyric_typing::CallsiteReifier initializerReifier(typeSystem);
        TU_RETURN_IF_NOT_OK (initializerReifier.initialize(invoker));
        TU_RETURN_IF_STATUS (invoker.invoke(ctorBlock, initializerReifier));

        // store default value in instance field
        TU_RETURN_IF_NOT_OK (ctorBlock->store(fieldRef));

        // mark member as initialized
        TU_RETURN_IF_NOT_OK (instanceSymbol->setMemberInitialized(memberName));
    }

    TU_LOG_INFO << "declared ctor " << ctorSymbol->getSymbolUrl() << " for " << instanceSymbol->getSymbolUrl();

    // add return instruction
    TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_RETURN));

    if (!instanceSymbol->isCompletelyInitialized())
        instanceBlock->throwAssemblerInvariant("instance {} is not completely initialized",
            instanceSymbol->getSymbolUrl().toString());

    return {};
}

static tempo_utils::Status
compile_definstance_def(
    lyric_assembler::InstanceSymbol *instanceSymbol,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (instanceSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *instanceBlock = instanceSymbol->instanceBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 2);

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
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(instanceBlock, typeNode));

    // parse the parameter list
    auto pack = walker.getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(instanceBlock, pack));

    // declare the method
    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, instanceSymbol->declareMethod(identifier, access));

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
                instanceBlock, entry->second, p, {}, moduleEntry));
            callSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    // compile named parameter initializers
    for (const auto &p : parameterPack.namedParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                instanceBlock, entry->second, p, {}, moduleEntry));
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
        return moduleEntry.logAndContinue(body.getLocation(),
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", returnType.toString());

    // validate that each exit returns the expected type
    for (auto it = procHandle->exitTypesBegin(); it != procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, *it));
        if (!isReturnable)
            return moduleEntry.logAndContinue(body.getLocation(),
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", returnType.toString());
    }

    // return control to the caller
    return {};
}

static tempo_utils::Status
compile_definstance_impl_def(
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
                return moduleEntry.logAndContinue(p.init.getValue().getLocation(),
                    lyric_compiler::CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "list parameter '{}' has unexpected initializer", p.name);
        }
        for (const auto &p : packSpec.namedParameterSpec) {
            if (!p.init.isEmpty())
                return moduleEntry.logAndContinue(p.init.getValue().getLocation(),
                    lyric_compiler::CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
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
        return moduleEntry.logAndContinue(body.getLocation(),
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", returnType.toString());

    // validate that body returns the expected type
    for (auto it = procHandle->exitTypesBegin(); it != procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, *it));
        if (!isReturnable)
            return moduleEntry.logAndContinue(body.getLocation(),
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", returnType.toString());
    }

    // return control to the caller
    return {};
}

static tempo_utils::Status
compile_definstance_impl(
    lyric_assembler::InstanceSymbol *instanceSymbol,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (instanceSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *instanceBlock = instanceSymbol->instanceBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // parse the impl type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(instanceBlock, typeNode));

    // resolve the impl type
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(instanceBlock, implSpec));

    // declare the instance impl
    lyric_assembler::ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, instanceSymbol->declareImpl(implType));

    // compile each impl def
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
        switch (childId) {
            case lyric_schema::LyricAstId::Def:
                TU_RETURN_IF_NOT_OK (compile_definstance_impl_def(implHandle, child, moduleEntry));
                break;
            default:
                instanceBlock->throwAssemblerInvariant("expected impl def");
        }
    }

    return {};
}

tempo_utils::Status
lyric_compiler::internal::compile_definstance(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry,
    lyric_assembler::InstanceSymbol **instanceSymbolPtr)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();

    // get instance name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    std::vector<lyric_parser::NodeWalker> vals;
    std::vector<lyric_parser::NodeWalker> vars;
    std::vector<lyric_parser::NodeWalker> defs;
    std::vector<lyric_parser::NodeWalker> impls;

    // make initial pass over class body
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
        switch (childId) {
            case lyric_schema::LyricAstId::Val:
                vals.emplace_back(child);
                break;
            case lyric_schema::LyricAstId::Var:
                vars.emplace_back(child);
                break;
            case lyric_schema::LyricAstId::Def:
                defs.emplace_back(child);
                break;
            case lyric_schema::LyricAstId::Impl:
                impls.emplace_back(child);
                break;
            default:
                block->throwAssemblerInvariant("expected instance body");
        }
    }

    //
    auto fundamentalSingleton = state->fundamentalCache()->getFundamentalUrl(
        lyric_assembler::FundamentalSymbol::Singleton);
    lyric_assembler::AbstractSymbol *superinstanceSym;
    TU_ASSIGN_OR_RETURN (superinstanceSym, state->symbolCache()->getOrImportSymbol(fundamentalSingleton));
    if (superinstanceSym->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
        block->throwAssemblerInvariant("invalid instance symbol {}", fundamentalSingleton.toString());
    auto *superInstance = cast_symbol_to_instance(superinstanceSym);

    lyric_assembler::InstanceSymbol *instanceSymbol;
    TU_ASSIGN_OR_RETURN (instanceSymbol, block->declareInstance(
        identifier, superInstance, lyric_object::AccessType::Public));

    TU_LOG_INFO << "declared instance " << instanceSymbol->getSymbolUrl() << " from " << superInstance->getSymbolUrl();

    tempo_utils::Status status;
    absl::flat_hash_set<std::string> instanceMemberNames;

    // compile members first
    for (const auto &val : vals) {
        auto valResult = compile_definstance_val(instanceSymbol, val, moduleEntry);
        if (valResult.isStatus())
            return valResult.getStatus();
        instanceMemberNames.insert(valResult.getResult());
    }
    for (const auto &var : vars) {
        auto varResult = compile_definstance_var(instanceSymbol, var, moduleEntry);
        if (varResult.isStatus())
            return varResult.getStatus();
        instanceMemberNames.insert(varResult.getResult());
    }

    // then compile constructor
    status = compile_definstance_init(instanceSymbol, instanceMemberNames, moduleEntry);
    if (!status.isOk())
        return status;

    // then compile methods
    for (const auto &def : defs) {
        status = compile_definstance_def(instanceSymbol, def, moduleEntry);
        if (!status.isOk())
            return status;
    }

    // compile impls last
    for (const auto &impl : impls) {
        status = compile_definstance_impl(instanceSymbol, impl, moduleEntry);
        if (!status.isOk())
            return status;
    }

    if (instanceSymbolPtr != nullptr) {
        *instanceSymbolPtr = instanceSymbol;
    }

    return CompilerStatus::ok();
}
