
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/impl_handle.h>

lyric_assembler::ImplCache::ImplCache(ObjectState *objectState)
    : m_objectState(objectState)
{
    TU_ASSERT (m_objectState != nullptr);
}

lyric_assembler::ImplCache::~ImplCache()
{
    for (auto *implHandle : m_importedImpls) {
        delete implHandle;
    }
    for (auto *implHandle : m_declaredImpls) {
        delete implHandle;
    }
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ImplCache::makeImpl(
    const lyric_common::TypeDef &consumerType,
    const lyric_common::SymbolUrl &receiverUrl,
    bool isDeclOnly,
    BlockHandle *parentBlock)
{
    auto name = absl::StrCat("$impl", m_declaredImpls.size());
    auto *implHandle = new ImplHandle(name, consumerType, receiverUrl, isDeclOnly,
        parentBlock, m_objectState);
    m_declaredImpls.push_back(implHandle);
    return implHandle;
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ImplCache::makeImpl(
    const lyric_common::TypeDef &consumerType,
    const lyric_common::SymbolUrl &receiverUrl,
    TemplateHandle *receiverTemplate,
    bool isDeclOnly,
    BlockHandle *parentBlock)
{
    auto name = absl::StrCat("$impl", m_declaredImpls.size());
    auto *implHandle = new ImplHandle(name, consumerType, receiverUrl, receiverTemplate, isDeclOnly,
        parentBlock, m_objectState);
    m_declaredImpls.push_back(implHandle);
    return implHandle;
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ImplCache::importImpl(std::shared_ptr<lyric_importer::ImplImport> implImport)
{
    TU_ASSERT (implImport != nullptr);
    auto *implHandle = new ImplHandle(implImport, m_objectState);
    m_importedImpls.push_back(implHandle);
    return implHandle;
}

std::vector<lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ImplCache::implsBegin() const
{
    return m_declaredImpls.cbegin();
}

std::vector<lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ImplCache::implsEnd() const
{
    return m_declaredImpls.cend();
}

int
lyric_assembler::ImplCache::numImpls() const
{
    return m_declaredImpls.size();
}

bool
lyric_assembler::ImplCache::hasEnvImpl(const lyric_common::TypeDef &consumerType) const
{
    return m_envImpls.contains(consumerType);
}

lyric_common::SymbolUrl
lyric_assembler::ImplCache::getEnvImpl(const lyric_common::TypeDef &consumerType) const
{
    auto iterator = m_envImpls.find(consumerType);
    if (iterator == m_envImpls.cend())
        return {};
    return iterator->second;
}

tempo_utils::Status
lyric_assembler::ImplCache::insertEnvImpl(
    const lyric_common::TypeDef &consumerType,
    const lyric_common::SymbolUrl &url)
{
    auto iterator = m_envImpls.find(consumerType);
    if (iterator != m_envImpls.cend())
        return AssemblerStatus::forCondition(AssemblerCondition::kImplConflict,
            "env impl {} is already set", consumerType.toString());
    m_envImpls[consumerType] = url;
    return {};
}