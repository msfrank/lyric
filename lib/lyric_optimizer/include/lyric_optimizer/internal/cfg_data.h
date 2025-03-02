#ifndef LYRIC_OPTIMIZER_INTERNAL_CFG_DATA_H
#define LYRIC_OPTIMIZER_INTERNAL_CFG_DATA_H

#include <vector>

#include <absl/container/flat_hash_map.h>

#include <lyric_object/object_types.h>

#include "cfg_types.h"

#include "../directive_chain.h"
#include "../instance.h"
#include "../variable.h"

namespace lyric_optimizer::internal {

    // forward declarations
    struct BasicBlockPriv;
    struct InstancePriv;

    struct TransferPriv {
        Edge transferEdge;
        lyric_object::Opcode trailer = lyric_object::Opcode::OP_UNKNOWN;
    };

    struct PrevPriv {
        Edge prevEdge;
    };

    struct NextPriv {
        Edge nextEdge;
    };

    struct VariablePriv {
        std::string name;
        std::weak_ptr<BasicBlockPriv> declaration;
        std::vector<std::shared_ptr<InstancePriv>> instances;
        tu_uint32 counter;
    };

    struct InstancePriv {
        std::weak_ptr<VariablePriv> variable;
        tu_uint32 generation;
    };

    struct PhiPriv {
        Instance target;
        std::vector<Instance> arguments;
    };

    struct BasicBlockPriv {
        Vertex blockVertex;
        std::string name;
        absl::flat_hash_map<std::string,std::shared_ptr<VariablePriv>> declarations;
        absl::flat_hash_map<std::string,std::shared_ptr<PhiPriv>> phis;
        std::vector<std::shared_ptr<DirectiveChain>> directives;
        std::unique_ptr<TransferPriv> transfer;
        std::unique_ptr<PrevPriv> prev;
        std::unique_ptr<NextPriv> next;
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
    };
}

#endif // LYRIC_OPTIMIZER_INTERNAL_CFG_DATA_H
