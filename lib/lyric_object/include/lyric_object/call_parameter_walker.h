#ifndef LYRIC_OBJECT_CALL_PARAMETER_WALKER_H
#define LYRIC_OBJECT_CALL_PARAMETER_WALKER_H

#include <lyric_common/type_def.h>

#include "object_types.h"
#include "type_walker.h"

namespace lyric_object {

    // forward declarations
    class CallWalker;
    class LinkWalker;

    class CallParameterWalker {

    public:
        CallParameterWalker();
        CallParameterWalker(const CallParameterWalker &other);

        bool isValid() const;

        Parameter getParameter() const;

        PlacementType getPlacement() const;
        bool isVariable() const;

        std::string getParameterName() const;
        std::string getParameterLabel() const;
        TypeWalker getParameterType() const;

        bool hasInitializer() const;
        AddressType initializerAddressType() const;
        CallWalker getNearInitializer() const;
        LinkWalker getFarInitializer() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_callDescriptor;
        tu_uint8 m_parameterOffset;

        CallParameterWalker(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *callDescriptor,
            tu_uint8 parameterOffset);

        friend class CallWalker;
    };
}

#endif // LYRIC_OBJECT_CALL_PARAMETER_WALKER_H
