
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/constructor_handler.h>
#include <lyric_compiler/defenum_handler.h>
#include <lyric_compiler/defenum_utils.h>
#include <lyric_compiler/member_handler.h>
#include <lyric_compiler/method_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::DefEnumHandler::DefEnumHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(nullptr)
{
}

lyric_compiler::DefEnumHandler::DefEnumHandler(
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
lyric_compiler::DefEnumHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    TU_LOG_INFO << "before DefEnumHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    if (!node->isClass(lyric_schema::kLyricAstDefEnumClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected DefEnum node");

    // get enum name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get enum access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

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
    std::vector<lyric_parser::ArchetypeNode *> caseNodes;
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
                        "duplicate enum init declaration");
                initNode = child;
                break;
            case lyric_schema::LyricAstId::Val: {
                valNodes.push_back(child);
                break;
            }
            case lyric_schema::LyricAstId::Case: {
                caseNodes.push_back(child);
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

        auto definition = std::make_unique<EnumDefinition>(&m_defenum, block, driver);
        ctx.appendChoice(std::move(definition));
    }

    // resolve the super enum type if specified, otherwise derive from Singleton
    auto superEnumType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Category);

    // resolve the super enum symbol
    TU_ASSIGN_OR_RETURN (m_defenum.superenumSymbol, block->resolveEnum(superEnumType));

    // declare the enum
    TU_ASSIGN_OR_RETURN (m_defenum.enumSymbol, block->declareEnum(
        identifier, m_defenum.superenumSymbol, lyric_compiler::convert_access_type(access),
        lyric_object::DeriveType::Sealed, isAbstract));

    // add enum to the current namespace if specified
    if (m_currentNamespace != nullptr) {
        TU_RETURN_IF_NOT_OK (m_currentNamespace->putTarget(m_defenum.enumSymbol->getSymbolUrl()));
    }

    // declare val members
    for (auto &valNode : valNodes) {
        Member member;
        TU_ASSIGN_OR_RETURN (member, declare_enum_member(
            valNode, /* isVariable= */ false, m_defenum.enumSymbol, typeSystem));
        m_defenum.members[valNode] = member;
    }

    // declare enum init
    if (initNode != nullptr) {
        TU_ASSIGN_OR_RETURN (m_defenum.initCall, declare_enum_init(
            initNode, m_defenum.enumSymbol, allocatorTrap, typeSystem));
    } else {
        TU_ASSIGN_OR_RETURN (m_defenum.initCall, declare_enum_default_init(
            &m_defenum, m_defenum.enumSymbol, allocatorTrap, symbolCache, typeSystem));
    }

    // declare methods
    for (auto &defNode : defNodes) {
        Method method;
        TU_ASSIGN_OR_RETURN (method, declare_enum_method(
            defNode, m_defenum.enumSymbol, typeSystem));
        m_defenum.methods[defNode] = method;
    }

    // declare impls
    for (auto &implNode : implNodes) {
        Impl impl;
        TU_ASSIGN_OR_RETURN (impl, declare_enum_impl(
            implNode, m_defenum.enumSymbol, typeSystem));
        m_defenum.impls[implNode] = impl;
    }

    // FIXME: declare enum cases
    for (auto &caseNode : caseNodes) {
        lyric_assembler::EnumSymbol *enumcase;
        TU_ASSIGN_OR_RETURN (enumcase, declare_enum_case(
            caseNode, m_defenum.enumSymbol, block, typeSystem));
        m_defenum.cases[caseNode] = enumcase;
    }

    return {};
}

tempo_utils::Status
lyric_compiler::DefEnumHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    TU_LOG_INFO << "after DefEnumHandler@" << this;

    if (!m_isSideEffect) {
        auto *driver = getDriver();
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::EnumDefinition::EnumDefinition(
    DefEnum *defenum,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_defenum(defenum)
{
    TU_ASSERT (m_defenum != nullptr);
}

tempo_utils::Status
lyric_compiler::EnumDefinition::decide(
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
            TU_RETURN_IF_NOT_OK (m_defenum->superenumSymbol->prepareCtor(*superInvoker));
            auto handler = std::make_unique<ConstructorHandler>(
                std::move(superInvoker), m_defenum->initCall, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Val: {
            auto member = m_defenum->members.at(node);
            auto handler = std::make_unique<MemberHandler>(member, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Case: {
            auto *enumcase = m_defenum->cases.at(node);
            auto handler = std::make_unique<EnumCase>(enumcase, block, driver);
            ctx.setChoice(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Def: {
            auto method = m_defenum->methods.at(node);
            auto handler = std::make_unique<MethodHandler>(method, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Impl: {
            auto impl = m_defenum->impls.at(node);
            auto handler = std::make_unique<ImplHandler>(impl, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                "unexpected AST node");
    }
}

lyric_compiler::EnumCase::EnumCase(
    lyric_assembler::EnumSymbol *enumcase,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_enumcase(enumcase)
{
    TU_ASSERT (m_enumcase != nullptr);
}

tempo_utils::Status
lyric_compiler::EnumCase::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    // declare the case ctor
    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, m_enumcase->declareCtor(lyric_object::AccessType::Public));

    // define the ctor
    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, ctorSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));
    auto *procBuilder = procHandle->procCode();
    auto *fragment = procBuilder->rootFragment();

    // find the superenum ctor
    auto superInvoker = std::make_unique<lyric_assembler::ConstructableInvoker>();
    TU_RETURN_IF_NOT_OK (m_enumcase->superEnum()->prepareCtor(*superInvoker));

    auto caseinit = std::make_unique<CaseInit>(std::move(superInvoker), fragment, block, driver);
    ctx.setGrouping(std::move(caseinit));

    return {};
}

lyric_compiler::CaseInit::CaseInit(
    std::unique_ptr<lyric_assembler::ConstructableInvoker> &&invoker,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseInvokableHandler(block, block, fragment, driver),
      m_invoker(std::move(invoker))
{
    TU_ASSERT (m_invoker != nullptr);
}

tempo_utils::Status
lyric_compiler::CaseInit::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *fragment = getFragment();

    // load the uninitialized receiver onto the top of the stack
    TU_RETURN_IF_NOT_OK (fragment->loadThis());

    return BaseInvokableHandler::before(state, node, ctx);
}

tempo_utils::Status
lyric_compiler::CaseInit::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *fragment = getFragment();

    lyric_typing::CallsiteReifier reifier(typeSystem);

    TU_RETURN_IF_NOT_OK (reifier.initialize(*m_invoker));

    auto *constructable = m_invoker->getConstructable();
    TU_RETURN_IF_NOT_OK (placeArguments(constructable, reifier, fragment));

    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_invoker->invoke(getInvokeBlock(), reifier, fragment, /* flags= */ 0));

    return driver->pushResult(returnType);
}