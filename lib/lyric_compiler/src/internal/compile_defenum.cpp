
#include <absl/strings/match.h>

#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
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
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(enumBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto valType = parseAssignableResult.getResult();

    // determine the access type
    lyric_object::AccessType accessType = lyric_object::AccessType::Public;
    if (absl::StartsWith(identifier, "__")) {
        accessType = lyric_object::AccessType::Private;
    } else if (absl::StartsWith(identifier, "_")) {
        accessType = lyric_object::AccessType::Protected;
    }

    // compile the member initializer if specified
    lyric_common::SymbolUrl init;
    if (walker.hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        tu_uint32 defaultOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstDefaultOffset, defaultOffset);
        auto defaultInit = walker.getNodeAtOffset(defaultOffset);
        auto compileDefaultResult = lyric_compiler::internal::compile_default_initializer(
            baseEnum->enumBlock(), identifier, {}, valType, defaultInit, moduleEntry);
        if (compileDefaultResult.isStatus())
            return compileDefaultResult.getStatus();
        init = compileDefaultResult.getResult();
    }

    auto declareMemberResult = baseEnum->declareMember(identifier, valType,
        false, accessType, init);
    if (declareMemberResult.isStatus())
        return declareMemberResult.getStatus();

    TU_LOG_INFO << "declared val member " << identifier << " for " << baseEnum->getSymbolUrl();

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
    auto *state = moduleEntry.getState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    if (walker.numChildren() < 2)
        enumBlock->throwSyntaxError(walker, "invalid def");

    // get method name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get method return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(enumBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto returnSpec = parseAssignableResult.getResult();

    // compile the parameter list
    auto pack = walker.getChild(0);
    auto parsePackResult = typeSystem->parsePack(enumBlock, pack);
    if (parsePackResult.isStatus())
        return parsePackResult.getStatus();
    auto packSpec = parsePackResult.getResult();

    // compile initializers
    absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    for (const auto &p : packSpec.parameterSpec) {
        if (!p.init.isEmpty()) {
            auto compileInitializerResult = lyric_compiler::internal::compile_default_initializer(
                enumBlock, p.name, {}, p.type, p.init.getValue(), moduleEntry);
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
    auto declareMethodResult = baseEnum->declareMethod(identifier, packSpec.parameterSpec,
        packSpec.restSpec, packSpec.ctxSpec, returnSpec, accessType);
    if (declareMethodResult.isStatus())
        return declareMethodResult.getStatus();
    auto methodUrl = declareMethodResult.getResult();
    auto *sym = state->symbolCache()->getSymbol(methodUrl);
    if (sym == nullptr)
        enumBlock->throwAssemblerInvariant("missing call symbol {}", methodUrl.toString());
    if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        enumBlock->throwAssemblerInvariant("invalid call symbol {}", methodUrl.toString());
    auto *call = cast_symbol_to_call(sym);

    // add initializers to the call
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

    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), bodyType));
    if (!isReturnable)
        return enumBlock->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "def body does not match return type {}", call->getReturnType().toString());

    // validate that each exit returns the expected type
    for (const auto &exitType : call->listExitTypes()) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), exitType));
        if (!isReturnable)
            return enumBlock->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "def body does not match return type {}", call->getReturnType().toString());
    }

    // return control to the caller
    return lyric_compiler::CompilerStatus::ok();
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
    auto *state = moduleEntry.getState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    if (walker.numChildren() < 1)
        baseBlock->throwSyntaxError(walker, "invalid init");

    // compile the parameter list
    auto pack = walker.getChild(0);
    auto parsePackResult = typeSystem->parsePack(baseBlock, pack);
    if (parsePackResult.isStatus())
        return parsePackResult.getStatus();
    auto packSpec = parsePackResult.getResult();

    for (const auto &p : packSpec.parameterSpec) {
        if (!p.label.empty())
            baseBlock->throwSyntaxError(pack, "named parameters are not allowed for enum constructor");
        if (!p.init.isEmpty())
            baseBlock->throwSyntaxError(pack, "default initialization is not supported for enum constructor");
    }
    if (!packSpec.restSpec.isEmpty())
        baseBlock->throwSyntaxError(pack, "rest parameter is not allowed for enum constructor");
    if (!packSpec.ctxSpec.empty())
        baseBlock->throwSyntaxError(pack, "ctx parameter is not allowed for enum constructor");

    // declare the base constructor
    auto declareBaseCtor = baseEnum->declareCtor(packSpec.parameterSpec, lyric_object::AccessType::Public);
    if (declareBaseCtor.isStatus())
        return declareBaseCtor.getStatus();
    auto baseCtorUrl = declareBaseCtor.getResult();
    auto *baseCtorSym = state->symbolCache()->getSymbol(baseCtorUrl);
    if (baseCtorSym == nullptr)
        baseBlock->throwAssemblerInvariant("missing call symbol {}", baseCtorUrl.toString());
    if (baseCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        baseBlock->throwAssemblerInvariant("invalid call symbol {}", baseCtorUrl.toString());
    auto *baseCtor = cast_symbol_to_call(baseCtorSym);

    auto *proc = baseCtor->callProc();
    auto *code = proc->procCode();
    auto *ctorBlock = proc->procBlock();

    // find the superenum ctor
    auto resolveSuperCtorResult = baseEnum->superEnum()->resolveCtor();
    if (resolveSuperCtorResult.isStatus())
        return resolveSuperCtorResult.getStatus();
    auto superCtor = resolveSuperCtorResult.getResult();

    lyric_typing::CallsiteReifier reifier(superCtor.getParameters(), superCtor.getRest(), typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize());

    //
    auto status = code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
    if (status.notOk())
        return status;

    // call super ctor, enum is now on the top of the stack
    auto invokeSuperCtorResult = superCtor.invoke(ctorBlock, reifier);
    if (invokeSuperCtorResult.isStatus())
        return invokeSuperCtorResult.getStatus();

    // compile the constructor body if defined
    if (walker.numChildren() == 2) {
        auto body = walker.getChild(1);

        // create block for init and declare synthetic $this variable
        lyric_assembler::BlockHandle initBlock(proc, code, ctorBlock, ctorBlock->blockState());
        auto declareThisResult = initBlock.declareVariable("$this",
            baseEnum->getAssignableType(), lyric_parser::BindingType::VALUE);
        if (declareThisResult.isStatus())
            return declareThisResult.getStatus();
        auto thisRef = declareThisResult.getResult();

        // store instance in $this
        status = initBlock.store(thisRef);
        if (!status.isOk())
            return status;

        // compile body
        auto result = lyric_compiler::internal::compile_block(&initBlock, body, moduleEntry);
        if (result.isStatus())
            return result.getStatus();

        // discard return value of constructor body
        status = code->popValue();
        if (!status.isOk())
            return status;

        // load $this onto top of the stack
        status = initBlock.load(thisRef);
        if (!status.isOk())
            return status;
    }

    TU_LOG_INFO << "declared ctor " << baseCtorUrl << " for " << baseEnum->getSymbolUrl();

    // add return instruction
    status = code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    if (!baseEnum->isCompletelyInitialized())
        baseBlock->throwAssemblerInvariant("enum {} is not completely initialized",
            baseEnum->getSymbolUrl().toString());

    return status;
}

