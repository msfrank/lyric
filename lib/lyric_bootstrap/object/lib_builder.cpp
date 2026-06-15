#include <tempo_utils/file_writer.h>
#include <tempo_utils/log_stream.h>

#include <lyric_runtime/library_plugin.h>
#include <lyric_runtime/trap_index.h>

#include "builder_state.h"
#include "compile_action.h"
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
#include "compile_converter.h"
#include "compile_descriptor.h"
#include "compile_discard_protocol.h"
#include "compile_enum.h"
#include "compile_equality.h"
#include "compile_existential.h"
#include "compile_field.h"
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
#include "compile_num.h"
#include "compile_object.h"
#include "compile_ordered.h"
#include "compile_pair.h"
#include "compile_prelude.h"
#include "compile_proposition.h"
#include "compile_protocol.h"
#include "compile_record.h"
#include "compile_rest.h"
#include "compile_seq.h"
#include "compile_singleton.h"
#include "compile_status.h"
#include "compile_string.h"
#include "compile_struct.h"
#include "compile_tuple.h"
#include "compile_type.h"
#include "compile_undef.h"
#include "compile_unwrap.h"
#include "compile_varargs.h"

#include "native_prelude.h"
#include "prelude_symbols.h"

#define NUM_FUNCTION_CLASSES        8
#define NUM_TUPLE_CLASSES           8

