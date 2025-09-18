
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

static const char *OP_UNKNOWN_name             = "???";
static const char *OP_NOOP_name                = "OP_NOOP";
static const char *OP_UNDEF_name               = "OP_UNDEF";
static const char *OP_NIL_name                 = "OP_NIL";
static const char *OP_TRUE_name                = "OP_TRUE";
static const char *OP_FALSE_name               = "OP_FALSE";
static const char *OP_I64_name                 = "OP_I64";
static const char *OP_DBL_name                 = "OP_DBL";
static const char *OP_CHR_name                 = "OP_CHR";
static const char *OP_LITERAL_name             = "OP_LITERAL";
static const char *OP_STRING_name              = "OP_STRING";
static const char *OP_URL_name                 = "OP_URL";
static const char *OP_STATIC_name              = "OP_STATIC";
static const char *OP_SYNTHETIC_name           = "OP_SYNTHETIC";
static const char *OP_DESCRIPTOR_name          = "OP_DESCRIPTOR";
static const char *OP_LOAD_name                = "OP_LOAD";
static const char *OP_STORE_name               = "OP_STORE";
static const char *OP_VA_LOAD_name             = "OP_VA_LOAD";
static const char *OP_VA_SIZE_name             = "OP_VA_SIZE";
static const char *OP_POP_name                 = "OP_POP";
static const char *OP_DUP_name                 = "OP_DUP";
static const char *OP_PICK_name                = "OP_PICK";
static const char *OP_DROP_name                = "OP_DROP";
static const char *OP_RPICK_name               = "OP_RPICK";
static const char *OP_RDROP_name               = "OP_RDROP";
static const char *OP_I64_ADD_name             = "OP_I64_ADD";
static const char *OP_I64_SUB_name             = "OP_I64_SUB";
static const char *OP_I64_MUL_name             = "OP_I64_MUL";
static const char *OP_I64_DIV_name             = "OP_I64_DIV";
static const char *OP_I64_NEG_name             = "OP_I64_NEG";
static const char *OP_DBL_ADD_name             = "OP_DBL_ADD";
static const char *OP_DBL_SUB_name             = "OP_DBL_SUB";
static const char *OP_DBL_MUL_name             = "OP_DBL_MUL";
static const char *OP_DBL_DIV_name             = "OP_DBL_DIV";
static const char *OP_DBL_NEG_name             = "OP_DBL_NEG";
static const char *OP_BOOL_CMP_name            = "OP_BOOL_CMP";
static const char *OP_I64_CMP_name             = "OP_I64_CMP";
static const char *OP_DBL_CMP_name             = "OP_DBL_CMP";
static const char *OP_CHR_CMP_name             = "OP_CHR_CMP";
static const char *OP_TYPE_CMP_name            = "OP_TYPE_CMP";
static const char *OP_LOGICAL_AND_name         = "OP_LOGICAL_AND";
static const char *OP_LOGICAL_OR_name          = "OP_LOGICAL_OR";
static const char *OP_LOGICAL_NOT_name         = "OP_LOGICAL_NOT";
static const char *OP_BITWISE_AND_name         = "OP_BITWISE_AND";
static const char *OP_BITWISE_OR_name          = "OP_BITWISE_OR";
static const char *OP_BITWISE_XOR_name         = "OP_BITWISE_XOR";
static const char *OP_BITWISE_RIGHT_SHIFT_name = "OP_BITWISE_RIGHT_SHIFT";
static const char *OP_BITWISE_LEFT_SHIFT_name  = "OP_BITWISE_LEFT_SHIFT";
static const char *OP_IF_NIL_name              = "OP_IF_NIL";
static const char *OP_IF_NOTNIL_name           = "OP_IF_NOTNIL";
static const char *OP_IF_TRUE_name             = "OP_IF_TRUE";
static const char *OP_IF_FALSE_name            = "OP_IF_FALSE";
static const char *OP_IF_ZERO_name             = "OP_IF_ZERO";
static const char *OP_IF_NOTZERO_name          = "OP_IF_NOTZERO";
static const char *OP_IF_GT_name               = "OP_IF_GT";
static const char *OP_IF_GE_name               = "OP_IF_GE";
static const char *OP_IF_LT_name               = "OP_IF_LT";
static const char *OP_IF_LE_name               = "OP_IF_LE";
static const char *OP_JUMP_name                = "OP_JUMP";
static const char *OP_IMPORT_name              = "OP_IMPORT";
static const char *OP_CALL_STATIC_name         = "OP_CALL_STATIC";
static const char *OP_CALL_VIRTUAL_name        = "OP_CALL_VIRTUAL";
static const char *OP_CALL_CONCEPT_name        = "OP_CALL_CONCEPT";
static const char *OP_CALL_EXISTENTIAL_name    = "OP_CALL_EXISTENTIAL";
static const char *OP_TRAP_name                = "OP_TRAP";
static const char *OP_RETURN_name              = "OP_RETURN";
static const char *OP_NEW_name                 = "OP_NEW";
static const char *OP_TYPE_OF_name             = "OP_TYPE_OF";
static const char *OP_INTERRUPT_name           = "OP_INTERRUPT";
static const char *OP_HALT_name                = "OP_HALT";
static const char *OP_ABORT_name               = "OP_ABORT";

