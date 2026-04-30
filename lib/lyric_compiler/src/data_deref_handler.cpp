
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/protocol_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/block_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/constant_utils.h>
#include <lyric_compiler/data_deref_handler.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/new_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>

static tempo_utils::Status
invoke_call(
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::DataDeref *dataDeref,
    lyric_compiler::CompilerScanDriver *driver,
    lyric_compiler::DecideContext &ctx)
{
    TU_NOTNULL (node);
    TU_NOTNULL (dataDeref);
    TU_NOTNULL (driver);
    auto *typeSystem = driver->getTypeSystem();

    if (!dataDeref->effects.empty())
        return lyric_compiler::CompilerStatus::forCondition(
            lyric_compiler::CompilerCondition::kCompilerInvariant,
            "invalid call deref; effects vector is not empty");

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    auto *bindingBlock = dataDeref->block;
    auto *invokeBlock = dataDeref->block;

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

    auto call = std::make_unique<lyric_compiler::DataDerefCall>(dataDeref, bindingBlock, invokeBlock,
        std::move(invoker), std::move(reifier), dataDeref->fragment, driver);
    ctx.setGrouping(std::move(call));

    return {};
}

static tempo_utils::Status
invoke_method(
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::DataDeref *dataDeref,
    lyric_compiler::CompilerScanDriver *driver,
    lyric_compiler::DecideContext &ctx)
{
    TU_NOTNULL (node);
    TU_NOTNULL (dataDeref);
    TU_NOTNULL (driver);

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    if (dataDeref->effects.empty())
        return lyric_compiler::CompilerStatus::forCondition(
            lyric_compiler::CompilerCondition::kCompilerInvariant,
            "invalid method deref; effects vector is empty");
    const auto &effect = dataDeref->effects.back();

    auto receiverType = effect.receiverType;
    switch (receiverType.getType()) {
        case lyric_common::TypeDefType::Concrete:
        case lyric_common::TypeDefType::Placeholder:
            break;
        default:
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kSyntaxError,
                "cannot invoke method '{}' on {}", identifier, receiverType.toString());
    }

    if (effect.pushResult == false)
        return lyric_compiler::CompilerStatus::forCondition(
            lyric_compiler::CompilerCondition::kCompilerInvariant,
            "invalid method deref; last effect did not push result");

    TU_NOTNULL (effect.derefSymbol);
    auto *bindingBlock = effect.derefSymbol->derefBlock();
    auto *invokeBlock = dataDeref->block;

    auto *typeSystem = driver->getTypeSystem();
    auto *symbolCache = driver->getSymbolCache();

    auto thisReceiver = current_ref_is_this_receiver(symbolCache, dataDeref->block, dataDeref->effects);

    // verify the receiver is valid
    if (receiverType.getType() != lyric_common::TypeDefType::Concrete)
        return lyric_compiler::CompilerStatus::forCondition(lyric_compiler::CompilerCondition::kSyntaxError,
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

        case lyric_assembler::SymbolType::PROTOCOL: {
            auto *protocolSymbol = cast_symbol_to_protocol(receiver);
            TU_RETURN_IF_NOT_OK (protocolSymbol->prepareMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(receiver);
            TU_RETURN_IF_NOT_OK (structSymbol->prepareMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        default:
            return lyric_compiler::CompilerStatus::forCondition(lyric_compiler::CompilerCondition::kInvalidSymbol,
                "invalid receiver symbol {}", receiverUrl.toString());
    }

    auto reifier = std::make_unique<lyric_typing::CallsiteReifier>(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier->initialize(*invoker, callsiteArguments));

    auto method = std::make_unique<lyric_compiler::DataDerefMethod>(dataDeref, bindingBlock, invokeBlock,
        std::move(invoker), std::move(reifier), dataDeref->fragment, driver);
    ctx.setGrouping(std::move(method));

    return {};
}

static tempo_utils::Status
invoke_global_method(
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::DataDeref *dataDeref,
    lyric_compiler::CompilerScanDriver *driver,
    lyric_compiler::DecideContext &ctx)
{
    TU_NOTNULL (node);
    TU_NOTNULL (dataDeref);
    TU_NOTNULL (driver);

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    if (dataDeref->effects.empty())
        return lyric_compiler::CompilerStatus::forCondition(
            lyric_compiler::CompilerCondition::kCompilerInvariant,
            "invalid global method deref; effects vector is empty");
    const auto &effect = dataDeref->effects.back();

    auto receiverType = effect.receiverType;
    switch (receiverType.getType()) {
        case lyric_common::TypeDefType::Concrete:
        case lyric_common::TypeDefType::Placeholder:
            break;
        default:
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kSyntaxError,
                "cannot invoke global method '{}' on {}", identifier, receiverType.toString());
    }

    if (effect.pushResult == true)
        return lyric_compiler::CompilerStatus::forCondition(
            lyric_compiler::CompilerCondition::kCompilerInvariant,
            "invalid global method deref; last effect pushed result");

    TU_NOTNULL (effect.derefSymbol);
    auto *derefSymbol = effect.derefSymbol;
    auto *bindingBlock = derefSymbol->derefBlock();
    auto *invokeBlock = dataDeref->block;

    auto *typeSystem = driver->getTypeSystem();
    auto *symbolCache = driver->getSymbolCache();

    auto thisReceiver = current_ref_is_this_receiver(symbolCache, dataDeref->block, dataDeref->effects);

    // verify the receiver is valid
    if (receiverType.getType() != lyric_common::TypeDefType::Concrete)
        return lyric_compiler::CompilerStatus::forCondition(lyric_compiler::CompilerCondition::kSyntaxError,
            "invalid receiver type {}", receiverType.toString());

    // extract the type arguments from the receiver type
    auto concreteArguments = receiverType.getConcreteArguments();
    std::vector callsiteArguments(concreteArguments.begin(), concreteArguments.end());

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

    auto invoker = std::make_unique<lyric_assembler::CallableInvoker>();

    // prepare method on receiver
    switch (derefSymbol->getSymbolType()) {

        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = lyric_assembler::cast_symbol_to_class(derefSymbol);
            TU_RETURN_IF_NOT_OK (classSymbol->prepareGlobalMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = lyric_assembler::cast_symbol_to_concept(derefSymbol);
            TU_RETURN_IF_NOT_OK (conceptSymbol->prepareGlobalMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = lyric_assembler::cast_symbol_to_enum(derefSymbol);
            TU_RETURN_IF_NOT_OK (enumSymbol->prepareGlobalMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = lyric_assembler::cast_symbol_to_instance(derefSymbol);
            TU_RETURN_IF_NOT_OK (instanceSymbol->prepareGlobalMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::NAMESPACE: {
            auto *namespaceSymbol = lyric_assembler::cast_symbol_to_namespace(derefSymbol);
            TU_RETURN_IF_NOT_OK (namespaceSymbol->prepareTargetMethod(identifier, *invoker, dataDeref->block));
            break;
        }

        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = lyric_assembler::cast_symbol_to_struct(derefSymbol);
            TU_RETURN_IF_NOT_OK (structSymbol->prepareGlobalMethod(identifier, receiverType, *invoker, thisReceiver));
            break;
        }

        default:
            return lyric_compiler::CompilerStatus::forCondition(lyric_compiler::CompilerCondition::kInvalidSymbol,
                "invalid receiver symbol {}", derefSymbol->getSymbolUrl().toString());
    }

    auto reifier = std::make_unique<lyric_typing::CallsiteReifier>(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier->initialize(*invoker, callsiteArguments));

    auto method = std::make_unique<lyric_compiler::DataDerefMethod>(dataDeref, bindingBlock, invokeBlock,
        std::move(invoker), std::move(reifier), dataDeref->fragment, driver);
    ctx.setGrouping(std::move(method));

    return {};
}

lyric_compiler::DerefThisBehavior::DerefThisBehavior(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : m_isSideEffect(isSideEffect),
      m_privateDeref(true),
      m_driver(driver)
{
    TU_NOTNULL (fragment);
    TU_NOTNULL (block);
    TU_NOTNULL (m_driver);
    m_deref = new DataDeref();
    m_deref->fragment = fragment;
    m_deref->block = block;
}

lyric_compiler::DerefThisBehavior::DerefThisBehavior(DataDeref *deref, CompilerScanDriver *driver)
    : m_isSideEffect(false),
      m_privateDeref(false),
      m_deref(deref),
      m_driver(driver)
{
    TU_NOTNULL (m_deref);
    TU_NOTNULL (m_driver);
}

lyric_compiler::DerefThisBehavior::~DerefThisBehavior()
{
    if (m_privateDeref) {
        delete m_deref;;
    }
}

tempo_utils::Status
lyric_compiler::DerefThisBehavior::enter(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    EnterContext &ctx)
{
    return deref_this(m_deref->effects, m_deref->block, m_deref->fragment, m_driver);
}

tempo_utils::Status
lyric_compiler::DerefThisBehavior::exit(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    ExitContext &ctx)
{
    const auto &effect = m_deref->effects.back();
    TU_ASSERT (effect.pushResult);

    if (m_isSideEffect) {
        auto resultType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
        TU_RETURN_IF_NOT_OK (m_deref->fragment->popValue());
    }

    return {};
}

lyric_compiler::DerefNameBehavior::DerefNameBehavior(DataDeref *deref, CompilerScanDriver *driver)
    : m_isSideEffect(false),
      m_privateDeref(false),
      m_deref(deref),
      m_driver(driver)
{
    TU_NOTNULL (m_deref);
    TU_NOTNULL (m_driver);
}

lyric_compiler::DerefNameBehavior::DerefNameBehavior(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : m_isSideEffect(isSideEffect),
      m_privateDeref(true),
      m_driver(driver)
{
    TU_NOTNULL (fragment);
    TU_NOTNULL (block);
    TU_NOTNULL (m_driver);
    m_deref = new DataDeref();
    m_deref->fragment = fragment;
    m_deref->block = block;
}
lyric_compiler::DerefNameBehavior::~DerefNameBehavior()
{
    if (m_privateDeref) {
        delete m_deref;
    }
}

tempo_utils::Status
lyric_compiler::DerefNameBehavior::enter(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    EnterContext &ctx)
{
    return deref_name(node, m_deref->effects, m_deref->block, m_deref->fragment, m_driver);
}

tempo_utils::Status
lyric_compiler::DerefNameBehavior::exit(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    ExitContext &ctx)
{
    const auto &effect = m_deref->effects.back();
    if (!effect.pushResult)
        return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
            "invalid deref target {}", effect.derefSymbol->getSymbolUrl().toString());

    if (m_isSideEffect) {
        auto resultType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
        TU_RETURN_IF_NOT_OK (m_deref->fragment->popValue());
    }

    return {};
}

lyric_compiler::DataDerefHandler::DataDerefHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect)
{
    m_deref.fragment = fragment;
    TU_ASSERT (m_deref.fragment != nullptr);
    m_deref.block = block;
    TU_ASSERT (m_deref.block != nullptr);
}

tempo_utils::Status
lyric_compiler::DataDerefHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before DataDerefHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    auto numChildren = node->numChildren();
    if (numChildren == 0)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "empty deref statement");

    if (numChildren == 1) {
        auto single = std::make_unique<DataDerefSingle>(&m_deref, block, driver);
        ctx.appendChoice(std::move(single));
        return {};
    }

    auto first = std::make_unique<DataDerefFirst>(&m_deref, block, driver);
    ctx.appendChoice(std::move(first));

    for (int i = 1; i < node->numChildren() - 1; i++) {
        auto next = std::make_unique<DataDerefNext>(&m_deref, block, driver);
        ctx.appendChoice(std::move(next));
    }

    auto last = std::make_unique<DataDerefLast>(&m_deref, block, driver);
    ctx.appendChoice(std::move(last));

    return {};
}

tempo_utils::Status
lyric_compiler::DataDerefHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after DataDerefHandler@" << this;

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

lyric_compiler::DataDerefSingle::DataDerefSingle(
    DataDeref *deref,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_deref(deref)
{
    TU_ASSERT (m_deref != nullptr);
}

tempo_utils::Status
lyric_compiler::DataDerefSingle::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fragment = m_deref->fragment;

    auto astId = resource->getId();
    switch (astId) {

        // deref constant value
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Raw:
            return deref_literal(node, m_deref->effects, block, fragment, driver);

        // deref receiver
        case lyric_schema::LyricAstId::This: {
            auto handler = std::make_unique<DerefThisBehavior>(m_deref, driver);
            ctx.setBehavior(std::move(handler));
            return {};
        }

        // deref name
        case lyric_schema::LyricAstId::Name: {
            auto handler = std::make_unique<DerefNameBehavior>(m_deref, driver);
            ctx.setBehavior(std::move(handler));
            return {};
        }

        // invoke function
        case lyric_schema::LyricAstId::Call:
            return invoke_call(node, m_deref, driver, ctx);

        // evaluate grouping
        case lyric_schema::LyricAstId::Block: {
            auto handler = std::make_unique<DataDerefBlock>(m_deref, block, fragment, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }

        // construct instance
        case lyric_schema::LyricAstId::New: {
            auto handler = std::make_unique<DataDerefNew>(m_deref, block, fragment, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid deref target node");
    }
}

lyric_compiler::DataDerefFirst::DataDerefFirst(
    DataDeref *deref,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_deref(deref)
{
    TU_ASSERT (m_deref != nullptr);
}

tempo_utils::Status
lyric_compiler::DataDerefFirst::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
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
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Raw:
            return deref_literal(node, m_deref->effects, block, fragment, driver);

        // deref receiver
        case lyric_schema::LyricAstId::This:
            return deref_this(m_deref->effects, block, fragment, driver);

        // deref name
        case lyric_schema::LyricAstId::Name:
            return deref_name(node, m_deref->effects, block, fragment, driver);

        // invoke function
        case lyric_schema::LyricAstId::Call:
            return invoke_call(node, m_deref, driver, ctx);

        // evaluate grouping
        case lyric_schema::LyricAstId::Block: {
            auto handler = std::make_unique<DataDerefBlock>(m_deref, block, fragment, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }

        // construct instance
        case lyric_schema::LyricAstId::New: {
            auto handler = std::make_unique<DataDerefNew>(m_deref, block, fragment, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid first deref target");
    }
}

lyric_compiler::DataDerefNext::DataDerefNext(
    DataDeref *deref,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_deref(deref)
{
    TU_ASSERT (m_deref != nullptr);
}

tempo_utils::Status
lyric_compiler::DataDerefNext::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());
    auto astId = resource->getId();

    auto *driver = getDriver();
    auto *fragment = m_deref->fragment;

    const auto &effect = m_deref->effects.back();

    if (effect.pushResult) {
        switch (astId) {
            case lyric_schema::LyricAstId::Name:
                return deref_member(node, m_deref->effects, m_deref->block, fragment, driver);
            case lyric_schema::LyricAstId::Call:
                return invoke_method(node, m_deref, driver, ctx);
            default:
                return CompilerStatus::forCondition(
                    CompilerCondition::kCompilerInvariant, "invalid deref target node");
        }
    }

    switch (astId) {
        case lyric_schema::LyricAstId::Name:
            return deref_global_member(node, m_deref->effects, m_deref->block, fragment, driver);
        case lyric_schema::LyricAstId::Call:
            return invoke_global_method(node, m_deref, driver, ctx);
        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid deref target node");
    }
}

lyric_compiler::DataDerefLast::DataDerefLast(
    DataDeref *deref,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_deref(deref)
{
    TU_ASSERT (m_deref != nullptr);
}

tempo_utils::Status
lyric_compiler::DataDerefLast::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());
    auto astId = resource->getId();

    auto *driver = getDriver();
    auto *fragment = m_deref->fragment;

    const auto &effect = m_deref->effects.back();

    if (effect.pushResult) {
        switch (astId) {
            case lyric_schema::LyricAstId::Name:
                TU_RETURN_IF_NOT_OK (deref_member(node, m_deref->effects, m_deref->block, fragment, driver));
                break;
            case lyric_schema::LyricAstId::Call:
                TU_RETURN_IF_NOT_OK (invoke_method(node, m_deref, driver, ctx));
                break;
            default:
                return CompilerStatus::forCondition(
                    CompilerCondition::kCompilerInvariant, "invalid deref target node");
        }
    } else {
        switch (astId) {
            case lyric_schema::LyricAstId::Name:
                TU_RETURN_IF_NOT_OK (deref_global_member(node, m_deref->effects, m_deref->block, fragment, driver));
                break;
            case lyric_schema::LyricAstId::Call:
                TU_RETURN_IF_NOT_OK (invoke_global_method(node, m_deref, driver, ctx));
                break;
            default:
                return CompilerStatus::forCondition(
                    CompilerCondition::kCompilerInvariant, "invalid deref target node");
        }
    }

    return {};
}

lyric_compiler::DataDerefCall::DataDerefCall(
    DataDeref *deref,
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *invokeBlock,
    std::unique_ptr<lyric_assembler::CallableInvoker> &&invoker,
    std::unique_ptr<lyric_typing::CallsiteReifier> &&reifier,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
    : BaseInvokableHandler(bindingBlock, invokeBlock, fragment, driver),
      m_deref(deref),
      m_invoker(std::move(invoker)),
      m_reifier(std::move(reifier))
{
    TU_NOTNULL (m_deref);
    TU_NOTNULL (m_invoker);
    TU_NOTNULL (m_reifier);
}

tempo_utils::Status
lyric_compiler::DataDerefCall::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *symbolCache = driver->getSymbolCache();
    auto *fragment = getFragment();

    TU_RETURN_IF_NOT_OK (placeArguments(m_invoker->getCallable(), *m_reifier, fragment));

    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_invoker->invoke(getInvokeBlock(), *m_reifier, fragment));

    lyric_assembler::AbstractSymbol *derefSymbol;

    // store the deref symbol if return type is concrete
    switch (returnType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            derefSymbol = symbolCache->getSymbolOrNull(returnType.getConcreteUrl());
            break;
        case lyric_common::TypeDefType::Intersection:
        case lyric_common::TypeDefType::Union:
        case lyric_common::TypeDefType::Placeholder:
        case lyric_common::TypeDefType::NoReturn:
            derefSymbol = nullptr;
            break;
        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "unexpected return type {}", returnType.toString());
    }

    // push the call return result
    TU_RETURN_IF_NOT_OK (driver->pushResult(returnType));

    // append the deref effect
    DerefEffect effect;
    effect.receiverType = returnType;
    effect.derefSymbol = derefSymbol;
    effect.pushResult = true;
    effect.sideEffecting = true;
    m_deref->effects.push_back(std::move(effect));

    return {};
}

lyric_compiler::DataDerefMethod::DataDerefMethod(
    DataDeref *deref,
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *invokeBlock,
    std::unique_ptr<lyric_assembler::CallableInvoker> &&invoker,
    std::unique_ptr<lyric_typing::CallsiteReifier> &&reifier,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
    : BaseInvokableHandler(bindingBlock, invokeBlock, fragment, driver),
      m_deref(deref),
      m_invoker(std::move(invoker)),
      m_reifier(std::move(reifier))
{
    TU_NOTNULL (m_deref);
    TU_NOTNULL (m_invoker);
    TU_NOTNULL (m_reifier);
}

tempo_utils::Status
lyric_compiler::DataDerefMethod::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *symbolCache = driver->getSymbolCache();
    auto *fragment = getFragment();

    TU_RETURN_IF_NOT_OK (placeArguments(m_invoker->getCallable(), *m_reifier, fragment));

    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_invoker->invoke(getInvokeBlock(), *m_reifier, fragment));

    lyric_assembler::AbstractSymbol *derefSymbol;

    // store the deref symbol if return type is concrete
    switch (returnType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            derefSymbol = symbolCache->getSymbolOrNull(returnType.getConcreteUrl());
            break;
        case lyric_common::TypeDefType::Intersection:
        case lyric_common::TypeDefType::Union:
        case lyric_common::TypeDefType::Placeholder:
        case lyric_common::TypeDefType::NoReturn:
            derefSymbol = nullptr;
            break;
        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "unexpected return type {}", returnType.toString());
    }

    // pop the receiver
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // push the method return result
    TU_RETURN_IF_NOT_OK (driver->pushResult(returnType));

    // append the deref effect
    DerefEffect effect;
    effect.receiverType = returnType;
    effect.derefSymbol = derefSymbol;
    effect.pushResult = true;
    effect.sideEffecting = true;
    m_deref->effects.push_back(std::move(effect));

    return {};
}

lyric_compiler::DataDerefBlock::DataDerefBlock(
    DataDeref *deref,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_deref(deref)
{
    TU_NOTNULL (m_deref);
    TU_NOTNULL (block);
    TU_NOTNULL (fragment);
    TU_NOTNULL (driver);

    auto groupBlock = std::make_unique<lyric_assembler::BlockHandle>(
        block->blockProc(), block, block->blockState());
    m_blockHandler = std::make_unique<BlockHandler>(
        std::move(groupBlock), /* requiresResult= */ true, /* isSideEffect= */ false, fragment, driver);
}

tempo_utils::Status
lyric_compiler::DataDerefBlock::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    return m_blockHandler->before(state, node, ctx);
}

tempo_utils::Status
lyric_compiler::DataDerefBlock::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_RETURN_IF_NOT_OK (m_blockHandler->after(state, node, ctx));

    auto *driver = getDriver();

    auto receiverType = driver->peekResult();

    lyric_assembler::AbstractSymbol *derefSymbol;
    if (receiverType.getType() == lyric_common::TypeDefType::Concrete) {
        auto *symbolCache = driver->getSymbolCache();
        derefSymbol = symbolCache->getSymbolOrNull(receiverType.getConcreteUrl());
    } else {
        derefSymbol = nullptr;
    }

    DerefEffect effect;
    effect.receiverType = receiverType;
    effect.derefSymbol = derefSymbol;
    effect.pushResult = true;
    effect.sideEffecting = false;
    m_deref->effects.push_back(std::move(effect));

    return {};
}

