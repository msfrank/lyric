#ifndef LYRIC_SCHEMA_POSIX_STAT_SCHEMA_H
#define LYRIC_SCHEMA_POSIX_STAT_SCHEMA_H

#include <array>

#include <tempo_utils/schema.h>

namespace lyric_schema {

    class PosixStatNs : public tempo_utils::SchemaNs {
    public:
        constexpr PosixStatNs() : tempo_utils::SchemaNs("http://www.w3.org/ns/posix/stat") {};
    };
    constexpr PosixStatNs kPosixStatNs;

    enum class PosixStatId {
        ATime,          // "time of last access"
        BlockSize,      // "blocksize for file system I/O"
        Blocks,         // "number of 512 byte blocks allocated"
        CTime,          // "time of last status change"
        Dev,            // "ID of device containing file"
        Gid,            // "group ID of owner"
        Ino,            // "inode number"
        Mode,           // "protection"
        MTime,          // "time of last modification"
        NLink,          // "number of hard links"
        RDev,           // "device ID (if special file)"
        Size,           // "total size, in bytes"
        Uid,            // "user ID of owner"
        NUM_IDS,
    };

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatATimeProperty(
        &kPosixStatNs, PosixStatId::ATime, "atime", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatBlockSizeProperty(
        &kPosixStatNs, PosixStatId::BlockSize, "blocksize", tempo_utils::PropertyType::kUInt64);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatBlocksProperty(
        &kPosixStatNs, PosixStatId::Blocks, "blocks", tempo_utils::PropertyType::kUInt64);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatCTimeProperty(
        &kPosixStatNs, PosixStatId::CTime, "ctime", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatDevProperty(
        &kPosixStatNs, PosixStatId::Dev, "dev", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatGidProperty(
        &kPosixStatNs, PosixStatId::Gid, "gid", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatInoProperty(
        &kPosixStatNs, PosixStatId::Ino, "ino", tempo_utils::PropertyType::kUInt64);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatModeProperty(
        &kPosixStatNs, PosixStatId::Mode, "mode", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatMTimeProperty(
        &kPosixStatNs, PosixStatId::MTime, "mtime", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatNLinkProperty(
        &kPosixStatNs, PosixStatId::NLink, "nlink", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatRDevProperty(
        &kPosixStatNs, PosixStatId::RDev, "rdev", tempo_utils::PropertyType::kUInt32);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatSizeProperty(
        &kPosixStatNs, PosixStatId::Size, "size", tempo_utils::PropertyType::kUInt64);

    constexpr tempo_utils::SchemaProperty<PosixStatNs,PosixStatId>
    kPosixStatUidProperty(
        &kPosixStatNs, PosixStatId::Uid, "uid", tempo_utils::PropertyType::kUInt32);
}

#endif // LYRIC_SCHEMA_POSIX_STAT_SCHEMA_H