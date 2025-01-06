
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/scan_proc.h>

struct ScanProcData {
    lyric_common::ModuleLocation location;
    lyric_object::LyricObject object;
    lyric_archiver::SymbolReferenceSet *symbolReferenceSet;
};

static tempo_utils::Result<lyric_common::SymbolUrl>
lookup_symbol(
    lyric_object::LinkageSection section,
    tu_uint32 address,
    ScanProcData &data)
{
    auto root = data.object.getObject();
    if (lyric_object::IS_NEAR(address)) {
        auto symbolPath = root.getSymbolPath(section, lyric_object::GET_DESCRIPTOR_OFFSET(address));
        if (!symbolPath.isValid())
            return lyric_archiver::ArchiverStatus::forCondition(
                lyric_archiver::ArchiverCondition::kArchiverInvariant, "no such symbol");
        return lyric_common::SymbolUrl(data.location, symbolPath);
    } else if (lyric_object::IS_FAR(address)) {
        auto link = root.getLink(lyric_object::GET_LINK_OFFSET(address));
        if (link.getLinkageSection() != section)
            return lyric_archiver::ArchiverStatus::forCondition(
                lyric_archiver::ArchiverCondition::kArchiverInvariant, "invalid symbol");
        return link.getLinkUrl();
    } else {
        return lyric_archiver::ArchiverStatus::forCondition(
            lyric_archiver::ArchiverCondition::kArchiverInvariant, "invalid symbol");
    }
}

static tempo_utils::Status
scan_load(const lyric_object::OpCell &op, ScanProcData &data)
{
    const auto &operands = op.operands.flags_u8_address_u32;

    lyric_common::SymbolUrl symbolUrl;
    switch (operands.flags) {

        case lyric_object::LOAD_ARGUMENT:
        case lyric_object::LOAD_LEXICAL:
        case lyric_object::LOAD_LOCAL:
            return {};

        case lyric_object::LOAD_ENUM: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Enum, operands.address, data));
            break;
        }
        case lyric_object::LOAD_FIELD: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Field, operands.address, data));
            break;
        }
        case lyric_object::LOAD_INSTANCE: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Instance, operands.address, data));
            break;
        }
        case lyric_object::LOAD_STATIC: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Static, operands.address, data));
            break;
        }
        default:
            return lyric_archiver::ArchiverStatus::forCondition(
                lyric_archiver::ArchiverCondition::kArchiverInvariant, "invalid load type");
    }
    data.symbolReferenceSet->insertReference(symbolUrl);

    return {};
}

static tempo_utils::Status
scan_descriptor(const lyric_object::OpCell &op, ScanProcData &data)
{
    const auto &operands = op.operands.flags_u8_address_u32;
    auto linkage = lyric_object::descriptor_to_linkage_section(operands.flags);

    lyric_common::SymbolUrl symbolUrl;
    switch (linkage) {

        case lyric_object::LinkageSection::Action:
        case lyric_object::LinkageSection::Call:
        case lyric_object::LinkageSection::Class:
        case lyric_object::LinkageSection::Concept:
        case lyric_object::LinkageSection::Enum:
        case lyric_object::LinkageSection::Existential:
        case lyric_object::LinkageSection::Field:
        case lyric_object::LinkageSection::Instance:
        case lyric_object::LinkageSection::Struct:
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(linkage, operands.address, data));
            break;

        default:
            return lyric_archiver::ArchiverStatus::forCondition(
                lyric_archiver::ArchiverCondition::kArchiverInvariant, "invalid load type");
    }
    data.symbolReferenceSet->insertReference(symbolUrl);

    return {};
}

