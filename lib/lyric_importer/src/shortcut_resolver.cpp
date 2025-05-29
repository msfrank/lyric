
#include <lyric_importer/importer_result.h>
#include <lyric_importer/shortcut_resolver.h>

bool
lyric_importer::ShortcutResolver::hasShortcut(const std::string &shortcut) const
{
    if (shortcut.empty())
        return false;
    return m_shortcuts.contains(shortcut);
}

tempo_utils::UrlOrigin
lyric_importer::ShortcutResolver::getShortcut(const std::string &shortcut) const
{
    if (shortcut.empty())
        return {};
    auto entry = m_shortcuts.find(shortcut);
    if (entry != m_shortcuts.cend())
        return entry->second;
    return {};
}

absl::flat_hash_map<std::string,tempo_utils::UrlOrigin>::const_iterator
lyric_importer::ShortcutResolver::shortcutsBegin() const
{
    return m_shortcuts.cbegin();
}

absl::flat_hash_map<std::string,tempo_utils::UrlOrigin>::const_iterator
lyric_importer::ShortcutResolver::shortcutsEnd() const
{
    return m_shortcuts.cend();
}

int
lyric_importer::ShortcutResolver::numShortcuts() const
{
    return m_shortcuts.size();
}

tempo_utils::Status
lyric_importer::ShortcutResolver::insertShortcut(const std::string &shortcut, const tempo_utils::UrlOrigin &origin)
{
    if (shortcut.empty())
        return ImporterStatus::forCondition(ImporterCondition::kImporterInvariant,
            "invalid shortcut");
    if (!origin.isValid())
        return ImporterStatus::forCondition(ImporterCondition::kImporterInvariant,
            "invalid shortcut {}; invalid origin", shortcut);
    if (m_shortcuts.contains(shortcut))
        return ImporterStatus::forCondition(ImporterCondition::kImporterInvariant,
            "shortcut {} already exists", shortcut);
    m_shortcuts[shortcut] = origin;
    return {};
}

tempo_utils::Result<tempo_utils::UrlOrigin>
lyric_importer::ShortcutResolver::resolveShortcut(const std::string &shortcut) const
{
    if (shortcut.empty())
        return ImporterStatus::forCondition(ImporterCondition::kImporterInvariant,
            "invalid shortcut");
    auto entry = m_shortcuts.find(shortcut);
    if (entry == m_shortcuts.cend())
        return ImporterStatus::forCondition(ImporterCondition::kImporterInvariant,
            "missing shortcut {}", shortcut);
    return entry->second;
}