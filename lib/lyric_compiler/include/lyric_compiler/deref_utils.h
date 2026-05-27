#ifndef LYRIC_COMPILER_DEREF_UTILS_H
#define LYRIC_COMPILER_DEREF_UTILS_H

#include <lyric_assembler/block_handle.h>
#include <lyric_common/type_def.h>
#include <lyric_parser/archetype_node.h>

#include "compiler_scan_driver.h"

namespace lyric_compiler {

    /**
     * DerefEffect captures the context of a deref operation.
     */
    struct DerefEffect {
        lyric_common::TypeDef receiverType;                     /**< The receiver type after the deref has occurred. */
        lyric_assembler::AbstractSymbol *derefSymbol = nullptr; /**< The dereferenced symbol. */
        bool pushResult = false;                                /**< true if the deref pushed a result onto the data stack. */
        bool sideEffecting = false;                             /**< true if the deref is side-effecting. */
    };

    bool current_ref_is_this_receiver(
        const lyric_assembler::SymbolCache *symbolCache,
        const lyric_assembler::BlockHandle *bindingBlock,
        const std::vector<DerefEffect> &effects);

    bool current_ref_is_this_or_inherited_receiver(
        lyric_assembler::TypeCache *typeCache,
        const lyric_assembler::SymbolCache *symbolCache,
        const lyric_assembler::BlockHandle *bindingBlock,
        const std::vector<DerefEffect> &effects);

    tempo_utils::Status deref_literal(
        const lyric_parser::ArchetypeNode *node,
        std::vector<DerefEffect> &effects,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status deref_this(
        std::vector<DerefEffect> &effects,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status deref_name(
        const lyric_parser::ArchetypeNode *node,
        std::vector<DerefEffect> &effects,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status deref_member(
        const lyric_parser::ArchetypeNode *node,
        std::vector<DerefEffect> &effects,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status deref_global_member(
        const lyric_parser::ArchetypeNode *node,
        std::vector<DerefEffect> &effects,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);
}

#endif // LYRIC_COMPILER_DEREF_UTILS_H
