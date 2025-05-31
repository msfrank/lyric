
#include <lyric_rewriter/macro_registry.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::MacroRegistry::MacroRegistry(bool excludePredefinedNames)
    : m_isSealed(false)
{
    if (!excludePredefinedNames) {
    }
}

tempo_utils::Status
lyric_rewriter::MacroRegistry::registerMacroName(const std::string &macroName, MakeMacroFunc func)
{
    if (macroName.empty())
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid macro name");
    if (func == nullptr)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid MakeMacro func");
    if (m_isSealed)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "cannot mutate sealed macro registry");
    if (m_makeMacroFuncs.contains(macroName))
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "macro name '{}' is already registered", macroName);
    m_makeMacroFuncs[macroName] = func;
    return {};
}

tempo_utils::Status
lyric_rewriter::MacroRegistry::replaceMacroName(const std::string &macroName, MakeMacroFunc func)
{
    if (macroName.empty())
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid macro name");
    if (func == nullptr)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid MakeMacro func");
    if (m_isSealed)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "cannot mutate sealed macro registry");
    m_makeMacroFuncs[macroName] = func;
    return {};
}

tempo_utils::Status
lyric_rewriter::MacroRegistry::deregisterMacroName(const std::string &macroName)
{
    if (macroName.empty())
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid macro name");
    if (m_isSealed)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "cannot mutate sealed macro registry");
    auto entry = m_makeMacroFuncs.find(macroName);
    if (entry == m_makeMacroFuncs.cend())
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "macro name '{}' is not registered", macroName);
    m_makeMacroFuncs.erase(entry);
    return {};
}

void
lyric_rewriter::MacroRegistry::sealRegistry()
{
    m_isSealed = true;
}

tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractMacro>>
lyric_rewriter::MacroRegistry::makeMacro(std::string_view macroName) const
{
    if (!m_isSealed)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "macro registry must be sealed to make macros");
    auto entry = m_makeMacroFuncs.find(macroName);
    if (entry == m_makeMacroFuncs.cend())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "unknown macro name '{}'", macroName);
    auto macro = entry->second();
    if (macro == nullptr)
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid macro name '{}'", macroName);
    return macro;
}
