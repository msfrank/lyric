
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/object_root.h>
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

#include "lyric_compiler/entry_handler.h"

lyric_compiler::CompilerScanDriver::CompilerScanDriver(
    lyric_assembler::ObjectRoot *root,
    lyric_assembler::ObjectState *state)
    : m_root(root),
      m_state(state),
      m_typeSystem(nullptr)
{
    TU_ASSERT (m_root != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_compiler::CompilerScanDriver::~CompilerScanDriver()
{
    delete m_typeSystem;
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::initialize(std::unique_ptr<BaseGrouping> &&rootGrouping)
{
    if (m_typeSystem != nullptr)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "module entry is already initialized");

    m_typeSystem = new lyric_typing::TypeSystem(m_state);

    // push the root grouping
    auto groupingData = std::make_unique<GroupingData>();
    groupingData->grouping = std::move(rootGrouping);
    groupingData->pending = true;
    m_groupings.push_back(std::move(groupingData));

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

lyric_assembler::ObjectRoot *
lyric_compiler::CompilerScanDriver::getObjectRoot() const
{
    return m_root;
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

lyric_compiler::CompilerScanDriverBuilder::CompilerScanDriverBuilder(
    const lyric_common::ModuleLocation &location,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
    tempo_tracing::ScopeManager *scopeManager,
    const lyric_assembler::ObjectStateOptions &objectStateOptions)
    : m_location(location),
      m_localModuleCache(std::move(localModuleCache)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_shortcutResolver(std::move(shortcutResolver)),
      m_scopeManager(scopeManager),
      m_objectStateOptions(objectStateOptions)
{
}

tempo_utils::Status
lyric_compiler::CompilerScanDriverBuilder::applyPragma(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node)
{
    if (node->isNamespace(lyric_schema::kLyricAssemblerNs)) {
        lyric_schema::LyricAssemblerId assemblerId;
        TU_RETURN_IF_NOT_OK (node->parseId(lyric_schema::kLyricAssemblerVocabulary, assemblerId));
        switch (assemblerId) {
            case lyric_schema::LyricAssemblerId::Plugin: {
                TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstModuleLocation, m_pluginLocation));
                break;
            }
            default:
                break;
        }
    }
    return {};
}

tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractScanDriver>>
lyric_compiler::CompilerScanDriverBuilder::makeScanDriver()
{
    // construct the object state
    if (m_pluginLocation.isValid()) {
        m_state = std::make_unique<lyric_assembler::ObjectState>(m_location, m_localModuleCache,
            m_systemModuleCache, m_shortcutResolver, m_scopeManager, m_pluginLocation, m_objectStateOptions);
    } else {
        m_state = std::make_unique<lyric_assembler::ObjectState>(m_location, m_localModuleCache,
            m_systemModuleCache, m_shortcutResolver, m_scopeManager, m_objectStateOptions);
    }

    // define the object root
    lyric_assembler::ObjectRoot *root;
    TU_ASSIGN_OR_RETURN (root, m_state->defineRoot());

    // initialize the driver
    auto driver = std::make_shared<CompilerScanDriver>(root, m_state.get());
    auto rootHandler = std::make_unique<EntryHandler>(driver.get());
    TU_RETURN_IF_NOT_OK (driver->initialize(std::move(rootHandler)));

    return std::static_pointer_cast<lyric_rewriter::AbstractScanDriver>(driver);
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_compiler::CompilerScanDriverBuilder::toObject() const
{
    return m_state->toObject();
}
