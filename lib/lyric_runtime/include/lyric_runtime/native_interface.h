#ifndef LYRIC_RUNTIME_NATIVE_INTERFACE_H
#define LYRIC_RUNTIME_NATIVE_INTERFACE_H

#include <tempo_utils/status.h>

namespace lyric_runtime {

    // forward declarations
    class BytecodeInterpreter;
    class BytecodeSegment;
    class InterpreterState;
    class NativeInterface;
    class SegmentManager;

    /**
     *
     */
    typedef const NativeInterface *(*NativeInitFunc)();

    /**
     *
     */
    typedef tempo_utils::Status (*NativeFunc)(BytecodeInterpreter *, lyric_runtime::InterpreterState *);

    /**
     * Abstract class describing the interface to a native plugin.
     */
    class NativeInterface {

    public:
        virtual ~NativeInterface() = default;

        /**
         * This method is called when the native plugin is loaded into a segment. This enables the plugin
         * to attach data to the segment which can be used by plugin traps.
         *
         * @param segment
         * @return
         */
        virtual bool load(BytecodeSegment *segment) const = 0;

        /**
         * This method is called right before the associated segment is destroyed. If the plugin has attached
         * data to the segment then the data should be released in this method.
         *
         */
        virtual void unload(BytecodeSegment *segment) const = 0;

        /**
         * Returns the function pointer to the trap at the specified `index`.
         *
         * @param index The trap index.
         * @return The function pointer for the trap, or nullptr if the index is out of range.
         */
        virtual NativeFunc getTrap(uint32_t index) const = 0;

        /**
         * Returns the number of traps contained in the native plugin.
         *
         * @return The number of traps.
         */
        virtual uint32_t numTraps() const = 0;
    };
}

#endif // LYRIC_RUNTIME_NATIVE_INTERFACE_H
