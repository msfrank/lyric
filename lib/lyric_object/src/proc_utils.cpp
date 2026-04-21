
#include <lyric_object/object_result.h>
#include <lyric_object/proc_utils.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/bytes_iterator.h>
#include <tempo_utils/status.h>


tempo_utils::Status
lyric_object::parse_proc(std::span<const tu_uint8> bytecode, tu_uint32 offset, std::span<const tu_uint8> &proc)
{
    if (bytecode.size() <= offset)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc offset");

    tempo_utils::BytesIterator it(bytecode.subspan(offset));

    // there must be 4 bytes available to read starting at offset within the bytecode span
    tu_uint32 size;
    if (!it.nextU32(size))
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc offset");

    // there must be `size` bytes available to read in the remaining span
    if (it.bytesLeft() < size)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc header");

    TU_ASSERT (it.nextSlice(proc, size));

    return {};
}

tempo_utils::Status
lyric_object::parse_proc_info(std::span<const tu_uint8> bytecode, tu_uint32 offset, ProcInfo &procInfo)
{
    std::span<const tu_uint8> proc;
    TU_RETURN_IF_NOT_OK (parse_proc(bytecode, offset, proc));

    // construct iterator for the proc
    tempo_utils::BytesIterator it(proc);

    // there must be enough bytes to read the entire proc header
    if (it.bytesLeft() < kProcHeaderSizeInBytes)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc header");

    TU_ASSERT (it.nextU16(procInfo.num_arguments));
    TU_ASSERT (it.nextU16(procInfo.num_locals));
    TU_ASSERT (it.nextU16(procInfo.num_lexicals));

    tu_uint32 trailerSize;
    TU_ASSERT (it.nextU32(trailerSize));

    // there must be enough bytes to read the entire lexicals table
    auto lexicalsSize = procInfo.num_lexicals * kProcLexicalSizeInBytes;
    if (it.bytesLeft() < lexicalsSize)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc header");

    // calculate the lexicals span
    TU_ASSERT (it.nextSlice(procInfo.lexicals, lexicalsSize));

    // there must be enough bytes to read the entire proc trailer
    if (it.bytesLeft() < trailerSize)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc trailer");
    auto codeSize = it.bytesLeft() - trailerSize;

    // calculate the code span
    TU_ASSERT (it.nextSlice(procInfo.code, codeSize));

    // calculate the trailer span
    TU_ASSERT (it.nextSlice(procInfo.trailer, trailerSize));

    // assert that we have read all bytes in the proc
    TU_ASSERT (it.isValid() == false);
    return {};
}

tempo_utils::Status
lyric_object::parse_proc_activation(std::span<const tu_uint8> bytecode, tu_uint32 offset, ActivationInfo &activationInfo)
{
    std::span<const tu_uint8> proc;
    TU_RETURN_IF_NOT_OK (parse_proc(bytecode, offset, proc));

    // construct iterator for the proc
    tempo_utils::BytesIterator it(proc);

    // there must be enough bytes to read the entire proc header
    if (it.bytesLeft() < kProcHeaderSizeInBytes)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc header");

    TU_ASSERT (it.nextU16(activationInfo.num_arguments));
    TU_ASSERT (it.nextU16(activationInfo.num_locals));
    TU_ASSERT (it.nextU16(activationInfo.num_lexicals));

    return {};
}

tempo_utils::Status
lyric_object::parse_lexicals_table(const ProcInfo &procInfo, std::vector<ProcLexical> &lexicals)
{
    if (procInfo.lexicals.size() != procInfo.num_lexicals * kProcLexicalSizeInBytes)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc lexicals table");

    tempo_utils::BytesIterator it(procInfo.lexicals);

    lexicals.resize(procInfo.num_lexicals);
    for (int i = 0; i < procInfo.num_lexicals; i++) {
        ProcLexical &lexical = lexicals.at(i);
        TU_ASSERT (it.nextU32(lexical.activation_call));
        TU_ASSERT (it.nextU32(lexical.target_offset));
        TU_ASSERT (it.nextU8(lexical.lexical_target));
    }

    // assert that we have read all bytes in the lexicals table
    TU_ASSERT (it.isValid() == false);
    return {};
}

