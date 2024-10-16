
#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/argument_variable.h>
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
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/synthetic_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <tempo_utils/log_stream.h>

lyric_assembler::BlockHandle::BlockHandle(
    ProcHandle *blockProc,
    ObjectState *state,
    bool isRoot)
    : m_blockNs(nullptr),
      m_blockProc(blockProc),
      m_parentBlock(nullptr),
      m_state(state),
      m_isRoot(isRoot)
{
    TU_ASSERT (m_blockProc != nullptr);
    TU_ASSERT (m_state != nullptr);
    if (!m_isRoot) {
        m_definition = m_blockProc->getActivation();
    }
}

lyric_assembler::BlockHandle::BlockHandle(
    const lyric_common::SymbolUrl &definition,
    BlockHandle *parentBlock,
    bool isRoot)
    : m_definition(definition),
      m_blockNs(nullptr),
      m_parentBlock(parentBlock),
      m_isRoot(isRoot)
{
    TU_ASSERT (m_definition.isValid());
    TU_ASSERT (m_parentBlock != nullptr);
    m_blockProc = parentBlock->m_blockProc;
    m_state = parentBlock->m_state;
    TU_ASSERT (m_blockProc != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::BlockHandle::BlockHandle(
    NamespaceSymbol *blockNs,
    ProcHandle *blockProc,
    BlockHandle *parentBlock,
    ObjectState *state,
    bool isRoot)
    : m_blockNs(blockNs),
      m_blockProc(blockProc),
      m_parentBlock(parentBlock),
      m_state(state),
      m_isRoot(isRoot)
{
    TU_ASSERT (m_blockNs != nullptr);
    TU_ASSERT (m_blockProc != nullptr);
    TU_ASSERT (m_parentBlock != nullptr);
    TU_ASSERT (m_state != nullptr);
    if (!m_isRoot) {
        m_definition = m_blockProc->getActivation();
    }
}

lyric_assembler::BlockHandle::BlockHandle(
    ProcHandle *blockProc,
    BlockHandle *parentBlock,
    ObjectState *state)
    : m_blockNs(nullptr),
      m_blockProc(blockProc),
      m_parentBlock(parentBlock),
      m_state(state),
      m_isRoot(false)
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
      m_isRoot(false),
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
      m_isRoot(false),
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

//lyric_assembler::ProcBuilder *
//lyric_assembler::BlockHandle::blockCode()
//{
//    return m_blockCode;
//}

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

bool
lyric_assembler::BlockHandle::isRoot() const
{
    return m_isRoot;
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

absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl>::const_iterator
lyric_assembler::BlockHandle::instancesBegin() const
{
    return m_impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl>::const_iterator
lyric_assembler::BlockHandle::instancesEnd() const
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

static tempo_utils::Result<lyric_assembler::SymbolBinding>
resolve_binding_tail_recursive(
    lyric_assembler::BlockHandle *block,
    const std::vector<std::string> &path,
    std::vector<std::string>::const_iterator curr,
    std::vector<std::string>::const_iterator end)   // NOLINT(misc-no-recursion)
{
    if (curr == end || !block->hasBinding(*curr))
        return block->logAndContinue(
            lyric_assembler::AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", lyric_common::SymbolPath(path).toString());

    auto binding = block->getBinding(*curr);

    // if we have reached the last segment then return the binding
    if (++curr == end)
        return binding;

    // if there are subsequent segments then the binding must be a descriptor
    if (binding.bindingType != lyric_assembler::BindingType::Descriptor)
        return block->logAndContinue(
            lyric_assembler::AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", lyric_common::SymbolPath(path).toString());

    auto *state = block->blockState();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(binding.symbolUrl));

    // if there are subsequent segments then the symbol must be a namespace
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::NAMESPACE)
        return block->logAndContinue(
            lyric_assembler::AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", lyric_common::SymbolPath(path).toString());

    auto *childBlock = cast_symbol_to_namespace(symbol)->namespaceBlock();

    return resolve_binding_tail_recursive(childBlock, path, curr, end);
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
    for (auto *block = this; block != nullptr; block = block->blockParent()) {
        if (!block->hasBinding(*curr))
            continue;
        return resolve_binding_tail_recursive(block, path, curr, path.cend());
    }

    // if definition is not found in any reachable block, then check env
    if (m_state->symbolCache()->hasEnvBinding(*curr)) {
        auto binding = m_state->symbolCache()->getEnvBinding(*curr);
        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(binding.symbolUrl));

        // if there are no more path segments then return the binding
        if (++curr == path.cend())
            return binding;

        // otherwise we expect the current segment is a namespace
        BlockHandle *block;
        switch (symbol->getSymbolType()) {
            case lyric_assembler::SymbolType::NAMESPACE:
                block = cast_symbol_to_namespace(symbol)->namespaceBlock();
                break;
            default:
                return logAndContinue(
                    lyric_assembler::AssemblerCondition::kMissingSymbol,
                    tempo_tracing::LogSeverity::kError,
                    "missing symbol {}", lyric_common::SymbolPath(path).toString());
        }

        // continue the resolution
        return resolve_binding_tail_recursive(block, path, curr, path.cend());
    }

    return logAndContinue(
        lyric_assembler::AssemblerCondition::kMissingSymbol,
        tempo_tracing::LogSeverity::kError,
        "missing symbol {}", lyric_common::SymbolPath(path).toString());
}

//tempo_utils::Result<lyric_common::TypeDef>
//lyric_assembler::BlockHandle::resolveAssignable(const lyric_typing::TypeSpec &assignableSpec) // NOLINT(misc-no-recursion)
//{
//    if (assignableSpec.getType() == lyric_parser::AssignableType::SINGULAR)
//        return resolveSingular(assignableSpec);
//
//    if (assignableSpec.getType() == lyric_parser::AssignableType::UNION) {
//        std::vector<lyric_common::TypeDef> unionMembers;
//        for (const auto &memberSpec : assignableSpec.getUnion()) {
//            auto resolveMemberResult = resolveSingular(memberSpec);
//            if (resolveMemberResult.isStatus())
//                return resolveMemberResult;
//            unionMembers.push_back(resolveMemberResult.getResult());
//        }
//        return m_state->typeCache()->resolveUnion(unionMembers);
//    }
//    if (assignableSpec.getType() == lyric_parser::AssignableType::INTERSECTION) {
//        std::vector<lyric_common::TypeDef> intersectionMembers;
//        for (const auto &memberSpec : assignableSpec.getIntersection()) {
//            auto resolveMemberResult = resolveSingular(memberSpec);
//            if (resolveMemberResult.isStatus())
//                return resolveMemberResult;
//            intersectionMembers.push_back(resolveMemberResult.getResult());
//        }
//        return m_state->typeCache()->resolveIntersection(intersectionMembers);
//    }
//
//    m_state->throwAssemblerInvariant("failed to resolve non singular base type {}", assignableSpec.toString());
//}
//
//tempo_utils::Result<lyric_common::TypeDef>
//lyric_assembler::BlockHandle::resolveSingular(const lyric_typing::TypeSpec &assignableSpec) // NOLINT(misc-no-recursion)
//{
//    if (assignableSpec.getType() != lyric_parser::AssignableType::SINGULAR)
//        throwAssemblerInvariant("{} is a non-singular type", assignableSpec.toString());
//
//    // if spec has parameters, then resolve them to types first
//    std::vector<lyric_common::TypeDef> typeParameters;
//    for (const auto &parameter : assignableSpec.getTypeParameters()) {
//        auto resolveParameterResult = resolveAssignable(parameter);
//        if (resolveParameterResult.isStatus())
//            return resolveParameterResult;
//        typeParameters.push_back(resolveParameterResult.getResult());
//    }
//
//    // resolve the base to a binding
//    SymbolBinding binding;
//    TU_ASSIGN_OR_RETURN (binding, resolveBinding(assignableSpec.getTypePath().getPath()));
//
//    // we expect the binding is either a descriptor or placeholder
//    lyric_common::TypeDef typeDef;
//    switch (binding.bindingType) {
//        case BindingType::Descriptor:
//            typeDef = lyric_common::TypeDef::forConcrete(binding.symbolUrl, typeParameters);
//            break;
//        case BindingType::Placeholder:
//            typeDef = binding.typeDef;
//            break;
//        default:
//            return logAndContinue(
//                lyric_assembler::AssemblerCondition::kMissingSymbol,
//                tempo_tracing::LogSeverity::kError,
//                "{} is not a valid type", assignableSpec.getTypePath().toString());
//    }
//
//    auto *typeCache = m_state->typeCache();
//
//    // add the type to the typecache if it doesn't already exist
//    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(typeDef));
//
//    return typeDef;
//}

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
                lyric_assembler::AssemblerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "{} does not refer to a valid type", typePath.toString());
    }

    auto *typeCache = m_state->typeCache();

    // add the type to the typecache if it doesn't already exist
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(typeDef));

    return typeDef;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::resolveDefinition(const std::vector<std::string> &path)
{
    SymbolBinding binding;
    TU_ASSIGN_OR_RETURN (binding, resolveBinding(path));

    auto *symbolCache = m_state->symbolCache();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(binding.symbolUrl));

    switch (symbol->getSymbolType()) {
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
                lyric_assembler::AssemblerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "missing definition {}", lyric_common::SymbolPath(path).toString());
    }
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::resolveDefinition(const lyric_common::SymbolPath &symbolPath)
{
    std::vector<std::string> typePath;
    for (const auto &part : symbolPath.getPath()) {
        typePath.push_back(part);
    }
    return resolveDefinition(typePath);
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::BlockHandle::declareVariable(
    const std::string &name,
    lyric_object::AccessType access,
    const lyric_common::TypeDef &assignableType,
    bool isVariable)
{
    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare variable {}; symbol is already defined", name);

    BindingType bindingType = isVariable? BindingType::Variable : BindingType::Value;
    ReferenceType referenceType = isVariable? ReferenceType::Variable : ReferenceType::Value;

    // make type handle if it doesn't exist already (for example union or intersection types)
    TU_RETURN_IF_STATUS (m_state->typeCache()->getOrMakeType(assignableType));

    // if this is a root block then create a static variable, otherwise create a local
    auto localUrl = makeSymbolUrl(name);
    auto address = m_blockProc->allocateLocal();
    auto *localVariable = new LocalVariable(localUrl, access, assignableType, address);
    auto status = m_state->symbolCache()->insertSymbol(localUrl, localVariable);
    if (status.notOk()) {
        delete localVariable;
        return status;
    }

    SymbolBinding binding;
    binding.symbolUrl = localUrl;
    binding.typeDef = assignableType;
    binding.bindingType = bindingType;
    m_bindings[name] = binding;

    return DataReference(localUrl, assignableType, referenceType);
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::BlockHandle::declareTemporary(
    const lyric_common::TypeDef &assignableType,
    bool isVariable)
{
    auto name = absl::StrCat("$tmp", m_blockProc->numLocals());

    if (m_bindings.contains(name))
        throwAssemblerInvariant("failed to declare temporary {}; symbol is is already defined", name);

    BindingType bindingType = isVariable? BindingType::Variable : BindingType::Value;
    ReferenceType referenceType = isVariable? ReferenceType::Variable : ReferenceType::Value;

    // make type handle if it doesn't exist already (for example union or intersection types)
    TU_RETURN_IF_STATUS (m_state->typeCache()->getOrMakeType(assignableType));

    // temporary variables are always local
    auto localUrl = makeSymbolUrl(name);
    auto address = m_blockProc->allocateLocal();
    auto *localVariable = new LocalVariable(localUrl, lyric_object::AccessType::Private, assignableType, address);
    auto status = m_state->symbolCache()->insertSymbol(localUrl, localVariable);
    if (status.notOk()) {
        delete localVariable;
        return status;
    }

    SymbolBinding binding;
    binding.symbolUrl = localUrl;
    binding.typeDef = assignableType;
    binding.bindingType = bindingType;
    m_bindings[name] = binding;

    return DataReference(localUrl, assignableType, referenceType);
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::BlockHandle::declareStatic(
    const std::string &name,
    lyric_object::AccessType access,
    const lyric_common::TypeDef &assignableType,
    bool isVariable,
    bool declOnly)
{
    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare static {}; symbol is already defined", name);

    BindingType bindingType = isVariable? BindingType::Variable : BindingType::Value;
    ReferenceType referenceType = isVariable? ReferenceType::Variable : ReferenceType::Value;

    // make type handle if it doesn't exist already (for example union or intersection types)
    lyric_assembler::TypeHandle *staticType;
    TU_ASSIGN_OR_RETURN (staticType, m_state->typeCache()->getOrMakeType(assignableType));

    auto staticUrl = makeSymbolUrl(name);

    auto *staticSymbol = new StaticSymbol(staticUrl, access, isVariable, staticType, declOnly, this, m_state);
    auto status = m_state->appendStatic(staticSymbol);
    if (status.notOk()) {
        delete staticSymbol;
        return status;
    }

    SymbolBinding binding;
    binding.symbolUrl = staticUrl;
    binding.typeDef = assignableType;
    binding.bindingType = bindingType;
    m_bindings[name] = binding;

    return DataReference(staticUrl, assignableType, referenceType);
}

inline lyric_assembler::DataReference
symbol_binding_to_data_reference(const lyric_assembler::SymbolBinding &binding)
{
    switch (binding.bindingType) {
        case lyric_assembler::BindingType::Value:
            return lyric_assembler::DataReference{binding.symbolUrl,
                binding.typeDef, lyric_assembler::ReferenceType::Value};
        case lyric_assembler::BindingType::Variable:
            return lyric_assembler::DataReference{binding.symbolUrl,
                binding.typeDef, lyric_assembler::ReferenceType::Variable};
        case lyric_assembler::BindingType::Descriptor:
            return lyric_assembler::DataReference{binding.symbolUrl,
                binding.typeDef, lyric_assembler::ReferenceType::Descriptor};
        default:
            return lyric_assembler::DataReference{{}, {}, lyric_assembler::ReferenceType::Invalid};
    }
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::BlockHandle::resolveReference(const std::string &name)
{
    BlockHandle *block = this;

    // if variable exists in a parent block in the current proc, then return it
    for (; block != nullptr && block->m_blockProc == m_blockProc; block = block->m_parentBlock) {
        if (block->m_bindings.contains(name)) {
            const auto &binding = block->m_bindings[name];
            lyric_assembler::AbstractSymbol *symbol;
            TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(binding.symbolUrl));

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
                {
                    return symbol_binding_to_data_reference(binding);
                }
                default:
                    return logAndContinue(lyric_assembler::AssemblerCondition::kMissingVariable,
                        tempo_tracing::LogSeverity::kError,
                        "missing variable {}", name);
            }
        }
    }

    // if variable exists in a parent proc, then import lexical into this proc
    for (; block != nullptr; block = block->m_parentBlock) {
        if (block->m_bindings.contains(name)) {
            const auto &binding = block->m_bindings[name];
            lyric_assembler::AbstractSymbol *symbol;
            TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(binding.symbolUrl));

            lyric_assembler::LexicalTarget lexicalTarget;
            uint32_t targetOffset = lyric_runtime::INVALID_ADDRESS_U32;
            lyric_common::TypeDef typeDef;

            switch (symbol->getSymbolType()) {
                // found a local in a parent proc
                case SymbolType::LOCAL: {
                    auto *local = static_cast<LocalVariable *>(symbol);
                    if (local->getAccessType() == lyric_object::AccessType::Private)
                        return logAndContinue(lyric_assembler::AssemblerCondition::kInvalidBinding,
                            tempo_tracing::LogSeverity::kError,
                            "variable {} is not visible from the current scope", name);
                    lexicalTarget = lyric_assembler::LexicalTarget::Local;
                    targetOffset = local->getOffset().getOffset();
                    typeDef = binding.typeDef;
                    break;
                }

                // found a static variable or a symbol
                case SymbolType::EXISTENTIAL:
                case SymbolType::INSTANCE:
                case SymbolType::CALL:
                case SymbolType::CLASS:
                case SymbolType::CONCEPT:
                case SymbolType::STRUCT:
                case SymbolType::ENUM:
                case SymbolType::NAMESPACE:
                case SymbolType::STATIC: {
                    return symbol_binding_to_data_reference(binding);
                }

                // other symbol types are not valid
                case SymbolType::ARGUMENT:
                case SymbolType::LEXICAL:
                default:
                    return logAndContinue(lyric_assembler::AssemblerCondition::kInvalidBinding,
                        tempo_tracing::LogSeverity::kError,
                        "variable {} is not visible from the current scope", name);
            }

            auto activation = block->blockProc()->getActivation();
            TU_ASSERT (m_state->symbolCache()->hasSymbol(activation));
            lyric_assembler::AbstractSymbol *activationSymbol;
            TU_ASSIGN_OR_RETURN (activationSymbol, m_state->symbolCache()->getOrImportSymbol(activation));
            auto *activationCall = cast_symbol_to_call(activationSymbol);

            // import lexical variable into this env, and return it
            auto lexicalUrl = makeSymbolUrl(name);
            auto address = m_blockProc->allocateLexical(lexicalTarget, targetOffset, activationCall);
            auto *lexicalVariable = new LexicalVariable(lexicalUrl, typeDef, address);
            m_state->symbolCache()->insertSymbol(lexicalUrl, lexicalVariable);

            SymbolBinding lexical;
            lexical.symbolUrl = lexicalUrl;
            lexical.typeDef = typeDef;
            lexical.bindingType = BindingType::Value;
            m_bindings[name] = lexical;

            return symbol_binding_to_data_reference(lexical);
        }
    }

    // if variable is not found in any reachable block, then check env
    if (m_state->symbolCache()->hasEnvBinding(name)) {
        const auto binding = m_state->symbolCache()->getEnvBinding(name);
        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(binding.symbolUrl));

        switch (symbol->getSymbolType()) {
            case SymbolType::EXISTENTIAL:
            case SymbolType::CONSTANT:
            case SymbolType::STATIC:
            case SymbolType::INSTANCE:
            case SymbolType::CALL:
            case SymbolType::CLASS:
            case SymbolType::CONCEPT:
            case SymbolType::STRUCT:
            case SymbolType::ENUM:
            case SymbolType::NAMESPACE:
                return symbol_binding_to_data_reference(binding);
            default:
                return logAndContinue(lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    tempo_tracing::LogSeverity::kError,
                    "environment binding {} has invalid symbol type", name);
        }
    }

    // we have exhausted our search, variable is missing
    return logAndContinue(lyric_assembler::AssemblerCondition::kMissingVariable,
        tempo_tracing::LogSeverity::kError,
        "missing variable {}", name);
}

