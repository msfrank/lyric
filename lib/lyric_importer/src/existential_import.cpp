
#include <lyric_importer/existential_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/existential_walker.h>
#include <lyric_object/object_types.h>

namespace lyric_importer {
    struct ExistentialImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isDeclOnly;
        lyric_object::DeriveType derive;
        lyric_object::AccessType access;
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
    : BaseImport(moduleImport),
      m_existentialOffset(existentialOffset)
{
    TU_ASSERT (m_existentialOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::ExistentialImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::ExistentialImport::isDeclOnly()
{
    load();
    return m_priv->isDeclOnly;
}

lyric_object::DeriveType
lyric_importer::ExistentialImport::getDerive()
{
    load();
    return m_priv->derive;
}

lyric_object::AccessType
lyric_importer::ExistentialImport::getAccess()
{
    load();
    return m_priv->access;
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

    auto moduleImport = getModuleImport();
    auto location = moduleImport->getLocation();
    auto existentialWalker = moduleImport->getObject().getObject().getExistential(m_existentialOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, existentialWalker.getSymbolPath());

    priv->isDeclOnly = existentialWalker.isDeclOnly();

    priv->derive = existentialWalker.getDeriveType();
    if (priv->derive == lyric_object::DeriveType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import existential at index {} in module {}; invalid derive type",
                m_existentialOffset, location.toString()));

    priv->access = existentialWalker.getAccess();
    if (priv->access == lyric_object::AccessType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import existential at index {} in module {}; invalid access type",
                m_existentialOffset, location.toString()));

    priv->existentialType = moduleImport->getType(
        existentialWalker.getExistentialType().getDescriptorOffset());

    if (existentialWalker.hasTemplate()) {
        priv->existentialTemplate = moduleImport->getTemplate(
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
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import existential at index {} in module {}; invalid super existential",
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
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import existential at index {} in module {}; invalid method at index {}",
                        m_existentialOffset, location.toString(), i));
        }
        auto name = callUrl.getSymbolName();
        priv->methods[name] = callUrl;
    }

    for (tu_uint8 i = 0; i < existentialWalker.numImpls(); i++) {
        auto implWalker = existentialWalker.getImpl(i);

        auto *implType = moduleImport->getType(implWalker.getImplType().getDescriptorOffset());
        auto *implImport = moduleImport->getImpl(implWalker.getDescriptorOffset());
        priv->impls[implType->getTypeDef()] = implImport;
    }

    for (tu_uint8 i = 0; i < existentialWalker.numSealedSubExistentials(); i++) {
        auto subExistentialType = existentialWalker.getSealedSubExistential(i);
        auto *sealedType = moduleImport->getType(subExistentialType.getDescriptorOffset());
        priv->sealedTypes.insert(sealedType->getTypeDef());
    }

    m_priv = std::move(priv);
}
