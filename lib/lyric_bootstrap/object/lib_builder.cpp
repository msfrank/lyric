#include <tempo_utils/file_writer.h>
#include <tempo_utils/log_stream.h>

#include <lyric_runtime/trap_index.h>

#include "builder_state.h"
#include "compile_any.h"
#include "compile_arithmetic.h"
#include "compile_binding.h"
#include "compile_bool.h"
#include "compile_bytes.h"
#include "compile_call.h"
#include "compile_category.h"
#include "compile_char.h"
#include "compile_class.h"
#include "compile_comparison.h"
#include "compile_concept.h"
#include "compile_descriptor.h"
#include "compile_enum.h"
#include "compile_equality.h"
#include "compile_float.h"
#include "compile_function.h"
#include "compile_idea.h"
#include "compile_int.h"
#include "compile_instance.h"
#include "compile_intrinsic.h"
#include "compile_iterable.h"
#include "compile_iterator.h"
#include "compile_map.h"
#include "compile_namespace.h"
#include "compile_nil.h"
#include "compile_undef.h"
#include "compile_object.h"
#include "compile_ordered.h"
#include "compile_pair.h"
#include "compile_prelude.h"
#include "compile_proposition.h"
#include "compile_record.h"
#include "compile_rest.h"
#include "compile_seq.h"
#include "compile_singleton.h"
#include "compile_string.h"
#include "compile_struct.h"
#include "compile_tuple.h"
#include "compile_type.h"
#include "compile_unwrap.h"
#include "compile_url.h"
#include "compile_varargs.h"
#include "compile_status.h"
#include "lyric_runtime/library_plugin.h"

#define NUM_FUNCTION_CLASSES        8
#define NUM_TUPLE_CLASSES           8

