#ifndef LYRIC_ASSEMBLER_BASE_HANDLE_H
#define LYRIC_ASSEMBLER_BASE_HANDLE_H

#include "assembler_result.h"

namespace lyric_assembler {

    /**
     * Base handle implementation which abstracts loading and manages private handle data.
     *
     * @tparam PrivType The private data type for the handle.
     */
    template <class PrivType>
    class BaseHandle {

    public:
        BaseHandle() = default;

        virtual ~BaseHandle()
        {
            delete m_priv;
        }

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
        explicit BaseHandle(PrivType *priv)
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
                auto *thiz = const_cast<BaseHandle *>(this);
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

#endif // LYRIC_ASSEMBLER_BASE_HANDLE_H
