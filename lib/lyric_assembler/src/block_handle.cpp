
#include <absl/container/flat_hash_set.h>
#include <lyric_assembler/binding_symbol.h>

#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/function_callable.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/local_variable.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/linkage_symbol.h>
#include <tempo_utils/log_stream.h>

/**
 * Allocate a new BlockHandle for the prelude block.
 */
lyric_assembler::BlockHandle::BlockHandle(ObjectState *state)
    : m_blockNs(nullptr),
      m_blockProc(nullptr),
      m_parentBlock(nullptr),
      m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

/**
 * Allocate a new BlockHandle. This is used when constructing the root block and the
 * $global namespace block.
 */
lyric_assembler::BlockHandle::BlockHandle(
    BlockHandle *parentBlock,
    ObjectState *state)
    : m_blockNs(nullptr),
      m_blockProc(nullptr),
      m_parentBlock(parentBlock),
      m_state(state)
{
    TU_ASSERT (m_state != nullptr);
    TU_ASSERT (m_parentBlock != nullptr);
}

lyric_assembler::BlockHandle::BlockHandle(
    const lyric_common::SymbolUrl &definition,
    BlockHandle *parentBlock)
    : m_definition(definition),
      m_blockNs(nullptr),
      m_blockProc(nullptr),
      m_parentBlock(parentBlock)
{
    TU_ASSERT (m_definition.isValid());
    TU_ASSERT (m_parentBlock != nullptr);
    //m_blockProc = parentBlock->m_blockProc;
    //TU_ASSERT (m_blockProc != nullptr);
    m_state = parentBlock->m_state;
    TU_ASSERT (m_state != nullptr);
}

//lyric_assembler::BlockHandle::BlockHandle(
//    NamespaceSymbol *blockNs,
//    BlockHandle *parentBlock,
//    ObjectState *state,
//    bool isRoot)
//    : m_blockNs(blockNs),
//      m_blockProc(nullptr),
//      m_parentBlock(parentBlock),
//      m_state(state),
//      m_isRoot(isRoot)
//{
//    TU_ASSERT (m_blockNs != nullptr);
//    TU_ASSERT (m_parentBlock != nullptr);
//    TU_ASSERT (m_state != nullptr);
//    m_definition = m_blockNs->getSymbolUrl();
//}

lyric_assembler::BlockHandle::BlockHandle(
    ProcHandle *blockProc,
    BlockHandle *parentBlock,
    ObjectState *state)
    : m_blockNs(nullptr),
      m_blockProc(blockProc),
      m_parentBlock(parentBlock),
      m_state(state)
{
    TU_ASSERT (m_blockProc != nullptr);
    TU_ASSERT (m_parentBlock != nullptr);
    TU_ASSERT (m_state != nullptr);
    m_definition = m_blockProc->getActivation();
}

lyric_assembler::BlockHandle::BlockHandle(
    const absl::flat_hash_map<std::string,SymbolBinding> &initialBindings,
    ProcHandle *blockProc,
    BlockHandle *parentBlock,
    ObjectState *state)
    : m_blockNs(nullptr),
      m_blockProc(blockProc),
      m_parentBlock(parentBlock),
      m_state(state),
      m_bindings(initialBindings)
{
    TU_ASSERT (m_blockProc != nullptr);
    TU_ASSERT (m_parentBlock != nullptr);
    TU_ASSERT (m_state != nullptr);
    m_definition = m_blockProc->getActivation();
}

