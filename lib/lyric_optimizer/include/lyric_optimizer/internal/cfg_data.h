#ifndef LYRIC_OPTIMIZER_INTERNAL_CFG_DATA_H
#define LYRIC_OPTIMIZER_INTERNAL_CFG_DATA_H

#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_object/object_types.h>

#include "cfg_types.h"

#include "../directive_chain.h"
#include "../instance.h"
#include "../phi_function.h"
#include "../value.h"
#include "../variable.h"

namespace lyric_optimizer::internal {

    // forward declarations
    struct BasicBlockPriv;
    struct InstancePriv;

    constexpr tu_uint32 kCounterMax = 0xFFFFFFFF;

    struct PrevPriv {
        Edge prevEdge;
    };

    struct NextPriv {
        Edge nextEdge;
        std::shared_ptr<AbstractDirective> operand;
    };

    struct TransferPriv {
        Edge transferEdge;
        lyric_object::Opcode trailer = lyric_object::Opcode::OP_UNKNOWN;
        std::shared_ptr<AbstractDirective> operand;
    };

    struct ValuePriv {
        std::forward_list<std::shared_ptr<AbstractDirective>> values;
    };

    struct VariablePriv {
        std::string name;
        lyric_assembler::SymbolType type;
        int offset;
        std::vector<std::shared_ptr<InstancePriv>> instances;
        tu_uint32 counter;
    };

    struct InstancePriv {
        std::weak_ptr<VariablePriv> variable;
        tu_uint32 generation;
        Value value;
    };

    struct BasicBlockPriv {
        tu_uint32 id;
        Vertex blockVertex;
        std::string name;
        absl::flat_hash_set<Instance> phis;
        std::vector<std::shared_ptr<DirectiveChain>> directives;
        std::unique_ptr<PrevPriv> prev;
        std::unique_ptr<NextPriv> next;
        std::unique_ptr<TransferPriv> transfer;
    };

    struct GraphPriv {
        Graph graph;
        std::shared_ptr<BasicBlockPriv> entry;
        std::shared_ptr<BasicBlockPriv> exit;
        absl::flat_hash_map<Vertex,std::shared_ptr<BasicBlockPriv>> vertexBlocks;
        absl::flat_hash_map<std::string,std::shared_ptr<BasicBlockPriv>> labeledBlocks;
        absl::flat_hash_map<std::string,std::shared_ptr<VariablePriv>> variables;
        std::vector<std::shared_ptr<VariablePriv>> arguments;
        std::vector<std::shared_ptr<VariablePriv>> locals;
        std::vector<std::shared_ptr<VariablePriv>> lexicals;
        tu_uint32 counter;
    };
}

#endif // LYRIC_OPTIMIZER_INTERNAL_CFG_DATA_H
