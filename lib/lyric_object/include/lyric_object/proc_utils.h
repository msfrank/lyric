#ifndef LYRIC_OBJECT_PROC_UTILS_H
#define LYRIC_OBJECT_PROC_UTILS_H

#include <span>

#include <tempo_utils/integer_types.h>
#include <tempo_utils/status.h>

#include "object_types.h"

namespace lyric_object {

    /**
     * The size of the proc header in bytes. The proc header consists of:
     *   num_arguments:     uint16
     *   num_locals:        uint16
     *   num_lexicals:      uint16
     *   trailer_size:      uint32
     */
    constexpr tu_uint32 kProcHeaderSizeInBytes = 10;

    /**
     * The size of a proc lexical in bytes. A proc lexical consists of:
     *   activation_call:   uint32
     *   target_offset:     uint32
     *   lexical_target:    uint8
     */
    constexpr tu_uint32 kProcLexicalSizeInBytes = 9;

    /**
     * The size of a proc trailer in bytes. A proc trailer consists of:
     *   num_checks:        uint16
     *   num_exceptions:    uint16
     *   num_cleanups:      uint16
     */
    constexpr tu_uint32 kProcTrailerSizeInBytes = 6;

    /**
     * The size of a proc check in bytes. A proc check consists of:
     *   interval_offset:   uint32
     *   interval_size:     uint32
     *   first_exception:   uint16
     *   num_exceptions:    uint16
     *   exception_local:   uint16
    */
    constexpr tu_uint32 kProcCheckSizeInBytes = 14;

    /**
     * The size of a proc exception in bytes. A proc exception consists of:
     *   exception_type:    uint32
     *   catch_offset:      uint32
     *   catch_size:        uint32
    */
    constexpr tu_uint32 kProcExceptionSizeInBytes = 12;

    /**
     * The size of a proc cleanup in bytes. A proc cleanup consists of:
     *   interval_offset:   uint32
     *   interval_size:     uint32
     *   parent_cleanup:    uint16
     *   cleanup_offset:    uint32
     *   cleanup_size:      uint32
    */
    constexpr tu_uint32 kProcCleanupSizeInBytes = 18;

    /**
     * Proc information.
     */
    struct ProcInfo {
        std::span<const tu_uint8> proc;
        tu_uint16 num_arguments;
        tu_uint16 num_locals;
        tu_uint16 num_lexicals;
        std::span<const tu_uint8> lexicals;
        std::span<const tu_uint8> code;
        std::span<const tu_uint8> trailer;
    };

    /**
     * Proc lexical information.
     */
    struct ProcLexical {
        tu_uint32 activation_call;
        tu_uint32 target_offset;
        tu_uint8 lexical_target;
    };

    /**
     * Proc trailer information.
     */
    struct TrailerInfo {
        std::span<const tu_uint8> trailer;
        tu_uint16 num_checks;
        tu_uint16 num_exceptions;
        tu_uint16 num_cleanups;
        std::span<const tu_uint8> checks;
        std::span<const tu_uint8> exceptions;
        std::span<const tu_uint8> cleanups;
    };

    /**
     * Proc check information.
     */
    struct ProcCheck {
        tu_uint32 interval_offset;      /**< offset from the start of the bytecode marking the start of the interval. */
        tu_uint32 interval_size;        /**< the size of the interval in bytes. */
        tu_uint16 first_exception;      /**< index of the first exception to check. */
        tu_uint16 num_exceptions;       /**< the number of exceptions associated with the check. */
        tu_uint16 exception_local;      /**< offset of the local variable in the proc where the exception ref should be stored. */
    };

    /**
     * Proc exception information.
     */
    struct ProcException {
        tu_uint32 exception_type;       /**< index of the exception type in the types section of the object. */
        tu_uint32 catch_offset;         /**< offset from the start of the bytecode marking the start of the catch handler. */
        tu_uint32 catch_size;           /**< the size of the catch handler in bytes. */
    };

    /**
     * Proc cleanup information.
     */
    struct ProcCleanup {
        tu_uint32 interval_offset;      /**< offset from the start of the bytecode marking the start of the interval. */
        tu_uint32 interval_size;        /**< the size of the interval in bytes. */
        tu_uint16 parent_cleanup;       /**< index of the enclosing check, otherwise invalid offset if there is no parent. */
        tu_uint32 cleanup_offset;       /**< offset from the start of the bytecode marking the start of the cleanup handler. */
        tu_uint32 cleanup_size;         /**< the size of the cleanup handler in bytes. */
    };

    tempo_utils::Status parse_proc_info(std::span<const tu_uint8> bytecode, tu_uint32 offset, ProcInfo &procInfo);

    tempo_utils::Status parse_lexicals_table_entry(const ProcInfo &procInfo, int index, ProcLexical &lexical);

    tempo_utils::Status parse_lexicals_table(const ProcInfo &procInfo, std::vector<ProcLexical> &lexicals);

    tempo_utils::Status parse_proc_trailer(const ProcInfo &procInfo, TrailerInfo &trailerInfo);

    tempo_utils::Status parse_checks_table(const TrailerInfo &trailerInfo, std::vector<ProcCheck> &checks);

    tempo_utils::Status parse_exceptions_table(const TrailerInfo &trailerInfo, std::vector<ProcException> &exceptions);

    tempo_utils::Status parse_cleanups_table(const TrailerInfo &trailerInfo, std::vector<ProcCleanup> &cleanups);
}

#endif // LYRIC_OBJECT_PROC_UTILS_H