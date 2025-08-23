
#include <lyric_importer/enum_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/enum_walker.h>

namespace lyric_importer {
    struct EnumImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isAbstract;
        bool isDeclOnly;
        lyric_object::DeriveType derive;
        bool isHidden;
        TypeImport *enumType;
        lyric_common::SymbolUrl superEnum;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> members;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> methods;
        absl::flat_hash_map<lyric_common::TypeDef,ImplImport *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        std::string allocator;
    };
}

lyric_importer::EnumImport::EnumImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 enumOffset)
    : BaseImport(moduleImport),
      m_enumOffset(enumOffset)
{
    TU_ASSERT (m_enumOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::EnumImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::EnumImport::isAbstract()
{
    load();
    return m_priv->isAbstract;
}

bool
lyric_importer::EnumImport::isDeclOnly()
{
    load();
    return m_priv->isDeclOnly;
}

lyric_object::DeriveType
lyric_importer::EnumImport::getDerive()
{
    load();
    return m_priv->derive;
}

bool
lyric_importer::EnumImport::isHidden()
{
    load();
    return m_priv->isHidden;
}

lyric_importer::TypeImport *
lyric_importer::EnumImport::getEnumType()
{
    load();
    return m_priv->enumType;
}

lyric_common::SymbolUrl
lyric_importer::EnumImport::getSuperEnum()
{
    load();
    return m_priv->superEnum;
}

lyric_common::SymbolUrl
lyric_importer::EnumImport::getMember(std::string_view name)
{
    load();
    if (m_priv->members.contains(name))
        return m_priv->members.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::EnumImport::membersBegin()
{
    load();
    return m_priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::EnumImport::membersEnd()
{
    load();
    return m_priv->members.cend();
}

tu_uint8
lyric_importer::EnumImport::numMembers()
{
    load();
    return m_priv->members.size();
}

lyric_common::SymbolUrl
lyric_importer::EnumImport::getMethod(std::string_view name)
{
    load();
    if (m_priv->methods.contains(name))
        return m_priv->methods.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::EnumImport::methodsBegin()
{
    load();
    return m_priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::EnumImport::methodsEnd()
{
    load();
    return m_priv->methods.cend();
}

tu_uint8
lyric_importer::EnumImport::numMethods()
{
    load();
    return m_priv->methods.size();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_importer::ImplImport *>::const_iterator
lyric_importer::EnumImport::implsBegin()
{
    load();
    return m_priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_importer::ImplImport *>::const_iterator
lyric_importer::EnumImport::implsEnd()
{
    load();
    return m_priv->impls.cend();
}

tu_uint8
lyric_importer::EnumImport::numImpls()
{
    load();
    return m_priv->impls.size();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::EnumImport::sealedTypesBegin()
{
    load();
    return m_priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::EnumImport::sealedTypesEnd()
{
    load();
    return m_priv->sealedTypes.cend();
}

int
lyric_importer::EnumImport::numSealedTypes()
{
    load();
    return m_priv->sealedTypes.size();
}

bool
lyric_importer::EnumImport::hasAllocator()
{
    return !m_priv->allocator.empty();
}

std::string
lyric_importer::EnumImport::getAllocator()
{
    return m_priv->allocator;
}

void
lyric_importer::EnumImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = getModuleImport();
    auto objectLocation = moduleImport->getObjectLocation();
    auto enumWalker = moduleImport->getObject().getObject().getEnum(m_enumOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(objectLocation, enumWalker.getSymbolPath());

    priv->isAbstract = enumWalker.isAbstract();
    priv->isDeclOnly = enumWalker.isDeclOnly();

    priv->derive = enumWalker.getDeriveType();
    if (priv->derive == lyric_object::DeriveType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import enum at index {} in module {}; invalid derive type",
                m_enumOffset, objectLocation.toString()));

    switch (enumWalker.getAccess()) {
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
                    "cannot import enum at index {} in module {}; invalid access type",
                    m_enumOffset, objectLocation.toString()));
    }

    priv->enumType = moduleImport->getType(
        enumWalker.getEnumType().getDescriptorOffset());

    if (enumWalker.hasSuperEnum()) {
        switch (enumWalker.superEnumAddressType()) {
            case lyric_object::AddressType::Near:
                priv->superEnum = lyric_common::SymbolUrl(
                    objectLocation, enumWalker.getNearSuperEnum().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                priv->superEnum = enumWalker.getFarSuperEnum().getLinkUrl(objectLocation);
                break;
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import enum at index {} in module {}; invalid super enum",
                        m_enumOffset, objectLocation.toString()));
        }
    }

    for (tu_uint8 i = 0; i < enumWalker.numMembers(); i++) {
        auto member = enumWalker.getMember(i);
        lyric_common::SymbolUrl fieldUrl;
        switch (member.memberAddressType()) {
            case lyric_object::AddressType::Near:
                fieldUrl = lyric_common::SymbolUrl(objectLocation, member.getNearField().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                fieldUrl = member.getFarField().getLinkUrl(objectLocation);
                break;
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import enum at index {} in module {}; invalid member at index {}",
                        m_enumOffset, objectLocation.toString(), i));
        }
        auto name = fieldUrl.getSymbolName();
        priv->members[name] = fieldUrl;
    }

    for (tu_uint8 i = 0; i < enumWalker.numMethods(); i++) {
        auto method = enumWalker.getMethod(i);
        lyric_common::SymbolUrl callUrl;
        switch (method.methodAddressType()) {
            case lyric_object::AddressType::Near:
                callUrl = lyric_common::SymbolUrl(objectLocation, method.getNearCall().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                callUrl = method.getFarCall().getLinkUrl(objectLocation);
                break;
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import enum at index {} in module {}; invalid method at index {}",
                        m_enumOffset, objectLocation.toString(), i));
        }
        auto name = callUrl.getSymbolName();
        priv->methods[name] = callUrl;
    }

    for (tu_uint8 i = 0; i < enumWalker.numImpls(); i++) {
        auto implWalker = enumWalker.getImpl(i);

        auto *implType = moduleImport->getType(implWalker.getImplType().getDescriptorOffset());
        auto *implImport = moduleImport->getImpl(implWalker.getDescriptorOffset());
        priv->impls[implType->getTypeDef()] = implImport;
    }

    for (tu_uint8 i = 0; i < enumWalker.numSealedSubEnums(); i++) {
        auto subEnumType = enumWalker.getSealedSubEnum(i);
        auto *sealedType = moduleImport->getType(subEnumType.getDescriptorOffset());
        priv->sealedTypes.insert(sealedType->getTypeDef());
    }

    auto trapNumber = enumWalker.getAllocator();
    if (trapNumber != lyric_object::INVALID_ADDRESS_U32) {
        auto plugin = moduleImport->getPlugin();
        if (plugin == nullptr)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import enum at index {} in module {}; invalid allocator trap",
                    m_enumOffset, objectLocation.toString()));
        auto *trap = plugin->getTrap(trapNumber);
        if (trap == nullptr)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import enum at index {} in module {}; invalid allocator trap",
                    m_enumOffset, objectLocation.toString()));
        priv->allocator = trap->name;
    }

    m_priv = std::move(priv);
}
