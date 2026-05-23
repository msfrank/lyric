
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
    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    // get abstract flag
    bool isAbstract;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsAbstract, isAbstract));

    // get instance derive type
    lyric_parser::DeriveType derive;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));

    // get allocator trap
    if (node->hasAttr(lyric_assembler::kLyricAssemblerTrapName)) {
        TU_RETURN_IF_NOT_OK (node->parseAttr(
            lyric_assembler::kLyricAssemblerTrapName, m_allocatorTrapName));
    }

    // declare the instance
    TU_ASSIGN_OR_RETURN (m_definstance.instanceSymbol, block->declareInstance(
        identifier, isHidden, isAbstract, lyric_compiler::convert_derive_type(derive)));

    std::vector<lyric_parser::ArchetypeNode *> initNodes;
    std::vector<lyric_parser::ArchetypeNode *> fieldNodes;
    std::vector<lyric_parser::ArchetypeNode *> defNodes;
    std::vector<lyric_parser::ArchetypeNode *> declNodes;
    std::vector<lyric_parser::ArchetypeNode *> implNodes;
    std::vector<lyric_parser::ArchetypeNode *> globalNodes;

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        switch (astId) {
            case lyric_schema::LyricAstId::Init: {
                initNodes.push_back(child);
                break;
            }
            case lyric_schema::LyricAstId::Field: {
                fieldNodes.push_back(child);
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
            case lyric_schema::LyricAstId::Global: {
                globalNodes.push_back(child);
                break;
            }
            default:
                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                    "unexpected AST node");
        }

        auto definition = std::make_unique<InstanceDefinition>(&m_definstance, block, driver);
        ctx.appendChoice(std::move(definition));
    }

    lyric_common::TypeDef superInstanceType;

    // resolve the super instance type if specified, otherwise derive from Singleton
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *typeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
        lyric_typing::TypeSpec superInstanceSpec;
        TU_ASSIGN_OR_RETURN (superInstanceSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (superInstanceType, typeSystem->resolveAssignable(block, superInstanceSpec));
    } else {
        superInstanceType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Singleton);
    }

    // finalize the instance
    TU_RETURN_IF_NOT_OK (m_definstance.instanceSymbol->finalizeInstance(superInstanceType));

    // verify that instance is a valid subtype of superinstance
    TU_RETURN_IF_NOT_OK (typeSystem->validateSubtype(superInstanceType, m_definstance.instanceSymbol->superInstance()));

    // add instance to the current namespace if specified
    if (m_currentNamespace != nullptr) {
        TU_RETURN_IF_NOT_OK (m_currentNamespace->putTarget(m_definstance.instanceSymbol->getSymbolUrl()));
    }

    m_definstance.global.definitionBlock = m_definstance.instanceSymbol->instanceBlock();

    // declare members
    for (auto &fieldNode : fieldNodes) {
        Member member;
        TU_ASSIGN_OR_RETURN (member, declare_instance_member(fieldNode, m_definstance.instanceSymbol, typeSystem));
        m_definstance.members[fieldNode] = member;
    }

    // declare stubs
    for (auto &declNode : declNodes) {
        Stub stub;
        TU_ASSIGN_OR_RETURN (stub, declare_instance_stub(
            declNode, m_definstance.instanceSymbol, typeSystem));
        m_definstance.stubs[declNode] = stub;
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

    // declare globals
    for (auto &globalNode : globalNodes) {
        TU_RETURN_IF_NOT_OK (declare_global_block(globalNode, &m_definstance.global, typeSystem));
    }

    // declare constructors if any are specified, otherwise declare a default constructor
    if (!initNodes.empty()) {
        for (auto &initNode : initNodes) {
            Constructor constructor;
            TU_ASSIGN_OR_RETURN (constructor, declare_instance_init(
                initNode, m_definstance.instanceSymbol, m_allocatorTrapName, typeSystem));
            m_definstance.ctors[initNode] = constructor;
        }
    } else {
        TU_ASSIGN_OR_RETURN (m_definstance.defaultCtor, declare_instance_default_init(
            m_definstance.instanceSymbol, m_allocatorTrapName));
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

    if (m_definstance.defaultCtor != nullptr) {
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
            auto constructor = m_definstance->ctors.at(node);
            auto handler = std::make_unique<ConstructorHandler>(constructor, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Field: {
            auto member = m_definstance->members.at(node);
            auto handler = std::make_unique<MemberHandler>(member, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Decl: {
            auto stub = m_definstance->stubs.at(node);
            auto handler = std::make_unique<StubHandler>(stub, block, driver);
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
        case lyric_schema::LyricAstId::Global: {
            if (!m_definstance->instanceSymbol->isAbstract())
                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                    "global block is not valid on a non-abstract instance");
            auto handler = std::make_unique<GlobalHandler>(&m_definstance->global, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                "unexpected AST node");
    }
}
