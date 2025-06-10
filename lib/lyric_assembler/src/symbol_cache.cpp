
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/typename_symbol.h>

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