static tempo_utils::Status
scan_store(const lyric_object::OpCell &op, ScanProcData &data)
{
    const auto &operands = op.operands.flags_u8_address_u32;

    lyric_common::SymbolUrl symbolUrl;
    switch (operands.flags) {

        case lyric_object::STORE_ARGUMENT:
        case lyric_object::STORE_LEXICAL:
        case lyric_object::STORE_LOCAL:
            return {};

        case lyric_object::STORE_FIELD: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Field, operands.address, data));
            break;
        }
        case lyric_object::STORE_STATIC: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Static, operands.address, data));
            break;
        }

        default:
            return lyric_archiver::ArchiverStatus::forCondition(
                lyric_archiver::ArchiverCondition::kArchiverInvariant, "invalid store type");
    }
    data.symbolReferenceSet->insertReference(symbolUrl);

    return {};
}

static tempo_utils::Status
scan_call(const lyric_object::OpCell &op, ScanProcData &data)
{
    auto &operands = op.operands.flags_u8_address_u32_placement_u16;

    lyric_common::SymbolUrl symbolUrl;
    switch (op.opcode) {
        case lyric_object::Opcode::OP_CALL_STATIC: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Call, operands.address, data));
            break;
        }
        case lyric_object::Opcode::OP_CALL_VIRTUAL: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Call, operands.address, data));
            break;
        }
        case lyric_object::Opcode::OP_CALL_EXISTENTIAL: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Call, operands.address, data));
            break;
        }
        case lyric_object::Opcode::OP_CALL_CONCEPT: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Action, operands.address, data));
            break;
        }
        default:
            return lyric_archiver::ArchiverStatus::forCondition(
                lyric_archiver::ArchiverCondition::kArchiverInvariant, "invalid call opcode");
    }
    data.symbolReferenceSet->insertReference(symbolUrl);

    return {};
}

static tempo_utils::Status
scan_new(const lyric_object::OpCell &op, ScanProcData &data)
{
    if (op.opcode != lyric_object::Opcode::OP_NEW)
        return lyric_archiver::ArchiverStatus::forCondition(
            lyric_archiver::ArchiverCondition::kArchiverInvariant, "invalid call opcode");

    auto &operands = op.operands.flags_u8_address_u32_placement_u16;

    lyric_common::SymbolUrl symbolUrl;
    switch (lyric_object::GET_NEW_TYPE(operands.flags)) {
        case lyric_object::NEW_CLASS: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Class, operands.address, data));
            break;
        }
        case lyric_object::NEW_ENUM: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Enum, operands.address, data));
            break;
        }
        case lyric_object::NEW_INSTANCE: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Instance, operands.address, data));
            break;
        }
        case lyric_object::NEW_STRUCT: {
            TU_ASSIGN_OR_RETURN(symbolUrl, lookup_symbol(
                lyric_object::LinkageSection::Struct, operands.address, data));
            break;
        }
        default:
            return lyric_archiver::ArchiverStatus::forCondition(
                lyric_archiver::ArchiverCondition::kArchiverInvariant, "invalid new type");
    }
    data.symbolReferenceSet->insertReference(symbolUrl);

    return {};
}

