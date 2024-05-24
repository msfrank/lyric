
#include <lyric_assembler/internal/write_templates.h>

static tempo_utils::Status
write_template(
    lyric_assembler::TypeCache *typeCache,
    const lyric_assembler::TemplateHandle *templateHandle,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::TemplateDescriptor>> &templates_vector)
{
    auto templateUrlPathString = templateHandle->getTemplateUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(templateUrlPathString);

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
            if (!typeCache->hasType(tp.typeDef))
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "missing constraint type");

            lyric_assembler::TypeHandle *typeHandle;
            TU_ASSIGN_OR_RETURN (typeHandle, typeCache->getOrMakeType(tp.typeDef));
            constraints.emplace_back(placeholder_offset, bound, typeHandle->getAddress().getAddress());
        }
    }

    // add template descriptor
    templates_vector.push_back(lyo1::CreateTemplateDescriptor(buffer, fullyQualifiedName,
        buffer.CreateVectorOfStructs(placeholders), buffer.CreateVectorOfStructs(constraints),
        buffer.CreateVectorOfStrings(names)));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_templates(
    TypeCache *typeCache,
    flatbuffers::FlatBufferBuilder &buffer,
    TemplatesOffset &templatesOffset)
{
    TU_ASSERT (typeCache != nullptr);

    std::vector<flatbuffers::Offset<lyo1::TemplateDescriptor>> templates_vector;

    for (auto iterator = typeCache->templatesBegin(); iterator != typeCache->templatesEnd(); iterator++) {
        auto &templateHandle = *iterator;
        TU_RETURN_IF_NOT_OK (write_template(typeCache, templateHandle, buffer, templates_vector));
    }

    // create the templates vector
    templatesOffset = buffer.CreateVector(templates_vector);

    return {};
}
