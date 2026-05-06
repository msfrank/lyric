#ifndef LYRIC_ASSEMBLER_TYPE_CONTRACT_H
#define LYRIC_ASSEMBLER_TYPE_CONTRACT_H

#include "abstract_symbol.h"
#include "template_handle.h"

namespace lyric_assembler {

    class TypeContract {
    public:
        TypeContract() = default;
        TypeContract(
            const lyric_common::TypeDef &consumerType,
            const lyric_common::TypeDef &implementationType,
            AbstractSymbol *providerSymbol);
        TypeContract(
            const lyric_common::TypeDef &consumerType,
            const lyric_common::TypeDef &implementationType,
            AbstractSymbol *providerSymbol,
            TemplateHandle *providerTemplate);

        bool isValid() const;

        lyric_common::TypeDef getConsumerType() const;
        lyric_common::TypeDef getImplementationType() const;
        AbstractSymbol *getProviderSymbol() const;
        TemplateHandle *getProviderTemplate() const;

        static tempo_utils::Result<TypeContract> load(
            AbstractSymbol *providerSymbol,
            TypeHandle *implementationTypeHandle);
        static tempo_utils::Result<TypeContract> load(
            AbstractSymbol *providerSymbol,
            const lyric_common::TypeDef &implementationType);

    private:
        struct Priv {
            AbstractSymbol *providerSymbol = nullptr;
            TemplateHandle *providerTemplate = nullptr;
            lyric_common::TypeDef consumerType;
            lyric_common::TypeDef implementationType;
        };
        std::shared_ptr<Priv> m_priv;
    };
}

#endif // LYRIC_ASSEMBLER_TYPE_CONTRACT_H
