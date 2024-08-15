
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/undeclared_symbol.h>
#include <lyric_symbolizer/internal/symbolize_handle.h>

lyric_symbolizer::internal::SymbolizeHandle::SymbolizeHandle(lyric_assembler::ObjectState *state)
    : m_parent(nullptr), m_state(state)
{
    TU_ASSERT (m_state != nullptr);
    auto *scopeManager = m_state->scopeManager();
    m_span = scopeManager->makeSpan();
}

lyric_symbolizer::internal::SymbolizeHandle::SymbolizeHandle(
    const lyric_common::SymbolUrl &blockUrl,
    SymbolizeHandle *parent)
    : m_blockUrl(blockUrl), m_parent(parent)
{
    TU_ASSERT (m_blockUrl.isValid());
    TU_ASSERT (m_parent != nullptr);
    m_state = m_parent->m_state;
    auto *scopeManager = m_state->scopeManager();
    m_span = scopeManager->makeSpan();
}

lyric_symbolizer::internal::SymbolizeHandle *
lyric_symbolizer::internal::SymbolizeHandle::blockParent() const
{
    return m_parent;
}

lyric_assembler::ObjectState *
lyric_symbolizer::internal::SymbolizeHandle::blockState() const
{
    return m_state;
}

std::shared_ptr<tempo_tracing::TraceSpan>
lyric_symbolizer::internal::SymbolizeHandle::getSpan() const
{
    return m_span;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_symbolizer::internal::SymbolizeHandle::declareSymbol(
    const std::string &identifier,
    lyric_object::LinkageSection section)
{
    auto path = m_blockUrl.getSymbolPath();
    auto symbolUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(path.getPath(), identifier));
    auto *undecl = new lyric_assembler::UndeclaredSymbol(symbolUrl, section);
    auto status = m_state->appendUndeclared(undecl);
    if (status.notOk()) {
        delete undecl;
        return status;
    }
    return symbolUrl;
}

tempo_utils::Status
lyric_symbolizer::internal::SymbolizeHandle::declareImport(const lyric_common::ModuleLocation &location)
{
    auto *importCache = m_state->importCache();

    if (!importCache->hasImport(location)) {
        std::shared_ptr<lyric_importer::ModuleImport> moduleImport;
        TU_ASSIGN_OR_RETURN (moduleImport, importCache->importModule(location, lyric_assembler::ImportFlags::ApiLinkage));
        TU_RETURN_IF_NOT_OK (importCache->touchImport(moduleImport->getLocation()));
    }
    return SymbolizerStatus::ok();
}
