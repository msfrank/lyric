
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/typename_symbol.h>

#include "lyric_assembler/call_symbol.h"

lyric_assembler::SymbolCache::SymbolCache(ObjectState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::SymbolCache::~SymbolCache()
{
    for (auto &ptr : m_symcache) {
        delete ptr.second;
    }
    while (!m_typenames.empty()) {
        delete m_typenames.front();
        m_typenames.pop();
    }
}

/**
 * Returns true if the symbol cache contains the given symbol `symbolUrl`, otherwise returns false.
 *
 * @param symbolUrl The symbol url which uniquely identifies the symbol.
 * @return  true if the cache contains the symbol, otherwise false.
 */
bool
lyric_assembler::SymbolCache::hasSymbol(const lyric_common::SymbolUrl &symbolUrl) const
{
    return m_symcache.contains(symbolUrl);
}

/**
 * Returns a pointer to the given symbol `symbolUrl` if the symbol is present in the symbol cache,
 * otherwise returns nullptr.
 *
 * @param symbolUrl The symbol url which uniquely identifies the symbol.
 * @return A pointer to the AbstractSymbol.
 */
lyric_assembler::AbstractSymbol *
lyric_assembler::SymbolCache::getSymbolOrNull(const lyric_common::SymbolUrl &symbolUrl) const
{
    auto iterator = m_symcache.find(symbolUrl);
    if (iterator != m_symcache.cend())
        return iterator->second;
    return nullptr;
}

/**
 * Returns a pointer to the given symbol `symbolUrl`, importing it into the symbol cache if necessary.
 * If the symbol is not present in the symbol cache and could not be imported then a `tempo_utils::Status`
 * is returned containing the failure.
 *
 * @param symbolUrl The symbol url which uniquely identifies the symbol.
 * @return A `tempo_utils::Result` containing a pointer to the AbstractSymbol on success, otherwise
 *     a `tempo_utils::Status` on failure.
 */
tempo_utils::Result<lyric_assembler::AbstractSymbol *>
lyric_assembler::SymbolCache::getOrImportSymbol(const lyric_common::SymbolUrl &symbolUrl) const
{
    AbstractSymbol *sym;

    auto iterator = m_symcache.find(symbolUrl);
    if (iterator != m_symcache.cend()) {
        sym = iterator->second;
    } else {
        auto *importCache = m_state->importCache();
        TU_ASSIGN_OR_RETURN (sym, importCache->importSymbol(symbolUrl));
    }

    return sym;
}

inline lyric_object::LinkageSection
symbol_type_to_linkage_section(lyric_assembler::AbstractSymbol *sym)
{
    if (sym == nullptr)
        return lyric_object::LinkageSection::Invalid;
    switch (sym->getSymbolType()) {
        case lyric_assembler::SymbolType::ACTION:
            return lyric_object::LinkageSection::Action;
        case lyric_assembler::SymbolType::BINDING:
            return lyric_object::LinkageSection::Binding;
        case lyric_assembler::SymbolType::CALL:
            return lyric_object::LinkageSection::Call;
        case lyric_assembler::SymbolType::CLASS:
            return lyric_object::LinkageSection::Class;
        case lyric_assembler::SymbolType::CONCEPT:
            return lyric_object::LinkageSection::Concept;
        case lyric_assembler::SymbolType::ENUM:
            return lyric_object::LinkageSection::Enum;
        case lyric_assembler::SymbolType::EXISTENTIAL:
            return lyric_object::LinkageSection::Existential;
        case lyric_assembler::SymbolType::FIELD:
            return lyric_object::LinkageSection::Field;
        case lyric_assembler::SymbolType::INSTANCE:
            return lyric_object::LinkageSection::Instance;
        case lyric_assembler::SymbolType::NAMESPACE:
            return lyric_object::LinkageSection::Namespace;
        case lyric_assembler::SymbolType::STATIC:
            return lyric_object::LinkageSection::Static;
        case lyric_assembler::SymbolType::STRUCT:
            return lyric_object::LinkageSection::Struct;
        default:
            return lyric_object::LinkageSection::Invalid;
    }
}

tempo_utils::Status
lyric_assembler::SymbolCache::insertSymbol(
    const lyric_common::SymbolUrl &symbolUrl,
    AbstractSymbol *abstractSymbol,
    TypenameSymbol *existingTypename)
{
    auto entry = m_symcache.find(symbolUrl);

    // if symbol is in the cache then check whether it's appropriate to replace it
    if (entry != m_symcache.cend()) {
        auto *previousSymbol = entry->second;
        if (previousSymbol->getSymbolType() != SymbolType::TYPENAME)
            return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
                "symbol {} is already defined", symbolUrl.toString());
        if (existingTypename && previousSymbol != existingTypename)
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "existing typename doesn't match symbol in cache");
        m_typenames.push(cast_symbol_to_typename(previousSymbol));
    }

    m_symcache[symbolUrl] = abstractSymbol;
    return {};
}

