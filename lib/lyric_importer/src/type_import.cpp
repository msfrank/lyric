
#include <lyric_importer/importer_result.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/type_walker.h>

namespace lyric_importer {
    struct TypeImport::Priv {
        lyric_common::TypeDef typeDef;
        TypeImport *superType;
        std::vector<TypeImport *> typeArguments;
    };
}

lyric_importer::TypeImport::TypeImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 typeOffset)
    : BaseImport(moduleImport),
      m_typeOffset(typeOffset)
{
    TU_ASSERT (m_typeOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::TypeDef
lyric_importer::TypeImport::getTypeDef()
{
    load();
    return m_priv->typeDef;
}

lyric_importer::TypeImport *
lyric_importer::TypeImport::getSuperType()
{
    load();
    return m_priv->superType;
}

std::vector<lyric_importer::TypeImport *>::const_iterator
lyric_importer::TypeImport::argumentsBegin()
{
    load();
    return m_priv->typeArguments.cbegin();
}

std::vector<lyric_importer::TypeImport *>::const_iterator
lyric_importer::TypeImport::argumentsEnd()
{
    load();
    return m_priv->typeArguments.cend();
}

int
lyric_importer::TypeImport::numArguments()
{
    load();
    return m_priv->typeArguments.size();
}

tu_uint32
lyric_importer::TypeImport::getTypeOffset() const
{
    return m_typeOffset;
}

static lyric_common::SymbolUrl
import_type_symbol(
    const lyric_object::ObjectWalker &object,
    const lyric_object::ConcreteTypeWalker &concreteTypeWalker,
    const lyric_common::ModuleLocation &objectLocation)
{
    switch (concreteTypeWalker.getLinkageSection()) {
        case lyric_object::LinkageSection::Binding:
        case lyric_object::LinkageSection::Call:
        case lyric_object::LinkageSection::Class:
        case lyric_object::LinkageSection::Concept:
        case lyric_object::LinkageSection::Enum:
        case lyric_object::LinkageSection::Existential:
        case lyric_object::LinkageSection::Instance:
        case lyric_object::LinkageSection::Static:
        case lyric_object::LinkageSection::Struct:
            break;
        default:
            throw tempo_utils::StatusException(
                lyric_importer::ImporterStatus::forCondition(
                lyric_importer::ImporterCondition::kImportError,
                "cannot import type in module {}; invalid descriptor section", objectLocation.toString()));
    }

    switch (lyric_object::GET_ADDRESS_TYPE(concreteTypeWalker.getLinkageIndex())) {
        case lyric_object::AddressType::Near: {
            auto symbolPath = object.getSymbolPath(
                concreteTypeWalker.getLinkageSection(),
                lyric_object::GET_DESCRIPTOR_OFFSET(concreteTypeWalker.getLinkageIndex()));
            if (!symbolPath.isValid())
                throw tempo_utils::StatusException(
                    lyric_importer::ImporterStatus::forCondition(
                        lyric_importer::ImporterCondition::kImportError,
                        "cannot import type in module {}; symbol not found", objectLocation.toString()));
            return lyric_common::SymbolUrl(objectLocation, symbolPath);
        }
        case lyric_object::AddressType::Far: {
            auto linkUrl = object.getLink(
                lyric_object::GET_LINK_OFFSET(concreteTypeWalker.getLinkageIndex())).getLinkUrl();
            if (!linkUrl.isValid())
                throw tempo_utils::StatusException(
                    lyric_importer::ImporterStatus::forCondition(
                        lyric_importer::ImporterCondition::kImportError,
                        "cannot import type in module {}; link not found", objectLocation.toString()));
            return linkUrl;
        }
        default:
            throw tempo_utils::StatusException(
                lyric_importer::ImporterStatus::forCondition(
                    lyric_importer::ImporterCondition::kImportError,
                    "cannot import type in module {}; invalid descriptor", objectLocation.toString()));
    }
}

static lyric_common::TypeDef
import_assignable_type(const lyric_object::TypeWalker &typeWalker, lyric_importer::ModuleImport *moduleImport)
{
    TU_ASSERT (typeWalker.isValid());

    auto typeOffset = typeWalker.getDescriptorOffset();
    auto objectLocation = moduleImport->getObjectLocation();

    switch (typeWalker.getTypeDefType()) {

        case lyric_common::TypeDefType::Concrete: {
            auto concreteTypeWalker = typeWalker.concreteType();

            auto object = moduleImport->getObject().getObject();
            auto concreteUrl = import_type_symbol(object, concreteTypeWalker, objectLocation);

            std::vector<lyric_common::TypeDef> concreteParameters;
            for (const auto &p : concreteTypeWalker.getParameters()) {
                auto parameterOffset = p.getDescriptorOffset();
                if (typeOffset <= parameterOffset)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "cannot import type in module {}; invalid descriptor", objectLocation.toString()));
                auto *parameterType = moduleImport->getType(p.getDescriptorOffset());
                if (parameterType == nullptr)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "type at index {} not found in module {}", p.getDescriptorOffset(), objectLocation.toString()));
                concreteParameters.push_back(parameterType->getTypeDef());
            }

            return lyric_common::TypeDef::forConcrete(concreteUrl, concreteParameters);
        }

        case lyric_common::TypeDefType::Placeholder: {
            auto placeholderTypeWalker = typeWalker.placeholderType();

            lyric_common::SymbolUrl templateUrl;
            switch (placeholderTypeWalker.placeholderTemplateAddressType()) {
                case lyric_object::AddressType::Near: {
                    auto templatePath = placeholderTypeWalker.getNearPlaceholderTemplate().getSymbolPath();
                    templateUrl = lyric_common::SymbolUrl(objectLocation, templatePath);
                    break;
                }
                case lyric_object::AddressType::Far: {
                    auto templateLink = placeholderTypeWalker.getFarPlaceholderTemplate();
                    templateUrl = templateLink.getLinkUrl();
                    break;
                }
                default:
                    break;
            }

            std::vector<lyric_common::TypeDef> placeholderParameters;
            for (const auto &p : placeholderTypeWalker.getParameters()) {
                auto parameterOffset = p.getDescriptorOffset();
                if (typeOffset <= parameterOffset)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "cannot import type in module {}; invalid descriptor", objectLocation.toString()));
                auto *parameterType = moduleImport->getType(parameterOffset);
                if (parameterType == nullptr)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "type at index {} not found in module {}", p.getDescriptorOffset(), objectLocation.toString()));
                placeholderParameters.push_back(parameterType->getTypeDef());
            }

            return lyric_common::TypeDef::forPlaceholder(
                placeholderTypeWalker.getPlaceholderIndex(), templateUrl, placeholderParameters);
        }

        case lyric_common::TypeDefType::Union: {
            auto disjunction = typeWalker.unionType();

            std::vector<lyric_common::TypeDef> unionMembers;
            for (const auto &m : disjunction.getMembers()) {
                auto memberOffset = m.getDescriptorOffset();
                if (typeOffset <= memberOffset)
                    throw tempo_utils::StatusException(lyric_importer::ImporterStatus::forCondition(
                        lyric_importer::ImporterCondition::kImportError,
                        "cannot import type in module {}; invalid descriptor", objectLocation.toString()));
                auto memberType = moduleImport->getType(memberOffset);
                if (memberType == nullptr)
                    throw tempo_utils::StatusException(lyric_importer::ImporterStatus::forCondition(
                        lyric_importer::ImporterCondition::kImportError,
                        "type at index {} not found in module {}", m.getDescriptorOffset(), objectLocation.toString()));
                unionMembers.push_back(memberType->getTypeDef());
            }
            if (unionMembers.empty())
                throw tempo_utils::StatusException(
                    lyric_importer::ImporterStatus::forCondition(
                        lyric_importer::ImporterCondition::kImportError,
                        "type at index {} in module {} has no union members", typeOffset, objectLocation.toString()));

            return lyric_common::TypeDef::forUnion(unionMembers);
        }

        case lyric_common::TypeDefType::Intersection: {
            auto conjunction = typeWalker.intersectionType();

            std::vector<lyric_common::TypeDef> intersectionMembers;
            for (const auto &m : conjunction.getMembers()) {
                auto memberOffset = m.getDescriptorOffset();
                if (typeOffset <= memberOffset)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "cannot import type in module {}; invalid descriptor", objectLocation.toString()));
                auto *memberType = moduleImport->getType(memberOffset);
                if (memberType == nullptr)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "type at index {} not found in module {}", m.getDescriptorOffset(), objectLocation.toString()));
                intersectionMembers.push_back(memberType->getTypeDef());
            }
            if (intersectionMembers.empty())
                throw tempo_utils::StatusException(
                    lyric_importer::ImporterStatus::forCondition(
                        lyric_importer::ImporterCondition::kImportError,
                        "type at index {} in module {} has no intersection members",
                        typeOffset, objectLocation.toString()));

            return lyric_common::TypeDef::forIntersection(intersectionMembers);
        }

        // special type: NoReturn
        case lyric_common::TypeDefType::NoReturn: {
            return lyric_common::TypeDef::noReturn();
        }

        default:
            throw tempo_utils::StatusException(
                lyric_importer::ImporterStatus::forCondition(
                    lyric_importer::ImporterCondition::kImportError,
                    "cannot import type at index {} in module {}; invalid assignable type",
                    typeOffset, objectLocation.toString()));
    }
}

