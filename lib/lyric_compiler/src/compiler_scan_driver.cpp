
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/undeclared_symbol.h>
#include <lyric_compiler/base_choice.h>
#include <lyric_compiler/base_grouping.h>
#include <lyric_compiler/compiler_scan_driver.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/visitor_context.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_compiler::CompilerScanDriver::CompilerScanDriver(lyric_assembler::ObjectState *state)
    : m_state(state),
      m_entryCall(nullptr),
      m_globalNamespace(nullptr),
      m_typeSystem(nullptr)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_compiler::CompilerScanDriver::~CompilerScanDriver()
{
    delete m_typeSystem;
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::initialize(std::unique_ptr<BaseGrouping> &&rootGrouping)
{
    if (!m_groupings.empty())
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant,"module entry is already initialized");

    auto typeSystem = std::make_unique<lyric_typing::TypeSystem>(m_state);

    auto *fundamentalCache = m_state->fundamentalCache();
    auto *typeCache = m_state->typeCache();

    auto location = m_state->getLocation();

    // lookup the Function0 class and get its type handle
    auto functionClassUrl = fundamentalCache->getFunctionUrl(0);
    if (!functionClassUrl.isValid())
        m_state->throwAssemblerInvariant("missing Function0 symbol");

    tempo_utils::Status status;

    // ensure that NoReturn is in the type cache
    auto returnType = lyric_common::TypeDef::noReturn();
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(returnType));

    lyric_assembler::TypeHandle *entryTypeHandle;
    TU_ASSIGN_OR_RETURN (entryTypeHandle, typeCache->declareFunctionType(returnType, {}, {}));

    // create the $entry call
    lyric_common::SymbolUrl entryUrl(location, lyric_common::SymbolPath({"$entry"}));
    auto entryCall = std::make_unique<lyric_assembler::CallSymbol>(
        entryUrl, returnType, entryTypeHandle, m_state);
    TU_RETURN_IF_NOT_OK (m_state->appendCall(entryCall.get()));
    m_entryCall = entryCall.release();

    // resolve the Namespace type
    auto namespaceType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Namespace);
    lyric_assembler::TypeHandle *namespaceTypeHandle;
    TU_ASSIGN_OR_RETURN (namespaceTypeHandle, typeCache->getOrMakeType(namespaceType));

    // create the $global namespace
    lyric_common::SymbolUrl globalUrl(location, lyric_common::SymbolPath({"$global"}));
    auto globalNamespace = std::make_unique<lyric_assembler::NamespaceSymbol>(
        globalUrl, namespaceTypeHandle, m_entryCall->callProc(), m_state);
    TU_RETURN_IF_NOT_OK (m_state->appendNamespace(globalNamespace.get()));
    m_globalNamespace = globalNamespace.release();

    // take ownership of the typesystem
    m_typeSystem = typeSystem.release();

    // push the root grouping
    auto groupingData = std::make_unique<GroupingData>();
    groupingData->grouping = std::move(rootGrouping);
    groupingData->pending = true;
    m_groupings.push_back(std::move(groupingData));

    return {};
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::arrange(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &children)
{
    children.clear();
    for (int i = node->numChildren() - 1; i >= 0; i--) {
        children.emplace_back(node->getChild(i), i);
    }
    return {};
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (m_groupings.empty())
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "no handler available for node");
    auto &groupingData = m_groupings.back();
    auto &grouping = groupingData->grouping;

    TU_LOG_INFO << "PUSH ArchetypeNode@" << node;

    // if grouping is pending, then run before handler and return
    if (groupingData->pending) {
        BeforeContext beforeContext(ctx, this);
        TU_RETURN_IF_NOT_OK (grouping->before(state, node, beforeContext));
        auto handlers = beforeContext.takeHandlers();
        grouping->appendHandlers(std::move(handlers));
        groupingData->node = node;
        groupingData->pending = false;
        return {};
    }

    auto *handler = grouping->currentHandler();
    TU_ASSERT (handler != nullptr);

    switch (handler->type) {

        case HandlerType::Behavior: {
            auto &behavior = handler->behavior;
            EnterContext enterContext(ctx, this);
            TU_RETURN_IF_NOT_OK (behavior->enter(this, state, node, grouping->getBlock(), enterContext));
            auto handlers = enterContext.takeHandlers();
            grouping->appendHandlers(std::move(handlers));
            return {};
        }

        case HandlerType::Choice: {
            std::unique_ptr<BaseChoice> choice = std::move(handler->choice);
            handler->type = HandlerType::Invalid;
            DecideContext decideContext(ctx, this);
            TU_RETURN_IF_NOT_OK (choice->decide(state, node, decideContext));
            auto decision = decideContext.takeHandler();

            if (decision == nullptr)
                return {};

            switch (decision->type) {
                case HandlerType::Behavior:
                    handler->type = HandlerType::Behavior;
                    handler->behavior = std::move(decision->behavior);
                    return enter(state, node, ctx);
                case HandlerType::Choice:
                    handler->type = HandlerType::Choice;
                    handler->choice = std::move(decision->choice);
                    return enter(state, node, ctx);
                case HandlerType::Grouping:
                    handler->type = HandlerType::Grouping;
                    handler->grouping = std::move(decision->grouping);
                    return enter(state, node, ctx);
                case HandlerType::Invalid:
                    return {};
                default:
                    return CompilerStatus::forCondition(
                        CompilerCondition::kCompilerInvariant, "invalid decision for choice");
            }
        }

        case HandlerType::Grouping: {
            auto childGrouping = std::make_unique<GroupingData>();
            childGrouping->grouping = std::move(handler->grouping);
            childGrouping->pending = true;
            handler->type = HandlerType::Invalid;
            m_groupings.push_back(std::move(childGrouping));
            return enter(state, node, ctx);
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid handler for grouping");
    }
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (m_groupings.empty())
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "no grouping available for node");
    auto &groupingData = m_groupings.back();
    TU_ASSERT (!groupingData->pending);
    auto &grouping = groupingData->grouping;

    TU_LOG_INFO << "POP ArchetypeNode@" << node;

    // we have reached the end of the node group
    if (groupingData->node == node) {
        if (!grouping->isFinished())
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "unexpected handler at the end of the grouping");

        AfterContext afterContext(ctx, this);
        TU_RETURN_IF_NOT_OK (grouping->after(state, node, afterContext));
        m_groupings.pop_back();

        if (!m_groupings.empty()) {
            auto &parentData = m_groupings.back();
            auto &parentGrouping = parentData->grouping;
            parentGrouping->advanceHandler();
        }

        return {};
    }

    auto *handler = grouping->currentHandler();
    TU_ASSERT (handler != nullptr);

    switch (handler->type) {
        case HandlerType::Behavior: {
            auto &behavior = handler->behavior;
            ExitContext exitContext(ctx, this);
            TU_RETURN_IF_NOT_OK (behavior->exit(this, state, node, grouping->getBlock(), exitContext));
            break;
        }
        case HandlerType::Invalid:
            break;
        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "no grouping available for node");
    }

    grouping->advanceHandler();

    return {};
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::finish()
{
    if (!m_groupings.empty())
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "unexpected grouping on the stack");
    return {};
}

