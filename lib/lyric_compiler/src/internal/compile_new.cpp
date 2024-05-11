
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
    const lyric_parser::Assignable &typeHint)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    lyric_common::TypeDef newType;
    if (walker.hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        // resolve the new type from the type offset of the new node
        tu_uint32 typeOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
        auto assignedType = walker.getNodeAtOffset(typeOffset);
        auto resolveNewTypeResult = typeSystem->resolveAssignable(block, assignedType);
        if (resolveNewTypeResult.isStatus())
            return resolveNewTypeResult;
        newType = resolveNewTypeResult.getResult();
    } else if (typeHint.isValid()) {
        // resolve the new type from the specified type hint
        auto resolveNewTypeResult = block->resolveAssignable(typeHint);
        if (resolveNewTypeResult.isStatus())
            return resolveNewTypeResult.getStatus();
        newType = resolveNewTypeResult.getResult();
    } else {
        block->throwSyntaxError(walker, "missing type offset");
    }

    if (newType.getType() != lyric_common::TypeDefType::Concrete)
        block->throwSyntaxError(walker, "new type {} is not constructable", newType.toString());

    block->blockState()->typeCache()->touchType(newType);

    // resolve the symbol ctor
    auto newUrl = newType.getConcreteUrl();
    if (!block->blockState()->symbolCache()->hasSymbol(newUrl))
        block->throwAssemblerInvariant("missing new symbol {}", newUrl.toString());
    auto *sym = block->blockState()->symbolCache()->getSymbol(newUrl);

    sym->touch();

    // allocate the ctor invoker
    lyric_assembler::CtorInvoker ctor;
    switch (sym->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(sym);
            auto resolveCtorResult = classSymbol->resolveCtor();
            if (resolveCtorResult.isStatus())
                return resolveCtorResult.getStatus();
            ctor = resolveCtorResult.getResult();
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(sym);
            auto resolveCtorResult = structSymbol->resolveCtor();
            if (resolveCtorResult.isStatus())
                return resolveCtorResult.getStatus();
            ctor = resolveCtorResult.getResult();
            break;
        }
        default:
            return block->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "cannot construct new instance of {}", newType.toString());
    }

    // construct the callsite reifier
    std::vector<lyric_common::TypeDef> newTypeArguments(
        newType.concreteArgumentsBegin(), newType.concreteArgumentsEnd());
    lyric_typing::CallsiteReifier ctorReifier(ctor.getParameters(), ctor.getRest(),
        ctor.getTemplateUrl(), ctor.getTemplateParameters(), newTypeArguments,
        typeSystem);
    TU_RETURN_IF_NOT_OK (ctorReifier.initialize());

    // place the ctor arguments on the stack
    TU_RETURN_IF_NOT_OK (compile_placement(block, block, ctor, ctorReifier, walker, moduleEntry));

    // invoke the ctor
    return ctor.invokeNew(block, ctorReifier);
}
