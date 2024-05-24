
#include <lyric_assembler/internal/write_literals.h>

tempo_utils::Status
lyric_assembler::internal::write_literals(
    LiteralCache *literalCache,
    flatbuffers::FlatBufferBuilder &buffer,
    LiteralsOffset &literalsOffset)
{
    TU_ASSERT (literalCache != nullptr);

    std::vector<flatbuffers::Offset<lyo1::LiteralDescriptor>> literals_vector;

    for (auto iterator = literalCache->literalsBegin(); iterator != literalCache->literalsEnd(); iterator++) {
        auto &literalHandle = *iterator;

        switch (literalHandle->getType()) {
            case lyric_runtime::LiteralCellType::NIL: {
                auto value = lyo1::CreateTFNUValue(buffer, lyo1::TrueFalseNilUndef::Nil);
                literals_vector.push_back(
                    lyo1::CreateLiteralDescriptor(
                        buffer, lyo1::Value::TFNUValue, value.Union()));
                break;
            }
            case lyric_runtime::LiteralCellType::UNDEF: {
                auto value = lyo1::CreateTFNUValue(buffer, lyo1::TrueFalseNilUndef::Undef);
                literals_vector.push_back(
                    lyo1::CreateLiteralDescriptor(
                        buffer, lyo1::Value::TFNUValue, value.Union()));
                break;
            }
            case lyric_runtime::LiteralCellType::BOOL: {
                auto value = lyo1::CreateTFNUValue(buffer,
                    literalHandle->getBool() ? lyo1::TrueFalseNilUndef::True : lyo1::TrueFalseNilUndef::False);
                literals_vector.push_back(
                    lyo1::CreateLiteralDescriptor(
                        buffer, lyo1::Value::TFNUValue, value.Union()));
                break;
            }
            case lyric_runtime::LiteralCellType::I64: {
                auto value = lyo1::CreateInt64Value(buffer, literalHandle->getInt64());
                literals_vector.push_back(
                    lyo1::CreateLiteralDescriptor(
                        buffer, lyo1::Value::Int64Value, value.Union()));
                break;
            }
            case lyric_runtime::LiteralCellType::DBL: {
                auto value = lyo1::CreateFloat64Value(buffer, literalHandle->getDouble());
                literals_vector.push_back(
                    lyo1::CreateLiteralDescriptor(
                        buffer, lyo1::Value::Float64Value, value.Union()));
                break;
            }
            case lyric_runtime::LiteralCellType::CHAR32: {
                auto value = lyo1::CreateCharValue(buffer, literalHandle->getUChar32());
                literals_vector.push_back(
                    lyo1::CreateLiteralDescriptor(
                        buffer, lyo1::Value::CharValue, value.Union()));
                break;
            }
            case lyric_runtime::LiteralCellType::UTF8: {
                auto utf8 = literalHandle->getString();
                auto str = buffer.CreateString(utf8->data(), utf8->size());
                auto value = lyo1::CreateStringValue(buffer, str);
                literals_vector.push_back(
                    lyo1::CreateLiteralDescriptor(
                        buffer, lyo1::Value::StringValue, value.Union()));
                break;
            }
            default:
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid literal");
        }
    }

    // create the literals vector
    literalsOffset = buffer.CreateVector(literals_vector);

    return {};
}
