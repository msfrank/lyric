
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/internal/type_utils.h>
#include <lyric_object/type_walker.h>
#include <lyric_object/union_type_walker.h>

lyric_object::UnionTypeWalker::UnionTypeWalker()
    : m_unionAssignable(nullptr)
{
}

lyric_object::UnionTypeWalker::UnionTypeWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
     void *unionAssignable)
    : m_reader(reader),
      m_unionAssignable(unionAssignable)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_unionAssignable != nullptr);
}

lyric_object::UnionTypeWalker::UnionTypeWalker(const UnionTypeWalker &other)
    : m_reader(other.m_reader),
      m_unionAssignable(other.m_unionAssignable)
{
}

bool
lyric_object::UnionTypeWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_unionAssignable;
}

lyric_common::TypeDef
lyric_object::UnionTypeWalker::getTypeDef() const
{
    if (!isValid())
        return {};
    auto *unionAssignable = static_cast<const lyo1::UnionAssignable *>(m_unionAssignable);

    std::vector<lyric_common::TypeDef> parameters;
    if (unionAssignable->union_members()) {
        for (const auto &index : *unionAssignable->union_members()) {
            TypeWalker walker(m_reader, index);
            if (!walker.isValid())
                return {};
            auto parameter = walker.getTypeDef();
            if (!parameter.isValid())
                return {};
            parameters.push_back(parameter);
        }
    }

    auto result = lyric_common::TypeDef::forUnion(parameters);
    return result.orElse({});
}

std::vector<lyric_object::TypeWalker>
lyric_object::UnionTypeWalker::getMembers() const
{
    if (!isValid())
        return {};
    auto *unionAssignable = static_cast<const lyo1::UnionAssignable *>(m_unionAssignable);
    std::vector<TypeWalker> members;
    if (unionAssignable->union_members()) {
        for (tu_uint32 p : *unionAssignable->union_members()) {
            members.push_back(TypeWalker(m_reader, p));
        }
    }
    return members;
}
