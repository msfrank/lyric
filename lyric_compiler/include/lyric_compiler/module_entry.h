#ifndef LYRIC_COMPILER_MODULE_ENTRY_H
#define LYRIC_COMPILER_MODULE_ENTRY_H

#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_common/symbol_url.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_typing/type_system.h>

namespace lyric_compiler {

    class ModuleEntry {

    public:
        ModuleEntry(lyric_assembler::AssemblyState *state);
        ~ModuleEntry();

        lyric_assembler::AssemblyState *getState() const;

        tempo_utils::Status initialize();

        lyric_common::AssemblyLocation getLocation() const;
        lyric_assembler::NamespaceSymbol *getRoot() const;
        lyric_assembler::CallSymbol *getEntry() const;
        lyric_typing::TypeSystem *getTypeSystem() const;


        void putExitType(const lyric_common::TypeDef &exitType);
        absl::flat_hash_set<lyric_common::TypeDef> listExitTypes() const;

    private:
        lyric_assembler::AssemblyState *m_state;
        lyric_assembler::NamespaceSymbol *m_root;
        lyric_assembler::CallSymbol *m_entry;
        lyric_typing::TypeSystem *m_typeSystem;

    public:
        /**
         *
         * @tparam NsType
         * @tparam IdType
         * @param walker
         * @param schemaClass
         */
        template<class NsType, class IdType>
        void
        checkClassOrThrow(
            const lyric_parser::NodeWalker &walker,
            const tempo_utils::SchemaClass<NsType,IdType> &schemaClass) const
        {
            if (!walker.isClass(schemaClass)) {
                auto status = m_state->logAndContinue(CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "expected {} node", schemaClass.getName());
                throw CompilerException(status);
            }
        }

        /**
         *
         * @tparam NsType
         * @tparam IdType
         * @param walker
         * @param schemaClass
         * @param numChildren
         */
        template<class NsType, class IdType>
        void
        checkClassAndChildCountOrThrow(
            const lyric_parser::NodeWalker &walker,
            const tempo_utils::SchemaClass<NsType,IdType> &schemaClass,
            int numChildren) const
        {
            TU_ASSERT (numChildren >= 0);
            if (!walker.isClass(schemaClass)) {
                auto status = m_state->logAndContinue(CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "expected {} node", schemaClass.getName());
                throw CompilerException(status);
            }
            if (walker.numChildren() != numChildren) {
                auto status = m_state->logAndContinue(CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid {} node; expected {} children but found {}",
                    schemaClass.getName(), numChildren, walker.numChildren());
                throw CompilerException(status);
            }
        }

        /**
         *
         * @tparam NsType
         * @tparam IdType
         * @param walker
         * @param schemaClass
         * @param minChildren
         * @param maxChildren
         */
        template<class NsType, class IdType>
        void
        checkClassAndChildRangeOrThrow(
            const lyric_parser::NodeWalker &walker,
            const tempo_utils::SchemaClass<NsType,IdType> &schemaClass,
            int minChildren,
            int maxChildren = -1) const
        {
            TU_ASSERT (minChildren >= 0);
            if (!walker.isClass(schemaClass)) {
                auto status = m_state->logAndContinue(CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "expected {} node", schemaClass.getName());
                throw CompilerException(status);
            }
            if (walker.numChildren() < minChildren) {
                auto status = m_state->logAndContinue(CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid {} node; expected at least {} children but found {}",
                    schemaClass.getName(), minChildren, walker.numChildren());
                throw CompilerException(status);
            }
            if (maxChildren >= 0) {
                TU_ASSERT (maxChildren >= minChildren);
                if (walker.numChildren() > maxChildren) {
                    auto status = m_state->logAndContinue(CompilerCondition::kSyntaxError,
                        tempo_tracing::LogSeverity::kError,
                        "invalid {} node; expected at most {} children but found {}",
                        schemaClass.getName(), maxChildren, walker.numChildren());
                    throw CompilerException(status);
                }
            }
        }

        /**
         *
         * @tparam AttrType
         * @tparam SerdeType
         * @param walker
         * @param attr
         * @param value
         */
        template<class AttrType, typename SerdeType = typename AttrType::SerdeType>
        void
        parseAttrOrThrow(
            const lyric_parser::NodeWalker &walker,
            const AttrType &attr,
            SerdeType &value) const
        {
            auto status = walker.parseAttr(attr, value);
            if (status.isOk())
                return;
            tempo_utils::AttrStatus attrStatus;
            if (!status.convertTo(attrStatus))
                throw tempo_utils::StatusException(status);

            CompilerStatus compilerStatus;
            switch (attrStatus.getCondition()) {
                case tempo_utils::AttrCondition::kConversionError:
                    compilerStatus = m_state->logAndContinue(CompilerCondition::kSyntaxError,
                        tempo_tracing::LogSeverity::kError,
                        "cannot parse {} attr: {}", attr.getResource()->getName(), attrStatus.toString());
                    break;
                case tempo_utils::AttrCondition::kMissingValue:
                    compilerStatus = m_state->logAndContinue(CompilerCondition::kSyntaxError,
                        tempo_tracing::LogSeverity::kError,
                        "missing {} attr", attr.getResource()->getName());
                    break;
                case tempo_utils::AttrCondition::kParseError:
                    compilerStatus = m_state->logAndContinue(CompilerCondition::kSyntaxError,
                        tempo_tracing::LogSeverity::kError,
                        "invalid {} attr", attr.getResource()->getName());
                    break;
                case tempo_utils::AttrCondition::kWrongType:
                    compilerStatus = m_state->logAndContinue(CompilerCondition::kSyntaxError,
                        tempo_tracing::LogSeverity::kError,
                        "invalid {} attr", attr.getResource()->getName());
                    break;
            }

            throw CompilerException(compilerStatus);
        }

        template<class NsType, class IdType>
        void
        parseIdOrThrow(
            const lyric_parser::NodeWalker &walker,
            const tempo_utils::SchemaVocabulary<NsType,IdType> &vocabulary,
            IdType &id) const
        {
            auto status = walker.parseId(vocabulary, id);
            if (status.notOk())
                throw CompilerException(m_state->logAndContinue(CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "expected node in schema ns {}", vocabulary.getNs()->getNs()));
        }
    };
}

#endif // LYRIC_COMPILER_MODULE_ENTRY_H
