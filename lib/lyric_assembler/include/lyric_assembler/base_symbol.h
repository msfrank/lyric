#ifndef LYRIC_ASSEMBLER_BASE_SYMBOL_H
#define LYRIC_ASSEMBLER_BASE_SYMBOL_H

#include <lyric_object//object_types.h>

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

        /**
         * Indicates whether the symbol is copied.
         *
         * @return true if the symbol is copied, otherwise false.
         */
        bool isCopied() const override
        {
            return m_isCopied;
        }

    protected:
        /**
         * Constructor for a symbol which is either copied or imported.
         *
         * @param isCopied true if the symbol is being copied into the object state, otherwise false.
         */
        explicit BaseSymbol(bool isCopied)
            : m_isImported(!isCopied),
              m_isCopied(isCopied)
        {
        }

        /**
         * Constructor for a symbol which is declared.
         *
         * @param priv a pointer to private symbol data.
         */
        explicit BaseSymbol(PrivType *priv)
            : m_isImported(false),
              m_isCopied(false),
              m_priv(priv)
        {
            TU_ASSERT (m_priv != nullptr);
        }

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
        }

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
        }

        /**
         *
         */
        virtual PrivType *load() = 0;

    private:
        bool m_isImported;
        bool m_isCopied;
        PrivType *m_priv = nullptr;
    };
}

#endif // LYRIC_ASSEMBLER_BASE_SYMBOL_H
