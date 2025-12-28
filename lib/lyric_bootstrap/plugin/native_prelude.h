#ifndef ZURI_CORE_NATIVE_CORE_H
#define ZURI_CORE_NATIVE_CORE_H

#include <lyric_runtime/native_interface.h>

class NativeCore : public lyric_runtime::NativeInterface {

public:
    NativeCore() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(tu_uint32 index) const override;
    tu_uint32 numTraps() const override;
};

const NativeCore kPreludeInterface;

#endif // ZURI_CORE_NATIVE_CORE_H
