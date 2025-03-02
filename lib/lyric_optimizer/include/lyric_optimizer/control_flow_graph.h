#ifndef LYRIC_OPTIMIZER_CONTROL_FLOW_GRAPH_H
#define LYRIC_OPTIMIZER_CONTROL_FLOW_GRAPH_H

#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/local_variable.h>

#include "basic_block.h"

namespace lyric_optimizer {

    // forward declarations
    namespace internal {
        struct GraphPriv;
    }

    class ControlFlowGraph {
    public:
        ControlFlowGraph();
        ControlFlowGraph(tu_int32 numArguments, tu_int32 numLocals, tu_int32 numLexicals);
        ControlFlowGraph(const ControlFlowGraph &other);

        bool isValid() const;

        BasicBlock getEntryBlock() const;
        BasicBlock getExitBlock() const;
        tempo_utils::Result<BasicBlock> addBasicBlock();
        int numBasicBlocks() const;

        Variable getArgument(int offset) const;
        Variable resolveArgument(lyric_assembler::ArgumentVariable *argumentVariable) const;
        int numArguments() const;

        Variable getLocal(int offset) const;
        Variable resolveLocal(lyric_assembler::LocalVariable *localVariable) const;
        int numLocals() const;

        Variable getLexical(int offset) const;
        Variable resolveLexical(lyric_assembler::LexicalVariable *lexicalVariable) const;
        int numLexicals() const;

        void print();

        std::shared_ptr<internal::GraphPriv> getPriv() const;

    private:
        std::shared_ptr<internal::GraphPriv> m_priv;

        friend class print_visitor;
    };
}

#endif // LYRIC_OPTIMIZER_CONTROL_FLOW_GRAPH_H
