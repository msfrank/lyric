#ifndef LYRIC_REWRITER_MACRO_REGISTRY_H
#define LYRIC_REWRITER_MACRO_REGISTRY_H

#include "abstract_macro.h"

namespace lyric_rewriter {

    class MacroRegistry {
    public:
        explicit MacroRegistry(bool excludePredefinedNames = false);

        using MakeMacroFunc = std::function<std::shared_ptr<AbstractMacro>()>;

        tempo_utils::Status registerMacroName(const std::string &macroName, MakeMacroFunc func);
        tempo_utils::Status replaceMacroName(const std::string &macroName, MakeMacroFunc func);
        tempo_utils::Status deregisterMacroName(const std::string &macroName);

        void sealRegistry();

        tempo_utils::Result<std::shared_ptr<AbstractMacro>> makeMacro(std::string_view macroName) const;

    private:
        absl::flat_hash_map<std::string, MakeMacroFunc> m_makeMacroFuncs;
        bool m_isSealed;
    };
}

#endif // LYRIC_REWRITER_MACRO_REGISTRY_H
