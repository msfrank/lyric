
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_compiler/internal/compile_call.h>
#include <lyric_compiler/internal/compile_new.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/callsite_reifier.h>
#include <tempo_utils/log_stream.h>

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_new(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry,
    const lyric_common::TypeDef &typeHint)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    lyric_common::TypeDef newType;
    if (walker.hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        // resolve the new type from the type offset of the new node
        lyric_parser::NodeWalker typeNode;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
        lyric_typing::TypeSpec newSpec;
        TU_ASSIGN_OR_RETURN (newSpec, typeSystem->parseAssignable(block, typeNode));
        TU_ASSIGN_OR_RETURN (newType, typeSystem->resolveAssignable(block, newSpec));
    } else if (typeHint.isValid()) {
//        // resolve the new type from the specified type hint
//        auto resolveNewTypeResult = block->resolveAssignable(typeHint);
//        if (resolveNewTypeResult.isStatus())
//            return resolveNewTypeResult.getStatus();
//        newType = resolveNewTypeResult.getResult();
        newType = typeHint;
    } else {
        block->throwAssemblerInvariant("missing type offset");
    }

    if (newType.getType() != lyric_common::TypeDefType::Concrete)
        return moduleEntry.logAndContinue(walker.getLocation(),
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "new type {} is not constructable", newType.toString());

    //TU_RETURN_IF_NOT_OK (typeCache->touchType(newType));

    // resolve the symbol ctor
    auto newUrl = newType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, block->blockState()->symbolCache()->getOrImportSymbol(newUrl));

    // allocate the ctor invoker
    lyric_assembler::ConstructableInvoker ctorInvoker;
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(symbol);
            TU_RETURN_IF_NOT_OK (classSymbol->prepareCtor(ctorInvoker));
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(symbol);
            TU_RETURN_IF_NOT_OK (structSymbol->prepareCtor(ctorInvoker));
            break;
        }
        default:
            return block->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "cannot construct new instance of {}", newType.toString());
    }

    std::vector<lyric_common::TypeDef> newTypeArguments(
        newType.concreteArgumentsBegin(), newType.concreteArgumentsEnd());

    // construct the callsite reifier
    lyric_typing::CallsiteReifier ctorReifier(typeSystem);
    TU_RETURN_IF_NOT_OK (ctorReifier.initialize(ctorInvoker, newTypeArguments));

    // place the ctor arguments on the stack
    TU_RETURN_IF_NOT_OK (compile_placement(
        ctorInvoker.getConstructable(), block, block, ctorReifier, walker, moduleEntry));

    // invoke the ctor
    return ctorInvoker.invokeNew(block, ctorReifier, 0);
}
