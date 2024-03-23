
#include <lyric_importer/existential_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/existential_walker.h>
#include <lyric_object/object_types.h>

namespace lyric_importer {
    struct ExistentialImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        lyric_object::DeriveType derive;
        TypeImport *existentialType;
        TemplateImport *existentialTemplate;
        lyric_common::SymbolUrl superExistential;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> methods;
        absl::flat_hash_map<lyric_common::TypeDef,ImplImport *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
    };
}

lyric_importer::ExistentialImport::ExistentialImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 existentialOffset)
    : m_moduleImport(moduleImport),
      m_existentialOffset(existentialOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_existentialOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::ExistentialImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

lyric_object::DeriveType
lyric_importer::ExistentialImport::getDerive()
{
    load();
    return m_priv->derive;
}

lyric_importer::TypeImport *
lyric_importer::ExistentialImport::getExistentialType()
{
    load();
    return m_priv->existentialType;
}

lyric_importer::TemplateImport *
lyric_importer::ExistentialImport::getExistentialTemplate()
{
    load();
    return m_priv->existentialTemplate;
}

lyric_common::SymbolUrl
lyric_importer::ExistentialImport::getSuperExistential()
{
    load();
    return m_priv->superExistential;
}

lyric_common::SymbolUrl
lyric_importer::ExistentialImport::getMethod(std::string_view name)
{
    load();
    if (m_priv->methods.contains(name))
        return m_priv->methods.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::ExistentialImport::methodsBegin()
{
    load();
    return m_priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::ExistentialImport::methodsEnd()
{
    load();
    return m_priv->methods.cend();
}

tu_uint8
lyric_importer::ExistentialImport::numMethods()
{
    load();
    return m_priv->methods.size();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_importer::ImplImport *>::const_iterator
lyric_importer::ExistentialImport::implsBegin()
{
    load();
    return m_priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_importer::ImplImport *>::const_iterator
lyric_importer::ExistentialImport::implsEnd()
{
    load();
    return m_priv->impls.cend();
}

tu_uint8
lyric_importer::ExistentialImport::numImpls()
{
    load();
    return m_priv->impls.size();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::ExistentialImport::sealedTypesBegin()
{
    load();
    return m_priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::ExistentialImport::sealedTypesEnd()
{
    load();
    return m_priv->sealedTypes.cend();
}

int
lyric_importer::ExistentialImport::numSealedTypes()
{
    load();
    return m_priv->sealedTypes.size();
}

void
lyric_importer::ExistentialImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto location = m_moduleImport->getLocation();
    auto existentialWalker = m_moduleImport->getObject().getObject().getExistential(m_existentialOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, existentialWalker.getSymbolPath());

    if (existentialWalker.getDeriveType() == lyric_object::DeriveType::Invalid)
        throw ImporterException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import existential at index {} in assembly {}; invalid derive type",
                m_existentialOffset, location.toString()));
    priv->derive = existentialWalker.getDeriveType();

    priv->existentialType = m_moduleImport->getType(
        existentialWalker.getExistentialType().getDescriptorOffset());

    if (existentialWalker.hasTemplate()) {
        priv->existentialTemplate = m_moduleImport->getTemplate(
            existentialWalker.getTemplate().getDescriptorOffset());
    } else {
        priv->existentialTemplate = nullptr;
    }

    if (existentialWalker.hasSuperExistential()) {
        switch (existentialWalker.superExistentialAddressType()) {
            case lyric_object::AddressType::Near:
                priv->superExistential = lyric_common::SymbolUrl(location,
                    existentialWalker.getNearSuperExistential().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                priv->superExistential = existentialWalker.getFarSuperExistential().getLinkUrl();
                break;
            default:
                throw ImporterException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import existential at index {} in assembly {}; invalid super existential",
                        m_existentialOffset, location.toString()));
        }
    }

    for (tu_uint8 i = 0; i < existentialWalker.numMethods(); i++) {
        auto method = existentialWalker.getMethod(i);
        lyric_common::SymbolUrl callUrl;
        switch (method.methodAddressType()) {
            case lyric_object::AddressType::Near:
                callUrl = lyric_common::SymbolUrl(location, method.getNearCall().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                callUrl = method.getFarCall().getLinkUrl();
                break;
            default:
                throw ImporterException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import existential at index {} in assembly {}; invalid method at index {}",
                        m_existentialOffset, location.toString(), i));
        }
        auto name = callUrl.getSymbolName();
        priv->methods[name] = callUrl;
    }

    for (tu_uint8 i = 0; i < existentialWalker.numImpls(); i++) {
        auto implWalker = existentialWalker.getImpl(i);

        auto *implType = m_moduleImport->getType(implWalker.getImplType().getDescriptorOffset());
        auto *implImport = m_moduleImport->getImpl(implWalker.getDescriptorOffset());
        priv->impls[implType->getTypeDef()] = implImport;
    }

    for (tu_uint8 i = 0; i < existentialWalker.numSealedSubExistentials(); i++) {
        auto subExistentialType = existentialWalker.getSealedSubExistential(i);
        auto *sealedType = m_moduleImport->getType(subExistentialType.getDescriptorOffset());
        priv->sealedTypes.insert(sealedType->getTypeDef());
    }

    m_priv = std::move(priv);
}