lyric_assembler::BlockHandle::BlockHandle(
    const lyric_common::SymbolUrl &definition,
    const absl::flat_hash_map<std::string,SymbolBinding> &importedBindings,
    ObjectState *state)
    : m_definition(definition),
      m_blockNs(nullptr),
      m_blockProc(nullptr),
      m_parentBlock(nullptr),
      m_state(state),
      m_bindings(importedBindings)
{
    TU_ASSERT (m_definition.isValid());
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::NamespaceSymbol *
lyric_assembler::BlockHandle::blockNs()
{
    if (m_blockNs)
        return m_blockNs;
    if (m_parentBlock)
        return m_parentBlock->blockNs();
    return nullptr;
}

lyric_assembler::ProcHandle *
lyric_assembler::BlockHandle::blockProc()
{
    return m_blockProc;
}

lyric_assembler::BlockHandle *
lyric_assembler::BlockHandle::blockParent()
{
    return m_parentBlock;
}

lyric_assembler::ObjectState *
lyric_assembler::BlockHandle::blockState()
{
    return m_state;
}

bool
lyric_assembler::BlockHandle::isImported() const
{
    // no parent block and a valid definition url indicates an imported block
    return m_parentBlock == nullptr && m_definition.isValid();
}

lyric_common::SymbolUrl
lyric_assembler::BlockHandle::getDefinition() const
{
    if (m_definition.isValid())
        return m_definition;
    if (m_parentBlock == nullptr)
        return {};
    return m_parentBlock->getDefinition();
}

absl::flat_hash_map<std::string,lyric_assembler::SymbolBinding>::const_iterator
lyric_assembler::BlockHandle::symbolsBegin() const
{
    return m_bindings.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::SymbolBinding>::const_iterator
lyric_assembler::BlockHandle::symbolsEnd() const
{
    return m_bindings.cend();
}

absl::flat_hash_map<lyric_common::TypeDef, lyric_assembler::ImplReference>::const_iterator
lyric_assembler::BlockHandle::implsBegin() const
{
    return m_impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef, lyric_assembler::ImplReference>::const_iterator
lyric_assembler::BlockHandle::implsEnd() const
{
    return m_impls.cend();
}

/**
 * Returns whether the current block contains a binding with the specified name.
 *
 * @param name The binding name.
 * @return true if there is a binding with the specified name in the current block, otherwise false.
 */
bool
lyric_assembler::BlockHandle::hasBinding(const std::string &name) const
{
    return m_bindings.contains(name);
}

/**
 * Returns the binding with the specified name in the current block. If the binding is not present then the
 * returned symbol binding is invalid.
 *
 * @param name The binding name.
 * @return The symbol binding.
 */
lyric_assembler::SymbolBinding
lyric_assembler::BlockHandle::getBinding(const std::string &name) const
{
    if (m_bindings.contains(name))
        return m_bindings.at(name);
    return {};
}

// forward declaration
static tempo_utils::Result<lyric_assembler::SymbolBinding>
resolve_binding_tail_recursive(
    lyric_assembler::BlockHandle *resolveBlock,
    lyric_assembler::NamespaceSymbol *targetNs,
    const std::vector<std::string> &path,
    std::vector<std::string>::const_iterator curr,
    std::vector<std::string>::const_iterator end);

static tempo_utils::Result<lyric_assembler::SymbolBinding>
resolve_binding_tail_recursive(
    lyric_assembler::BlockHandle *resolveBlock,
    lyric_assembler::BlockHandle *targetBlock,
    const std::vector<std::string> &path,
    std::vector<std::string>::const_iterator curr,
    std::vector<std::string>::const_iterator end)   // NOLINT(misc-no-recursion)
{
    if (curr == end || !targetBlock->hasBinding(*curr))
        return resolveBlock->logAndContinue(
            lyric_assembler::AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", lyric_common::SymbolPath(path).toString());

    auto binding = targetBlock->getBinding(*curr);

    // if we have reached the last segment then return the binding
    if (++curr == end)
        return binding;

    // if there are subsequent segments then the binding must be a descriptor
    if (binding.bindingType != lyric_assembler::BindingType::Descriptor)
        return resolveBlock->logAndContinue(
            lyric_assembler::AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", lyric_common::SymbolPath(path).toString());

    auto *state = resolveBlock->blockState();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(binding.symbolUrl));

    // if there are subsequent segments then the symbol must be a namespace
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::NAMESPACE)
        return resolveBlock->logAndContinue(
            lyric_assembler::AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", lyric_common::SymbolPath(path).toString());

    auto *childNs = cast_symbol_to_namespace(symbol);

    return resolve_binding_tail_recursive(resolveBlock, childNs, path, curr, end);
}

static tempo_utils::Result<lyric_assembler::SymbolBinding>
resolve_binding_tail_recursive(
    lyric_assembler::BlockHandle *resolveBlock,
    lyric_assembler::NamespaceSymbol *targetNs,
    const std::vector<std::string> &path,
    std::vector<std::string>::const_iterator curr,
    std::vector<std::string>::const_iterator end)   // NOLINT(misc-no-recursion)
{
    if (curr == end || !targetNs->hasSymbol(*curr))
        return resolveBlock->logAndContinue(
            lyric_assembler::AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", lyric_common::SymbolPath(path).toString());

    auto symbolUrl = targetNs->getSymbol(*curr);

    auto *state = resolveBlock->blockState();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(symbolUrl));

    // if we have reached the last segment then return the binding
    if (++curr == end) {
        lyric_assembler::SymbolBinding symbolBinding;
        symbolBinding.symbolUrl = symbolUrl;
        symbolBinding.bindingType = lyric_assembler::BindingType::Descriptor;
        switch (symbol->getSymbolType()) {
            case lyric_assembler::SymbolType::CALL:
            case lyric_assembler::SymbolType::CLASS:
            case lyric_assembler::SymbolType::CONCEPT:
            case lyric_assembler::SymbolType::ENUM:
            case lyric_assembler::SymbolType::EXISTENTIAL:
            case lyric_assembler::SymbolType::INSTANCE:
            case lyric_assembler::SymbolType::STATIC:
            case lyric_assembler::SymbolType::STRUCT:
                symbolBinding.typeDef = symbol->getTypeDef();
                return symbolBinding;
            default:
                return resolveBlock->logAndContinue(
                    lyric_assembler::AssemblerCondition::kMissingSymbol,
                    tempo_tracing::LogSeverity::kError,
                    "missing symbol {}", lyric_common::SymbolPath(path).toString());
        }
    }

    // if there are subsequent segments then the symbol must be a namespace
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::NAMESPACE)
        return resolveBlock->logAndContinue(
            lyric_assembler::AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", lyric_common::SymbolPath(path).toString());

    auto *childNs = cast_symbol_to_namespace(symbol);

    return resolve_binding_tail_recursive(resolveBlock, childNs, path, curr, end);
}

/**
 *
 * @param path
 * @return
 */
tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::resolveBinding(const std::vector<std::string> &path)
{
    if (path.empty())
        throwAssemblerInvariant("invalid binding path");

    auto curr = path.cbegin();

    // walk the chain of blocks to the root, checking for type
    for (auto *targetBlock = this; targetBlock != nullptr; targetBlock = targetBlock->blockParent()) {
        if (!targetBlock->hasBinding(*curr))
            continue;
        return resolve_binding_tail_recursive(this, targetBlock, path, curr, path.cend());
    }

    // if variable is not found in any reachable block, then check the global namespace
    auto *root = m_state->objectRoot();
    auto *globalNamespace = root->globalNamespace();
    auto *globalBlock = globalNamespace->namespaceBlock();
    if (globalBlock->hasBinding(*curr)) {
        return resolve_binding_tail_recursive(this, globalBlock, path, curr, path.cend());
    }

    // we have exhausted our search, binding is missing
    return logAndContinue(lyric_assembler::AssemblerCondition::kMissingSymbol,
        tempo_tracing::LogSeverity::kError,
        "missing symbol {}", lyric_common::SymbolPath(path).toString());
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::BlockHandle::resolveSingular(
    const lyric_common::SymbolPath &typePath,
    const std::vector<lyric_common::TypeDef> &typeArguments) // NOLINT(misc-no-recursion)
{
    if (!typePath.isValid())
        m_state->throwAssemblerInvariant("invalid type path {}", typePath.toString());

    // resolve the type path to a binding
    SymbolBinding binding;
    TU_ASSIGN_OR_RETURN (binding, resolveBinding(typePath.getPath()));

    // we expect the binding is either a descriptor or placeholder
    lyric_common::TypeDef typeDef;
    switch (binding.bindingType) {
        case BindingType::Descriptor:
            typeDef = lyric_common::TypeDef::forConcrete(binding.symbolUrl, typeArguments);
            break;
        case BindingType::Placeholder:
            typeDef = binding.typeDef;
            break;
        default:
            return logAndContinue(
                AssemblerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "{} does not refer to a valid type", typePath.toString());
    }

    // if the type is concrete and the symbol is a binding then resolve the binding target
    if (typeDef.getType() == lyric_common::TypeDefType::Concrete) {
        auto *symbolCache = m_state->symbolCache();
        AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, symbolCache->getOrImportSymbol(typeDef.getConcreteUrl()));
        if (sym->getSymbolType() == SymbolType::BINDING) {
            auto *bindingSymbol = cast_symbol_to_binding(sym);
            TU_ASSIGN_OR_RETURN (typeDef, bindingSymbol->resolveTarget(typeArguments));
        }
    }

    auto *typeCache = m_state->typeCache();

    // add the type to the typecache if it doesn't already exist
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(typeDef));

    return typeDef;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::resolveDefinition(const lyric_common::SymbolPath &symbolPath)
{
    std::vector<std::string> path;
    for (const auto &part : symbolPath.getPath()) {
        path.push_back(part);
    }

    SymbolBinding binding;
    TU_ASSIGN_OR_RETURN (binding, resolveBinding(path));

    auto *symbolCache = m_state->symbolCache();
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(binding.symbolUrl));

    switch (symbol->getSymbolType()) {
        case SymbolType::BINDING:
        case SymbolType::CALL:
        case SymbolType::CLASS:
        case SymbolType::CONCEPT:
        case SymbolType::ENUM:
        case SymbolType::EXISTENTIAL:
        case SymbolType::INSTANCE:
        case SymbolType::STRUCT:
            return symbol->getSymbolUrl();
        default:
            return logAndContinue(
                AssemblerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "missing definition {}", lyric_common::SymbolPath(path).toString());
    }
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::resolveFunction(const std::string &name)
{
    SymbolBinding binding;
    TU_ASSIGN_OR_RETURN (binding, resolveBinding({name}));

    auto *symbolCache = m_state->symbolCache();
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(binding.symbolUrl));

    if (symbol->getSymbolType() == SymbolType::BINDING) {
        auto *bindingSymbol = cast_symbol_to_binding(symbol);
        lyric_common::TypeDef targetType;
        TU_ASSIGN_OR_RETURN (targetType, bindingSymbol->resolveTarget({}));
        if (targetType.getType() != lyric_common::TypeDefType::Concrete)
            return logAndContinue(AssemblerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "missing function {}", lyric_common::SymbolPath({name}).toString());
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(targetType.getConcreteUrl()));
    }

    switch (symbol->getSymbolType()) {
        case SymbolType::CALL:
            return symbol->getSymbolUrl();
        default:
            return logAndContinue(AssemblerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "missing function {}", lyric_common::SymbolPath({name}).toString());
    }
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::BlockHandle::declareVariable(
    const std::string &name,
    lyric_object::AccessType access,
    const lyric_common::TypeDef &assignableType,
    bool isVariable)
{
    auto *symbolCache = m_state->symbolCache();
    auto *typeCache = m_state->typeCache();

    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare variable {}; symbol is already defined", name);

    BindingType bindingType = isVariable? BindingType::Variable : BindingType::Value;
    ReferenceType referenceType = isVariable? ReferenceType::Variable : ReferenceType::Value;

    // make type handle if it doesn't exist already (for example union or intersection types)
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(assignableType));

    // if this is a root block then create a static variable, otherwise create a local
    auto localUrl = makeSymbolUrl(name);
    auto address = m_blockProc->allocateLocal();
    auto localVariable = std::make_unique<LocalVariable>(
        localUrl, access, assignableType, address);
    TU_RETURN_IF_NOT_OK (symbolCache->insertSymbol(localUrl, localVariable.get()));
    localVariable.release();

    SymbolBinding binding;
    binding.bindingType = bindingType;
    binding.symbolUrl = localUrl;
    binding.typeDef = assignableType;
    m_bindings[name] = binding;

    return DataReference(referenceType, localUrl, assignableType);
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::BlockHandle::declareTemporary(
    const lyric_common::TypeDef &assignableType,
    bool isVariable)
{
    auto *symbolCache = m_state->symbolCache();
    auto *typeCache = m_state->typeCache();

    auto name = absl::StrCat("$tmp", m_blockProc->numLocals());

    if (m_bindings.contains(name))
        throwAssemblerInvariant("failed to declare temporary {}; symbol is is already defined", name);

    BindingType bindingType = isVariable? BindingType::Variable : BindingType::Value;
    ReferenceType referenceType = isVariable? ReferenceType::Variable : ReferenceType::Value;

    // make type handle if it doesn't exist already (for example union or intersection types)
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(assignableType));

    // temporary variables are always local
    auto localUrl = makeSymbolUrl(name);
    auto address = m_blockProc->allocateLocal();
    auto localVariable = std::make_unique<LocalVariable>(
        localUrl, lyric_object::AccessType::Private, assignableType, address);
    TU_RETURN_IF_NOT_OK (symbolCache->insertSymbol(localUrl, localVariable.get()));
    localVariable.release();

    SymbolBinding binding;
    binding.bindingType = bindingType;
    binding.symbolUrl = localUrl;
    binding.typeDef = assignableType;
    m_bindings[name] = binding;

    return DataReference(referenceType, localUrl, assignableType);
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::BlockHandle::declareStatic(
    const std::string &name,
    lyric_object::AccessType access,
    const lyric_common::TypeDef &assignableType,
    bool isVariable,
    bool declOnly)
{
     auto *typeCache = m_state->typeCache();

    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare static {}; symbol is already defined", name);

    BindingType bindingType = BindingType::Descriptor;
    ReferenceType referenceType = isVariable? ReferenceType::Variable : ReferenceType::Value;

    // make type handle if it doesn't exist already (for example union or intersection types)
    lyric_assembler::TypeHandle *staticType;
    TU_ASSIGN_OR_RETURN (staticType, typeCache->getOrMakeType(assignableType));

    auto staticUrl = makeSymbolUrl(name);

    auto staticSymbol = std::make_unique<StaticSymbol>(
        staticUrl, access, isVariable, staticType, declOnly, this, m_state);
    TU_RETURN_IF_NOT_OK (m_state->appendStatic(staticSymbol.get()));
    staticSymbol.release();

    SymbolBinding binding;
    binding.bindingType = bindingType;
    binding.symbolUrl = staticUrl;
    binding.typeDef = assignableType;
    m_bindings[name] = binding;

    return DataReference(referenceType, staticUrl, assignableType);
}

inline lyric_assembler::DataReference
symbol_binding_to_data_reference(const lyric_assembler::SymbolBinding &binding)
{
    switch (binding.bindingType) {

        case lyric_assembler::BindingType::Value:
            return lyric_assembler::DataReference(
                lyric_assembler::ReferenceType::Value, binding.symbolUrl, binding.typeDef);

        case lyric_assembler::BindingType::Variable:
            return lyric_assembler::DataReference(
                lyric_assembler::ReferenceType::Variable, binding.symbolUrl, binding.typeDef);

        case lyric_assembler::BindingType::Descriptor: {
            return lyric_assembler::DataReference(
                lyric_assembler::ReferenceType::Descriptor, binding.symbolUrl, binding.typeDef);
        }

        case lyric_assembler::BindingType::Namespace: {
            return lyric_assembler::DataReference(
                lyric_assembler::ReferenceType::Namespace, binding.symbolUrl, binding.typeDef);
        }

        default:
            return {};
    }
}

static tempo_utils::Result<lyric_assembler::DataReference>
resolve_binding_target(
    lyric_assembler::BindingSymbol *bindingSymbol,
    lyric_assembler::ObjectState *state)
{
    lyric_common::TypeDef targetType;

    TU_ASSIGN_OR_RETURN (targetType, bindingSymbol->resolveTarget({}));
    if (targetType.getType() != lyric_common::TypeDefType::Concrete)
        return state->logAndContinue(lyric_assembler::AssemblerCondition::kAssemblerInvariant,
            tempo_tracing::LogSeverity::kError,
            "invalid target type {}", targetType.toString());
    auto concreteUrl = targetType.getConcreteUrl();

    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(concreteUrl));
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CALL:
        case lyric_assembler::SymbolType::CLASS:
        case lyric_assembler::SymbolType::CONCEPT:
        case lyric_assembler::SymbolType::ENUM:
        case lyric_assembler::SymbolType::EXISTENTIAL:
        case lyric_assembler::SymbolType::INSTANCE:
        case lyric_assembler::SymbolType::STATIC:
        case lyric_assembler::SymbolType::STRUCT:
            return lyric_assembler::DataReference(
                lyric_assembler::ReferenceType::Descriptor, concreteUrl, symbol->getTypeDef());
        default:
            return state->logAndContinue(lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                tempo_tracing::LogSeverity::kError,
                "invalid binding target {}", concreteUrl.toString());
    }
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::BlockHandle::resolveReference(const std::string &name)
{
    auto *symbolCache = m_state->symbolCache();
    BlockHandle *block = this;

    // if variable exists in a parent block in the current proc, then return it
    for (; block != nullptr && block->m_blockProc == m_blockProc; block = block->m_parentBlock) {
        if (block->m_bindings.contains(name)) {
            const auto &binding = block->m_bindings[name];
            AbstractSymbol *symbol;
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(binding.symbolUrl));

            switch (symbol->getSymbolType()) {
                case SymbolType::ARGUMENT:
                case SymbolType::CALL:
                case SymbolType::CLASS:
                case SymbolType::CONCEPT:
                case SymbolType::CONSTANT:
                case SymbolType::ENUM:
                case SymbolType::EXISTENTIAL:
                case SymbolType::INSTANCE:
                case SymbolType::LEXICAL:
                case SymbolType::LOCAL:
                case SymbolType::NAMESPACE:
                case SymbolType::STATIC:
                case SymbolType::STRUCT:
                case SymbolType::SYNTHETIC:
                    return symbol_binding_to_data_reference(binding);

                case SymbolType::BINDING:
                    return resolve_binding_target(cast_symbol_to_binding(symbol), m_state);

                default:
                    return logAndContinue(AssemblerCondition::kMissingVariable,
                        tempo_tracing::LogSeverity::kError,
                        "missing variable {}", name);
            }
        }
    }

    // if variable exists in a parent proc, then import lexical into this proc
    for (; block != nullptr; block = block->m_parentBlock) {
        if (block->m_bindings.contains(name)) {
            const auto &binding = block->m_bindings[name];
            AbstractSymbol *symbol;
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(binding.symbolUrl));

            LexicalTarget lexicalTarget;
            uint32_t targetOffset = lyric_runtime::INVALID_ADDRESS_U32;
            lyric_common::TypeDef typeDef;

            switch (symbol->getSymbolType()) {
                // found a local in a parent proc
                case SymbolType::LOCAL: {
                    auto *local = static_cast<LocalVariable *>(symbol);
                    if (local->getAccessType() == lyric_object::AccessType::Private)
                        return logAndContinue(AssemblerCondition::kInvalidBinding,
                            tempo_tracing::LogSeverity::kError,
                            "variable {} is not visible from the current scope", name);
                    lexicalTarget = LexicalTarget::Local;
                    targetOffset = local->getOffset().getOffset();
                    typeDef = binding.typeDef;
                    break;
                }

                // found a static variable or a symbol
                case SymbolType::CALL:
                case SymbolType::CLASS:
                case SymbolType::CONCEPT:
                case SymbolType::ENUM:
                case SymbolType::EXISTENTIAL:
                case SymbolType::INSTANCE:
                case SymbolType::NAMESPACE:
                case SymbolType::STATIC:
                case SymbolType::STRUCT:
                    return symbol_binding_to_data_reference(binding);

                case SymbolType::BINDING:
                    return resolve_binding_target(cast_symbol_to_binding(symbol), m_state);

                // other symbol types are not valid
                case SymbolType::ARGUMENT:
                case SymbolType::LEXICAL:
                default:
                    return logAndContinue(AssemblerCondition::kInvalidBinding,
                        tempo_tracing::LogSeverity::kError,
                        "variable {} is not visible from the current scope", name);
            }

            auto activation = block->blockProc()->getActivation();
            TU_ASSERT (m_state->symbolCache()->hasSymbol(activation));
            lyric_assembler::AbstractSymbol *activationSymbol;
            TU_ASSIGN_OR_RETURN (activationSymbol, symbolCache->getOrImportSymbol(activation));
            auto *activationCall = cast_symbol_to_call(activationSymbol);

            // import lexical variable into this env, and return it
            auto lexicalUrl = makeSymbolUrl(name);
            auto address = m_blockProc->allocateLexical(lexicalTarget, targetOffset, activationCall);
            auto *lexicalVariable = new LexicalVariable(lexicalUrl, typeDef, address);
            symbolCache->insertSymbol(lexicalUrl, lexicalVariable);

            SymbolBinding lexical;
            lexical.symbolUrl = lexicalUrl;
            lexical.typeDef = typeDef;
            lexical.bindingType = BindingType::Value;
            m_bindings[name] = lexical;

            return symbol_binding_to_data_reference(lexical);
        }
    }

    // if variable is not found in any reachable block, then check the global namespace
    auto *root = m_state->objectRoot();
    auto *globalNamespace = root->globalNamespace();
    auto *globalBlock = globalNamespace->namespaceBlock();
    if (globalBlock->hasBinding(name)) {
        const auto &binding = globalBlock->m_bindings[name];
        AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(binding.symbolUrl));

        switch (symbol->getSymbolType()) {
            case SymbolType::CALL:
            case SymbolType::CLASS:
            case SymbolType::CONCEPT:
            case SymbolType::ENUM:
            case SymbolType::EXISTENTIAL:
            case SymbolType::INSTANCE:
            case SymbolType::NAMESPACE:
            case SymbolType::STATIC:
            case SymbolType::STRUCT:
                return symbol_binding_to_data_reference(binding);

            case SymbolType::BINDING:
                return resolve_binding_target(cast_symbol_to_binding(symbol), m_state);

            default:
                return logAndContinue(AssemblerCondition::kMissingVariable,
                    tempo_tracing::LogSeverity::kError,
                    "missing variable {}", name);
        }
    }

    // we have exhausted our search, variable is missing
    debugBindings();
    return logAndContinue(AssemblerCondition::kMissingVariable,
        tempo_tracing::LogSeverity::kError,
        "missing variable {}", name);
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::BlockHandle::declareFunction(
    const std::string &name,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    bool declOnly)
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *typeCache = m_state->typeCache();

    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare function {}; symbol is already defined", name);

    auto functionUrl = makeSymbolUrl(name);

    // create the template if there are any template parameters
    TemplateHandle *functionTemplate = nullptr;
    if (!templateParameters.empty()) {
        TU_ASSIGN_OR_RETURN (functionTemplate, typeCache->makeTemplate(functionUrl, templateParameters, this));
    }

    // create the call
    std::unique_ptr<CallSymbol> callSymbol;
    if (functionTemplate) {
        callSymbol = std::make_unique<CallSymbol>(functionUrl, access, lyric_object::CallMode::Normal,
            functionTemplate, declOnly, this, m_state);
    } else {
        callSymbol = std::make_unique<CallSymbol>(functionUrl, access, lyric_object::CallMode::Normal,
            declOnly, this, m_state);
    }

    TU_RETURN_IF_NOT_OK (m_state->appendCall(callSymbol.get()));
    auto *callPtr = callSymbol.release();

    SymbolBinding binding;
    binding.symbolUrl = functionUrl;
    binding.bindingType = BindingType::Descriptor;
    binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Descriptor);
    m_bindings[name] = binding;

    return callPtr;
}

