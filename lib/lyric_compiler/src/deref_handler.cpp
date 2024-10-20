
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/block_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/constant_utils.h>
#include <lyric_compiler/deref_handler.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>

lyric_compiler::DerefHandler::DerefHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect)
{
    m_deref.fragment = fragment;
    TU_ASSERT (m_deref.fragment != nullptr);
    m_deref.bindingBlock = block;
    TU_ASSERT (m_deref.bindingBlock != nullptr);
    m_deref.invokeBlock = block;
    TU_ASSERT (m_deref.invokeBlock != nullptr);
}

tempo_utils::Status
lyric_compiler::DerefHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_INFO << "before DerefHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    auto initial = std::make_unique<DerefInitial>(&m_deref, block, driver);
    ctx.appendChoice(std::move(initial));

    for (int i = 0; i < node->numChildren() - 1; i++) {
        auto next = std::make_unique<DerefNext>(&m_deref, block, driver);
        ctx.appendChoice(std::move(next));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::DerefHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_INFO << "after DerefHandler@" << this;

    if (m_isSideEffect) {
        auto *driver = getDriver();
        auto resultType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
        if (resultType.getType() != lyric_common::TypeDefType::NoReturn) {
            TU_RETURN_IF_NOT_OK (m_deref.fragment->popValue());
        }
    }

    return {};
}

lyric_compiler::DerefCall::DerefCall(
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *invokeBlock,
    std::unique_ptr<lyric_assembler::CallableInvoker> &&invoker,
    std::unique_ptr<lyric_typing::CallsiteReifier> &&reifier,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
    : BaseInvokableHandler(bindingBlock, invokeBlock, fragment, driver),
      m_invoker(std::move(invoker)),
      m_reifier(std::move(reifier))
{
    TU_ASSERT (m_invoker != nullptr);
    TU_ASSERT (m_reifier != nullptr);
}

tempo_utils::Status
lyric_compiler::DerefCall::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *fragment = getFragment();

    TU_RETURN_IF_NOT_OK (placeArguments(m_invoker->getCallable(), *m_reifier, fragment));

    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_invoker->invoke(getInvokeBlock(), *m_reifier, fragment));
    return driver->pushResult(returnType);
}

static tempo_utils::Status
invoke_call(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *invokeBlock,
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::CodeFragment *fragment,
    lyric_compiler::CompilerScanDriver *driver,
    lyric_compiler::DecideContext &ctx)
{
    auto *typeSystem = driver->getTypeSystem();

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    std::vector<lyric_common::TypeDef> callsiteArguments;

    // if type arguments are specified at the callsite then append them
    if (node->hasAttr(lyric_parser::kLyricAstTypeArgumentsOffset)) {
        lyric_parser::ArchetypeNode *typeArgumentsNode;
        node->parseAttr(lyric_parser::kLyricAstTypeArgumentsOffset, typeArgumentsNode);
        auto typeArgumentsWalker = typeArgumentsNode->getArchetypeNode();
        std::vector<lyric_typing::TypeSpec> typeArgumentsSpec;
        TU_ASSIGN_OR_RETURN (typeArgumentsSpec, typeSystem->parseTypeArguments(invokeBlock, typeArgumentsWalker));
        std::vector<lyric_common::TypeDef> typeArguments;
        TU_ASSIGN_OR_RETURN (callsiteArguments, typeSystem->resolveTypeArguments(invokeBlock, typeArgumentsSpec));
    }

    auto invoker = std::make_unique<lyric_assembler::CallableInvoker>();
    TU_RETURN_IF_NOT_OK (bindingBlock->prepareFunction(identifier, *invoker));

    auto reifier = std::make_unique<lyric_typing::CallsiteReifier>(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier->initialize(*invoker, callsiteArguments));

    auto call = std::make_unique<lyric_compiler::DerefCall>(
        bindingBlock, invokeBlock, std::move(invoker), std::move(reifier), fragment, driver);
    ctx.setGrouping(std::move(call));

    return {};
}

