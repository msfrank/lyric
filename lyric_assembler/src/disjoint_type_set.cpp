
#include <lyric_assembler/disjoint_type_set.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/type_handle.h>

lyric_assembler::DisjointTypeSet::DisjointTypeSet(const lyric_assembler::AssemblyState *state)
    : m_state(state),
      m_root(new TypeLevel())
{
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::DisjointTypeSet::~DisjointTypeSet()
{
    delete m_root;
}

static bool
address_cmp(const lyric_assembler::TypeLevel *lhs, const lyric_assembler::TypeLevel *rhs)
{
    return lhs->address.getAddress() < rhs->address.getAddress();
}

tempo_utils::Status
lyric_assembler::DisjointTypeSet::putType(const lyric_common::TypeDef &type)
{
    auto *typeHandle = m_state->typeCache()->getType(type);
    if (typeHandle == nullptr)
        m_state->throwAssemblerInvariant("missing type {} in type cache", type.toString());
    typeHandle->touch();

    auto signature = typeHandle->getTypeSignature();
    if (!signature.isValid())
        m_state->throwAssemblerInvariant("type {} has invalid signature", type.toString());

    TypeLevel *curr = m_root;

    //
    for (auto iterator = signature.signatureBegin(); iterator != signature.signatureEnd(); iterator++) {

        // search for the next type address in the current level
        TypeLevel cmp;
        cmp.address = *iterator;
        auto lower = std::lower_bound(curr->children.cbegin(), curr->children.cend(), &cmp, address_cmp);

        if (lower == curr->children.cend() || (*lower)->address != *iterator) {
            // if curr does not contain the type address then add a new level
            auto *child = new TypeLevel{};
            child->address = *iterator;
            curr->children.push_back(child);
            std::sort(curr->children.begin(), curr->children.end(), address_cmp);
            curr = child;
        } else {
            // if curr does contain the type address and the level is a leaf then type is not disjoint in the set
            curr = *lower;
            if (curr->leaf.isValid())
                return AssemblerStatus::forCondition(AssemblerCondition::kIncompatibleType,
                    "type {} is not disjoint with {}", type.toString(), curr->leaf.toString());
        }
    }

    // the type is disjoint in the set, so set the leaf type
    TU_ASSERT (!curr->leaf.isValid());
    curr->leaf = type;

    return AssemblerStatus::ok();
}

lyric_assembler::TypeLevel::~TypeLevel()
{
    for (auto *child : children) {
        delete child;
    }
}
