
#include <lyric_bootstrap/internal/bootstrap_types.h>

#include "bytes_traps.h"
#include "category_ref.h"
#include "closure_ref.h"
#include "float_traps.h"
#include "map_ref.h"
#include "native_prelude.h"
#include "object_ref.h"
#include "pair_ref.h"
#include "record_ref.h"
#include "rest_iterator_ref.h"
#include "rest_traps.h"
#include "seq_ref.h"
#include "singleton_ref.h"
#include "status_traps.h"
#include "string_traps.h"
#include "url_traps.h"

std::array<lyric_runtime::NativeTrap,55> kPreludeTraps = {{
    {bytes_at, "BytesAt", 0},
    { bytes_compare, "BytesCompare", 0 },
    { bytes_length, "BytesLength", 0 },
    { bytes_to_string, "BytesToString", 0 },
    { category_alloc, "CategoryAlloc", 0 },
    { closure_alloc, "ClosureAlloc", 0 },
    { closure_apply, "ClosureApply", 0 },
    { closure_ctor, "ClosureCtor", 0 },
    { float_ceil, "FloatCeil", 0 },
    { float_floor, "FloatFloor", 0 },
    { float_trunc, "FloatTrunc", 0 },
    { map_alloc, "MapAlloc", 0 },
    { map_ctor, "MapCtor", 0 },
    { map_contains, "MapContains", 0 },
    { map_get, "MapGet", 0 },
    { map_size, "MapSize", 0 },
    { map_update, "MapUpdate", 0 },
    { map_remove, "MapRemove", 0 },
    { map_iterate, "MapIterate", 0 },
    { map_iterator_alloc, "MapIteratorAlloc", 0 },
    { map_iterator_next, "MapIteratorNext", 0 },
    { map_iterator_valid, "MapIteratorValid", 0 },
    { object_alloc, "ObjectAlloc", 0 },
    { pair_alloc, "PairAlloc", 0 },
    { pair_ctor, "PairCtor", 0 },
    { pair_first, "PairFirst", 0 },
    { pair_second, "PairSecond", 0 },
    { record_alloc, "RecordAlloc", 0 },
    { rest_size, "RestSize", 0 },
    { rest_get, "RestGet", 0 },
    { rest_iterate, "RestIterate", 0 },
    { rest_iterator_alloc, "RestIteratorAlloc", 0 },
    { rest_iterator_next, "RestIteratorNext", 0 },
    { rest_iterator_valid, "RestIteratorValid", 0 },
    { seq_alloc, "SeqAlloc", },
    { seq_ctor, "SeqCtor", 0 },
    { seq_size, "SeqSize", 0 },
    { seq_get, "SeqGet", 0 },
    { seq_append, "SeqAppend", 0 },
    { seq_extend, "SeqExtend", 0 },
    { seq_slice, "SeqSlice", 0 },
    { seq_iterate, "SeqIterate", 0 },
    { seq_iterator_alloc, "SeqIteratorAlloc", 0 },
    { seq_iterator_next, "SeqIteratorNext", 0 },
    { seq_iterator_valid, "SeqIteratorValid", 0 },
    { singleton_alloc, "SingletonAlloc", 0 },
    { status_alloc, "StatusAlloc", 0 },
    { status_ctor, "StatusCtor", 0 },
    { status_get_code, "StatusGetCode", 0 },
    { status_get_message, "StatusGetMessage", 0 },
    { string_at, "StringAt", 0 },
    { string_compare, "StringCompare", 0 },
    { string_length, "StringLength", 0 },
    { string_to_bytes, "StringToBytes", 0 },
    { url_equals, "UrlEquals", 0 },
}};

bool
NativeCore::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeCore::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

const lyric_runtime::NativeTrap *
NativeCore::getTrap(tu_uint32 index) const
{
    if (kPreludeTraps.size() <= index)
        return nullptr;
    return &kPreludeTraps.at(index);
}

uint32_t
NativeCore::numTraps() const
{
    return kPreludeTraps.size();
}

static const NativeCore iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}