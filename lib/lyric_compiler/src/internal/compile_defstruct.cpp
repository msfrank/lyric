#include <absl/strings/match.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/synthetic_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compiler_utils.h>
#include <lyric_compiler/internal/compile_block.h>
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

    // get val access level
    lyric_parser::AccessType access;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstAccessType, access);

    // get val type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec valSpec;
    TU_ASSIGN_OR_RETURN (valSpec, typeSystem->parseAssignable(structBlock, typeNode));
    lyric_common::TypeDef valType;
    TU_ASSIGN_OR_RETURN (valType, typeSystem->resolveAssignable(structBlock, valSpec));

    // verify that the val type derives from either Intrinsic or Record
    auto *fundamentalCache = state->fundamentalCache();
    auto IntrinsicOrRecord = lyric_common::TypeDef::forUnion({
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic),
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record),
    });
    auto compareResult = typeSystem->compareAssignable(IntrinsicOrRecord, valType);
    if (compareResult.isStatus())
        return compareResult.getStatus();
    switch (compareResult.getResult()) {
        case lyric_runtime::TypeComparison::EQUAL:
        case lyric_runtime::TypeComparison::EXTENDS:
            break;
        default:
            return moduleEntry.logAndContinue(walker.getLocation(),
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "struct member {} must derive from Data", identifier);
    }

    lyric_assembler::FieldSymbol *fieldSymbol;
    TU_ASSIGN_OR_RETURN (fieldSymbol, structSymbol->declareMember(
        identifier, valType, lyric_compiler::internal::convert_access_type(access)));

    TU_LOG_INFO << "declared val member " << identifier << " for " << structSymbol->getSymbolUrl();

    // compile the member initializer if specified
    if (walker.numChildren() > 0) {
        auto defaultInit = walker.getChild(0);
        TU_RETURN_IF_NOT_OK (lyric_compiler::internal::compile_member_initializer(
            fieldSymbol, defaultInit, moduleEntry));
    }

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

    // declare the constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, structSymbol->declareCtor(lyric_object::AccessType::Public));

    lyric_assembler::ParameterPack parameterPack;

    // compile the given parameter list if specified, otherwise synthesize the parameter list
    if (initPack.isValid()) {
        lyric_typing::PackSpec packSpec;
        TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(structBlock, initPack));
        TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(structBlock, packSpec));

        // compile list parameter initializers
        for (const auto &p : parameterPack.listParameters) {
            auto entry = packSpec.initializers.find(p.name);
            if (entry != packSpec.initializers.cend()) {
                lyric_common::SymbolUrl initializerUrl;
                TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                    structBlock, entry->second, p, {}, moduleEntry));
                ctorSymbol->putInitializer(p.name, initializerUrl);
            }
        }

        // compile named parameter initializers
        for (const auto &p : parameterPack.namedParameters) {
            auto entry = packSpec.initializers.find(p.name);
            if (entry != packSpec.initializers.cend()) {
                lyric_common::SymbolUrl initializerUrl;
                TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                    structBlock, entry->second, p, {}, moduleEntry));
                ctorSymbol->putInitializer(p.name, initializerUrl);
            }
        }
    } else {
        tu_uint8 currparam = 0;
        for (const auto &memberName : structMemberNames) {
            auto mayberMember = structSymbol->getMember(memberName);
            if (mayberMember.isEmpty())
                structBlock->throwAssemblerInvariant("missing struct member {}", memberName);
            auto fieldRef = mayberMember.getValue();
            lyric_assembler::AbstractSymbol *symbol;
            TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(fieldRef.symbolUrl));
            if (symbol->getSymbolType() != lyric_assembler::SymbolType::FIELD)
                structBlock->throwAssemblerInvariant("invalid struct field {}", fieldRef.symbolUrl.toString());
            auto *fieldSymbol = cast_symbol_to_field(symbol);
            auto fieldInitializerUrl = fieldSymbol->getInitializer();

            lyric_assembler::Parameter p;
            p.index = currparam++;
            p.name = memberName;
            p.label = memberName;
            p.isVariable = false;
            p.placement = lyric_object::PlacementType::Invalid;
            p.typeDef = fieldSymbol->getAssignableType();

            if (fieldInitializerUrl.isValid()) {
                if (!state->symbolCache()->hasSymbol(fieldInitializerUrl))
                    structBlock->throwAssemblerInvariant(
                        "missing field initializer {}", fieldInitializerUrl.toString());
                p.placement = lyric_object::PlacementType::NamedOpt;
                ctorSymbol->putInitializer(memberName, fieldInitializerUrl);
                parameterPack.namedParameters.push_back(p);
            } else {
                p.placement = lyric_object::PlacementType::Named;
                parameterPack.namedParameters.push_back(p);
            }

        }
    }

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, ctorSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));

    auto *code = procHandle->procCode();
    auto *ctorBlock = procHandle->procBlock();

    // find the superstruct ctor
    lyric_assembler::ConstructableInvoker superCtor;
    TU_RETURN_IF_NOT_OK (structSymbol->superStruct()->prepareCtor(superCtor));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(superCtor));

    // load the uninitialized struct onto the top of the stack
    TU_RETURN_IF_NOT_OK (code->loadSynthetic(lyric_assembler::SyntheticType::THIS));

    // if super call is present, then place arguments
    if (initSuper.isValid()) {
        TU_RETURN_IF_NOT_OK (lyric_compiler::internal::compile_placement(
            superCtor.getConstructable(), ctorBlock, ctorBlock, reifier, initSuper, moduleEntry));
    }

    // call super ctor
    TU_RETURN_IF_STATUS (superCtor.invoke(ctorBlock, reifier, 0));

    // compile the constructor body if it exists, otherwise synthesize the body
    if (initBody.isValid()) {
        TU_RETURN_IF_STATUS (lyric_compiler::internal::compile_block(ctorBlock, initBody, moduleEntry));
    } else {
        for (const auto &memberName : structMemberNames) {

            // resolve argument binding
            lyric_assembler::DataReference argVar;
            TU_ASSIGN_OR_RETURN (argVar, ctorBlock->resolveReference(memberName));

            // resolve the member binding
            auto maybeBinding = structSymbol->getMember(memberName);
            if (maybeBinding.isEmpty())
                structBlock->throwAssemblerInvariant("missing struct member {}", memberName);
            auto fieldVar = maybeBinding.getValue();

            // load argument value
            TU_RETURN_IF_NOT_OK (ctorBlock->load(argVar));

            // store default value in struct field
            TU_RETURN_IF_NOT_OK (ctorBlock->store(fieldVar));

            // mark member as initialized
            TU_RETURN_IF_NOT_OK (structSymbol->setMemberInitialized(memberName));
        }
    }

    // if any members are uninitialized, then try to default-initialize them
    for (const auto &memberName : structMemberNames) {

        // skip members which have been initialized already
        if (structSymbol->isMemberInitialized(memberName))
            continue;

        lyric_assembler::AbstractSymbol *symbol;

        // resolve the member binding
        auto maybeMember = structSymbol->getMember(memberName);
        if (maybeMember.isEmpty())
            structBlock->throwAssemblerInvariant("missing struct member {}", memberName);
        auto fieldRef = maybeMember.getValue();
        TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(fieldRef.symbolUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            structBlock->throwAssemblerInvariant("invalid struct field {}", fieldRef.symbolUrl.toString());
        auto *fieldSymbol = cast_symbol_to_field(symbol);
        auto fieldInitializerUrl = fieldSymbol->getInitializer();
        if (!fieldInitializerUrl.isValid())
            structBlock->throwAssemblerInvariant("missing field initializer {}", fieldInitializerUrl.toString());
        TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(fieldInitializerUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
            structBlock->throwAssemblerInvariant("invalid field initializer {}", fieldInitializerUrl.toString());
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

        // store default value in struct field
        TU_RETURN_IF_NOT_OK (ctorBlock->store(fieldRef));

        // mark member as initialized
        TU_RETURN_IF_NOT_OK (structSymbol->setMemberInitialized(memberName));
    }

    TU_LOG_INFO << "declared ctor " << ctorSymbol->getSymbolUrl() << " for " << structSymbol->getSymbolUrl();

    // add return instruction
    TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_RETURN));

    if (!structSymbol->isCompletelyInitialized())
        structBlock->throwAssemblerInvariant("struct {} is not completely initialized",
            structSymbol->getSymbolUrl().toString());

    return {};
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

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 2);

    // get method name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get method access level
    lyric_parser::AccessType access;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstAccessType, access);

    // parse the return type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(structBlock, typeNode));

    // parse the parameter list
    auto pack = walker.getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(structBlock, pack));

    // declare the method
    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, structSymbol->declareMethod(
        identifier, lyric_compiler::internal::convert_access_type(access)));

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
                structBlock, entry->second, p, {}, moduleEntry));
            callSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    // compile named parameter initializers
    for (const auto &p : parameterPack.namedParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                structBlock, entry->second, p, {}, moduleEntry));
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
            "def body does not match return type {}", returnType.toString());

    // validate that each exit returns the expected type
    for (auto it = procHandle->exitTypesBegin(); it != procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, *it));
        if (!isReturnable)
            return moduleEntry.logAndContinue(body.getLocation(),
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "def body does not match return type {}", returnType.toString());
    }

    // return control to the caller
    return lyric_compiler::CompilerStatus::ok();
}