tempo_utils::Result<lyric_assembler::TypenameSymbol *>
lyric_assembler::SymbolCache::putTypename(const lyric_common::SymbolUrl &typenameUrl)
{
    if (m_symcache.contains(typenameUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "symbol {} is already defined", typenameUrl.toString());
    auto *typenameSymbol = new TypenameSymbol(typenameUrl);
    m_symcache[typenameUrl] = typenameSymbol;
    return typenameSymbol;
}

absl::flat_hash_map<lyric_common::SymbolUrl, lyric_assembler::AbstractSymbol *>::const_iterator
lyric_assembler::SymbolCache::symbolsBegin() const
{
    return m_symcache.cbegin();
}

absl::flat_hash_map<lyric_common::SymbolUrl, lyric_assembler::AbstractSymbol *>::const_iterator
lyric_assembler::SymbolCache::symbolsEnd() const
{
    return m_symcache.cend();
}

int
lyric_assembler::SymbolCache::numSymbols() const
{
    return m_symcache.size();
}

tempo_utils::Result<lyric_assembler::ActionSymbol *>
lyric_assembler::SymbolCache::getOrImportAction(const lyric_common::SymbolUrl &actionUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(actionUrl));
    if (sym->getSymbolType() != SymbolType::ACTION)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid action", actionUrl.toString());
    return (ActionSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::BindingSymbol *>
lyric_assembler::SymbolCache::getOrImportBinding(const lyric_common::SymbolUrl &bindingUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(bindingUrl));
    if (sym->getSymbolType() != SymbolType::BINDING)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid binding", bindingUrl.toString());
    return (BindingSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::SymbolCache::getOrImportCall(const lyric_common::SymbolUrl &callUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(callUrl));
    if (sym->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid call", callUrl.toString());
    return (CallSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::ClassSymbol *>
lyric_assembler::SymbolCache::getOrImportClass(const lyric_common::SymbolUrl &classUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(classUrl));
    if (sym->getSymbolType() != SymbolType::CLASS)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid class", classUrl.toString());
    return (ClassSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::ConceptSymbol *>
lyric_assembler::SymbolCache::getOrImportConcept(const lyric_common::SymbolUrl &conceptUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(conceptUrl));
    if (sym->getSymbolType() != SymbolType::CONCEPT)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid concept", conceptUrl.toString());
    return (ConceptSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::EnumSymbol *>
lyric_assembler::SymbolCache::getOrImportEnum(const lyric_common::SymbolUrl &enumUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(enumUrl));
    if (sym->getSymbolType() != SymbolType::ENUM)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid enum", enumUrl.toString());
    return (EnumSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::ExistentialSymbol *>
lyric_assembler::SymbolCache::getOrImportExistential(const lyric_common::SymbolUrl &existentialUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(existentialUrl));
    if (sym->getSymbolType() != SymbolType::EXISTENTIAL)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid existential", existentialUrl.toString());
    return (ExistentialSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::FieldSymbol *>
lyric_assembler::SymbolCache::getOrImportField(const lyric_common::SymbolUrl &fieldUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(fieldUrl));
    if (sym->getSymbolType() != SymbolType::FIELD)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid field", fieldUrl.toString());
    return (FieldSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::InstanceSymbol *>
lyric_assembler::SymbolCache::getOrImportInstance(const lyric_common::SymbolUrl &instanceUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(instanceUrl));
    if (sym->getSymbolType() != SymbolType::INSTANCE)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid instance", instanceUrl.toString());
    return (InstanceSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::NamespaceSymbol *>
lyric_assembler::SymbolCache::getOrImportNamespace(const lyric_common::SymbolUrl &namespaceUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(namespaceUrl));
    if (sym->getSymbolType() != SymbolType::NAMESPACE)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid namespace", namespaceUrl.toString());
    return (NamespaceSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::StaticSymbol *>
lyric_assembler::SymbolCache::getOrImportStatic(const lyric_common::SymbolUrl &staticUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(staticUrl));
    if (sym->getSymbolType() != SymbolType::STATIC)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid static", staticUrl.toString());
    return (StaticSymbol *) sym;
}

tempo_utils::Result<lyric_assembler::StructSymbol *>
lyric_assembler::SymbolCache::getOrImportStruct(const lyric_common::SymbolUrl &structUrl) const
{
    AbstractSymbol *sym;
    TU_ASSIGN_OR_RETURN (sym, getOrImportSymbol(structUrl));
    if (sym->getSymbolType() != SymbolType::STRUCT)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidSymbol,
            "symbol {} is not a valid struct", structUrl.toString());
    return (StructSymbol *) sym;
}