static tempo_utils::Status
compile_defenum_base_default_init(
    lyric_assembler::EnumSymbol *baseEnum,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (baseEnum != nullptr);
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();
    auto *baseBlock = baseEnum->enumBlock();

    // declare the default no-parameters constructor
    auto declareBaseCtor = baseEnum->declareCtor({}, lyric_object::AccessType::Public);
    if (declareBaseCtor.isStatus())
        return declareBaseCtor.getStatus();
    auto baseCtorUrl = declareBaseCtor.getResult();
    auto *baseCtorSym = state->symbolCache()->getSymbol(baseCtorUrl);
    if (baseCtorSym == nullptr)
        baseBlock->throwAssemblerInvariant("missing call symbol {}", baseCtorUrl.toString());
    if (baseCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        baseBlock->throwAssemblerInvariant("invalid call symbol {}", baseCtorUrl.toString());
    auto *baseCtor = cast_symbol_to_call(baseCtorSym);

    auto *proc = baseCtor->callProc();
    auto *code = proc->procCode();
    auto *ctorBlock = proc->procBlock();

    // find the superenum ctor
    auto resolveSuperCtorResult = baseEnum->superEnum()->resolveCtor();
    if (resolveSuperCtorResult.isStatus())
        return resolveSuperCtorResult.getStatus();
    auto superCtor = resolveSuperCtorResult.getResult();

    lyric_typing::CallsiteReifier reifier(superCtor.getParameters(), superCtor.getRest(), typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize());

    //
    auto status = code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
    if (status.notOk())
        return status;

    // call super ctor, enum is now on the top of the stack
    auto invokeSuperCtorResult = superCtor.invoke(ctorBlock, reifier);
    if (invokeSuperCtorResult.isStatus())
        return invokeSuperCtorResult.getStatus();

    // create block for init and declare synthetic $this variable
    lyric_assembler::BlockHandle initBlock(proc, code, ctorBlock, ctorBlock->blockState());
    auto declareThisResult = initBlock.declareVariable("$this",
        baseEnum->getAssignableType(), lyric_parser::BindingType::VALUE);
    if (declareThisResult.isStatus())
        return declareThisResult.getStatus();
    auto var = declareThisResult.getResult();

    // store enum in $this
    status = initBlock.store(var);
    if (!status.isOk())
        return status;

    // load $this onto top of the stack
    status = initBlock.load(var);
    if (!status.isOk())
        return status;

    TU_LOG_INFO << "declared ctor " << baseCtorUrl << " for " << baseEnum->getSymbolUrl();

    // add return instruction
    status = code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    if (!baseEnum->isCompletelyInitialized())
        baseBlock->throwAssemblerInvariant("enum {} is not completely initialized",
            baseEnum->getSymbolUrl().toString());

    return status;
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
    auto *state = moduleEntry.getState();
    auto *caseBlock = caseEnum->enumBlock();

    // declare the constructor
    auto declareCtorResult = caseEnum->declareCtor({}, lyric_object::AccessType::Public);
    if (declareCtorResult.isStatus())
        return declareCtorResult.getStatus();
    auto caseCtorUrl = declareCtorResult.getResult();
    auto *caseCtorSym = state->symbolCache()->getSymbol(caseCtorUrl);
    if (caseCtorSym == nullptr)
        caseBlock->throwAssemblerInvariant("missing call symbol {}", caseCtorUrl.toString());
    if (caseCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        caseBlock->throwAssemblerInvariant("invalid call symbol {}", caseCtorUrl.toString());
    auto *caseCtor= cast_symbol_to_call(caseCtorSym);

    auto *proc = caseCtor->callProc();
    auto *code = proc->procCode();
    auto *ctorBlock = proc->procBlock();

    // find the superenum ctor
    auto resolveSuperCtorResult = caseEnum->superEnum()->resolveCtor();
    if (resolveSuperCtorResult.isStatus())
        return resolveSuperCtorResult.getStatus();
    auto superCtor = resolveSuperCtorResult.getResult();

    lyric_typing::CallsiteReifier reifier(superCtor.getParameters(), superCtor.getRest(), typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize());

    tempo_utils::Status status;

    status = code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
    if (status.notOk())
        return status;

    // apply the remainder of the parameters
    status = lyric_compiler::internal::compile_placement(ctorBlock, ctorBlock, superCtor,
        reifier, walker, moduleEntry);
    if (!status.isOk())
        return status;

    // call super ctor, enum is now on the top of the stack
    auto invokeSuperCtorResult = superCtor.invoke(ctorBlock, reifier);
    if (invokeSuperCtorResult.isStatus())
        return invokeSuperCtorResult.getStatus();

    // create block for init and declare synthetic $this variable
    lyric_assembler::BlockHandle initBlock(proc, code, ctorBlock, ctorBlock->blockState());
    auto declareThisResult = initBlock.declareVariable("$this",
        caseEnum->getAssignableType(), lyric_parser::BindingType::VALUE);
    if (declareThisResult.isStatus())
        return declareThisResult.getStatus();
    auto var = declareThisResult.getResult();

    // store enum in $this
    status = initBlock.store(var);
    if (!status.isOk())
        return status;

    // load $this onto top of the stack
    status = initBlock.load(var);
    if (!status.isOk())
        return status;

    TU_LOG_INFO << "declared ctor " << caseCtorUrl << " for " << caseEnum->getSymbolUrl();

    // add return instruction
    status = code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    if (!caseEnum->isCompletelyInitialized())
        caseBlock->throwAssemblerInvariant("enum {} is not completely initialized",
            caseEnum->getSymbolUrl().toString());

    return status;
}

static tempo_utils::Status
compile_defenum_case(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::EnumSymbol *baseEnum,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstCaseClass, 1);

    // get case call
    auto caseCall = walker.getChild(0);
    moduleEntry.checkClassOrThrow(caseCall, lyric_schema::kLyricAstCallClass);

    // get case name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(caseCall, lyric_parser::kLyricAstIdentifier, identifier);

    auto declCaseResult = block->declareEnum(identifier, baseEnum,
        lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declCaseResult.isStatus())
        return declCaseResult.getStatus();
    auto enumCaseUrl = declCaseResult.getResult();

    auto *enumCaseSym = state->symbolCache()->getSymbol(enumCaseUrl);
    if (enumCaseSym == nullptr)
        block->throwAssemblerInvariant("missing enum symbol {}", enumCaseUrl.toString());
    if (enumCaseSym->getSymbolType() != lyric_assembler::SymbolType::ENUM)
        block->throwAssemblerInvariant("invalid enum symbol {}", enumCaseUrl.toString());
    auto *caseEnum = cast_symbol_to_enum(enumCaseSym);

    TU_LOG_INFO << "declared enum case " << identifier << " with url " << enumCaseUrl;

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
            auto compileInitializerResult = lyric_compiler::internal::compile_default_initializer(
                implBlock, p.name, {}, p.type, p.init.getValue(), moduleEntry);
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

    // add initializers to the call
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

    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), bodyType));
    if (!isReturnable)
        return implBlock->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", call->getReturnType().toString());

    // validate that each exit returns the expected type
    for (const auto &exitType : call->listExitTypes()) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), exitType));
        if (!isReturnable)
            return implBlock->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", call->getReturnType().toString());
    }

    // return control to the caller
    return lyric_compiler::CompilerStatus::ok();
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

    // get impl type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(enumBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto implSpec = parseAssignableResult.getResult();

    tempo_utils::Status status;

    // declare the enum impl
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, enumSymbol->declareImpl(implSpec));
    auto *implHandle = enumSymbol->getImpl(implType);

    // compile each impl def
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
        switch (childId) {
            case lyric_schema::LyricAstId::Def:
                status = compile_defenum_impl_def(implHandle, child, moduleEntry);
                break;
            default:
                enumBlock->throwSyntaxError(child, "expected impl def");
        }
        if (!status.isOk())
            return status;
    }

    return lyric_compiler::CompilerStatus::ok();
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
    auto *categorySym = state->symbolCache()->getSymbol(fundamentalCategory);
    if (categorySym == nullptr)
        block->throwAssemblerInvariant("missing enum symbol {}", fundamentalCategory.toString());
    if (categorySym->getSymbolType() != lyric_assembler::SymbolType::ENUM)
        block->throwAssemblerInvariant("invalid enum symbol {}", fundamentalCategory.toString());
    auto *categoryEnum = cast_symbol_to_enum(categorySym);

    // declare the base enum
    auto declareBaseEnum = block->declareEnum(identifier, categoryEnum,
        lyric_object::AccessType::Public, lyric_object::DeriveType::Sealed);
    if (declareBaseEnum.isStatus())
        return declareBaseEnum.getStatus();
    auto baseEnumUrl = declareBaseEnum.getResult();

    auto *baseEnumSym = state->symbolCache()->getSymbol(baseEnumUrl);
    if (baseEnumSym == nullptr)
        block->throwAssemblerInvariant("missing enum symbol {}", baseEnumUrl.toString());
    if (baseEnumSym->getSymbolType() != lyric_assembler::SymbolType::ENUM)
        block->throwAssemblerInvariant("invalid enum symbol {}", baseEnumUrl.toString());
    auto *baseEnum = cast_symbol_to_enum(baseEnumSym);

    TU_LOG_INFO << "declared base enum " << identifier << " with url " << baseEnumUrl;

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

    return CompilerStatus::ok();
}