tempo_utils::Status
lyric_object::parse_proc_trailer(const ProcInfo &procInfo, TrailerInfo &trailerInfo)
{
    // special case: it is not an error if the trailer is empty
    if (procInfo.trailer.empty()) {
        trailerInfo.num_checks = 0;
        trailerInfo.num_exceptions = 0;
        trailerInfo.num_cleanups = 0;
        trailerInfo.checks = {};
        trailerInfo.exceptions = {};
        trailerInfo.cleanups = {};
        return {};
    }

    tempo_utils::BytesIterator it(procInfo.trailer);

    if (it.bytesLeft() < kProcTrailerSizeInBytes)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc trailer");

    TU_ASSERT (it.nextU16(trailerInfo.num_checks));
    TU_ASSERT (it.nextU16(trailerInfo.num_exceptions));
    TU_ASSERT (it.nextU16(trailerInfo.num_cleanups));

    // there must be enough bytes to read the entire checks table
    auto checksSize = trailerInfo.num_checks * kProcCheckSizeInBytes;
    if (it.bytesLeft() < checksSize)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc trailer");

    // calculate the checks span
    TU_ASSERT (it.nextSlice(trailerInfo.checks, checksSize));

    // there must be enough bytes to read the entire exception table
    auto exceptionsSize = trailerInfo.num_exceptions * kProcExceptionSizeInBytes;
    if (it.bytesLeft() < exceptionsSize)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc trailer");

    // calculate the exceptions span
    TU_ASSERT (it.nextSlice(trailerInfo.exceptions, exceptionsSize));

    // there must be enough bytes to read the entire cleanups table
    auto cleanupsSize = trailerInfo.num_cleanups * kProcCleanupSizeInBytes;
    if (it.bytesLeft() < cleanupsSize)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc trailer");

    // calculate the cleanups span
    TU_ASSERT (it.nextSlice(trailerInfo.cleanups, cleanupsSize));

    // assert that we have read all bytes in the trailer
    TU_ASSERT (it.isValid() == false);
    return {};
}

tempo_utils::Status
lyric_object::parse_checks_table(const TrailerInfo &trailerInfo, std::vector<ProcCheck> &checks)
{
    if (trailerInfo.checks.size() != trailerInfo.num_checks * kProcCheckSizeInBytes)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc checks table");

    tempo_utils::BytesIterator it(trailerInfo.checks);

    checks.resize(trailerInfo.num_checks);
    for (int i = 0; i < trailerInfo.num_checks; i++) {
        ProcCheck &check = checks.at(i);
        TU_ASSERT (it.nextU32(check.interval_offset));
        TU_ASSERT (it.nextU32(check.interval_size));
        TU_ASSERT (it.nextU16(check.first_exception));
        TU_ASSERT (it.nextU16(check.num_exceptions));
        TU_ASSERT (it.nextU16(check.exception_local));
    }

    // assert that we have read all bytes in the checks table
    TU_ASSERT (it.isValid() == false);
    return {};
}

tempo_utils::Status
lyric_object::parse_exceptions_table(const TrailerInfo &trailerInfo, std::vector<ProcException> &exceptions)
{
    if (trailerInfo.exceptions.size() != trailerInfo.num_exceptions * kProcExceptionSizeInBytes)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc exceptions table");

    tempo_utils::BytesIterator it(trailerInfo.exceptions);

    exceptions.resize(trailerInfo.num_exceptions);
    for (int i = 0; i < trailerInfo.num_exceptions; i++) {
        ProcException &exc = exceptions.at(i);
        TU_ASSERT (it.nextU32(exc.exception_type));
        TU_ASSERT (it.nextU32(exc.catch_offset));
        TU_ASSERT (it.nextU32(exc.catch_size));
    }

    // assert that we have read all bytes in the checks table
    TU_ASSERT (it.isValid() == false);
    return {};
}

tempo_utils::Status
lyric_object::parse_cleanups_table(const TrailerInfo &trailerInfo, std::vector<ProcCleanup> &cleanups)
{
    if (trailerInfo.cleanups.size() != trailerInfo.num_cleanups * kProcCleanupSizeInBytes)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc cleanups table");

    tempo_utils::BytesIterator it(trailerInfo.cleanups);

    cleanups.resize(trailerInfo.num_cleanups);
    for (int i = 0; i < trailerInfo.num_cleanups; i++) {
        ProcCleanup &cleanup = cleanups.at(i);
        TU_ASSERT (it.nextU32(cleanup.interval_offset));
        TU_ASSERT (it.nextU32(cleanup.interval_size));
        TU_ASSERT (it.nextU16(cleanup.parent_cleanup));
        TU_ASSERT (it.nextU32(cleanup.cleanup_offset));
        TU_ASSERT (it.nextU32(cleanup.cleanup_size));
    }

    // assert that we have read all bytes in the checks table
    TU_ASSERT (it.isValid() == false);
    return {};
}
