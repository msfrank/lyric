#ifndef LYRIC_SERDE_CHANGE_WALKER_H
#define LYRIC_SERDE_CHANGE_WALKER_H

#include "append_operation_walker.h"
#include "emit_operation_walker.h"
#include "insert_operation_walker.h"
#include "operation_path.h"
#include "remove_operation_walker.h"
#include "replace_operation_walker.h"
#include "serde_types.h"
#include "update_operation_walker.h"
#include "value_walker.h"

namespace lyric_serde {

    class ChangeWalker {

    public:
        ChangeWalker();
        ChangeWalker(const ChangeWalker &other);

        bool isValid() const;

        tu_uint32 getIndex() const;

        std::string getId() const;
        ChangeOperation getOperationType() const;

        AppendOperationWalker getAppendOperation() const;
        EmitOperationWalker getEmitOperation() const;
        InsertOperationWalker getInsertOperation() const;
        RemoveOperationWalker getRemoveOperation() const;
        ReplaceOperationWalker getReplaceOperation() const;
        UpdateOperationWalker getUpdateOperation() const;

    private:
        std::shared_ptr<const internal::PatchsetReader> m_reader;
        tu_uint32 m_index;

        ChangeWalker(std::shared_ptr<const internal::PatchsetReader> reader, tu_uint32 index);
        friend class PatchsetWalker;
        friend class LyricPatchset;
    };
}

#endif // LYRIC_SERDE_CHANGE_WALKER_H
