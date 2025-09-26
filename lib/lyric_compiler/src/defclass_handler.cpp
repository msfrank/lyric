
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
    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

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
    if (node->hasAttr(lyric_assembler::kLyricAssemblerTrapName)) {
        TU_RETURN_IF_NOT_OK (node->parseAttr(
            lyric_assembler::kLyricAssemblerTrapName, m_allocatorTrapName));
    }

    std::vector<lyric_parser::ArchetypeNode *> initNodes;
    std::vector<lyric_parser::ArchetypeNode *> valNodes;
    std::vector<lyric_parser::ArchetypeNode *> varNodes;
    std::vector<lyric_parser::ArchetypeNode *> defNodes;
    std::vector<lyric_parser::ArchetypeNode *> declNodes;
    std::vector<lyric_parser::ArchetypeNode *> implNodes;

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        switch (astId) {
            case lyric_schema::LyricAstId::Init: {
                initNodes.push_back(child);
                break;
            }
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
            case lyric_schema::LyricAstId::Decl: {
                declNodes.push_back(child);
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

    lyric_common::TypeDef superClassType;

    // resolve the super class type if specified, otherwise derive from Object
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *typeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
        lyric_typing::TypeSpec superClassSpec;
        TU_ASSIGN_OR_RETURN (superClassSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (superClassType, typeSystem->resolveAssignable(block, superClassSpec));
    } else {
        superClassType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    }

    // resolve the super class symbol
    TU_ASSIGN_OR_RETURN (m_defclass.superclassSymbol, block->resolveClass(superClassType));

    // declare the class
    TU_ASSIGN_OR_RETURN (m_defclass.classSymbol, block->declareClass(
        identifier, m_defclass.superclassSymbol, isHidden,
        m_defclass.templateSpec.templateParameters, lyric_compiler::convert_derive_type(derive)));

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

    // declare abstract methods
    for (auto &declNode : declNodes) {
        Method method;
        TU_ASSIGN_OR_RETURN (method, declare_class_abstract_method(
            declNode, m_defclass.classSymbol, typeSystem));
        m_defclass.methods[declNode] = method;
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

    // declare constructors if any are specified, otherwise declare a default constructor
    if (!initNodes.empty()) {
        for (auto &initNode : initNodes) {
            Constructor constructor;
            TU_ASSIGN_OR_RETURN (constructor, declare_class_init(
                initNode, m_defclass.classSymbol, m_allocatorTrapName, typeSystem));
            m_defclass.ctors[initNode] = constructor;
        }
    } else {
        TU_ASSIGN_OR_RETURN (m_defclass.defaultCtor, declare_class_default_init(
            m_defclass.classSymbol, m_allocatorTrapName));
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

    if (m_defclass.defaultCtor != nullptr) {
        auto *symbolCache = driver->getSymbolCache();
        auto *typeSystem = driver->getTypeSystem();
        TU_RETURN_IF_NOT_OK (define_class_default_init(&m_defclass, m_allocatorTrapName, symbolCache, typeSystem));
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
            auto constructor = m_defclass->ctors.at(node);
            auto handler = std::make_unique<ConstructorHandler>(constructor, block, driver);
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
        case lyric_schema::LyricAstId::Decl: {
            auto method = m_defclass->methods.at(node);
            auto handler = std::make_unique<AbstractMethodHandler>(method, block, driver);
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