//tempo_utils::Status
//lyric_assembler::BlockHandle::load(const DataReference &ref)
//{
//    auto *fragment = m_blockCode->rootFragment();
//    return fragment->loadRef(ref);
//}
//
//tempo_utils::Status
//lyric_assembler::BlockHandle::store(const DataReference &ref, bool initialStore)
//{
//    auto *fragment = m_blockCode->rootFragment();
//
//    switch (ref.referenceType) {
//        case ReferenceType::Variable:
//            return fragment->storeRef(ref);
//        case ReferenceType::Value: {
//            if (initialStore)
//                return fragment->storeRef(ref);
//            [[fallthrough]];
//        }
//        case ReferenceType::Descriptor:
//            return logAndContinue(AssemblerCondition::kInvalidBinding,
//                tempo_tracing::LogSeverity::kError,
//                "cannot store data at reference {}", ref.symbolUrl.toString());
//        default:
//            return AssemblerStatus::forCondition(
//                AssemblerCondition::kAssemblerInvariant, "invalid data reference");
//    }
//}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::BlockHandle::declareFunction(
    const std::string &name,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    bool declOnly)
{
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
    CallSymbol *callSymbol;
    if (functionTemplate) {
        callSymbol = new CallSymbol(functionUrl, access, lyric_object::CallMode::Normal,
            functionTemplate, declOnly, this, m_state);
    } else {
        callSymbol = new CallSymbol(functionUrl, access, lyric_object::CallMode::Normal,
            declOnly, this, m_state);
    }

    auto status = m_state->appendCall(callSymbol);
    if (status.notOk()) {
        delete callSymbol;
        return status;
    }

    SymbolBinding binding;
    binding.symbolUrl = functionUrl;
    binding.bindingType = BindingType::Descriptor;
    m_bindings[name] = binding;

    return callSymbol;
}

