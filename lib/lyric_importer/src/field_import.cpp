
#include <lyric_importer/field_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_object/field_walker.h>

namespace lyric_importer {
    struct FieldImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isHidden;
        bool isVariable;
        bool isDeclOnly;
        TypeImport *fieldType;
        lyric_common::SymbolUrl initializer;
    };
}

lyric_importer::FieldImport::FieldImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 fieldOffset)
    : BaseImport(moduleImport),
      m_fieldOffset(fieldOffset)
{
    TU_ASSERT (m_fieldOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::FieldImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::FieldImport::isVariable()
{
    load();
    return m_priv->isVariable;
}

bool
lyric_importer::FieldImport::isDeclOnly()
{
    load();
    return m_priv->isDeclOnly;
}

bool
lyric_importer::FieldImport::isHidden()
{
    load();
    return m_priv->isHidden;
}

lyric_importer::TypeImport *
lyric_importer::FieldImport::getFieldType()
{
    load();
    return m_priv->fieldType;
}

lyric_common::SymbolUrl
lyric_importer::FieldImport::getInitializer()
{
    load();
    return m_priv->initializer;
}

void
lyric_importer::FieldImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = getModuleImport();
    auto objectLocation = moduleImport->getObjectLocation();
    auto fieldWalker = moduleImport->getObject().getField(m_fieldOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(objectLocation, fieldWalker.getSymbolPath());

    switch (fieldWalker.getAccess()) {
        case lyric_object::AccessType::Hidden:
            priv->isHidden = true;
            break;
        case lyric_object::AccessType::Public:
            priv->isHidden = false;
            break;
        default:
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import field at index {} in module {}; invalid access type",
                    m_fieldOffset, objectLocation.toString()));
    }

    priv->isVariable = fieldWalker.isVariable();
    priv->isDeclOnly = fieldWalker.isDeclOnly();

    priv->fieldType = moduleImport->getType(
        fieldWalker.getFieldType().getDescriptorOffset());

    if (fieldWalker.hasInitializer()) {
        switch (fieldWalker.initializerAddressType()) {
            case lyric_object::AddressType::Near: {
                priv->initializer = lyric_common::SymbolUrl(
                    objectLocation, fieldWalker.getNearInitializer().getSymbolPath());
                break;
            }
            case lyric_object::AddressType::Far: {
                priv->initializer = fieldWalker.getFarInitializer().getLinkUrl(objectLocation);
                break;
            }
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                        "cannot import field at index {} in module {}; invalid initializer",
                        fieldWalker.getDescriptorOffset(), objectLocation.toString()));
        }
    }

    m_priv = std::move(priv);
}
