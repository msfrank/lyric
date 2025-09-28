#ifndef LYRIC_PARSER_AST_ATTRS_H
#define LYRIC_PARSER_AST_ATTRS_H

#include <lyric_common/module_location.h>
#include <lyric_common/common_serde.h>
#include <lyric_common/symbol_url.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_schema/attr_serde.h>
#include <tempo_schema/url_serde.h>

#include "archetype_node.h"
#include "archetype_state.h"
#include "parser_types.h"
#include "stateful_attr.h"

namespace lyric_parser {

    class BaseTypeAttr : public tempo_schema::AttrSerde<BaseType> {

        using SerdeType = BaseType;

    public:
        explicit BaseTypeAttr(const tempo_schema::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_schema::AbstractAttrWriter *writer,
            const BaseType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParser *parser,
            BaseType &value) const override;
    };

    class NotationTypeAttr : public tempo_schema::AttrSerde<NotationType> {

        using SerdeType = NotationType;

    public:
        explicit NotationTypeAttr(const tempo_schema::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_schema::AbstractAttrWriter *writer,
            const NotationType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParser *parser,
            NotationType &value) const override;
    };

    class BoundTypeAttr : public tempo_schema::AttrSerde<BoundType> {

        using SerdeType = BoundType;

    public:
        explicit BoundTypeAttr(const tempo_schema::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_schema::AbstractAttrWriter *writer,
            const BoundType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParser *parser,
            BoundType &value) const override;
    };

    class VarianceTypeAttr : public tempo_schema::AttrSerde<VarianceType> {

        using SerdeType = VarianceType;

    public:
        explicit VarianceTypeAttr(const tempo_schema::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_schema::AbstractAttrWriter *writer,
            const VarianceType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParser *parser,
            VarianceType &value) const override;
    };

    class DeriveTypeAttr : public tempo_schema::AttrSerde<DeriveType> {

        using SerdeType = DeriveType;

    public:
        explicit DeriveTypeAttr(const tempo_schema::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_schema::AbstractAttrWriter *writer,
            const DeriveType &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParser *parser,
            DeriveType &value) const override;
    };

    class NodeAttr : public StatefulAttr {
    public:
        explicit NodeAttr(const tempo_schema::ComparableResource *resource);
        tempo_utils::Result<tu_uint32> writeAttr(
            tempo_schema::AbstractAttrWriterWithState<ArchetypeState> *writer,
            ArchetypeNode * const &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParserWithState<ArchetypeState> *parser,
            ArchetypeNode * &value) const override;
        tempo_utils::Status parseAttr(
            tu_uint32 index,
            tempo_schema::AbstractAttrParserWithState<std::shared_ptr<const internal::ArchetypeReader>> *parser,
            NodeWalker &value) const override;
    };

    extern const tempo_schema::StringAttr kLyricAstLiteralValue;

    extern const BaseTypeAttr kLyricAstBaseType;
    extern const NotationTypeAttr kLyricAstNotationType;
    extern const BoundTypeAttr kLyricAstBoundType;
    extern const VarianceTypeAttr kLyricAstVarianceType;
    extern const DeriveTypeAttr kLyricAstDeriveType;

    extern const tempo_schema::UrlAttr kLyricAstImportLocation;
    extern const lyric_common::ModuleLocationAttr kLyricAstModuleLocation;
    extern const lyric_common::SymbolPathAttr kLyricAstSymbolPath;
    extern const tempo_schema::StringAttr kLyricAstIdentifier;
    extern const tempo_schema::StringAttr kLyricAstLabel;
    extern const tempo_schema::BoolAttr kLyricAstIsHidden;
    extern const tempo_schema::BoolAttr kLyricAstIsVariable;
    extern const tempo_schema::BoolAttr kLyricAstNoOverride;
    extern const tempo_schema::BoolAttr kLyricAstThisBase;

    extern const NodeAttr kLyricAstTypeOffset;
    extern const NodeAttr kLyricAstDefaultOffset;
    extern const NodeAttr kLyricAstFinallyOffset;
    extern const NodeAttr kLyricAstRestOffset;
    extern const NodeAttr kLyricAstGenericOffset;
    extern const NodeAttr kLyricAstTypeArgumentsOffset;
    extern const NodeAttr kLyricAstMacroListOffset;
}


#endif // LYRIC_PARSER_AST_ATTRS_H