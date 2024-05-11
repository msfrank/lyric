
#include <absl/container/flat_hash_map.h>
#include <unicode/umachine.h>
#include <unicode/ustring.h>

#include <lyric_packaging/generated/manifest.h>
#include <lyric_packaging/manifest_attr.h>
#include <lyric_packaging/manifest_entry.h>
#include <lyric_packaging/manifest_namespace.h>
#include <lyric_packaging/manifest_state.h>
#include <lyric_packaging/package_types.h>
#include <lyric_schema/packaging_schema.h>
#include <tempo_utils/memory_bytes.h>

lyric_packaging::ManifestState::ManifestState()
{
}

bool
lyric_packaging::ManifestState::hasNamespace(const tempo_utils::Url &nsUrl) const
{
    return getNamespace(nsUrl) != nullptr;
}

lyric_packaging::ManifestNamespace *
lyric_packaging::ManifestState::getNamespace(int index) const
{
    if (0 <= index && std::cmp_less(index, m_manifestNamespaces.size()))
        return m_manifestNamespaces.at(index);
    return nullptr;
}

lyric_packaging::ManifestNamespace *
lyric_packaging::ManifestState::getNamespace(const tempo_utils::Url &nsUrl) const
{
    if (!m_namespaceIndex.contains(nsUrl))
        return nullptr;
    return getNamespace(m_namespaceIndex.at(nsUrl));
}

tempo_utils::Result<lyric_packaging::ManifestNamespace *>
lyric_packaging::ManifestState::putNamespace(const tempo_utils::Url &nsUrl)
{
    if (m_namespaceIndex.contains(nsUrl)) {
        auto index = m_namespaceIndex.at(nsUrl);
        TU_ASSERT (0 <= index && index < m_manifestNamespaces.size());
        return m_manifestNamespaces.at(index);
    }
    NamespaceAddress address(m_manifestNamespaces.size());
    auto *ns = new ManifestNamespace(nsUrl, address, this);
    m_manifestNamespaces.push_back(ns);
    m_namespaceIndex[nsUrl] = address.getAddress();
    return ns;
}

std::vector<lyric_packaging::ManifestNamespace *>::const_iterator
lyric_packaging::ManifestState::namespacesBegin() const
{
    return m_manifestNamespaces.cbegin();
}

std::vector<lyric_packaging::ManifestNamespace *>::const_iterator
lyric_packaging::ManifestState::namespacesEnd() const
{
    return m_manifestNamespaces.cend();
}

int
lyric_packaging::ManifestState::numNamespaces() const
{
    return m_manifestNamespaces.size();
}

tempo_utils::Result<lyric_packaging::ManifestAttr *>
lyric_packaging::ManifestState::appendAttr(AttrId id, const tempo_utils::AttrValue &value)
{
    AttrAddress address(m_manifestAttrs.size());
    auto *attr = new ManifestAttr(id, value, address, this);
    m_manifestAttrs.push_back(attr);
    return attr;
}

lyric_packaging::ManifestAttr *
lyric_packaging::ManifestState::getAttr(int index) const
{
    if (0 <= index && std::cmp_less(index, m_manifestAttrs.size()))
        return m_manifestAttrs.at(index);
    return nullptr;
}

std::vector<lyric_packaging::ManifestAttr *>::const_iterator
lyric_packaging::ManifestState::attrsBegin() const
{
    return m_manifestAttrs.cbegin();
}

std::vector<lyric_packaging::ManifestAttr *>::const_iterator
lyric_packaging::ManifestState::attrsEnd() const
{
    return m_manifestAttrs.cend();
}

int
lyric_packaging::ManifestState::numAttrs() const
{
    return m_manifestAttrs.size();
}

bool
lyric_packaging::ManifestState::hasEntry(const EntryPath &path) const
{
    return getEntry(path) != nullptr;
}

lyric_packaging::ManifestEntry *
lyric_packaging::ManifestState::getEntry(int index) const
{
    if (0 <= index && std::cmp_less(index, m_manifestEntries.size()))
        return m_manifestEntries.at(index);
    return nullptr;
}

