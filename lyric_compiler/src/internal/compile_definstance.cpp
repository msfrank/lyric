#include <absl/strings/match.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/synthetic_symbol.h>
#include <lyric_compiler/internal/compile_definstance.h>
#include <lyric_compiler/internal/compile_node.h>
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
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto resolveValSpecResult = typeSystem->parseAssignable(instanceBlock, type);
    if (resolveValSpecResult.isStatus())
        return resolveValSpecResult.getStatus();
    auto valType = resolveValSpecResult.getResult();

    lyric_object::AccessType accessType = lyric_object::AccessType::Public;
    if (absl::StartsWith(identifier, "__")) {
        accessType = lyric_object::AccessType::Private;
    } else if (absl::StartsWith(identifier, "_")) {
        accessType = lyric_object::AccessType::Protected;
    }

    // compile the member initializer if specified
    lyric_common::SymbolUrl init;
    if (walker.numChildren() > 0) {
        auto defaultInit = walker.getChild(0);
        auto compileDefaultResult = lyric_compiler::internal::compile_default_initializer(instanceSymbol->instanceBlock(),
            identifier, valType, defaultInit, moduleEntry);
        if (compileDefaultResult.isStatus())
            return compileDefaultResult.getStatus();
        init = compileDefaultResult.getResult();
    }

    auto declareMemberResult = instanceSymbol->declareMember(identifier, resolveValSpecResult.getResult(),
        false, accessType, init);
    if (declareMemberResult.isStatus())
        return declareMemberResult.getStatus();

    TU_LOG_INFO << "declared val member" << identifier << "for" << instanceSymbol->getSymbolUrl();

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
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto resolveVarSpecResult = typeSystem->parseAssignable(instanceBlock, type);
    if (resolveVarSpecResult.isStatus())
        return resolveVarSpecResult.getStatus();
    auto varType = resolveVarSpecResult.getResult();

    lyric_object::AccessType accessType = lyric_object::AccessType::Public;
    if (absl::StartsWith(identifier, "__")) {
        accessType = lyric_object::AccessType::Private;
    } else if (absl::StartsWith(identifier, "_")) {
        accessType = lyric_object::AccessType::Protected;
    }

    // compile the member initializer if specified
    lyric_common::SymbolUrl init;
    if (walker.numChildren() > 0) {
        auto defaultInit = walker.getChild(0);
        auto compileDefaultResult = lyric_compiler::internal::compile_default_initializer(instanceSymbol->instanceBlock(),
            identifier, varType, defaultInit, moduleEntry);
        if (compileDefaultResult.isStatus())
            return compileDefaultResult.getStatus();
        init = compileDefaultResult.getResult();
    }

    auto declareMemberResult = instanceSymbol->declareMember(identifier, resolveVarSpecResult.getResult(),
        true, accessType, init);
    if (declareMemberResult.isStatus())
        return declareMemberResult.getStatus();

    TU_LOG_INFO << "declared var member" << identifier << "for" << instanceSymbol->getSymbolUrl();

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
    auto declareCtorResult = instanceSymbol->declareCtor(lyric_object::AccessType::Public);
    if (declareCtorResult.isStatus())
        return declareCtorResult.getStatus();
    auto ctorUrl = declareCtorResult.getResult();
    auto *ctorSym = state->symbolCache()->getSymbol(ctorUrl);
    if (ctorSym == nullptr)
        instanceBlock->throwAssemblerInvariant("missing call symbol {}", ctorUrl.toString());
    if (ctorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        instanceBlock->throwAssemblerInvariant("invalid call symbol {}", ctorUrl.toString());
    auto *ctor = cast_symbol_to_call(ctorSym);

    auto *proc = ctor->callProc();
    auto *code = proc->procCode();
    auto *ctorBlock = proc->procBlock();

    // find the superinstance ctor
    auto resolveSuperCtorResult = instanceSymbol->superInstance()->resolveCtor();
    if (resolveSuperCtorResult.isStatus())
        return resolveSuperCtorResult.getStatus();
    auto superCtor = resolveSuperCtorResult.getResult();

    lyric_typing::CallsiteReifier reifier(superCtor.getParameters(), superCtor.getRest(),
        superCtor.getTemplateUrl(), superCtor.getTemplateParameters(),
        superCtor.getTemplateArguments(), typeSystem);

    //
    auto status = code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
    if (status.notOk())
        return status;

    // call super ctor, instance is now on the top of the stack
    auto invokeSuperCtorResult = superCtor.invoke(ctorBlock, reifier);
    if (invokeSuperCtorResult.isStatus())
        return invokeSuperCtorResult.getStatus();

    // if any members are uninitialized, then try to default-initialize them
    for (const auto &memberName : instanceMemberNames) {

        // skip members which have been initialized already
        if (instanceSymbol->isMemberInitialized(memberName))
            continue;

        // resolve the member binding
        auto maybeBinding = instanceSymbol->getMember(memberName);
        if (maybeBinding.isEmpty())
            instanceBlock->throwAssemblerInvariant("missing instance member {}", memberName);
        auto fieldVar = maybeBinding.getValue();
        if (!state->symbolCache()->hasSymbol(fieldVar.symbol))
            instanceBlock->throwAssemblerInvariant("missing instance field {}", fieldVar.symbol.toString());
        auto *sym = state->symbolCache()->getSymbol(fieldVar.symbol);
        if (sym->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            instanceBlock->throwAssemblerInvariant("invalid instance field {}", fieldVar.symbol.toString());
        auto *fieldSymbol = cast_symbol_to_field(sym);
        auto fieldInitializerUrl = fieldSymbol->getInitializer();
        if (!fieldInitializerUrl.isValid())
            instanceBlock->throwAssemblerInvariant("missing field initializer {}", fieldInitializerUrl.toString());
        if (!state->symbolCache()->hasSymbol(fieldInitializerUrl))
            instanceBlock->throwAssemblerInvariant("missing field initializer {}", fieldInitializerUrl.toString());
        sym = state->symbolCache()->getSymbol(fieldInitializerUrl);
        if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            instanceBlock->throwAssemblerInvariant("invalid field initializer {}", fieldInitializerUrl.toString());
        auto *initializerCall = cast_symbol_to_call(sym);

        // load $this onto the top of the stack
        status = code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        if (!status.isOk())
            return status;

        // invoke initializer to place default value onto the top of the stack
        lyric_assembler::CallInvoker initializerInvoker(initializerCall);
        lyric_typing::CallsiteReifier initializerReifier(
            initializerInvoker.getParameters(), initializerInvoker.getRest(),
            initializerInvoker.getTemplateUrl(), initializerInvoker.getTemplateParameters(),
            initializerInvoker.getTemplateArguments(), typeSystem);
        auto invokeInitializerResult = initializerInvoker.invoke(ctorBlock, initializerReifier);
        if (invokeInitializerResult.isStatus())
            return invokeInitializerResult.getStatus();

        // store default value in instance field
        status = ctorBlock->store(fieldVar);
        if (!status.isOk())
            return status;

        // mark member as initialized
        status = instanceSymbol->setMemberInitialized(memberName);
        if (!status.isOk())
            return status;
    }

    TU_LOG_INFO << "declared ctor " << ctorUrl << " for " << instanceSymbol->getSymbolUrl();

    // add return instruction
    status = code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    if (!instanceSymbol->isCompletelyInitialized())
        instanceBlock->throwAssemblerInvariant("instance {} is not completely initialized",
            instanceSymbol->getSymbolUrl().toString());

    return status;
}

static tempo_utils::Status
compile_definstance_def(
    lyric_assembler::InstanceSymbol *instanceSymbol,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (instanceSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 2);

    auto *instanceBlock = instanceSymbol->instanceBlock();

    // get method name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get method return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto compileTypeResult = typeSystem->parseAssignable(instanceBlock, type);
    if (compileTypeResult.isStatus())
        return compileTypeResult.getStatus();
    auto returnSpec = compileTypeResult.getResult();

    // compile the parameter list
    auto pack = walker.getChild(0);
    auto compilePackResult = typeSystem->parsePack(instanceBlock, pack);
    if (compilePackResult.isStatus())
        return compilePackResult.getStatus();
    auto packSpec = compilePackResult.getResult();

    // compile initializers
    absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    for (const auto &p : packSpec.parameterSpec) {
        if (!p.init.isEmpty()) {
            auto compileInitializerResult = lyric_compiler::internal::compile_default_initializer(instanceBlock,
                p.name, p.type, p.init.getValue(), moduleEntry);
            if (compileInitializerResult.isStatus())
                return compileInitializerResult.getStatus();
            initializers[p.name] = compileInitializerResult.getResult();
        }
    }

    lyric_object::AccessType accessType = lyric_object::AccessType::Public;
    if (absl::StartsWith(identifier, "__")) {
        accessType = lyric_object::AccessType::Private;
    } else if (absl::StartsWith(identifier, "_")) {
        accessType = lyric_object::AccessType::Protected;
    }

    // declare the method
    auto declareMethodResult = instanceSymbol->declareMethod(identifier, packSpec.parameterSpec,
        packSpec.restSpec, packSpec.ctxSpec, returnSpec, accessType);
    if (declareMethodResult.isStatus())
        return declareMethodResult.getStatus();
    auto methodUrl = declareMethodResult.getResult();
    auto *sym = state->symbolCache()->getSymbol(methodUrl);
    if (sym == nullptr)
        instanceBlock->throwAssemblerInvariant("missing call symbol {}", methodUrl.toString());
    if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        instanceBlock->throwAssemblerInvariant("invalid call symbol {}", methodUrl.toString());
    auto *call = cast_symbol_to_call(sym);
    for (const auto &entry : initializers) {
        call->putInitializer(entry.first, entry.second);
    }

    // compile the method body
    auto body = walker.getChild(1);
    auto *proc = call->callProc();
    auto compileBodyResult = lyric_compiler::internal::compile_block(proc->procBlock(), body, moduleEntry);
    if (compileBodyResult.isStatus())
        return compileBodyResult.getStatus();
    auto bodyType = compileBodyResult.getResult();

    // add return instruction
    auto status = proc->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    // validate that body returns the expected type
    if (!typeSystem->isAssignable(call->getReturnType(), bodyType))
        return instanceBlock->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", call->getReturnType().toString());
    for (const auto &exitType : call->listExitTypes()) {
        if (!typeSystem->isAssignable(call->getReturnType(), exitType))
            return instanceBlock->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", call->getReturnType().toString());
    }

    // return control to the caller
    return lyric_compiler::CompilerStatus::ok();
}

static tempo_utils::Status
compile_definstance_impl_def(
    lyric_assembler::ImplHandle *implHandle,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (implHandle != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 2);

    auto *implBlock = implHandle->implBlock();

    // get method name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get method return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(implBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto returnSpec = parseAssignableResult.getResult();

    // compile the parameter list
    auto pack = walker.getChild(0);
    auto parsePackResult = typeSystem->parsePack(implBlock, pack);
    if (parsePackResult.isStatus())
        return parsePackResult.getStatus();
    auto packSpec = parsePackResult.getResult();

    // compile initializers
    absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    for (const auto &p : packSpec.parameterSpec) {
        if (!p.init.isEmpty()) {
            auto compileInitializerResult = lyric_compiler::internal::compile_default_initializer(implBlock,
                p.name, p.type, p.init.getValue(), moduleEntry);
            if (compileInitializerResult.isStatus())
                return compileInitializerResult.getStatus();
            initializers[p.name] = compileInitializerResult.getResult();
        }
    }

    // declare the impl extension
    lyric_assembler::ExtensionMethod extension;
    TU_ASSIGN_OR_RETURN (extension, implHandle->declareExtension(
        identifier, packSpec.parameterSpec, packSpec.restSpec, packSpec.ctxSpec, returnSpec));
    auto *sym = state->symbolCache()->getSymbol(extension.methodCall);
    if (sym == nullptr)
        implBlock->throwAssemblerInvariant("missing call symbol {}", extension.methodCall.toString());
    if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        implBlock->throwAssemblerInvariant("invalid call symbol {}", extension.methodCall.toString());
    auto *call = cast_symbol_to_call(sym);
    for (const auto &entry : initializers) {
        call->putInitializer(entry.first, entry.second);
    }

    // compile the method body
    auto body = walker.getChild(1);
    auto *proc = call->callProc();
    auto compileBodyResult = lyric_compiler::internal::compile_block(proc->procBlock(), body, moduleEntry);
    if (compileBodyResult.isStatus())
        return compileBodyResult.getStatus();
    auto bodyType = compileBodyResult.getResult();

    // add return instruction
    auto status = proc->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    // validate that body returns the expected type
    if (!typeSystem->isAssignable(call->getReturnType(), bodyType))
        return implBlock->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", call->getReturnType().toString());
    for (const auto &exitType : call->listExitTypes()) {
        if (!typeSystem->isAssignable(call->getReturnType(), exitType))
            return implBlock->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", call->getReturnType().toString());
    }

    // return control to the caller
    return lyric_compiler::CompilerStatus::ok();
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

    // get impl type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(instanceBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto implSpec = parseAssignableResult.getResult();

    tempo_utils::Status status;

    // declare the instance impl
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, instanceSymbol->declareImpl(implSpec));
    auto *implHandle = instanceSymbol->getImpl(implType);

    // compile each impl def
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
        switch (childId) {
            case lyric_schema::LyricAstId::Def:
                status = compile_definstance_impl_def(implHandle, child, moduleEntry);
                break;
            default:
                instanceBlock->throwSyntaxError(child, "expected impl def");
        }
        if (!status.isOk())
            return status;
    }

    return lyric_compiler::CompilerStatus::ok();
}

