
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/constructor_handler.h>
#include <lyric_compiler/definstance_handler.h>
#include <lyric_compiler/definstance_utils.h>
#include <lyric_compiler/member_handler.h>
#include <lyric_compiler/method_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::DefInstanceHandler::DefInstanceHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(nullptr)
{
}

lyric_compiler::DefInstanceHandler::DefInstanceHandler(
    bool isSideEffect,
    lyric_assembler::NamespaceSymbol *currentNamespace,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(currentNamespace)
{
    TU_ASSERT (m_currentNamespace != nullptr);
}

tempo_utils::Status
lyric_compiler::DefInstanceHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before DefInstanceHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *typeSystem = driver->getTypeSystem();

    if (!node->isClass(lyric_schema::kLyricAstDefInstanceClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected DefInstance node");

    // get instance name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get instance access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    // get instance derive type
    lyric_parser::DeriveType derive;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));

    // get allocator trap
    std::string allocatorTrap;
    if (node->hasAttr(lyric_assembler::kLyricAssemblerTrapName)) {
        TU_RETURN_IF_NOT_OK (node->parseAttr(
            lyric_assembler::kLyricAssemblerTrapName, allocatorTrap));
    }

    // FIXME: get abstract flag from node
    bool isAbstract = false;

    lyric_parser::ArchetypeNode *initNode = nullptr;
    std::vector<lyric_parser::ArchetypeNode *> valNodes;
    std::vector<lyric_parser::ArchetypeNode *> varNodes;
    std::vector<lyric_parser::ArchetypeNode *> defNodes;
    std::vector<lyric_parser::ArchetypeNode *> implNodes;

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        switch (astId) {
            case lyric_schema::LyricAstId::Init:
                if (initNode != nullptr)
                    return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                        "duplicate instance init declaration");
                initNode = child;
                break;
            case lyric_schema::LyricAstId::Val: {
                valNodes.push_back(child);
                break;
            }
            case lyric_schema::LyricAstId::Var: {
                varNodes.push_back(child);
                break;
            }
            case lyric_schema::LyricAstId::Def: {
                defNodes.push_back(child);
                break;
            }
            case lyric_schema::LyricAstId::Impl: {
                implNodes.push_back(child);
                break;
            }
            default:
                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                    "unexpected AST node");
        }

        auto definition = std::make_unique<InstanceDefinition>(&m_definstance, block, driver);
        ctx.appendChoice(std::move(definition));
    }

    // resolve the super instance type if specified, otherwise derive from Singleton
    auto superInstanceType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Singleton);

    // resolve the super instance symbol
    TU_ASSIGN_OR_RETURN (m_definstance.superinstanceSymbol, block->resolveInstance(superInstanceType));

    // declare the instance
    TU_ASSIGN_OR_RETURN (m_definstance.instanceSymbol, block->declareInstance(
        identifier, m_definstance.superinstanceSymbol, lyric_compiler::convert_access_type(access),
        lyric_compiler::convert_derive_type(derive), isAbstract));

    // add instance to the current namespace if specified
    if (m_currentNamespace != nullptr) {
        TU_RETURN_IF_NOT_OK (m_currentNamespace->putTarget(m_definstance.instanceSymbol->getSymbolUrl()));
    }

    // declare val members
    for (auto &valNode : valNodes) {
        Member member;
        TU_ASSIGN_OR_RETURN (member, declare_instance_member(
            valNode, /* isVariable= */ false, m_definstance.instanceSymbol, typeSystem));
        m_definstance.members[valNode] = member;
    }

    // declare var members
    for (auto &varNode : varNodes) {
        Member member;
        TU_ASSIGN_OR_RETURN (member, declare_instance_member(
            varNode, /* isVariable= */ true, m_definstance.instanceSymbol, typeSystem));
        m_definstance.members[varNode] = member;
    }

    // declare instance init
    TU_ASSIGN_OR_RETURN (m_definstance.initCall, declare_instance_init(
        m_definstance.instanceSymbol, allocatorTrap));
    if (initNode == nullptr) {
        m_definstance.defaultInit = true;
    }

    // declare methods
    for (auto &defNode : defNodes) {
        Method method;
        TU_ASSIGN_OR_RETURN (method, declare_instance_method(
            defNode, m_definstance.instanceSymbol, typeSystem));
        m_definstance.methods[defNode] = method;
    }

    // declare impls
    for (auto &implNode : implNodes) {
        Impl impl;
        TU_ASSIGN_OR_RETURN (impl, declare_instance_impl(
            implNode, m_definstance.instanceSymbol, typeSystem));
        m_definstance.impls[implNode] = impl;
    }

    return {};
}

tempo_utils::Status
lyric_compiler::DefInstanceHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after DefInstanceHandler@" << this;

    auto *driver = getDriver();

    if (m_definstance.defaultInit) {
        auto *symbolCache = driver->getSymbolCache();
        auto *typeSystem = driver->getTypeSystem();
        TU_RETURN_IF_NOT_OK (define_instance_default_init(&m_definstance, symbolCache, typeSystem));
    }

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::InstanceDefinition::InstanceDefinition(
    DefInstance *definstance,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_definstance(definstance)
{
    TU_ASSERT (m_definstance != nullptr);
}

tempo_utils::Status
lyric_compiler::InstanceDefinition::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    lyric_schema::LyricAstId astId;
    TU_RETURN_IF_NOT_OK (node->parseId(lyric_schema::kLyricAstVocabulary, astId));
    switch (astId) {
        case lyric_schema::LyricAstId::Init: {
            auto superInvoker = std::make_unique<lyric_assembler::ConstructableInvoker>();
            TU_RETURN_IF_NOT_OK (m_definstance->superinstanceSymbol->prepareCtor(*superInvoker));
            auto handler = std::make_unique<ConstructorHandler>(
                std::move(superInvoker), m_definstance->initCall, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Val:
        case lyric_schema::LyricAstId::Var: {
            auto member = m_definstance->members.at(node);
            auto handler = std::make_unique<MemberHandler>(member, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Def: {
            auto method = m_definstance->methods.at(node);
            auto handler = std::make_unique<MethodHandler>(method, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Impl: {
            auto impl = m_definstance->impls.at(node);
            auto handler = std::make_unique<ImplHandler>(impl, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                "unexpected AST node");
    }
}
