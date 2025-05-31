#ifndef LYRIC_BUILD_INTERNAL_BUILD_MACROS_H
#define LYRIC_BUILD_INTERNAL_BUILD_MACROS_H

#include <lyric_rewriter/macro_registry.h>

namespace lyric_build::internal {

    tempo_utils::Result<std::shared_ptr<lyric_rewriter::MacroRegistry>> make_build_macros();
}

#endif // LYRIC_BUILD_INTERNAL_BUILD_MACROS_H
