
#include <lyric_archiver/archiver_state.h>
#include <lyric_archiver/symbol_reference_set.h>

lyric_archiver::SymbolReferenceSet::SymbolReferenceSet(ArchiverState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

bool
lyric_archiver::SymbolReferenceSet::isEmpty() const
{
    return m_copyReferences.empty();
}

tempo_utils::Status
lyric_archiver::SymbolReferenceSet::insertReference(const lyric_common::SymbolUrl &symbolUrl)
{
    auto location = symbolUrl.getModuleLocation();
    if (m_state->hasImport(location)) {
        m_copyReferences.insert(symbolUrl);
    }
    return {};
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_archiver::SymbolReferenceSet::takeReference()
{
    if (m_copyReferences.empty())
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "symbol reference set is empty");
    auto it = m_copyReferences.begin();
    auto symbolUrl = *it;
    m_copyReferences.erase(it);
    return symbolUrl;
}