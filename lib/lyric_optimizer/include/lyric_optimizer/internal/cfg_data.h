#ifndef LYRIC_OPTIMIZER_INTERNAL_CFG_DATA_H
#define LYRIC_OPTIMIZER_INTERNAL_CFG_DATA_H

#include <vector>

#include <absl/container/flat_hash_map.h>

#include <lyric_assembler/abstract_instruction.h>
#include <lyric_common/symbol_url.h>

#include "cfg_types.h"

namespace lyric_optimizer::internal {

    struct LabelPriv {
        Edge labelEdge;
        lyric_object::Opcode trailer = lyric_object::Opcode::OP_UNKNOWN;
    };

    struct NextPriv {
        Edge nextEdge;
    };

    struct BasicBlockPriv : public AbstractBlock {
        Vertex blockVertex;
        std::string name;
        std::vector<std::shared_ptr<lyric_assembler::BasicInstruction>> instructions;
        std::unique_ptr<LabelPriv> label;
        std::unique_ptr<NextPriv> next;
    };

    struct GraphPriv {
        Graph graph;
        std::shared_ptr<BasicBlockPriv> entry;
        std::shared_ptr<BasicBlockPriv> exit;
        absl::flat_hash_map<Vertex,std::shared_ptr<BasicBlockPriv>> vertexBlocks;
        absl::flat_hash_map<std::string,std::shared_ptr<BasicBlockPriv>> labeledBlocks;
    };
}

#endif // LYRIC_OPTIMIZER_INTERNAL_CFG_DATA_H
