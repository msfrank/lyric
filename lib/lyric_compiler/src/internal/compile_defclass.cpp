#include <absl/strings/match.h>

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/synthetic_symbol.h>
#include <lyric_compiler/internal/compile_call.h>
#include <lyric_compiler/internal/compile_defclass.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>
#include <tempo_utils/log_stream.h>

static tempo_utils::Result<std::string>
compile_defclass_val(
    lyric_assembler::ClassSymbol *classSymbol,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (classSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *classBlock = classSymbol->classBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // get val name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get val type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(classBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto valType = parseAssignableResult.getResult();

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
        auto compileDefaultResult = lyric_compiler::internal::compile_default_initializer(
            classSymbol->classBlock(), identifier, templateParameters, valType, defaultInit, moduleEntry);
        if (compileDefaultResult.isStatus())
            return compileDefaultResult.getStatus();
        init = compileDefaultResult.getResult();
    }

    auto declareMemberResult = classSymbol->declareMember(identifier, valType,
        false, accessType, init);
    if (declareMemberResult.isStatus())
        return declareMemberResult.getStatus();
    TU_LOG_INFO << "declared val member " << identifier << " for " << classSymbol->getSymbolUrl();

    return identifier;
}

static tempo_utils::Result<std::string>
compile_defclass_var(
    lyric_assembler::ClassSymbol *classSymbol,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (classSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *classBlock = classSymbol->classBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // get var name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get var type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(classBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto varType = parseAssignableResult.getResult();

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
        auto compileDefaultResult = lyric_compiler::internal::compile_default_initializer(
            classSymbol->classBlock(), identifier, templateParameters, varType, defaultInit, moduleEntry);
        if (compileDefaultResult.isStatus())
            return compileDefaultResult.getStatus();
        init = compileDefaultResult.getResult();
    }

    auto declareMemberResult = classSymbol->declareMember(identifier, varType,
        true, accessType, init);
    if (declareMemberResult.isStatus())
        return declareMemberResult.getStatus();
    TU_LOG_INFO << "declared var member " << identifier << " for " << classSymbol->getSymbolUrl();

    return identifier;
}

