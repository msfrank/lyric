
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
    lyric_compiler::CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect)
{
}

tempo_utils::Status
lyric_compiler::DefInstanceHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    TU_LOG_INFO << "before DefInstanceHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    if (!node->isClass(lyric_schema::kLyricAstDefInstanceClass))
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
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

    // FIXME: get abstract flag from node
    bool isAbstract = false;

    std::vector<lyric_parser::ArchetypeNode *> valNodes;
    std::vector<lyric_parser::ArchetypeNode *> varNodes;
    std::vector<lyric_parser::ArchetypeNode *> defNodes;
    std::vector<lyric_parser::ArchetypeNode *> implNodes;

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        switch (astId) {
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
                return block->logAndContinue(CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
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
    TU_ASSIGN_OR_RETURN (m_definstance.initCall, declare_instance_default_init(
        &m_definstance, m_definstance.instanceSymbol, symbolCache, typeSystem));

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
    lyric_compiler::AfterContext &ctx)
{
    TU_LOG_INFO << "after DefInstanceHandler@" << this;

    if (!m_isSideEffect) {
        auto *driver = getDriver();
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
            return block->logAndContinue(CompilerCondition::kSyntaxError,
                tempo_tracing::LogSeverity::kError,
                "unexpected AST node");
    }
}