const char *lyric_object::opcode_to_name(Opcode opcode)
{
    switch (opcode) {
        case Opcode::OP_NOOP:
            return OP_NOOP_name;
        case Opcode::OP_UNDEF:
            return OP_UNDEF_name;
        case Opcode::OP_NIL:
            return OP_NIL_name;
        case Opcode::OP_TRUE:
            return OP_TRUE_name;
        case Opcode::OP_FALSE:
            return OP_FALSE_name;
        case Opcode::OP_I64:
            return OP_I64_name;
        case Opcode::OP_DBL:
            return OP_DBL_name;
        case Opcode::OP_CHR:
            return OP_CHR_name;
        case Opcode::OP_LITERAL:
            return OP_LITERAL_name;
        case Opcode::OP_STRING:
            return OP_STRING_name;
        case Opcode::OP_URL:
            return OP_URL_name;
        case Opcode::OP_STATIC:
            return OP_STATIC_name;
        case Opcode::OP_SYNTHETIC:
            return OP_SYNTHETIC_name;
        case Opcode::OP_DESCRIPTOR:
            return OP_DESCRIPTOR_name;
        case Opcode::OP_LOAD:
            return OP_LOAD_name;
        case Opcode::OP_STORE:
            return OP_STORE_name;
        case Opcode::OP_VA_LOAD:
            return OP_VA_LOAD_name;
        case Opcode::OP_VA_SIZE:
            return OP_VA_SIZE_name;
        case Opcode::OP_POP:
            return OP_POP_name;
        case Opcode::OP_DUP:
            return OP_DUP_name;
        case Opcode::OP_PICK:
            return OP_PICK_name;
        case Opcode::OP_DROP:
            return OP_DROP_name;
        case Opcode::OP_RPICK:
            return OP_RPICK_name;
        case Opcode::OP_RDROP:
            return OP_RDROP_name;
        case Opcode::OP_I64_ADD:
            return OP_I64_ADD_name;
        case Opcode::OP_I64_SUB:
            return OP_I64_SUB_name;
        case Opcode::OP_I64_MUL:
            return OP_I64_MUL_name;
        case Opcode::OP_I64_DIV:
            return OP_I64_DIV_name;
        case Opcode::OP_I64_NEG:
            return OP_I64_NEG_name;
        case Opcode::OP_DBL_ADD:
            return OP_DBL_ADD_name;
        case Opcode::OP_DBL_SUB:
            return OP_DBL_SUB_name;
        case Opcode::OP_DBL_MUL:
            return OP_DBL_MUL_name;
        case Opcode::OP_DBL_DIV:
            return OP_DBL_DIV_name;
        case Opcode::OP_DBL_NEG:
            return OP_DBL_NEG_name;
        case Opcode::OP_BOOL_CMP:
            return OP_BOOL_CMP_name;
        case Opcode::OP_I64_CMP:
            return OP_I64_CMP_name;
        case Opcode::OP_DBL_CMP:
            return OP_DBL_CMP_name;
        case Opcode::OP_CHR_CMP:
            return OP_CHR_CMP_name;
        case Opcode::OP_TYPE_CMP:
            return OP_TYPE_CMP_name;
        case Opcode::OP_LOGICAL_AND:
            return OP_LOGICAL_AND_name;
        case Opcode::OP_LOGICAL_OR:
            return OP_LOGICAL_OR_name;
        case Opcode::OP_LOGICAL_NOT:
            return OP_LOGICAL_NOT_name;
        case Opcode::OP_BITWISE_AND:
            return OP_BITWISE_AND_name;
        case Opcode::OP_BITWISE_OR:
            return OP_BITWISE_OR_name;
        case Opcode::OP_BITWISE_XOR:
            return OP_BITWISE_XOR_name;
        case Opcode::OP_BITWISE_RIGHT_SHIFT:
            return OP_BITWISE_RIGHT_SHIFT_name;
        case Opcode::OP_BITWISE_LEFT_SHIFT:
            return OP_BITWISE_LEFT_SHIFT_name;
        case Opcode::OP_IF_NIL:
            return OP_IF_NIL_name;
        case Opcode::OP_IF_NOTNIL:
            return OP_IF_NOTNIL_name;
        case Opcode::OP_IF_TRUE:
            return OP_IF_TRUE_name;
        case Opcode::OP_IF_FALSE:
            return OP_IF_FALSE_name;
        case Opcode::OP_IF_ZERO:
            return OP_IF_ZERO_name;
        case Opcode::OP_IF_NOTZERO:
            return OP_IF_NOTZERO_name;
        case Opcode::OP_IF_GT:
            return OP_IF_GT_name;
        case Opcode::OP_IF_GE:
            return OP_IF_GE_name;
        case Opcode::OP_IF_LT:
            return OP_IF_LT_name;
        case Opcode::OP_IF_LE:
            return OP_IF_LE_name;
        case Opcode::OP_JUMP:
            return OP_JUMP_name;
        case Opcode::OP_IMPORT:
            return OP_IMPORT_name;
        case Opcode::OP_CALL_STATIC:
            return OP_CALL_STATIC_name;
        case Opcode::OP_CALL_VIRTUAL:
            return OP_CALL_VIRTUAL_name;
        case Opcode::OP_CALL_CONCEPT:
            return OP_CALL_CONCEPT_name;
        case Opcode::OP_CALL_EXISTENTIAL:
            return OP_CALL_EXISTENTIAL_name;
        case Opcode::OP_TRAP:
            return OP_TRAP_name;
        case Opcode::OP_RETURN:
            return OP_RETURN_name;
        case Opcode::OP_NEW:
            return OP_NEW_name;
        case Opcode::OP_TYPE_OF:
            return OP_TYPE_OF_name;
        case Opcode::OP_INTERRUPT:
            return OP_INTERRUPT_name;
        case Opcode::OP_HALT:
            return OP_HALT_name;
        case Opcode::OP_ABORT:
            return OP_ABORT_name;
        case Opcode::OP_UNKNOWN:
        case Opcode::LAST_:
        default:
            return OP_UNKNOWN_name;
    }
}

tempo_utils::LogMessage&& lyric_object::operator<<(tempo_utils::LogMessage&& message, Opcode opcode) {
    std::forward<tempo_utils::LogMessage>(message) << opcode_to_name(opcode);
    return std::move(message);
}