tempo_utils::Status
lyric_assembler::BlockHandle::prepareFunction(const std::string &name, CallableInvoker &invoker)
{
    lyric_common::SymbolUrl functionUrl;
    TU_ASSIGN_OR_RETURN (functionUrl, resolveFunction(name));

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(functionUrl));
    if (symbol->getSymbolType() != SymbolType::CALL)
        throwAssemblerInvariant("invalid call symbol {}", functionUrl.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    auto callable = std::make_unique<FunctionCallable>(callSymbol, /* isInlined= */ false);
    return invoker.initialize(std::move(callable));
}

tempo_utils::Result<lyric_assembler::ClassSymbol *>
lyric_assembler::BlockHandle::declareClass(
    const std::string &name,
    ClassSymbol *superClass,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    lyric_object::DeriveType derive,
    bool isAbstract,
    bool declOnly)
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *typeCache = m_state->typeCache();

    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare class {}; symbol is already defined", name);

    auto superDerive = superClass->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive class {} from {}; base class is marked final",
            name, superClass->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && superClass->isImported())
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive class {} from {}; sealed base class must be located in the same module",
            name, superClass->getSymbolUrl().toString());

    auto classUrl = makeSymbolUrl(name);

    // create the template if there are any template parameters
    TemplateHandle *classTemplate = nullptr;
    if (!templateParameters.empty()) {
        TU_ASSIGN_OR_RETURN (classTemplate, typeCache->makeTemplate(classUrl, templateParameters, this));
    }

   // create the type
    TypeHandle *typeHandle;
    if (classTemplate) {
        TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(
            classUrl, classTemplate->getPlaceholders(), superClass->getTypeDef()));
    } else {
        TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(
            classUrl, {}, superClass->getTypeDef()));
    }

    // create the class
    std::unique_ptr<ClassSymbol> classSymbol;
    if (classTemplate) {
        classSymbol = std::make_unique<ClassSymbol>(classUrl, access, derive, isAbstract, typeHandle,
            classTemplate, superClass, declOnly, this, m_state);
    } else {
        classSymbol = std::make_unique<ClassSymbol>(classUrl, access, derive, isAbstract,
            typeHandle, superClass, declOnly, this, m_state);
    }

    TU_RETURN_IF_NOT_OK (m_state->appendClass(classSymbol.get()));
    auto *classPtr = classSymbol.release();

    SymbolBinding binding;
    binding.symbolUrl = classUrl;
    binding.bindingType = BindingType::Descriptor;
    binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Descriptor);
    m_bindings[name] = binding;

    return classPtr;
}

