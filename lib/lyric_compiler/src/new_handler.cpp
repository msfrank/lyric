
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/new_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::NewHandler::NewHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseInvokableHandler(block, block, fragment, driver),
      m_isSideEffect(isSideEffect)
{
}

lyric_compiler::NewHandler::NewHandler(
    const lyric_common::TypeDef &typeHint,
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : NewHandler(isSideEffect, fragment, block, driver)
{
    m_typeHint = typeHint;
    TU_ASSERT (m_typeHint.isValid());
}

tempo_utils::Status
lyric_compiler::NewHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getInvokeBlock();
    auto *driver = getDriver();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    lyric_common::TypeDef newType;

    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        // resolve the new type from the type offset of the new node
        lyric_parser::ArchetypeNode *typeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
        lyric_typing::TypeSpec newSpec;
        TU_ASSIGN_OR_RETURN (newSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (newType, typeSystem->resolveAssignable(block, newSpec));
    } else if (m_typeHint.isValid()) {
        newType = m_typeHint;
    } else {
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected new type or type hint");
    }

    if (newType.getType() != lyric_common::TypeDefType::Concrete)
        return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
            "new type {} is not constructable", newType.toString());

    // resolve the symbol ctor
    auto newUrl = newType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(newUrl));

    // prepare the ctor invoker
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(symbol);
            TU_RETURN_IF_NOT_OK (classSymbol->prepareCtor(m_invoker));
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(symbol);
            TU_RETURN_IF_NOT_OK (structSymbol->prepareCtor(lyric_object::kCtorSpecialSymbol, m_invoker));
            break;
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                "cannot construct new instance of {}", newType.toString());
    }

    // construct the callsite reifier
    m_reifier = std::make_unique<lyric_typing::CallsiteReifier>(typeSystem);
    std::vector<lyric_common::TypeDef> newTypeArguments(
        newType.concreteArgumentsBegin(), newType.concreteArgumentsEnd());
    TU_RETURN_IF_NOT_OK (m_reifier->initialize(m_invoker, newTypeArguments));

    return BaseInvokableHandler::before(state, node, ctx);
}

tempo_utils::Status
lyric_compiler::NewHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *fragment = getFragment();

    TU_RETURN_IF_NOT_OK (placeArguments(m_invoker.getConstructable(), *m_reifier, fragment));

    lyric_common::TypeDef newType;
    TU_ASSIGN_OR_RETURN (newType, m_invoker.invokeNew(getInvokeBlock(), *m_reifier, fragment, /* flags= */ 0));

    if (m_isSideEffect) {
        return m_fragment->popValue();
    }

    return driver->pushResult(newType);
}