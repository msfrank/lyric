
#include <lyric_rewriter/macro_registry.h>

lyric_rewriter::MacroRegistry::MacroRegistry(
    const absl::flat_hash_map<std::string,std::shared_ptr<AbstractMacro>> &macros)
    : m_macros(macros)
{
}

std::shared_ptr<lyric_rewriter::AbstractMacro>
lyric_rewriter::MacroRegistry::getMacro(std::string_view macroName)
{
    auto entry = m_macros.find(macroName);
    if (entry != m_macros.cend())
        return entry->second;
    return {};
}
