#ifndef LYRIC_ASSEMBLER_BASE_SYMBOL_H
#define LYRIC_ASSEMBLER_BASE_SYMBOL_H

#include "abstract_symbol.h"
#include "base_handle.h"
#include "assembler_result.h"

namespace lyric_assembler {

    /**
     * Base implementation of an abstract symbol with methods to manage the type-specific symbol address.
     *
     * @tparam AddressType The type-specific address type for the symbol.
     */
    template <class AddressType, class PrivType>
    class BaseSymbol : public AbstractSymbol {

    public:
        BaseSymbol() = default;

        virtual ~BaseSymbol()
        {
            delete m_priv;
        }

        /**
         * Returns the type-specific linkage section for the symbol.
         *
         * @return The linkage section.
         */
        virtual lyric_object::LinkageSection getLinkage() const = 0;

        /**
         * Indicates whether the symbol is imported.
         *
         * @return true if the symbol is imported, otherwise false.
         */
        bool isImported() const
        {
            return m_isImported;
        }

    protected:
        /**
         *
         * @param address
         */
        explicit BaseSymbol(AddressType address, PrivType *priv)
            : m_address(address),
              m_isImported(false),
              m_priv(priv)
        {
            TU_ASSERT (m_address.isValid());
            TU_ASSERT (m_priv != nullptr);
        };

        /**
         *
         * @return
         */
        PrivType *getPriv()
        {
            if (m_priv == nullptr) {
                m_priv = load();
            }
            return m_priv;
        };

        /**
         *
         * @return
         */
        const PrivType *getPriv() const
        {
            if (m_priv == nullptr) {
                auto *thiz = const_cast<BaseSymbol *>(this);
                thiz->m_priv = thiz->load();
            }
            return m_priv;
        };

        /**
         *
         */
        virtual PrivType *load() = 0;

    private:
        AddressType m_address;
        const bool m_isImported = true;
        PrivType *m_priv = nullptr;

    public:
        /**
         * Return the type-specific address for the symbol.
         *
         * @return The symbol address.
         */
        AddressType getAddress() const
        {
            return m_address;
        };

        /**
         * Update the type-specific address for the symbol. If the symbol already has an address then an error
         * status is returned.
         *
         * @param address The new symbol address.
         * @return Ok status if the update was successful, otherwise error status.
         */
        tempo_utils::Status updateAddress(AddressType address)
        {
            TU_ASSERT (address.isValid());
            if (m_address.isValid())
                return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                    "address is already set for {}", getSymbolUrl().toString());
            m_address = address;
            return AssemblerStatus::ok();
        };
    };
}

#endif // LYRIC_ASSEMBLER_BASE_SYMBOL_H
