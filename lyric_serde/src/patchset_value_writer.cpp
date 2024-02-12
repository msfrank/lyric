//
//#include <lyric_serde/patchset_namespace.h>
//#include <lyric_serde/patchset_value.h>
//#include <lyric_serde/patchset_value_writer.h>
//
//lyric_serde::PatchsetValueWriter::PatchsetValueWriter(const tempo_utils::AttrKey &key, PatchsetState *state)
//    : m_key(key),
//      m_state(state)
//{
//    TU_ASSERT (m_state != nullptr);
//}
//
//tempo_utils::AttrResult<tu_uint32>
//lyric_serde::PatchsetValueWriter::putNamespace(const tempo_utils::Url &nsUrl)
//{
//    auto putNamespaceResult = m_state->putNamespace(nsUrl);
//    if (putNamespaceResult.isStatus())
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kConversionError,"failed to create namespace");
//    auto *ns = putNamespaceResult.getResult();
//    return ns->getAddress().getAddress();
//}
//
//tempo_utils::AttrResult<tu_uint32>
//lyric_serde::PatchsetValueWriter::putNil()
//{
//    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
//    auto putNamespaceResult = putNamespace(nsUrl);
//    if (putNamespaceResult.isStatus())
//        return putNamespaceResult.getStatus();
//    auto nsAddress = putNamespaceResult.getResult();
//    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id), nullptr);
//    if (appendAttrResult.isStatus())
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kConversionError, "failed to append attr");
//    auto *attr = appendAttrResult.getResult();
//    return attr->getAddress().getAddress();
//}
//
//tempo_utils::AttrResult<tu_uint32>
//lyric_serde::PatchsetValueWriter::putBool(bool b)
//{
//    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
//    auto putNamespaceResult = putNamespace(nsUrl);
//    if (putNamespaceResult.isStatus())
//        return putNamespaceResult.getStatus();
//    auto nsAddress = putNamespaceResult.getResult();
//    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id), b);
//    if (appendAttrResult.isStatus())
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kConversionError, "failed to append attr");
//    auto *attr = appendAttrResult.getResult();
//    return attr->getAddress().getAddress();
//}
//
//tempo_utils::AttrResult<tu_uint32>
//lyric_serde::PatchsetValueWriter::putInt64(tu_int64 i64)
//{
//    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
//    auto putNamespaceResult = putNamespace(nsUrl);
//    if (putNamespaceResult.isStatus())
//        return putNamespaceResult.getStatus();
//    auto nsAddress = putNamespaceResult.getResult();
//    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id), i64);
//    if (appendAttrResult.isStatus())
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kConversionError, "failed to append attr");
//    auto *attr = appendAttrResult.getResult();
//    return attr->getAddress().getAddress();
//}
//
//tempo_utils::AttrResult<tu_uint32>
//lyric_serde::PatchsetValueWriter::putFloat64(double dbl)
//{
//    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
//    auto putNamespaceResult = putNamespace(nsUrl);
//    if (putNamespaceResult.isStatus())
//        return putNamespaceResult.getStatus();
//    auto nsAddress = putNamespaceResult.getResult();
//    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id), dbl);
//    if (appendAttrResult.isStatus())
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kConversionError, "failed to append attr");
//    auto *attr = appendAttrResult.getResult();
//    return attr->getAddress().getAddress();
//}
//
//tempo_utils::AttrResult<tu_uint32>
//lyric_serde::PatchsetValueWriter::putUInt64(tu_uint64 u64)
//{
//    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
//    auto putNamespaceResult = putNamespace(nsUrl);
//    if (putNamespaceResult.isStatus())
//        return putNamespaceResult.getStatus();
//    auto nsAddress = putNamespaceResult.getResult();
//    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id), u64);
//    if (appendAttrResult.isStatus())
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kConversionError, "failed to append attr");
//    auto *attr = appendAttrResult.getResult();
//    return attr->getAddress().getAddress();
//}
//
//tempo_utils::AttrResult<tu_uint32>
//lyric_serde::PatchsetValueWriter::putUInt32(tu_uint32 u32)
//{
//    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
//    auto putNamespaceResult = putNamespace(nsUrl);
//    if (putNamespaceResult.isStatus())
//        return putNamespaceResult.getStatus();
//    auto nsAddress = putNamespaceResult.getResult();
//    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id), u32);
//    if (appendAttrResult.isStatus())
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kConversionError, "failed to append attr");
//    auto *attr = appendAttrResult.getResult();
//    return attr->getAddress().getAddress();
//}
//
//tempo_utils::AttrResult<tu_uint32>
//lyric_serde::PatchsetValueWriter::putUInt16(tu_uint16 u16)
//{
//    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
//    auto putNamespaceResult = putNamespace(nsUrl);
//    if (putNamespaceResult.isStatus())
//        return putNamespaceResult.getStatus();
//    auto nsAddress = putNamespaceResult.getResult();
//    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id), u16);
//    if (appendAttrResult.isStatus())
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kConversionError, "failed to append attr");
//    auto *attr = appendAttrResult.getResult();
//    return attr->getAddress().getAddress();
//}
//
//tempo_utils::AttrResult<tu_uint32>
//lyric_serde::PatchsetValueWriter::putUInt8(tu_uint8 u8)
//{
//    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
//    auto putNamespaceResult = putNamespace(nsUrl);
//    if (putNamespaceResult.isStatus())
//        return putNamespaceResult.getStatus();
//    auto nsAddress = putNamespaceResult.getResult();
//    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id), u8);
//    if (appendAttrResult.isStatus())
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kConversionError, "failed to append attr");
//    auto *attr = appendAttrResult.getResult();
//    return attr->getAddress().getAddress();
//}
//
//tempo_utils::AttrResult<tu_uint32>
//lyric_serde::PatchsetValueWriter::putString(std::string_view str)
//{
//    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
//    auto putNamespaceResult = putNamespace(nsUrl);
//    if (putNamespaceResult.isStatus())
//        return putNamespaceResult.getStatus();
//    auto nsAddress = putNamespaceResult.getResult();
//    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id), std::string(str));
//    if (appendAttrResult.isStatus())
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kConversionError, "failed to append attr");
//    auto *attr = appendAttrResult.getResult();
//    return attr->getAddress().getAddress();
//}