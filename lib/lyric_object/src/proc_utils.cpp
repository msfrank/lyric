
#include <lyric_object/object_result.h>
#include <lyric_object/proc_utils.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/status.h>

tempo_utils::Status
lyric_object::parse_proc_info(std::span<const tu_uint8> bytecode, tu_uint32 offset, ProcInfo &procInfo)
{
    // there must be 4 bytes available to read starting at offset within the bytecode span
    if (bytecode.size() <= offset + 4)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc offset");

    // calculate the proc span
    const uint8_t *ptr = bytecode.data() + offset;
    tu_uint32 nleft = tempo_utils::read_u32_and_advance(ptr);
    if (bytecode.size() < offset + 4 + nleft)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc header");
    procInfo.proc = std::span(ptr, nleft);

    // there must be enough bytes to read the entire proc header
    if (nleft < kProcHeaderSizeInBytes)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc header");

    procInfo.num_arguments = tempo_utils::read_u16_and_advance(ptr);
    procInfo.num_locals = tempo_utils::read_u16_and_advance(ptr);
    procInfo.num_lexicals = tempo_utils::read_u16_and_advance(ptr);
    auto trailerSize = tempo_utils::read_u32_and_advance(ptr);
    nleft -= 10;

    // there must be enough bytes to read the entire lexicals table
    auto lexicalsSize = procInfo.num_lexicals * kProcLexicalSizeInBytes;
    if (nleft < lexicalsSize)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc header");

    // calculate the lexicals span
    procInfo.lexicals = std::span(ptr, lexicalsSize);
    ptr += lexicalsSize;
    nleft -= lexicalsSize;

    // there must be enough bytes to read the entire proc trailer
    if (nleft < trailerSize)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc trailer");
    auto codeSize = nleft - trailerSize;

    // calculate the code span
    procInfo.code = std::span(ptr, codeSize);
    ptr += codeSize;
    nleft -= codeSize;

    // calculate the trailer span
    procInfo.trailer = std::span(ptr, trailerSize);

    // assert that we have read all bytes in the proc
    TU_ASSERT (nleft - trailerSize == 0);
    TU_ASSERT (ptr + trailerSize == procInfo.proc.data() + procInfo.proc.size());

    return {};
}

tempo_utils::Status
lyric_object::parse_lexicals_table_entry(const ProcInfo &procInfo, int index, ProcLexical &lexical)
{
    if (procInfo.num_lexicals <= index)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc lexical at index {}", index);

    const tu_uint8 *ptr = procInfo.lexicals.data();
    ptr += kProcLexicalSizeInBytes * index;
    lexical.activation_call = tempo_utils::read_u32_and_advance(ptr);
    lexical.target_offset = tempo_utils::read_u32_and_advance(ptr);
    lexical.lexical_target = tempo_utils::read_u8_and_advance(ptr);
    return {};
}

tempo_utils::Status
lyric_object::parse_lexicals_table(const ProcInfo &procInfo, std::vector<ProcLexical> &lexicals)
{
    const tu_uint8 *ptr = procInfo.lexicals.data();
    auto nleft = procInfo.lexicals.size();

    lexicals.resize(procInfo.num_lexicals);

    for (int i = 0; i < procInfo.num_lexicals; i++) {
        if (nleft < kProcLexicalSizeInBytes)
            return ObjectStatus::forCondition(
                ObjectCondition::kObjectInvariant, "invalid proc lexical");
        ProcLexical &lexical = lexicals.at(i);
        lexical.activation_call = tempo_utils::read_u32_and_advance(ptr);
        lexical.target_offset = tempo_utils::read_u32_and_advance(ptr);
        lexical.lexical_target = tempo_utils::read_u8_and_advance(ptr);
        nleft -= kProcLexicalSizeInBytes;
    }

    // assert that we have read all bytes in the lexicals table
    TU_ASSERT (ptr == procInfo.lexicals.data() + procInfo.lexicals.size());
    TU_ASSERT (nleft == 0);

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

    const tu_uint8 *ptr = procInfo.trailer.data();
    auto nleft = procInfo.trailer.size();

    if (nleft < kProcTrailerSizeInBytes)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc trailer");

    trailerInfo.num_checks = tempo_utils::read_u16_and_advance(ptr);
    trailerInfo.num_exceptions = tempo_utils::read_u16_and_advance(ptr);
    trailerInfo.num_cleanups = tempo_utils::read_u16_and_advance(ptr);
    nleft -= kProcTrailerSizeInBytes;

    // there must be enough bytes to read the entire checks table
    auto checksSize = trailerInfo.num_checks * kProcCheckSizeInBytes;
    if (nleft < checksSize)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc trailer");

    // calculate the checks span
    trailerInfo.checks = std::span(ptr, checksSize);
    ptr += checksSize;
    nleft -= checksSize;

    // there must be enough bytes to read the entire exception table
    auto exceptionsSize = trailerInfo.num_exceptions * kProcExceptionSizeInBytes;
    if (nleft < exceptionsSize)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc trailer");

    // calculate the exceptions span
    trailerInfo.exceptions = std::span(ptr, exceptionsSize);
    ptr += exceptionsSize;
    nleft -= exceptionsSize;

    // there must be enough bytes to read the entire cleanups table
    auto cleanupsSize = trailerInfo.num_cleanups * kProcCleanupSizeInBytes;
    if (nleft < cleanupsSize)
        return ObjectStatus::forCondition(
            ObjectCondition::kObjectInvariant, "invalid proc trailer");

    // calculate the cleanups span
    trailerInfo.cleanups = std::span(ptr, cleanupsSize);

    // assert that we have read all bytes in the trailer
    TU_ASSERT (nleft - cleanupsSize == 0);
    TU_ASSERT (ptr + cleanupsSize == procInfo.trailer.data() + procInfo.trailer.size());

    return {};
}

