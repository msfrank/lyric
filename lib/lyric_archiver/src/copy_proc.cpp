
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/copy_proc.h>
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/local_variable.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/proc_builder.h>
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>

struct CopyProcData {
    lyric_common::ModuleLocation location;
    lyric_object::LyricObject object;
    lyric_common::SymbolUrl activation;
    lyric_assembler::CodeFragment *fragment;
    lyric_assembler::ObjectState *state;
    std::vector<tu_uint32> instructionOffsets;
    std::vector<std::pair<tu_uint32,lyric_assembler::JumpTarget>> jumpTargets;
};

static tempo_utils::Status
apply_literal(const lyric_object::OpCell &op, CopyProcData &data)
{
    return lyric_assembler::AssemblerStatus::forCondition(
        lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid synthetic type");
}

static tempo_utils::Status
apply_synthetic(const lyric_object::OpCell &op, CopyProcData &data)
{
    const auto &operands = op.operands.type_u8;
    switch (operands.type) {
        case lyric_object::SYNTHETIC_THIS:
            return data.fragment->loadThis();
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid synthetic type");
    }
}

static lyric_common::SymbolUrl
lookup_symbol(
    tu_uint32 address,
    lyric_object::LinkageSection section,
    CopyProcData &data)
{
    auto root = data.object.getObject();
    if (lyric_object::IS_NEAR(address)) {
        auto symbolPath = root.getSymbolPath(section, address);
        if (!symbolPath.isValid())
            return {};
        return {data.location, symbolPath};
    } else {
        auto link = root.getLink(address);
        if (link.getLinkageSection() != section)
            return {};
        return link.getLinkUrl();
    }
}

inline lyric_common::SymbolUrl
get_variable_url(std::string_view name, CopyProcData &data)
{
    auto basePath = data.activation.getSymbolPath();
    lyric_common::SymbolPath variablePath(basePath.getPath(), name);
    return lyric_common::SymbolUrl(data.activation.getModuleLocation(), variablePath);
}

static tempo_utils::Status
apply_load(const lyric_object::OpCell &op, CopyProcData &data)
{
    const auto &operands = op.operands.flags_u8_address_u32;
    switch (operands.flags) {

        case lyric_object::LOAD_ARGUMENT: {
            auto name = absl::StrCat("arg", operands.address);
            auto argumentUrl = get_variable_url(name, data);
            auto argumentSymbol = std::make_unique<lyric_assembler::ArgumentVariable>(
                argumentUrl, lyric_common::TypeDef{}, lyric_assembler::BindingType::Invalid,
                lyric_assembler::ArgumentOffset(operands.address));
            auto *symbolCache = data.state->symbolCache();
            TU_RETURN_IF_NOT_OK (symbolCache->insertSymbol(argumentUrl, argumentSymbol.get()));
            auto *symbol = argumentSymbol.release();
            return data.fragment->loadData(symbol);
        }
        case lyric_object::LOAD_LEXICAL: {
            auto name = absl::StrCat("lex", operands.address);
            auto lexicalUrl = get_variable_url(name, data);
            auto lexicalSymbol = std::make_unique<lyric_assembler::LexicalVariable>(
                lexicalUrl, lyric_common::TypeDef{}, lyric_assembler::LexicalOffset(operands.address));
            auto *symbolCache = data.state->symbolCache();
            TU_RETURN_IF_NOT_OK (symbolCache->insertSymbol(lexicalUrl, lexicalSymbol.get()));
            auto *symbol = lexicalSymbol.release();
            return data.fragment->loadData(symbol);
        }
        case lyric_object::LOAD_LOCAL: {
            auto name = absl::StrCat("loc", operands.address);
            auto localUrl = get_variable_url(name, data);
            auto localSymbol = std::make_unique<lyric_assembler::LocalVariable>(
                localUrl, lyric_object::AccessType::Invalid, lyric_common::TypeDef{},
                lyric_assembler::LocalOffset(operands.address));
            auto *symbolCache = data.state->symbolCache();
            TU_RETURN_IF_NOT_OK (symbolCache->insertSymbol(localUrl, localSymbol.get()));
            auto *symbol = localSymbol.release();
            return data.fragment->loadData(symbol);
        }
        case lyric_object::LOAD_ENUM: {
            auto enumUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Enum, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::EnumSymbol *enumSymbol;
            TU_ASSIGN_OR_RETURN (enumSymbol, importCache->importEnum(enumUrl));
            return data.fragment->loadData(enumSymbol);
        }
        case lyric_object::LOAD_FIELD: {
            auto fieldUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Field, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::FieldSymbol *fieldSymbol;
            TU_ASSIGN_OR_RETURN (fieldSymbol, importCache->importField(fieldUrl));
            return data.fragment->loadData(fieldSymbol);
        }
        case lyric_object::LOAD_INSTANCE: {
            auto instanceUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Instance, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::InstanceSymbol *instanceSymbol;
            TU_ASSIGN_OR_RETURN (instanceSymbol, importCache->importInstance(instanceUrl));
            return data.fragment->loadData(instanceSymbol);
        }
        case lyric_object::LOAD_STATIC: {
            auto staticUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Static, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::StaticSymbol *staticSymbol;
            TU_ASSIGN_OR_RETURN (staticSymbol, importCache->importStatic(staticUrl));
            return data.fragment->loadData(staticSymbol);
        }
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid load type");
    }
}

static tempo_utils::Status
apply_descriptor(const lyric_object::OpCell &op, CopyProcData &data)
{
    const auto &operands = op.operands.flags_u8_address_u32;

    auto linkage = lyric_object::descriptor_to_linkage_section(operands.flags);
    switch (linkage) {

        case lyric_object::LinkageSection::Action: {
            auto actionUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Action, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::ActionSymbol *actionSymbol;
            TU_ASSIGN_OR_RETURN (actionSymbol, importCache->importAction(actionUrl));
            return data.fragment->loadDescriptor(actionSymbol);
        }
        case lyric_object::LinkageSection::Call: {
            auto callUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Call, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::CallSymbol *callSymbol;
            TU_ASSIGN_OR_RETURN (callSymbol, importCache->importCall(callUrl));
            return data.fragment->loadDescriptor(callSymbol);
        }
        case lyric_object::LinkageSection::Class: {
            auto classUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Class, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::ClassSymbol *classSymbol;
            TU_ASSIGN_OR_RETURN (classSymbol, importCache->importClass(classUrl));
            return data.fragment->loadDescriptor(classSymbol);
        }
        case lyric_object::LinkageSection::Concept: {
            auto conceptUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Concept, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::ConceptSymbol *conceptSymbol;
            TU_ASSIGN_OR_RETURN (conceptSymbol, importCache->importConcept(conceptUrl));
            return data.fragment->loadDescriptor(conceptSymbol);
        }
        case lyric_object::LinkageSection::Enum: {
            auto enumUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Enum, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::EnumSymbol *enumSymbol;
            TU_ASSIGN_OR_RETURN (enumSymbol, importCache->importEnum(enumUrl));
            return data.fragment->loadDescriptor(enumSymbol);
        }
        case lyric_object::LinkageSection::Existential: {
            auto existentialUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Existential, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::ExistentialSymbol *existentialSymbol;
            TU_ASSIGN_OR_RETURN (existentialSymbol, importCache->importExistential(existentialUrl));
            return data.fragment->loadDescriptor(existentialSymbol);
        }
        case lyric_object::LinkageSection::Field: {
            auto fieldUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Field, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::FieldSymbol *fieldSymbol;
            TU_ASSIGN_OR_RETURN (fieldSymbol, importCache->importField(fieldUrl));
            return data.fragment->loadDescriptor(fieldSymbol);
        }
        case lyric_object::LinkageSection::Instance: {
            auto instanceUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Instance, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::InstanceSymbol *instanceSymbol;
            TU_ASSIGN_OR_RETURN (instanceSymbol, importCache->importInstance(instanceUrl));
            return data.fragment->loadDescriptor(instanceSymbol);
        }
        case lyric_object::LinkageSection::Struct: {
            auto structUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Struct, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::StructSymbol *structSymbol;
            TU_ASSIGN_OR_RETURN (structSymbol, importCache->importStruct(structUrl));
            return data.fragment->loadData(structSymbol);
        }
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid load type");
    }
}

static tempo_utils::Status
apply_store(const lyric_object::OpCell &op, CopyProcData &data)
{
    const auto &operands = op.operands.flags_u8_address_u32;
    switch (operands.flags) {

        case lyric_object::STORE_ARGUMENT: {
            auto name = absl::StrCat("arg", operands.address);
            auto argumentUrl = get_variable_url(name, data);
            auto argumentSymbol = std::make_unique<lyric_assembler::ArgumentVariable>(
                argumentUrl, lyric_common::TypeDef{}, lyric_assembler::BindingType::Invalid,
                lyric_assembler::ArgumentOffset(operands.address));
            auto *symbolCache = data.state->symbolCache();
            TU_RETURN_IF_NOT_OK (symbolCache->insertSymbol(argumentUrl, argumentSymbol.get()));
            auto *symbol = argumentSymbol.release();
            return data.fragment->storeData(symbol);
        }
        case lyric_object::STORE_LEXICAL: {
            auto name = absl::StrCat("lex", operands.address);
            auto lexicalUrl = get_variable_url(name, data);
            auto lexicalSymbol = std::make_unique<lyric_assembler::LexicalVariable>(
                lexicalUrl, lyric_common::TypeDef{}, lyric_assembler::LexicalOffset(operands.address));
            auto *symbolCache = data.state->symbolCache();
            TU_RETURN_IF_NOT_OK (symbolCache->insertSymbol(lexicalUrl, lexicalSymbol.get()));
            auto *symbol = lexicalSymbol.release();
            return data.fragment->storeData(symbol);
        }
        case lyric_object::STORE_LOCAL: {
            auto name = absl::StrCat("loc", operands.address);
            auto localUrl = get_variable_url(name, data);
            auto localSymbol = std::make_unique<lyric_assembler::LocalVariable>(
                localUrl, lyric_object::AccessType::Invalid, lyric_common::TypeDef{},
                lyric_assembler::LocalOffset(operands.address));
            auto *symbolCache = data.state->symbolCache();
            TU_RETURN_IF_NOT_OK (symbolCache->insertSymbol(localUrl, localSymbol.get()));
            auto *symbol = localSymbol.release();
            return data.fragment->storeData(symbol);
        }
        case lyric_object::STORE_FIELD: {
            auto fieldUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Field, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::FieldSymbol *fieldSymbol;
            TU_ASSIGN_OR_RETURN (fieldSymbol, importCache->importField(fieldUrl));
            return data.fragment->storeData(fieldSymbol);
        }
        case lyric_object::STORE_STATIC: {
            auto staticUrl = lookup_symbol(
                operands.address, lyric_object::LinkageSection::Static, data);
            auto *importCache = data.state->importCache();
            lyric_assembler::StaticSymbol *staticSymbol;
            TU_ASSIGN_OR_RETURN (staticSymbol, importCache->importStatic(staticUrl));
            return data.fragment->storeData(staticSymbol);
        }
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid store type");
    }
}

static tempo_utils::Status
apply_jump(const lyric_object::OpCell &op, CopyProcData &data)
{
    const auto &operands = op.operands.jump_i16;

    tu_uint32 address = (op.offset + 3) + operands.jump;

    lyric_assembler::JumpTarget jumpTarget;
    switch (op.opcode) {
        case lyric_object::Opcode::OP_IF_NIL:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->jumpIfNil());
            break;
        case lyric_object::Opcode::OP_IF_NOTNIL:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->jumpIfNotNil());
            break;
        case lyric_object::Opcode::OP_IF_TRUE:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->jumpIfTrue());
            break;
        case lyric_object::Opcode::OP_IF_FALSE:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->jumpIfFalse());
            break;
        case lyric_object::Opcode::OP_IF_ZERO:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->jumpIfZero());
            break;
        case lyric_object::Opcode::OP_IF_NOTZERO:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->jumpIfNotZero());
            break;
        case lyric_object::Opcode::OP_IF_GT:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->jumpIfGreaterThan());
            break;
        case lyric_object::Opcode::OP_IF_GE:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->jumpIfGreaterOrEqual());
            break;
        case lyric_object::Opcode::OP_IF_LT:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->jumpIfLessThan());
            break;
        case lyric_object::Opcode::OP_IF_LE:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->jumpIfLessOrEqual());
            break;
        case lyric_object::Opcode::OP_JUMP:
            TU_ASSIGN_OR_RETURN (jumpTarget, data.fragment->unconditionalJump());
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid jump opcode");
    }

    data.jumpTargets.emplace_back(address, jumpTarget);

    return {};
}

