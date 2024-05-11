
#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_split.h>

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
lyric_compiler::internal::compile_default_initializer(
    lyric_assembler::BlockHandle *block,
    const std::string &name,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    const lyric_parser::Assignable &type,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    auto identifier = absl::StrCat("$init$", name);

    // declare the initializer
    auto declareInitializerResult = block->declareFunction(identifier,
        {}, {}, {}, type, lyric_object::AccessType::Public, templateParameters);
    if (declareInitializerResult.isStatus())
        return declareInitializerResult.getStatus();
    auto initializerUrl = declareInitializerResult.getResult();
    auto *sym = block->blockState()->symbolCache()->getSymbol(initializerUrl);
    if (sym == nullptr)
        block->throwAssemblerInvariant("missing call symbol {}", initializerUrl.toString());
    if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        block->throwAssemblerInvariant("invalid call symbol {}", initializerUrl.toString());
    auto *call = cast_symbol_to_call(sym);
    auto *proc = call->callProc();

    // compile the initializer body
    lyric_schema::LyricAstId initializerId{};
    moduleEntry.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, initializerId);
    lyric_common::TypeDef bodyType;

    switch (initializerId) {
        case lyric_schema::LyricAstId::New: {
            auto compileNewResult = compile_new(proc->procBlock(), walker, moduleEntry, type);
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
            auto compileConstantResult = compile_constant(proc->procBlock(), walker, moduleEntry);
            if (compileConstantResult.isStatus())
                return compileConstantResult.getStatus();
            bodyType = compileConstantResult.getResult();
            break;
        }
        default:
            block->throwSyntaxError(walker, "invalid initializer");
    }

    // add return instruction
    auto status = proc->procCode()->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), bodyType));
    if (!isReturnable)
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "initializer is incompatible with type {}", call->getReturnType().toString());

    // validate that each exit returns the expected type
    for (const auto &exitType : call->listExitTypes()) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), exitType));
        if (!isReturnable)
            return block->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "initializer is incompatible with type {}", call->getReturnType().toString());
    }

    return initializerUrl;
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
    auto *state = moduleEntry.getState();

    // declare the initializer
    auto declareInitializerResult = staticSymbol->declareInitializer();
    if (declareInitializerResult.isStatus())
        return declareInitializerResult.getStatus();
    auto initializerUrl = declareInitializerResult.getResult();
    auto *sym = state->symbolCache()->getSymbol(initializerUrl);
    if (sym == nullptr)
        state->throwAssemblerInvariant("missing call symbol {}", initializerUrl.toString());
    if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        state->throwAssemblerInvariant("invalid call symbol {}", initializerUrl.toString());
    auto *call = cast_symbol_to_call(sym);
    auto *proc = call->callProc();
    auto *code = proc->procCode();
    auto *block = proc->procBlock();

    // compile the initializer body
    auto exprResult = compile_expression(block, walker, moduleEntry);
    if (exprResult.isStatus())
        return exprResult.getStatus();
    auto bodyType = exprResult.getResult();

    // add return instruction
    auto status = code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    if (!status.isOk())
        return status;

    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), bodyType));
    if (!isReturnable)
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "initializer is incompatible with type {}", call->getReturnType().toString());

    // validate that each exit returns the expected type
    for (const auto &exitType : call->listExitTypes()) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(call->getReturnType(), exitType));
        if (!isReturnable)
            return block->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "initializer is incompatible with type {}", call->getReturnType().toString());
    }

    return CompilerStatus::ok();
}