int
main(int argc, char *argv[])
{
    // we expect a single argument which is the destination path where to write the object
    if (argc != 2)
        return -1;
    std::filesystem::path destinationPath(argv[1]);

    auto location = lyric_common::ModuleLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION);
    BuilderState state(location);

    PreludeSymbols preludeSymbols;

    // define the Any existential, which is the top of the main type hierarchy
    preludeSymbols.AnyExistential = build_core_Any(state);

    // define the Undef existential, which exists in its own isolated type hierarchy
    preludeSymbols.UndefExistential = build_core_Undef(state);

    // define Intrinsic existential
    preludeSymbols.IntrinsicExistential = build_core_Intrinsic(state, preludeSymbols);

    // declare intrinsic existentials
    preludeSymbols.NilExistential = build_core_Nil(state, preludeSymbols);
    preludeSymbols.BoolExistential = declare_core_Bool(state, preludeSymbols);
    preludeSymbols.CharExistential = declare_core_Char(state, preludeSymbols);
    preludeSymbols.NumExistential = declare_core_Num(state, preludeSymbols);
    preludeSymbols.StringExistential = declare_core_String(state, preludeSymbols);
    preludeSymbols.BytesExistential = declare_core_Bytes(state, preludeSymbols);

    // signed integral types
    preludeSymbols.I64Existential = declare_core_I64(state, preludeSymbols);
    preludeSymbols.I32Existential = declare_core_I32(state, preludeSymbols);
    preludeSymbols.I16Existential = declare_core_I16(state, preludeSymbols);
    preludeSymbols.I8Existential = declare_core_I8(state, preludeSymbols);

    // unsigned integral types
    preludeSymbols.U64Existential = declare_core_U64(state, preludeSymbols);
    preludeSymbols.U32Existential = declare_core_U32(state, preludeSymbols);
    preludeSymbols.U16Existential = declare_core_U16(state, preludeSymbols);
    preludeSymbols.U8Existential = declare_core_U8(state, preludeSymbols);

    // floating point types
    preludeSymbols.F32Existential = declare_core_F32(state, preludeSymbols);
    preludeSymbols.F64Existential = declare_core_F64(state, preludeSymbols);

    // declare Rest existential
    preludeSymbols.RestExistential = declare_core_Rest(state, preludeSymbols);

    // declare Descriptor existential
    preludeSymbols.DescriptorExistential = declare_core_Descriptor(state, preludeSymbols);

    // declare Type existential
    preludeSymbols.TypeExistential = declare_core_Type(state, preludeSymbols);

    // define descriptor existentials
    preludeSymbols.ActionExistential = build_core_Action(state, preludeSymbols);
    preludeSymbols.BindingExistential = build_core_Binding(state, preludeSymbols);
    preludeSymbols.CallExistential = build_core_Call(state, preludeSymbols);
    preludeSymbols.ClassExistential = build_core_Class(state, preludeSymbols);
    preludeSymbols.ConceptExistential = build_core_Concept(state, preludeSymbols);
    preludeSymbols.EnumExistential = build_core_Enum(state, preludeSymbols);
    preludeSymbols.ExistentialExistential = build_core_Existential(state, preludeSymbols);
    preludeSymbols.FieldExistential = build_core_Field(state, preludeSymbols);
    preludeSymbols.InstanceExistential = build_core_Instance(state, preludeSymbols);
    preludeSymbols.NamespaceExistential = build_core_Namespace(state, preludeSymbols);
    preludeSymbols.ProtocolExistential = declare_core_Protocol(state, preludeSymbols);
    preludeSymbols.StructExistential = build_core_Struct(state, preludeSymbols);

    // declare (but do not define) root types for concepts, classes, structs, instances, and enums
    preludeSymbols.ObjectClass = declare_core_Object(state, preludeSymbols);
    preludeSymbols.IdeaConcept = declare_core_Idea(state, preludeSymbols);
    preludeSymbols.CategoryEnum = declare_core_Category(state, preludeSymbols);
    preludeSymbols.SingletonInstance = declare_core_Singleton(state, preludeSymbols);
    preludeSymbols.RecordStruct = declare_core_Record(state, preludeSymbols);

    // declare Function classes
    for (int i = 0; i <= NUM_FUNCTION_CLASSES; i++) {
        auto *FunctionNClass = declare_core_FunctionN(state, i, preludeSymbols);
        preludeSymbols.functionClasses.push_back(FunctionNClass);
    }

    // define core concepts
    preludeSymbols.ArithmeticConcept = build_core_Arithmetic(state, preludeSymbols);
    preludeSymbols.ComparisonConcept = build_core_Comparison(state, preludeSymbols);
    preludeSymbols.ConverterConcept = build_core_Converter(state, preludeSymbols);
    preludeSymbols.EqualityConcept = build_core_Equality(state, preludeSymbols);
    preludeSymbols.IteratorConcept = build_core_Iterator(state, preludeSymbols);
    preludeSymbols.IterableConcept = build_core_Iterable(state, preludeSymbols);
    preludeSymbols.OrderedConcept = build_core_Ordered(state, preludeSymbols);
    preludeSymbols.PropositionConcept = build_core_Proposition(state, preludeSymbols);
    preludeSymbols.VariadicConcept = build_core_Varargs(state, preludeSymbols);

    // define descriptor existentials
    build_core_Protocol(state, preludeSymbols);

    // define intrinsic existentials
    build_core_Bool(state, preludeSymbols);
    build_core_Char(state, preludeSymbols);
    build_core_Num(state, preludeSymbols);
    build_core_Descriptor(state, preludeSymbols);
    build_core_Bytes(state, preludeSymbols);
    build_core_String(state, preludeSymbols);

    build_core_I64(state, preludeSymbols);
    build_core_U64(state, preludeSymbols);
    build_core_F64(state, preludeSymbols);
    build_core_F32(state, preludeSymbols);

    // define Type existential
    build_core_Type(state, preludeSymbols);

    // define Function classes
    for (int i = 0; i <= NUM_FUNCTION_CLASSES; i++) {
        build_core_FunctionN(state, i, preludeSymbols);
    }

    // define core reference types
    build_core_Singleton(state, preludeSymbols);
    build_core_Category(state, preludeSymbols);
    build_core_Object(state, preludeSymbols);
    build_core_Record(state, preludeSymbols);

    // status struct hierarchy
    preludeSymbols.StatusStruct = build_core_Status(state, preludeSymbols);
    preludeSymbols.OkStruct = build_core_Ok(state, preludeSymbols);
    preludeSymbols.ErrorStruct = build_core_Error(state, preludeSymbols);

    build_core_Error_code(tempo_utils::StatusCode::kCancelled, "Cancelled", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kInvalidArgument, "InvalidArgument", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kDeadlineExceeded, "DeadlineExceeded", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kNotFound, "NotFound", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kAlreadyExists, "AlreadyExists", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kPermissionDenied, "PermissionDenied", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kUnauthenticated, "Unauthenticated", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kResourceExhausted, "ResourceExhausted", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kFailedPrecondition, "FailedPrecondition", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kAborted, "Aborted", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kUnavailable, "Unavailable", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kOutOfRange, "OutOfRange", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kUnimplemented, "Unimplemented", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kInternal, "Internal", state, preludeSymbols);
    build_core_Error_code(tempo_utils::StatusCode::kUnknown, "Unknown", state, preludeSymbols);

    // define core instances
    build_core_BoolInstance(state, preludeSymbols);
    build_core_CharInstance(state, preludeSymbols);
    build_core_IntInstance(state, preludeSymbols);
    build_core_FloatInstance(state, preludeSymbols);
    build_core_StringInstance(state, preludeSymbols);
    build_core_BytesInstance(state, preludeSymbols);

    for (int i = 1; i < NUM_TUPLE_CLASSES; i++) {
        auto *TupleNClass = declare_core_TupleN(state, i, preludeSymbols);
        preludeSymbols.tupleClasses.push_back(TupleNClass);
        auto *UnwrapNConcept = build_core_UnwrapN(state, i, preludeSymbols);
        preludeSymbols.unwrapConcepts.push_back(UnwrapNConcept);
        build_core_TupleN(state, i, preludeSymbols);
    }

    // define data union type
    preludeSymbols.DataUnionType = state.addUnionType({
        preludeSymbols.IntrinsicExistential->existentialType,
        preludeSymbols.RecordStruct->structType});

    // define data union iterator type
    preludeSymbols.DataIteratorType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        preludeSymbols.IteratorConcept->concept_index, {preludeSymbols.DataUnionType});

    // define Pair struct
    preludeSymbols.PairStruct = build_core_Pair(state, preludeSymbols);

    // define Rest existential
    preludeSymbols.RestIteratorClass = build_core_RestIterator(state, preludeSymbols);
    build_core_Rest(state, preludeSymbols);

    // define Seq struct
    preludeSymbols.SeqIteratorClass = build_core_SeqIterator(state, preludeSymbols);
    build_core_Seq(state, preludeSymbols);

    // define Map struct
    preludeSymbols.MapIteratorClass = build_core_MapIterator(state, preludeSymbols);
    build_core_Map(state, preludeSymbols);

    // define DiscardProtocol
    preludeSymbols.DiscardProtocol = build_core_DiscardProtocol(state, preludeSymbols);

    // define prelude functions
    build_core_prelude_trap(state, preludeSymbols);
    build_core_prelude_va_load(state, preludeSymbols);
    build_core_prelude_va_size(state, preludeSymbols);

    auto object = state.toObject();

    // validate that every trap refers to a trap present in the prelude plugin
    auto trapIndex = std::make_shared<lyric_runtime::TrapIndex>(&kPreludeInterface);
    TU_RAISE_IF_NOT_OK (trapIndex->initialize());
    TU_ASSERT (object.hasPlugin());
    auto walker = object.getPlugin();
    for (tu_uint32 i = 0; i < walker.numTraps(); ++i) {
        auto trapNumber = trapIndex->lookupTrap(walker.trapValue(i));
        TU_ASSERT (trapNumber != lyric_object::INVALID_ADDRESS_U32);
    }

    // write object to file
    tempo_utils::FileWriter writer(destinationPath, object.bytesView(),
        tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    if (!writer.isValid()) {
        TU_LOG_INFO << "failed to write output to " << destinationPath << "; " << writer.getStatus();
        return -1;
    }

    TU_LOG_INFO << "wrote output to " << destinationPath;
    return 0;
}
