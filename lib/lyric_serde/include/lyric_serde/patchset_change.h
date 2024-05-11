#ifndef LYRIC_SERDE_PATCHSET_CHANGE_H
#define LYRIC_SERDE_PATCHSET_CHANGE_H

#include <filesystem>

#include <tempo_utils/attr.h>

#include "operation_path.h"
#include "patchset_state.h"
#include "serde_types.h"

namespace lyric_serde {

    class PatchsetChange {

    public:
        PatchsetChange(std::string_view id, ChangeAddress address, PatchsetState *state);

        std::string getId() const;
        ChangeOperation getOperation() const;
        ChangeAddress getAddress() const;

        OperationPath getOperationPath() const;
        ValueAddress getOperationValue() const;
        tu_uint32 getOperationIndex() const;
        tu_int16 getOperationNsKey() const;
        tu_uint32 getOperationIdValue() const;

        void setAppendOperation(const OperationPath &path, ValueAddress value);
        void setInsertOperation(const OperationPath &path, tu_uint32 index, ValueAddress value);
        void setUpdateOperation(
            const OperationPath &path,
            const char *nsUrl,
            tu_uint32 idValue,
            ValueAddress value);
        void setReplaceOperation(const OperationPath &path, ValueAddress value);
        void setRemoveOperation(const OperationPath &path);
        void setEmitOperation(ValueAddress value);
        //void setInvokeOperation();
        //void setAcceptOperation();

    private:
        std::string m_id;
        ChangeAddress m_address;
        PatchsetState *m_state;
        ChangeOperation m_operation;
        OperationPath m_path;
        ValueAddress m_value;
        tu_uint32 m_index;
        tu_int16 m_nsKey;
        tu_uint32 m_idValue;

    public:
        /**
         *
         * @tparam NsType
         * @tparam IdType
         * @param path
         * @param property
         * @param value
         */
        template <class NsType, class IdType>
        void setUpdateOperation(
            const OperationPath &path,
            const tempo_utils::SchemaProperty<NsType,IdType> &property,
            ValueAddress value);
    };
}

#endif // LYRIC_SERDE_PATCHSET_CHANGE_H