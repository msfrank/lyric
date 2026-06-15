#ifndef LYRIC_BOOTSTRAP_PRELUDE_SYMBOLS_H
#define LYRIC_BOOTSTRAP_PRELUDE_SYMBOLS_H

#include "builder_state.h"

struct PreludeSymbols {

    CoreExistential *AnyExistential = nullptr;

    CoreExistential *UndefExistential = nullptr;

    /*
     * intrinsics
     */

    CoreExistential *IntrinsicExistential = nullptr;

    CoreExistential *NilExistential = nullptr;
    CoreExistential *BoolExistential = nullptr;
    CoreExistential *CharExistential = nullptr;
    CoreExistential *NumExistential = nullptr;
    CoreExistential *StringExistential = nullptr;
    CoreExistential *BytesExistential = nullptr;

    CoreExistential *I64Existential = nullptr;
    CoreExistential *I32Existential = nullptr;
    CoreExistential *I16Existential = nullptr;
    CoreExistential *I8Existential = nullptr;

    CoreExistential *U64Existential = nullptr;
    CoreExistential *U32Existential = nullptr;
    CoreExistential *U16Existential = nullptr;
    CoreExistential *U8Existential = nullptr;

    CoreExistential *F64Existential = nullptr;
    CoreExistential *F32Existential = nullptr;

    CoreExistential *RestExistential = nullptr;

    /*
     * descriptors
     */

    CoreExistential *DescriptorExistential = nullptr;

    CoreExistential *ActionExistential = nullptr;
    CoreExistential *BindingExistential = nullptr;
    CoreExistential *CallExistential = nullptr;
    CoreExistential *ClassExistential = nullptr;
    CoreExistential *ConceptExistential = nullptr;
    CoreExistential *EnumExistential = nullptr;
    CoreExistential *ExistentialExistential = nullptr;
    CoreExistential *FieldExistential = nullptr;
    CoreExistential *InstanceExistential = nullptr;
    CoreExistential *NamespaceExistential = nullptr;
    CoreExistential *ProtocolExistential = nullptr;
    CoreExistential *StructExistential = nullptr;
    CoreExistential *TypeExistential = nullptr;

    /*
     * archetypic symbols
     */
    CoreClass *ObjectClass = nullptr;
    CoreConcept *IdeaConcept = nullptr;
    CoreEnum *CategoryEnum = nullptr;
    CoreInstance *SingletonInstance = nullptr;
    CoreStruct *RecordStruct = nullptr;

    //
    std::vector<const CoreClass *> functionClasses;
    std::vector<const CoreClass *> tupleClasses;
    std::vector<const CoreConcept *> unwrapConcepts;

    /*
     * concepts
     */
    CoreConcept *ArithmeticConcept = nullptr;
    CoreConcept *ComparisonConcept  = nullptr;
    CoreConcept *ConverterConcept = nullptr;
    CoreConcept *EqualityConcept = nullptr;
    CoreConcept *IteratorConcept = nullptr;
    CoreConcept *IterableConcept = nullptr;
    CoreConcept *OrderedConcept = nullptr;
    CoreConcept *PropositionConcept = nullptr;
    CoreConcept *VariadicConcept = nullptr;

    /*
     * structs
     */
    CoreStruct *StatusStruct = nullptr;
    CoreStruct *OkStruct = nullptr;
    CoreStruct *ErrorStruct = nullptr;
    CoreStruct *PairStruct = nullptr;

    /*
     * classes
     */
    CoreClass *MapIteratorClass = nullptr;
    CoreClass *RestIteratorClass = nullptr;
    CoreClass *SeqIteratorClass = nullptr;

    /*
     * protocols
     */
    CoreProtocol *DiscardProtocol = nullptr;

    /*
     * types
     */
    CoreType *DataUnionType = nullptr;
    CoreType *DataIteratorType = nullptr;
};

#endif // LYRIC_BOOTSTRAP_PRELUDE_SYMBOLS_H
