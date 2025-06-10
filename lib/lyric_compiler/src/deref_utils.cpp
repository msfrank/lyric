
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/resolve_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/member_reifier.h>

/**
 * dereference 'this' keyword. this overload is intended to be used to compile a bare 'this'
 * expression or a data dereference expression with a single element.
 */
tempo_utils::Status
lyric_compiler::deref_this(
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (driver != nullptr);

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, block->resolveReference("$this"));

    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();

    // ensure symbol for $this is imported
    TU_RETURN_IF_STATUS(symbolCache->getOrImportSymbol(ref.symbolUrl));

    // load the receiver
    TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));

    return driver->pushResult(ref.typeDef);
}

/**
 * dereference 'this' keyword. this overload is intended to be used to compile a data dereference
 * expression with multiple elements. note that the 'this' dereference element is only valid as the
 * first element in the expression.
 */
tempo_utils::Status
lyric_compiler::deref_this(
    lyric_assembler::DataReference &ref,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (driver != nullptr);

    TU_ASSIGN_OR_RETURN (ref, block->resolveReference("$this"));

    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();

    // ensure symbol for $this is imported
    TU_RETURN_IF_STATUS(symbolCache->getOrImportSymbol(ref.symbolUrl));

    // load the receiver
    TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));

    return driver->pushResult(ref.typeDef);
}

inline tempo_utils::Result<lyric_assembler::DataReference>
resolve_binding(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    return block->resolveReference(identifier);
}

inline tempo_utils::Result<lyric_common::TypeDef>
deref_descriptor_if_allowed(
    lyric_common::SymbolUrl &symbolUrl,
    lyric_assembler::CodeFragment *fragment,
    lyric_compiler::CompilerScanDriver *driver)
{
    auto *symbolCache = driver->getSymbolCache();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(symbolUrl));
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::ENUM:
        case lyric_assembler::SymbolType::INSTANCE:
        case lyric_assembler::SymbolType::STATIC:
            TU_RETURN_IF_NOT_OK (fragment->loadData(symbol));
            return symbol->getTypeDef();
        default:
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kCompilerInvariant, "invalid deref target");
    }
}

tempo_utils::Status
lyric_compiler::deref_name(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (block != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (driver != nullptr);

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, resolve_binding(node, block));

    lyric_common::TypeDef resultType;
    switch (ref.referenceType) {
        case lyric_assembler::ReferenceType::Value:
        case lyric_assembler::ReferenceType::Variable:
            TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));
            resultType = ref.typeDef;
            break;

        case lyric_assembler::ReferenceType::Descriptor:
            TU_ASSIGN_OR_RETURN (resultType, deref_descriptor_if_allowed(ref.symbolUrl, fragment, driver));
            break;
        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid deref target");
    }
    return driver->pushResult(resultType);
}

inline tempo_utils::Status
update_namespace_block(lyric_assembler::AbstractSymbol *symbol, lyric_assembler::BlockHandle **blockptr)
{
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::NAMESPACE)
        return lyric_compiler::CompilerStatus::forCondition(
            lyric_compiler::CompilerCondition::kCompilerInvariant, "invalid deref namespace");
    auto *namespaceSymbol = cast_symbol_to_namespace(symbol);
    *blockptr = namespaceSymbol->namespaceBlock();
    return {};
}

tempo_utils::Status
lyric_compiler::deref_name(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::DataReference &ref,
    lyric_assembler::BlockHandle **blockptr,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (blockptr != nullptr && *blockptr != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (driver != nullptr);

    TU_ASSIGN_OR_RETURN (ref, resolve_binding(node, *blockptr));

    auto *state = (*blockptr)->blockState();
    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));

    lyric_common::TypeDef resultType;
    switch (ref.referenceType) {
        //
        case lyric_assembler::ReferenceType::Namespace:
            return update_namespace_block(symbol, blockptr);

        case lyric_assembler::ReferenceType::Descriptor: {
            TU_ASSIGN_OR_RETURN (resultType, deref_descriptor_if_allowed(ref.symbolUrl, fragment, driver));
            break;
        }

        case lyric_assembler::ReferenceType::Value:
        case lyric_assembler::ReferenceType::Variable:
            TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));
            resultType = ref.typeDef;
            break;

        default:
            break;
    }

    return driver->pushResult(resultType);
}