tempo_utils::Result<lyric_assembler::ClassSymbol *>
lyric_assembler::BlockHandle::resolveClass(const lyric_common::TypeDef &classType)
{
    if (classType.getType() != lyric_common::TypeDefType::Concrete)
        return logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "invalid class type {}", classType.toString());
    auto classUrl = classType.getConcreteUrl();

    lyric_common::SymbolUrl resolvedUrl;
    TU_ASSIGN_OR_RETURN (resolvedUrl, resolveDefinition(classUrl.getSymbolPath()));

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(resolvedUrl));
    if (symbol->getSymbolType() != SymbolType::CLASS)
        return logAndContinue(AssemblerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "type {} is not a class", classType.toString());

    return cast_symbol_to_class(symbol);
}

tempo_utils::Result<lyric_assembler::ConceptSymbol *>
lyric_assembler::BlockHandle::declareConcept(
    const std::string &name,
    ConceptSymbol *superConcept,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    lyric_object::DeriveType derive,
    bool declOnly)
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *typeCache = m_state->typeCache();

    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare concept {}; symbol is already defined", name);

    auto conceptUrl = makeSymbolUrl(name);

    // create the template if there are any template parameters
    TemplateHandle *conceptTemplate = nullptr;
    if (!templateParameters.empty()) {
        TU_ASSIGN_OR_RETURN (conceptTemplate, typeCache->makeTemplate(conceptUrl, templateParameters, this));
    }

    // create the type
    TypeHandle *typeHandle;
    if (conceptTemplate) {
        TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(
            conceptUrl, conceptTemplate->getPlaceholders(), superConcept->getTypeDef()));
    } else {
        TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(
            conceptUrl, {}, superConcept->getTypeDef()));
    }

    // create the concept
    std::unique_ptr<ConceptSymbol> conceptSymbol;
    if (conceptTemplate) {
        conceptSymbol = std::make_unique<ConceptSymbol>(conceptUrl, access, derive, typeHandle,
            conceptTemplate, superConcept, declOnly, this, m_state);
    } else {
        conceptSymbol = std::make_unique<ConceptSymbol>(conceptUrl, access, derive, typeHandle,
            superConcept, declOnly, this, m_state);
    }

    TU_RETURN_IF_NOT_OK (m_state->appendConcept(conceptSymbol.get()));
    auto *conceptPtr = conceptSymbol.release();

    SymbolBinding binding;
    binding.bindingType = BindingType::Descriptor;
    binding.symbolUrl = conceptUrl;
    binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Descriptor);
    m_bindings[name] = binding;

    return conceptPtr;
}

