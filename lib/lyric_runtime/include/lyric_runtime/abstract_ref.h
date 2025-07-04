#ifndef LYRIC_RUNTIME_ABSTRACT_REF_H
#define LYRIC_RUNTIME_ABSTRACT_REF_H

#include <absl/hash/hash.h>

#include "system_scheduler.h"
#include "virtual_table.h"

namespace lyric_runtime {

    class AbstractRef {

    public:
        virtual ~AbstractRef() = default;

        /**
         * Returns the symbol url for the ref.
         *
         * @return The symbol url for the ref.
         */
        virtual lyric_common::SymbolUrl getSymbolUrl() const = 0;

        /**
         * Returns the member resolver for the ref. If the ref does not support members then returns nullptr.
         *
         * @return The member resolver for the ref.
         */
        virtual const AbstractMemberResolver *getMemberResolver() const = 0;

        /**
         * Returns the method resolver for the ref. If the ref does not support methods then returns nullptr.
         *
         * @return The method resolver for the ref.
         */
        virtual const AbstractMethodResolver *getMethodResolver() const = 0;

        /**
         * Returns the extension resolver for the ref. If the ref does not support extensions then returns nullptr.
         *
         * @return The extension resolver for the ref.
         */
        virtual const AbstractExtensionResolver *getExtensionResolver() const = 0;

        /**
         * returns a data cell containing the value stored in the specified field. If `field` does not
         * contain a valid field descriptor, or if there is no such field for the ref, or if the ref type
         * does not support accessing fields, then the method must return an invalid data cell.
         *
         * @param field A data cell containing a field descriptor.
         * @return A data cell containing the field value, or an invalid data cell if there was an error.
         */
        virtual DataCell getField(const DataCell &field) const = 0;

        /**
         * Sets the value of the field specified by `field` to `value`, and returns a data cell containing
         * the previous value of the field. If `field` does not contain a valid field descriptor, or if
         * there is no such field for the ref, of if the ref type does not support updating fields, then
         * the method must return an invalid data cell.
         *
         * @param field A data cell containing the field descriptor.
         * @param value A data cell containing the new field value.
         * @return A data cell containing the previous value, or an invalid data cell if there was an error.
         */
        virtual DataCell setField(const DataCell &field, const DataCell &value) = 0;

        /**
         * Returns whether the value of the ref equals the value of other.
         *
         * @return true if the ref value equals the value of other, otherwise false.
         */
        virtual bool equals(const AbstractRef *other) const = 0;

        /**
         * Determines the size of the raw data representing the ref and assigns it to `size`. If the ref type
         * does not raw copying then `size` is undefined.
         *
         * @return true if `size` was assigned, otherwise false.
         */
        virtual bool rawSize(tu_int32 &size) const = 0;

        /**
         * Copy up to `size` bytes from the ref into `dst`, starting from `offset`, and return the count of
         * bytes which were copied. If dst is nullptr, or if the ref type does not support raw copying, then
         * the method must return -1. If `size` is larger than the buffer pointed to by `dst`, then the result
         * is undefined (and will very likely cause a segfault!)
         *
         * @return The number of bytes copied, or -1 if there was an error.
         */
        virtual tu_int32 rawCopy(tu_int32 offset, char *dst, tu_int32 size) = 0;

        /**
         * Assigns the value of the ref as a UTF-8 encoded string to `utf8`. If the ref is not representable
         * as a string then `utf8` is undefined.
         *
         * @return true if conversion was successful and `utf8` was assigned, otherwise false.
         */
        virtual bool utf8Value(std::string &utf8) const = 0;

        /**
         * Assigns the value of the ref as a url to `url`. If the ref is not representable as a url then `url`
         * is undefined.
         *
         * @return true if conversion was successful and `url` was assigned, otherwise false.
         */
        virtual bool uriValue(tempo_utils::Url &url) const = 0;

        /**
         * Updates the specified `state` by hashing the content of the ref. If the ref does not support
         * hashing then `state` is untouched and the method must return false.
         *
         * @return true if the state was updated, otherwise false.
         */
        virtual bool hashValue(absl::HashState state) = 0;

        /**
         * Returns whether the ref supports iteration and contains at least one element.
         *
         * @return true if the ref is a valid iterator, otherwise false.
         */
        virtual bool iteratorValid() = 0;

        /**
         * Assigns the next iterator element to `next`. If the ref does not support iteration, or the ref
         * does not contain any remaining elements, then `next` is untouched and the method must return false.
         *
         * @return true if `next` was assigned, otherwise false.
         */
        virtual bool iteratorNext(DataCell &next) = 0;

        /**
         * Assigns the specified `promise` to the ref. If the ref type does not support the futures protocol
         * then `promise` is untouched and the method must return false.
         *
         * @param promise
         * @return true if `promise` was assigned, otherwise false.
         */
        virtual bool prepareFuture(std::shared_ptr<Promise> promise) = 0;

        /**
         * Instructs the ref to await the promise(es) assigned to the ref, suspending the current task if
         * necessary. If the ref type does not support the futures protocol then side effects must not occur
         * and the method must return false.
         *
         * @return true if the ref is awaiting, otherwise false.
         */
        virtual bool awaitFuture(SystemScheduler *systemScheduler) = 0;

        /**
         * Assigns the result of the future to `result`. If the ref type does not support the futures protocol
         * or if there is no result yet then the method must return false.
         *
         * @param result
         * @return true if a result is available, otherwise false.
         */
        virtual bool resolveFuture(DataCell &result) = 0;

        /**
         * Modify the specified `task` such that the closure contained in the ref is immediately invoked when
         * the task resumes. If the ref type does not support the closure protocol or if the task is in Done
         * state then side effects must not occur and the method must return false.
         *
         * @param task
         * @return true if the closure call frame has been pushed onto the task call stack, otherwise false.
         */
        virtual bool applyClosure(Task *task, std::vector<DataCell> &args, InterpreterState *state) = 0;

        /**
         * Returns the error status code. If the ref is not a subtype of Status then the method must return kOk.
         *
         * @return The `tempo_utils::StatusCode` containing the error status.
         */
        virtual tempo_utils::StatusCode errorStatusCode() = 0;

        /**
         * Generate a human-readable representation which describes the ref.
         *
         * @return A string describing the ref.
         */
        virtual std::string toString() const = 0;

        /**
         * Returns whether or not the ref has been marked reachable during the GC mark phase.
         *
         * @return true if the ref is marked as reachable, otherwise false.
         */
        virtual bool isReachable() const = 0;

        /**
         * Sets the ref as reachable.
         */
        virtual void setReachable() = 0;

        /**
         * Clears the reachable status.
         */
        virtual void clearReachable() = 0;

        /**
         * Invoked by the GC as the last step before deallocation.
         */
        virtual void finalize() = 0;
    };
}

#endif // LYRIC_RUNTIME_ABSTRACT_REF_H