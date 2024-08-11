
#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_split.h>

#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_constant.h>
#include <lyric_compiler/internal/compile_new.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_compiler::internal::compile_param_initializer(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    const lyric_assembler::Parameter &param,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    auto identifier = absl::StrCat("$init$", param.name);

    // declare the initializer
    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
        identifier, lyric_object::AccessType::Public, templateParameters));

    // define the initializer without a return type
    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}));

    // compile the initializer body
    lyric_schema::LyricAstId initializerId{};
    moduleEntry.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, initializerId);
    lyric_common::TypeDef bodyType;

    switch (initializerId) {
        case lyric_schema::LyricAstId::New: {
            auto compileNewResult = compile_new(procHandle->procBlock(),
                walker, moduleEntry, param.typeDef);
            if (compileNewResult.isStatus())
                return compileNewResult.getStatus();
            bodyType = compileNewResult.getResult();
            break;
        }
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::SymbolRef: {
            TU_ASSIGN_OR_RETURN (bodyType, compile_constant(procHandle->procBlock(), walker, moduleEntry));
            break;
        }
        default:
            block->throwSyntaxError(walker, "invalid param initializer");
    }

    procHandle->putExitType(bodyType);

    // add return instruction
    TU_RETURN_IF_NOT_OK (procHandle->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN));

    // finalize the initializer to set the return type
    TU_RETURN_IF_NOT_OK (callSymbol->finalizeCall());

    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(param.typeDef, bodyType));
    if (!isReturnable)
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "param initializer is incompatible with type {}", param.typeDef.toString());

    // validate that each exit returns the expected type
    for (auto it = procHandle->exitTypesBegin(); it != procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(param.typeDef, *it));
        if (!isReturnable)
            return block->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "param initializer is incompatible with type {}", param.typeDef.toString());
    }

    return callSymbol->getSymbolUrl();
}

tempo_utils::Status
lyric_compiler::internal::compile_member_initializer(
    lyric_assembler::FieldSymbol *fieldSymbol,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (fieldSymbol != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

//    auto identifier = absl::StrCat("$init$", memberName);
//
//    // declare the initializer
//    lyric_assembler::CallSymbol *callSymbol;
//    TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
//        identifier, lyric_object::AccessType::Public, templateParameters));

    auto memberType = fieldSymbol->getAssignableType();

    // define the initializer without a return type
    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, fieldSymbol->defineInitializer());

    auto *block = procHandle->procBlock();

    // compile the initializer body
    lyric_schema::LyricAstId initializerId{};
    moduleEntry.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, initializerId);
    lyric_common::TypeDef bodyType;

    switch (initializerId) {
        case lyric_schema::LyricAstId::New: {
            auto compileNewResult = compile_new(procHandle->procBlock(),
                walker, moduleEntry, memberType);
            if (compileNewResult.isStatus())
                return compileNewResult.getStatus();
            bodyType = compileNewResult.getResult();
            break;
        }
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::SymbolRef: {
            TU_ASSIGN_OR_RETURN (bodyType, compile_constant(procHandle->procBlock(), walker, moduleEntry));
            break;
        }
        default:
            block->throwSyntaxError(walker, "invalid member initializer");
    }

    procHandle->putExitType(bodyType);

    // add return instruction
    TU_RETURN_IF_NOT_OK (procHandle->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN));

    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(memberType, bodyType));
    if (!isReturnable)
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "member initializer is incompatible with type {}", memberType.toString());

    // validate that each exit returns the expected type
    for (auto it = procHandle->exitTypesBegin(); it != procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(memberType, *it));
        if (!isReturnable)
            return block->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "member initializer is incompatible with type {}", memberType.toString());
    }

    return {};
}

tempo_utils::Status
lyric_compiler::internal::compile_static_initializer(
    lyric_assembler::StaticSymbol *staticSymbol,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (staticSymbol != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    // declare the initializer
    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, staticSymbol->defineInitializer());
    auto *code = procHandle->procCode();
    auto *block = procHandle->procBlock();

    // compile the initializer body
    lyric_common::TypeDef bodyType;
    TU_ASSIGN_OR_RETURN (bodyType, compile_expression(block, walker, moduleEntry));

    // add return instruction
    auto status = code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    auto staticType = staticSymbol->getAssignableType();
    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(staticType, bodyType));
    if (!isReturnable)
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "static initializer is incompatible with type {}", staticType.toString());

    // validate that each exit returns the expected type
    for (auto it = procHandle->exitTypesBegin(); it != procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(staticType, *it));
        if (!isReturnable)
            return block->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "static initializer is incompatible with type {}", staticType.toString());
    }

    return {};
}
