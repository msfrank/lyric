
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/object_types.h>
#include <tempo_utils/big_endian.h>

lyric_object::LinkageSection
lyric_object::descriptor_to_linkage_section(tu_uint8 section)
{
    return internal::descriptor_to_linkage_section(
        static_cast<lyo1::DescriptorSection>(section));
}

tu_uint8
lyric_object::linkage_to_descriptor_section(lyric_object::LinkageSection section)
{
    return static_cast<tu_uint8>(internal::linkage_to_descriptor_section(section));
}

tempo_utils::LogMessage&& lyric_object::operator<<(tempo_utils::LogMessage&& message, Opcode opcode) {
    switch (opcode) {
        case Opcode::OP_UNKNOWN:
            std::forward<tempo_utils::LogMessage>(message) << "OP_UNKNOWN";
            break;
        case Opcode::OP_NOOP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_NOOP";
            break;
        case Opcode::OP_NIL:
            std::forward<tempo_utils::LogMessage>(message) << "OP_NIL";
            break;
        case Opcode::OP_TRUE:
            std::forward<tempo_utils::LogMessage>(message) << "OP_TRUE";
            break;
        case Opcode::OP_FALSE:
            std::forward<tempo_utils::LogMessage>(message) << "OP_FALSE";
            break;
        case Opcode::OP_I64:
            std::forward<tempo_utils::LogMessage>(message) << "OP_I64";
            break;
        case Opcode::OP_DBL:
            std::forward<tempo_utils::LogMessage>(message) << "OP_DBL";
            break;
        case Opcode::OP_CHR:
            std::forward<tempo_utils::LogMessage>(message) << "OP_CHR";
            break;
        case Opcode::OP_LITERAL:
            std::forward<tempo_utils::LogMessage>(message) << "OP_LITERAL";
            break;
        case Opcode::OP_STATIC:
            std::forward<tempo_utils::LogMessage>(message) << "OP_STATIC";
            break;
        case Opcode::OP_SYNTHETIC:
            std::forward<tempo_utils::LogMessage>(message) << "OP_SYNTHETIC";
            break;
        case Opcode::OP_DESCRIPTOR:
            std::forward<tempo_utils::LogMessage>(message) << "OP_DESCRIPTOR";
            break;
        case Opcode::OP_LOAD:
            std::forward<tempo_utils::LogMessage>(message) << "OP_LOAD";
            break;
        case Opcode::OP_STORE:
            std::forward<tempo_utils::LogMessage>(message) << "OP_STORE";
            break;
        case Opcode::OP_VA_LOAD:
            std::forward<tempo_utils::LogMessage>(message) << "OP_VA_LOAD";
            break;
        case Opcode::OP_VA_SIZE:
            std::forward<tempo_utils::LogMessage>(message) << "OP_VA_SIZE";
            break;
        case Opcode::OP_POP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_POP";
            break;
        case Opcode::OP_DUP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_DUP";
            break;
        case Opcode::OP_PICK:
            std::forward<tempo_utils::LogMessage>(message) << "OP_PICK";
            break;
        case Opcode::OP_DROP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_DROP";
            break;
        case Opcode::OP_RPICK:
            std::forward<tempo_utils::LogMessage>(message) << "OP_RPICK";
            break;
        case Opcode::OP_RDROP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_RDROP";
            break;
        case Opcode::OP_I64_ADD:
            std::forward<tempo_utils::LogMessage>(message) << "OP_I64_ADD";
            break;
        case Opcode::OP_I64_SUB:
            std::forward<tempo_utils::LogMessage>(message) << "OP_I64_SUB";
            break;
        case Opcode::OP_I64_MUL:
            std::forward<tempo_utils::LogMessage>(message) << "OP_I64_MUL";
            break;
        case Opcode::OP_I64_DIV:
            std::forward<tempo_utils::LogMessage>(message) << "OP_I64_DIV";
            break;
        case Opcode::OP_I64_NEG:
            std::forward<tempo_utils::LogMessage>(message) << "OP_I64_NEG";
            break;
        case Opcode::OP_DBL_ADD:
            std::forward<tempo_utils::LogMessage>(message) << "OP_DBL_ADD";
            break;
        case Opcode::OP_DBL_SUB:
            std::forward<tempo_utils::LogMessage>(message) << "OP_DBL_SUB";
            break;
        case Opcode::OP_DBL_MUL:
            std::forward<tempo_utils::LogMessage>(message) << "OP_DBL_MUL";
            break;
        case Opcode::OP_DBL_DIV:
            std::forward<tempo_utils::LogMessage>(message) << "OP_DBL_DIV";
            break;
        case Opcode::OP_DBL_NEG:
            std::forward<tempo_utils::LogMessage>(message) << "OP_DBL_NEG";
            break;
        case Opcode::OP_BOOL_CMP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_BOOL_CMP";
            break;
        case Opcode::OP_I64_CMP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_I64_CMP";
            break;
        case Opcode::OP_DBL_CMP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_DBL_CMP";
            break;
        case Opcode::OP_CHR_CMP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_CHR_CMP";
            break;
        case Opcode::OP_TYPE_CMP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_TYPE_CMP";
            break;
        case Opcode::OP_LOGICAL_AND:
            std::forward<tempo_utils::LogMessage>(message) << "OP_LOGICAL_AND";
            break;
        case Opcode::OP_LOGICAL_OR:
            std::forward<tempo_utils::LogMessage>(message) << "OP_LOGICAL_OR";
            break;
        case Opcode::OP_LOGICAL_NOT:
            std::forward<tempo_utils::LogMessage>(message) << "OP_LOGICAL_NOT";
            break;
        case Opcode::OP_IF_NIL:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IF_NIL";
            break;
        case Opcode::OP_IF_NOTNIL:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IF_NOTNIL";
            break;
        case Opcode::OP_IF_TRUE:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IF_TRUE";
            break;
        case Opcode::OP_IF_FALSE:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IF_FALSE";
            break;
        case Opcode::OP_IF_ZERO:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IF_ZERO";
            break;
        case Opcode::OP_IF_NOTZERO:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IF_NOTZERO";
            break;
        case Opcode::OP_IF_GT:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IF_GT";
            break;
        case Opcode::OP_IF_GE:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IF_GE";
            break;
        case Opcode::OP_IF_LT:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IF_LT";
            break;
        case Opcode::OP_IF_LE:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IF_LE";
            break;
        case Opcode::OP_JUMP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_JUMP";
            break;
        case Opcode::OP_IMPORT:
            std::forward<tempo_utils::LogMessage>(message) << "OP_IMPORT";
            break;
        case Opcode::OP_CALL_STATIC:
            std::forward<tempo_utils::LogMessage>(message) << "OP_CALL_STATIC";
            break;
        case Opcode::OP_CALL_VIRTUAL:
            std::forward<tempo_utils::LogMessage>(message) << "OP_CALL_VIRTUAL";
            break;
        case Opcode::OP_CALL_CONCEPT:
            std::forward<tempo_utils::LogMessage>(message) << "OP_CALL_CONCEPT";
            break;
        case Opcode::OP_CALL_EXISTENTIAL:
            std::forward<tempo_utils::LogMessage>(message) << "OP_CALL_EXISTENTIAL";
            break;
        case Opcode::OP_TRAP:
            std::forward<tempo_utils::LogMessage>(message) << "OP_TRAP";
            break;
        case Opcode::OP_RETURN:
            std::forward<tempo_utils::LogMessage>(message) << "OP_RETURN";
            break;
        case Opcode::OP_NEW:
            std::forward<tempo_utils::LogMessage>(message) << "OP_NEW";
            break;
        case Opcode::OP_TYPE_OF:
            std::forward<tempo_utils::LogMessage>(message) << "OP_TYPE_OF";
            break;
        case Opcode::OP_INTERRUPT:
            std::forward<tempo_utils::LogMessage>(message) << "OP_INTERRUPT";
            break;
        case Opcode::OP_HALT:
            std::forward<tempo_utils::LogMessage>(message) << "OP_HALT";
            break;
        case Opcode::OP_ABORT:
            std::forward<tempo_utils::LogMessage>(message) << "OP_ABORT";
            break;
        case Opcode::LAST_:
        default:
            std::forward<tempo_utils::LogMessage>(message) << "???";
            break;
    }
    return std::move(message);
}
