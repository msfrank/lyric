#ifndef ZURI_CORE_MAP_REF_H
#define ZURI_CORE_MAP_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

enum class MapNodeType {
    VALUE,
    INDEX,
};

struct MapNode {
    MapNodeType type;
    int refcount;
};

struct ValueMapNode : public MapNode {
    lyric_runtime::DataCell key;
    lyric_runtime::DataCell value;
    tu_uint32 hash;
};

struct ChainMapNode : public MapNode {
    lyric_runtime::DataCell key;
    lyric_runtime::DataCell value;
    ChainMapNode *next;
};

constexpr int BITS_PER_LEVEL = 4;

constexpr int table_slots(int bitsPerLevel) {
    int numSlots = 1;
    for (int i = 0; i < bitsPerLevel; i++) {
        numSlots *= 2;
    }
    return numSlots;
}

typedef std::array<MapNode *,table_slots(BITS_PER_LEVEL)> IndexMapNodeTable;

struct IndexMapNode : public MapNode {
    IndexMapNodeTable table;
};

class MapRef : public lyric_runtime::BaseRef {

public:
    explicit MapRef(const lyric_runtime::VirtualTable *vtable);
    ~MapRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    MapNode *getNode() const;
    void setNode(MapNode *node);

    int mapSize() const;
    bool mapContains(const lyric_runtime::DataCell &key) const;
    lyric_runtime::DataCell mapGet(const lyric_runtime::DataCell &key) const;
    MapNode *mapUpdate(const lyric_runtime::DataCell &key, const lyric_runtime::DataCell &value) const;
    MapNode *mapRemove(const lyric_runtime::DataCell &key) const;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    MapNode *m_node;
};

tempo_utils::Status map_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status map_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status map_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status map_contains(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status map_get(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status map_update(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status map_remove(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_CORE_MAP_REF_H