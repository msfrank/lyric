
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/constructor_handler.h>
#include <lyric_compiler/defclass_handler.h>
#include <lyric_compiler/defclass_utils.h>
#include <lyric_compiler/member_handler.h>
#include <lyric_compiler/method_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::DefClassHandler::DefClassHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(nullptr)
{
}

lyric_compiler::DefClassHandler::DefClassHandler(
    bool isSideEffect,
    lyric_assembler::NamespaceSymbol *currentNamespace,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(currentNamespace)
{
    TU_ASSERT (m_currentNamespace != nullptr);
}

tempo_utils::Status
lyric_compiler::DefClassHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    TU_LOG_VV << "before DefClassHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *typeSystem = driver->getTypeSystem();

    if (!node->isClass(lyric_schema::kLyricAstDefClassClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected DefClass node");

    // get class name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get class access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    // get class derive type
    lyric_parser::DeriveType derive;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));

    // if class is generic, then compile the template parameter list
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (m_defclass.templateSpec, typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

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
                        "duplicate class init declaration");
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

        auto definition = std::make_unique<ClassDefinition>(&m_defclass, block, driver);
        ctx.appendChoice(std::move(definition));
    }

    // resolve the super class type if specified, otherwise derive from Object
    auto superClassType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    if (initNode) {
        auto *superNode = initNode->getChild(1);
        TU_ASSERT (superNode != nullptr);
        if (superNode->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
            lyric_parser::ArchetypeNode *typeNode;
            TU_RETURN_IF_NOT_OK (superNode->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
            lyric_typing::TypeSpec superClassSpec;
            TU_ASSIGN_OR_RETURN (superClassSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
            TU_ASSIGN_OR_RETURN (superClassType, typeSystem->resolveAssignable(block, superClassSpec));
        }
    }

    // resolve the super class symbol
    TU_ASSIGN_OR_RETURN (m_defclass.superclassSymbol, block->resolveClass(superClassType));

    // declare the class
    TU_ASSIGN_OR_RETURN (m_defclass.classSymbol, block->declareClass(
        identifier, m_defclass.superclassSymbol, lyric_compiler::convert_access_type(access),
        m_defclass.templateSpec.templateParameters, lyric_compiler::convert_derive_type(derive),
        isAbstract));

    // add class to the current namespace if specified
    if (m_currentNamespace != nullptr) {
        TU_RETURN_IF_NOT_OK (m_currentNamespace->putTarget(m_defclass.classSymbol->getSymbolUrl()));
    }

    // declare val members
    for (auto &valNode : valNodes) {
        Member member;
        TU_ASSIGN_OR_RETURN (member, declare_class_member(
            valNode, /* isVariable= */ false, m_defclass.classSymbol, typeSystem));
        m_defclass.members[valNode] = member;
    }

    // declare var members
    for (auto &varNode : varNodes) {
        Member member;
        TU_ASSIGN_OR_RETURN (member, declare_class_member(
            varNode, /* isVariable= */ true, m_defclass.classSymbol, typeSystem));
        m_defclass.members[varNode] = member;
    }

    // declare class init
    if (initNode != nullptr) {
        TU_ASSIGN_OR_RETURN (m_defclass.initCall, declare_class_init(
            initNode, m_defclass.classSymbol, allocatorTrap, typeSystem));
    } else {
        TU_ASSIGN_OR_RETURN (m_defclass.initCall, declare_class_default_init(
            m_defclass.classSymbol, allocatorTrap));
        m_defclass.defaultInit = true;
    }

    // declare methods
    for (auto &defNode : defNodes) {
        Method method;
        TU_ASSIGN_OR_RETURN (method, declare_class_method(
            defNode, m_defclass.classSymbol, typeSystem));
        m_defclass.methods[defNode] = method;
    }

    // declare impls
    for (auto &implNode : implNodes) {
        Impl impl;
        TU_ASSIGN_OR_RETURN (impl, declare_class_impl(
            implNode, m_defclass.classSymbol, typeSystem));
        m_defclass.impls[implNode] = impl;
    }

    return {};
}

tempo_utils::Status
lyric_compiler::DefClassHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after DefClassHandler@" << this;

    auto *driver = getDriver();

    if (m_defclass.defaultInit) {
        auto *symbolCache = driver->getSymbolCache();
        auto *typeSystem = driver->getTypeSystem();
        TU_RETURN_IF_NOT_OK (define_class_default_init(&m_defclass, symbolCache, typeSystem));
    }

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::ClassDefinition::ClassDefinition(
    DefClass *defclass,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_defclass(defclass)
{
    TU_ASSERT (m_defclass != nullptr);
}

tempo_utils::Status
lyric_compiler::ClassDefinition::decide(
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
            TU_RETURN_IF_NOT_OK (m_defclass->superclassSymbol->prepareCtor(*superInvoker));
            auto handler = std::make_unique<ConstructorHandler>(
                std::move(superInvoker), m_defclass->initCall, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Val:
        case lyric_schema::LyricAstId::Var: {
            auto member = m_defclass->members.at(node);
            auto handler = std::make_unique<MemberHandler>(member, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Def: {
            auto method = m_defclass->methods.at(node);
            auto handler = std::make_unique<MethodHandler>(method, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Impl: {
            auto impl = m_defclass->impls.at(node);
            auto handler = std::make_unique<ImplHandler>(impl, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                "unexpected AST node");
    }
}