lyric_compiler::DerefInitial::DerefInitial(
    lyric_compiler::Deref *deref,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_deref(deref)
{
    TU_ASSERT (m_deref != nullptr);
}

tempo_utils::Status
lyric_compiler::DerefInitial::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fragment = m_deref->fragment;

    auto astId = resource->getId();
    switch (astId) {

        // deref constant
        case lyric_schema::LyricAstId::Nil:
            return constant_nil(m_deref->invokeBlock, fragment, driver);
        case lyric_schema::LyricAstId::Undef:
            return constant_undef(m_deref->invokeBlock, fragment, driver);
        case lyric_schema::LyricAstId::True:
            return constant_true(m_deref->invokeBlock, fragment, driver);
        case lyric_schema::LyricAstId::False:
            return constant_false(m_deref->invokeBlock, fragment, driver);
        case lyric_schema::LyricAstId::Integer:
            return constant_integer(node, m_deref->invokeBlock, fragment, driver);
        case lyric_schema::LyricAstId::Float:
            return constant_float(node, m_deref->invokeBlock, fragment, driver);
        case lyric_schema::LyricAstId::Char:
            return constant_char(node, m_deref->invokeBlock, fragment, driver);
        case lyric_schema::LyricAstId::String:
            return constant_string(node, m_deref->invokeBlock, fragment, driver);
        case lyric_schema::LyricAstId::Url:
            return constant_url(node, m_deref->invokeBlock, fragment, driver);
        case lyric_schema::LyricAstId::SymbolRef:
            return constant_symbol(node, m_deref->invokeBlock, fragment, driver);

        // deref receiver
        case lyric_schema::LyricAstId::This:
            m_deref->thisReceiver = true;
            return deref_this(m_deref->invokeBlock, fragment, driver);

        // deref name
        case lyric_schema::LyricAstId::Name:
            return deref_name(node, &m_deref->bindingBlock, fragment, driver);

        // invoke function
        case lyric_schema::LyricAstId::Call:
            return invoke_call(node, m_deref->invokeBlock, m_deref->bindingBlock, fragment, driver, ctx);

        // evaluate grouping
        case lyric_schema::LyricAstId::Block: {
            auto groupBlock = std::make_unique<lyric_assembler::BlockHandle>(
                block->blockProc(), block, block->blockState());
            auto group = std::make_unique<BlockHandler>(
                std::move(groupBlock), /* requiresResult= */ true, /* isSideEffect= */ false, fragment, driver);
            ctx.setGrouping(std::move(group));
            return {};
        }

        // invoke construction
        case lyric_schema::LyricAstId::New:
            //return invokeNew(node);

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid deref target node");
    }
}