static tempo_utils::Status
apply_call(const lyric_object::OpCell &op, CopyProcData &data)
{
    TU_UNREACHABLE();
}

tempo_utils::Status
lyric_archiver::copy_proc(
    const lyric_common::ModuleLocation &location,
    const lyric_object::LyricObject &object,
    const lyric_object::ProcHeader &header,
    lyric_object::BytecodeIterator it,
    lyric_assembler::ProcHandle *procHandle,
    lyric_assembler::ObjectState *state)
{
    auto *procCode = procHandle->procCode();

    CopyProcData data;
    data.location = location;
    data.object = object;
    data.activation = procHandle->getActivation();
    data.state = state;
    data.fragment = procCode->rootFragment();

    lyric_object::OpCell op;

    // append each instruction to the proc
    while (it.getNext(op)) {
        switch (op.opcode) {

            case lyric_object::Opcode::OP_NOOP:
                TU_RETURN_IF_NOT_OK (data.fragment->noOperation());
                break;
            case lyric_object::Opcode::OP_NIL:
                TU_RETURN_IF_NOT_OK (data.fragment->immediateNil());
                break;
            case lyric_object::Opcode::OP_UNDEF:
                TU_RETURN_IF_NOT_OK (data.fragment->immediateUndef());
                break;
            case lyric_object::Opcode::OP_TRUE:
                TU_RETURN_IF_NOT_OK (data.fragment->immediateBool(true));
                break;
            case lyric_object::Opcode::OP_FALSE:
                TU_RETURN_IF_NOT_OK (data.fragment->immediateBool(false));
                break;
            case lyric_object::Opcode::OP_I64:
                TU_RETURN_IF_NOT_OK (data.fragment->immediateInt(op.operands.immediate_i64.i64));
                break;
            case lyric_object::Opcode::OP_DBL:
                TU_RETURN_IF_NOT_OK (data.fragment->immediateFloat(op.operands.immediate_dbl.dbl));
                break;
            case lyric_object::Opcode::OP_CHR:
                TU_RETURN_IF_NOT_OK (data.fragment->immediateChar(op.operands.immediate_chr.chr));
                break;

            case lyric_object::Opcode::OP_STRING:
            case lyric_object::Opcode::OP_URL:
            case lyric_object::Opcode::OP_LITERAL:
                TU_RETURN_IF_NOT_OK (apply_literal(op, data));
                break;

            case lyric_object::Opcode::OP_SYNTHETIC:
                TU_RETURN_IF_NOT_OK (apply_synthetic(op, data));
                break;
            case lyric_object::Opcode::OP_LOAD:
                TU_RETURN_IF_NOT_OK (apply_load(op, data));
                break;
            case lyric_object::Opcode::OP_DESCRIPTOR:
                TU_RETURN_IF_NOT_OK (apply_descriptor(op, data));
                break;
            case lyric_object::Opcode::OP_STORE:
                TU_RETURN_IF_NOT_OK (apply_store(op, data));
                break;
            case lyric_object::Opcode::OP_POP:
                TU_RETURN_IF_NOT_OK (data.fragment->popValue());
                break;
            case lyric_object::Opcode::OP_DUP:
                TU_RETURN_IF_NOT_OK (data.fragment->dupValue());
                break;
            case lyric_object::Opcode::OP_PICK:
                TU_RETURN_IF_NOT_OK (data.fragment->pickValue(op.operands.offset_u16.offset));
                break;
            case lyric_object::Opcode::OP_DROP:
                TU_RETURN_IF_NOT_OK (data.fragment->dropValue(op.operands.offset_u16.offset));
                break;
            case lyric_object::Opcode::OP_RPICK:
                TU_RETURN_IF_NOT_OK (data.fragment->rpickValue(op.operands.offset_u16.offset));
                break;
            case lyric_object::Opcode::OP_RDROP:
                TU_RETURN_IF_NOT_OK (data.fragment->rdropValue(op.operands.offset_u16.offset));
                break;
            case lyric_object::Opcode::OP_I64_ADD:
                TU_RETURN_IF_NOT_OK (data.fragment->intAdd());
                break;
            case lyric_object::Opcode::OP_I64_SUB:
                TU_RETURN_IF_NOT_OK (data.fragment->intSubtract());
                break;
            case lyric_object::Opcode::OP_I64_MUL:
                TU_RETURN_IF_NOT_OK (data.fragment->intMultiply());
                break;
            case lyric_object::Opcode::OP_I64_DIV:
                TU_RETURN_IF_NOT_OK (data.fragment->intDivide());
                break;
            case lyric_object::Opcode::OP_I64_NEG:
                TU_RETURN_IF_NOT_OK (data.fragment->intNegate());
                break;
            case lyric_object::Opcode::OP_DBL_ADD:
                TU_RETURN_IF_NOT_OK (data.fragment->floatAdd());
                break;
            case lyric_object::Opcode::OP_DBL_SUB:
                TU_RETURN_IF_NOT_OK (data.fragment->floatSubtract());
                break;
            case lyric_object::Opcode::OP_DBL_MUL:
                TU_RETURN_IF_NOT_OK (data.fragment->floatMultiply());
                break;
            case lyric_object::Opcode::OP_DBL_DIV:
                TU_RETURN_IF_NOT_OK (data.fragment->floatDivide());
                break;
            case lyric_object::Opcode::OP_DBL_NEG:
                TU_RETURN_IF_NOT_OK (data.fragment->floatNegate());
                break;
            case lyric_object::Opcode::OP_BOOL_CMP:
                TU_RETURN_IF_NOT_OK (data.fragment->boolCompare());
                break;
            case lyric_object::Opcode::OP_I64_CMP:
                TU_RETURN_IF_NOT_OK (data.fragment->intCompare());
                break;
            case lyric_object::Opcode::OP_DBL_CMP:
                TU_RETURN_IF_NOT_OK (data.fragment->floatCompare());
                break;
            case lyric_object::Opcode::OP_CHR_CMP:
                TU_RETURN_IF_NOT_OK (data.fragment->charCompare());
                break;
            case lyric_object::Opcode::OP_TYPE_CMP:
                TU_RETURN_IF_NOT_OK (data.fragment->typeCompare());
                break;
            case lyric_object::Opcode::OP_LOGICAL_AND:
                TU_RETURN_IF_NOT_OK (data.fragment->logicalAnd());
                break;
            case lyric_object::Opcode::OP_LOGICAL_OR:
                TU_RETURN_IF_NOT_OK (data.fragment->logicalOr());
                break;
            case lyric_object::Opcode::OP_LOGICAL_NOT:
                TU_RETURN_IF_NOT_OK (data.fragment->logicalNot());
                break;

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
                TU_RETURN_IF_NOT_OK (apply_jump(op, data));
                break;

            case lyric_object::Opcode::OP_CALL_STATIC:
            case lyric_object::Opcode::OP_CALL_VIRTUAL:
            case lyric_object::Opcode::OP_CALL_CONCEPT:
            case lyric_object::Opcode::OP_CALL_EXISTENTIAL:
            case lyric_object::Opcode::OP_NEW:
                TU_RETURN_IF_NOT_OK (apply_call(op, data));
                break;

            case lyric_object::Opcode::OP_RETURN:
                TU_RETURN_IF_NOT_OK (data.fragment->returnToCaller());
                break;

            case lyric_object::Opcode::OP_TRAP:
                TU_RETURN_IF_NOT_OK (data.fragment->trap(
                    op.operands.flags_u8_address_u32.address, op.operands.flags_u8_address_u32.flags));
                break;

            case lyric_object::Opcode::OP_TYPE_OF:
                TU_RETURN_IF_NOT_OK (data.fragment->invokeTypeOf());
                break;
            case lyric_object::Opcode::OP_INTERRUPT:
                TU_RETURN_IF_NOT_OK (data.fragment->invokeInterrupt());
                break;
            case lyric_object::Opcode::OP_HALT:
                TU_RETURN_IF_NOT_OK (data.fragment->invokeHalt());
                break;
            case lyric_object::Opcode::OP_ABORT:
                TU_RETURN_IF_NOT_OK (data.fragment->invokeAbort());
                break;

            case lyric_object::Opcode::OP_STATIC:
            case lyric_object::Opcode::OP_VA_LOAD:
            case lyric_object::Opcode::OP_VA_SIZE:
            case lyric_object::Opcode::OP_IMPORT:
            default:
                return ArchiverStatus::forCondition(
                    ArchiverCondition::kArchiverInvariant, "invalid opcode");
        }

        data.instructionOffsets.push_back(op.offset);
    }

    // sort the jump targets by address descending
    std::sort(data.jumpTargets.begin(), data.jumpTargets.end(),
              [](auto &lhs, auto &rhs) -> bool {
                  return lhs.first > rhs.first;
              }
    );

    absl::flat_hash_map<tu_uint32,lyric_assembler::JumpLabel> jumpLabels;

    // patch jump targets
    for (const auto &entry : data.jumpTargets) {
        const auto &address = entry.first;
        const auto &jumpTarget = entry.second;
        const auto &offsets = data.instructionOffsets;

        lyric_assembler::JumpLabel jumpLabel;
        if (!jumpLabels.contains(address)) {
            auto iptr = std::find(offsets.cbegin(), offsets.cend(), address);
            if (iptr != offsets.cend()) {
                auto index = std::distance(offsets.cbegin(), iptr);
                TU_ASSIGN_OR_RETURN (jumpLabel, data.fragment->insertLabel(index));
            } else {
                TU_ASSIGN_OR_RETURN (jumpLabel, data.fragment->appendLabel());
            }
            jumpLabels[address] = jumpLabel;
        } else {
            jumpLabel = jumpLabels[address];
        }
        TU_RETURN_IF_NOT_OK (data.fragment->patchTarget(jumpTarget, jumpLabel));
    }

    return {};
}