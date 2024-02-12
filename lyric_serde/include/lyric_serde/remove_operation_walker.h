#ifndef LYRIC_SERDE_REMOVE_OPERATION_WALKER_H
#define LYRIC_SERDE_REMOVE_OPERATION_WALKER_H

#include "operation_path.h"
#include "serde_types.h"
#include "value_walker.h"

namespace lyric_serde {

    class RemoveOperationWalker {

    public:
        RemoveOperationWalker();
        RemoveOperationWalker(const RemoveOperationWalker &other);

        bool isValid() const;

        OperationPath getPath() const;

    private:
        std::shared_ptr<const internal::PatchsetReader> m_reader;
        tu_uint32 m_index;

        RemoveOperationWalker(std::shared_ptr<const internal::PatchsetReader> reader, tu_uint32 index);
        friend class ChangeWalker;
    };
}

#endif // LYRIC_SERDE_REMOVE_OPERATION_WALKER_H
