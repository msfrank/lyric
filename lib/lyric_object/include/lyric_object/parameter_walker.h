#ifndef LYRIC_OBJECT_PARAMETER_WALKER_H
#define LYRIC_OBJECT_PARAMETER_WALKER_H

#include <lyric_common/type_def.h>

#include "object_types.h"
#include "type_walker.h"

namespace lyric_object {

    // forward declarations
    class ActionWalker;
    class CallWalker;
    class LinkWalker;

    class ParameterWalker {

    public:
        ParameterWalker();
        ParameterWalker(const ParameterWalker &other);

        bool isValid() const;

        Parameter getParameter() const;

        std::string getParameterName() const;
        TypeWalker getParameterType() const;
        PlacementType getPlacement() const;
        bool isVariable() const;
        bool isCtx() const;

        bool hasInitializer() const;
        AddressType initializerAddressType() const;
        CallWalker getNearInitializer() const;
        LinkWalker getFarInitializer() const;

        tu_uint8 getParameterOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_parameter;
        tu_uint8 m_parameterOffset;
        PlacementType m_placement;

        ParameterWalker(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *parameter,
            tu_uint8 parameterOffset,
            PlacementType placement);

        friend class ActionWalker;
        friend class CallWalker;
    };
}

#endif // LYRIC_OBJECT_PARAMETER_WALKER_H
