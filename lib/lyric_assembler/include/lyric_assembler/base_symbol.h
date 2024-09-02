#ifndef LYRIC_ASSEMBLER_BASE_SYMBOL_H
#define LYRIC_ASSEMBLER_BASE_SYMBOL_H

#include "abstract_symbol.h"
#include "base_handle.h"
#include "assembler_result.h"

namespace lyric_assembler {

    /**
     * Base implementation of an abstract symbol with methods to manage the type-specific private data. In
     * particular the getPriv protected methods allow the symbol to load the private data from const qualified
     * methods.
     *
     * @tparam PrivType The type-specific private data struct for the symbol.
     */
    template <class PrivType>
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
        bool isImported() const override
        {
            return m_isImported;
        }

    protected:
        /**
         *
         * @param address
         */
        explicit BaseSymbol(PrivType *priv)
            : m_isImported(false),
              m_priv(priv)
        {
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
        const bool m_isImported = true;
        PrivType *m_priv = nullptr;
    };
}

#endif // LYRIC_ASSEMBLER_BASE_SYMBOL_H
