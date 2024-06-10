//#ifndef LYRIC_OBJECT_REST_PARAMETER_WALKER_H
//#define LYRIC_OBJECT_REST_PARAMETER_WALKER_H
//
//#include <lyric_common/type_def.h>
//
//#include "object_types.h"
//#include "type_walker.h"
//
//namespace lyric_object {
//
//    // forward declarations
//
//    class RestParameterWalker {
//
//    public:
//        RestParameterWalker();
//        RestParameterWalker(const RestParameterWalker &other);
//
//        bool isValid() const;
//
//        Parameter getParameter() const;
//
//        PlacementType getPlacement() const;
//        bool isVariable() const;
//
//        std::string getParameterName() const;
//        TypeWalker getParameterType() const;
//
//    private:
//        std::shared_ptr<const internal::ObjectReader> m_reader;
//        void *m_parameter;
//        void *m_names;
//
//        RestParameterWalker(
//            std::shared_ptr<const internal::ObjectReader> reader,
//            void *parameter,
//            void *names);
//
//        friend class ActionWalker;
//        friend class CallWalker;
//    };
//}
//
//#endif // LYRIC_OBJECT_REST_PARAMETER_WALKER_H