tempo_utils::Result<lyric_assembler::ConceptSymbol *>
lyric_assembler::BlockHandle::resolveConcept(const lyric_common::TypeDef &conceptType)
{
    if (conceptType.getType() != lyric_common::TypeDefType::Concrete)
        return logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "invalid concept type {}", conceptType.toString());
    auto conceptUrl = conceptType.getConcreteUrl();

    lyric_common::SymbolUrl resolvedUrl;
    TU_ASSIGN_OR_RETURN (resolvedUrl, resolveDefinition(conceptUrl.getSymbolPath()));

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(resolvedUrl));
    if (symbol->getSymbolType() != SymbolType::CONCEPT)
        return logAndContinue(AssemblerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "type {} is not a concept", conceptType.toString());

    return cast_symbol_to_concept(symbol);
}

tempo_utils::Result<lyric_assembler::EnumSymbol *>
lyric_assembler::BlockHandle::declareEnum(
    const std::string &name,
    EnumSymbol *superEnum,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    bool declOnly)
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *typeCache = m_state->typeCache();

    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare enum {}; symbol is already defined", name);

    auto superDerive = superEnum->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive enum {} from {}; base enum is marked final",
            name, superEnum->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && superEnum->isImported())
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive enum {} from {}; sealed base enum must be located in the same module",
            name, superEnum->getSymbolUrl().toString());

    auto enumUrl = makeSymbolUrl(name);

    // create the type
    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(
        enumUrl, {}, superEnum->getTypeDef()));

    // create the instance
    auto enumSymbol = std::make_unique<EnumSymbol>(enumUrl, access, derive,
        isAbstract, typeHandle, superEnum, declOnly, this, m_state);

    TU_RETURN_IF_NOT_OK (m_state->appendEnum(enumSymbol.get()));
    auto *enumPtr = enumSymbol.release();

    SymbolBinding binding;
    binding.bindingType = BindingType::Descriptor;
    binding.symbolUrl = enumUrl;
    binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Descriptor);
    m_bindings[name] = binding;

    return enumPtr;
}

