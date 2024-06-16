#include <absl/strings/match.h>

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/synthetic_symbol.h>
#include <lyric_compiler/internal/compile_block.h>
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
    lyric_parser::Assignable valSpec;
    TU_ASSIGN_OR_RETURN (valSpec, typeSystem->parseAssignable(classBlock, type));
    lyric_common::TypeDef valType;
    TU_ASSIGN_OR_RETURN (valType, typeSystem->resolveAssignable(classBlock, valSpec));

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
        TU_ASSIGN_OR_RETURN (init, lyric_compiler::internal::compile_member_initializer(
            classSymbol->classBlock(), defaultInit, identifier, valType, templateParameters, moduleEntry));
    }

    TU_RETURN_IF_STATUS (classSymbol->declareMember(identifier, valType, false, accessType, init));
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
    lyric_parser::Assignable varSpec;
    TU_ASSIGN_OR_RETURN (varSpec, typeSystem->parseAssignable(classBlock, type));
    lyric_common::TypeDef varType;
    TU_ASSIGN_OR_RETURN (varType, typeSystem->resolveAssignable(classBlock, varSpec));

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
        TU_ASSIGN_OR_RETURN (init, lyric_compiler::internal::compile_member_initializer(
            classSymbol->classBlock(), defaultInit, identifier, varType, templateParameters, moduleEntry));
    }

    TU_RETURN_IF_STATUS (classSymbol->declareMember(identifier, varType,
        true, accessType, init));
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

    // declare the constructor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, classSymbol->declareCtor(lyric_object::AccessType::Public));

    lyric_typing::PackSpec packSpec;
    lyric_assembler::ParameterPack parameterPack;

    // compile the parameter list if specified, otherwise default to an empty list
    if (initPack.isValid()) {
        TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(classBlock, initPack));
        TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(classBlock, packSpec));
    }

    // compile list parameter initializers
    for (const auto &p : parameterPack.listParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                classBlock, entry->second, p, templateParameters, moduleEntry));
            ctorSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    // compile named parameter initializers
    for (const auto &p : parameterPack.namedParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                classBlock, entry->second, p, templateParameters, moduleEntry));
            ctorSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, ctorSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));

    auto *code = procHandle->procCode();
    auto *ctorBlock = procHandle->procBlock();

    // find the superclass ctor
    lyric_assembler::ConstructableInvoker superCtor;
    TU_RETURN_IF_NOT_OK (classSymbol->superClass()->prepareCtor(superCtor));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(superCtor));

    // load the uninitialized class onto the top of the stack
    TU_RETURN_IF_NOT_OK (code->loadSynthetic(lyric_assembler::SyntheticType::THIS));

    // if super statement is present, then place arguments
    if (initSuper.isValid()) {
        TU_RETURN_IF_NOT_OK (lyric_compiler::internal::compile_placement(
            superCtor.getConstructable(), ctorBlock, ctorBlock, reifier, initSuper, moduleEntry));
    }

    // call super ctor
    TU_RETURN_IF_STATUS (superCtor.invoke(ctorBlock, reifier, 0));

    // compile the constructor body if it exists
    if (initBody.isValid()) {
        TU_RETURN_IF_STATUS (lyric_compiler::internal::compile_block(ctorBlock, initBody, moduleEntry));
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
        TU_RETURN_IF_NOT_OK (code->loadSynthetic(lyric_assembler::SyntheticType::THIS));

        // invoke initializer to place default value onto the top of the stack
        auto callable = std::make_unique<lyric_assembler::FunctionCallable>(initializerCall);
        lyric_assembler::CallableInvoker invoker;
        TU_RETURN_IF_NOT_OK (invoker.initialize(std::move(callable)));
        lyric_typing::CallsiteReifier initializerReifier(typeSystem);
        TU_RETURN_IF_NOT_OK (initializerReifier.initialize(invoker));
        TU_RETURN_IF_STATUS (invoker.invoke(ctorBlock, initializerReifier));

        // store default value in class field
        TU_RETURN_IF_NOT_OK (ctorBlock->store(fieldRef));

        // mark member as initialized
        TU_RETURN_IF_NOT_OK (classSymbol->setMemberInitialized(memberName));
    }

    TU_LOG_INFO << "declared ctor " << ctorSymbol->getSymbolUrl() << " for " << classSymbol->getSymbolUrl();

    // add return instruction
    TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_RETURN));

    if (!classSymbol->isCompletelyInitialized())
        classBlock->throwAssemblerInvariant("class {} is not completely initialized",
            classSymbol->getSymbolUrl().toString());

    return {};
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

    // determine the access level
    lyric_object::AccessType access = lyric_object::AccessType::Public;
    if (absl::StartsWith(identifier, "__")) {
        access = lyric_object::AccessType::Private;
    } else if (absl::StartsWith(identifier, "_")) {
        access = lyric_object::AccessType::Protected;
    }

    // parse the return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    lyric_parser::Assignable returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(classBlock, type));

    // parse the parameter list
    auto pack = walker.getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(classBlock, pack));

    // declare the method
    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, classSymbol->declareMethod(identifier, access));

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
                classBlock, entry->second, p, templateParameters, moduleEntry));
            callSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    // compile named parameter initializers
    for (const auto &p : parameterPack.namedParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                classBlock, entry->second, p, templateParameters, moduleEntry));
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
        return classBlock->logAndContinue(body,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", returnType.toString());

    // validate that each exit returns the expected type
    for (auto it = procHandle->exitTypesBegin(); it != procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, *it));
        if (!isReturnable)
            return classBlock->logAndContinue(body,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", returnType.toString());
    }

    // return control to the caller
    return {};
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
    auto *implBlock = implHandle->implBlock();
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 2);

    // get method name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // parse the return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    lyric_parser::Assignable returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(implBlock, type));

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

    // parse the impl type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    lyric_parser::Assignable implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(classBlock, type));

    // resolve the impl type
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(classBlock, implSpec));

    // declare the class impl
    lyric_assembler::ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, classSymbol->declareImpl(implType));

    // compile each impl def
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
        switch (childId) {
            case lyric_schema::LyricAstId::Def:
                TU_RETURN_IF_NOT_OK (compile_defclass_impl_def(implHandle, templateParameters, child, moduleEntry));
                break;
            default:
                classBlock->throwSyntaxError(child, "expected impl def");
        }
    }

    return {};
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
    lyric_typing::TemplateSpec templateSpec;
    if (walker.hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        tu_uint32 genericOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstGenericOffset, genericOffset);
        auto generic = walker.getNodeAtOffset(genericOffset);
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(block, generic));
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

    return {};
}
