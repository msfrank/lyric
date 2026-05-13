
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/symbol_cache.h>

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/local_variable.h>
#include <lyric_assembler/struct_symbol.h>

lyric_assembler::ImplCache::ImplCache(ObjectState *objectState)
    : m_objectState(objectState)
{
    TU_ASSERT (m_objectState != nullptr);
}

lyric_assembler::ImplCache::~ImplCache()
{
    for (auto *implHandle : m_importedImpls) {
        delete implHandle;
    }
    for (auto *implHandle : m_declaredImpls) {
        delete implHandle;
    }
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ImplCache::makeImpl(
    const lyric_common::TypeDef &consumerType,
    const lyric_common::SymbolUrl &receiverUrl,
    bool isDeclOnly,
    BlockHandle *parentBlock)
{
    auto name = absl::StrCat("$impl", m_declaredImpls.size());
    auto *implHandle = new ImplHandle(name, consumerType, receiverUrl, isDeclOnly,
        parentBlock, m_objectState);
    m_declaredImpls.push_back(implHandle);
    return implHandle;
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ImplCache::makeImpl(
    const lyric_common::TypeDef &consumerType,
    const lyric_common::SymbolUrl &receiverUrl,
    TemplateHandle *receiverTemplate,
    bool isDeclOnly,
    BlockHandle *parentBlock)
{
    auto name = absl::StrCat("$impl", m_declaredImpls.size());
    auto *implHandle = new ImplHandle(name, consumerType, receiverUrl, receiverTemplate, isDeclOnly,
        parentBlock, m_objectState);
    m_declaredImpls.push_back(implHandle);
    return implHandle;
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ImplCache::importImpl(std::shared_ptr<lyric_importer::ImplImport> implImport)
{
    TU_ASSERT (implImport != nullptr);
    auto *implHandle = new ImplHandle(implImport, m_objectState);
    m_importedImpls.push_back(implHandle);
    return implHandle;
}

std::vector<lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ImplCache::implsBegin() const
{
    return m_declaredImpls.cbegin();
}

std::vector<lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ImplCache::implsEnd() const
{
    return m_declaredImpls.cend();
}

int
lyric_assembler::ImplCache::numImpls() const
{
    return m_declaredImpls.size();
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ImplCache::getOrImportImpl(
    const lyric_common::SymbolUrl &symbolUrl,
    const lyric_common::TypeDef &implType,
    bool allowMissing) const
{
    auto *symbolCache = m_objectState->symbolCache();

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(symbolUrl, allowMissing));
    if (symbol == nullptr)
        return nullptr;

    switch (symbol->getSymbolType()) {
        case SymbolType::BINDING: {
            auto *bindingSymbol = cast_symbol_to_binding(symbol);
            lyric_common::TypeDef targetType;
            TU_ASSIGN_OR_RETURN (targetType, bindingSymbol->resolveTarget({}));
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(targetType.getConcreteUrl(), allowMissing));
            break;
        }
        case SymbolType::LOCAL: {
            auto *localVariable = cast_symbol_to_local(symbol);
            auto localType = localVariable->getTypeDef();
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(localType.getConcreteUrl(), allowMissing));
            break;
        }
        case SymbolType::LEXICAL: {
            auto *lexicalVariable = cast_symbol_to_lexical(symbol);
            auto lexicalType = lexicalVariable->getTypeDef();
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(lexicalType.getConcreteUrl(), allowMissing));
            break;
        }
        case SymbolType::ARGUMENT: {
            auto *argumentVariable = cast_symbol_to_local(symbol);
            auto argumentType = argumentVariable->getTypeDef();
            TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(argumentType.getConcreteUrl(), allowMissing));
            break;
        }
        default:
            break;
    }
    if (symbol == nullptr)
        return nullptr;

    ImplHandle *implHandle;
    switch (symbol->getSymbolType()) {
        case SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(symbol);
            implHandle = classSymbol->getImpl(implType);
            break;
        }
        case SymbolType::CONCEPT: {
            auto *conceptSymbol = cast_symbol_to_concept(symbol);
            implHandle = conceptSymbol->getImpl(implType);
            break;
        }
        case SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(symbol);
            implHandle = enumSymbol->getImpl(implType);
            break;
        }
        case SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(symbol);
            implHandle = existentialSymbol->getImpl(implType);
            break;
        }
        case SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(symbol);
            implHandle = instanceSymbol->getImpl(implType);
            break;
        }
        case SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(symbol);
            implHandle = structSymbol->getImpl(implType);
            break;
        }
        default:
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid symbol {}", symbol->getSymbolUrl().toString());
    }

    if (implHandle != nullptr)
        return implHandle;

    if (allowMissing)
        return nullptr;
    return AssemblerStatus::forCondition(AssemblerCondition::kMissingImpl,
        "missing impl for {}", implType.toString());
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ImplCache::getOrImportImpl(const ImplReference &implRef, bool allowMissing) const
{
    return getOrImportImpl(implRef.usingRef.symbolUrl, implRef.implType, allowMissing);
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::ImplCache::getOrImportImplMethod(
    const lyric_common::SymbolUrl &symbolUrl,
    const lyric_common::TypeDef &implType,
    const ActionSymbol *actionSymbol,
    bool allowMissing) const
{
    TU_NOTNULL (actionSymbol);

    if (implType.getConcreteUrl() != actionSymbol->getReceiverUrl())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant,
            "action {} does not apply to impl {}",
            actionSymbol->getSymbolUrl().toString(), implType.toString());

    ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, getOrImportImpl(symbolUrl, implType, allowMissing));
    if (implHandle == nullptr)
        return nullptr;

    auto name = actionSymbol->getSymbolUrl().getSymbolName();
    auto *callSymbol = implHandle->getMethod(name);
    if (callSymbol != nullptr)
        return callSymbol;

    if (allowMissing)
        return nullptr;
    return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
        "missing impl method {} on {}", name, implType.toString());
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::ImplCache::getOrImportImplMethod(
    const ImplReference &implRef,
    const ActionSymbol *actionSymbol,
    bool allowMissing) const
{
    return getOrImportImplMethod(implRef.usingRef.symbolUrl, implRef.implType, actionSymbol, allowMissing);
}

bool
lyric_assembler::ImplCache::hasEnvImpl(const lyric_common::TypeDef &consumerType) const
{
    return m_envImpls.contains(consumerType);
}

lyric_common::SymbolUrl
lyric_assembler::ImplCache::getEnvImpl(const lyric_common::TypeDef &consumerType) const
{
    auto iterator = m_envImpls.find(consumerType);
    if (iterator == m_envImpls.cend())
        return {};
    return iterator->second;
}

tempo_utils::Status
lyric_assembler::ImplCache::insertEnvImpl(
    const lyric_common::TypeDef &consumerType,
    const lyric_common::SymbolUrl &url)
{
    auto iterator = m_envImpls.find(consumerType);
    if (iterator != m_envImpls.cend())
        return AssemblerStatus::forCondition(AssemblerCondition::kImplConflict,
            "env impl {} is already set", consumerType.toString());
    m_envImpls[consumerType] = url;
    return {};
}