static std::vector<lyric_importer::TypeImport *>
import_type_arguments(const lyric_object::TypeWalker &typeWalker, lyric_importer::ModuleImport *moduleImport)
{
    TU_ASSERT (typeWalker.isValid());

    auto typeOffset = typeWalker.getDescriptorOffset();
    auto objectLocation = moduleImport->getObjectLocation();
    std::vector<lyric_importer::TypeImport *> typeArguments;

    switch (typeWalker.getTypeDefType()) {

        case lyric_common::TypeDefType::Concrete: {
            auto concreteTypeWalker = typeWalker.concreteType();

            for (const auto &p : concreteTypeWalker.getParameters()) {
                auto parameterOffset = p.getDescriptorOffset();
                if (typeOffset <= parameterOffset)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "cannot import type in module {}; invalid descriptor", objectLocation.toString()));
                auto *parameterType = moduleImport->getType(p.getDescriptorOffset());
                if (parameterType == nullptr)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "type at index {} not found in module {}", p.getDescriptorOffset(), objectLocation.toString()));
                typeArguments.push_back(parameterType);
            }

            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            auto placeholderTypeWalker = typeWalker.placeholderType();

            for (const auto &p : placeholderTypeWalker.getParameters()) {
                auto parameterOffset = p.getDescriptorOffset();
                if (typeOffset <= parameterOffset)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "cannot import type in module {}; invalid descriptor", objectLocation.toString()));
                auto *parameterType = moduleImport->getType(parameterOffset);
                if (parameterType == nullptr)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "type at index {} not found in module {}", p.getDescriptorOffset(), objectLocation.toString()));
                typeArguments.push_back(parameterType);
            }

            break;
        }

        case lyric_common::TypeDefType::Union: {
            auto disjunction = typeWalker.unionType();

            for (const auto &m : disjunction.getMembers()) {
                auto memberOffset = m.getDescriptorOffset();
                if (typeOffset <= memberOffset)
                    throw tempo_utils::StatusException(lyric_importer::ImporterStatus::forCondition(
                        lyric_importer::ImporterCondition::kImportError,
                        "cannot import type in module {}; invalid descriptor", objectLocation.toString()));
                auto memberType = moduleImport->getType(memberOffset);
                if (memberType == nullptr)
                    throw tempo_utils::StatusException(lyric_importer::ImporterStatus::forCondition(
                        lyric_importer::ImporterCondition::kImportError,
                        "type at index {} not found in module {}", m.getDescriptorOffset(), objectLocation.toString()));
                typeArguments.push_back(memberType);
            }

            break;
        }

        case lyric_common::TypeDefType::Intersection: {
            auto conjunction = typeWalker.intersectionType();

            for (const auto &m : conjunction.getMembers()) {
                auto memberOffset = m.getDescriptorOffset();
                if (typeOffset <= memberOffset)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "cannot import type in module {}; invalid descriptor", objectLocation.toString()));
                auto *memberType = moduleImport->getType(memberOffset);
                if (memberType == nullptr)
                    throw tempo_utils::StatusException(
                        lyric_importer::ImporterStatus::forCondition(
                            lyric_importer::ImporterCondition::kImportError,
                            "type at index {} not found in module {}", m.getDescriptorOffset(), objectLocation.toString()));
                typeArguments.push_back(memberType);
            }

            break;
        }

        // special type NoReturn never has type arguments
        case lyric_common::TypeDefType::NoReturn: {
            break;
        }

        default:
            throw tempo_utils::StatusException(
                lyric_importer::ImporterStatus::forCondition(
                    lyric_importer::ImporterCondition::kImportError,
                    "cannot import type at index {} in module {}; invalid assignable type",
                    typeOffset, objectLocation.toString()));
    }

    return typeArguments;
}

void
lyric_importer::TypeImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = getModuleImport();
    auto typeWalker = moduleImport->getObject().getObject().getType(m_typeOffset);
    priv->typeDef = import_assignable_type(typeWalker, moduleImport.get());
    priv->typeArguments = import_type_arguments(typeWalker, moduleImport.get());

    if (typeWalker.hasSuperType()) {
        priv->superType = moduleImport->getType(typeWalker.getSuperType().getDescriptorOffset());
    } else {
        priv->superType = nullptr;
    }

    m_priv = std::move(priv);
}