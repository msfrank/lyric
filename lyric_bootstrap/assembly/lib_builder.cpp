#include <tempo_utils/file_writer.h>
#include <tempo_utils/log_stream.h>

#include "builder_state.h"
#include "compile_any.h"
#include "compile_arithmetic.h"
#include "compile_bool.h"
#include "compile_call.h"
#include "compile_category.h"
#include "compile_char.h"
#include "compile_class.h"
#include "compile_closure.h"
#include "compile_code.h"
#include "compile_comparison.h"
#include "compile_concept.h"
#include "compile_intrinsic.h"
#include "compile_descriptor.h"
#include "compile_nil.h"
#include "compile_enum.h"
#include "compile_equality.h"
#include "compile_float.h"
#include "compile_function.h"
#include "compile_idea.h"
#include "compile_int.h"
#include "compile_instance.h"
#include "compile_iterator.h"
#include "compile_map.h"
#include "compile_namespace.h"
#include "compile_present.h"
#include "compile_object.h"
#include "compile_ordered.h"
#include "compile_pair.h"
#include "compile_proposition.h"
#include "compile_record.h"
#include "compile_rest.h"
#include "compile_seq.h"
#include "compile_singleton.h"
#include "compile_string.h"
#include "compile_struct.h"
#include "compile_tuple.h"
#include "compile_unwrap.h"
#include "compile_url.h"
#include "compile_utf8.h"
#include "compile_varargs.h"
#include "compile_status.h"

#define NUM_FUNCTION_CLASSES        8
#define NUM_TUPLE_CLASSES           8

