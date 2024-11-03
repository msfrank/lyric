
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/constructor_handler.h>
#include <lyric_compiler/defstruct_handler.h>
#include <lyric_compiler/defstruct_utils.h>
#include <lyric_compiler/member_handler.h>
#include <lyric_compiler/method_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::DefStructHandler::DefStructHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect)
{
}

tempo_utils::Status
lyric_compiler::DefStructHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    TU_LOG_INFO << "before DefStructHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    if (!node->isClass(lyric_schema::kLyricAstDefStructClass))
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "expected DefStruct node");

    // get struct name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get struct access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    // get struct derive type
    lyric_parser::DeriveType derive;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));

    // FIXME: get abstract flag from node
    bool isAbstract = false;

    lyric_parser::ArchetypeNode *initNode = nullptr;
    std::vector<lyric_parser::ArchetypeNode *> valNodes;
    std::vector<lyric_parser::ArchetypeNode *> defNodes;
    std::vector<lyric_parser::ArchetypeNode *> implNodes;

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        switch (astId) {
            case lyric_schema::LyricAstId::Init:
                if (initNode != nullptr)
                    return block->logAndContinue(CompilerCondition::kSyntaxError,
                        tempo_tracing::LogSeverity::kError,
                        "duplicate struct init declaration");
                initNode = child;
                break;
            case lyric_schema::LyricAstId::Val: {
                valNodes.push_back(child);
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

        auto definition = std::make_unique<StructDefinition>(&m_defstruct, block, driver);
        ctx.appendChoice(std::move(definition));
    }

    // resolve the super struct type if specified, otherwise derive from Record
    auto superStructType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record);
    if (initNode) {
        auto *superNode = initNode->getChild(1);
        TU_ASSERT (superNode != nullptr);
        if (superNode->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
            lyric_parser::ArchetypeNode *typeNode;
            TU_RETURN_IF_NOT_OK (superNode->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
            lyric_typing::TypeSpec superStructSpec;
            TU_ASSIGN_OR_RETURN (superStructSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
            TU_ASSIGN_OR_RETURN (superStructType, typeSystem->resolveAssignable(block, superStructSpec));
        }
    }

    // resolve the super struct symbol
    TU_ASSIGN_OR_RETURN (m_defstruct.superstructSymbol, block->resolveStruct(superStructType));

    // declare the struct
    TU_ASSIGN_OR_RETURN (m_defstruct.structSymbol, block->declareStruct(
        identifier, m_defstruct.superstructSymbol, lyric_compiler::convert_access_type(access),
        lyric_compiler::convert_derive_type(derive), isAbstract));

    // declare val members
    for (auto &valNode : valNodes) {
        Member member;
        TU_ASSIGN_OR_RETURN (member, declare_struct_member(valNode, m_defstruct.structSymbol, typeSystem));
        m_defstruct.members[valNode] = member;
    }

    // declare struct init
    if (initNode != nullptr) {
        TU_ASSIGN_OR_RETURN (m_defstruct.initCall, declare_struct_init(
            initNode, m_defstruct.structSymbol, typeSystem));
    } else {
        TU_ASSIGN_OR_RETURN (m_defstruct.initCall, declare_struct_default_init(
            &m_defstruct, m_defstruct.structSymbol, symbolCache, typeSystem));
    }

    // declare methods
    for (auto &defNode : defNodes) {
        Method method;
        TU_ASSIGN_OR_RETURN (method, declare_struct_method(
            defNode, m_defstruct.structSymbol, typeSystem));
        m_defstruct.methods[defNode] = method;
    }

    // declare impls
    for (auto &implNode : implNodes) {
        Impl impl;
        TU_ASSIGN_OR_RETURN (impl, declare_struct_impl(
            implNode, m_defstruct.structSymbol, typeSystem));
        m_defstruct.impls[implNode] = impl;
    }

    return {};
}

tempo_utils::Status
lyric_compiler::DefStructHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    TU_LOG_INFO << "after DefStructHandler@" << this;

    if (!m_isSideEffect) {
        auto *driver = getDriver();
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::StructDefinition::StructDefinition(
    DefStruct *defstruct,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_defstruct(defstruct)
{
    TU_ASSERT (m_defstruct != nullptr);
}

tempo_utils::Status
lyric_compiler::StructDefinition::decide(
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
            TU_RETURN_IF_NOT_OK (m_defstruct->superstructSymbol->prepareCtor(*superInvoker));
            auto handler = std::make_unique<ConstructorHandler>(
                std::move(superInvoker), m_defstruct->initCall, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Val: {
            auto member = m_defstruct->members.at(node);
            auto handler = std::make_unique<MemberHandler>(member, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Def: {
            auto method = m_defstruct->methods.at(node);
            auto handler = std::make_unique<MethodHandler>(method, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Impl: {
            auto impl = m_defstruct->impls.at(node);
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