tempo_utils::Result<lyric_assembler::EnumSymbol *>
lyric_assembler::BlockHandle::resolveEnum(const lyric_common::TypeDef &enumType)
{
    if (enumType.getType() != lyric_common::TypeDefType::Concrete)
        return logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "invalid enum type {}", enumType.toString());
    auto enumUrl = enumType.getConcreteUrl();

    lyric_common::SymbolUrl resolvedUrl;
    TU_ASSIGN_OR_RETURN (resolvedUrl, resolveDefinition(enumUrl.getSymbolPath()));

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(enumUrl));
    if (symbol->getSymbolType() != SymbolType::ENUM)
        return logAndContinue(AssemblerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "type {} is not an enum", enumType.toString());

    return cast_symbol_to_enum(symbol);
}

tempo_utils::Result<lyric_assembler::InstanceSymbol *>
lyric_assembler::BlockHandle::declareInstance(
    const std::string &name,
    InstanceSymbol *superInstance,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    bool declOnly)
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *typeCache = m_state->typeCache();

    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare instance {}; symbol is already defined", name);

    auto superDerive = superInstance->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive instance {} from {}; base instance is marked final",
            name, superInstance->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && superInstance->isImported())
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive instance {} from {}; sealed base instance must be located in the same module",
            name, superInstance->getSymbolUrl().toString());

    auto instanceUrl = makeSymbolUrl(name);

    // create the type
    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(
        instanceUrl, {}, superInstance->getTypeDef()));

    // create the instance
    auto instanceSymbol = std::make_unique<InstanceSymbol>(instanceUrl, access, derive,
        isAbstract, typeHandle, superInstance, declOnly, this, m_state);

    TU_RETURN_IF_NOT_OK (m_state->appendInstance(instanceSymbol.get()));
    auto *instancePtr = instanceSymbol.release();

    SymbolBinding binding;
    binding.bindingType = BindingType::Descriptor;
    binding.symbolUrl = instanceUrl;
    binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Descriptor);
    m_bindings[name] = binding;

    return instancePtr;
}

tempo_utils::Result<lyric_assembler::InstanceSymbol *>
lyric_assembler::BlockHandle::resolveInstance(const lyric_common::TypeDef &instanceType)
{
    if (instanceType.getType() != lyric_common::TypeDefType::Concrete)
        return logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "invalid instance type {}", instanceType.toString());
    auto instanceUrl = instanceType.getConcreteUrl();

    lyric_common::SymbolUrl resolvedUrl;
    TU_ASSIGN_OR_RETURN (resolvedUrl, resolveDefinition(instanceUrl.getSymbolPath()));

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(instanceUrl));
    if (symbol->getSymbolType() != SymbolType::INSTANCE)
        return logAndContinue(AssemblerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "type {} is not an instance", instanceType.toString());

    return cast_symbol_to_instance(symbol);
}

tempo_utils::Result<lyric_assembler::StructSymbol *>
lyric_assembler::BlockHandle::declareStruct(
    const std::string &name,
    StructSymbol *superStruct,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    bool declOnly)
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *typeCache = m_state->typeCache();

    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare struct {}; symbol is already defined", name);

    auto superDerive = superStruct->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive struct {} from {}; base struct is marked final",
            name, superStruct->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && superStruct->isImported())
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive struct {} from {}; sealed base struct must be located in the same module",
            name, superStruct->getSymbolUrl().toString());

    auto structUrl = makeSymbolUrl(name);

    // create the type
    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(
        structUrl, {}, superStruct->getTypeDef()));

    // create the struct
    auto structSymbol = std::make_unique<StructSymbol>(structUrl, access, derive,
        isAbstract, typeHandle, superStruct, declOnly, this, m_state);

    TU_RETURN_IF_NOT_OK (m_state->appendStruct(structSymbol.get()));
    auto *structPtr = structSymbol.release();

    SymbolBinding binding;
    binding.bindingType = BindingType::Descriptor;
    binding.symbolUrl = structUrl;
    binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Descriptor);
    m_bindings[name] = binding;

    return structPtr;
}

tempo_utils::Result<lyric_assembler::StructSymbol *>
lyric_assembler::BlockHandle::resolveStruct(const lyric_common::TypeDef &structType)
{
    if (structType.getType() != lyric_common::TypeDefType::Concrete)
        return logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "invalid struct type {}", structType.toString());
    auto structUrl = structType.getConcreteUrl();

    lyric_common::SymbolUrl resolvedUrl;
    TU_ASSIGN_OR_RETURN (resolvedUrl, resolveDefinition(structUrl.getSymbolPath()));

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(resolvedUrl));
    if (symbol->getSymbolType() != SymbolType::STRUCT)
        return logAndContinue(AssemblerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "type {} is not a struct", structType.toString());

    return cast_symbol_to_struct(symbol);
}

