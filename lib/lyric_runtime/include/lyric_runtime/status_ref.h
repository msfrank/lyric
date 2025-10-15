#ifndef LYRIC_RUNTIME_STATUS_REF_H
#define LYRIC_RUNTIME_STATUS_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

namespace lyric_runtime {

    class StatusRef : public BaseRef {

    public:
        explicit StatusRef(const VirtualTable *vtable);
        StatusRef(const VirtualTable *vtable, tempo_utils::StatusCode statusCode, StringRef *message);
        ~StatusRef() override;

        bool getField(const DataCell &field, DataCell &value) const override;
        bool setField(const DataCell &field, const DataCell &value, DataCell *prev) override;
        std::string toString() const override;

        tempo_utils::StatusCode errorStatusCode() override;
        DataCell getStatusCode() const;
        void setStatusCode(tempo_utils::StatusCode statusCode);

        std::string errorMessage() override;
        DataCell getMessage() const;
        void setMessage(const DataCell &message);

    protected:
        void setMembersReachable() override;
        void clearMembersReachable() override;

    private:
        tempo_utils::StatusCode m_statusCode;
        StringRef *m_message;
        std::vector<DataCell> m_fields;
    };
}

#endif // LYRIC_RUNTIME_STATUS_REF_H