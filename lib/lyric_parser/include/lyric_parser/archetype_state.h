#ifndef LYRIC_PARSER_ARCHETYPE_STATE_H
#define LYRIC_PARSER_ARCHETYPE_STATE_H

#include <filesystem>

#include <tempo_tracing/scope_manager.h>
#include <tempo_tracing/span_log.h>
#include <tempo_tracing/trace_span.h>

#include "archetype_attr_writer.h"
#include "archetype_attr.h"
#include "archetype_namespace.h"
#include "archetype_node.h"
#include "lyric_archetype.h"
#include "parser_types.h"
#include "parser_attrs.h"
#include "parse_result.h"

namespace lyric_parser {

    // forward declarations
    class ArchetypeNamespace;
    class ArchetypeNode;
    class ArchetypeAttr;

    class ArchetypeState {

    public:
        ArchetypeState(const tempo_utils::Url &sourceUrl, tempo_tracing::ScopeManager *scopeManager);

        ArchetypeId *getId(int index) const;
        int numIds() const;

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
        std::vector<ArchetypeId *> m_archetypeIds;
        std::vector<ArchetypeNamespace *> m_archetypeNamespaces;
        std::vector<ArchetypeNode *> m_archetypeNodes;
        std::vector<ArchetypeAttr *> m_archetypeAttrs;
        absl::flat_hash_map<tempo_utils::Url,tu_uint32> m_namespaceIndex;
        std::stack<ArchetypeNode *> m_nodeStack;
        std::vector<std::string> m_symbolStack;

        ArchetypeId *makeId(ArchetypeDescriptorType type, tu_uint32 offset);
        tempo_utils::Result<ArchetypeNamespace *> putNamespace(const char *nsString);
        tempo_utils::Result<ArchetypeNode *> appendNode(
            ArchetypeNamespace *nodeNamespace,
            tu_uint32 nodeId,
            const ParseLocation &location);
        tempo_utils::Result<ArchetypeAttr *> appendAttr(AttrId id, AttrValue value);

        friend class ArchetypeAttrWriter;
        friend class NodeAttr;

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
            const ParseLocation &location)
        {
            auto putNamespaceResult = putNamespace(nodeClass.getNsUrl());
            if (putNamespaceResult.isStatus())
                return putNamespaceResult.getStatus();
            auto *nodeNamespace = putNamespaceResult.getResult();
            return appendNode(nodeNamespace, nodeClass.getIdValue(), location);
        };

        template<class NsType, class IdType>
        void checkNodeOrThrow(
            const ArchetypeNode *node,
            const tempo_utils::SchemaClass<NsType,IdType> &schemaClass)
        {
            if (!node->isClass(schemaClass)) {
                auto status = logAndContinue(ParseCondition::kParseInvariant,
                    tempo_tracing::LogSeverity::kError,
                    "expected {} node", schemaClass.getName());
                throw tempo_utils::StatusException(status);
            }
        }

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
            const ParseLocation &location)
        {
            auto appendNodeResult = appendNode(nodeClass, location);
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
            return ArchetypeAttrWriter::createAttr(this, serde, value);
        };

        template <class P, class PS, class W, class WS>
        tempo_utils::Result<ArchetypeAttr *>
        appendAttr(const tempo_utils::TypedSerde<P,PS,W,WS> &serde, const W &value)
        {
            return ArchetypeAttrWriter::createAttr(this, serde, value);
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

        template <class P, class PS, class W, class WS>
        ArchetypeAttr *
        appendAttrOrThrow(const tempo_utils::TypedSerde<P,PS,W,WS> &serde, const W &value)
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
        template <typename ConditionType, typename... Args,
            typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        StatusType logAndContinue(
            const ParseLocation &location,
            ConditionType condition,
            tempo_tracing::LogSeverity severity,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = StatusType::forCondition(condition, span->traceId(), span->spanId(),
                messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), severity);
            log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
            log->putField(kLyricParserLineNumber, location.lineNumber);
            log->putField(kLyricParserColumnNumber, location.columnNumber);
            log->putField(kLyricParserFileOffset, location.fileOffset);
            log->putField(kLyricParserTextSpan, location.textSpan);
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
        template <typename ConditionType, typename... Args,
            typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        StatusType logAndContinue(
            ConditionType condition,
            tempo_tracing::LogSeverity severity,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = StatusType::forCondition(condition, span->traceId(), span->spanId(),
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
        void throwSyntaxError(const ParseLocation &location, fmt::string_view messageFmt = {}, Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = ParseStatus::forCondition(ParseCondition::kSyntaxError,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
            log->putField(kLyricParserLineNumber, location.lineNumber);
            log->putField(kLyricParserColumnNumber, location.columnNumber);
            log->putField(kLyricParserFileOffset, location.fileOffset);
            log->putField(kLyricParserTextSpan, location.textSpan);
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
            const ParseLocation &location,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = ParseStatus::forCondition(ParseCondition::kParseInvariant,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
            log->putField(kLyricParserLineNumber, location.lineNumber);
            log->putField(kLyricParserColumnNumber, location.columnNumber);
            log->putField(kLyricParserFileOffset, location.fileOffset);
            log->putField(kLyricParserTextSpan, location.textSpan);
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
        void throwParseInvariant(fmt::string_view messageFmt = {}, Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = ParseStatus::forCondition(ParseCondition::kParseInvariant,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(kLyricParserIdentifier, currentSymbolString());
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
        void throwIncompleteModule(const ParseLocation &location, fmt::string_view messageFmt = {}, Args... messageArgs)
        {
            auto span = m_scopeManager->peekSpan();
            auto status = ParseStatus::forCondition(ParseCondition::kIncompleteModule,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(kLyricParserIdentifier, currentSymbolString());
            log->putField(kLyricParserLineNumber, location.lineNumber);
            log->putField(kLyricParserColumnNumber, location.columnNumber);
            log->putField(kLyricParserFileOffset, location.fileOffset);
            log->putField(kLyricParserTextSpan, location.textSpan);
            throw tempo_utils::StatusException(status);
        }
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_STATE_H