tempo_utils::Status
lyric_assembler::BlockHandle::useImpls(
    const DataReference &usingRef,
    const absl::flat_hash_set<lyric_common::TypeDef> &implTypes)
{
    AbstractSymbol *symbol;
    auto *symbolCache = m_state->symbolCache();
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(usingRef.symbolUrl));

    switch (symbol->getSymbolType()) {
        case SymbolType::BINDING: {
            auto *bindingSymbol = cast_symbol_to_binding(symbol);
            lyric_common::TypeDef targetType;
            TU_ASSIGN_OR_RETURN (targetType, bindingSymbol->resolveTarget({}));
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(targetType.getConcreteUrl()));
            break;
        }
        case SymbolType::LOCAL: {
            auto *localVariable = cast_symbol_to_local(symbol);
            auto localType = localVariable->getTypeDef();
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(localType.getConcreteUrl()));
            break;
        }
        case SymbolType::LEXICAL: {
            auto *lexicalVariable = cast_symbol_to_lexical(symbol);
            auto lexicalType = lexicalVariable->getTypeDef();
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(lexicalType.getConcreteUrl()));
            break;
        }
        case SymbolType::ARGUMENT: {
            auto *argumentVariable = cast_symbol_to_local(symbol);
            auto argumentType = argumentVariable->getTypeDef();
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(argumentType.getConcreteUrl()));
            break;
        }
        default:
            break;
    }

    absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *>::const_iterator implsBegin;
    absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *>::const_iterator implsEnd;

    switch (symbol->getSymbolType()) {
        case SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(symbol);
            implsBegin = classSymbol->implsBegin();
            implsEnd = classSymbol->implsEnd();
            break;
        }
        case SymbolType::CONCEPT: {
            auto *conceptSymbol = cast_symbol_to_concept(symbol);
            implsBegin = conceptSymbol->implsBegin();
            implsEnd = conceptSymbol->implsEnd();
            break;
        }
        case SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(symbol);
            implsBegin = enumSymbol->implsBegin();
            implsEnd = enumSymbol->implsEnd();
            break;
        }
        case SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(symbol);
            implsBegin = existentialSymbol->implsBegin();
            implsEnd = existentialSymbol->implsEnd();
            break;
        }
        case SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(symbol);
            implsBegin = instanceSymbol->implsBegin();
            implsEnd = instanceSymbol->implsEnd();
            break;
        }
        case SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(symbol);
            implsBegin = structSymbol->implsBegin();
            implsEnd = structSymbol->implsEnd();
            break;
        }
        default:
            return logAndContinue(AssemblerCondition::kInvalidSymbol,
                tempo_tracing::LogSeverity::kError,
                "symbol {} does not support impls", usingRef.symbolUrl.toString());
    }

    for (auto iterator = implsBegin; iterator != implsEnd; iterator++) {
        auto *implHandle = iterator->second;
        auto implType = implHandle->implType()->getTypeDef();
        if (!implTypes.empty() && !implTypes.contains(implType))
            continue;
        if (m_impls.contains(implType))
            return logAndContinue(AssemblerCondition::kImplConflict,
                tempo_tracing::LogSeverity::kError,
                "symbol {} conflicts with impl {}", usingRef.symbolUrl.toString(), implType.toString());
        ImplReference implRef;
        implRef.implType = implType;
        implRef.usingRef = usingRef;
        m_impls[implType] = implRef;
    }

    return {};
}

tempo_utils::Status
lyric_assembler::BlockHandle::useImpls(
    const InstanceSymbol *instanceSymbol,
    const absl::flat_hash_set<lyric_common::TypeDef> &implTypes)
{
    TU_ASSERT (instanceSymbol != nullptr);

    DataReference usingRef;
    usingRef.referenceType = ReferenceType::Value;
    usingRef.symbolUrl = instanceSymbol->getSymbolUrl();
    usingRef.typeDef = instanceSymbol->getTypeDef();
    return useImpls(usingRef, implTypes);
}

bool
lyric_assembler::BlockHandle::hasImpl(const lyric_common::TypeDef &implType) const
{
    return m_impls.contains(implType);
}

Option<lyric_assembler::ImplReference>
lyric_assembler::BlockHandle::getImpl(const lyric_common::TypeDef &implType) const
{
    auto entry = m_impls.find(implType);
    if (entry != m_impls.cend())
        return Option(entry->second);
    return {};
}

tempo_utils::Result<lyric_assembler::ImplReference>
lyric_assembler::BlockHandle::resolveImpl(
    const lyric_common::TypeDef &implType,
    const std::vector<lyric_common::TypeDef> &fallbackImplTypes)
{
    // look for a suitable instance in the current block or an ancestor block
    for (auto *block = this; block != nullptr; block = block->m_parentBlock) {
        if (block->m_impls.contains(implType)) {
            auto entry = block->m_impls.find(implType);
            if (entry != m_impls.cend())
                return entry->second;
        }
    }

    if (fallbackImplTypes.empty())
        return logAndContinue(AssemblerCondition::kMissingImpl,
            tempo_tracing::LogSeverity::kError,
            "no instance found implementing {}", implType.toString());

    auto &front = fallbackImplTypes.front();
    auto cbegin = fallbackImplTypes.cbegin();
    std::vector remaining(++cbegin, fallbackImplTypes.cend());
    return resolveImpl(front, remaining);
}

