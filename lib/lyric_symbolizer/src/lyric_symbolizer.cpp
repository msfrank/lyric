
#include <lyric_assembler/object_state.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_symbolizer/lyric_symbolizer.h>
#include <lyric_symbolizer/symbolizer_result.h>
#include <lyric_symbolizer/symbolizer_scan_driver.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_utils/log_stream.h>

lyric_symbolizer::LyricSymbolizer::LyricSymbolizer(
    const lyric_common::ModuleLocation &origin,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    const SymbolizerOptions &options)
    : m_origin(origin),
      m_localModuleCache(std::move(localModuleCache)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_options(options)
{
    TU_ASSERT (m_origin.isValid());
}

lyric_symbolizer::LyricSymbolizer::LyricSymbolizer(const LyricSymbolizer &other)
    : m_localModuleCache(other.m_localModuleCache),
      m_systemModuleCache(other.m_systemModuleCache),
      m_options(other.m_options)
{
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_symbolizer::LyricSymbolizer::symbolizeModule(
    const lyric_common::ModuleLocation &location,
    const lyric_parser::LyricArchetype &archetype,
    const lyric_assembler::ObjectStateOptions &objectStateOptions,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    if (!location.isValid())
        return SymbolizerStatus::forCondition(
            SymbolizerCondition::kSymbolizerInvariant, "invalid module location");

    auto walker = archetype.getRoot();
    if (!walker.isValid())
        return SymbolizerStatus::forCondition(
            SymbolizerCondition::kSymbolizerInvariant, "invalid archetype");

    try {
        // create the trace context
        std::shared_ptr<tempo_tracing::TraceContext> context;
        if (recorder != nullptr) {
            TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
        } else {
            TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
        }

        // ensure context is released
        tempo_tracing::ReleaseContext releaser(context);

        // create the root span
        tempo_tracing::EnterScope scope("lyric_symbolizer::LyricSymbolizer::symbolizeModule");

        std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver;
        if (m_options.shortcutResolver != nullptr) {
            shortcutResolver = m_options.shortcutResolver;
        } else {
            shortcutResolver = std::make_shared<lyric_importer::ShortcutResolver>();
        }

        // construct the symbolizer state
        auto builder = std::make_shared<SymbolizerScanDriverBuilder>(location, m_origin,
            m_localModuleCache, m_systemModuleCache, shortcutResolver, objectStateOptions);

        lyric_rewriter::RewriterOptions rewriterOptions;
        rewriterOptions.visitorRegistry = m_options.visitorRegistry;

        lyric_rewriter::LyricRewriter rewriter(rewriterOptions);
        TU_RETURN_IF_NOT_OK (rewriter.scanArchetype(archetype, location.toUrl(), builder, recorder));

        // construct object from assembly state and return it
        return builder->toObject();

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}
