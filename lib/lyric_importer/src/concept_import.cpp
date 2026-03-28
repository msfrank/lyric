
#include <lyric_importer/concept_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/concept_walker.h>


lyric_importer::ConceptImport::ConceptImport(
    std::weak_ptr<ModuleImport> moduleImport,
    tu_uint32 conceptOffset)
    : BaseImport(std::move(moduleImport)),
      m_conceptOffset(conceptOffset)
{
    TU_ASSERT (m_conceptOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::ConceptImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::ConceptImport::isDeclOnly()
{
    load();
    return m_priv->isDeclOnly;
}

lyric_object::DeriveType
lyric_importer::ConceptImport::getDerive()
{
    load();
    return m_priv->derive;
}

bool
lyric_importer::ConceptImport::isHidden()
{
    load();
    return m_priv->isHidden;
}

std::weak_ptr<lyric_importer::TypeImport>
lyric_importer::ConceptImport::getConceptType()
{
    load();
    return m_priv->conceptType;
}

lyric_common::SymbolUrl
lyric_importer::ConceptImport::getSuperConcept()
{
    load();
    return m_priv->superConcept;
}

bool
lyric_importer::ConceptImport::hasConceptTemplate()
{
    load();
    return m_priv->hasTemplate;
}

std::weak_ptr<lyric_importer::TemplateImport>
lyric_importer::ConceptImport::getConceptTemplate()
{
    load();
    return m_priv->conceptTemplate;
}

lyric_common::SymbolUrl
lyric_importer::ConceptImport::getAction(std::string_view name)
{
    load();
    if (m_priv->actions.contains(name))
        return m_priv->actions.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::ConceptImport::actionsBegin()
{
    load();
    return m_priv->actions.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::ConceptImport::actionsEnd()
{
    load();
    return m_priv->actions.cend();
}

tu_uint8
lyric_importer::ConceptImport::numActions()
{
    load();
    return m_priv->actions.size();
}

absl::flat_hash_map<lyric_common::TypeDef,std::weak_ptr<lyric_importer::ImplImport>>::const_iterator
lyric_importer::ConceptImport::implsBegin()
{
    load();
    return m_priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,std::weak_ptr<lyric_importer::ImplImport>>::const_iterator
lyric_importer::ConceptImport::implsEnd()
{
    load();
    return m_priv->impls.cend();
}

tu_uint8
lyric_importer::ConceptImport::numImpls()
{
    load();
    return m_priv->impls.size();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::ConceptImport::sealedTypesBegin()
{
    load();
    return m_priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::ConceptImport::sealedTypesEnd()
{
    load();
    return m_priv->sealedTypes.cend();
}

int
lyric_importer::ConceptImport::numSealedTypes()
{
    load();
    return m_priv->sealedTypes.size();
}

void
lyric_importer::ConceptImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = acquireModuleImport();
    if (moduleImport == nullptr)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(ImporterCondition::kImporterInvariant,
                "invalid module import"));

    auto objectLocation = moduleImport->getObjectLocation();
    auto conceptWalker = moduleImport->getObject().getConcept(m_conceptOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(objectLocation, conceptWalker.getSymbolPath());

    priv->isDeclOnly = conceptWalker.isDeclOnly();

    priv->derive = conceptWalker.getDeriveType();
    if (priv->derive == lyric_object::DeriveType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import concept at index {} in module {}; invalid derive type",
                m_conceptOffset, objectLocation.toString()));

    switch (conceptWalker.getAccess()) {
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
                    "cannot import concept at index {} in module {}; invalid access type",
                    m_conceptOffset, objectLocation.toString()));
    }

    priv->conceptType = moduleImport->getType(
        conceptWalker.getConceptType().getDescriptorOffset());

    priv->hasTemplate = conceptWalker.hasTemplate();
    if (priv->hasTemplate) {
        priv->conceptTemplate = moduleImport->getTemplate(
            conceptWalker.getTemplate().getDescriptorOffset());
    }

    if (conceptWalker.hasSuperConcept()) {
        switch (conceptWalker.superConceptAddressType()) {
            case lyric_object::AddressType::Near:
                priv->superConcept = lyric_common::SymbolUrl(
                    objectLocation, conceptWalker.getNearSuperConcept().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                priv->superConcept = conceptWalker.getFarSuperConcept().getLinkUrl(objectLocation);
                break;
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import concept at index {} in module {}; invalid super concept",
                        m_conceptOffset, objectLocation.toString()));
        }
    }

    for (tu_uint8 i = 0; i < conceptWalker.numActions(); i++) {
        auto action = conceptWalker.getAction(i);
        lyric_common::SymbolUrl actionUrl;
        switch (action.actionAddressType()) {
            case lyric_object::AddressType::Near:
                actionUrl = lyric_common::SymbolUrl(objectLocation, action.getNearAction().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                actionUrl = action.getFarAction().getLinkUrl(objectLocation);
                break;
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import concept at index {} in module {}; invalid action at index {}",
                        m_conceptOffset, objectLocation.toString(), i));
        }
        auto name = actionUrl.getSymbolName();
        priv->actions[name] = actionUrl;
    }

    for (tu_uint8 i = 0; i < conceptWalker.numImpls(); i++) {
        auto implWalker = conceptWalker.getImpl(i);

        auto implType = moduleImport->getType(implWalker.getImplType().getDescriptorOffset());
        auto implImport = moduleImport->getImpl(implWalker.getDescriptorOffset());
        priv->impls[implType->getTypeDef()] = implImport;
    }

    for (tu_uint8 i = 0; i < conceptWalker.numSealedSubConcepts(); i++) {
        auto subConceptType = conceptWalker.getSealedSubConcept(i);
        auto sealedType = moduleImport->getType(subConceptType.getDescriptorOffset());
        priv->sealedTypes.insert(sealedType->getTypeDef());
    }

    m_priv = std::move(priv);
}
