
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/local_variable.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/impl_utils.h>

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_compiler::resolve_impl_handle(
    const lyric_assembler::ImplReference &implRef,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::SymbolCache *symbolCache)
{
    const auto &usingRef = implRef.usingRef;

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(usingRef.symbolUrl));

    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::BINDING: {
            auto *bindingSymbol = cast_symbol_to_binding(symbol);
            lyric_common::TypeDef targetType;
            TU_ASSIGN_OR_RETURN (targetType, bindingSymbol->resolveTarget({}));
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(targetType.getConcreteUrl()));
            break;
        }
        case lyric_assembler::SymbolType::LOCAL: {
            auto *localVariable = cast_symbol_to_local(symbol);
            auto localType = localVariable->getTypeDef();
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(localType.getConcreteUrl()));
            break;
        }
        case lyric_assembler::SymbolType::LEXICAL: {
            auto *lexicalVariable = cast_symbol_to_lexical(symbol);
            auto lexicalType = lexicalVariable->getTypeDef();
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(lexicalType.getConcreteUrl()));
            break;
        }
        case lyric_assembler::SymbolType::ARGUMENT: {
            auto *argumentVariable = cast_symbol_to_local(symbol);
            auto argumentType = argumentVariable->getTypeDef();
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(argumentType.getConcreteUrl()));
            break;
        }
        default:
            break;
    }

    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(symbol);
            return classSymbol->getImpl(implRef.implType);
        }
        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = cast_symbol_to_concept(symbol);
            return conceptSymbol->getImpl(implRef.implType);
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(symbol);
            return enumSymbol->getImpl(implRef.implType);
        }
        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(symbol);
            return existentialSymbol->getImpl(implRef.implType);
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(symbol);
            return instanceSymbol->getImpl(implRef.implType);
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(symbol);
            return structSymbol->getImpl(implRef.implType);
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kMissingImpl,
                "missing impl for {}", implRef.implType.toString());
    }
}

tempo_utils::Status
lyric_compiler::prepare_impl_action(
    const std::string &actionName,
    const lyric_assembler::ImplReference &implRef,
    lyric_assembler::CallableInvoker &invoker,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::SymbolCache *symbolCache)
{
    lyric_assembler::ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, resolve_impl_handle(implRef, block, symbolCache));
    return implHandle->prepareExtension(actionName, implRef.usingRef, invoker);
}