tempo_utils::Status
lyric_archiver::scan_proc(
    const lyric_common::ModuleLocation &location,
    const lyric_object::LyricObject &object,
    lyric_object::BytecodeIterator code,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    ScanProcData data;
    data.location = location;
    data.object = object;
    data.symbolReferenceSet = &symbolReferenceSet;

    lyric_object::OpCell op;

    // append each instruction to the proc
    while (code.getNext(op)) {
        switch (op.opcode) {

            case lyric_object::Opcode::OP_NOOP:
            case lyric_object::Opcode::OP_NIL:
            case lyric_object::Opcode::OP_UNDEF:
            case lyric_object::Opcode::OP_TRUE:
            case lyric_object::Opcode::OP_FALSE:
            case lyric_object::Opcode::OP_I64:
            case lyric_object::Opcode::OP_DBL:
            case lyric_object::Opcode::OP_CHR:
            case lyric_object::Opcode::OP_STRING:
            case lyric_object::Opcode::OP_URL:
            case lyric_object::Opcode::OP_LITERAL:
            case lyric_object::Opcode::OP_SYNTHETIC:
            case lyric_object::Opcode::OP_POP:
            case lyric_object::Opcode::OP_DUP:
            case lyric_object::Opcode::OP_PICK:
            case lyric_object::Opcode::OP_DROP:
            case lyric_object::Opcode::OP_RPICK:
            case lyric_object::Opcode::OP_RDROP:
            case lyric_object::Opcode::OP_I64_ADD:
            case lyric_object::Opcode::OP_I64_SUB:
            case lyric_object::Opcode::OP_I64_MUL:
            case lyric_object::Opcode::OP_I64_DIV:
            case lyric_object::Opcode::OP_I64_NEG:
            case lyric_object::Opcode::OP_DBL_ADD:
            case lyric_object::Opcode::OP_DBL_SUB:
            case lyric_object::Opcode::OP_DBL_MUL:
            case lyric_object::Opcode::OP_DBL_DIV:
            case lyric_object::Opcode::OP_DBL_NEG:
            case lyric_object::Opcode::OP_BOOL_CMP:
            case lyric_object::Opcode::OP_I64_CMP:
            case lyric_object::Opcode::OP_DBL_CMP:
            case lyric_object::Opcode::OP_CHR_CMP:
            case lyric_object::Opcode::OP_TYPE_CMP:
            case lyric_object::Opcode::OP_LOGICAL_AND:
            case lyric_object::Opcode::OP_LOGICAL_OR:
            case lyric_object::Opcode::OP_LOGICAL_NOT:
            case lyric_object::Opcode::OP_IF_NIL:
            case lyric_object::Opcode::OP_IF_NOTNIL:
            case lyric_object::Opcode::OP_IF_TRUE:
            case lyric_object::Opcode::OP_IF_FALSE:
            case lyric_object::Opcode::OP_IF_ZERO:
            case lyric_object::Opcode::OP_IF_NOTZERO:
            case lyric_object::Opcode::OP_IF_GT:
            case lyric_object::Opcode::OP_IF_GE:
            case lyric_object::Opcode::OP_IF_LT:
            case lyric_object::Opcode::OP_IF_LE:
            case lyric_object::Opcode::OP_JUMP:
            case lyric_object::Opcode::OP_RETURN:
            case lyric_object::Opcode::OP_TRAP:
            case lyric_object::Opcode::OP_TYPE_OF:
            case lyric_object::Opcode::OP_INTERRUPT:
            case lyric_object::Opcode::OP_HALT:
            case lyric_object::Opcode::OP_ABORT:
                break;

            case lyric_object::Opcode::OP_LOAD:
                TU_RETURN_IF_NOT_OK (scan_load(op, data));
                break;

            case lyric_object::Opcode::OP_DESCRIPTOR:
                TU_RETURN_IF_NOT_OK (scan_descriptor(op, data));
                break;

            case lyric_object::Opcode::OP_STORE:
                TU_RETURN_IF_NOT_OK (scan_store(op, data));
                break;

            case lyric_object::Opcode::OP_CALL_STATIC:
            case lyric_object::Opcode::OP_CALL_VIRTUAL:
            case lyric_object::Opcode::OP_CALL_CONCEPT:
            case lyric_object::Opcode::OP_CALL_EXISTENTIAL:
                TU_RETURN_IF_NOT_OK (scan_call(op, data));
                break;

            case lyric_object::Opcode::OP_NEW:
                TU_RETURN_IF_NOT_OK (scan_new(op, data));
                break;

            case lyric_object::Opcode::OP_STATIC:
            case lyric_object::Opcode::OP_VA_LOAD:
            case lyric_object::Opcode::OP_VA_SIZE:
            case lyric_object::Opcode::OP_IMPORT:
            default:
                return ArchiverStatus::forCondition(
                    ArchiverCondition::kArchiverInvariant, "invalid opcode");
        }
    }

    return {};
}