tempo_utils::Status
lyric_assembler::BlockHandle::prepareFunction(const std::string &name, CallableInvoker &invoker)
{
    auto resolveDefinitionResult = resolveDefinition({name});
    if (resolveDefinitionResult.isStatus())
        return resolveDefinitionResult.getStatus();

    auto functionUrl = resolveDefinitionResult.getResult();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(functionUrl));
    if (symbol->getSymbolType() != SymbolType::CALL)
        throwAssemblerInvariant("invalid call symbol {}", functionUrl.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    auto callable = std::make_unique<FunctionCallable>(callSymbol, /* isInlined= */ false);
    return invoker.initialize(std::move(callable));
}

tempo_utils::Status
lyric_assembler::BlockHandle::prepareExtension(
    const lyric_common::TypeDef &receiverType,
    const std::string &name,
    CallableInvoker &invoker)
{
//    auto resolveDefinitionResult = resolveReference(QStringList(name));
//    if (resolveDefinitionResult.isStatus())
//        return tempo_utils::Result<CallInvoker>(lyric_assembler::AssemblerStatus::missingFunction(name));
//
//    auto functionUrl = resolveDefinitionResult.getResult();
//    auto *sym = m_state->symcache[functionUrl];
//    if (sym == nullptr)
//        return tempo_utils::Result<CallInvoker>(
//            lyric_assembler::AssemblerStatus::internalViolation("invalid extension call"));
//    if (sym->getSymbolType() != SymbolType::CALL)
//        return tempo_utils::Result<CallInvoker>(lyric_assembler::AssemblerStatus::missingFunction(name));
//    auto *call = cast_symbol_to_call(sym);
//
//    if (!bool(call->getFlags() | lya1::CallFlags::Magnet))
//        return tempo_utils::Result<CallInvoker>(
//            lyric_assembler::AssemblerStatus::internalViolation("invalid extension call"));
//
//    return tempo_utils::Result<CallInvoker>(CallInvoker(call));
    throwAssemblerInvariant("resolveExtension is unimplemented");
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
    ClassSymbol *classSymbol;
    if (classTemplate) {
        classSymbol = new ClassSymbol(classUrl, access, derive, isAbstract, typeHandle,
            classTemplate, superClass, declOnly, this, m_state);
    } else {
        classSymbol = new ClassSymbol(classUrl, access, derive, isAbstract,
            typeHandle, superClass, declOnly, this, m_state);
    }

    auto status = m_state->appendClass(classSymbol);
    if (status.notOk()) {
        delete classSymbol;
        return status;
    }

    SymbolBinding binding;
    binding.symbolUrl = classUrl;
    binding.typeDef = classSymbol->getTypeDef();
    binding.bindingType = BindingType::Descriptor;
    m_bindings[name] = binding;

    return classSymbol;
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
    ConceptSymbol *conceptSymbol;
    if (conceptTemplate) {
        conceptSymbol = new ConceptSymbol(conceptUrl, access, derive, typeHandle,
            conceptTemplate, superConcept, declOnly, this, m_state);
    } else {
        conceptSymbol = new ConceptSymbol(conceptUrl, access, derive, typeHandle,
            superConcept, declOnly, this, m_state);
    }

    auto status = m_state->appendConcept(conceptSymbol);
    if (status.notOk()) {
        delete conceptSymbol;
        return status;
    }

    SymbolBinding binding;
    binding.symbolUrl = conceptUrl;
    binding.typeDef = conceptSymbol->getTypeDef();
    binding.bindingType = BindingType::Descriptor;
    m_bindings[name] = binding;

    return conceptSymbol;
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
    TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareSubType(
        enumUrl, {}, superEnum->getTypeDef()));

    // create the instance
    auto *enumSymbol = new EnumSymbol(enumUrl, access, derive, isAbstract, typeHandle,
        superEnum, declOnly, this, m_state);

    auto status = m_state->appendEnum(enumSymbol);
    if (status.notOk()) {
        delete enumSymbol;
        return status;
    }

    SymbolBinding binding;
    binding.symbolUrl = enumUrl;
    binding.typeDef = enumSymbol->getTypeDef();
    binding.bindingType = BindingType::Descriptor;
    m_bindings[name] = binding;

    return enumSymbol;
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
    TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareSubType(
        instanceUrl, {}, superInstance->getTypeDef()));

    // create the instance
    auto *instanceSymbol = new InstanceSymbol(instanceUrl, access, derive, isAbstract, typeHandle,
        superInstance, declOnly, this, m_state);

    auto status = m_state->appendInstance(instanceSymbol);
    if (status.notOk()) {
        delete instanceSymbol;
        return status;
    }

    SymbolBinding binding;
    binding.symbolUrl = instanceUrl;
    binding.typeDef = instanceSymbol->getTypeDef();
    binding.bindingType = BindingType::Descriptor;
    m_bindings[name] = binding;

    return instanceSymbol;
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
    TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareSubType(
        structUrl, {}, superStruct->getTypeDef()));

    // create the struct
    auto *structSymbol = new StructSymbol(structUrl, access, derive, isAbstract,
        typeHandle, superStruct, declOnly, this, m_state);

    auto status = m_state->appendStruct(structSymbol);
    if (status.notOk()) {
        delete structSymbol;
        return status;
    }

    SymbolBinding binding;
    binding.symbolUrl = structUrl;
    binding.typeDef = structSymbol->getTypeDef();
    binding.bindingType = BindingType::Descriptor;
    m_bindings[name] = binding;

    return structSymbol;
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
lyric_assembler::BlockHandle::useSymbol(
    const lyric_common::SymbolUrl &symbolUrl,
    const absl::flat_hash_set<lyric_common::TypeDef> &implTypes)
{
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(symbolUrl));

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
                "symbol {} does not support impls", symbolUrl.toString());
    }

    for (auto iterator = implsBegin; iterator != implsEnd; iterator++) {
        auto *implHandle = iterator->second;
        auto implType = implHandle->implType()->getTypeDef();
        if (!implTypes.empty() && !implTypes.contains(implType))
            continue;
        if (m_impls.contains(implType)) {
            const auto &impl = m_impls.at(implType);
            return logAndContinue(AssemblerCondition::kImplConflict,
                tempo_tracing::LogSeverity::kError,
                "symbol {} conflicts with impl {}", symbolUrl.toString(), impl.toString());
        }
        m_impls[implType] = symbolUrl;
    }

    return AssemblerStatus::ok();
}

