#ifndef LYRIC_COMMON_RUNTIME_CONVERSIONS_H
#define LYRIC_COMMON_RUNTIME_CONVERSIONS_H

#include <lyric_common/module_location.h>
#include <lyric_common/symbol_path.h>
#include <lyric_common/symbol_url.h>
#include <tempo_config/abstract_config_parser.h>

namespace lyric_common {

    class ModuleLocationParser : public tempo_config::AbstractConfigParser<ModuleLocation> {
    public:
        ModuleLocationParser();
        ModuleLocationParser(const lyric_common::ModuleLocation &moduleLocationDefault);

        tempo_utils::Status parseValue(
            const tempo_config::ConfigNode &node,
            ModuleLocation &moduleLocation) const override;

    private:
        Option<lyric_common::ModuleLocation> m_default;
    };

    class SymbolPathParser : public tempo_config::AbstractConfigParser<SymbolPath> {
    public:
        SymbolPathParser();
        SymbolPathParser(const SymbolPath &symbolPathDefault);
        tempo_utils::Status parseValue(
            const tempo_config::ConfigNode &node,
            SymbolPath &symbolPath) const override;

    private:
        Option<SymbolPath> m_default;
    };

    class SymbolUrlParser : public tempo_config::AbstractConfigParser<SymbolUrl> {
    public:
        SymbolUrlParser();
        SymbolUrlParser(const SymbolUrl &symbolUrlDefault);
        tempo_utils::Status parseValue(
            const tempo_config::ConfigNode &node,
            SymbolUrl &symbolUrl) const override;

    private:
        Option<SymbolUrl> m_default;
    };
}

#endif // LYRIC_COMMON_RUNTIME_CONVERSIONS_H