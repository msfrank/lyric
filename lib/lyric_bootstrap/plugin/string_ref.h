#ifndef ZURI_CORE_STRING_REF_H
#define ZURI_CORE_STRING_REF_H

#include <unicode/ustring.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

//class StringRef : public lyric_runtime::BaseRef {
//
//public:
//    explicit StringRef(const lyric_runtime::VirtualTable *vtable);
//    StringRef(const lyric_runtime::VirtualTable *vtable, const UChar *data, int32_t size);
//    ~StringRef() override;
//
//    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
//    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
//    bool equals(const AbstractRef *other) const override;
//    bool rawSize(tu_int32 &size) const override;
//    tu_int32 rawCopy(tu_int32 offset, char *dst, tu_int32 size) override;
//    bool utf8Value(std::string &utf8) const override;
//    bool hashValue(absl::HashState state) override;
//    bool serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index) override;
//    std::string toString() const override;
//
//    lyric_runtime::DataCell stringAt(int index) const;
//    lyric_runtime::DataCell stringCompare(StringRef *other) const;
//    lyric_runtime::DataCell stringLength() const;
//
//    const char *getStringData() const;
//    int32_t getStringSize() const;
//    void setStringData(const char *data, int32_t size);
//
//protected:
//    void setMembersReachable() override;
//    void clearMembersReachable() override;
//
//private:
//    char *m_data;
//    int32_t m_size;
//};

//tempo_utils::Status string_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
//tempo_utils::Status string_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status string_at(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status string_compare(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status string_length(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_CORE_STRING_REF_H