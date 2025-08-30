
#include <lyric_importer/importer_result.h>
#include <lyric_importer/struct_import.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/struct_walker.h>

namespace lyric_importer {
    struct StructImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isDeclOnly;
        lyric_object::DeriveType derive;
        bool isHidden;
        TypeImport *structType;
        lyric_common::SymbolUrl superStruct;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> members;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> methods;
        absl::flat_hash_map<lyric_common::TypeDef,ImplImport *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        std::string allocator;
    };
}

lyric_importer::StructImport::StructImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 structOffset)
    : BaseImport(moduleImport),
      m_structOffset(structOffset)
{
    TU_ASSERT (m_structOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::StructImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::StructImport::isDeclOnly()
{
    load();
    return m_priv->isDeclOnly;
}

lyric_object::DeriveType
lyric_importer::StructImport::getDerive()
{
    load();
    return m_priv->derive;
}

bool
lyric_importer::StructImport::isHidden()
{
    load();
    return m_priv->isHidden;
}

lyric_importer::TypeImport *
lyric_importer::StructImport::getStructType()
{
    load();
    return m_priv->structType;
}

lyric_common::SymbolUrl
lyric_importer::StructImport::getSuperStruct()
{
    load();
    return m_priv->superStruct;
}

lyric_common::SymbolUrl
lyric_importer::StructImport::getMember(std::string_view name)
{
    load();
    if (m_priv->members.contains(name))
        return m_priv->members.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::StructImport::membersBegin()
{
    load();
    return m_priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::StructImport::membersEnd()
{
    load();
    return m_priv->members.cend();
}

tu_uint8
lyric_importer::StructImport::numMembers()
{
    load();
    return m_priv->members.size();
}

lyric_common::SymbolUrl
lyric_importer::StructImport::getMethod(std::string_view name)
{
    load();
    if (m_priv->methods.contains(name))
        return m_priv->methods.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::StructImport::methodsBegin()
{
    load();
    return m_priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::StructImport::methodsEnd()
{
    load();
    return m_priv->methods.cend();
}

tu_uint8
lyric_importer::StructImport::numMethods()
{
    load();
    return m_priv->methods.size();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_importer::ImplImport *>::const_iterator
lyric_importer::StructImport::implsBegin()
{
    load();
    return m_priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_importer::ImplImport *>::const_iterator
lyric_importer::StructImport::implsEnd()
{
    load();
    return m_priv->impls.cend();
}

tu_uint8
lyric_importer::StructImport::numImpls()
{
    load();
    return m_priv->impls.size();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::StructImport::sealedTypesBegin()
{
    load();
    return m_priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::StructImport::sealedTypesEnd()
{
    load();
    return m_priv->sealedTypes.cend();
}

int
lyric_importer::StructImport::numSealedTypes()
{
    load();
    return m_priv->sealedTypes.size();
}

bool
lyric_importer::StructImport::hasAllocator()
{
    return !m_priv->allocator.empty();
}

std::string
lyric_importer::StructImport::getAllocator()
{
    return m_priv->allocator;
}

void
lyric_importer::StructImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = getModuleImport();
    auto objectLocation = moduleImport->getObjectLocation();
    auto structWalker = moduleImport->getObject().getStruct(m_structOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(objectLocation, structWalker.getSymbolPath());

    priv->isDeclOnly = structWalker.isDeclOnly();

    priv->derive = structWalker.getDeriveType();
    if (priv->derive == lyric_object::DeriveType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import struct at index {} in module {}; invalid derive type",
                m_structOffset, objectLocation.toString()));

    switch (structWalker.getAccess()) {
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
                    "cannot import struct at index {} in module {}; invalid access type",
                    m_structOffset, objectLocation.toString()));
    }

    priv->structType = moduleImport->getType(
        structWalker.getStructType().getDescriptorOffset());

    if (structWalker.hasSuperStruct()) {
        switch (structWalker.superStructAddressType()) {
            case lyric_object::AddressType::Near:
                priv->superStruct = lyric_common::SymbolUrl(
                    objectLocation, structWalker.getNearSuperStruct().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                priv->superStruct = structWalker.getFarSuperStruct().getLinkUrl(objectLocation);
                break;
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import struct at index {} in module {}; invalid super struct",
                        m_structOffset, objectLocation.toString()));
        }
    }

    for (tu_uint8 i = 0; i < structWalker.numMembers(); i++) {
        auto member = structWalker.getMember(i);
        if (!member.isValid()) {
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import struct at index {} in module {}; invalid member at index {}",
                    m_structOffset, objectLocation.toString(), i));
        }

        lyric_common::SymbolUrl fieldUrl(objectLocation, member.getSymbolPath());

        auto name = fieldUrl.getSymbolName();
        priv->members[name] = fieldUrl;
    }

    for (tu_uint8 i = 0; i < structWalker.numMethods(); i++) {
        auto method = structWalker.getMethod(i);
        if (!method.isValid()) {
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import struct at index {} in module {}; invalid method at index {}",
                    m_structOffset, objectLocation.toString(), i));
        }

        lyric_common::SymbolUrl callUrl(objectLocation, method.getSymbolPath());

        auto name = callUrl.getSymbolName();
        priv->methods[name] = callUrl;
    }

    for (tu_uint8 i = 0; i < structWalker.numImpls(); i++) {
        auto impl = structWalker.getImpl(i);
        if (!impl.isValid()) {
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import struct at index {} in module {}; invalid impl at index {}",
                    m_structOffset, objectLocation.toString(), i));
        }

        auto *implType = moduleImport->getType(impl.getImplType().getDescriptorOffset());
        auto *implImport = moduleImport->getImpl(impl.getDescriptorOffset());
        priv->impls[implType->getTypeDef()] = implImport;
    }

    for (tu_uint8 i = 0; i < structWalker.numSealedSubStructs(); i++) {
        auto subStructType = structWalker.getSealedSubStruct(i);
        auto *sealedType = moduleImport->getType(subStructType.getDescriptorOffset());
        priv->sealedTypes.insert(sealedType->getTypeDef());
    }

    auto trapNumber = structWalker.getAllocator();
    if (trapNumber != lyric_object::INVALID_ADDRESS_U32) {
        auto plugin = moduleImport->getPlugin();
        if (plugin == nullptr)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import struct at index {} in module {}; invalid allocator trap",
                    m_structOffset, objectLocation.toString()));
        auto *trap = plugin->getTrap(trapNumber);
        if (trap == nullptr)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(
                    ImporterCondition::kImportError,
                    "cannot import struct at index {} in module {}; invalid allocator trap",
                    m_structOffset, objectLocation.toString()));
        priv->allocator = trap->name;
    }

    m_priv = std::move(priv);
}