lyric_packaging::ManifestEntry *
lyric_packaging::ManifestState::getEntry(const EntryPath &path) const
{
    if (!m_pathIndex.contains(path))
        return nullptr;
    return getEntry(m_pathIndex.at(path));
}

tempo_utils::Result<lyric_packaging::ManifestEntry *>
lyric_packaging::ManifestState::appendEntry(EntryType type, const EntryPath &path)
{
    if (m_pathIndex.contains(path))
        return PackageStatus::forCondition(
            PackageCondition::kDuplicateEntry, "entry already exists at path");

    if (path.isEmpty()) {
        // an empty path indicates the package (i.e. root) entry
        EntryAddress address(m_manifestEntries.size());
        auto *entry = new ManifestEntry(type, path, address, this);
        m_manifestEntries.push_back(entry);
        m_pathIndex[path] = address.getAddress();
        return entry;
    } else {
        // a non-empty path must have a parent
        auto parentPath = EntryPath(path.getInit());
        if (!m_pathIndex.contains(parentPath))
            return PackageStatus::forCondition(
                PackageCondition::kPackageInvariant, "entry is missing parent");
        auto *parent = getEntry(m_pathIndex.at(parentPath));

        EntryAddress address(m_manifestEntries.size());
        auto *entry = new ManifestEntry(type, path, address, this);
        m_manifestEntries.push_back(entry);
        m_pathIndex[path] = address.getAddress();
        parent->putChild(entry);

        return entry;
    }
}

std::vector<lyric_packaging::ManifestEntry *>::const_iterator
lyric_packaging::ManifestState::entriesBegin() const
{
    return m_manifestEntries.cbegin();
}

std::vector<lyric_packaging::ManifestEntry *>::const_iterator
lyric_packaging::ManifestState::entriesEnd() const
{
    return m_manifestEntries.cend();
}

int
lyric_packaging::ManifestState::numEntries() const
{
    return m_manifestEntries.size();
}

