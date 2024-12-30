
#include <lyric_assembler/assembler_types.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/write_types.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>

tempo_utils::Status
lyric_assembler::internal::touch_type(
    const TypeHandle *typeHandle,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (typeHandle != nullptr);

    auto typeDef = typeHandle->getTypeDef();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertType(typeDef, typeHandle, alreadyInserted));
    if (alreadyInserted)
        return {};

    auto symbolUrl = typeHandle->getTypeSymbol();
    if (symbolUrl.isValid()) {
        auto *symbolCache = objectState->symbolCache();
        AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(symbolUrl));
        switch (symbol->getSymbolType()) {
            case SymbolType::CALL:
                TU_RETURN_IF_NOT_OK (writer.touchCall(cast_symbol_to_call(symbol)));
                break;
            case SymbolType::CLASS:
                TU_RETURN_IF_NOT_OK (writer.touchClass(cast_symbol_to_class(symbol)));
                break;
            case SymbolType::CONCEPT:
                TU_RETURN_IF_NOT_OK (writer.touchConcept(cast_symbol_to_concept(symbol)));
                break;
            case SymbolType::ENUM:
                TU_RETURN_IF_NOT_OK (writer.touchEnum(cast_symbol_to_enum(symbol)));
                break;
            case SymbolType::EXISTENTIAL:
                TU_RETURN_IF_NOT_OK (writer.touchExistential(cast_symbol_to_existential(symbol)));
                break;
            case SymbolType::INSTANCE:
                TU_RETURN_IF_NOT_OK (writer.touchInstance(cast_symbol_to_instance(symbol)));
                break;
            case SymbolType::STATIC:
                TU_RETURN_IF_NOT_OK (writer.touchStatic(cast_symbol_to_static(symbol)));
                break;
            case SymbolType::STRUCT:
                TU_RETURN_IF_NOT_OK (writer.touchStruct(cast_symbol_to_struct(symbol)));
                break;
            default:
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid symbol for type");
        }
    }

    switch (typeDef.getType()) {
        case lyric_common::TypeDefType::Concrete:
        case lyric_common::TypeDefType::Placeholder:
        case lyric_common::TypeDefType::Intersection:
        case lyric_common::TypeDefType::Union: {
            auto *typeCache = objectState->typeCache();
            for (auto it = typeHandle->typeArgumentsBegin(); it != typeHandle->typeArgumentsEnd(); it++) {
                TypeHandle *typeArgumentHandle;
                TU_ASSIGN_OR_RETURN (typeArgumentHandle, typeCache->getOrMakeType(*it));
                TU_RETURN_IF_NOT_OK (touch_type(typeArgumentHandle, objectState, writer));
            }
            break;
        }
        default:
            break;
    }

    auto *supertypeHandle = typeHandle->getSuperType();
    if (supertypeHandle != nullptr) {
        TU_RETURN_IF_NOT_OK (touch_type(supertypeHandle, objectState, writer));
    }

    return {};
}

