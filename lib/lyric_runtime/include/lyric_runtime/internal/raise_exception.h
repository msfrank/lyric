#ifndef LYRIC_RUNTIME_INTERNAL_RAISE_EXCEPTION_H
#define LYRIC_RUNTIME_INTERNAL_RAISE_EXCEPTION_H

#include <lyric_object/bytecode_iterator.h>
#include <lyric_runtime/segment_manager.h>
#include <lyric_runtime/stackful_coroutine.h>
#include <lyric_runtime/subroutine_manager.h>
#include <lyric_runtime/type_manager.h>
#include <tempo_utils/status.h>

namespace lyric_runtime::internal {

    tempo_utils::Status raise_exception(
             const lyric_object::OpCell &op,
             const DataCell &exc,
             StackfulCoroutine *currentCoro,
             SegmentManager *segmentManager,
             SubroutineManager *subroutineManager,
             TypeManager *typeManager);

}

#endif // LYRIC_RUNTIME_INTERNAL_RAISE_EXCEPTION_H