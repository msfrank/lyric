
#include <lyric_importer/field_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_object/field_walker.h>

namespace lyric_importer {
    struct FieldImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        lyric_object::AccessType access;
        bool isVariable;
        TypeImport *fieldType;
        lyric_common::SymbolUrl initializer;
    };
}

lyric_importer::FieldImport::FieldImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 fieldOffset)
    : m_moduleImport(moduleImport),
      m_fieldOffset(fieldOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_fieldOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::FieldImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

lyric_object::AccessType
lyric_importer::FieldImport::getAccess()
{
    load();
    return m_priv->access;
}

bool
lyric_importer::FieldImport::isVariable()
{
    load();
    return m_priv->isVariable;
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

    auto location = m_moduleImport->getLocation();
    auto fieldWalker = m_moduleImport->getObject().getObject().getField(m_fieldOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, fieldWalker.getSymbolPath());

    if (fieldWalker.getAccess() == lyric_object::AccessType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import field at index {} in assembly {}; invalid access type",
                m_fieldOffset, location.toString()));
    priv->access = fieldWalker.getAccess();

    priv->isVariable = fieldWalker.isVariable();

    priv->fieldType = m_moduleImport->getType(
        fieldWalker.getFieldType().getDescriptorOffset());

    if (fieldWalker.hasInitializer()) {
        switch (fieldWalker.initializerAddressType()) {
            case lyric_object::AddressType::Near: {
                priv->initializer = lyric_common::SymbolUrl(
                    location, fieldWalker.getNearInitializer().getSymbolPath());
                break;
            }
            case lyric_object::AddressType::Far: {
                priv->initializer = fieldWalker.getFarInitializer().getLinkUrl();
                break;
            }
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                        "cannot import field at index {} in assembly {}; invalid initializer",
                        fieldWalker.getDescriptorOffset(), location.toString()));
        }
    }

    m_priv = std::move(priv);
}
