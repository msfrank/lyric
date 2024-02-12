
#include <lyric_importer/importer_result.h>
#include <lyric_importer/instance_import.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/instance_walker.h>

namespace lyric_importer {
    struct InstanceImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isAbstract;
        lyric_object::DeriveType derive;
        TypeImport *instanceType;
        lyric_common::SymbolUrl superInstance;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> members;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> methods;
        absl::flat_hash_map<lyric_common::TypeDef,ImplImport *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        tu_uint32 allocator;
    };
}

lyric_importer::InstanceImport::InstanceImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 instanceOffset)
    : m_moduleImport(moduleImport),
      m_instanceOffset(instanceOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_instanceOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::InstanceImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::InstanceImport::isAbstract()
{
    load();
    return m_priv->isAbstract;
}

lyric_object::DeriveType
lyric_importer::InstanceImport::getDerive()
{
    load();
    return m_priv->derive;
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
    return m_priv->allocator != lyric_object::INVALID_ADDRESS_U32;
}

tu_uint32
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

    auto location = m_moduleImport->getLocation();
    auto instanceWalker = m_moduleImport->getObject().getObject().getInstance(m_instanceOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, instanceWalker.getSymbolPath());

    priv->isAbstract = instanceWalker.isAbstract();

    if (instanceWalker.getDeriveType() == lyric_object::DeriveType::Invalid)
        throw ImporterException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import instance at index {} in assembly {}; invalid derive type",
                m_instanceOffset, location.toString()));
    priv->derive = instanceWalker.getDeriveType();

    priv->instanceType = m_moduleImport->getType(
        instanceWalker.getInstanceType().getDescriptorOffset());

    if (instanceWalker.hasSuperInstance()) {
        switch (instanceWalker.superInstanceAddressType()) {
            case lyric_object::AddressType::Near:
                priv->superInstance = lyric_common::SymbolUrl(
                    location, instanceWalker.getNearSuperInstance().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                priv->superInstance = instanceWalker.getFarSuperInstance().getLinkUrl();
                break;
            default:
                throw ImporterException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import instance at index {} in assembly {}; invalid super instance",
                        m_instanceOffset, location.toString()));
        }
    }

    for (tu_uint8 i = 0; i < instanceWalker.numMembers(); i++) {
        auto member = instanceWalker.getMember(i);
        lyric_common::SymbolUrl fieldUrl;
        switch (member.memberAddressType()) {
            case lyric_object::AddressType::Near:
                fieldUrl = lyric_common::SymbolUrl(location, member.getNearField().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                fieldUrl = member.getFarField().getLinkUrl();
                break;
            default:
                throw ImporterException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import instance at index {} in assembly {}; invalid member at index {}",
                        m_instanceOffset, location.toString(), i));
        }
        auto name = fieldUrl.getSymbolName();
        priv->members[name] = fieldUrl;
    }

    for (tu_uint8 i = 0; i < instanceWalker.numMethods(); i++) {
        auto method = instanceWalker.getMethod(i);
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
                        "cannot import instance at index {} in assembly {}; invalid method at index {}",
                        m_instanceOffset, location.toString(), i));
        }
        auto name = callUrl.getSymbolName();
        priv->methods[name] = callUrl;
    }

    for (tu_uint8 i = 0; i < instanceWalker.numImpls(); i++) {
        auto implWalker = instanceWalker.getImpl(i);

        auto *implType = m_moduleImport->getType(implWalker.getImplType().getDescriptorOffset());
        auto *implImport = m_moduleImport->getImpl(implWalker.getDescriptorOffset());
        priv->impls[implType->getTypeDef()] = implImport;
    }

    for (tu_uint8 i = 0; i < instanceWalker.numSealedSubInstances(); i++) {
        auto subInstanceType = instanceWalker.getSealedSubInstance(i);
        auto *sealedType = m_moduleImport->getType(subInstanceType.getDescriptorOffset());
        priv->sealedTypes.insert(sealedType->getTypeDef());
    }

    priv->allocator = instanceWalker.getAllocator();

    m_priv = std::move(priv);
}
