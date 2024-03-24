#ifndef LYRIC_BOOSTRAP_INTERNAL_BOOTSTRAP_TYPES_H
#define LYRIC_BOOSTRAP_INTERNAL_BOOTSTRAP_TYPES_H

#include <cstdint>

namespace lyric_bootstrap::internal {

    enum class BootstrapTrap : uint32_t {
        CATEGORY_ALLOC,
        CLOSURE_ALLOC,
        CLOSURE_APPLY,
        CLOSURE_CTOR,
        FLOAT_CEIL,
        FLOAT_FLOOR,
        FLOAT_TRUNC,
        ITERATOR_ALLOC,
        ITERATOR_VALID,
        ITERATOR_NEXT,
        MAP_ALLOC,
        MAP_CTOR,
        MAP_CONTAINS,
        MAP_GET,
        MAP_SIZE,
        MAP_UPDATE,
        MAP_REMOVE,
        OBJECT_ALLOC,
        PAIR_ALLOC,
        PAIR_CTOR,
        PAIR_FIRST,
        PAIR_SECOND,
        RECORD_ALLOC,
        SEQ_ALLOC,
        SEQ_CTOR,
        SEQ_SIZE,
        SEQ_GET,
        SEQ_APPEND,
        SEQ_EXTEND,
        SEQ_SLICE,
        SEQ_ITER,
        SINGLETON_ALLOC,
        STATUS_ALLOC,
        STRING_ALLOC,
        STRING_AT,
        STRING_COMPARE,
        STRING_CTOR,
        STRING_LENGTH,
        URI_ALLOC,
        URI_CTOR,
        URI_EQUALS,
        LAST_,
    };
}

#endif // LYRIC_BOOSTRAP_INTERNAL_BOOTSTRAP_TYPES_H