tempo_utils::Status
lyric_compiler::internal::compile_definstance(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
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
                block->throwSyntaxError(child, "expected instance body");
        }
    }

    //
    auto fundamentalSingleton = state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Singleton);
    auto *superInstanceSym = state->symbolCache()->getSymbol(fundamentalSingleton);
    if (superInstanceSym == nullptr)
        block->throwAssemblerInvariant("missing instance symbol {}", fundamentalSingleton.toString());
    if (superInstanceSym->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
        block->throwAssemblerInvariant("invalid instance symbol {}", fundamentalSingleton.toString());
    auto *superInstance = cast_symbol_to_instance(superInstanceSym);

    auto declInstanceResult = block->declareInstance(
        identifier, superInstance, lyric_object::AccessType::Public);
    if (declInstanceResult.isStatus())
        return declInstanceResult.getStatus();
    auto instanceUrl = declInstanceResult.getResult();

    auto *instanceSym = state->symbolCache()->getSymbol(instanceUrl);
    if (instanceSym == nullptr)
        block->throwAssemblerInvariant("missing instance symbol {}", instanceUrl.toString());
    if (instanceSym->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
        block->throwAssemblerInvariant("invalid instance symbol {}", instanceUrl.toString());
    auto *instance = cast_symbol_to_instance(instanceSym);

    TU_LOG_INFO << "declared instance " << identifier << " with url " << instanceUrl;

    tempo_utils::Status status;
    absl::flat_hash_set<std::string> instanceMemberNames;

    // compile members first
    for (const auto &val : vals) {
        auto valResult = compile_definstance_val(instance, val, moduleEntry);
        if (valResult.isStatus())
            return valResult.getStatus();
        instanceMemberNames.insert(valResult.getResult());
    }
    for (const auto &var : vars) {
        auto varResult = compile_definstance_var(instance, var, moduleEntry);
        if (varResult.isStatus())
            return varResult.getStatus();
        instanceMemberNames.insert(varResult.getResult());
    }

    // then compile constructor
    status = compile_definstance_init(instance, instanceMemberNames, moduleEntry);
    if (!status.isOk())
        return status;

    // then compile methods
    for (const auto &def : defs) {
        status = compile_definstance_def(instance, def, moduleEntry);
        if (!status.isOk())
            return status;
    }

    // compile impls last
    for (const auto &impl : impls) {
        status = compile_definstance_impl(instance, impl, moduleEntry);
        if (!status.isOk())
            return status;
    }

    return CompilerStatus::ok();
}