int
main(int argc, char *argv[])
{
    // we expect a single argument which is the destination path where to write the assembly
    if (argc != 2)
        return -1;
    std::filesystem::path destinationPath(argv[1]);

    auto location = lyric_common::ModuleLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION);
    auto preludeLib = std::make_shared<tempo_utils::LibraryLoader>(PRELUDE_PLUGIN_PATH, "native_init");
    auto native_init = (lyric_runtime::NativeInitFunc) preludeLib->symbolPointer();
    auto preludePlugin = std::make_shared<lyric_runtime::LibraryPlugin>(preludeLib, native_init());
    auto trapIndex = std::make_shared<lyric_runtime::TrapIndex>(preludePlugin);
    TU_RAISE_IF_NOT_OK (trapIndex->initialize());

    BuilderState state(location, trapIndex);

    // define the Any existential, which is the top of the type hierarchy
    auto *AnyExistential = build_core_Any(state);

    // define the Nil existential, which exists in its own type hierarchy
    auto *NilExistential = build_core_Nil(state);

    // define Intrinsic existential
    auto *IntrinsicExistential = build_core_Intrinsic(state, AnyExistential);

    // declare intrinsic existentials
    build_core_Undef(state, IntrinsicExistential);
    auto *BoolExistential = declare_core_Bool(state, IntrinsicExistential);
    auto *CharExistential = declare_core_Char(state, IntrinsicExistential);
    auto *IntExistential = declare_core_Int(state, IntrinsicExistential);
    auto *FloatExistential = declare_core_Float(state, IntrinsicExistential);
    auto *BytesExistential = declare_core_Bytes(state, IntrinsicExistential);
    auto *StringExistential = declare_core_String(state, IntrinsicExistential);
    auto *UrlExistential = declare_core_Url(state, IntrinsicExistential);

    // declare Rest existential
    auto *RestExistential = declare_core_Rest(state, AnyExistential);

    // declare Descriptor existential
    auto *DescriptorExistential = declare_core_Descriptor(state, AnyExistential);

    // declare Type existential
    auto *TypeExistential = declare_core_Type(state, AnyExistential);

    // define descriptor existentials
    build_core_Binding(state, DescriptorExistential);
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

    // define core concepts
    auto *ArithmeticConcept = build_core_Arithmetic(state, IdeaConcept);
    auto *ComparisonConcept = build_core_Comparison(state, IdeaConcept, BoolExistential->existentialType);
    auto *EqualityConcept = build_core_Equality(state, IdeaConcept, BoolExistential->existentialType);
    auto *IteratorConcept = build_core_Iterator(state, IdeaConcept, BoolExistential->existentialType);
    auto *IterableConcept = build_core_Iterable(state, IdeaConcept, IteratorConcept);
    auto *OrderedConcept = build_core_Ordered(state, IdeaConcept, IntExistential->existentialType);
    auto *PropositionConcept = build_core_Proposition(state, IdeaConcept, BoolExistential->existentialType);
    auto *UnwrapConcept = build_core_Unwrap(state, IdeaConcept);
    build_core_Varargs(state, IdeaConcept);

    // define intrinsic existentials
    build_core_Bool(state, BoolExistential);
    build_core_Char(state, CharExistential);
    build_core_Int(state, IntExistential);
    build_core_Float(state, FloatExistential);
    build_core_Descriptor(state, DescriptorExistential);
    build_core_Bytes(state, BytesExistential, IntExistential->existentialType,
        StringExistential->existentialType);
    build_core_String(state, StringExistential, IntExistential->existentialType,
        CharExistential->existentialType, BytesExistential->existentialType);
    build_core_Url(state, UrlExistential);

    // define Type existential
    build_core_Type(state, TypeExistential, IntExistential->existentialType,
        BoolExistential->existentialType);

    // define Function classes
    for (int i = 0; i <= NUM_FUNCTION_CLASSES; i++) {
        build_core_FunctionN(state, i, functionClasses[i], CallExistential->existentialType);
    }

    // define core reference types
    build_core_Singleton(state, SingletonInstance);
    build_core_Category(state, CategoryEnum);
    build_core_Object(state, ObjectClass);
    build_core_Record(state, RecordStruct);

    // core status structs
    auto *StatusStruct = build_core_Status(
        state, IntExistential->existentialType, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kCancelled,
        "Cancelled", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kInvalidArgument,
        "InvalidArgument", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kDeadlineExceeded,
        "DeadlineExceeded", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kNotFound,
        "NotFound", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kAlreadyExists,
        "AlreadyExists", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kPermissionDenied,
        "PermissionDenied", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kUnauthenticated,
        "Unauthenticated", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kResourceExhausted,
        "ResourceExhausted", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kFailedPrecondition,
        "FailedPrecondition", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kAborted,
        "Aborted", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kUnavailable,
        "Unavailable", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kOutOfRange,
        "OutOfRange", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kUnimplemented,
        "Unimplemented", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kInternal,
        "Internal", state, StatusStruct, StringExistential->existentialType);
    build_core_Status_code(tempo_utils::StatusCode::kUnknown,
        "Unknown", state, StatusStruct, StringExistential->existentialType);

    // define core instances
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
    build_core_BytesInstance(state, BytesExistential->existentialType, SingletonInstance,
        ComparisonConcept, EqualityConcept, OrderedConcept,
        IntExistential->existentialType, BoolExistential->existentialType);
    build_core_StringInstance(state, StringExistential->existentialType, SingletonInstance,
        ComparisonConcept, EqualityConcept, OrderedConcept,
        CharExistential->existentialType, IntExistential->existentialType, BoolExistential->existentialType);
    build_core_UrlInstance(state, UrlExistential->existentialType, SingletonInstance,
        EqualityConcept,
        IntExistential->existentialType, BoolExistential->existentialType);

    for (int i = 1; i < NUM_TUPLE_CLASSES; i++) {
        auto *TupleNClass = build_core_TupleN(state, i, ObjectClass);
        build_core_TupleNInstance(state, TupleNClass, SingletonInstance, UnwrapConcept);
    }

    // define Data union type
    auto *DataUnionType = state.addUnionType({IntrinsicExistential->existentialType, RecordStruct->structType});
    auto *DataIterableType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        IterableConcept->concept_index, {DataUnionType});
    auto *DataIteratorType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        IteratorConcept->concept_index, {DataUnionType});

    // define Pair struct
    build_core_Pair(state, RecordStruct, DataUnionType);

    // define Rest existential
    auto *RestIteratorClass = build_core_RestIterator(state, ObjectClass, IteratorConcept,
        BoolExistential->existentialType);
    build_core_Rest(state, RestExistential, IterableConcept, IterableConcept, RestIteratorClass,
        IntExistential->existentialType, NilExistential->existentialType);

    // define Seq struct
    auto *SeqIteratorClass = build_core_SeqIterator(state, ObjectClass, IteratorConcept, DataUnionType,
        DataIteratorType, BoolExistential->existentialType);
    build_core_Seq(state, RecordStruct, IteratorConcept, IterableConcept, SeqIteratorClass, DataUnionType,
        DataIteratorType, DataIterableType, BoolExistential->existentialType, IntExistential->existentialType);

    // define Map struct
    auto *MapIteratorClass = build_core_MapIterator(state, ObjectClass, IteratorConcept, DataUnionType,
        DataIteratorType, BoolExistential->existentialType);
    build_core_Map(state, RecordStruct, IteratorConcept, IterableConcept, MapIteratorClass, DataUnionType,
        DataIteratorType, DataIterableType, BoolExistential->existentialType, IntExistential->existentialType);

    // define prelude functions
    build_core_prelude_trap(state, IntExistential->existentialType, state.noReturnType);
    build_core_prelude_va_load(state, IntExistential->existentialType, AnyExistential->existentialType);
    build_core_prelude_va_size(state, IntExistential->existentialType);

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
