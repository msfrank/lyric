#ifndef LYRIC_PARSER_ARCHETYPE_STATE_H
#define LYRIC_PARSER_ARCHETYPE_STATE_H

#include <filesystem>

#include <antlr4-runtime.h>

#include <tempo_tracing/scope_manager.h>
#include <tempo_tracing/span_log.h>
#include <tempo_tracing/trace_span.h>

#include <ModuleLexer.h>
#include <ModuleParser.h>
#include <ModuleParserBaseListener.h>

#include "archetype_attr_writer.h"
#include "lyric_archetype.h"
#include "parser_types.h"
#include "parse_result.h"
#include "parser_attrs.h"

namespace lyric_parser {

    // forward declarations
    class ArchetypeNamespace;
    class ArchetypeNode;
    class ArchetypeAttr;

    class ArchetypeState {

    public:
        ArchetypeState(const tempo_utils::Url &sourceUrl, tempo_tracing::ScopeManager *scopeManager);

        ArchetypeNode *makeSType(ModuleParser::SimpleTypeContext *ctx);
        ArchetypeNode *makePType(ModuleParser::ParametricTypeContext *ctx);
        ArchetypeNode *makeUType(ModuleParser::UnionTypeContext *ctx);
        ArchetypeNode *makeIType(ModuleParser::IntersectionTypeContext *ctx);
        ArchetypeNode *makeType(ModuleParser::AssignableTypeContext *ctx);
        ArchetypeNode *makeTypeArguments(ModuleParser::TypeArgumentsContext *ctx);

        ArchetypeNode *makeGeneric(
            ModuleParser::PlaceholderSpecContext *pctx,
            ModuleParser::ConstraintSpecContext *cctx);

        bool hasNamespace(const tempo_utils::Url &nsUrl) const;
        ArchetypeNamespace *getNamespace(int index) const;
        ArchetypeNamespace *getNamespace(const tempo_utils::Url &nsUrl) const;
        tempo_utils::Result<ArchetypeNamespace *> putNamespace(const tempo_utils::Url &nsUrl);
        std::vector<ArchetypeNamespace *>::const_iterator namespacesBegin() const;
        std::vector<ArchetypeNamespace *>::const_iterator namespacesEnd() const;
        int numNamespaces() const;

        ArchetypeAttr *getAttr(int index) const;
        std::vector<ArchetypeAttr *>::const_iterator attrsBegin() const;
        std::vector<ArchetypeAttr *>::const_iterator attrsEnd() const;
        int numAttrs() const;

        ArchetypeNode *getNode(int index) const;
        std::vector<ArchetypeNode *>::const_iterator nodesBegin() const;
        std::vector<ArchetypeNode *>::const_iterator nodesEnd() const;
        int numNodes() const;

        bool isEmpty();
        void pushNode(ArchetypeNode *node);
        ArchetypeNode *popNode();
        ArchetypeNode *peekNode();

        void pushSymbol(const std::string &identifier);
        std::string popSymbol();
        std::string popSymbolAndCheck(std::string_view checkIdentifier);
        std::string peekSymbol();
        std::vector<std::string> currentSymbolPath() const;
        std::string currentSymbolString() const;

        tempo_tracing::ScopeManager *scopeManager() const;

        tempo_utils::Result<LyricArchetype> toArchetype() const;

    private:
        tempo_utils::Url m_sourceUrl;
        tempo_tracing::ScopeManager *m_scopeManager;
        std::vector<ArchetypeNamespace *> m_archetypeNamespaces;
        std::vector<ArchetypeNode *> m_archetypeNodes;
        std::vector<ArchetypeAttr *> m_archetypeAttrs;
        absl::flat_hash_map<tempo_utils::Url,tu_uint32> m_namespaceIndex;
        std::stack<ArchetypeNode *> m_nodeStack;
        std::vector<std::string> m_symbolStack;

        tempo_utils::Result<tu_uint32> putNamespace(const char *nsString);
        tempo_utils::Result<ArchetypeNode *> appendNode(tu_uint32 nodeNs, tu_uint32 nodeId, antlr4::Token *token);
        tempo_utils::Result<ArchetypeAttr *> appendAttr(AttrId id, tempo_utils::AttrValue value);

        friend class ArchetypeAttrWriter;

    public:
        /**
         *
         * @tparam NsType
         * @tparam IdType
         * @param nodeClass
         * @param token
         * @return
         */
        template <class NsType, class IdType>
        tempo_utils::Result<ArchetypeNode *>
        appendNode(
            tempo_utils::SchemaClass<NsType,IdType> nodeClass,
            antlr4::Token *token)
        {
            auto putNamespaceResult = putNamespace(nodeClass.getNsUrl());
            if (putNamespaceResult.isStatus())
                return putNamespaceResult.getStatus();
            auto nsOffset = putNamespaceResult.getResult();
            return appendNode(nsOffset, nodeClass.getIdValue(), token);
        };

