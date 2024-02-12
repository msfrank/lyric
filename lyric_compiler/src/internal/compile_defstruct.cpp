#include <absl/strings/match.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/synthetic_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_call.h>
#include <lyric_compiler/internal/compile_defstruct.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>
#include <tempo_utils/log_stream.h>

static tempo_utils::Result<std::string>
compile_defstruct_val(
    lyric_assembler::StructSymbol *structSymbol,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (structSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *structBlock = structSymbol->structBlock();
    auto *state = structBlock->blockState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // get val name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get val type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(structBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto valType = parseAssignableResult.getResult();

    auto resolveValTypeResult = structBlock->resolveAssignable(valType);
    if (resolveValTypeResult.isStatus())
        return resolveValTypeResult.getStatus();

    // verify that the val type derives from either Intrinsic or Record
    auto *fundamentalCache = state->fundamentalCache();
    auto IntrinsicOrRecord = lyric_common::TypeDef::forUnion({
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic),
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record),
    });
    auto compareResult = typeSystem->compareAssignable(IntrinsicOrRecord, resolveValTypeResult.getResult());
    if (compareResult.isStatus())
        return compareResult.getStatus();
    switch (compareResult.getResult()) {
        case lyric_runtime::TypeComparison::EQUAL:
        case lyric_runtime::TypeComparison::EXTENDS:
            break;
        default:
            return structBlock->logAndContinue(walker,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "struct member {} must derive from Data", identifier);
    }

    if (absl::StartsWith(identifier, "__")) {
        return structBlock->logAndContinue(walker,
            lyric_compiler::CompilerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "private member {} not allowed for struct", identifier);
    } else if (absl::StartsWith(identifier, "_")) {
        return structBlock->logAndContinue(walker,
            lyric_compiler::CompilerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "protected member {} not allowed for struct", identifier);
    }

    // compile the member initializer if specified
    lyric_common::SymbolUrl init;
    if (walker.numChildren() > 0) {
        auto defaultInit = walker.getChild(0);
        auto compileDefaultResult = lyric_compiler::internal::compile_default_initializer(structSymbol->structBlock(),
            identifier, valType, defaultInit, moduleEntry);
        if (compileDefaultResult.isStatus())
            return compileDefaultResult.getStatus();
        init = compileDefaultResult.getResult();
    }

    auto declareMemberResult = structSymbol->declareMember( identifier, valType, init);
    if (declareMemberResult.isStatus())
        return declareMemberResult.getStatus();
    TU_LOG_INFO << "declared val member " << identifier << " for " << structSymbol->getSymbolUrl();

    return identifier;
}

