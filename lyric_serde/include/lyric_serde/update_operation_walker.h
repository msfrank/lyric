#ifndef LYRIC_SERDE_UPDATE_OPERATION_WALKER_H
#define LYRIC_SERDE_UPDATE_OPERATION_WALKER_H

#include "operation_path.h"
#include "serde_types.h"
#include "value_walker.h"

namespace lyric_serde {

    class UpdateOperationWalker {

    public:
        UpdateOperationWalker();
        UpdateOperationWalker(const UpdateOperationWalker &other);

        bool isValid() const;

        OperationPath getPath() const;
        tu_int16 getNs() const;
        tu_uint32 getId() const;
        ValueWalker getValue() const;

    private:
        std::shared_ptr<const internal::PatchsetReader> m_reader;
        tu_uint32 m_index;

        UpdateOperationWalker(std::shared_ptr<const internal::PatchsetReader> reader, tu_uint32 index);
        friend class ChangeWalker;
    };
}

#endif // LYRIC_SERDE_UPDATE_OPERATION_WALKER_H