        /**
         *
         * @tparam NsType
         * @tparam IdType
         * @param nodeClass
         * @param token
         * @return
         */
        template <class NsType, class IdType>
        ArchetypeNode *
        appendNodeOrThrow(
            tempo_utils::SchemaClass<NsType,IdType> nodeClass,
            antlr4::Token *token)
        {
            auto appendNodeResult = appendNode(nodeClass, token);
            if (appendNodeResult.isResult())
                return appendNodeResult.getResult();
            throw tempo_utils::StatusException(appendNodeResult.getStatus());
        };

        /**
          *
          * @tparam T
          * @param serde
          * @param value
          * @return
          */
        template <typename T>
        tempo_utils::Result<ArchetypeAttr *>
        appendAttr(const tempo_utils::AttrSerde<T> &serde, const T &value)
        {
            ArchetypeAttrWriter writer(serde.getKey(), this);
            auto result = serde.writeAttr(&writer, value);
            if (result.isStatus())
                return result.getStatus();
            auto *attr = getAttr(result.getResult());
            if (attr == nullptr)
                return ParseStatus::forCondition(ParseCondition::kParseInvariant, "missing serialized attr");
            return attr;
        };

        template <typename T>
        ArchetypeAttr *
        appendAttrOrThrow(const tempo_utils::AttrSerde<T> &serde, const T &value)
        {
            auto appendAttrResult = appendAttr(serde, value);
            if (appendAttrResult.isResult())
                return appendAttrResult.getResult();
            throw tempo_utils::StatusException(appendAttrResult.getStatus());
        };

        /**
         *
         * @tparam Args
         * @param walker
         * @param condition
         * @param severity
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename... Args>
        ParseStatus logAndContinue(
            antlr4::Token *token,
            ParseCondition condition,
            tempo_tracing::LogSeverity severity,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = ParseStatus::forCondition(condition, span->traceId(), span->spanId(),
                messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), severity);
            log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
            log->putField(kLyricParserLineNumber, static_cast<tu_int64>(token->getLine()));
            log->putField(kLyricParserColumnNumber, static_cast<tu_int64>(token->getCharPositionInLine()));
            log->putField(kLyricParserFileOffset, static_cast<tu_int64>(token->getStartIndex()));
            log->putField(kLyricParserTextSpan, static_cast<tu_int64>(token->getStopIndex() - token->getStartIndex()));
            return status;
        }

        /**
         *
         * @tparam Args
         * @param condition
         * @param severity
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename... Args>
        ParseStatus logAndContinue(
            ParseCondition condition,
            tempo_tracing::LogSeverity severity,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = ParseStatus::forCondition(condition, span->traceId(), span->spanId(),
                messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), severity);
            log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
            return status;
        }
        /**
         *
         * @tparam Args
         * @param token
         * @param messageFmt
         * @param args
         */
        template <typename... Args>
        void throwSyntaxError(antlr4::Token *token, fmt::string_view messageFmt = {}, Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = ParseStatus::forCondition(ParseCondition::kSyntaxError,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
            throw tempo_utils::StatusException(status);
        }

        /**
          *
          * @tparam Args
          * @param token
          * @param messageFmt
          * @param args
          */
        template <typename... Args>
        void throwParseInvariant(
            NodeAddress address,
            antlr4::Token *token,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = ParseStatus::forCondition(ParseCondition::kParseInvariant,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
            log->putField(kLyricParserLineNumber, static_cast<tu_int64>(token->getLine()));
            log->putField(kLyricParserColumnNumber, static_cast<tu_int64>(token->getCharPositionInLine()));
            log->putField(kLyricParserFileOffset, static_cast<tu_int64>(token->getStartIndex()));
            log->putField(kLyricParserTextSpan, static_cast<tu_int64>(token->getStopIndex() - token->getStartIndex()));
            log->putField(kLyricParserNodeOffset, static_cast<tu_int64>(address.getAddress()));
            throw tempo_utils::StatusException(status);
        }

        /**
          *
          * @tparam Args
          * @param token
          * @param messageFmt
          * @param args
          */
        template <typename... Args>
        void throwParseInvariant(NodeAddress address, fmt::string_view messageFmt = {}, Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = ParseStatus::forCondition(ParseCondition::kParseInvariant,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(kLyricParserIdentifier, currentSymbolString());
            log->putField(kLyricParserNodeOffset, static_cast<tu_int64>(address.getAddress()));
            throw tempo_utils::StatusException(status);
        }

        /**
         *
         * @tparam Args
         * @param token
         * @param messageFmt
         * @param args
         */
        template <typename... Args>
        void throwIncompleteModule(antlr4::Token *token, fmt::string_view messageFmt = {}, Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = ParseStatus::forCondition(ParseCondition::kIncompleteModule,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(kLyricParserIdentifier, currentSymbolString());
            log->putField(kLyricParserLineNumber, static_cast<tu_int64>(token->getLine()));
            log->putField(kLyricParserColumnNumber, static_cast<tu_int64>(token->getCharPositionInLine()));
            log->putField(kLyricParserFileOffset, static_cast<tu_int64>(token->getStartIndex()));
            log->putField(kLyricParserTextSpan, static_cast<tu_int64>(token->getStopIndex() - token->getStartIndex()));
            throw tempo_utils::StatusException(status);
        }
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_STATE_H
