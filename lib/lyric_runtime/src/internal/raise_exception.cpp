
#include <lyric_runtime/internal/raise_exception.h>

tempo_utils::Status
lyric_runtime::internal::raise_exception(
    const lyric_object::OpCell &op,
    const DataCell &exc,
    StackfulCoroutine *currentCoro,
    SegmentManager *segmentManager,
    SubroutineManager *subroutineManager,
    TypeManager *typeManager)
{
    CallCell *frame = nullptr;
    BytecodeSegment *segment = nullptr;
    std::vector<lyric_object::ProcCheck> checks;
    std::vector<lyric_object::ProcException> exceptions;
    lyric_object::CallWalker call;
    int exception_match = -1;

    // get the runtime type of the exception
    DataCell excType;
    TU_ASSIGN_OR_RETURN (excType, typeManager->typeOf(exc));

    while (!currentCoro->callStackEmpty()) {
        TU_RETURN_IF_NOT_OK (currentCoro->peekCall(&frame));

        auto segmentIndex = frame->getCallSegment();
        segment = segmentManager->getSegment(segmentIndex);
        TU_ASSERT (segment != nullptr);

        auto object = segment->getObject();
        call = object.getCall(frame->getCallIndex());
        TU_ASSERT (call.isValid());

        lyric_object::ProcInfo procInfo;
        TU_ASSIGN_OR_RETURN (procInfo, call.getProcInfo());

        lyric_object::TrailerInfo trailerInfo;
        TU_RETURN_IF_NOT_OK (lyric_object::parse_proc_trailer(procInfo, trailerInfo));
        TU_RETURN_IF_NOT_OK (lyric_object::parse_checks_table(trailerInfo, checks));
        TU_RETURN_IF_NOT_OK (lyric_object::parse_exceptions_table(trailerInfo, exceptions));

        // find the most specific exception entry in the current activation which matches the exception
        for (int i = 0; exception_match < 0 &&  i < checks.size(); i++) {
            const auto &check = checks.at(i);
            if (check.interval_offset <= op.offset && op.offset < (check.interval_offset + check.interval_size)) {
                for (int j = 0; exception_match < 0 && j < check.num_exceptions; j++) {
                    const auto &exception = exceptions.at(check.first_exception + j);
                    auto *typeEntry = segment->lookupType(exception.exception_type);
                    auto catchType = DataCell::forType(typeEntry);
                    TypeComparison cmp;
                    TU_ASSIGN_OR_RETURN (cmp, typeManager->compareTypes(excType, catchType));
                    switch (cmp) {
                        case TypeComparison::EQUAL:
                        case TypeComparison::EXTENDS:
                            exception_match = check.first_exception + j;
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        // if there is a matching exception then run the catch handler
        if (0 <= exception_match)
            break;

        // otherwise return to caller and continue searching for a matching catch
        tempo_utils::Status status;
        if (!subroutineManager->returnToCaller(currentCoro, status))
            return status;
    }

    // if no catch handler was found then halt the interpreter
    if (exception_match < 0)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "no handler found for exception");

    // otherwise construct a new iterator starting at the exception catch and transfer control
    auto &exception = exceptions.at(exception_match);
    auto ip = currentCoro->peekIP();
    ip.reset(exception.catch_offset);
    TU_ASSERT (ip.isValid());
    currentCoro->transferControl(ip);

    return {};
}