bool
lyric_assembler::BlockHandle::hasImpl(const lyric_common::TypeDef &implType) const
{
    return m_impls.contains(implType);
}

Option<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::getImpl(const lyric_common::TypeDef &implType) const
{
    if (m_impls.contains(implType))
        return Option<lyric_common::SymbolUrl>(m_impls.at(implType));
    return {};
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::resolveImpl(const lyric_common::TypeDef &implType, ResolveMode mode)
{
    // look for a suitable instance in the current block or an ancestor block
    for (auto *block = this; block != nullptr; block = block->m_parentBlock) {
        if (block->m_impls.contains(implType)) {
            const auto &instanceUrl = block->m_impls[implType];
            return instanceUrl;
        }
    }

    // if instance is not found in any reachable block, then check env
    if (m_state->symbolCache()->hasEnvInstance(implType)) {
        const auto instanceUrl = m_state->symbolCache()->getEnvInstance(implType);
        return instanceUrl;
    }

    switch (mode) {
        case ResolveMode::kDefault:
            return logAndContinue(AssemblerCondition::kMissingImpl,
                tempo_tracing::LogSeverity::kError,
                "no instance found implementing {}", implType.toString());
        case ResolveMode::kNoStatusIfMissing:
            return lyric_common::SymbolUrl();
        default:
            throwAssemblerInvariant("invalid resolve mode");
    }
}

inline tempo_utils::Result<lyric_common::TypeDef>
determine_binding_type(
    lyric_assembler::AbstractSymbol *targetSymbol,
    const lyric_common::TypeDef &aliasType,
    lyric_assembler::BlockHandle *block)
{
    if (!aliasType.isValid())
        return targetSymbol->getTypeDef();

    auto *state = block->blockState();
    auto *typeCache = state->typeCache();

    lyric_assembler::TypeHandle *aliasTypeHandle;
    TU_ASSIGN_OR_RETURN (aliasTypeHandle, typeCache->getOrMakeType(aliasType));

    lyric_assembler::TypeSignature targetSig;
    TU_ASSIGN_OR_RETURN (targetSig, typeCache->resolveSignature(targetSymbol->getSymbolUrl()));

    lyric_assembler::TypeSignature aliasSig;
    TU_ASSIGN_OR_RETURN (aliasSig, typeCache->resolveSignature(aliasTypeHandle));

    auto cmp = targetSig.compare(aliasSig);
    if (cmp != lyric_runtime::TypeComparison::EXTENDS)
        block->logAndContinue(lyric_assembler::AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "alias target must extend or equal alias type {}", aliasType.toString());

    return aliasType;
}

inline tempo_utils::Result<lyric_common::TypeDef>
determine_binding_type(
    const lyric_common::SymbolUrl &targetUrl,
    const lyric_common::TypeDef &aliasType,
    lyric_assembler::BlockHandle *block)
{
    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();
    lyric_assembler::AbstractSymbol *targetSymbol;
    TU_ASSIGN_OR_RETURN (targetSymbol, symbolCache->getOrImportSymbol(targetUrl));
    return determine_binding_type(targetSymbol, aliasType, block);
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareAlias(
    const std::string &alias,
    const lyric_common::SymbolUrl &targetUrl,
    const lyric_common::TypeDef &aliasType)
{
    if (m_bindings.contains(alias))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; symbol is already defined", alias);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(targetUrl));

    SymbolBinding binding;

    // set binding type
    switch (symbol->getSymbolType()) {
        case SymbolType::CALL:
        case SymbolType::CLASS:
        case SymbolType::CONCEPT:
        case SymbolType::CONSTANT:
        case SymbolType::ENUM:
        case SymbolType::EXISTENTIAL:
        case SymbolType::INSTANCE:
        case SymbolType::NAMESPACE:
        case SymbolType::STRUCT:
            binding.symbolUrl = targetUrl;
            binding.bindingType = BindingType::Descriptor;
            if (aliasType.isValid())
                return logAndContinue(AssemblerCondition::kIncompatibleType,
                    tempo_tracing::LogSeverity::kError,
                    "cannot declare alias {}; alias type {} is valid for target", alias, aliasType.toString());
            break;

        case SymbolType::STATIC: {
            binding.symbolUrl = targetUrl;
            binding.bindingType = BindingType::Value;
            TU_ASSIGN_OR_RETURN (binding.typeDef, determine_binding_type(symbol, aliasType, this));
            break;
        }

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
    const SymbolBinding &targetBinding,
    const lyric_common::TypeDef &aliasType)
{
    if (m_bindings.contains(alias))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; symbol is already defined", alias);
    TU_RETURN_IF_STATUS (m_state->symbolCache()->getOrImportSymbol(targetBinding.symbolUrl));

    SymbolBinding binding;

    switch (targetBinding.bindingType) {
        case BindingType::Descriptor:
        case BindingType::Placeholder:
            binding.symbolUrl = targetBinding.symbolUrl;
            binding.bindingType = targetBinding.bindingType;
            if (aliasType.isValid())
                return logAndContinue(AssemblerCondition::kIncompatibleType,
                    tempo_tracing::LogSeverity::kError,
                    "cannot declare alias {}; alias type {} is valid for target", alias, aliasType.toString());
            break;

        case BindingType::Variable:
        case BindingType::Value: {
            binding.symbolUrl = targetBinding.symbolUrl;
            binding.bindingType = targetBinding.bindingType;
            TU_ASSIGN_OR_RETURN (binding.typeDef, determine_binding_type(targetBinding.symbolUrl, aliasType, this));
            break;
        }

        default:
            throwAssemblerInvariant("failed to declare alias to {}; invalid binding type",
                targetBinding.symbolUrl.toString());
    }
    binding.typeDef = aliasType.isValid()? aliasType : targetBinding.typeDef;

    m_bindings[alias] = binding;
    return binding;
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareAlias(
    const std::string &alias,
    const DataReference &targetRef,
    const lyric_common::TypeDef &aliasType)
{
    if (m_bindings.contains(alias))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; symbol is already defined", alias);
    TU_RETURN_IF_STATUS (m_state->symbolCache()->getOrImportSymbol(targetRef.symbolUrl));

    SymbolBinding binding;

    // TODO: perform validation here? at least check if target type and alias type are disjoint
    binding.typeDef = aliasType.isValid()? aliasType : targetRef.typeDef;
    switch (targetRef.referenceType) {
        case ReferenceType::Descriptor: {
            binding.symbolUrl = targetRef.symbolUrl;
            binding.bindingType = BindingType::Descriptor;
            if (aliasType.isValid())
                return logAndContinue(AssemblerCondition::kIncompatibleType,
                    tempo_tracing::LogSeverity::kError,
                    "cannot declare alias {}; alias type {} is valid for target", alias, aliasType.toString());
            break;
        }
        case ReferenceType::Value: {
            binding.symbolUrl = targetRef.symbolUrl;
            binding.bindingType = BindingType::Value;
            TU_ASSIGN_OR_RETURN (binding.typeDef, determine_binding_type(targetRef.symbolUrl, aliasType, this));
            break;
        }
        case ReferenceType::Variable: {
            binding.symbolUrl = targetRef.symbolUrl;
            binding.bindingType = BindingType::Variable;
            TU_ASSIGN_OR_RETURN (binding.typeDef, determine_binding_type(targetRef.symbolUrl, aliasType, this));
            break;
        }
        default:
            throwAssemblerInvariant("failed to declare alias to {}; invalid reference type",
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

tempo_utils::Result<lyric_assembler::NamespaceSymbol *>
lyric_assembler::BlockHandle::declareNamespace(
    const std::string &name,
    lyric_object::AccessType access,
    bool declOnly)
{
    if (m_bindings.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare namespace {}; symbol is already defined", name);

    //superNs->touch();
    auto nsUrl = makeSymbolUrl(name);

    // resolve the type
    auto fundamentalNamespace = m_state->fundamentalCache()->getFundamentalUrl(FundamentalSymbol::Namespace);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(fundamentalNamespace));
    auto nsDescriptorType = symbol->getTypeDef();
    lyric_assembler::TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->getOrMakeType(nsDescriptorType));

    auto *superNs = blockNs();

    // create the namespace
    auto *namespaceSymbol = new NamespaceSymbol(nsUrl, access, typeHandle,
        superNs, declOnly, this, m_state, m_isRoot);

    auto status = m_state->appendNamespace(namespaceSymbol);
    if (status.notOk()) {
        delete namespaceSymbol;
        return status;
    }

    SymbolBinding binding;
    binding.symbolUrl = nsUrl;
    binding.typeDef = nsDescriptorType;
    binding.bindingType = BindingType::Descriptor;
    m_bindings[name] = binding;

    return namespaceSymbol;
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