tempo_utils::Result<lyric_assembler::BindingSymbol *>
lyric_assembler::BlockHandle::declareBinding(
    const std::string &name,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *typeCache = m_state->typeCache();

    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare binding {}; symbol is already defined", name);

    auto bindingUrl = makeSymbolUrl(name);

    // create the template if there are any template parameters
    TemplateHandle *bindingTemplate = nullptr;
    if (!templateParameters.empty()) {
        TU_ASSIGN_OR_RETURN (bindingTemplate, typeCache->makeTemplate(bindingUrl, templateParameters, this));
    }

    // ensure that Binding is in the type cache
    auto bindingType = fundamentalCache->getFundamentalType(FundamentalSymbol::Binding);
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(bindingType));

    // create the type
    TypeHandle *typeHandle;
    if (bindingTemplate) {
        TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(
            bindingUrl, bindingTemplate->getPlaceholders(), bindingType));
    } else {
        TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(bindingUrl, {}, bindingType));
    }

    // create the binding
    std::unique_ptr<BindingSymbol> bindingSymbol;
    if (bindingTemplate) {
        bindingSymbol = std::make_unique<BindingSymbol>(bindingUrl, access, typeHandle,
            bindingTemplate, this, m_state);
    } else {
        bindingSymbol = std::make_unique<BindingSymbol>(bindingUrl, access, typeHandle,
            this, m_state);
    }

    TU_RETURN_IF_NOT_OK (m_state->appendBinding(bindingSymbol.get()));
    auto *bindingPtr = bindingSymbol.release();

    SymbolBinding binding;
    binding.bindingType = BindingType::Descriptor;
    binding.symbolUrl = bindingUrl;
    binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Descriptor);
    m_bindings[name] = binding;

    return bindingPtr;
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareAlias(
    const std::string &alias,
    const lyric_common::SymbolUrl &targetUrl)
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *symbolCache = m_state->symbolCache();

    if (m_bindings.contains(alias))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; symbol is already defined", alias);

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(targetUrl));

    SymbolBinding binding;

    // if the symbol is linkage, then determine the type from the linkage
    auto symbolType = symbol->getSymbolType();
    if (symbolType == SymbolType::LINKAGE) {
        switch (cast_symbol_to_linkage(symbol)->getLinkage()) {
            case lyric_object::LinkageSection::Action:
            case lyric_object::LinkageSection::Binding:
            case lyric_object::LinkageSection::Call:
            case lyric_object::LinkageSection::Class:
            case lyric_object::LinkageSection::Concept:
            case lyric_object::LinkageSection::Enum:
            case lyric_object::LinkageSection::Existential:
            case lyric_object::LinkageSection::Field:
            case lyric_object::LinkageSection::Instance:
            case lyric_object::LinkageSection::Static:
            case lyric_object::LinkageSection::Struct:
                binding.bindingType = BindingType::Descriptor;
                binding.symbolUrl = targetUrl;
                binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Descriptor);
                break;
            case lyric_object::LinkageSection::Namespace:
                binding.bindingType = BindingType::Namespace;
                binding.symbolUrl = targetUrl;
                binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Namespace);
                break;
            default:
                return logAndContinue(AssemblerCondition::kMissingSymbol,
                    tempo_tracing::LogSeverity::kError,
                    "cannot declare alias {}; {} linkage symbol section is not a valid target",
                    alias, targetUrl.toString());
        }
        m_bindings[alias] = binding;
        return binding;
    }

    // set binding type
    switch (symbolType) {
        case SymbolType::BINDING:
        case SymbolType::CALL:
        case SymbolType::CLASS:
        case SymbolType::CONCEPT:
        case SymbolType::CONSTANT:
        case SymbolType::ENUM:
        case SymbolType::EXISTENTIAL:
        case SymbolType::INSTANCE:
        case SymbolType::STATIC:
        case SymbolType::STRUCT:
            binding.bindingType = BindingType::Descriptor;
            binding.symbolUrl = targetUrl;
            binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Descriptor);
            break;

        case SymbolType::NAMESPACE:
            binding.bindingType = BindingType::Namespace;
            binding.symbolUrl = targetUrl;
            binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Namespace);
            break;

        default:
            return logAndContinue(AssemblerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "cannot declare alias {}; {} is not a valid target", alias, targetUrl.toString());
    }

    m_bindings[alias] = binding;
    return binding;
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareAlias(
    const std::string &alias,
    const SymbolBinding &binding)
{
    auto *symbolCache = m_state->symbolCache();

    if (m_bindings.contains(alias))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; symbol is already defined", alias);

    // verify binding symbol exists in the symbol cache
    TU_RETURN_IF_STATUS (symbolCache->getOrImportSymbol(binding.symbolUrl));

    m_bindings[alias] = binding;
    return binding;
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareAlias(
    const std::string &alias,
    const DataReference &targetRef)
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *symbolCache = m_state->symbolCache();

    if (m_bindings.contains(alias))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; symbol is already defined", alias);

    // verify binding symbol exists in the symbol cache
    TU_RETURN_IF_STATUS (symbolCache->getOrImportSymbol(targetRef.symbolUrl));

    SymbolBinding binding;

    switch (targetRef.referenceType) {

        case ReferenceType::Descriptor:
            binding.bindingType = BindingType::Descriptor;
            binding.symbolUrl = targetRef.symbolUrl;
            binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Descriptor);
            break;

        case ReferenceType::Namespace:
            binding.bindingType = BindingType::Namespace;
            binding.symbolUrl = targetRef.symbolUrl;
            binding.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Namespace);
            break;

        case ReferenceType::Value:
            binding.bindingType = BindingType::Value;
            binding.symbolUrl = targetRef.symbolUrl;
            binding.typeDef = targetRef.typeDef;
            break;

        case ReferenceType::Variable:
            binding.bindingType = BindingType::Variable;
            binding.symbolUrl = targetRef.symbolUrl;
            binding.typeDef = targetRef.typeDef;
            break;

        default:
            throwAssemblerInvariant("failed to declare alias to {}; invalid reference type",
                targetRef.symbolUrl.toString());
    }

    m_bindings[alias] = binding;
    return binding;
}

/**
 *
 */
tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareAlias(
    const std::string &alias,
    const DataReference &targetRef,
    const lyric_common::TypeDef &aliasType)
{
    auto *symbolCache = m_state->symbolCache();

    if (m_bindings.contains(alias))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; symbol is already defined", alias);

    // verify binding symbol exists in the symbol cache
    TU_RETURN_IF_STATUS (symbolCache->getOrImportSymbol(targetRef.symbolUrl));

    SymbolBinding binding;

    switch (targetRef.referenceType) {

        case ReferenceType::Value:
            binding.bindingType = BindingType::Value;
            binding.symbolUrl = targetRef.symbolUrl;
            binding.typeDef = aliasType;
            break;

        case ReferenceType::Variable:
            binding.bindingType = BindingType::Variable;
            binding.symbolUrl = targetRef.symbolUrl;
            binding.typeDef = aliasType;
            break;

        default:
            return logAndContinue(AssemblerCondition::kAssemblerInvariant,
                tempo_tracing::LogSeverity::kError,
                "failed to declare alias to {}; invalid reference type",
                targetRef.symbolUrl.toString());
    }

    m_bindings[alias] = binding;
    return binding;
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareAlias(
    const std::string &alias,
    const lyric_common::SymbolUrl &templateUrl,
    int placeholderIndex)
{
    if (m_bindings.contains(alias))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; symbol is already defined", alias);
    if (!m_state->typeCache()->hasTemplate(templateUrl))
        return logAndContinue(AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; missing template for {}", alias, templateUrl.toString());

    SymbolBinding binding;
    binding.typeDef = lyric_common::TypeDef::forPlaceholder(placeholderIndex, templateUrl);
    binding.bindingType = BindingType::Placeholder;

    m_bindings[alias] = binding;
    return binding;
}

lyric_common::SymbolUrl
lyric_assembler::BlockHandle::makeSymbolUrl(const std::string &name) const
{
    const auto enclosingDefinition = getDefinition();
    auto path = enclosingDefinition.getSymbolPath().getPath();
    path.push_back(name);
    auto symbolUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(path));
    TU_LOG_INFO << "symbol url for " << name << " is " << symbolUrl.toString();
    return symbolUrl;
}

void
lyric_assembler::BlockHandle::debugBindings() const
{
    for (auto *block = this; block != nullptr; block = block->m_parentBlock) {
        auto definition = block->getDefinition();
        if (definition.isValid()) {
            TU_LOG_INFO << "DEBUG BlockHandle@" << block << " [" << definition << "] bindings:";
        } else {
            TU_LOG_INFO << "DEBUG BlockHandle@" << block << " bindings:";
        }
        for (auto &entry : block->m_bindings) {
            auto &name = entry.first;
            auto &binding = entry.second;
            TU_LOG_INFO << "  " << name << " -> " << binding.symbolUrl;
        }
    }
}