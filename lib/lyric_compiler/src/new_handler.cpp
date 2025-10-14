
#include <lyric_assembler/binding_symbol.h>
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

    lyric_common::SymbolUrl newUrl;
    std::vector<lyric_common::TypeDef> newTypeArguments;

    if (node->hasAttr(lyric_parser::kLyricAstSymbolPath)) {
        lyric_common::SymbolPath newPath;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, newPath));
        TU_ASSIGN_OR_RETURN (newUrl, block->resolveDefinition(newPath));

        if (node->hasAttr(lyric_parser::kLyricAstTypeArgumentsOffset)) {
            lyric_parser::ArchetypeNode *typeArgumentsNode;
            TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeArgumentsOffset, typeArgumentsNode));
            std::vector<lyric_typing::TypeSpec> typeArgumentsSpec;
            TU_ASSIGN_OR_RETURN (typeArgumentsSpec, typeSystem->parseTypeArguments(
                block, typeArgumentsNode->getArchetypeNode()));
            TU_ASSIGN_OR_RETURN (newTypeArguments, typeSystem->resolveTypeArguments(block, typeArgumentsSpec));
        }
    } else if (m_typeHint.isValid()) {
        if (m_typeHint.getType() != lyric_common::TypeDefType::Concrete)
            return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                "new type {} is not constructable", m_typeHint.toString());
        newUrl = m_typeHint.getConcreteUrl();
        newTypeArguments = std::vector(
            m_typeHint.concreteArgumentsBegin(), m_typeHint.concreteArgumentsEnd());
    } else {
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected new type or type hint");
    }

    // resolve the new url
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(newUrl));

    std::string ctorName;
    lyric_assembler::AbstractSymbol *receiverSymbol;

    // if the new symbol refers to a binding then resolve the target
    if (symbol->getSymbolType() == lyric_assembler::SymbolType::BINDING) {
        auto *binding = lyric_assembler::cast_symbol_to_binding(symbol);
        lyric_common::TypeDef targetType;
        TU_ASSIGN_OR_RETURN (targetType, binding->resolveTarget(newTypeArguments));
        if (targetType.getType() != lyric_common::TypeDefType::Concrete)
            return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                "new type {} is not constructable", targetType.toString());
        auto targetUrl = targetType.getConcreteUrl();
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(targetUrl));
        newTypeArguments = std::vector(
            targetType.concreteArgumentsBegin(), targetType.concreteArgumentsEnd());
    }

    // if new symbol refers to a named constructor then determine the new type from the receiver
    if (symbol->getSymbolType() == lyric_assembler::SymbolType::CALL) {
        auto *ctorCall = lyric_assembler::cast_symbol_to_call(symbol);
        auto receiverUrl = ctorCall->getReceiverUrl();
        TU_ASSIGN_OR_RETURN (receiverSymbol, symbolCache->getOrImportSymbol(receiverUrl));
        ctorName = newUrl.getSymbolName();
    } else {
        receiverSymbol = symbol;
        ctorName = lyric_object::kCtorSpecialSymbol;
    }

    // prepare the ctor invoker
    switch (receiverSymbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(receiverSymbol);
            TU_RETURN_IF_NOT_OK (classSymbol->prepareCtor(ctorName, m_invoker));
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(receiverSymbol);
            TU_RETURN_IF_NOT_OK (structSymbol->prepareCtor(ctorName, m_invoker));
            break;
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                "cannot construct new instance of {}",
                receiverSymbol->getSymbolUrl().toString());
    }

    // construct the callsite reifier
    m_reifier = std::make_unique<lyric_typing::CallsiteReifier>(typeSystem);
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