lyric_compiler::DerefNext::DerefNext(
    lyric_compiler::Deref *deref,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_deref(deref)
{
    TU_ASSERT (m_deref != nullptr);
}

static tempo_utils::Status
invoke_method(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *invokeBlock,
    lyric_assembler::BlockHandle *bindingBlock,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver,
    lyric_assembler::CodeFragment *fragment,
    lyric_compiler::CompilerScanDriver *driver,
    lyric_compiler::DecideContext &ctx)
{
    auto *typeSystem = driver->getTypeSystem();
    auto *symbolCache = driver->getSymbolCache();

    // get the method name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // verify the receiver is valid
    if (receiverType.getType() != lyric_common::TypeDefType::Concrete)
        return invokeBlock->logAndContinue(lyric_compiler::CompilerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "invalid receiver type {}", receiverType.toString());
    auto receiverUrl = receiverType.getConcreteUrl();

    // extract the type arguments from the receiver type
    auto concreteArguments = receiverType.getConcreteArguments();
    std::vector<lyric_common::TypeDef> callsiteArguments(concreteArguments.begin(), concreteArguments.end());

    // if type arguments are specified at the callsite then use them over the receiver type arguments
    if (node->hasAttr(lyric_parser::kLyricAstTypeArgumentsOffset)) {
        lyric_parser::ArchetypeNode *typeArgumentsNode;
        node->parseAttr(lyric_parser::kLyricAstTypeArgumentsOffset, typeArgumentsNode);
        auto typeArgumentsWalker = typeArgumentsNode->getArchetypeNode();
        std::vector<lyric_typing::TypeSpec> typeArgumentsSpec;
        TU_ASSIGN_OR_RETURN (typeArgumentsSpec, typeSystem->parseTypeArguments(invokeBlock, typeArgumentsWalker));
        std::vector<lyric_common::TypeDef> typeArguments;
        TU_ASSIGN_OR_RETURN (callsiteArguments, typeSystem->resolveTypeArguments(invokeBlock, typeArgumentsSpec));
    }

    // get the symbol for the receiver
    lyric_assembler::AbstractSymbol *receiver;
    TU_ASSIGN_OR_RETURN (receiver, symbolCache->getOrImportSymbol(receiverUrl));

    auto invoker = std::make_unique<lyric_assembler::CallableInvoker>();

    // prepare method on receiver
    switch (receiver->getSymbolType()) {

        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(receiver);
            TU_RETURN_IF_NOT_OK (classSymbol->prepareMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = cast_symbol_to_concept(receiver);
            TU_RETURN_IF_NOT_OK (conceptSymbol->prepareAction(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(receiver);
            TU_RETURN_IF_NOT_OK (enumSymbol->prepareMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(receiver);
            TU_RETURN_IF_NOT_OK (existentialSymbol->prepareMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(receiver);
            TU_RETURN_IF_NOT_OK (instanceSymbol->prepareMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(receiver);
            TU_RETURN_IF_NOT_OK (structSymbol->prepareMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        default:
            return invokeBlock->logAndContinue(lyric_compiler::CompilerCondition::kInvalidSymbol,
                tempo_tracing::LogSeverity::kError,
                "invalid receiver symbol {}", receiverUrl.toString());
    }

    auto reifier = std::make_unique<lyric_typing::CallsiteReifier>(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier->initialize(*invoker, callsiteArguments));

    auto method = std::make_unique<lyric_compiler::DerefMethod>(
        receiverType, bindingBlock, invokeBlock, std::move(invoker), std::move(reifier), fragment, driver);
    ctx.setGrouping(std::move(method));

    return {};
}

tempo_utils::Status
lyric_compiler::DerefNext::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto *driver = getDriver();
    auto *fragment = m_deref->fragment;

    auto receiverType = driver->peekResult();

    auto thisReceiver = m_deref->thisReceiver;
    if (thisReceiver) {
        m_deref->thisReceiver = false;
    }

    auto astId = resource->getId();
    switch (astId) {

        case lyric_schema::LyricAstId::Name:
            return deref_member(node, m_deref->bindingBlock, fragment, receiverType, thisReceiver, driver);

            // apply function
        case lyric_schema::LyricAstId::Call:
            return invoke_method(node, m_deref->invokeBlock, m_deref->bindingBlock,
                receiverType, thisReceiver, fragment, driver, ctx);

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invlid deref target node");
    }
}

lyric_compiler::DerefMethod::DerefMethod(
    const lyric_common::TypeDef &receiverType,
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *invokeBlock,
    std::unique_ptr<lyric_assembler::CallableInvoker> &&invoker,
    std::unique_ptr<lyric_typing::CallsiteReifier> &&reifier,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
    : BaseInvokableHandler(bindingBlock, invokeBlock, fragment, driver),
      m_receiverType(receiverType),
      m_invoker(std::move(invoker)),
      m_reifier(std::move(reifier))
{
    TU_ASSERT (m_invoker != nullptr);
    TU_ASSERT (m_reifier != nullptr);
}

tempo_utils::Status
lyric_compiler::DerefMethod::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *fragment = getFragment();

    TU_RETURN_IF_NOT_OK (placeArguments(m_invoker->getCallable(), *m_reifier, fragment));

    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_invoker->invoke(getInvokeBlock(), *m_reifier, fragment));

    // pop the receiver
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // push the method return
    return driver->pushResult(returnType);
}
