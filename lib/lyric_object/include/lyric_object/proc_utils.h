#ifndef LYRIC_OBJECT_PROC_UTILS_H
#define LYRIC_OBJECT_PROC_UTILS_H

#include <span>

#include <tempo_utils/integer_types.h>
#include <tempo_utils/status.h>

#include "object_types.h"

namespace lyric_object {

    /**
     * The size of the proc header in bytes. The proc header consists of:
     *   num_arguments:     uint16 (2 bytes)
     *   num_locals:        uint16 (2 bytes)
     *   num_lexicals:      uint16 (2 bytes)
     *   trailer_size:      uint32 (4 bytes)
     */
    constexpr tu_uint32 kProcHeaderSizeInBytes = 10;

    /**
     * The size of a proc lexical in bytes. A proc lexical consists of:
     *   activation_call:   uint32 (4 bytes)
     *   target_offset:     uint32 (4 bytes)
     *   lexical_target:    uint8 (1 byte)
     */
    constexpr tu_uint32 kProcLexicalSizeInBytes = 9;

    /**
     * The size of a proc trailer in bytes. A proc trailer consists of:
     *   num_checks:        uint16 (2 bytes)
     *   num_exceptions:    uint16 (2 bytes)
     *   num_cleanups:      uint16 (2 bytes)
     */
    constexpr tu_uint32 kProcTrailerSizeInBytes = 6;

    constexpr tu_uint32 kProcCheckSizeInBytes = 14;

    constexpr tu_uint32 kProcExceptionSizeInBytes = 12;

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
        tu_uint16 parent_check;         /**< index of the enclosing check, otherwise invalid offset if there is no parent. */
        tu_uint16 first_exception;      /**< index of the first exception to check. */
        tu_uint16 num_exceptions;       /**< the number of exceptions associated with the check. */
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