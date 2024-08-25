#ifndef LYRIC_COMPILER_COMPILER_SCAN_DRIVER_H
#define LYRIC_COMPILER_COMPILER_SCAN_DRIVER_H

#include <lyric_assembler/object_state.h>
#include <lyric_rewriter/abstract_scan_driver.h>
#include <lyric_typing/type_system.h>

#include "abstract_compiler_context.h"

namespace lyric_compiler {

    class CompilerScanDriver : public lyric_rewriter::AbstractScanDriver {
    public:
        explicit CompilerScanDriver(lyric_assembler::ObjectState *state);
        ~CompilerScanDriver() override;

        tempo_utils::Status initialize();

        tempo_utils::Status arrange(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &children) override;

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

        lyric_typing::TypeSystem *getTypeSystem() const;

        AbstractCompilerContext *peekContext();
        tempo_utils::Status pushContext(std::unique_ptr<AbstractCompilerContext> ctx);
        tempo_utils::Status popContext();

        lyric_common::TypeDef peekResult();
        tempo_utils::Status pushResult(const lyric_common::TypeDef &result);
        tempo_utils::Status popResult();

//        tempo_utils::Status declareStatic(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
//        tempo_utils::Status pushFunction(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
//        tempo_utils::Status pushNamespace(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
//        tempo_utils::Status pushClass(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
//        tempo_utils::Status pushConcept(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
//        tempo_utils::Status pushEnum(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
//        tempo_utils::Status pushInstance(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
//        tempo_utils::Status pushStruct(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);

    private:
        lyric_assembler::ObjectState *m_state;
        lyric_assembler::NamespaceSymbol *m_root;
        lyric_assembler::CallSymbol *m_entry;
        lyric_typing::TypeSystem *m_typeSystem;
        std::vector<std::unique_ptr<AbstractCompilerContext>> m_contexts;
        std::stack<lyric_common::TypeDef> m_results;
    };
}

#endif // LYRIC_COMPILER_COMPILER_SCAN_DRIVER_H