static tempo_utils::Status
compile_defclass_init(
    lyric_assembler::ClassSymbol *classSymbol,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    const lyric_parser::NodeWalker &initPack,
    const lyric_parser::NodeWalker &initSuper,
    const lyric_parser::NodeWalker &initBody,
    const absl::flat_hash_set<std::string> &classMemberNames,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (classSymbol != nullptr);
    auto *classBlock = classSymbol->classBlock();
    auto *state = classBlock->blockState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    lyric_assembler::PackSpec packSpec;
    absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;

    // compile the parameter list if specified, otherwise default to an empty list
    if (initPack.isValid()) {
        auto parsePackResult = typeSystem->parsePack(classBlock, initPack);
        if (parsePackResult.isStatus())
            return parsePackResult.getStatus();
        packSpec = parsePackResult.getResult();
        for (const auto &p : packSpec.parameterSpec) {
            if (!p.init.isEmpty()) {
                auto compileInitializerResult = lyric_compiler::internal::compile_default_initializer(
                    classBlock, p.name, templateParameters, p.type, p.init.getValue(), moduleEntry);
                if (compileInitializerResult.isStatus())
                    return compileInitializerResult.getStatus();
                initializers[p.name] = compileInitializerResult.getResult();
            }
        }
    }

    // declare the constructor
    auto declareCtorResult = classSymbol->declareCtor(packSpec.parameterSpec, packSpec.restSpec,
        packSpec.ctxSpec, lyric_object::AccessType::Public);
    if (declareCtorResult.isStatus())
        return declareCtorResult.getStatus();
    auto ctorUrl = declareCtorResult.getResult();
    lyric_assembler::AbstractSymbol *ctorSym;
    TU_ASSIGN_OR_RETURN (ctorSym, state->symbolCache()->getOrImportSymbol(ctorUrl));
    if (ctorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        classBlock->throwAssemblerInvariant("invalid call symbol {}", ctorUrl.toString());
    auto *ctor = cast_symbol_to_call(ctorSym);

    // add initializers to the ctor
    for (const auto &entry : initializers) {
        ctor->putInitializer(entry.first, entry.second);
    }

    auto *proc = ctor->callProc();
    auto *code = proc->procCode();
    auto *ctorBlock = proc->procBlock();

    // find the superclass ctor
    auto resolveSuperCtorResult = classSymbol->superClass()->resolveCtor();
    if (resolveSuperCtorResult.isStatus())
        return resolveSuperCtorResult.getStatus();
    auto superCtor = resolveSuperCtorResult.getResult();

    lyric_typing::CallsiteReifier reifier(superCtor.getParameters(), superCtor.getRest(),
        superCtor.getTemplateUrl(), superCtor.getTemplateParameters(), {}, typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize());

    // load the uninitialized class onto the top of the stack
    tempo_utils::Status status = code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
    if (!status.isOk())
        return status;

    // if super call is present, then apply the remainder of the parameters
    if (initSuper.isValid()) {
        status = lyric_compiler::internal::compile_placement(ctorBlock, ctorBlock, superCtor,
            reifier, initSuper, moduleEntry);
        if (!status.isOk())
            return status;
    }

    // call super ctor, class is now on the top of the stack
    auto invokeSuperCtorResult = superCtor.invoke(ctorBlock, reifier);
    if (invokeSuperCtorResult.isStatus())
        return invokeSuperCtorResult.getStatus();

    // compile the constructor body if it exists
    if (initBody.isValid()) {
        auto result = lyric_compiler::internal::compile_block(ctorBlock, initBody, moduleEntry);
        if (result.isStatus())
            return result.getStatus();
    }

    // if any members are uninitialized, then try to default-initialize them
    for (const auto &memberName : classMemberNames) {

        // skip members which have been initialized already
        if (classSymbol->isMemberInitialized(memberName))
            continue;

        lyric_assembler::AbstractSymbol *symbol;

        // resolve the member binding
        auto maybeMember = classSymbol->getMember(memberName);
        if (maybeMember.isEmpty())
            classBlock->throwAssemblerInvariant("missing class member {}", memberName);
        auto fieldRef = maybeMember.getValue();
        TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(fieldRef.symbolUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            classBlock->throwAssemblerInvariant("invalid class field {}", fieldRef.symbolUrl.toString());
        auto *fieldSymbol = cast_symbol_to_field(symbol);
        auto fieldInitializerUrl = fieldSymbol->getInitializer();
        if (!fieldInitializerUrl.isValid())
            classBlock->throwAssemblerInvariant("missing field initializer {}", fieldInitializerUrl.toString());
        TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(fieldInitializerUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
            classBlock->throwAssemblerInvariant("invalid field initializer {}", fieldInitializerUrl.toString());
        auto *initializerCall = cast_symbol_to_call(symbol);

        // load $this onto the top of the stack
        status = code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        if (!status.isOk())
            return status;

        // invoke initializer to place default value onto the top of the stack
        lyric_assembler::CallInvoker initializerInvoker(initializerCall);
        lyric_typing::CallsiteReifier initializerReifier(initializerInvoker.getParameters(),
            initializerInvoker.getRest(), initializerInvoker.getTemplateUrl(),
            initializerInvoker.getTemplateParameters(), {}, typeSystem);
        TU_RETURN_IF_NOT_OK (initializerReifier.initialize());
        auto invokeInitializerResult = initializerInvoker.invoke(ctorBlock, initializerReifier);
        if (invokeInitializerResult.isStatus())
            return invokeInitializerResult.getStatus();

        // store default value in class field
        status = ctorBlock->store(fieldRef);
        if (!status.isOk())
            return status;

        // mark member as initialized
        status = classSymbol->setMemberInitialized(memberName);
        if (!status.isOk())
            return status;
    }

    TU_LOG_INFO << "declared ctor " << ctorUrl << " for " << classSymbol->getSymbolUrl();

    // add return instruction
    status = code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    if (!classSymbol->isCompletelyInitialized())
        classBlock->throwAssemblerInvariant("class {} is not completely initialized",
            classSymbol->getSymbolUrl().toString());

    return status;
}

static tempo_utils::Status
compile_defclass_def(
    lyric_assembler::ClassSymbol *classSymbol,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (classSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *classBlock = classSymbol->classBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    if (walker.numChildren() < 2)
        classBlock->throwSyntaxError(walker, "invalid def");

    // get method name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get method return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(classBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto returnSpec = parseAssignableResult.getResult();

    // compile the parameter list
    auto pack = walker.getChild(0);
    auto parsePackResult = typeSystem->parsePack(classBlock, pack);
    if (parsePackResult.isStatus())
        return parsePackResult.getStatus();
    auto packSpec = parsePackResult.getResult();

    // compile initializers
    absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    for (const auto &p : packSpec.parameterSpec) {
        if (!p.init.isEmpty()) {
            auto compileInitializerResult = lyric_compiler::internal::compile_default_initializer(
                classBlock, p.name, templateParameters, p.type, p.init.getValue(), moduleEntry);
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
    auto declareMethodResult = classSymbol->declareMethod(
        identifier, packSpec.parameterSpec,
        packSpec.restSpec, packSpec.ctxSpec, returnSpec, accessType);
    if (declareMethodResult.isStatus())
        return declareMethodResult.getStatus();
    auto methodUrl = declareMethodResult.getResult();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, classBlock->blockState()->symbolCache()->getOrImportSymbol(methodUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
        classBlock->throwAssemblerInvariant("invalid call symbol {}", methodUrl.toString());
    auto *call = cast_symbol_to_call(symbol);

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
        return classBlock->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", call->getReturnType().toString());

    // validate that each exit returns the expected type
    for (const auto &exitType : call->listExitTypes()) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), exitType));
        if (!isReturnable)
            return classBlock->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", call->getReturnType().toString());
    }

    // return control to the caller
    return lyric_compiler::CompilerStatus::ok();
}

static tempo_utils::Status
compile_defclass_impl_def(
    lyric_assembler::ImplHandle *implHandle,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
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
                implBlock, p.name, templateParameters, p.type, p.init.getValue(), moduleEntry);
            if (compileInitializerResult.isStatus())
                return compileInitializerResult.getStatus();
            initializers[p.name] = compileInitializerResult.getResult();
        }
    }

    // declare the impl extension
    lyric_assembler::ExtensionMethod extension;
    TU_ASSIGN_OR_RETURN (extension, implHandle->declareExtension(
        identifier, packSpec.parameterSpec, packSpec.restSpec, packSpec.ctxSpec, returnSpec));
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(extension.methodCall));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
        implBlock->throwAssemblerInvariant("invalid call symbol {}", extension.methodCall.toString());
    auto *call = cast_symbol_to_call(symbol);

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
compile_defclass_impl(
    lyric_assembler::ClassSymbol *classSymbol,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (classSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *classBlock = classSymbol->classBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // get impl type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(classBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto implSpec = parseAssignableResult.getResult();

    tempo_utils::Status status;

    // declare the class impl
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, classSymbol->declareImpl(implSpec));
    auto *implHandle = classSymbol->getImpl(implType);

    // compile each impl def
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
        switch (childId) {
            case lyric_schema::LyricAstId::Def:
                status = compile_defclass_impl_def(implHandle, templateParameters, child, moduleEntry);
                break;
            default:
                classBlock->throwSyntaxError(child, "expected impl def");
        }
        if (!status.isOk())
            return status;
    }

    return lyric_compiler::CompilerStatus::ok();
}