tempo_utils::Status
lyric_object::parse_checks_table(const TrailerInfo &trailerInfo, std::vector<ProcCheck> &checks)
{
    const tu_uint8 *ptr = trailerInfo.checks.data();
    auto nleft = trailerInfo.checks.size();

    checks.resize(trailerInfo.num_checks);

    for (int i = 0; i < trailerInfo.num_checks; i++) {
        if (nleft < kProcCheckSizeInBytes)
            return ObjectStatus::forCondition(
                ObjectCondition::kObjectInvariant, "invalid proc check");
        ProcCheck &check = checks.at(i);
        check.interval_offset = tempo_utils::read_u32_and_advance(ptr);
        check.interval_size = tempo_utils::read_u32_and_advance(ptr);
        check.parent_check = tempo_utils::read_u16_and_advance(ptr);
        check.first_exception = tempo_utils::read_u16_and_advance(ptr);
        check.num_exceptions = tempo_utils::read_u16_and_advance(ptr);
        nleft -= kProcCheckSizeInBytes;
    }

    // assert that we have read all bytes in the checks table
    TU_ASSERT (ptr == trailerInfo.checks.data() + trailerInfo.checks.size());
    TU_ASSERT (nleft == 0);

    return {};
}

tempo_utils::Status
lyric_object::parse_exceptions_table(const TrailerInfo &trailerInfo, std::vector<ProcException> &exceptions)
{
    const tu_uint8 *ptr = trailerInfo.exceptions.data();
    auto nleft = trailerInfo.exceptions.size();

    exceptions.resize(trailerInfo.num_exceptions);

    for (int i = 0; i < trailerInfo.num_exceptions; i++) {
        if (nleft < kProcExceptionSizeInBytes)
            return ObjectStatus::forCondition(
                ObjectCondition::kObjectInvariant, "invalid proc exception");
        ProcException &exc = exceptions.at(i);
        exc.exception_type = tempo_utils::read_u32_and_advance(ptr);
        exc.catch_offset = tempo_utils::read_u32_and_advance(ptr);
        exc.catch_size = tempo_utils::read_u32_and_advance(ptr);
        nleft -= kProcExceptionSizeInBytes;
    }

    // assert that we have read all bytes in the checks table
    TU_ASSERT (ptr == trailerInfo.exceptions.data() + trailerInfo.exceptions.size());
    TU_ASSERT (nleft == 0);

    return {};
}

tempo_utils::Status
lyric_object::parse_cleanups_table(const TrailerInfo &trailerInfo, std::vector<ProcCleanup> &cleanups)
{
    const tu_uint8 *ptr = trailerInfo.cleanups.data();
    auto nleft = trailerInfo.cleanups.size();

    cleanups.resize(trailerInfo.num_cleanups);

    for (int i = 0; i < trailerInfo.num_cleanups; i++) {
        if (nleft < kProcCleanupSizeInBytes)
            return ObjectStatus::forCondition(
                ObjectCondition::kObjectInvariant, "invalid proc cleanup");
        ProcCleanup &cleanup = cleanups.at(i);
        cleanup.interval_offset = tempo_utils::read_u32_and_advance(ptr);
        cleanup.interval_size = tempo_utils::read_u32_and_advance(ptr);
        cleanup.parent_cleanup = tempo_utils::read_u16_and_advance(ptr);
        cleanup.cleanup_offset = tempo_utils::read_u32_and_advance(ptr);
        cleanup.cleanup_size = tempo_utils::read_u32_and_advance(ptr);
        nleft -= kProcCleanupSizeInBytes;
    }

    // assert that we have read all bytes in the checks table
    TU_ASSERT (ptr == trailerInfo.cleanups.data() + trailerInfo.cleanups.size());
    TU_ASSERT (nleft == 0);

    return {};
}