static tempo_utils::Status
compile_defstruct_init(
    lyric_assembler::StructSymbol *structSymbol,
    const lyric_parser::NodeWalker &initPack,
    const lyric_parser::NodeWalker &initSuper,
    const lyric_parser::NodeWalker &initBody,
    const absl::flat_hash_set<std::string> &structMemberNames,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (structSymbol != nullptr);
    auto *structBlock = structSymbol->structBlock();
    auto *state = structBlock->blockState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    lyric_assembler::PackSpec packSpec;
    absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;

    // compile the given parameter list if specified, otherwise synthesize the parameter list
    if (initPack.isValid()) {
        auto parsePackResult = typeSystem->parsePack(structBlock, initPack);
        if (parsePackResult.isStatus())
            return parsePackResult.getStatus();
        packSpec = parsePackResult.getResult();
        for (const auto &p : packSpec.parameterSpec) {
            if (!p.init.isEmpty()) {
                auto compileInitializerResult = lyric_compiler::internal::compile_default_initializer(structBlock,
                    p.name, p.type, p.init.getValue(), moduleEntry);
                if (compileInitializerResult.isStatus())
                    return compileInitializerResult.getStatus();
                initializers[p.name] = compileInitializerResult.getResult();
            }
        }
    } else {
        for (const auto &memberName : structMemberNames) {
            auto maybeBinding = structSymbol->getMember(memberName);
            if (maybeBinding.isEmpty())
                structBlock->throwAssemblerInvariant("missing struct member {}", memberName);
            auto fieldVar = maybeBinding.getValue();
            if (!state->symbolCache()->hasSymbol(fieldVar.symbol))
                structBlock->throwAssemblerInvariant("missing struct field {}", fieldVar.symbol.toString());
            auto *sym = state->symbolCache()->getSymbol(fieldVar.symbol);
            if (sym->getSymbolType() != lyric_assembler::SymbolType::FIELD)
                structBlock->throwAssemblerInvariant("invalid struct field {}", fieldVar.symbol.toString());
            auto *fieldSymbol = cast_symbol_to_field(sym);
            auto fieldInitializerUrl = fieldSymbol->getInitializer();

            lyric_assembler::ParameterSpec p;
            p.name = memberName;
            p.label = memberName;
            p.binding = lyric_parser::BindingType::VALUE;
            p.type = lyric_parser::Assignable::fromTypeDef(fieldSymbol->getAssignableType());

            if (fieldInitializerUrl.isValid()) {
                if (!state->symbolCache()->hasSymbol(fieldInitializerUrl))
                    structBlock->throwAssemblerInvariant(
                        "missing field initializer {}", fieldInitializerUrl.toString());
                initializers[p.name] = fieldInitializerUrl;
                p.init = Option(lyric_parser::NodeWalker());
            }

            packSpec.parameterSpec.push_back(p);
        }
    }

    // declare the constructor
    auto declareCtorResult = structSymbol->declareCtor(
        packSpec.parameterSpec, packSpec.restSpec, lyric_object::AccessType::Public);
    if (declareCtorResult.isStatus())
        return declareCtorResult.getStatus();
    auto ctorUrl = declareCtorResult.getResult();
    auto *ctorSym = state->symbolCache()->getSymbol(ctorUrl);
    if (ctorSym == nullptr)
        structBlock->throwAssemblerInvariant("missing call symbol {}", ctorUrl.toString());
    if (ctorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        structBlock->throwAssemblerInvariant("invalid call symbol {}", ctorUrl.toString());
    auto *ctor = cast_symbol_to_call(ctorSym);
    for (const auto &entry : initializers) {
        ctor->putInitializer(entry.first, entry.second);
    }

    auto *proc = ctor->callProc();
    auto *code = proc->procCode();
    auto *ctorBlock = proc->procBlock();

    // find the superstruct ctor
    auto resolveSuperCtorResult = structSymbol->superStruct()->resolveCtor();
    if (resolveSuperCtorResult.isStatus())
        return resolveSuperCtorResult.getStatus();
    auto superCtor = resolveSuperCtorResult.getResult();

    lyric_typing::CallsiteReifier reifier(superCtor.getParameters(), superCtor.getRest(),
        superCtor.getTemplateUrl(), superCtor.getTemplateParameters(),
        superCtor.getTemplateArguments(), typeSystem);

    tempo_utils::Status status;

    // load the uninitialized instance onto the top of the stack
    status = code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
    if (!status.isOk())
        return status;

    // if super call is present, then apply the remainder of the parameters
    if (initSuper.isValid()) {
        status = lyric_compiler::internal::compile_placement(ctorBlock, ctorBlock, superCtor,
            reifier, initSuper, moduleEntry);
        if (!status.isOk())
            return status;
    }

    // call super ctor, instance is now on the top of the stack
    auto invokeSuperCtorResult = superCtor.invoke(ctorBlock, reifier);
    if (invokeSuperCtorResult.isStatus())
        return invokeSuperCtorResult.getStatus();

    // compile the constructor body if it exists, otherwise synthesize the body
    if (initBody.isValid()) {
        auto result = lyric_compiler::internal::compile_block(ctorBlock, initBody, moduleEntry);
        if (result.isStatus())
            return result.getStatus();
    } else {
        for (const auto &memberName : structMemberNames) {

            // resolve argument binding
            auto resolveBindingResult = ctorBlock->resolveBinding(memberName);
            if (resolveBindingResult.isStatus())
                return resolveBindingResult.getStatus();
            auto argVar = resolveBindingResult.getResult();

            // resolve the member binding
            auto maybeBinding = structSymbol->getMember(memberName);
            if (maybeBinding.isEmpty())
                structBlock->throwAssemblerInvariant("missing struct member {}", memberName);
            auto fieldVar = maybeBinding.getValue();

            // load argument value
            status = ctorBlock->load(argVar);
            if (!status.isOk())
                return status;

            // store default value in struct field
            status = ctorBlock->store(fieldVar);
            if (!status.isOk())
                return status;

            // mark member as initialized
            status = structSymbol->setMemberInitialized(memberName);
            if (!status.isOk())
                return status;
        }
    }

    // if any members are uninitialized, then try to default-initialize them
    for (const auto &memberName : structMemberNames) {

        // skip members which have been initialized already
        if (structSymbol->isMemberInitialized(memberName))
            continue;

        // resolve the member binding
        auto maybeBinding = structSymbol->getMember(memberName);
        if (maybeBinding.isEmpty())
            structBlock->throwAssemblerInvariant("missing struct member {}", memberName);
        auto fieldVar = maybeBinding.getValue();
        if (!state->symbolCache()->hasSymbol(fieldVar.symbol))
            structBlock->throwAssemblerInvariant("missing struct field {}", fieldVar.symbol.toString());
        auto *sym = state->symbolCache()->getSymbol(fieldVar.symbol);
        if (sym->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            structBlock->throwAssemblerInvariant("invalid struct field {}", fieldVar.symbol.toString());
        auto *fieldSymbol = cast_symbol_to_field(sym);
        auto fieldInitializerUrl = fieldSymbol->getInitializer();
        if (!fieldInitializerUrl.isValid())
            structBlock->throwAssemblerInvariant("missing field initializer {}", fieldInitializerUrl.toString());
        if (!state->symbolCache()->hasSymbol(fieldInitializerUrl))
            structBlock->throwAssemblerInvariant("missing field initializer {}", fieldInitializerUrl.toString());
        sym = state->symbolCache()->getSymbol(fieldInitializerUrl);
        if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            structBlock->throwAssemblerInvariant("invalid field initializer {}", fieldInitializerUrl.toString());
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

        // store default value in struct field
        status = ctorBlock->store(fieldVar);
        if (!status.isOk())
            return status;

        // mark member as initialized
        status = structSymbol->setMemberInitialized(memberName);
        if (!status.isOk())
            return status;
    }

    TU_LOG_INFO << "declared ctor " << ctorUrl << " for " << structSymbol->getSymbolUrl();

    // add return instruction
    status = code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    if (!structSymbol->isCompletelyInitialized())
        structBlock->throwAssemblerInvariant("struct {} is not completely initialized",
            structSymbol->getSymbolUrl().toString());

    return status;
}

static tempo_utils::Status
compile_defstruct_def(
    lyric_assembler::StructSymbol *structSymbol,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (structSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *structBlock = structSymbol->structBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    if (walker.numChildren() < 2)
        structBlock->throwSyntaxError(walker, "invalid def");

    // get method name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get method return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseAssignableResult = typeSystem->parseAssignable(structBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto returnSpec = parseAssignableResult.getResult();

    // compile the parameter list
    auto pack = walker.getChild(0);
    auto parsePackResult = typeSystem->parsePack(structBlock, pack);
    if (parsePackResult.isStatus())
        return parsePackResult.getStatus();
    auto packSpec = parsePackResult.getResult();

    // declare the method
    auto declareMethodResult = structSymbol->declareMethod(
        identifier, packSpec.parameterSpec,
        packSpec.restSpec, packSpec.ctxSpec, returnSpec);
    if (declareMethodResult.isStatus())
        return declareMethodResult.getStatus();
    auto methodUrl = declareMethodResult.getResult();
    auto *sym = structBlock->blockState()->symbolCache()->getSymbol(methodUrl);
    if (sym == nullptr)
        structBlock->throwAssemblerInvariant("missing call symbol {}", methodUrl.toString());
    if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        structBlock->throwAssemblerInvariant("invalid call symbol {}", methodUrl.toString());
    auto *call = cast_symbol_to_call(sym);

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
        return structBlock->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "def body does not match return type {}", call->getReturnType().toString());
    for (const auto &exitType : call->listExitTypes()) {
        if (!typeSystem->isAssignable(call->getReturnType(), exitType))
            return structBlock->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "def body does not match return type {}", call->getReturnType().toString());
    }

    // return control to the caller
    return lyric_compiler::CompilerStatus::ok();
}

tempo_utils::Status
lyric_compiler::internal::compile_defstruct(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // get struct name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_parser::NodeWalker init;
    std::vector<lyric_parser::NodeWalker> vals;
    std::vector<lyric_parser::NodeWalker> defs;

    // make initial pass over struct body
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);

        switch (childId) {
            case lyric_schema::LyricAstId::Init:
                if (init.isValid())
                    block->throwSyntaxError(child, "invalid init");
                init = child;
                break;
            case lyric_schema::LyricAstId::Val:
                vals.emplace_back(child);
                break;
            case lyric_schema::LyricAstId::Def:
                defs.emplace_back(child);
                break;
            default:
                block->throwSyntaxError(child, "expected struct body");
        }
    }

    lyric_parser::NodeWalker initPack;
    lyric_parser::NodeWalker initSuper;
    lyric_parser::NodeWalker initBody;
    lyric_common::SymbolUrl superStructUrl = state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Record);

    // if init was specified then process it
    if (init.isValid()) {
        moduleEntry.checkClassAndChildRangeOrThrow(init, lyric_schema::kLyricAstInitClass, 1, 3);

        initPack = init.getChild(0);
        moduleEntry.checkClassOrThrow(initPack, lyric_schema::kLyricAstPackClass);

        if (init.numChildren() == 2) {
            auto child = init.getChild(1);
            lyric_schema::LyricAstId childId{};
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
            superStructUrl = resolveSuperClassResult.getResult();
        }
    }

    //
    auto *superStructSym = state->symbolCache()->getSymbol(superStructUrl);
    if (superStructSym == nullptr)
        block->throwAssemblerInvariant("missing struct symbol {}", superStructUrl.toString());
    if (superStructSym->getSymbolType() != lyric_assembler::SymbolType::STRUCT)
        block->throwAssemblerInvariant("invalid struct symbol {}", superStructUrl.toString());
    auto *superStruct = cast_symbol_to_struct(superStructSym);

    auto declStructResult = block->declareStruct(
        identifier, superStruct, lyric_object::AccessType::Public);
    if (declStructResult.isStatus())
        return declStructResult.getStatus();
    auto structUrl = declStructResult.getResult();

    auto *sym = state->symbolCache()->getSymbol(structUrl);
    if (sym == nullptr)
        block->throwAssemblerInvariant("missing struct symbol {}", structUrl.toString());
    if (sym->getSymbolType() != lyric_assembler::SymbolType::STRUCT)
        block->throwAssemblerInvariant("invalid struct symbol {}", structUrl.toString());
    auto *structSymbol = cast_symbol_to_struct(sym);

    TU_LOG_INFO << "declared struct " << identifier << " with url " << structUrl;

    tempo_utils::Status status;
    absl::flat_hash_set<std::string> structMemberNames;

    // compile members first
    for (const auto &val : vals) {
        auto compileValResult = compile_defstruct_val(structSymbol, val, moduleEntry);
        if (compileValResult.isStatus())
            return compileValResult.getStatus();
        structMemberNames.insert(compileValResult.getResult());
    }

    // then compile constructor
    status = compile_defstruct_init(structSymbol, initPack, initSuper, initBody, structMemberNames, moduleEntry);
    if (!status.isOk())
        return status;

    // then compile methods
    for (const auto &def : defs) {
        status = compile_defstruct_def(structSymbol, def, moduleEntry);
        if (!status.isOk())
            return status;
    }

    return CompilerStatus::ok();
}
