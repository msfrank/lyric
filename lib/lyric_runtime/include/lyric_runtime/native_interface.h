#ifndef LYRIC_RUNTIME_NATIVE_INTERFACE_H
#define LYRIC_RUNTIME_NATIVE_INTERFACE_H

#include <tempo_utils/status.h>

namespace lyric_runtime {

    // forward declarations
    class BytecodeInterpreter;
    class InterpreterState;

    typedef tempo_utils::Status (*NativeFunc)(BytecodeInterpreter *, lyric_runtime::InterpreterState *);

    class NativeInterface {

    public:
        virtual ~NativeInterface() = default;
        virtual NativeFunc getTrap(uint32_t index) const = 0;
        virtual uint32_t numTraps() const = 0;
    };

    typedef const NativeInterface *(*NativeInitFunc)();
}

#endif // LYRIC_RUNTIME_NATIVE_INTERFACE_H
