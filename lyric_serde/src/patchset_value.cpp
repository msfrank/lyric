
#include <lyric_serde/patchset_value.h>

lyric_serde::PatchsetValue::PatchsetValue(
    std::unique_ptr<ValueVariant> &&variant,
    ValueAddress address,
    PatchsetState *state)
    : m_variant(std::move(variant)),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

const lyric_serde::ValueVariant *
lyric_serde::PatchsetValue::getValue() const
{
    return m_variant.get();
}

lyric_serde::ValueAddress
lyric_serde::PatchsetValue::getAddress() const
{
    return m_address;
}