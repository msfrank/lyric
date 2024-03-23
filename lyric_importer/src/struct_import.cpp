
#include <lyric_importer/importer_result.h>
#include <lyric_importer/struct_import.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/struct_walker.h>

namespace lyric_importer {
    struct StructImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isAbstract;
        lyric_object::DeriveType derive;
        TypeImport *structType;
        lyric_common::SymbolUrl superStruct;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> members;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> methods;
        absl::flat_hash_map<lyric_common::TypeDef,ImplImport *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        tu_uint32 allocator;
    };
}

lyric_importer::StructImport::StructImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 structOffset)
    : m_moduleImport(moduleImport),
      m_structOffset(structOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_structOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::StructImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::StructImport::isAbstract()
{
    load();
    return m_priv->isAbstract;
}

lyric_object::DeriveType
lyric_importer::StructImport::getDerive()
{
    load();
    return m_priv->derive;
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
    return m_priv->allocator != lyric_object::INVALID_ADDRESS_U32;
}

tu_uint32
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

    auto location = m_moduleImport->getLocation();
    auto structWalker = m_moduleImport->getObject().getObject().getStruct(m_structOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, structWalker.getSymbolPath());

    priv->isAbstract = structWalker.isAbstract();

    if (structWalker.getDeriveType() == lyric_object::DeriveType::Invalid)
        throw ImporterException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import struct at index {} in assembly {}; invalid derive type",
                m_structOffset, location.toString()));
    priv->derive = structWalker.getDeriveType();

    priv->structType = m_moduleImport->getType(
        structWalker.getStructType().getDescriptorOffset());

    if (structWalker.hasSuperStruct()) {
        switch (structWalker.superStructAddressType()) {
            case lyric_object::AddressType::Near:
                priv->superStruct = lyric_common::SymbolUrl(
                    location, structWalker.getNearSuperStruct().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                priv->superStruct = structWalker.getFarSuperStruct().getLinkUrl();
                break;
            default:
                throw ImporterException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import struct at index {} in assembly {}; invalid super struct",
                        m_structOffset, location.toString()));
        }
    }

    for (tu_uint8 i = 0; i < structWalker.numMembers(); i++) {
        auto member = structWalker.getMember(i);
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
                        "cannot import struct at index {} in assembly {}; invalid member at index {}",
                        m_structOffset, location.toString(), i));
        }
        auto name = fieldUrl.getSymbolName();
        priv->members[name] = fieldUrl;
    }

    for (tu_uint8 i = 0; i < structWalker.numMethods(); i++) {
        auto method = structWalker.getMethod(i);
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
                        "cannot import struct at index {} in assembly {}; invalid method at index {}",
                        m_structOffset, location.toString(), i));
        }
        auto name = callUrl.getSymbolName();
        priv->methods[name] = callUrl;
    }

    for (tu_uint8 i = 0; i < structWalker.numImpls(); i++) {
        auto implWalker = structWalker.getImpl(i);

        auto *implType = m_moduleImport->getType(implWalker.getImplType().getDescriptorOffset());
        auto *implImport = m_moduleImport->getImpl(implWalker.getDescriptorOffset());
        priv->impls[implType->getTypeDef()] = implImport;
    }

    for (tu_uint8 i = 0; i < structWalker.numSealedSubStructs(); i++) {
        auto subStructType = structWalker.getSealedSubStruct(i);
        auto *sealedType = m_moduleImport->getType(subStructType.getDescriptorOffset());
        priv->sealedTypes.insert(sealedType->getTypeDef());
    }

    priv->allocator = structWalker.getAllocator();

    m_priv = std::move(priv);
}
