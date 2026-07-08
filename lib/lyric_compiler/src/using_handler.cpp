
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/using_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/member_reifier.h>

static tempo_utils::Result<lyric_common::SymbolUrl>
define_capture_function(
    const std::vector<std::string> &remaining,
    const lyric_assembler::DataReference &initial,
    lyric_assembler::BlockHandle *usingBlock,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (!remaining.empty());
    auto *state = usingBlock->blockState();
    auto *symbolCache = state->symbolCache();

    auto name = absl::StrCat("$capture", state->numCalls());
    lyric_assembler::CallSymbol *captureSymbol;
    TU_ASSIGN_OR_RETURN (captureSymbol, usingBlock->declareFunction(name, /* isHidden= */ true, {}));

    lyric_assembler::Parameter p0;
    p0.index = 0;
    p0.name = "from";
    p0.placement = lyric_object::PlacementType::List;
    p0.typeDef = initial.typeDef;
    p0.isVariable = false;
    lyric_assembler::ParameterPack parameterPack{{p0}};

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, captureSymbol->defineCall(parameterPack));

    auto *block = procHandle->procBlock();
    auto *fragment = procHandle->procFragment();

    lyric_assembler::DataReference curr;
    TU_ASSIGN_OR_RETURN (curr, block->resolveReference(p0.name));
    TU_RETURN_IF_NOT_OK (fragment->loadRef(curr));

    for (const auto &identifier : remaining) {
        lyric_typing::MemberReifier reifier(typeSystem);
        const auto &receiverType = curr.typeDef;

        lyric_assembler::AbstractSymbol *derefSymbol;
        TU_ASSIGN_OR_RETURN (derefSymbol, symbolCache->getOrImportSymbol(curr.symbolUrl));

        switch (derefSymbol->getSymbolType()) {
            case lyric_assembler::SymbolType::CLASS: {
                auto *classSymbol = lyric_assembler::cast_symbol_to_class(derefSymbol);
                TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType, classSymbol->classTemplate()));
                TU_ASSIGN_OR_RETURN (curr, classSymbol->resolveMember(identifier, reifier, receiverType));
                break;
            }
            case lyric_assembler::SymbolType::ENUM: {
                auto *enumSymbol = lyric_assembler::cast_symbol_to_enum(derefSymbol);
                TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
                TU_ASSIGN_OR_RETURN (curr, enumSymbol->resolveMember(identifier, reifier, receiverType));
                break;
            }
            case lyric_assembler::SymbolType::INSTANCE: {
                auto *instanceSymbol = lyric_assembler::cast_symbol_to_instance(derefSymbol);
                TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
                TU_ASSIGN_OR_RETURN (curr, instanceSymbol->resolveMember(identifier, reifier, receiverType));
                break;
            }
            case lyric_assembler::SymbolType::STRUCT: {
                auto *structSymbol = lyric_assembler::cast_symbol_to_struct(derefSymbol);
                TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
                TU_ASSIGN_OR_RETURN (curr, structSymbol->resolveMember(identifier, reifier, receiverType));
                break;
            }
            default:
                return lyric_compiler::CompilerStatus::forCondition(
                    lyric_compiler::CompilerCondition::kInvalidSymbol,
                    "invalid receiver symbol {}", derefSymbol->getSymbolUrl().toString());
        }

        TU_RETURN_IF_NOT_OK (fragment->loadRef(curr));
    }

    procHandle->putExitType(curr.typeDef);
    TU_RETURN_IF_STATUS (captureSymbol->finalizeCall());

    return captureSymbol->getSymbolUrl();
}

lyric_compiler::RootUsingHandler::RootUsingHandler(CompilerScanDriver *driver)
    : BaseGrouping(driver)
{
}

tempo_utils::Status
lyric_compiler::RootUsingHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *driver = getDriver();
    auto *root = driver->getObjectRoot();
    auto *globalNamespace = root->globalNamespace();
    auto *block = globalNamespace->namespaceBlock();

    if (!node->isClass(lyric_schema::kLyricAstUsingClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Using node");

    // get the impl path
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, m_capture.usingPath));

    // if impl types were specified then handle them
    for (auto i = 0; i < node->numChildren(); i++) {
        auto impl = std::make_unique<UsingImpl>(&m_capture, block, driver);
        ctx.appendChoice(std::move(impl));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::RootUsingHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *root = driver->getObjectRoot();
    auto *globalNamespace = root->globalNamespace();
    auto *globalBlock = globalNamespace->namespaceBlock();
    auto *rootBlock = root->rootBlock();

    // resolve the initial reference in the using path from the global namespace block
    std::vector<std::string> remaining;
    TU_ASSIGN_OR_RETURN (m_capture.captureRef, globalBlock->resolveReference(m_capture.usingPath, remaining));

    // register the impls in the root block
    if (!remaining.empty()) {
        auto *typeSystem = driver->getTypeSystem();
        lyric_common::SymbolUrl captureUrl;
        TU_ASSIGN_OR_RETURN (captureUrl, define_capture_function(remaining, m_capture.captureRef, globalBlock, typeSystem));
        TU_RETURN_IF_NOT_OK (rootBlock->useImpls(m_capture.captureRef, captureUrl, m_capture.implTypes));
    } else {
        TU_RETURN_IF_NOT_OK (rootBlock->useImpls(m_capture.captureRef, m_capture.implTypes));
    }

    return {};
}

lyric_compiler::BlockUsingHandler::BlockUsingHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect)
{
}

tempo_utils::Status
lyric_compiler::BlockUsingHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *driver = getDriver();
    auto *block = getBlock();

    if (!node->isClass(lyric_schema::kLyricAstUsingClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Using node");

    // get the impl path
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, m_capture.usingPath));

    // if impl types were specified then handle them
    for (auto i = 0; i < node->numChildren(); i++) {
        auto impl = std::make_unique<UsingImpl>(&m_capture, block, driver);
        ctx.appendChoice(std::move(impl));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::BlockUsingHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    // resolve the initial reference in the using path
    std::vector<std::string> remaining;
    TU_ASSIGN_OR_RETURN (m_capture.captureRef, block->resolveReference(m_capture.usingPath, remaining));

    // register the impls in the current block
    if (!remaining.empty()) {
        auto *typeSystem = driver->getTypeSystem();
        lyric_common::SymbolUrl captureUrl;
        TU_ASSIGN_OR_RETURN (captureUrl, define_capture_function(remaining, m_capture.captureRef, block, typeSystem));
        TU_RETURN_IF_NOT_OK (block->useImpls(m_capture.captureRef, captureUrl, m_capture.implTypes));
    } else {
        TU_RETURN_IF_NOT_OK (block->useImpls(m_capture.captureRef, m_capture.implTypes));
    }

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::UsingImpl::UsingImpl(
    Capture *capture,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_capture(capture)
{
    TU_ASSERT (m_capture != nullptr);
}

tempo_utils::Status
lyric_compiler::UsingImpl::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    lyric_typing::TypeSpec implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(block, node->getArchetypeNode()));
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(block, implSpec));
    m_capture->implTypes.insert(implType);

    return {};
}