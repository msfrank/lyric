
#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/abstract_importer.h>
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_invoker.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/code_builder.h>
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
    CodeBuilder *blockCode,
    AssemblyState *state,
    bool isRoot)
    : m_blockNs(nullptr),
      m_blockProc(blockProc),
      m_blockCode(blockCode),
      m_parentBlock(nullptr),
      m_state(state),
      m_isRoot(isRoot)
{
    TU_ASSERT (m_blockProc != nullptr);
    TU_ASSERT (m_blockCode != nullptr);
    TU_ASSERT (m_state != nullptr);
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
    m_blockCode = parentBlock->m_blockCode;
    m_state = parentBlock->m_state;
    TU_ASSERT (m_blockProc != nullptr);
    TU_ASSERT (m_blockCode != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::BlockHandle::BlockHandle(
    NamespaceSymbol *blockNs,
    ProcHandle *blockProc,
    CodeBuilder *blockCode,
    BlockHandle *parentBlock,
    AssemblyState *state,
    bool isRoot)
    : m_definition(),
      m_blockNs(blockNs),
      m_blockProc(blockProc),
      m_blockCode(blockCode),
      m_parentBlock(parentBlock),
      m_state(state),
      m_isRoot(isRoot)
{
    TU_ASSERT (m_blockNs != nullptr);
    TU_ASSERT (m_blockProc != nullptr);
    TU_ASSERT (m_blockCode != nullptr);
    TU_ASSERT (m_parentBlock != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::BlockHandle::BlockHandle(
    ProcHandle *blockProc,
    CodeBuilder *blockCode,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : m_blockNs(nullptr),
      m_blockProc(blockProc),
      m_blockCode(blockCode),
      m_parentBlock(parentBlock),
      m_state(state),
      m_isRoot(false)
{
    TU_ASSERT (m_blockProc != nullptr);
    TU_ASSERT (m_blockCode != nullptr);
    TU_ASSERT (m_parentBlock != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::BlockHandle::BlockHandle(
    const absl::flat_hash_map<std::string,SymbolBinding> &initialVars,
    ProcHandle *blockProc,
    CodeBuilder *blockCode,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : m_blockNs(nullptr),
      m_blockProc(blockProc),
      m_blockCode(blockCode),
      m_parentBlock(parentBlock),
      m_state(state),
      m_isRoot(false),
      m_vars(initialVars)
{
    TU_ASSERT (m_blockProc != nullptr);
    TU_ASSERT (m_blockCode != nullptr);
    TU_ASSERT (m_parentBlock != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::BlockHandle::BlockHandle(
    const lyric_common::SymbolUrl &definition,
    const absl::flat_hash_map<std::string,SymbolBinding> &importedVars,
    AssemblyState *state)
    : m_definition(definition),
      m_blockNs(nullptr),
      m_blockProc(nullptr),
      m_blockCode(nullptr),
      m_parentBlock(nullptr),
      m_state(state),
      m_isRoot(false),
      m_vars(importedVars)
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

lyric_assembler::CodeBuilder *
lyric_assembler::BlockHandle::blockCode()
{
    return m_blockCode;
}

lyric_assembler::BlockHandle *
lyric_assembler::BlockHandle::blockParent()
{
    return m_parentBlock;
}

lyric_assembler::AssemblyState *
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
    return m_vars.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::SymbolBinding>::const_iterator
lyric_assembler::BlockHandle::symbolsEnd() const
{
    return m_vars.cend();
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

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::BlockHandle::resolveAssignable(const lyric_parser::Assignable &assignableSpec)
{
    if (assignableSpec.getType() == lyric_parser::AssignableType::SINGULAR)
        return resolveSingular(assignableSpec);

    if (assignableSpec.getType() == lyric_parser::AssignableType::UNION) {
        std::vector<lyric_common::TypeDef> unionMembers;
        for (const auto &memberSpec : assignableSpec.getUnion()) {
            auto resolveMemberResult = resolveSingular(memberSpec);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult;
            unionMembers.push_back(resolveMemberResult.getResult());
        }
        return m_state->typeCache()->resolveUnion(unionMembers);
    }
    if (assignableSpec.getType() == lyric_parser::AssignableType::INTERSECTION) {
        std::vector<lyric_common::TypeDef> intersectionMembers;
        for (const auto &memberSpec : assignableSpec.getIntersection()) {
            auto resolveMemberResult = resolveSingular(memberSpec);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult;
            intersectionMembers.push_back(resolveMemberResult.getResult());
        }
        return m_state->typeCache()->resolveIntersection(intersectionMembers);
    }

    m_state->throwAssemblerInvariant("failed to resolve non singular base type {}", assignableSpec.toString());
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::BlockHandle::resolveSingular(const lyric_parser::Assignable &assignableSpec)      // NOLINT(misc-no-recursion)
{
    if (assignableSpec.getType() != lyric_parser::AssignableType::SINGULAR)
        throwAssemblerInvariant("assignable {} is a non-singular type", assignableSpec.toString());

    // if spec has parameters, then resolve them to types first
    std::vector<lyric_common::TypeDef> typeParameters;
    for (const auto &parameter : assignableSpec.getTypeParameters()) {
        auto resolveParameterResult = resolveAssignable(parameter);
        if (resolveParameterResult.isStatus())
            return resolveParameterResult;
        typeParameters.push_back(resolveParameterResult.getResult());
    }

    // resolve the base to a type
    auto resolveTypePathResult = resolveDefinition(assignableSpec.getTypePath());
    if (resolveTypePathResult.isStatus())
        return resolveTypePathResult.getStatus();

    auto assignableType = lyric_common::TypeDef::forConcrete(
        resolveTypePathResult.getResult(), typeParameters);

    // if type is in the typecache then we're done
    if (m_state->typeCache()->hasType(assignableType))
        return assignableType;

    auto status = m_state->typeCache()->makeType(assignableType);
    if (status.notOk())
        return status;

    return assignableType;
}

static tempo_utils::Result<lyric_common::SymbolUrl>
resolve_definition_tail_recursive(
    lyric_assembler::BlockHandle *block,
    const std::vector<std::string> &path,
    std::vector<std::string>::const_iterator curr,
    std::vector<std::string>::const_iterator end)
{
    if (curr == end || !block->hasBinding(*curr))
        return block->logAndContinue(
            lyric_assembler::AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing definition for symbol {}", lyric_common::SymbolPath(path).toString());

    auto binding = block->getBinding(*curr);

    auto *state = block->blockState();
    auto *symbol = state->symbolCache()->getSymbol(binding.symbol);
    TU_ASSERT (symbol != nullptr);

    if (++curr == end) {
        switch (symbol->getSymbolType()) {
            case lyric_assembler::SymbolType::CALL:
            case lyric_assembler::SymbolType::CLASS:
            case lyric_assembler::SymbolType::CONCEPT:
            case lyric_assembler::SymbolType::ENUM:
            case lyric_assembler::SymbolType::EXISTENTIAL:
            case lyric_assembler::SymbolType::INSTANCE:
            case lyric_assembler::SymbolType::STRUCT:
                return symbol->getSymbolUrl();
            default:
                return block->logAndContinue(
                    lyric_assembler::AssemblerCondition::kMissingSymbol,
                    tempo_tracing::LogSeverity::kError,
                    "missing definition for symbol {}", lyric_common::SymbolPath(path).toString());
        }
    }

    lyric_assembler::BlockHandle *childBlock = nullptr;

    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::NAMESPACE:
            childBlock = cast_symbol_to_namespace(symbol)->namespaceBlock();
            break;
        default:
            break;
    }

    if (childBlock == nullptr)
        return block->logAndContinue(
            lyric_assembler::AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing definition for symbol {}", lyric_common::SymbolPath(path).toString());

    return resolve_definition_tail_recursive(childBlock, path, curr, end);
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::resolveDefinition(const std::vector<std::string> &path)
{
    if (path.empty())
        throwAssemblerInvariant("empty definition");

    auto curr = path.cbegin();

    // walk the chain of blocks to the root, checking for type
    for (auto *block = this; block != nullptr; block = block->blockParent()) {
        if (!block->hasBinding(*curr))
            continue;
        return resolve_definition_tail_recursive(block, path, curr, path.cend());
    }

    // if definition is not found in any reachable block, then check env
    if (m_state->symbolCache()->hasEnvBinding(*curr)) {
        auto binding = m_state->symbolCache()->getEnvBinding(*curr);
        auto *symbol = m_state->symbolCache()->getSymbol(binding.symbol);
        TU_ASSERT (symbol != nullptr);

        if (++curr == path.cend()) {
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
                        "missing definition for symbol {}", lyric_common::SymbolPath(path).toString());
            }
        }

        BlockHandle *block;
        switch (symbol->getSymbolType()) {
            case lyric_assembler::SymbolType::NAMESPACE:
                block = cast_symbol_to_namespace(symbol)->namespaceBlock();
                break;
            default:
                return logAndContinue(
                    lyric_assembler::AssemblerCondition::kMissingSymbol,
                    tempo_tracing::LogSeverity::kError,
                    "missing definition for symbol {}", lyric_common::SymbolPath(path).toString());
        }

        return resolve_definition_tail_recursive(block, path, curr, path.cend());
    }

    return logAndContinue(
        lyric_assembler::AssemblerCondition::kMissingSymbol,
        tempo_tracing::LogSeverity::kError,
        "missing definition for symbol {}", lyric_common::SymbolPath(path).toString());
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

//tempo_utils::Result<lyric_common::SymbolUrl>
//lyric_assembler::BlockHandle::resolveDefinition(const lyric_runtime::LiteralCell &literalCell)
//{
//    return m_state->fundamentalCache()->getFundamentalUrl(literalCell);
//}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareVariable(
    const std::string &name,
    const lyric_common::TypeDef &assignableType,
    lyric_parser::BindingType binding)
{
    if (m_vars.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare variable {}; symbol is already defined", name);

    // make type handle if it doesn't exist already (for example union or intersection types)
    auto status = m_state->typeCache()->makeType(assignableType);
    if (!status.isOk())
        return status;

    // if this is a root block then create a static variable, otherwise create a local
    auto localUrl = makeSymbolUrl(name);
    auto address = m_blockProc->allocateLocal();
    auto *localVariable = new LocalVariable(localUrl, assignableType, address);
    status = m_state->symbolCache()->insertSymbol(localUrl, localVariable);
    if (status.notOk()) {
        delete localVariable;
        return status;
    }

    SymbolBinding var;
    var.symbol = localUrl;
    var.type = assignableType;
    var.binding = binding;
    m_vars[name] = var;
    return var;
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareTemporary(const lyric_common::TypeDef &assignableType, lyric_parser::BindingType binding)
{
    auto name = absl::StrCat("$tmp", m_blockProc->numLocals());

    if (m_vars.contains(name))
        throwAssemblerInvariant("cannot declare temporary {}; symbol is is already defined", name);

    // make type handle if it doesn't exist already (for example union or intersection types)
    auto status = m_state->typeCache()->makeType(assignableType);
    if (!status.isOk())
        return status;

    SymbolBinding var;

    // temporary variables are always local
    auto localUrl = makeSymbolUrl(name);
    auto address = m_blockProc->allocateLocal();
    auto *localVariable = new LocalVariable(localUrl, assignableType, address);
    status = m_state->symbolCache()->insertSymbol(localUrl, localVariable);
    if (status.notOk()) {
        delete localVariable;
        return status;
    }

    var.symbol = localUrl;
    var.type = assignableType;
    var.binding = binding;
    m_vars[name] = var;
    return var;
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareStatic(
    const std::string &name,
    const lyric_common::TypeDef &assignableType,
    lyric_parser::BindingType binding,
    bool declOnly)
{
    if (m_vars.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare static {}; symbol is already defined", name);

    // make type handle if it doesn't exist already (for example union or intersection types)
    auto status = m_state->typeCache()->makeType(assignableType);
    if (!status.isOk())
        return status;

    auto *staticType = m_state->typeCache()->getType(assignableType);
    staticType->touch();

    auto staticUrl = makeSymbolUrl(name);

    bool isVariable = binding == lyric_parser::BindingType::VARIABLE? true : false;
    StaticAddress address;
    if (!declOnly)
        address = StaticAddress::near(m_state->numStatics());
    auto *staticSymbol = new StaticSymbol(staticUrl, isVariable, address, staticType, this, m_state);
    status = m_state->appendStatic(staticSymbol);
    if (status.notOk()) {
        delete staticSymbol;
        return status;
    }

    SymbolBinding var;
    var.symbol = staticUrl;
    var.type = assignableType;
    var.binding = binding;
    m_vars[name] = var;
    return var;
}

bool
lyric_assembler::BlockHandle::hasBinding(const std::string &name) const
{
    return m_vars.contains(name);
}

lyric_assembler::SymbolBinding
lyric_assembler::BlockHandle::getBinding(const std::string &name) const
{
    if (m_vars.contains(name))
        return m_vars.at(name);
    return {};
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::resolveBinding(const std::string &name)
{
    BlockHandle *block = this;

    // if variable exists in a parent block in the current proc, then return it
    for (; block != nullptr && block->m_blockProc == m_blockProc; block = block->m_parentBlock) {
        if (block->m_vars.contains(name)) {
            const auto &binding = block->m_vars[name];
            auto *symbol = m_state->symbolCache()->getSymbol(binding.symbol);
            TU_ASSERT (symbol != nullptr);

            switch (symbol->getSymbolType()) {
                case SymbolType::EXISTENTIAL:
                case SymbolType::ARGUMENT:
                case SymbolType::LOCAL:
                case SymbolType::LEXICAL:
                case SymbolType::CONSTANT:
                case SymbolType::STATIC:
                case SymbolType::INSTANCE:
                case SymbolType::SYNTHETIC:
                case SymbolType::CALL:
                case SymbolType::CLASS:
                case SymbolType::CONCEPT:
                case SymbolType::STRUCT:
                case SymbolType::ENUM:
                case SymbolType::NAMESPACE:
                    return binding;
                default:
                    return logAndContinue(lyric_assembler::AssemblerCondition::kMissingVariable,
                        tempo_tracing::LogSeverity::kError,
                        "missing variable {}", name);
            }
        }
    }

    // if variable exists in a parent proc, then import lexical into this proc
    for (; block != nullptr; block = block->m_parentBlock) {
        if (block->m_vars.contains(name)) {
            const auto &binding = block->m_vars[name];
            auto *symbol = m_state->symbolCache()->getSymbol(binding.symbol);
            TU_ASSERT (symbol != nullptr);

            lyric_assembler::LexicalTarget lexicalTarget;
            uint32_t targetOffset = lyric_runtime::INVALID_ADDRESS_U32;
            lyric_common::TypeDef assignableType;

            switch (symbol->getSymbolType()) {
                // found a local in a parent proc
                case SymbolType::LOCAL: {
                    auto *local = static_cast<LocalVariable *>(symbol);
                    lexicalTarget = lyric_assembler::LexicalTarget::Local;
                    targetOffset = local->getOffset().getOffset();
                    assignableType = binding.type;
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
                case SymbolType::STATIC:
                    return binding;

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
            auto *activationSym = m_state->symbolCache()->getSymbol(activation);
            TU_ASSERT (activationSym->getSymbolType() == SymbolType::CALL);
            auto activationAddress = cast_symbol_to_call(activationSym)->getAddress();

            // import lexical variable into this env, and return it
            auto lexicalUrl = makeSymbolUrl(name);
            auto address = m_blockProc->allocateLexical(lexicalTarget, targetOffset, activationAddress);
            auto *lexicalVariable = new LexicalVariable(lexicalUrl, assignableType, address);
            m_state->symbolCache()->insertSymbol(lexicalUrl, lexicalVariable);

            SymbolBinding var;
            var.symbol = lexicalUrl;
            var.type = assignableType;
            var.binding = lyric_parser::BindingType::VALUE;
            m_vars[name] = var;

            return var;
        }
    }

    // if variable is not found in any reachable block, then check env
    if (m_state->symbolCache()->hasEnvBinding(name)) {
        const auto binding = m_state->symbolCache()->getEnvBinding(name);
        auto *symbol = m_state->symbolCache()->getSymbol(binding.symbol);
        TU_ASSERT (symbol != nullptr);

        switch (symbol->getSymbolType()) {
            case SymbolType::EXISTENTIAL:
            case SymbolType::ARGUMENT:
            case SymbolType::LOCAL:
            case SymbolType::LEXICAL:
            case SymbolType::CONSTANT:
            case SymbolType::STATIC:
            case SymbolType::INSTANCE:
            case SymbolType::SYNTHETIC:
            case SymbolType::CALL:
            case SymbolType::CLASS:
            case SymbolType::CONCEPT:
            case SymbolType::STRUCT:
            case SymbolType::ENUM:
            case SymbolType::NAMESPACE:
                return binding;
            default:
                return logAndContinue(lyric_assembler::AssemblerCondition::kMissingVariable,
                    tempo_tracing::LogSeverity::kError,
                    "missing variable {}", name);
        }
    }

    // we have exhausted our search, variable is missing
    return logAndContinue(lyric_assembler::AssemblerCondition::kMissingVariable,
        tempo_tracing::LogSeverity::kError,
        "missing variable {}", name);
}

tempo_utils::Status
lyric_assembler::BlockHandle::load(const SymbolBinding &binding)
{
    auto *symbol = m_state->symbolCache()->getSymbol(binding.symbol);
    TU_ASSERT (symbol != nullptr);
    symbol->touch();
    switch (symbol->getSymbolType()) {
        case SymbolType::ARGUMENT:
            return m_blockCode->loadArgument(cast_symbol_to_argument(symbol)->getOffset());
        case SymbolType::LOCAL:
            return m_blockCode->loadLocal(cast_symbol_to_local(symbol)->getOffset());
        case SymbolType::LEXICAL:
            return m_blockCode->loadLexical(cast_symbol_to_lexical(symbol)->getOffset());
        case SymbolType::FIELD:
            return m_blockCode->loadField(cast_symbol_to_field(symbol)->getAddress());
        case SymbolType::STATIC:
            return m_blockCode->loadStatic(cast_symbol_to_static(symbol)->getAddress());
        case SymbolType::INSTANCE:
            return m_blockCode->loadInstance(cast_symbol_to_instance(symbol)->getAddress());
        case SymbolType::ENUM:
            return m_blockCode->loadEnum(cast_symbol_to_enum(symbol)->getAddress());
        case SymbolType::CALL:
            return m_blockCode->loadCall(cast_symbol_to_call(symbol)->getAddress());
        case SymbolType::CLASS:
            return m_blockCode->loadClass(cast_symbol_to_class(symbol)->getAddress());
        case SymbolType::CONCEPT:
            return m_blockCode->loadConcept(cast_symbol_to_concept(symbol)->getAddress());
        case SymbolType::STRUCT:
            return m_blockCode->loadStruct(cast_symbol_to_struct(symbol)->getAddress());
        case SymbolType::SYNTHETIC:
            return m_blockCode->loadSynthetic(cast_symbol_to_synthetic(symbol)->getSyntheticType());
        default:
            break;
    }
    return logAndContinue(AssemblerCondition::kInvalidBinding,
        tempo_tracing::LogSeverity::kError,
        "binding for symbol {} cannot be loaded from", binding.symbol.toString());
}

tempo_utils::Status
lyric_assembler::BlockHandle::store(const SymbolBinding &binding)
{
    auto *symbol = m_state->symbolCache()->getSymbol(binding.symbol);
    TU_ASSERT (symbol != nullptr);
    symbol->touch();
    switch (symbol->getSymbolType()) {
        case SymbolType::ARGUMENT:
            return m_blockCode->storeArgument(cast_symbol_to_argument(symbol)->getOffset());
        case SymbolType::LOCAL:
            return m_blockCode->storeLocal(cast_symbol_to_local(symbol)->getOffset());
        case SymbolType::LEXICAL:
            return m_blockCode->storeLexical(cast_symbol_to_lexical(symbol)->getOffset());
        case SymbolType::FIELD:
            return m_blockCode->storeField(cast_symbol_to_field(symbol)->getAddress());
        case SymbolType::STATIC:
            return m_blockCode->storeStatic(cast_symbol_to_static(symbol)->getAddress());
        default:
            break;
    }
    return logAndContinue(AssemblerCondition::kInvalidBinding,
        tempo_tracing::LogSeverity::kError,
        "binding for symbol {} cannot be stored to", binding.symbol.toString());
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::declareFunction(
    const std::string &name,
    const std::vector<lyric_assembler::ParameterSpec> &parameterSpec,
    const Option<lyric_assembler::ParameterSpec> &restSpec,
    const std::vector<lyric_assembler::ParameterSpec> &ctxSpec,
    const lyric_parser::Assignable &returnSpec,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    bool declOnly)
{
    if (m_vars.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare function {}; symbol is already defined", name);

    auto functionUrl = makeSymbolUrl(name);

    // create the template if there are any template parameters
    TemplateHandle *functionTemplate = nullptr;
    if (!templateParameters.empty()) {
        auto status = m_state->typeCache()->makeTemplate(functionUrl, templateParameters, this);
        if (!status.isOk())
            return status;
        functionTemplate = m_state->typeCache()->getTemplate(functionUrl);
    }

    AbstractResolver *resolver = functionTemplate? (AbstractResolver *) functionTemplate : this;

    std::vector<lyric_object::Parameter> parameters;
    Option<lyric_object::Parameter> rest;
    absl::flat_hash_set<std::string> names;
    absl::flat_hash_set<std::string> labels;

    for (const auto &p : parameterSpec) {
        auto resolveParamTypeResult = resolver->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = !p.label.empty()? p.label : p.name;
        param.placement = !p.label.empty()? lyric_object::PlacementType::Named : lyric_object::PlacementType::List;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveParamTypeResult.getResult();

        if (!p.init.isEmpty()) {
            if (param.placement != lyric_object::PlacementType::Named) {
                return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid initializer for positional parameter {}; only named parameters can be default-initialized",
                    p.name);
            } else {
                param.placement = lyric_object::PlacementType::Opt;
            }
        }

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for function {}",
                p.name, name);
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for function {}",
                p.label, name);
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    for (const auto &p : ctxSpec) {
        auto resolveParamTypeResult = resolver->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.placement = lyric_object::PlacementType::Ctx;
        param.isVariable = false;
        param.typeDef = resolveParamTypeResult.getResult();

        // if ctx parameter name is not specified, then generate a unique name
        param.name = p.name.empty()? absl::StrCat("$ctx", parameters.size()) : p.name;
        param.label = param.name;

        if (names.contains(param.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for function {}",
                p.name, name);
        names.insert(param.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for function {}",
                p.label, name);
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    if (!restSpec.isEmpty()) {
        const auto &p = restSpec.getValue();
        auto resolveRestTypeResult = resolver->resolveAssignable(p.type);
        if (resolveRestTypeResult.isStatus())
            return resolveRestTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = param.name;
        param.placement = lyric_object::PlacementType::Rest;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveRestTypeResult.getResult();

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for function {}",
                p.name, name);
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for function {}",
                p.label, name);
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        rest = Option<lyric_object::Parameter>(param);
    }

    auto resolveReturnTypeResult = resolver->resolveAssignable(returnSpec);
    if (resolveReturnTypeResult.isStatus())
        return resolveReturnTypeResult.getStatus();
    auto returnType = resolveReturnTypeResult.getResult();
    m_state->typeCache()->touchType(returnType);

    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareFunctionType(returnType, parameters, rest));

    CallAddress address;
    if (!declOnly)
        address = CallAddress::near(m_state->numCalls());

    // create the call
    CallSymbol *callSymbol;
    if (functionTemplate) {
        callSymbol = new CallSymbol(functionUrl, parameters, rest, returnType, access, address,
            lyric_object::CallMode::Normal, typeHandle, functionTemplate, functionTemplate->parentBlock(), m_state);
    } else {
        callSymbol = new CallSymbol(functionUrl, parameters, rest, returnType, access, address,
            lyric_object::CallMode::Normal, typeHandle, this, m_state);
    }

    auto status = m_state->appendCall(callSymbol);
    if (status.notOk()) {
        delete callSymbol;
        return status;
    }

    SymbolBinding ref;
    ref.symbol = functionUrl;
    ref.type = typeHandle->getTypeDef();
    ref.binding = lyric_parser::BindingType::DESCRIPTOR;
    m_vars[name] = ref;

    return functionUrl;
}

tempo_utils::Result<lyric_assembler::CallInvoker>
lyric_assembler::BlockHandle::resolveFunction(const std::string &name)
{
    auto resolveDefinitionResult = resolveDefinition({name});
    if (resolveDefinitionResult.isStatus())
        return resolveDefinitionResult.getStatus();

    auto functionUrl = resolveDefinitionResult.getResult();
    auto *sym = m_state->symbolCache()->getSymbol(functionUrl);
    if (sym == nullptr)
        throwAssemblerInvariant("missing call symbol {}", functionUrl.toString());
    if (sym->getSymbolType() != SymbolType::CALL)
        throwAssemblerInvariant("invalid call symbol {}", functionUrl.toString());
    auto *call = cast_symbol_to_call(sym);

    return CallInvoker(call);
}

tempo_utils::Result<lyric_assembler::CallInvoker>
lyric_assembler::BlockHandle::resolveExtension(const lyric_common::TypeDef &receiverType, const std::string &name)
{
//    auto resolveDefinitionResult = resolveBinding(QStringList(name));
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

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::declareStruct(
    const std::string &name,
    StructSymbol *superStruct,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    bool declOnly)
{
    if (m_vars.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare struct {}; symbol is already defined", name);

    superStruct->touch();

    auto superDerive = superStruct->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive struct {} from {}; base struct is marked final",
            name, superStruct->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && lyric_object::IS_FAR(superStruct->getAddress().getAddress()))
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive struct {} from {}; sealed base struct must be located in the same module",
            name, superStruct->getSymbolUrl().toString());

    auto structUrl = makeSymbolUrl(name);

    // create the type
    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareSubType(
        structUrl, {}, superStruct->getAssignableType()));

    StructAddress address;
    if (!declOnly)
        address = StructAddress::near(m_state->numStructs());

    // create the struct
    auto *structSymbol = new StructSymbol(structUrl, access, derive, isAbstract, address,
        typeHandle, superStruct, this, m_state);

    auto status = m_state->appendStruct(structSymbol);
    if (status.notOk()) {
        delete structSymbol;
        return status;
    }

    SymbolBinding ref;
    ref.symbol = structUrl;
    ref.type = structSymbol->getAssignableType();
    ref.binding = lyric_parser::BindingType::DESCRIPTOR;
    m_vars[name] = ref;

    return structUrl;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::resolveStruct(const lyric_parser::Assignable &structSpec)
{
    if (structSpec.getType() != lyric_parser::AssignableType::SINGULAR)
        return logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "invalid struct type {}", structSpec.toString());

    auto resolveDefinitionResult = resolveDefinition(structSpec.getTypePath());
    if (resolveDefinitionResult.isStatus())
        return resolveDefinitionResult.getStatus();

    auto structUrl = resolveDefinitionResult.getResult();
    auto *sym = m_state->symbolCache()->getSymbol(structUrl);
    if (sym == nullptr)
        throwAssemblerInvariant("missing struct symbol {}", structUrl.toString());
    if (sym->getSymbolType() != SymbolType::STRUCT)
        return logAndContinue(AssemblerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "type {} is not a struct", structSpec.toString());

    return structUrl;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::declareClass(
    const std::string &name,
    ClassSymbol *superClass,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    lyric_object::DeriveType derive,
    bool isAbstract,
    bool declOnly)
{
    if (m_vars.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare class {}; symbol is already defined", name);

    superClass->touch();

    auto superDerive = superClass->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive class {} from {}; base class is marked final",
            name, superClass->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && lyric_object::IS_FAR(superClass->getAddress().getAddress()))
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive class {} from {}; sealed base class must be located in the same module",
            name, superClass->getSymbolUrl().toString());

    auto classUrl = makeSymbolUrl(name);

    // create the template if there are any template parameters
    TemplateHandle *classTemplate = nullptr;
    if (!templateParameters.empty()) {
        auto status = m_state->typeCache()->makeTemplate(classUrl, templateParameters, this);
        if (!status.isOk())
            return status;
        classTemplate = m_state->typeCache()->getTemplate(classUrl);
    }

   // create the type
    TypeHandle *typeHandle;
    if (classTemplate) {
        TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareSubType(
            classUrl, classTemplate->getPlaceholders(), superClass->getAssignableType()));
    } else {
        TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareSubType(
            classUrl, {}, superClass->getAssignableType()));
    }

    ClassAddress address;
    if (!declOnly)
        address = ClassAddress::near(m_state->numClasses());

    // create the class
    ClassSymbol *classSymbol;
    if (classTemplate) {
        classSymbol = new ClassSymbol(classUrl, access, derive, isAbstract, address, typeHandle,
            classTemplate, superClass, classTemplate->parentBlock(), m_state);
    } else {
        classSymbol = new ClassSymbol(classUrl, access, derive, isAbstract, address,
            typeHandle, superClass, this, m_state);
    }

    auto status = m_state->appendClass(classSymbol);
    if (status.notOk()) {
        delete classSymbol;
        return status;
    }

    SymbolBinding ref;
    ref.symbol = classUrl;
    ref.type = classSymbol->getAssignableType();
    ref.binding = lyric_parser::BindingType::DESCRIPTOR;
    m_vars[name] = ref;

    return classUrl;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::resolveClass(const lyric_parser::Assignable &classSpec)
{
    if (classSpec.getType() != lyric_parser::AssignableType::SINGULAR)
        return logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "invalid class type {}", classSpec.toString());

    auto resolveDefinitionResult = resolveDefinition(classSpec.getTypePath());
    if (resolveDefinitionResult.isStatus())
        return resolveDefinitionResult.getStatus();

    auto classUrl = resolveDefinitionResult.getResult();
    auto *sym = m_state->symbolCache()->getSymbol(classUrl);
    if (sym == nullptr)
        throwAssemblerInvariant("missing class symbol {}", classUrl.toString());
    if (sym->getSymbolType() != SymbolType::CLASS)
        return logAndContinue(AssemblerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "type {} is not a class", classSpec.toString());

    return classUrl;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::declareConcept(
    const std::string &name,
    ConceptSymbol *superConcept,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    lyric_object::DeriveType derive,
    bool declOnly)
{
    if (m_vars.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare concept {}; symbol is already defined", name);

    superConcept->touch();

    auto conceptUrl = makeSymbolUrl(name);

    // create the template if there are any template parameters
    TemplateHandle *conceptTemplate = nullptr;
    if (!templateParameters.empty()) {
        auto status = m_state->typeCache()->makeTemplate(conceptUrl, templateParameters, this);
        if (!status.isOk())
            return status;
        conceptTemplate = m_state->typeCache()->getTemplate(conceptUrl);
    }

    // create the type
    TypeHandle *typeHandle;
    if (conceptTemplate) {
        TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareSubType(
            conceptUrl, conceptTemplate->getPlaceholders(), superConcept->getAssignableType()));
    } else {
        TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareSubType(
            conceptUrl, {}, superConcept->getAssignableType()));
    }

    ConceptAddress address;
    if (!declOnly)
        address = ConceptAddress::near(m_state->numConcepts());

    // create the concept
    ConceptSymbol *conceptSymbol;
    if (conceptTemplate) {
        conceptSymbol = new ConceptSymbol(conceptUrl, access, derive, address, typeHandle,
            conceptTemplate, superConcept, this, m_state);
    } else {
        conceptSymbol = new ConceptSymbol(conceptUrl, access, derive, address, typeHandle,
            superConcept, this, m_state);
    }

    auto status = m_state->appendConcept(conceptSymbol);
    if (status.notOk()) {
        delete conceptSymbol;
        return status;
    }

    SymbolBinding ref;
    ref.symbol = conceptUrl;
    ref.type = conceptSymbol->getAssignableType();
    ref.binding = lyric_parser::BindingType::DESCRIPTOR;
    m_vars[name] = ref;

    return conceptUrl;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::resolveConcept(const lyric_parser::Assignable &conceptSpec)
{
    if (conceptSpec.getType() != lyric_parser::AssignableType::SINGULAR)
        return logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "invalid concept type {}", conceptSpec.toString());

    auto resolveDefinitionResult = resolveDefinition(conceptSpec.getTypePath());
    if (resolveDefinitionResult.isStatus())
        return resolveDefinitionResult.getStatus();

    auto conceptUrl = resolveDefinitionResult.getResult();
    auto *sym = m_state->symbolCache()->getSymbol(conceptUrl);
    if (sym == nullptr)
        throwAssemblerInvariant("missing concept symbol {}", conceptUrl.toString());
    if (sym->getSymbolType() != SymbolType::CONCEPT)
        return logAndContinue(AssemblerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "type {} is not a concept", conceptSpec.toString());

    return conceptUrl;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::declareEnum(
    const std::string &name,
    EnumSymbol *superEnum,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    bool declOnly)
{
    if (m_vars.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare enum {}; symbol is already defined", name);

    superEnum->touch();

    auto superDerive = superEnum->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive enum {} from {}; base enum is marked final",
            name, superEnum->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && lyric_object::IS_FAR(superEnum->getAddress().getAddress()))
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive enum {} from {}; sealed base enum must be located in the same module",
            name, superEnum->getSymbolUrl().toString());

    auto enumUrl = makeSymbolUrl(name);

    // create the type
    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareSubType(
        enumUrl, {}, superEnum->getAssignableType()));

    EnumAddress address;
    if (!declOnly)
        address = EnumAddress::near(m_state->numEnums());

    // create the instance
    auto *enumSymbol = new EnumSymbol(enumUrl, access, derive, isAbstract, address, typeHandle,
        superEnum, this, m_state);

    auto status = m_state->appendEnum(enumSymbol);
    if (status.notOk()) {
        delete enumSymbol;
        return status;
    }

    SymbolBinding ref;
    ref.symbol = enumUrl;
    ref.type = enumSymbol->getAssignableType();
    ref.binding = lyric_parser::BindingType::DESCRIPTOR;
    m_vars[name] = ref;

    return enumUrl;
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::resolveEnum(const lyric_parser::Assignable &enumSpec)
{
    if (enumSpec.getType() != lyric_parser::AssignableType::SINGULAR)
        return logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "invalid enum type {}", enumSpec.toString());

    auto resolveDefinitionResult = resolveDefinition(enumSpec.getTypePath());
    if (resolveDefinitionResult.isStatus())
        return resolveDefinitionResult.getStatus();

    auto enumUrl = resolveDefinitionResult.getResult();
    auto *sym = m_state->symbolCache()->getSymbol(enumUrl);
    if (sym == nullptr)
        throwAssemblerInvariant("missing enum symbol {}", enumSpec.toString());
    if (sym->getSymbolType() != SymbolType::ENUM)
        return logAndContinue(AssemblerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "type {} is not an enum", enumSpec.toString());
    auto *enumSymbol = cast_symbol_to_enum(sym);

    return SymbolBinding(enumUrl, enumSymbol->getAssignableType(), lyric_parser::BindingType::VALUE);
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::declareInstance(
    const std::string &name,
    InstanceSymbol *superInstance,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    bool declOnly)
{
    if (m_vars.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare instance {}; symbol is already defined", name);

    superInstance->touch();

    auto superDerive = superInstance->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive instance {} from {}; base instance is marked final",
            name, superInstance->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && lyric_object::IS_FAR(superInstance->getAddress().getAddress()))
        return logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "cannot derive instance {} from {}; sealed base instance must be located in the same module",
            name, superInstance->getSymbolUrl().toString());

    auto instanceUrl = makeSymbolUrl(name);

    // create the type
    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, m_state->typeCache()->declareSubType(
        instanceUrl, {}, superInstance->getAssignableType()));

    InstanceAddress address;
    if (!declOnly)
        address = InstanceAddress::near(m_state->numInstances());

    // create the instance
    auto *instanceSymbol = new InstanceSymbol(instanceUrl, access, derive, isAbstract, address, typeHandle,
        superInstance, this, m_state);

    auto status = m_state->appendInstance(instanceSymbol);
    if (status.notOk()) {
        delete instanceSymbol;
        return status;
    }

    SymbolBinding ref;
    ref.symbol = instanceUrl;
    ref.type = instanceSymbol->getAssignableType();
    ref.binding = lyric_parser::BindingType::DESCRIPTOR;
    m_vars[name] = ref;

    return instanceUrl;
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::resolveInstance(const lyric_parser::Assignable &instanceSpec)
{
    if (instanceSpec.getType() != lyric_parser::AssignableType::SINGULAR)
        return logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "invalid instance type {}", instanceSpec.toString());

    auto resolveDefinitionResult = resolveDefinition(instanceSpec.getTypePath());
    if (resolveDefinitionResult.isStatus())
        return resolveDefinitionResult.getStatus();

    auto instanceUrl = resolveDefinitionResult.getResult();
    auto *sym = m_state->symbolCache()->getSymbol(instanceUrl);
    if (sym == nullptr)
        throwAssemblerInvariant("missing instance symbol {}", instanceSpec.toString());
    if (sym->getSymbolType() != SymbolType::INSTANCE)
        return logAndContinue(AssemblerCondition::kInvalidSymbol,
            tempo_tracing::LogSeverity::kError,
            "type {} is not an instance", instanceSpec.toString());
    auto *instanceSymbol = cast_symbol_to_instance(sym);

    return SymbolBinding(instanceUrl, instanceSymbol->getAssignableType(), lyric_parser::BindingType::VALUE);
}

tempo_utils::Status
lyric_assembler::BlockHandle::useSymbol(
    const lyric_common::SymbolUrl &symbolUrl,
    const absl::flat_hash_set<lyric_common::TypeDef> &implTypes)
{
    auto *sym = m_state->symbolCache()->getSymbol(symbolUrl);
    if (sym == nullptr)
        return logAndContinue(AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", symbolUrl.toString());

    absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *>::const_iterator implsBegin;
    absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *>::const_iterator implsEnd;

    switch (sym->getSymbolType()) {
        case SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(sym);
            implsBegin = classSymbol->implsBegin();
            implsEnd = classSymbol->implsEnd();
            break;
        }
        case SymbolType::CONCEPT: {
            auto *conceptSymbol = cast_symbol_to_concept(sym);
            implsBegin = conceptSymbol->implsBegin();
            implsEnd = conceptSymbol->implsEnd();
            break;
        }
        case SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(sym);
            implsBegin = enumSymbol->implsBegin();
            implsEnd = enumSymbol->implsEnd();
            break;
        }
        case SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(sym);
            implsBegin = existentialSymbol->implsBegin();
            implsEnd = existentialSymbol->implsEnd();
            break;
        }
        case SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(sym);
            implsBegin = instanceSymbol->implsBegin();
            implsEnd = instanceSymbol->implsEnd();
            break;
        }
        case SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(sym);
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

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareAlias(
    const std::string &alias,
    const lyric_common::SymbolUrl &target,
    const lyric_common::TypeDef &aliasType)
{
    if (m_vars.contains(alias))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; symbol is already defined", alias);
    if (!m_state->symbolCache()->hasSymbol(target))
        return logAndContinue(AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; missing symbol {}", alias, target.toString());
    auto *symbol = m_state->symbolCache()->getSymbol(target);

    SymbolBinding var;
    var.symbol = target;
    switch (symbol->getSymbolType()) {
        default:
            var.binding = lyric_parser::BindingType::DESCRIPTOR;
            break;
    }

    if (aliasType.isValid()) {
        // if aliasType is specified, then use it for the binding
        // FIXME: can we perform any validation here? at least check if symbol type and alias type are disjoint
        var.type = aliasType;
    } else {
        // otherwise use the declared assignable type of the symbol
        switch (symbol->getSymbolType()) {
            case SymbolType::CALL:
            case SymbolType::CLASS:
            case SymbolType::CONCEPT:
            case SymbolType::INSTANCE:
            case SymbolType::STATIC:
            case SymbolType::STRUCT:
            case SymbolType::NAMESPACE:
                var.type = symbol->getAssignableType();
                break;
            default:
                return logAndContinue(AssemblerCondition::kMissingSymbol,
                    tempo_tracing::LogSeverity::kError,
                    "cannot declare alias {}; {} is not a valid target", alias, target.toString());
        }
    }

    m_vars[alias] = var;
    return var;
}

tempo_utils::Result<lyric_assembler::SymbolBinding>
lyric_assembler::BlockHandle::declareAlias(
    const std::string &alias,
    const SymbolBinding &target,
    const lyric_common::TypeDef &aliasType)
{
    if (m_vars.contains(alias))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; symbol is already defined", alias);
    if (!m_state->symbolCache()->hasSymbol(target.symbol))
        return logAndContinue(AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "cannot declare alias {}; missing symbol {}", alias, target.symbol.toString());

    SymbolBinding var;
    var.symbol = target.symbol;
    var.binding = lyric_parser::BindingType::VALUE;
    // FIXME: can we perform any validation here? at least check if target type and alias type are disjoint
    var.type = aliasType.isValid()? aliasType : target.type;
    m_vars[alias] = var;

    return var;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::BlockHandle::declareNamespace(
    const std::string &name,
    lyric_object::AccessType access)
{
    if (m_vars.contains(name))
        return logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "cannot declare namespace {}; symbol is already defined", name);

    //superNs->touch();
    auto nsUrl = makeSymbolUrl(name);

    // resolve the type
    auto fundamentalNamespace = m_state->fundamentalCache()->getFundamentalUrl(FundamentalSymbol::Namespace);
    if (!m_state->symbolCache()->hasSymbol(fundamentalNamespace))
        throwAssemblerInvariant("missing fundamental symbol Namespace");
    auto *nsDescriptor = m_state->symbolCache()->getSymbol(fundamentalNamespace);
    auto nsDescriptorType = nsDescriptor->getAssignableType();
    if (!m_state->typeCache()->hasType(nsDescriptorType))
        throwAssemblerInvariant("missing type for {}", fundamentalNamespace.toString());
    auto *typeHandle = m_state->typeCache()->getType(nsDescriptorType);

    auto address = NamespaceAddress::near(m_state->numNamespaces());
    auto *superNs = blockNs();

    // create the namespace
    auto *namespaceSymbol = new NamespaceSymbol(nsUrl, access, address, typeHandle,
        superNs, this, m_state, m_isRoot);

    auto status = m_state->appendNamespace(namespaceSymbol);
    if (status.notOk()) {
        delete namespaceSymbol;
        return status;
    }

    SymbolBinding ref;
    ref.symbol = nsUrl;
    ref.type = namespaceSymbol->getAssignableType();
    ref.binding = lyric_parser::BindingType::DESCRIPTOR;
    m_vars[name] = ref;

    return nsUrl;
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