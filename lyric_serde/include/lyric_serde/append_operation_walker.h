#ifndef LYRIC_SERDE_APPEND_OPERATION_WALKER_H
#define LYRIC_SERDE_APPEND_OPERATION_WALKER_H

#include "operation_path.h"
#include "serde_types.h"
#include "value_walker.h"

namespace lyric_serde {

    class AppendOperationWalker {

    public:
        AppendOperationWalker();
        AppendOperationWalker(const AppendOperationWalker &other);

        bool isValid() const;

        OperationPath getPath() const;
        ValueWalker getValue() const;

    private:
        std::shared_ptr<const internal::PatchsetReader> m_reader;
        tu_uint32 m_index;

        AppendOperationWalker(std::shared_ptr<const internal::PatchsetReader> reader, tu_uint32 index);
        friend class ChangeWalker;
    };
}

#endif // LYRIC_SERDE_APPEND_OPERATION_WALKER_H
