
#include <lyric_assembler/assembler_types.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/write_types.h>
#include <lyric_assembler/struct_symbol.h>

static lyric_assembler::AssemblerStatus
write_type(
    lyric_assembler::TypeCache *typeCache,
    const lyric_assembler::TypeHandle *typeHandle,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::TypeDescriptor>> &types_vector)
{
    flatbuffers::Offset<void> assignableUnion;
    lyo1::Assignable unionType;

    auto assignableType = typeHandle->getTypeDef();
    switch (assignableType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            auto *sym = symbolCache->getSymbol(assignableType.getConcreteUrl());
            TU_ASSERT (sym != nullptr);
            lyo1::TypeSection concreteSection;
            uint32_t concreteDescriptor;
            std::vector<uint32_t> parameters;
            switch (sym->getSymbolType()) {
                case lyric_assembler::SymbolType::EXISTENTIAL:
                    concreteSection = lyo1::TypeSection::Existential;
                    concreteDescriptor = cast_symbol_to_existential(sym)->getAddress().getAddress();
                    break;
                case lyric_assembler::SymbolType::CLASS:
                    concreteSection = lyo1::TypeSection::Class;
                    concreteDescriptor = cast_symbol_to_class(sym)->getAddress().getAddress();
                    break;
                case lyric_assembler::SymbolType::CONCEPT:
                    concreteSection = lyo1::TypeSection::Concept;
                    concreteDescriptor = cast_symbol_to_concept(sym)->getAddress().getAddress();
                    break;
                case lyric_assembler::SymbolType::INSTANCE:
                    concreteSection = lyo1::TypeSection::Instance;
                    concreteDescriptor = cast_symbol_to_instance(sym)->getAddress().getAddress();
                    break;
                case lyric_assembler::SymbolType::ENUM:
                    concreteSection = lyo1::TypeSection::Enum;
                    concreteDescriptor = cast_symbol_to_enum(sym)->getAddress().getAddress();
                    break;
                case lyric_assembler::SymbolType::STRUCT:
                    concreteSection = lyo1::TypeSection::Struct;
                    concreteDescriptor = cast_symbol_to_struct(sym)->getAddress().getAddress();
                    break;
                default:
                    return lyric_assembler::AssemblerStatus::forCondition(
                        lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid type");
            }
            for (auto it = typeHandle->typeArgumentsBegin(); it != typeHandle->typeArgumentsEnd(); it++) {
                const auto *t = typeCache->getType(*it);
                TU_ASSERT (t != nullptr);
                auto address = t->getAddress();
                TU_ASSERT (address.isValid());
                parameters.emplace_back(address.getAddress());
            }
            auto assignable = lyo1::CreateConcreteAssignable(buffer, concreteSection,
                concreteDescriptor, buffer.CreateVector(parameters));
            assignableUnion = assignable.Union();
            unionType = lyo1::Assignable::ConcreteAssignable;
            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            auto templateUrl = assignableType.getPlaceholderTemplateUrl();
            auto *templateHandle = typeCache->getTemplate(templateUrl);
            TU_ASSERT (templateHandle != nullptr);
            uint32_t placeholderTemplate = templateHandle->getAddress().getAddress();
            TU_ASSERT (placeholderTemplate != lyric_runtime::INVALID_ADDRESS_U32);
            std::vector<uint32_t> parameters;
            for (auto it = typeHandle->typeArgumentsBegin(); it != typeHandle->typeArgumentsEnd(); it++) {
                const auto *t = typeCache->getType(*it);
                TU_ASSERT (t != nullptr);
                auto address = t->getAddress();
                TU_ASSERT (address.isValid());
                parameters.emplace_back(address.getAddress());
            }
            auto assignable = lyo1::CreatePlaceholderAssignable(buffer, placeholderTemplate,
                assignableType.getPlaceholderIndex(), buffer.CreateVector(parameters));
            assignableUnion = assignable.Union();
            unionType = lyo1::Assignable::PlaceholderAssignable;
            break;
        }

        case lyric_common::TypeDefType::Union: {
            std::vector<uint32_t> members;
            for (const auto &member : typeHandle->getTypeDef().getUnionMembers()) {
                const auto *t = typeCache->getType(member);
                TU_ASSERT (t != nullptr);
                auto address = t->getAddress();
                TU_ASSERT (address.isValid());
                members.emplace_back(address.getAddress());
            }
            auto assignable = lyo1::CreateUnionAssignable(buffer, buffer.CreateVector(members));
            assignableUnion = assignable.Union();
            unionType = lyo1::Assignable::UnionAssignable;
            break;
        }

        case lyric_common::TypeDefType::Intersection: {
            std::vector<uint32_t> members;
            for (const auto &member : typeHandle->getTypeDef().getIntersectionMembers()) {
                const auto *t = typeCache->getType(member);
                TU_ASSERT (t != nullptr);
                auto address = t->getAddress();
                TU_ASSERT (address.isValid());
                members.emplace_back(address.getAddress());
            }
            auto assignable = lyo1::CreateIntersectionAssignable(buffer, buffer.CreateVector(members));
            assignableUnion = assignable.Union();
            unionType = lyo1::Assignable::IntersectionAssignable;
            break;
        }

        case lyric_common::TypeDefType::NoReturn: {
            auto assignable = lyo1::CreateSpecialAssignable(buffer, lyo1::SpecialType::NoReturn);
            assignableUnion = assignable.Union();
            unionType = lyo1::Assignable::SpecialAssignable;
            break;
        }

        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid type");
    }

    uint32_t superType = lyric_runtime::INVALID_ADDRESS_U32;
    if (typeHandle->getSuperType() != nullptr) {
        auto address = typeHandle->getSuperType()->getAddress();
        TU_ASSERT(address.isValid());
        superType = address.getAddress();
    }

    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer, unionType, assignableUnion, superType));
    return lyric_assembler::AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::internal::write_types(
    TypeCache *typeCache,
    SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    TypesOffset &typesOffset)
{
    TU_ASSERT (typeCache != nullptr);

    std::vector<flatbuffers::Offset<lyo1::TypeDescriptor>> types_vector;

    for (auto iterator = typeCache->typesBegin(); iterator != typeCache->typesEnd(); iterator++) {
        auto &typeHandle = *iterator;
        TU_RETURN_IF_NOT_OK (write_type(typeCache, typeHandle, symbolCache, buffer, types_vector));
    }

    // create the types vector
    typesOffset = buffer.CreateVector(types_vector);

    return AssemblerStatus::ok();
}
