
#include <lyric_importer/importer_result.h>
#include <lyric_importer/instance_import.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/instance_walker.h>

namespace lyric_importer {
    struct InstanceImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isDeclOnly;
        lyric_object::DeriveType derive;
        bool isHidden;
        TypeImport *instanceType;
        lyric_common::SymbolUrl superInstance;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> members;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> methods;
        absl::flat_hash_map<lyric_common::TypeDef,ImplImport *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        std::string allocator;
    };
}

lyric_importer::InstanceImport::InstanceImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 instanceOffset)
    : BaseImport(moduleImport),
      m_instanceOffset(instanceOffset)
{
    TU_ASSERT (m_instanceOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::InstanceImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::InstanceImport::isDeclOnly()
{
    load();
    return m_priv->isDeclOnly;
}

lyric_object::DeriveType
lyric_importer::InstanceImport::getDerive()
{
    load();
    return m_priv->derive;
}

bool
lyric_importer::InstanceImport::isHidden()
{
    load();
    return m_priv->isHidden;
}

lyric_importer::TypeImport *
lyric_importer::InstanceImport::getInstanceType()
{
    load();
    return m_priv->instanceType;
}

lyric_common::SymbolUrl
lyric_importer::InstanceImport::getSuperInstance()
{
    load();
    return m_priv->superInstance;
}

lyric_common::SymbolUrl
lyric_importer::InstanceImport::getMember(std::string_view name)
{
    load();
    if (m_priv->members.contains(name))
        return m_priv->members.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::InstanceImport::membersBegin()
{
    load();
    return m_priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::InstanceImport::membersEnd()
{
    load();
    return m_priv->members.cend();
}

tu_uint8
lyric_importer::InstanceImport::numMembers()
{
    load();
    return m_priv->members.size();
}

lyric_common::SymbolUrl
lyric_importer::InstanceImport::getMethod(std::string_view name)
{
    load();
    if (m_priv->methods.contains(name))
        return m_priv->methods.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::InstanceImport::methodsBegin()
{
    load();
    return m_priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::InstanceImport::methodsEnd()
{
    load();
    return m_priv->methods.cend();
}

tu_uint8
lyric_importer::InstanceImport::numMethods()
{
    load();
    return m_priv->methods.size();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_importer::ImplImport *>::const_iterator
lyric_importer::InstanceImport::implsBegin()
{
    load();
    return m_priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_importer::ImplImport *>::const_iterator
lyric_importer::InstanceImport::implsEnd()
{
    load();
    return m_priv->impls.cend();
}

tu_uint8
lyric_importer::InstanceImport::numImpls()
{
    load();
    return m_priv->impls.size();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::InstanceImport::sealedTypesBegin()
{
    load();
    return m_priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::InstanceImport::sealedTypesEnd()
{
    load();
    return m_priv->sealedTypes.cend();
}

int
lyric_importer::InstanceImport::numSealedTypes()
{
    load();
    return m_priv->sealedTypes.size();
}

bool
lyric_importer::InstanceImport::hasAllocator()
{
    return !m_priv->allocator.empty();
}

std::string
lyric_importer::InstanceImport::getAllocator()
{
    return m_priv->allocator;
}

void
lyric_importer::InstanceImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = getModuleImport();
    auto objectLocation = moduleImport->getObjectLocation();
    auto instanceWalker = moduleImport->getObject().getObject().getInstance(m_instanceOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(objectLocation, instanceWalker.getSymbolPath());

    priv->isDeclOnly = instanceWalker.isDeclOnly();

    priv->derive = instanceWalker.getDeriveType();
    if (priv->derive == lyric_object::DeriveType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import instance at index {} in module {}; invalid derive type",
                m_instanceOffset, objectLocation.toString()));

    switch (instanceWalker.getAccess()) {
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
                    "cannot import instance at index {} in module {}; invalid access type",
                    m_instanceOffset, objectLocation.toString()));
    }

    priv->instanceType = moduleImport->getType(
        instanceWalker.getInstanceType().getDescriptorOffset());

    if (instanceWalker.hasSuperInstance()) {
        switch (instanceWalker.superInstanceAddressType()) {
            case lyric_object::AddressType::Near:
                priv->superInstance = lyric_common::SymbolUrl(
                    objectLocation, instanceWalker.getNearSuperInstance().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                priv->superInstance = instanceWalker.getFarSuperInstance().getLinkUrl(objectLocation);
                break;
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import instance at index {} in module {}; invalid super instance",
                        m_instanceOffset, objectLocation.toString()));
        }
    }

    for (tu_uint8 i = 0; i < instanceWalker.numMembers(); i++) {
        auto member = instanceWalker.getMember(i);
        if (!member.isValid()) {
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import instance at index {} in module {}; invalid member at index {}",
                    m_instanceOffset, objectLocation.toString(), i));
        }

        lyric_common::SymbolUrl fieldUrl(objectLocation, member.getSymbolPath());

        auto name = fieldUrl.getSymbolName();
        priv->members[name] = fieldUrl;
    }

    for (tu_uint8 i = 0; i < instanceWalker.numMethods(); i++) {
        auto method = instanceWalker.getMethod(i);
        if (!method.isValid()) {
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import instance at index {} in module {}; invalid method at index {}",
                    m_instanceOffset, objectLocation.toString(), i));
        }

        lyric_common::SymbolUrl callUrl(objectLocation, method.getSymbolPath());

        auto name = callUrl.getSymbolName();
        priv->methods[name] = callUrl;
    }

    for (tu_uint8 i = 0; i < instanceWalker.numImpls(); i++) {
        auto impl = instanceWalker.getImpl(i);
        if (!impl.isValid()) {
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import instance at index {} in module {}; invalid impl at index {}",
                    m_instanceOffset, objectLocation.toString(), i));
        }

        auto *implType = moduleImport->getType(impl.getImplType().getDescriptorOffset());
        auto *implImport = moduleImport->getImpl(impl.getDescriptorOffset());
        priv->impls[implType->getTypeDef()] = implImport;
    }

    for (tu_uint8 i = 0; i < instanceWalker.numSealedSubInstances(); i++) {
        auto subInstanceType = instanceWalker.getSealedSubInstance(i);
        auto *sealedType = moduleImport->getType(subInstanceType.getDescriptorOffset());
        priv->sealedTypes.insert(sealedType->getTypeDef());
    }

    auto trapNumber = instanceWalker.getAllocator();
    if (trapNumber != lyric_object::INVALID_ADDRESS_U32) {
        auto plugin = moduleImport->getPlugin();
        if (plugin == nullptr)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import instance at index {} in module {}; invalid allocator trap",
                    m_instanceOffset, objectLocation.toString()));
        auto *trap = plugin->getTrap(trapNumber);
        if (trap == nullptr)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import instance at index {} in module {}; invalid allocator trap",
                    m_instanceOffset, objectLocation.toString()));
        priv->allocator = trap->name;
    }

    m_priv = std::move(priv);
}
