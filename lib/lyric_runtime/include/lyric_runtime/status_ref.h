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

        bool getField(const Operand &field, Operand &value) const override;
        bool setField(const Operand &field, const Operand &value, Operand *prev) override;
        std::string toString() const override;

        tempo_utils::StatusCode statusCode() override;
        std::string statusMessage() override;

        Operand getStatusCode() const;
        void setStatusCode(tempo_utils::StatusCode statusCode);

        Operand getMessage() const;
        void setMessage(const Operand &message);

    protected:
        void setMembersReachable() override;
        void clearMembersReachable() override;

    private:
        tempo_utils::StatusCode m_statusCode;
        StringRef *m_message;
        std::vector<Operand> m_fields;
    };
}

#endif // LYRIC_RUNTIME_STATUS_REF_H