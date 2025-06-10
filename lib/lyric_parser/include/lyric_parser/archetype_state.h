#ifndef LYRIC_PARSER_ARCHETYPE_STATE_H
#define LYRIC_PARSER_ARCHETYPE_STATE_H

#include <filesystem>

#include <tempo_tracing/current_scope.h>
#include <tempo_tracing/span_log.h>
#include <tempo_tracing/trace_context.h>
#include <tempo_tracing/trace_span.h>

#include "archetype_state_attr_writer.h"
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
    class StatefulAttr;

    /**
     * Resource limits for ArchetypeState. If any ArchetypeState operations exceed one of the
     * specified limits then the operation will return ResourceExhausted status.
     */
    struct ArchetypeStateLimits {
        tu_uint32 maxNamespaces = 64;
        tu_uint32 maxAttrs = 65536;
        tu_uint32 maxNodes = 65536;
        tu_uint32 maxNodeChildren = 65536;
        tu_uint32 maxNodeAttrs = 65536;
        tu_uint32 maxPragmas = 64;
        tu_uint32 maxStackDepth = 512;
    };

    /**
     * Contains the mutable state used to build an archetype.
     */
    class ArchetypeState {

    public:
        explicit ArchetypeState(const tempo_utils::Url &sourceUrl, const ArchetypeStateLimits &limits = {});

        tempo_utils::Url getSourceUrl() const;
        ArchetypeStateLimits getLimits() const;

        tempo_utils::Result<ArchetypeNode *> load(const LyricArchetype &archetype);

        ArchetypeId *getId(int index) const;
        int numIds() const;

        tempo_utils::Result<ArchetypeNamespace *> putNamespace(const tempo_utils::Url &nsUrl);
        tempo_utils::Result<ArchetypeNamespace *> putNamespace(std::string_view nsView);

        bool hasNamespace(const tempo_utils::Url &nsUrl) const;
        ArchetypeNamespace *getNamespace(int index) const;
        ArchetypeNamespace *getNamespace(const tempo_utils::Url &nsUrl) const;
        std::vector<ArchetypeNamespace *>::const_iterator namespacesBegin() const;
        std::vector<ArchetypeNamespace *>::const_iterator namespacesEnd() const;
        int numNamespaces() const;

        tempo_utils::Result<ArchetypeAttr *> appendAttr(AttrId id, AttrValue value);

        ArchetypeAttr *getAttr(int index) const;
        std::vector<ArchetypeAttr *>::const_iterator attrsBegin() const;
        std::vector<ArchetypeAttr *>::const_iterator attrsEnd() const;
        int numAttrs() const;

        tempo_utils::Result<ArchetypeNode *> appendNode(
            ArchetypeNamespace *nodeNamespace,
            tu_uint32 nodeId,
            const ParseLocation &location);

        ArchetypeNode *getNode(int index) const;
        std::vector<ArchetypeNode *>::const_iterator nodesBegin() const;
        std::vector<ArchetypeNode *>::const_iterator nodesEnd() const;
        int numNodes() const;

        tempo_utils::Status addPragma(ArchetypeNode *pragmaNode);
        tempo_utils::Result<ArchetypeNode *> replacePragma(int index, ArchetypeNode *pragma);
        tempo_utils::Result<ArchetypeNode *> removePragma(int index);
        void clearPragmas();

        ArchetypeNode *getPragma(int index) const;
        std::vector<ArchetypeNode *>::const_iterator pragmasBegin() const;
        std::vector<ArchetypeNode *>::const_iterator pragmasEnd() const;
        int numPragmas() const;

        void setRoot(ArchetypeNode *node);
        void clearRoot();

        bool hasRoot() const;
        ArchetypeNode *getRoot() const;

        bool isEmpty();
        tempo_utils::Status pushNode(ArchetypeNode *node);
        tempo_utils::Result<ArchetypeNode *> popNode();
        tempo_utils::Result<ArchetypeNode *> peekNode();

        void pushSymbol(const std::string &identifier);
        std::string popSymbol();
        std::string popSymbolAndCheck(std::string_view checkIdentifier);
        std::string peekSymbol();

        std::vector<std::string> currentSymbolPath() const;
        std::string currentSymbolString() const;

        tempo_utils::Result<LyricArchetype> toArchetype() const;

    private:
        tempo_utils::Url m_sourceUrl;
        ArchetypeStateLimits m_limits;

        std::vector<ArchetypeId *> m_archetypeIds;
        std::vector<ArchetypeNamespace *> m_archetypeNamespaces;
        std::vector<ArchetypeNode *> m_archetypeNodes;
        std::vector<ArchetypeAttr *> m_archetypeAttrs;
        absl::flat_hash_map<tempo_utils::Url,tu_uint32> m_namespaceIndex;
        std::stack<ArchetypeNode *> m_nodeStack;
        std::vector<std::string> m_symbolStack;
        std::vector<ArchetypeNode *> m_pragmaNodes;
        ArchetypeNode *m_rootNode;

        ArchetypeId *makeId(ArchetypeDescriptorType type, tu_uint32 offset);

        friend class ArchetypeStateAttrWriter;
        friend class NodeAttr;

        tempo_utils::Result<ArchetypeAttr *>
        loadAttr(
            const LyricArchetype &archetype,
            const tempo_schema::AttrKey &key,
            const tempo_schema::AttrValue &value,
            ArchetypeState *state,
            std::vector<ArchetypeNode *> &nodeTable);

        tempo_utils::Result<ArchetypeNode *>
        loadNode(
            const LyricArchetype &archetype,
            const NodeWalker &node,
            ArchetypeState *state,
            std::vector<ArchetypeNode *> &nodeTable);

    public:
        /**
         * Append a node with the specified `nodeClass`, and return the node. If the node or associated
         * node namespace cannot be allocated then ResourceExhausted status is returned.
         *
         * @tparam NsType
         * @tparam IdType
         * @param nodeClass
         * @param location
         * @return
         */
        template <class NsType, class IdType>
        tempo_utils::Result<ArchetypeNode *>
        appendNode(
            tempo_schema::SchemaClass<NsType,IdType> nodeClass,
            const ParseLocation &location)
        {
            ArchetypeNamespace *nodeNamespace;
            TU_ASSIGN_OR_RETURN (nodeNamespace, putNamespace(nodeClass.getNsUrl()));
            return appendNode(nodeNamespace, nodeClass.getIdValue(), location);
        };

        /**
         * Peek the current node on the node stack.  If the node stack is not empty and the top node
         * matches the specified `schemaClass` then the node popped off the stack and returned. If the
         * top node exists and * does not match the specified schema class, or if the node stack is
         * empty, then ParseInvariant * status is returned without mutating the node stack.
         *
         * @tparam NsType
         * @tparam IdType
         * @param schemaClass
         * @return
         */
        template<class NsType, class IdType>
        tempo_utils::Result<ArchetypeNode *>
        popNode(const tempo_schema::SchemaClass<NsType,IdType> &schemaClass)
        {
            ArchetypeNode *node;
            TU_ASSIGN_OR_RETURN (node, peekNode());
            if (node->isClass(schemaClass))
                return popNode();
            return ParseStatus::forCondition(ParseCondition::kParseInvariant,
                "expected {} node", schemaClass.getName());
        }

        /**
         * Peek the current node on the node stack.  If the node stack is not empty and the top node
         * matches the specified `schemaClass` then the node is returned. If the top node exists and
         * does not match the specified schema class, or if the node stack is empty, then ParseInvariant
         * status is returned.
         *
         * @tparam NsType
         * @tparam IdType
         * @param schemaClass
         * @return
         */
        template<class NsType, class IdType>
        tempo_utils::Result<ArchetypeNode *>
        peekNode(const tempo_schema::SchemaClass<NsType,IdType> &schemaClass)
        {
            ArchetypeNode *node;
            TU_ASSIGN_OR_RETURN (node, peekNode());
            if (node->isClass(schemaClass))
                return node;
            return ParseStatus::forCondition(ParseCondition::kParseInvariant,
                "expected {} node", schemaClass.getName());
        }

        // /**
        //  *
        //  */
        // template<class NsType, class IdType>
        // void checkNodeOrThrow(
        //     const ArchetypeNode *node,
        //     const tempo_schema::SchemaClass<NsType,IdType> &schemaClass)
        // {
        //     if (!node->isClass(schemaClass)) {
        //         auto status = logAndContinue(ParseCondition::kParseInvariant,
        //             tempo_tracing::LogSeverity::kError,
        //             "expected {} node", schemaClass.getName());
        //         throw tempo_utils::StatusException(status);
        //     }
        // }
        //
        // /**
        //  *
        //  * @tparam NsType
        //  * @tparam IdType
        //  * @param nodeClass
        //  * @param token
        //  * @return
        //  */
        // template <class NsType, class IdType>
        // ArchetypeNode *
        // appendNodeOrThrow(
        //     tempo_schema::SchemaClass<NsType,IdType> nodeClass,
        //     const ParseLocation &location)
        // {
        //     auto appendNodeResult = appendNode(nodeClass, location);
        //     if (appendNodeResult.isResult())
        //         return appendNodeResult.getResult();
        //     throw tempo_utils::StatusException(appendNodeResult.getStatus());
        // };
        //
        // /**
        //  *
        //  * @tparam Args
        //  * @param walker
        //  * @param condition
        //  * @param severity
        //  * @param messageFmt
        //  * @param messageArgs
        //  * @return
        //  */
        // template <typename ConditionType, typename... Args,
        //     typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        // StatusType logAndContinue(
        //     const ParseLocation &location,
        //     ConditionType condition,
        //     tempo_tracing::LogSeverity severity,
        //     fmt::string_view messageFmt = {},
        //     Args... messageArgs)
        // {
        //     tempo_tracing::CurrentScope scope;
        //     auto span = scope.getSpan();
        //     auto status = StatusType::forCondition(condition, span->traceId(), span->spanId(),
        //         messageFmt, messageArgs...);
        //     auto log = span->logStatus(status, absl::Now(), severity);
        //     log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
        //     log->putField(kLyricParserLineNumber, location.lineNumber);
        //     log->putField(kLyricParserColumnNumber, location.columnNumber);
        //     log->putField(kLyricParserFileOffset, location.fileOffset);
        //     log->putField(kLyricParserTextSpan, location.textSpan);
        //     return status;
        // }
        //
        // /**
        //  *
        //  * @tparam Args
        //  * @param condition
        //  * @param severity
        //  * @param messageFmt
        //  * @param messageArgs
        //  * @return
        //  */
        // template <typename ConditionType, typename... Args,
        //     typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        // StatusType logAndContinue(
        //     ConditionType condition,
        //     tempo_tracing::LogSeverity severity,
        //     fmt::string_view messageFmt = {},
        //     Args... messageArgs)
        // {
        //     tempo_tracing::CurrentScope scope;
        //     auto span = scope.getSpan();
        //     auto status = StatusType::forCondition(condition, span->traceId(), span->spanId(),
        //         messageFmt, messageArgs...);
        //     auto log = span->logStatus(status, absl::Now(), severity);
        //     log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
        //     return status;
        // }
        //
        // /**
        //  *
        //  * @tparam Args
        //  * @param token
        //  * @param messageFmt
        //  * @param args
        //  */
        // template <typename... Args>
        // void throwSyntaxError(const ParseLocation &location, fmt::string_view messageFmt = {}, Args... messageArgs)
        // {
        //     tempo_tracing::CurrentScope scope;
        //     auto span = scope.getSpan();
        //     auto status = ParseStatus::forCondition(ParseCondition::kSyntaxError,
        //         span->traceId(), span->spanId(), messageFmt, messageArgs...);
        //     auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
        //     log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
        //     log->putField(kLyricParserLineNumber, location.lineNumber);
        //     log->putField(kLyricParserColumnNumber, location.columnNumber);
        //     log->putField(kLyricParserFileOffset, location.fileOffset);
        //     log->putField(kLyricParserTextSpan, location.textSpan);
        //     throw tempo_utils::StatusException(status);
        // }
        //
        // /**
        //   *
        //   * @tparam Args
        //   * @param token
        //   * @param messageFmt
        //   * @param args
        //   */
        // template <typename... Args>
        // void throwParseInvariant(
        //     const ParseLocation &location,
        //     fmt::string_view messageFmt = {},
        //     Args... messageArgs)
        // {
        //     tempo_tracing::CurrentScope scope;
        //     auto span = scope.getSpan();
        //     auto status = ParseStatus::forCondition(ParseCondition::kParseInvariant,
        //         span->traceId(), span->spanId(), messageFmt, messageArgs...);
        //     auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
        //     log->putField(lyric_parser::kLyricParserIdentifier, currentSymbolString());
        //     log->putField(kLyricParserLineNumber, location.lineNumber);
        //     log->putField(kLyricParserColumnNumber, location.columnNumber);
        //     log->putField(kLyricParserFileOffset, location.fileOffset);
        //     log->putField(kLyricParserTextSpan, location.textSpan);
        //     throw tempo_utils::StatusException(status);
        // }
        //
        // /**
        //   *
        //   * @tparam Args
        //   * @param token
        //   * @param messageFmt
        //   * @param args
        //   */
        // template <typename... Args>
        // void throwParseInvariant(fmt::string_view messageFmt = {}, Args... messageArgs)
        // {
        //     tempo_tracing::CurrentScope scope;
        //     auto span = scope.getSpan();
        //     auto status = ParseStatus::forCondition(ParseCondition::kParseInvariant,
        //         span->traceId(), span->spanId(), messageFmt, messageArgs...);
        //     auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
        //     log->putField(kLyricParserIdentifier, currentSymbolString());
        //     throw tempo_utils::StatusException(status);
        // }
        //
        // /**
        //  *
        //  * @tparam Args
        //  * @param token
        //  * @param messageFmt
        //  * @param args
        //  */
        // template <typename... Args>
        // void throwIncompleteModule(const ParseLocation &location, fmt::string_view messageFmt = {}, Args... messageArgs)
        // {
        //     tempo_tracing::CurrentScope scope;
        //     auto span = scope.getSpan();
        //     auto status = ParseStatus::forCondition(ParseCondition::kIncompleteModule,
        //         span->traceId(), span->spanId(), messageFmt, messageArgs...);
        //     auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
        //     log->putField(kLyricParserIdentifier, currentSymbolString());
        //     log->putField(kLyricParserLineNumber, location.lineNumber);
        //     log->putField(kLyricParserColumnNumber, location.columnNumber);
        //     log->putField(kLyricParserFileOffset, location.fileOffset);
        //     log->putField(kLyricParserTextSpan, location.textSpan);
        //     throw tempo_utils::StatusException(status);
        // }
    };
}

#endif // LYRIC_PARSER_ARCHETYPE_STATE_H