lyric_assembler::ObjectState *
lyric_compiler::CompilerScanDriver::getState() const
{
    return m_state;
}

lyric_assembler::FundamentalCache *
lyric_compiler::CompilerScanDriver::getFundamentalCache() const
{
    return m_state->fundamentalCache();
}

lyric_assembler::ImplCache *
lyric_compiler::CompilerScanDriver::getImplCache() const
{
    return m_state->implCache();
}

lyric_assembler::ImportCache *
lyric_compiler::CompilerScanDriver::getImportCache() const
{
    return m_state->importCache();
}

lyric_assembler::LiteralCache *
lyric_compiler::CompilerScanDriver::getLiteralCache() const
{
    return m_state->literalCache();
}

lyric_assembler::SymbolCache *
lyric_compiler::CompilerScanDriver::getSymbolCache() const
{
    return m_state->symbolCache();
}

lyric_assembler::TypeCache *
lyric_compiler::CompilerScanDriver::getTypeCache() const
{
    return m_state->typeCache();
}

lyric_assembler::NamespaceSymbol *
lyric_compiler::CompilerScanDriver::getGlobalNamespace() const
{
    return m_globalNamespace;
}

lyric_assembler::CallSymbol *
lyric_compiler::CompilerScanDriver::getEntryCall() const
{
    return m_entryCall;
}

lyric_typing::TypeSystem *
lyric_compiler::CompilerScanDriver::getTypeSystem() const
{
    return m_typeSystem;
}

lyric_compiler::BaseGrouping *
lyric_compiler::CompilerScanDriver::peekGrouping()
{
    if (m_groupings.empty())
        return nullptr;
    auto &groupingData = m_groupings.back();
    auto &grouping = groupingData->grouping;
    return grouping.get();
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::popGrouping()
{
    if (m_groupings.empty())
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "grouping stack is empty");
    m_groupings.pop_back();
    TU_LOG_INFO << "pop handler (" << (int) m_groupings.size() << " total)";
    return {};
}

tu_uint32
lyric_compiler::CompilerScanDriver::numGroupings() const
{
    return m_groupings.size();
}

lyric_common::TypeDef
lyric_compiler::CompilerScanDriver::peekResult()
{
    if (!m_results.empty())
        return m_results.top();
    return {};
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::pushResult(const lyric_common::TypeDef &result)
{
    m_results.push(result);
    TU_LOG_INFO << "push result " << result.toString() << " (" << (int) m_results.size() << " total)";
    return {};
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::popResult()
{
    if (m_results.empty())
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "results stack is empty");
    m_results.pop();
    TU_LOG_INFO << "pop result (" << (int) m_results.size() << " total)";
    return {};
}

tu_uint32
lyric_compiler::CompilerScanDriver::numResults() const
{
    return m_results.size();
}
