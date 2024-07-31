#ifndef LYRIC_REWRITER_LYRIC_AST_BASE_VISITOR_H
#define LYRIC_REWRITER_LYRIC_AST_BASE_VISITOR_H

#include <lyric_schema/ast_schema.h>

#include "rewrite_processor.h"

namespace lyric_rewriter {

    using ArrangeFunc = std::function<
        tempo_utils::Status(
            lyric_schema::LyricAstId,
            lyric_parser::ArchetypeNode *,
            std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &,
            void *)>;

    using EnterFunc = std::function<
        tempo_utils::Status(
            lyric_schema::LyricAstId,               // astId
            lyric_parser::ArchetypeNode *,          // node
            VisitorContext &,                       // ctx
            void *)>;                               // data

    using ExitFunc = std::function<
        tempo_utils::Status(
            lyric_schema::LyricAstId,               // astId
            lyric_parser::ArchetypeNode *,          // node
            const VisitorContext &,                 // ctx
            void *)>;                               // data

    using ReleaseFunc = std::function<void(void *)>;

    struct LyricAstOptions {
        ArrangeFunc arrange = nullptr;
        EnterFunc enter = nullptr;
        ExitFunc exit = nullptr;
        void *data = nullptr;
        ReleaseFunc release = nullptr;
    };

    class LyricAstBaseVisitor : public AbstractNodeVisitor {
    public:
        explicit LyricAstBaseVisitor(LyricAstOptions *options);
        ~LyricAstBaseVisitor() override;

    protected:
        std::shared_ptr<lyric_rewriter::AbstractNodeVisitor> makeVisitor(lyric_schema::LyricAstId astId);

        tempo_utils::Status arrangeChildren(
            lyric_schema::LyricAstId astId,
            lyric_parser::ArchetypeNode *node,
            std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &children);
        tempo_utils::Status invokeEnter(
            lyric_schema::LyricAstId astId,
            lyric_parser::ArchetypeNode *node,
            VisitorContext &ctx);
        tempo_utils::Status invokeExit(
            lyric_schema::LyricAstId astId,
            lyric_parser::ArchetypeNode *node,
            const VisitorContext &ctx);
        LyricAstOptions *getOptions() const;

    private:
        LyricAstOptions *m_options;
        std::shared_ptr<AbstractNodeVisitor> m_visitor;
    };
}

#endif // LYRIC_REWRITER_LYRIC_AST_BASE_VISITOR_H