static tempo_utils::Status
compile_defstruct_impl_def(
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
compile_defstruct_impl(
    lyric_assembler::StructSymbol *structSymbol,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (structSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *structBlock = structSymbol->structBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // parse the impl type
    lyric_parser::NodeWalker typeNode;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(structBlock, typeNode));

    // resolve the impl type
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(structBlock, implSpec));

    // declare the struct impl
    lyric_assembler::ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, structSymbol->declareImpl(implType));

    // compile each impl def
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
        switch (childId) {
            case lyric_schema::LyricAstId::Def:
                TU_RETURN_IF_NOT_OK (compile_defstruct_impl_def(implHandle, child, moduleEntry));
                break;
            default:
                structBlock->throwAssemblerInvariant("expected impl def");
        }
    }

    return {};
}

tempo_utils::Status
lyric_compiler::internal::compile_defstruct(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry,
    lyric_assembler::StructSymbol **structSymbolPtr)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();
    auto *typeSystem = moduleEntry.getTypeSystem();

    // get struct name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get struct access level
    lyric_parser::AccessType access;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstAccessType, access);

    lyric_parser::NodeWalker init;
    std::vector<lyric_parser::NodeWalker> vals;
    std::vector<lyric_parser::NodeWalker> defs;
    std::vector<lyric_parser::NodeWalker> impls;

    // make initial pass over struct body
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);

        switch (childId) {
            case lyric_schema::LyricAstId::Init:
                if (init.isValid())
                    block->throwAssemblerInvariant("invalid init");
                init = child;
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
                block->throwAssemblerInvariant("expected struct body");
        }
    }

    lyric_parser::NodeWalker initPack;
    lyric_parser::NodeWalker initSuper;
    lyric_parser::NodeWalker initBody;
    auto superStructType = state->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Record);

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
                    block->throwAssemblerInvariant("invalid init");
            }
        } else if (init.numChildren() == 3) {
            initSuper = init.getChild(1);
            moduleEntry.checkClassOrThrow(initSuper, lyric_schema::kLyricAstSuperClass);
            initBody = init.getChild(2);
            moduleEntry.checkClassOrThrow(initBody, lyric_schema::kLyricAstBlockClass);
        }

        //
        if (initSuper.isValid()) {
            lyric_parser::NodeWalker typeNode;
            moduleEntry.parseAttrOrThrow(initSuper, lyric_parser::kLyricAstTypeOffset, typeNode);
            lyric_typing::TypeSpec superSpec;
            TU_ASSIGN_OR_RETURN (superSpec, typeSystem->parseAssignable(block, typeNode));
            TU_ASSIGN_OR_RETURN (superStructType, typeSystem->resolveAssignable(block, superSpec));
        }
    }

    //
    lyric_assembler::StructSymbol *superStruct;
    TU_ASSIGN_OR_RETURN (superStruct, block->resolveStruct(superStructType));

    lyric_assembler::StructSymbol *structSymbol;
    TU_ASSIGN_OR_RETURN (structSymbol, block->declareStruct(
        identifier, superStruct, lyric_compiler::internal::convert_access_type(access)));

    TU_LOG_INFO << "declared struct " << structSymbol->getSymbolUrl() << " from " << superStruct->getSymbolUrl();

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

    // compile impls last
    for (const auto &impl : impls) {
        status = compile_defstruct_impl(structSymbol, impl, moduleEntry);
        if (!status.isOk())
            return status;
    }

    if (structSymbolPtr != nullptr) {
        *structSymbolPtr = structSymbol;
    }

    return CompilerStatus::ok();
}