static std::pair<lpm1::Value,flatbuffers::Offset<void>>
serialize_value(flatbuffers::FlatBufferBuilder &buffer, const tempo_utils::AttrValue &value)
{
    switch (value.getType()) {
        case tempo_utils::ValueType::Nil: {
            auto type = lpm1::Value::TrueFalseNilValue;
            auto offset = lpm1::CreateTrueFalseNilValue(buffer, lpm1::TrueFalseNil::Nil).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::Bool: {
            auto type = lpm1::Value::TrueFalseNilValue;
            auto tfn = value.getBool()? lpm1::TrueFalseNil::True : lpm1::TrueFalseNil::False;
            auto offset = lpm1::CreateTrueFalseNilValue(buffer, tfn).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::Int64: {
            auto type = lpm1::Value::Int64Value;
            auto offset = lpm1::CreateInt64Value(buffer, value.getInt64()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::Float64: {
            auto type = lpm1::Value::Float64Value;
            auto offset = lpm1::CreateFloat64Value(buffer, value.getFloat64()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt64: {
            auto type = lpm1::Value::UInt64Value;
            auto offset = lpm1::CreateUInt64Value(buffer, value.getUInt64()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt32: {
            auto type = lpm1::Value::UInt32Value;
            auto offset = lpm1::CreateUInt32Value(buffer, value.getUInt32()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt16: {
            auto type = lpm1::Value::UInt16Value;
            auto offset = lpm1::CreateUInt16Value(buffer, value.getUInt16()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt8: {
            auto type = lpm1::Value::UInt8Value;
            auto offset = lpm1::CreateUInt8Value(buffer, value.getUInt8()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::String: {
            auto type = lpm1::Value::StringValue;
            auto offset = lpm1::CreateStringValue(buffer, buffer.CreateSharedString(value.stringView())).Union();
            return {type, offset};
        }
        default:
            TU_UNREACHABLE();
    }
}

tempo_utils::Result<lyric_packaging::LyricManifest>
lyric_packaging::ManifestState::toManifest() const
{
    flatbuffers::FlatBufferBuilder buffer;

    std::vector<flatbuffers::Offset<lpm1::NamespaceDescriptor>> namespaces_vector;
    std::vector<flatbuffers::Offset<lpm1::AttrDescriptor>> attrs_vector;
    std::vector<flatbuffers::Offset<lpm1::EntryDescriptor>> entries_vector;
    std::vector<flatbuffers::Offset<lpm1::PathDescriptor>> paths_vector;

    // serialize namespaces
    for (const auto *ns : m_manifestNamespaces) {
        auto fb_nsUrl = buffer.CreateString(ns->getNsUrl().toString());
        namespaces_vector.push_back(lpm1::CreateNamespaceDescriptor(buffer, fb_nsUrl));
    }
    auto fb_namespaces = buffer.CreateVector(namespaces_vector);

    // serialize attributes
    for (const auto *attr : m_manifestAttrs) {
        auto id = attr->getAttrId();
        auto value = attr->getAttrValue();
        auto p = serialize_value(buffer, value);
        attrs_vector.push_back(lpm1::CreateAttrDescriptor(buffer,
            id.getAddress().getAddress(), id.getType(), p.first, p.second));
    }
    auto fb_attrs = buffer.CreateVector(attrs_vector);

    // serialize entries
    for (const auto *entry : m_manifestEntries) {
        auto pathString = entry->getEntryPath().toString();
        auto fb_path = buffer.CreateSharedString(pathString);

        // map the entry type
        lpm1::EntryType type;
        switch (entry->getEntryType()) {
            case EntryType::Package:
                type = lpm1::EntryType::Package;
                break;
            case EntryType::File:
                type = lpm1::EntryType::File;
                break;
            case EntryType::Directory:
                type = lpm1::EntryType::Directory;
                break;
            case EntryType::Link:
                type = lpm1::EntryType::Link;
                break;
            default:
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "invalid entry type");
        }

        // serialize entry attrs
        std::vector<uint32_t> entry_attrs;
        for (auto iterator = entry->attrsBegin(); iterator != entry->attrsEnd(); iterator++) {
            entry_attrs.push_back(iterator->second.getAddress());
        }
        auto fb_entry_attrs = buffer.CreateVector(entry_attrs);

        // serialize entry children
        std::vector<uint32_t> entry_children;
        for (auto iterator = entry->childrenBegin(); iterator != entry->childrenEnd(); iterator++) {
            entry_children.push_back(iterator->second.getAddress());
        }
        auto fb_entry_children = buffer.CreateVector(entry_children);

        // append entry
        entries_vector.push_back(lpm1::CreateEntryDescriptor(buffer,
            fb_path, type, fb_entry_attrs, fb_entry_children,
            entry->getEntryOffset(), entry->getEntrySize(),
            entry->getEntryDict().getAddress(), entry->getEntryLink().getAddress()));

        // append path
        paths_vector.push_back(lpm1::CreatePathDescriptor(buffer, fb_path, entry->getAddress().getAddress()));
    }
    auto fb_entries = buffer.CreateVector(entries_vector);

    // serialize sorted paths
    auto fb_paths = buffer.CreateVectorOfSortedTables(&paths_vector);

    // build package from buffer
    lpm1::ManifestBuilder manifestBuilder(buffer);

    manifestBuilder.add_abi(lpm1::ManifestVersion::Version1);
    manifestBuilder.add_namespaces(fb_namespaces);
    manifestBuilder.add_attrs(fb_attrs);
    manifestBuilder.add_entries(fb_entries);
    manifestBuilder.add_paths(fb_paths);

    // serialize package and mark the buffer as finished
    auto manifest = manifestBuilder.Finish();
    buffer.Finish(manifest, lpm1::ManifestIdentifier());

    // copy the flatbuffer into our own byte array and instantiate package
    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
    return LyricManifest(bytes);
}
