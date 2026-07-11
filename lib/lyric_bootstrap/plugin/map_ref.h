#ifndef LYRIC_BOOTSTRAP_MAP_REF_H
#define LYRIC_BOOTSTRAP_MAP_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/operand.h>
#include <tempo_utils/hash_array_mapped_trie.h>

struct MapKey {
    lyric_runtime::Operand key;

    template <typename H>
    friend H AbslHashValue(H state, const MapKey &mapKey)
    {
        mapKey.key.hashEquality(absl::HashState::Create(&state));
        return std::move(state);
    }
};

class KeyHash {
public:
    size_t operator()(const MapKey &key);
};

class KeyEqual {
public:
    bool operator()(const MapKey &lhs, const MapKey &rhs) const;
};

typedef tempo_utils::HashArrayMappedTrie<
    MapKey,
    lyric_runtime::Operand,
    KeyHash,
    KeyEqual> OperandMap;

typedef tempo_utils::HamtIterator<
    MapKey,
    lyric_runtime::Operand,
    KeyHash,
    KeyEqual> OperandMapIterator;

class MapRef : public lyric_runtime::BaseRef {

public:
    explicit MapRef(const lyric_runtime::VirtualTable *vtable);
    ~MapRef() override;

    static constexpr tu_uint64 type_tag() { return 0x85f6246ec6b3cbbc; }

    tu_uint64 getTypeTag() const override;

    std::string toString() const override;

    OperandMap getMap() const;
    void setMap(const OperandMap &map);

    size_t numEntries();
    bool getEntry(const MapKey &key, lyric_runtime::Operand &value);
    bool containsEntry(const MapKey &key);
    OperandMap update(const MapKey &key, const lyric_runtime::Operand &value);
    OperandMap remove(const MapKey &key);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    OperandMap m_hamt;
};

class MapIterator : public lyric_runtime::BaseRef {

public:
    explicit MapIterator(const lyric_runtime::VirtualTable *vtable);
    MapIterator(const lyric_runtime::VirtualTable *vtable, OperandMap map);

    static constexpr tu_uint64 type_tag() { return 0xee3ff776c5745a9f; }

    tu_uint64 getTypeTag() const override;

    std::string toString() const override;

    bool iteratorValid() override;
    bool iteratorNext(lyric_runtime::Operand &cell) override;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    struct Priv {
        OperandMap hamt;
        OperandMapIterator iterator;
    };
    std::shared_ptr<Priv> m_priv;
};

tempo_utils::Status map_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status map_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status map_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status map_contains(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status map_get(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status map_update(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status map_remove(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status map_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status map_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status map_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status map_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // LYRIC_BOOTSTRAP_MAP_REF_H