static tempo_utils::Status
write_type(
    const lyric_assembler::TypeHandle *typeHandle,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::TypeDescriptor>> &types_vector)
{
    flatbuffers::Offset<void> assignableUnion;
    lyo1::Assignable unionType;

    auto assignableType = typeHandle->getTypeDef();
    switch (assignableType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            lyo1::TypeSection concreteSection;
            tu_uint32 concreteDescriptor;
            std::vector<tu_uint32> parameters;
            auto concreteUrl = assignableType.getConcreteUrl();
            lyric_object::LinkageSection section;
            TU_ASSIGN_OR_RETURN (section, writer.getSymbolSection(concreteUrl));
            TU_ASSIGN_OR_RETURN (concreteDescriptor, writer.getSymbolAddress(concreteUrl));
            switch (section) {
                case lyric_object::LinkageSection::Call:
                    concreteSection = lyo1::TypeSection::Call;
                    break;
                case lyric_object::LinkageSection::Class:
                    concreteSection = lyo1::TypeSection::Class;
                    break;
                case lyric_object::LinkageSection::Concept:
                    concreteSection = lyo1::TypeSection::Concept;
                    break;
                case lyric_object::LinkageSection::Enum:
                    concreteSection = lyo1::TypeSection::Enum;
                    break;
                case lyric_object::LinkageSection::Existential:
                    concreteSection = lyo1::TypeSection::Existential;
                    break;
                case lyric_object::LinkageSection::Instance:
                    concreteSection = lyo1::TypeSection::Instance;
                    break;
                case lyric_object::LinkageSection::Static:
                    concreteSection = lyo1::TypeSection::Static;
                    break;
                case lyric_object::LinkageSection::Struct:
                    concreteSection = lyo1::TypeSection::Struct;
                    break;
                default:
                    return lyric_assembler::AssemblerStatus::forCondition(
                        lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid type");
            }
            for (auto it = typeHandle->typeArgumentsBegin(); it != typeHandle->typeArgumentsEnd(); it++) {
                tu_uint32 argumentType;
                TU_ASSIGN_OR_RETURN (argumentType, writer.getTypeOffset(*it));
                parameters.emplace_back(argumentType);
            }
            auto assignable = lyo1::CreateConcreteAssignable(buffer, concreteSection,
                concreteDescriptor, buffer.CreateVector(parameters));
            assignableUnion = assignable.Union();
            unionType = lyo1::Assignable::ConcreteAssignable;
            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            auto templateUrl = assignableType.getPlaceholderTemplateUrl();
            tu_uint32 placeholderTemplate;
            TU_ASSIGN_OR_RETURN (placeholderTemplate, writer.getTemplateOffset(templateUrl));
            std::vector<tu_uint32> parameters;
            for (auto it = typeHandle->typeArgumentsBegin(); it != typeHandle->typeArgumentsEnd(); it++) {
                tu_uint32 argumentType;
                TU_ASSIGN_OR_RETURN (argumentType, writer.getTypeOffset(*it));
                parameters.emplace_back(argumentType);
            }
            auto assignable = lyo1::CreatePlaceholderAssignable(buffer, placeholderTemplate,
                assignableType.getPlaceholderIndex(), buffer.CreateVector(parameters));
            assignableUnion = assignable.Union();
            unionType = lyo1::Assignable::PlaceholderAssignable;
            break;
        }

        case lyric_common::TypeDefType::Union: {
            std::vector<tu_uint32> members;
            for (const auto &member : typeHandle->getTypeDef().getUnionMembers()) {
                tu_uint32 memberType;
                TU_ASSIGN_OR_RETURN (memberType, writer.getTypeOffset(member));
                members.emplace_back(memberType);
            }
            auto assignable = lyo1::CreateUnionAssignable(buffer, buffer.CreateVector(members));
            assignableUnion = assignable.Union();
            unionType = lyo1::Assignable::UnionAssignable;
            break;
        }

        case lyric_common::TypeDefType::Intersection: {
            std::vector<tu_uint32> members;
            for (const auto &member : typeHandle->getTypeDef().getIntersectionMembers()) {
                tu_uint32 memberType;
                TU_ASSIGN_OR_RETURN (memberType, writer.getTypeOffset(member));
                members.emplace_back(memberType);
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

    tu_uint32 superType = lyric_runtime::INVALID_ADDRESS_U32;
    if (typeHandle->getSuperType() != nullptr) {
        TU_ASSIGN_OR_RETURN (superType, writer.getTypeOffset(typeHandle->getSuperType()->getTypeDef()));
    }

    types_vector.push_back(lyo1::CreateTypeDescriptor(buffer, unionType, assignableUnion, superType));
    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_types(
    const std::vector<const TypeHandle *> &types,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    TypesOffset &typesOffset)
{
    std::vector<flatbuffers::Offset<lyo1::TypeDescriptor>> types_vector;

    for (const auto *typeHandle : types) {
        TU_RETURN_IF_NOT_OK (write_type(typeHandle, writer, buffer, types_vector));
    }

    // create the types vector
    typesOffset = buffer.CreateVector(types_vector);

    return {};
}
