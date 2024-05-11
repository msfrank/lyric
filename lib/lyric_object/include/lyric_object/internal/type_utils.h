#ifndef LYRIC_OBJECT_INTERNAL_TYPE_UTILS_H
#define LYRIC_OBJECT_INTERNAL_TYPE_UTILS_H

#include <lyric_common/symbol_url.h>
#include <tempo_utils/result.h>

#include "object_reader.h"

namespace lyric_object::internal {

    tempo_utils::Result<lyric_common::SymbolUrl> load_type_symbol(
        std::shared_ptr<const ObjectReader> reader,
        const lyric_common::AssemblyLocation &location,
        lyo1::TypeSection section,
        uint32_t descriptor);
}

#endif // LYRIC_OBJECT_INTERNAL_TYPE_UTILS_H
