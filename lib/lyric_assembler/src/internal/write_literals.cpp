
#include <lyric_assembler/internal/write_literals.h>

tempo_utils::Status
lyric_assembler::internal::write_literals(
    const std::vector<const LiteralHandle *> &literals,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    LiteralsOffset &literalsOffset)
{
    std::vector<std::string> literals_vector;

    for (const auto *literalHandle : literals) {
        TU_NOTNULL (literalHandle);
        auto literalValue = literalHandle->getLiteral();
        literals_vector.push_back(std::move(literalValue));
    }

    // create the literals vector
    literalsOffset = buffer.CreateVectorOfStrings(literals_vector);

    return {};
}
