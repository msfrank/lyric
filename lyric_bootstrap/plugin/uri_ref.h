#ifndef ZURI_CORE_URI_REF_H
#define ZURI_CORE_URI_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/url.h>

class UrlRef : public lyric_runtime::BaseRef {

public:
    explicit UrlRef(const lyric_runtime::VirtualTable *vtable);
    ~UrlRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    bool equals(const AbstractRef *other) const override;
    bool uriValue(tempo_utils::Url &url) const override;
    bool hashValue(absl::HashState state) override;
    bool serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index) override;
    std::string toString() const override;

    lyric_runtime::DataCell uriEquals(UrlRef *other) const;

    tempo_utils::Url getUrl() const;
    void setUrl(const tempo_utils::Url &uri);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    tempo_utils::Url m_uri;
};

tempo_utils::Status uri_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status uri_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status uri_equals(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_CORE_URI_REF_H
