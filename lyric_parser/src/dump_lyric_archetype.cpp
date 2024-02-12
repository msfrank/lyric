#include <filesystem>
#include <iostream>

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_format.h>
#include <flatbuffers/idl.h>

#include <tempo_utils/file_reader.h>
#include <tempo_utils/unicode.h>
#include <tempo_utils/url.h>

#include "lyric_archetype.h"
#include "parser_types.h"

std::string
dump_lyric_archetype(const lyric_parser::LyricArchetype &archetype)
{
    const char *schemaPath;
    const char *schemaData;

    if (std::filesystem::exists(ARCHETYPE_FBS_SRC_PATH)) {
        tempo_utils::FileReader schemaReader(ARCHETYPE_FBS_SRC_PATH);
        if (!schemaReader.isValid())
            return {};
        schemaData = schemaReader.fileData();
        schemaPath = ARCHETYPE_FBS_SRC_PATH;
    }
    else if (std::filesystem::exists(ARCHETYPE_FBS_INSTALL_PATH)) {
        tempo_utils::FileReader schemaReader(ARCHETYPE_FBS_INSTALL_PATH);
        if (!schemaReader.isValid())
            return {};
        schemaData = schemaReader.fileData();
        schemaPath = ARCHETYPE_FBS_INSTALL_PATH;
    } else {
        return {};
    }

    flatbuffers::Parser parser;
    parser.Parse(schemaData, nullptr, schemaPath);
    parser.opts.strict_json = true;

    std::string jsonData;
    if (!GenerateText(parser, archetype.getBytes()->c_str(), &jsonData))
        return {};
    return jsonData;
}

void
print_lyric_archetype(const lyric_parser::LyricArchetype &archetype)
{
    std::cout << "----DUMP ARCHETYPE----" << std::endl;
    std::cout << dump_lyric_archetype(archetype) << std::endl;
    std::cout << "----END DUMP----" << std::endl;
}

