
#include <lyric_assembler/symbol_cache.h>

lyric_assembler::SymbolCache::SymbolCache(lyric_assembler::AssemblerTracer *tracer)
    : m_tracer(tracer)
{
    TU_ASSERT (m_tracer != nullptr);
}

lyric_assembler::SymbolCache::~SymbolCache()
{
    for (auto &ptr : m_symcache) {
        delete ptr.second;
    }
}

bool
lyric_assembler::SymbolCache::hasSymbol(const lyric_common::SymbolUrl &symbolUrl) const
{
    return m_symcache.contains(symbolUrl);
}

lyric_assembler::AbstractSymbol *
lyric_assembler::SymbolCache::getSymbol(const lyric_common::SymbolUrl &symbolUrl) const
{
    auto iterator = m_symcache.find(symbolUrl);
    if (iterator == m_symcache.cend())
        return nullptr;
    return iterator->second;
}

tempo_utils::Status
lyric_assembler::SymbolCache::insertSymbol(const lyric_common::SymbolUrl &symbolUrl, AbstractSymbol *abstractSymbol)
{
    if (m_symcache.contains(symbolUrl))
        return m_tracer->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "symbol {} is already defined", symbolUrl.toString());
    m_symcache[symbolUrl] = abstractSymbol;
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::SymbolCache::touchSymbol(const lyric_common::SymbolUrl &symbolUrl)
{
    if (!symbolUrl.isValid())
        m_tracer->throwAssemblerInvariant("invalid symbol {}", symbolUrl.toString());
    auto iterator = m_symcache.find(symbolUrl);
    if (iterator == m_symcache.cend())
        m_tracer->throwAssemblerInvariant("missing symbol {}", symbolUrl.toString());
    iterator->second->touch();
    return AssemblerStatus::ok();
}

int
lyric_assembler::SymbolCache::numSymbols() const
{
    return m_symcache.size();
}

bool
lyric_assembler::SymbolCache::hasEnvBinding(const std::string &name) const
{
    return m_envBindings.contains(name);
}

lyric_assembler::SymbolBinding
lyric_assembler::SymbolCache::getEnvBinding(const std::string &name) const
{
    if (m_envBindings.contains(name))
        return m_envBindings.at(name);
    return {};
}

tempo_utils::Status
lyric_assembler::SymbolCache::insertEnvBinding(const std::string &name, const SymbolBinding &binding)
{
    if (m_envBindings.contains(name))
        return m_tracer->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "env binding {} is already set", name);
    m_envBindings[name] = binding;
    return AssemblerStatus::ok();
}

bool
lyric_assembler::SymbolCache::hasEnvInstance(const lyric_common::TypeDef &type) const
{
    return m_envInstances.contains(type);
}

lyric_common::SymbolUrl
lyric_assembler::SymbolCache::getEnvInstance(const lyric_common::TypeDef &type) const
{
    if (m_envInstances.contains(type))
        return m_envInstances.at(type);
    return {};
}

tempo_utils::Status
lyric_assembler::SymbolCache::insertEnvInstance(const lyric_common::TypeDef &type, const lyric_common::SymbolUrl &url)
{
    if (m_envInstances.contains(type))
        return m_tracer->logAndContinue(AssemblerCondition::kImplConflict,
            tempo_tracing::LogSeverity::kError,
            "env instance {} is already set", type.toString());
    m_envInstances[type] = url;
    return AssemblerStatus::ok();
}
