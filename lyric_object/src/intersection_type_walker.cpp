
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/internal/type_utils.h>
#include <lyric_object/intersection_type_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::IntersectionTypeWalker::IntersectionTypeWalker()
    : m_intersectionAssignable(nullptr)
{
}

lyric_object::IntersectionTypeWalker::IntersectionTypeWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
     void *intersectionAssignable)
    : m_reader(reader),
      m_intersectionAssignable(intersectionAssignable)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_intersectionAssignable != nullptr);
}

lyric_object::IntersectionTypeWalker::IntersectionTypeWalker(const IntersectionTypeWalker &other)
    : m_reader(other.m_reader),
      m_intersectionAssignable(other.m_intersectionAssignable)
{
}

bool
lyric_object::IntersectionTypeWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_intersectionAssignable;
}

lyric_common::TypeDef
lyric_object::IntersectionTypeWalker::getTypeDef() const
{
    if (!isValid())
        return {};
    auto *intersectionAssignable = static_cast<const lyo1::IntersectionAssignable *>(m_intersectionAssignable);

    std::vector<lyric_common::TypeDef> parameters;
    if (intersectionAssignable->intersection_members()) {
        for (const auto &index : *intersectionAssignable->intersection_members()) {
            TypeWalker walker(m_reader, index);
            if (!walker.isValid())
                return {};
            auto parameter = walker.getTypeDef();
            if (!parameter.isValid())
                return {};
            parameters.push_back(parameter);
        }
    }

    return lyric_common::TypeDef::forIntersection(parameters);
}

std::vector<lyric_object::TypeWalker>
lyric_object::IntersectionTypeWalker::getMembers() const
{
    if (!isValid())
        return {};
    auto *intersectionAssignable = static_cast<const lyo1::IntersectionAssignable *>(m_intersectionAssignable);
    std::vector<TypeWalker> members;
    if (intersectionAssignable->intersection_members()) {
        for (tu_uint32 p : *intersectionAssignable->intersection_members()) {
            members.push_back(TypeWalker(m_reader, p));
        }
    }
    return members;
}