//static void
//render_archetype_attr(
//    const lyi1::AttributeDescriptor *attr,
//    const absl::flat_hash_map<tempo_utils::URI,tu_uint32> &nsUrls,
//    tu_uint32 astNsOffset,
//    std::string &buf,
//    int currIndent)
//{
//    auto attrNs = attr->attr_ns();
//    auto attrType = attr->attr_type();
//
//    std::string attrName;
//    if (attrNs == astNsOffset) {
//        attrName = lyric_parser::ast_attr_type_to_name(static_cast<lyric_parser::AstAttrType>(attrType));
//    } else {
//        attrName = absl::StrCat("<ns:", attrNs, ", type:", attrType, ">");
//    }
//    buf.append(currIndent, ' ');
//    buf.append(attrName);
//    buf.append(" = ");
//    switch (attr->attr_value_type()) {
//        case lyi1::Value::TrueFalseNilValue: {
//            auto *tfn = attr->attr_value_as_TrueFalseNilValue();
//            switch (tfn->tfn()) {
//                case lyi1::TrueFalseNil::True:  buf.append("true"); break;
//                case lyi1::TrueFalseNil::False: buf.append("false"); break;
//                case lyi1::TrueFalseNil::Nil:   buf.append("nil"); break;
//                default:                        buf.append("???"); break;
//            }
//            break;
//        }
//        case lyi1::Value::Int64Value: {
//            auto *i64 = attr->attr_value_as_Int64Value();
//            buf.append(absl::StrFormat("%d", i64->i64()));
//            break;
//        }
//        case lyi1::Value::Float64Value: {
//            auto *f64 = attr->attr_value_as_Float64Value();
//            buf.append(absl::StrFormat("%f", f64->f64()));
//            break;
//        }
//        case lyi1::Value::UInt32Value: {
//            auto *u32 = attr->attr_value_as_UInt32Value();
//            buf.append(absl::StrFormat("%d", u32->u32()));
//            break;
//        }
//        case lyi1::Value::UInt16Value: {
//            auto *u16 = attr->attr_value_as_UInt16Value();
//            buf.append(absl::StrFormat("%d", u16->u16()));
//            break;
//        }
//        case lyi1::Value::UInt8Value: {
//            auto *u8 = attr->attr_value_as_UInt8Value();
//            buf.append(absl::StrFormat("%d", u8->u8()));
//            break;
//        }
//        case lyi1::Value::CharValue: {
//            auto *chr = attr->attr_value_as_CharValue();
//            buf.append(tempo_utils::convert_to_utf8(chr->chr()));
//            break;
//        }
//        case lyi1::Value::StringValue: {
//            auto *str = attr->attr_value_as_StringValue();
//            if (str->utf8() != nullptr) {
//                buf.append("\"").append(str->utf8()->str()).append("\"");
//            } else {
//                buf.append("???");
//            }
//            break;
//        }
//        case lyi1::Value::BytesValue: {
//            auto *bytes = attr->attr_value_as_BytesValue();
//            buf.append("0x");
//            for (tu_uint32 i = 0; i < bytes->bytes()->size(); i++) {
//                buf.append(absl::StrFormat("%x", bytes->bytes()->Get(i)));
//            }
//            break;
//        }
//        default:
//            buf.append("???");
//            break;
//    }
//}
//
//static void
//render_archetype_node(
//    lyric_parser::ArchetypeWalker &walker,
//    const absl::flat_hash_map<tempo_utils::URI,tu_uint32> &nsUrls,
//    tu_uint32 astNsOffset,
//    std::string &buf,
//    int currIndent,
//    int indentSize)
//{
//    auto nodeNs = walker.getNs();
//    auto nodeType = walker.getType();
//
//    std::string nodeName;
//    if (nodeNs == astNsOffset) {
//        nodeName = lyric_parser::ast_node_type_to_name(static_cast<lyric_parser::AstNodeType>(nodeType));
//    } else {
//        nodeName = absl::StrCat("<ns:", nodeNs, ", type:", nodeType, ">");
//    }
//    buf.append(currIndent, ' ');
//    buf.append(nodeName);
//    buf.append("(");
//
//    for (tu_uint32 i = 0; i < walker.numAttrs(); i++) {
//        auto *attr = walker.getAttr(i);
//        buf.append("\n");
//        render_archetype_attr(attr, nsUrls, astNsOffset, buf, currIndent + indentSize);
//    }
//    buf.append(") {");
//
//    for (tu_uint32 i = 0; i < walker.numChildren(); i++) {
//        auto child = walker.getChild(i);
//        buf.append("\n");
//        render_archetype_node(child, nsUrls, astNsOffset, buf, currIndent + indentSize, indentSize);
//    }
//    buf.append("\n");
//    buf.append(currIndent, ' ');
//    buf.append("}");
//}
//
//std::string
//render_lyric_archetype(const lyric_parser::LyricArchetype &archetype)
//{
//    absl::flat_hash_map<tempo_utils::URI,tu_uint32> nsUrls;
//    tu_uint32 astNsOffset = lyric_parser::INVALID_ADDRESS_U32;
//
//    for (tu_uint32 i = 0; i < archetype.numNamespaces(); i++) {
//        auto *ns = archetype.getNamespace(i);
//        if (ns->ns_url() == nullptr)
//            return {};
//        auto nsUrl = tempo_utils::URI::fromString(ns->ns_url()->str());
//        if (!nsUrl.isValid())
//            return {};
//        if (nsUrls.contains(nsUrl))
//            return {};
//        nsUrls[nsUrl] = i;
//        if (nsUrl.toString() == NS_AST_1_URI) {
//            astNsOffset = i;
//        }
//    }
//
//    lyric_parser::ArchetypeWalker walker(archetype);
//    if (!walker.isValid())
//        return {};
//
//    std::string buf;
//    render_archetype_node(walker, nsUrls, astNsOffset, buf, 0, 2);
//    return buf;
//}