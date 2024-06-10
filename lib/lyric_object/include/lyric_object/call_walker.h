#ifndef LYRIC_OBJECT_CALL_WALKER_H
#define LYRIC_OBJECT_CALL_WALKER_H

#include <span>

#include "bytecode_iterator.h"
#include "object_types.h"
#include "template_walker.h"

namespace lyric_object {

    // forward declarations
    class ParameterWalker;
    class SymbolWalker;
    class TypeWalker;

    class CallWalker {

    public:
        CallWalker();
        CallWalker(const CallWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        bool isBound() const;
        bool isNoReturn() const;
        bool isDeclOnly() const;

        AccessType getAccess() const;
        CallMode getMode() const;

        bool hasTemplate() const;
        TemplateWalker getTemplate() const;

        bool hasReceiver() const;
        SymbolWalker getReceiver() const;

        tu_uint8 numListParameters() const;
        ParameterWalker getListParameter(tu_uint8 index) const;

        tu_uint8 numNamedParameters() const;
        ParameterWalker getNamedParameter(tu_uint8 index) const;

        bool hasRestParameter() const;
        ParameterWalker getRestParameter() const;

        std::span<const tu_uint8> getProc() const;
        ProcHeader getProcHeader() const;
        tu_uint32 getProcOffset() const;
        BytecodeIterator getBytecodeIterator() const;

        TypeWalker getCallType() const;
        TypeWalker getResultType() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_callOffset;

        CallWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 callOffset);

        friend class ClassMethod;
        friend class ClassWalker;
        friend class EnumMethod;
        friend class EnumWalker;
        friend class ExistentialMethod;
        friend class ExtensionWalker;
        friend class FieldWalker;
        friend class InstanceMethod;
        friend class InstanceWalker;
        friend class ObjectWalker;
        friend class ParameterWalker;
        friend class StaticWalker;
        friend class StructMethod;
        friend class StructWalker;
    };
}

#endif // LYRIC_OBJECT_CALL_WALKER_H