lyric_compiler::DataDerefNew::DataDerefNew(
    DataDeref *deref,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_deref(deref)
{
    TU_NOTNULL (m_deref);
    TU_NOTNULL (block);
    TU_NOTNULL (fragment);
    TU_NOTNULL (driver);

    m_newHandler = std::make_unique<NewHandler>(/* isSideEffect= */ false, fragment, block, driver);
}

tempo_utils::Status
lyric_compiler::DataDerefNew::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    return m_newHandler->before(state, node, ctx);
}

tempo_utils::Status
lyric_compiler::DataDerefNew::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_RETURN_IF_NOT_OK (m_newHandler->after(state, node, ctx));

    auto *driver = getDriver();

    auto receiverType = driver->peekResult();

    lyric_assembler::AbstractSymbol *derefSymbol;
    if (receiverType.getType() == lyric_common::TypeDefType::Concrete) {
        auto *symbolCache = driver->getSymbolCache();
        derefSymbol = symbolCache->getSymbolOrNull(receiverType.getConcreteUrl());
    } else {
        derefSymbol = nullptr;
    }

    DerefEffect effect;
    effect.receiverType = receiverType;
    effect.derefSymbol = derefSymbol;
    effect.pushResult = true;
    effect.sideEffecting = false;
    m_deref->effects.push_back(std::move(effect));

    return {};
}
