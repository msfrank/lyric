#ifndef ZURI_CORE_NATIVE_CORE_H
#define ZURI_CORE_NATIVE_CORE_H

#include <boost/predef.h>

#include <lyric_runtime/native_interface.h>

class NativeCore : public lyric_runtime::NativeInterface {

public:
    NativeCore() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(tu_uint32 index) const override;
    tu_uint32 numTraps() const override;
};

#if defined(BOOST_OS_LINUX) || defined(BOOST_OS_MACOS)

extern "C" const lyric_runtime::NativeInterface *native_init();

#elif defined(BOOST_OS_WINDOWS)

__declspec(dllexport) const lyric_runtime::NativeInterface *native_init();

#endif

#endif // ZURI_CORE_NATIVE_CORE_H