tempo_utils::Status
lyric_compiler::internal::compile_defclass(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry,
    lyric_assembler::ClassSymbol **classSymbolPtr)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassOrThrow(walker, lyric_schema::kLyricAstDefClassClass);

    // get class name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // if class is generic, then compile the template parameter list
    lyric_assembler::TemplateSpec templateSpec;
    if (walker.hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        tu_uint32 genericOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstGenericOffset, genericOffset);
        auto generic = walker.getNodeAtOffset(genericOffset);
        auto resolveTemplateResult = typeSystem->resolveTemplate(block, generic);
        if (resolveTemplateResult.isStatus())
            return resolveTemplateResult.getStatus();
        templateSpec = resolveTemplateResult.getResult();
    }

    lyric_parser::NodeWalker init;
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
            case lyric_schema::LyricAstId::Init:
                if (init.isValid())
                    block->throwSyntaxError(child, "duplicate init");
                init = child;
                break;
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
                block->throwSyntaxError(child, "expected class body");
        }
    }

    lyric_parser::NodeWalker initPack;
    lyric_parser::NodeWalker initSuper;
    lyric_parser::NodeWalker initBody;
    lyric_common::SymbolUrl superClassUrl = state->fundamentalCache()->getFundamentalUrl(
        lyric_assembler::FundamentalSymbol::Object);

    // if init was specified then process it
    if (init.isValid()) {
        moduleEntry.checkClassAndChildRangeOrThrow(init, lyric_schema::kLyricAstInitClass, 1, 3);

        initPack = init.getChild(0);
        moduleEntry.checkClassOrThrow(initPack, lyric_schema::kLyricAstPackClass);

        if (init.numChildren() == 2) {
            auto child = init.getChild(1);
            lyric_schema::LyricAstId childId;
            moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
            switch (childId) {
                case lyric_schema::LyricAstId::Super:
                    initSuper = child;
                    break;
                case lyric_schema::LyricAstId::Block:
                    initBody = child;
                    break;
                default:
                    block->throwSyntaxError(init, "invalid init");
            }
        } else if (init.numChildren() == 3) {
            initSuper = init.getChild(1);
            moduleEntry.checkClassOrThrow(initSuper, lyric_schema::kLyricAstSuperClass);
            initBody = init.getChild(2);
            moduleEntry.checkClassOrThrow(initBody, lyric_schema::kLyricAstBlockClass);
        }

        //
        if (initSuper.isValid()) {
            tu_uint32 typeOffset;
            moduleEntry.parseAttrOrThrow(initSuper, lyric_parser::kLyricAstTypeOffset, typeOffset);
            auto superType = initSuper.getNodeAtOffset(typeOffset);
            auto parseAssignableResult = typeSystem->parseAssignable(block, superType);
            if (parseAssignableResult.isStatus())
                return parseAssignableResult.getStatus();
            auto resolveSuperClassResult = block->resolveClass(parseAssignableResult.getResult());
            if (resolveSuperClassResult.isStatus())
                return resolveSuperClassResult.getStatus();
            superClassUrl = resolveSuperClassResult.getResult();
        }
    }

    //
    lyric_assembler::AbstractSymbol *superclassSym;
    TU_ASSIGN_OR_RETURN (superclassSym, block->blockState()->symbolCache()->getOrImportSymbol(superClassUrl));
    if (superclassSym->getSymbolType() != lyric_assembler::SymbolType::CLASS)
        block->throwAssemblerInvariant("invalid class symbol {}", superClassUrl.toString());
    auto *superClass = cast_symbol_to_class(superclassSym);

    auto declClassResult = block->declareClass(
        identifier, superClass, lyric_object::AccessType::Public,
        templateSpec.templateParameters);
    if (declClassResult.isStatus())
        return declClassResult.getStatus();
    auto classUrl = declClassResult.getResult();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, block->blockState()->symbolCache()->getOrImportSymbol(classUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CLASS)
        block->throwAssemblerInvariant("invalid class symbol {}", classUrl.toString());
    auto *classSymbol = cast_symbol_to_class(symbol);

    TU_LOG_INFO << "declared class " << identifier << " from " << superClassUrl << " with url " << classUrl;

    absl::flat_hash_set<std::string> classMemberNames;

    // compile members first
    for (const auto &val : vals) {
        std::string memberName;
        TU_ASSIGN_OR_RETURN (memberName, compile_defclass_val(
            classSymbol, templateSpec.templateParameters, val, moduleEntry));
        classMemberNames.insert(memberName);
    }
    for (const auto &var : vars) {
        std::string memberName;
        TU_ASSIGN_OR_RETURN (memberName, compile_defclass_var(
            classSymbol, templateSpec.templateParameters, var, moduleEntry));
        classMemberNames.insert(memberName);
    }

    // then compile constructor, which might be the default constructor if an init was not specified
    TU_RETURN_IF_NOT_OK (compile_defclass_init(
        classSymbol, templateSpec.templateParameters, initPack, initSuper, initBody, classMemberNames, moduleEntry));

    // then compile methods
    for (const auto &def : defs) {
        TU_RETURN_IF_NOT_OK (compile_defclass_def(classSymbol, templateSpec.templateParameters, def, moduleEntry));
    }

    // compile impls last
    for (const auto &impl : impls) {
        TU_RETURN_IF_NOT_OK (compile_defclass_impl(classSymbol, templateSpec.templateParameters, impl, moduleEntry));
    }

    if (classSymbolPtr) {
        *classSymbolPtr = classSymbol;
    }

    return CompilerStatus::ok();
}
