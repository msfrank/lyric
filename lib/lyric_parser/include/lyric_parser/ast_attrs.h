#ifndef LYRIC_PARSER_AST_ATTRS_H
#define LYRIC_PARSER_AST_ATTRS_H

#include <lyric_common/assembly_location.h>
#include <lyric_common/common_serde.h>
#include <lyric_common/symbol_url.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/attr.h>

#include "archetype_node.h"
#include "archetype_state.h"
#include "parser_types.h"

namespace lyric_parser {

    class BaseTypeAttr : public tempo_utils::AttrSerde<BaseType> {

        using SerdeType = BaseType;

    public:
        explicit BaseTypeAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriter *writer,
            const BaseType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParser *parser,
            BaseType &value) const override;
    };

    class NotationTypeAttr : public tempo_utils::AttrSerde<NotationType> {

        using SerdeType = NotationType;

    public:
        explicit NotationTypeAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriter *writer,
            const NotationType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParser *parser,
            NotationType &value) const override;
    };

    class BindingTypeAttr : public tempo_utils::AttrSerde<BindingType> {

        using SerdeType = BindingType;

    public:
        explicit BindingTypeAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriter *writer,
            const BindingType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParser *parser,
            BindingType &value) const override;
    };

    class MutationTypeAttr : public tempo_utils::AttrSerde<MutationType> {

        using SerdeType = MutationType;

    public:
        explicit MutationTypeAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriter *writer,
            const MutationType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParser *parser,
            MutationType &value) const override;
    };

    class AccessTypeAttr : public tempo_utils::AttrSerde<AccessType> {

        using SerdeType = AccessType;

    public:
        explicit AccessTypeAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriter *writer,
            const AccessType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParser *parser,
            AccessType &value) const override;
    };

    class BoundTypeAttr : public tempo_utils::AttrSerde<BoundType> {

        using SerdeType = BoundType;

    public:
        explicit BoundTypeAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriter *writer,
            const BoundType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParser *parser,
            BoundType &value) const override;
    };

    class VarianceTypeAttr : public tempo_utils::AttrSerde<VarianceType> {

        using SerdeType = VarianceType;

    public:
        explicit VarianceTypeAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriter *writer,
            const VarianceType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParser *parser,
            VarianceType &value) const override;
    };

    class NodeAttr : public tempo_utils::TypedSerde<
        NodeWalker,
        std::shared_ptr<const internal::ArchetypeReader>,
        ArchetypeNode *,
        ArchetypeState> {

        using ParseType = NodeWalker;
        using WriteType = ArchetypeNode *;

    public:
        explicit NodeAttr(const tempo_utils::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_utils::AbstractAttrWriterWithState<ArchetypeState> *writer,
            ArchetypeNode * const &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_utils::AbstractAttrParserWithState<std::shared_ptr<const internal::ArchetypeReader>> *parser,
            NodeWalker &value) const override;
    };

    extern const tempo_utils::StringAttr kLyricAstLiteralValue;

    extern const BaseTypeAttr kLyricAstBaseType;
    extern const NotationTypeAttr kLyricAstNotationType;
    extern const BindingTypeAttr kLyricAstBindingType;
    extern const MutationTypeAttr kLyricAstMutationType;
    extern const AccessTypeAttr kLyricAstAccessType;
    extern const BoundTypeAttr kLyricAstBoundType;
    extern const VarianceTypeAttr kLyricAstVarianceType;

    extern const lyric_common::AssemblyLocationAttr kLyricAstAssemblyLocation;
    extern const lyric_common::SymbolPathAttr kLyricAstSymbolPath;
    extern const tempo_utils::StringAttr kLyricAstIdentifier;
    extern const tempo_utils::StringAttr kLyricAstLabel;

    extern const NodeAttr kLyricAstTypeOffset;
    extern const NodeAttr kLyricAstDefaultOffset;
    extern const NodeAttr kLyricAstFinallyOffset;
    extern const NodeAttr kLyricAstRestOffset;
    extern const NodeAttr kLyricAstGenericOffset;
    extern const NodeAttr kLyricAstImplementsOffset;
    extern const NodeAttr kLyricAstTypeArgumentsOffset;
}


#endif // LYRIC_PARSER_AST_ATTRS_H