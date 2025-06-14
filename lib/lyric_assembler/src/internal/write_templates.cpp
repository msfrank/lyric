
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/internal/write_templates.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_template(
    const TemplateHandle *templateHandle,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (templateHandle != nullptr);

    auto templateUrl = templateHandle->getTemplateUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertTemplate(templateUrl, templateHandle, alreadyInserted));
    if (alreadyInserted)
        return {};

    auto *symbolCache = objectState->symbolCache();
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(templateUrl));
    switch (symbol->getSymbolType()) {
        case SymbolType::BINDING:
            TU_RETURN_IF_NOT_OK (writer.touchBinding(cast_symbol_to_binding(symbol)));
            break;
        case SymbolType::CALL:
            TU_RETURN_IF_NOT_OK (writer.touchCall(cast_symbol_to_call(symbol)));
            break;
        case SymbolType::CLASS:
            TU_RETURN_IF_NOT_OK (writer.touchClass(cast_symbol_to_class(symbol)));
            break;
        case SymbolType::CONCEPT:
            TU_RETURN_IF_NOT_OK (writer.touchConcept(cast_symbol_to_concept(symbol)));
            break;
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid symbol for template");
    }

    auto *typeCache = objectState->typeCache();
    for (auto it = templateHandle->templateParametersBegin(); it != templateHandle->templateParametersEnd(); it++) {
        const auto &tp = *it;
        switch (tp.bound) {
            case lyric_object::BoundType::None:
                break;
            case lyric_object::BoundType::Extends:
            case lyric_object::BoundType::Super:
                if (tp.typeDef.isValid()) {
                    TypeHandle *typeHandle;
                    TU_ASSIGN_OR_RETURN (typeHandle, typeCache->getOrMakeType(tp.typeDef));
                    TU_RETURN_IF_NOT_OK (writer.touchType(typeHandle));
                }
                break;
            default:
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid bound for type");
        }
    }

    return {};
}

static tempo_utils::Status
write_template(
    const lyric_assembler::TemplateHandle *templateHandle,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::TemplateDescriptor>> &templates_vector)
{
    auto templateUrlPathString = templateHandle->getTemplateUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(templateUrlPathString);

    auto supertemplateIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *supertemplateHandle = templateHandle->superTemplate();
    if (supertemplateHandle != nullptr) {
        TU_ASSIGN_OR_RETURN (supertemplateIndex, writer.getTemplateOffset(supertemplateHandle->getTemplateUrl()));
    }

    std::vector<lyo1::Placeholder> placeholders;
    std::vector<lyo1::Constraint> constraints;
    std::vector<std::string> names;

    for (int i = 0; i < templateHandle->numTemplateParameters(); i++) {
        const auto &tp = templateHandle->getTemplateParameter(i);

        tu_uint32 name_offset = names.size();
        names.push_back(tp.name);

        tu_uint32 placeholder_offset = placeholders.size();
        lyo1::PlaceholderVariance variance;
        switch (tp.variance) {
            case lyric_object::VarianceType::Invariant:
                variance = lyo1::PlaceholderVariance::Invariant;
                break;
            case lyric_object::VarianceType::Covariant:
                variance = lyo1::PlaceholderVariance::Covariant;
                break;
            case lyric_object::VarianceType::Contravariant:
                variance = lyo1::PlaceholderVariance::Contravariant;
                break;
            default:
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid placeholder variance");
        }
        placeholders.emplace_back(variance, name_offset);

        if (tp.bound != lyric_object::BoundType::None) {
            lyo1::ConstraintBound bound;
            switch (tp.bound) {
                case lyric_object::BoundType::Extends:
                    bound = lyo1::ConstraintBound::Extends;
                    break;
                case lyric_object::BoundType::Super:
                    bound = lyo1::ConstraintBound::Super;
                    break;
                default:
                    return lyric_assembler::AssemblerStatus::forCondition(
                        lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                        "invalid type bound");
            }
            if (!tp.typeDef.isValid())
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid constraint type");

            tu_uint32 typeOffset;
            TU_ASSIGN_OR_RETURN (typeOffset, writer.getTypeOffset(tp.typeDef));
            constraints.emplace_back(placeholder_offset, bound, typeOffset);
        }
    }

    // add template descriptor
    templates_vector.push_back(lyo1::CreateTemplateDescriptor(buffer, fullyQualifiedName,
        supertemplateIndex, buffer.CreateVectorOfStructs(placeholders),
        buffer.CreateVectorOfStructs(constraints), buffer.CreateVectorOfStrings(names)));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_templates(
    const std::vector<const TemplateHandle *> &templates,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    TemplatesOffset &templatesOffset)
{
    std::vector<flatbuffers::Offset<lyo1::TemplateDescriptor>> templates_vector;

    for (const auto *templateHandle : templates) {
        TU_RETURN_IF_NOT_OK (write_template(templateHandle, writer, buffer, templates_vector));
    }

    // create the templates vector
    templatesOffset = buffer.CreateVector(templates_vector);

    return {};
}
