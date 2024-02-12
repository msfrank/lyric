
#include <lyric_importer/class_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/class_walker.h>

namespace lyric_importer {
    struct ClassImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isAbstract;
        lyric_object::DeriveType derive;
        TypeImport *classType;
        TemplateImport *classTemplate;
        lyric_common::SymbolUrl superClass;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> members;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> methods;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        tu_uint32 allocator;
    };
}

lyric_importer::ClassImport::ClassImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 classOffset)
    : m_moduleImport(moduleImport),
      m_classOffset(classOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_classOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::ClassImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::ClassImport::isAbstract()
{
    load();
    return m_priv->isAbstract;
}

lyric_object::DeriveType
lyric_importer::ClassImport::getDerive()
{
    load();
    return m_priv->derive;
}

lyric_importer::TypeImport *
lyric_importer::ClassImport::getClassType()
{
    load();
    return m_priv->classType;
}

lyric_importer::TemplateImport *
lyric_importer::ClassImport::getClassTemplate()
{
    load();
    return m_priv->classTemplate;
}


lyric_common::SymbolUrl
lyric_importer::ClassImport::getSuperClass()
{
    load();
    return m_priv->superClass;
}

lyric_common::SymbolUrl
lyric_importer::ClassImport::getMember(std::string_view name)
{
    load();
    if (m_priv->members.contains(name))
        return m_priv->members.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::ClassImport::membersBegin()
{
    load();
    return m_priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::ClassImport::membersEnd()
{
    load();
    return m_priv->members.cend();
}

tu_uint8
lyric_importer::ClassImport::numMembers()
{
    load();
    return m_priv->members.size();
}

lyric_common::SymbolUrl
lyric_importer::ClassImport::getMethod(std::string_view name)
{
    load();
    if (m_priv->methods.contains(name))
        return m_priv->methods.at(name);
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::ClassImport::methodsBegin()
{
    load();
    return m_priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_importer::ClassImport::methodsEnd()
{
    load();
    return m_priv->methods.cend();
}

tu_uint8
lyric_importer::ClassImport::numMethods()
{
    load();
    return m_priv->methods.size();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::ClassImport::sealedTypesBegin()
{
    load();
    return m_priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_importer::ClassImport::sealedTypesEnd()
{
    load();
    return m_priv->sealedTypes.cend();
}

int
lyric_importer::ClassImport::numSealedTypes()
{
    load();
    return m_priv->sealedTypes.size();
}

bool
lyric_importer::ClassImport::hasAllocator()
{
    return m_priv->allocator != lyric_object::INVALID_ADDRESS_U32;
}

tu_uint32
lyric_importer::ClassImport::getAllocator()
{
    return m_priv->allocator;
}

void
lyric_importer::ClassImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto location = m_moduleImport->getLocation();
    auto classWalker = m_moduleImport->getObject().getObject().getClass(m_classOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, classWalker.getSymbolPath());

    priv->isAbstract = classWalker.isAbstract();

    if (classWalker.getDeriveType() == lyric_object::DeriveType::Invalid)
        throw ImporterException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import class at index {} in assembly {}; invalid derive type",
                m_classOffset, location.toString()));
    priv->derive = classWalker.getDeriveType();

    priv->classType = m_moduleImport->getType(
        classWalker.getClassType().getDescriptorOffset());

    if (classWalker.hasTemplate()) {
        priv->classTemplate = m_moduleImport->getTemplate(
            classWalker.getTemplate().getDescriptorOffset());
    } else {
        priv->classTemplate = nullptr;
    }

    if (classWalker.hasSuperClass()) {
        switch (classWalker.superClassAddressType()) {
            case lyric_object::AddressType::Near:
                priv->superClass = lyric_common::SymbolUrl(
                    location, classWalker.getNearSuperClass().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                priv->superClass = classWalker.getFarSuperClass().getLinkUrl();
                break;
            default:
                throw ImporterException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import class at index {} in assembly {}; invalid super class",
                        m_classOffset, location.toString()));
        }
    }

    for (tu_uint8 i = 0; i < classWalker.numMembers(); i++) {
        auto member = classWalker.getMember(i);
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
                        "cannot import class at index {} in assembly {}; invalid member at index {}",
                        m_classOffset, location.toString(), i));
        }
        auto name = fieldUrl.getSymbolName();
        priv->members[name] = fieldUrl;
    }

    for (tu_uint8 i = 0; i < classWalker.numMethods(); i++) {
        auto method = classWalker.getMethod(i);
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
                        "cannot import class at index {} in assembly {}; invalid method at index {}",
                        m_classOffset, location.toString(), i));
        }
        auto name = callUrl.getSymbolName();
        priv->methods[name] = callUrl;
    }

    for (tu_uint8 i = 0; i < classWalker.numSealedSubClasses(); i++) {
        auto subClassType = classWalker.getSealedSubClass(i);
        auto *sealedType = m_moduleImport->getType(subClassType.getDescriptorOffset());
        priv->sealedTypes.insert(sealedType->getTypeDef());
    }

    priv->allocator = classWalker.getAllocator();

    m_priv = std::move(priv);
}