
#include <lyric_bootstrap/internal/bootstrap_types.h>

#include "category_ref.h"
#include "closure_ref.h"
#include "iterator_ref.h"
#include "map_ref.h"
#include "native_prelude.h"
#include "object_ref.h"
#include "pair_ref.h"
#include "record_ref.h"
#include "seq_ref.h"
#include "singleton_ref.h"
#include "string_ref.h"
#include "uri_ref.h"

lyric_runtime::NativeFunc
NativeCore::getTrap(uint32_t index) const
{
    auto trapFunction = static_cast<lyric_bootstrap::internal::BootstrapTrap>(index);
    switch (trapFunction) {
        case lyric_bootstrap::internal::BootstrapTrap::CATEGORY_ALLOC:
            return category_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::CLOSURE_ALLOC:
            return closure_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::CLOSURE_APPLY:
            return closure_apply;
        case lyric_bootstrap::internal::BootstrapTrap::CLOSURE_CTOR:
            return closure_ctor;
        case lyric_bootstrap::internal::BootstrapTrap::ITERATOR_ALLOC:
            return iterator_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::ITERATOR_VALID:
            return iterator_valid;
        case lyric_bootstrap::internal::BootstrapTrap::ITERATOR_NEXT:
            return iterator_next;
        case lyric_bootstrap::internal::BootstrapTrap::MAP_ALLOC:
            return map_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::MAP_CTOR:
            return map_ctor;
        case lyric_bootstrap::internal::BootstrapTrap::MAP_CONTAINS:
            return map_contains;
        case lyric_bootstrap::internal::BootstrapTrap::MAP_GET:
            return map_get;
        case lyric_bootstrap::internal::BootstrapTrap::MAP_SIZE:
            return map_size;
        case lyric_bootstrap::internal::BootstrapTrap::MAP_UPDATE:
            return map_update;
        case lyric_bootstrap::internal::BootstrapTrap::MAP_REMOVE:
            return map_remove;
        case lyric_bootstrap::internal::BootstrapTrap::OBJECT_ALLOC:
            return object_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::PAIR_ALLOC:
            return pair_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::PAIR_CTOR:
            return pair_ctor;
        case lyric_bootstrap::internal::BootstrapTrap::PAIR_FIRST:
            return pair_first;
        case lyric_bootstrap::internal::BootstrapTrap::PAIR_SECOND:
            return pair_second;
        case lyric_bootstrap::internal::BootstrapTrap::RECORD_ALLOC:
            return record_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::SEQ_ALLOC:
            return seq_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::SEQ_CTOR:
            return seq_ctor;
        case lyric_bootstrap::internal::BootstrapTrap::SEQ_SIZE:
            return seq_size;
        case lyric_bootstrap::internal::BootstrapTrap::SEQ_GET:
            return seq_get;
        case lyric_bootstrap::internal::BootstrapTrap::SEQ_APPEND:
            return seq_append;
        case lyric_bootstrap::internal::BootstrapTrap::SEQ_EXTEND:
            return seq_extend;
        case lyric_bootstrap::internal::BootstrapTrap::SEQ_SLICE:
            return seq_slice;
        case lyric_bootstrap::internal::BootstrapTrap::SEQ_ITER:
            return seq_iter;
        case lyric_bootstrap::internal::BootstrapTrap::SINGLETON_ALLOC:
            return singleton_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::STRING_ALLOC:
            return string_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::STRING_AT:
            return string_at;
        case lyric_bootstrap::internal::BootstrapTrap::STRING_COMPARE:
            return string_compare;
        case lyric_bootstrap::internal::BootstrapTrap::STRING_CTOR:
            return string_ctor;
        case lyric_bootstrap::internal::BootstrapTrap::STRING_LENGTH:
            return string_length;
        case lyric_bootstrap::internal::BootstrapTrap::URI_ALLOC:
            return uri_alloc;
        case lyric_bootstrap::internal::BootstrapTrap::URI_CTOR:
            return uri_ctor;
        case lyric_bootstrap::internal::BootstrapTrap::URI_EQUALS:
            return uri_equals;
        case lyric_bootstrap::internal::BootstrapTrap::LAST_:
        default:
            return nullptr;
    }
}

uint32_t
NativeCore::numTraps() const
{
    return static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::LAST_);
}

static const NativeCore iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}