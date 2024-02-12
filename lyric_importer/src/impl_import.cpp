
#include <lyric_importer/impl_import.h>
#include <lyric_importer/importer_result.h>

namespace lyric_importer {
    struct ImplImport::Priv {
        TypeImport *implType;
        lyric_common::SymbolUrl implConcept;
        lyric_common::SymbolUrl receiverUrl;
        absl::flat_hash_map<std::string, Extension> extensions;
    };
}

lyric_importer::ImplImport::ImplImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 implOffset)
    : m_moduleImport(moduleImport),
      m_implOffset(implOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_implOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_importer::TypeImport *
lyric_importer::ImplImport::getImplType()
{
    load();
    return m_priv->implType;
}

lyric_common::SymbolUrl
lyric_importer::ImplImport::getImplConcept()
{
    load();
    return m_priv->implConcept;
}

lyric_common::SymbolUrl
lyric_importer::ImplImport::getReceiverUrl()
{
    load();
    return m_priv->receiverUrl;
}

absl::flat_hash_map<std::string,lyric_importer::Extension>::const_iterator
lyric_importer::ImplImport::extensionsBegin()
{
    load();
    return m_priv->extensions.cbegin();
}

absl::flat_hash_map<std::string,lyric_importer::Extension>::const_iterator
lyric_importer::ImplImport::extensionsEnd()
{
    load();
    return m_priv->extensions.cend();
}

int
lyric_importer::ImplImport::numExtensions()
{
    load();
    return m_priv->extensions.size();
}

void
lyric_importer::ImplImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto location = m_moduleImport->getLocation();
    auto implWalker = m_moduleImport->getObject().getObject().getImpl(m_implOffset);

    priv->receiverUrl = lyric_common::SymbolUrl(location, implWalker.getReceiver().getSymbolPath());

    priv->implType = m_moduleImport->getType(
        implWalker.getImplType().getDescriptorOffset());

    switch (implWalker.implConceptAddressType()) {
        case lyric_object::AddressType::Near:
            priv->implConcept = lyric_common::SymbolUrl(
                location, implWalker.getNearImplConcept().getSymbolPath());
            break;
        case lyric_object::AddressType::Far:
            priv->implConcept = implWalker.getFarImplConcept().getLinkUrl();
            break;
        default:
            throw ImporterException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import impl at index {} in assembly {}; invalid impl concept",
                    m_implOffset, location.toString()));
    }

    for (tu_uint8 i = 0; i < implWalker.numExtensions(); i++) {
        auto extensionWalker = implWalker.getExtension(i);

        Extension extension;

        switch (extensionWalker.actionAddressType()) {
            case lyric_object::AddressType::Near:
                extension.actionUrl = lyric_common::SymbolUrl(
                    location, extensionWalker.getNearAction().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                extension.actionUrl = extensionWalker.getFarAction().getLinkUrl();
                break;
            default:
                throw ImporterException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import impl at index {} in assembly {}; invalid extension at index {}",
                        m_implOffset, location.toString(), i));
        }

        switch (extensionWalker.callAddressType()) {
            case lyric_object::AddressType::Near:
                extension.callUrl = lyric_common::SymbolUrl(
                    location, extensionWalker.getNearCall().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                extension.callUrl = extensionWalker.getFarCall().getLinkUrl();
                break;
            default:
                throw ImporterException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import impl at index {} in assembly {}; invalid extension at index {}",
                        m_implOffset, location.toString(), i));
        }

        auto name = extension.actionUrl.getSymbolName();
        priv->extensions[name] = extension;
    }

    m_priv = std::move(priv);
}