tempo_utils::Status
lyric_compiler::deref_namespace(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::DataReference &ref,
    lyric_assembler::BlockHandle **blockptr,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (ref.referenceType == lyric_assembler::ReferenceType::Namespace);
    TU_ASSERT (blockptr != nullptr && *blockptr != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (driver != nullptr);

    TU_ASSIGN_OR_RETURN (ref, resolve_binding(node, *blockptr));

    auto *state = (*blockptr)->blockState();
    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));

    switch (ref.referenceType) {
        case lyric_assembler::ReferenceType::Namespace: {
            TU_RETURN_IF_NOT_OK (update_namespace_block(symbol, blockptr));
            break;
        }

        case lyric_assembler::ReferenceType::Value:
        case lyric_assembler::ReferenceType::Variable:
            break;

        case lyric_assembler::ReferenceType::Descriptor:
            switch (symbol->getSymbolType()) {
                case lyric_assembler::SymbolType::ENUM:
                case lyric_assembler::SymbolType::INSTANCE:
                case lyric_assembler::SymbolType::STATIC:
                    break;
                default:
                    return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                        "invalid namespace binding");
            }
            break;
        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid namespace binding");
    }

    // pop the prior namespace descriptor off the stack
    TU_RETURN_IF_NOT_OK (fragment->popValue());

    // load the namespace binding onto the stack
    TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));

    return driver->pushResult(ref.typeDef);
}

tempo_utils::Status
lyric_compiler::deref_member(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::DataReference &ref,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (block != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (receiverType.isValid());
    TU_ASSERT (driver != nullptr);

    auto *state = driver->getState();
    auto *typeSystem = driver->getTypeSystem();
    auto *symbolCache = state->symbolCache();

    auto receiverUrl = receiverType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *receiver;
    TU_ASSIGN_OR_RETURN (receiver, symbolCache->getOrImportSymbol(receiverUrl));

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    lyric_typing::MemberReifier reifier(typeSystem);
    switch (receiver->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(receiver);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType, classSymbol->classTemplate()));
            TU_ASSIGN_OR_RETURN (ref, classSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(receiver);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
            TU_ASSIGN_OR_RETURN (ref, enumSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(receiver);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
            TU_ASSIGN_OR_RETURN (ref, instanceSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(receiver);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
            TU_ASSIGN_OR_RETURN (ref, structSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kInvalidSymbol,
                "invalid receiver symbol {}", receiverUrl.toString());
    }

    // load the reference onto the stack
    TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));

    // drop the previous result from the stack
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // push member result
    return driver->pushResult(ref.typeDef);
}

inline tempo_utils::Status
update_descriptor_block(lyric_assembler::AbstractSymbol *symbol, lyric_assembler::BlockHandle **blockptr)
{
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(symbol);
            *blockptr = classSymbol->classBlock();
            return {};
        }
        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = cast_symbol_to_concept(symbol);
            *blockptr = conceptSymbol->conceptBlock();
            return {};
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(symbol);
            *blockptr = enumSymbol->enumBlock();
            return {};
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(symbol);
            *blockptr = instanceSymbol->instanceBlock();
            return {};
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(symbol);
            *blockptr = structSymbol->structBlock();
            return {};
        }
        case lyric_assembler::SymbolType::NAMESPACE: {
            auto *namespaceSymbol = cast_symbol_to_namespace(symbol);
            *blockptr = namespaceSymbol->namespaceBlock();
            return {};
        }
        default:
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kCompilerInvariant, "invalid descriptor type");
    }
}

tempo_utils::Status
lyric_compiler::deref_descriptor(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle **blockptr,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (blockptr != nullptr && *blockptr != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (driver != nullptr);

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, resolve_binding(node, *blockptr));

    auto *state = (*blockptr)->blockState();
    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));

    switch (ref.referenceType) {
        case lyric_assembler::ReferenceType::Namespace: {
            TU_RETURN_IF_NOT_OK (update_namespace_block(symbol, blockptr));
            break;
        }
        case lyric_assembler::ReferenceType::Descriptor: {
            TU_RETURN_IF_NOT_OK (update_descriptor_block(symbol, blockptr));
            break;
        }

        default: {
            std::string identifier;
            TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
            return CompilerStatus::forCondition(CompilerCondition::kMissingSymbol,
                "missing symbol '{}'", identifier);
        }
    }

    // load the namespace binding onto the stack
    TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));

    return driver->pushResult(ref.typeDef);
}