int
main(int argc, char *argv[])
{
    // we expect a single argument which is the destination path where to write the assembly
    if (argc != 2)
        return -1;
    std::filesystem::path destinationPath(argv[1]);

    absl::flat_hash_map<std::string,std::string> pluginsMap;

    BuilderState state(pluginsMap);

    // define the Any type, which is the top of the type hierarchy
    auto *AnyExistential = build_core_Any(state);

    // define the Nil type, which exists in its own type hierarchy
    build_core_Nil(state);

    // define the Intrinsic type
    auto *IntrinsicExistential = build_core_Intrinsic(state, AnyExistential);

    // define intrinsic subtypes
    build_core_Present(state, IntrinsicExistential);
    auto *BoolExistential = build_core_Bool(state, IntrinsicExistential);
    auto *CharExistential = build_core_Char(state, IntrinsicExistential);
    auto *IntExistential = build_core_Int(state, IntrinsicExistential);
    auto *FloatExistential = build_core_Float(state, IntrinsicExistential);
    auto *Utf8Existential = build_core_Utf8(state, IntrinsicExistential);

    // define Descriptor type
    auto *DescriptorExistential = build_core_Descriptor(state, AnyExistential);

    // define descriptor subtypes
    build_core_Namespace(state, DescriptorExistential);
    build_core_Class(state, DescriptorExistential);
    build_core_Struct(state, DescriptorExistential);
    build_core_Instance(state, DescriptorExistential);
    build_core_Enum(state, DescriptorExistential);
    build_core_Concept(state, DescriptorExistential);
    auto *CallExistential = build_core_Call(state, DescriptorExistential);

    // declare (but do not define) root types for concepts, classes, structs, instances, and enums
    auto *IdeaConcept = declare_core_Idea(state, AnyExistential);
    auto *ObjectClass = declare_core_Object(state, AnyExistential);
    auto *RecordStruct = declare_core_Record(state, AnyExistential);
    auto *SingletonInstance = declare_core_Singleton(state, AnyExistential);
    auto *CategoryEnum = declare_core_Category(state, AnyExistential);

    // declare Function classes
    std::vector<const CoreClass *> functionClasses;
    for (int i = 0; i <= NUM_FUNCTION_CLASSES; i++) {
        const auto *FunctionNClass = declare_core_FunctionN(state, i, ObjectClass);
        functionClasses.push_back(FunctionNClass);
    }

    // define Function classes
    for (int i = 0; i <= NUM_FUNCTION_CLASSES; i++) {
        build_core_FunctionN(state, i, functionClasses[i], CallExistential->existentialType);
    }

    // define Object and Singleton
    build_core_Singleton(state, SingletonInstance);
    build_core_Category(state, CategoryEnum);
    build_core_Object(state, ObjectClass);
    build_core_Record(state, RecordStruct);

    // core concepts
    auto *ArithmeticConcept = build_core_Arithmetic(state, IdeaConcept);
    auto *ComparisonConcept = build_core_Comparison(state, IdeaConcept, BoolExistential->existentialType);
    auto *EqualityConcept = build_core_Equality(state, IdeaConcept, BoolExistential->existentialType);
    auto *OrderedConcept = build_core_Ordered(state, IdeaConcept, IntExistential->existentialType);
    auto *PropositionConcept = build_core_Proposition(state, IdeaConcept, BoolExistential->existentialType);
    auto *UnwrapConcept = build_core_Unwrap(state, IdeaConcept);
    build_core_Varargs(state, IdeaConcept);

    // core classes
    auto *IteratorClass = build_core_Iterator(state, ObjectClass, BoolExistential->existentialType);
    //build_core_Rest(state, AnyType, VarargsConcept, IntType);

    // core structs
    auto *StringStruct = build_core_String(state, RecordStruct, Utf8Existential->existentialType,
        IntExistential->existentialType, CharExistential->existentialType);
    auto *UrlStruct = build_core_Url(state, RecordStruct, Utf8Existential->existentialType,
        IntExistential->existentialType, CharExistential->existentialType);

    // core status structs
    auto *StatusStruct = build_core_Status(state, RecordStruct, StringStruct->structType);
    build_core_Status_code("Ok", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("Cancelled", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("InvalidArgument", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("DeadlineExceeded", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("NotFound", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("AlreadyExists", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("PermissionDenied", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("Unauthenticated", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("ResourceExhausted", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("FailedPrecondition", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("Aborted", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("Unavailable", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("OutOfRange", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("Unimplemented", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("Internal", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("DataLoss", state, StatusStruct, StringStruct->structType);
    build_core_Status_code("Unknown", state, StatusStruct, StringStruct->structType);

    // core instances
    build_core_BoolInstance(state, BoolExistential->existentialType, SingletonInstance,
        EqualityConcept, OrderedConcept, PropositionConcept,
        IntExistential->existentialType);
    build_core_CharInstance(state, CharExistential->existentialType, SingletonInstance,
        ComparisonConcept, EqualityConcept, OrderedConcept,
        IntExistential->existentialType, BoolExistential->existentialType);
    build_core_IntInstance(state, IntExistential->existentialType, SingletonInstance,
        ArithmeticConcept, ComparisonConcept, EqualityConcept, OrderedConcept,
        BoolExistential->existentialType);
    build_core_FloatInstance(state, FloatExistential->existentialType, SingletonInstance,
        ArithmeticConcept, ComparisonConcept, EqualityConcept, OrderedConcept,
        IntExistential->existentialType, BoolExistential->existentialType);
    build_core_StringInstance(state, StringStruct->structType, SingletonInstance,
        ComparisonConcept, EqualityConcept, OrderedConcept,
        CharExistential->existentialType, IntExistential->existentialType, BoolExistential->existentialType);
    build_core_UrlInstance(state, UrlStruct->structType, SingletonInstance,
        EqualityConcept,
        IntExistential->existentialType, BoolExistential->existentialType);

    for (int i = 1; i < NUM_TUPLE_CLASSES; i++) {
        auto *TupleNClass = build_core_TupleN(state, i, ObjectClass);
        build_core_TupleNInstance(state, TupleNClass, SingletonInstance, UnwrapConcept);
    }

    //
    auto *DataUnionType = state.addUnionType({IntrinsicExistential->existentialType, RecordStruct->structType});

    build_core_Pair(state, RecordStruct, DataUnionType);
    build_core_Map(state, RecordStruct, DataUnionType,
        BoolExistential->existentialType, IntExistential->existentialType);
    build_core_Seq(state, RecordStruct, IteratorClass, DataUnionType,
        BoolExistential->existentialType, IntExistential->existentialType);

    auto bytes = state.toBytes();

    // write assembly to file
    tempo_utils::FileWriter writer(destinationPath, *bytes,
        tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    if (!writer.isValid()) {
        TU_LOG_INFO << "failed to write output " << destinationPath;
        return -1;
    }

    TU_LOG_INFO << "wrote output to " << destinationPath;
    return 0;
}
