#ifndef LYRIC_REWRITER_MACRO_REGISTRY_H
#define LYRIC_REWRITER_MACRO_REGISTRY_H

#include "abstract_macro.h"

namespace lyric_rewriter {

    class MacroRegistry {
    public:
        explicit MacroRegistry(const absl::flat_hash_map<std::string,std::shared_ptr<AbstractMacro>> &macros = {});

        std::shared_ptr<AbstractMacro> getMacro(std::string_view macroName);

    private:
        absl::flat_hash_map<std::string,std::shared_ptr<AbstractMacro>> m_macros;
    };
}

#endif // LYRIC_REWRITER_MACRO_REGISTRY_H
