#ifndef ZURI_CORE_BUILDER_STATE_H
#define ZURI_CORE_BUILDER_STATE_H

#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_bootstrap/internal/bootstrap_types.h>
#include <lyric_common/symbol_url.h>
#include <lyric_object/generated/object.h>
#include <lyric_object/bytecode_builder.h>
#include <tempo_utils/option_template.h>

struct CoreTemplate;
struct CoreCall;

struct CoreType {
    tu_uint32 type_index;
    lyo1::Assignable typeAssignable;
    lyo1::IntrinsicType intrinsicType;
    lyo1::TypeSection concreteSection;
    tu_uint32 concreteDescriptor;
    const CoreTemplate *placeholderTemplate;
    tu_uint32 placeholderIndex;
    std::vector<tu_uint32> typeParameters;
    const CoreType *superType;
};

struct CoreTemplate {
    tu_uint32 template_index;
    lyric_common::SymbolPath templatePath;
    absl::flat_hash_map<std::string,const CoreType *> types;
    std::vector<lyo1::Placeholder> placeholders;
    std::vector<lyo1::Constraint> constraints;
    std::vector<std::string> names;
};

struct CorePlaceholder {
    std::string name;
    lyo1::PlaceholderVariance variance;
};

struct CoreConstraint {
    std::string name;
    lyo1::ConstraintBound bound;
    const CoreType *type;
};

struct CoreAction {
    tu_uint32 action_index;
    lyric_common::SymbolPath actionPath;
    const CoreTemplate *actionTemplate;
    const CoreType *returnType;
    lyo1::TypeSection receiverSection;
    tu_uint32 receiverDescriptor;
    lyo1::ActionFlags flags;
    std::vector<lyo1::Parameter> parameters;
    Option<lyo1::Parameter> rest;
    std::vector<std::string> names;
};

struct CoreConcept {
    tu_uint32 concept_index;
    lyric_common::SymbolPath conceptPath;
    const CoreType *conceptType;
    const CoreTemplate *conceptTemplate;
    const CoreConcept *superConcept;
    lyo1::ConceptFlags flags;
    std::vector<CoreAction *> actions;
};

struct CoreParam {
    std::string paramName;
    const CoreType *paramType;
    const CoreCall *paramDefault;
    lyo1::ParameterFlags flags;
};

struct CoreField {
    tu_uint32 field_index;
    lyric_common::SymbolPath fieldPath;
    const CoreType *fieldType;
    lyo1::FieldFlags flags;
};

struct CoreCall {
    tu_uint32 call_index;
    lyric_common::SymbolPath callPath;
    const CoreType *callType;
    const CoreTemplate *callTemplate;
    const CoreType *returnType;
    lyo1::TypeSection receiverSection;
    tu_uint32 receiverDescriptor;
    lyo1::CallFlags flags;
    std::vector<lyo1::Parameter> parameters;
    Option<lyo1::Parameter> rest;
    std::vector<std::string> names;
    lyric_object::BytecodeBuilder code;
};

struct CoreImpl {
    tu_uint32 impl_index;
    const CoreType *implType;
    const CoreConcept *implConcept;
    lyric_common::SymbolPath receiverPath;
    lyo1::TypeSection receiverSection;
    tu_uint32 receiverDescriptor;
    std::vector<lyo1::ImplExtension> extensions;
};

struct CoreStatic {
    tu_uint32 static_index;
    lyric_common::SymbolPath staticPath;
    const CoreType *staticType;
    lyo1::StaticFlags flags;
};

struct CoreExistential {
    tu_uint32 existential_index;
    lyric_common::SymbolPath existentialPath;
    const CoreType *existentialType;
    const CoreTemplate *existentialTemplate;
    const CoreExistential *superExistential;
    lyo1::IntrinsicType intrinsicMapping;
    lyo1::ExistentialFlags flags;
};

struct CoreClass {
    tu_uint32 class_index;
    lyric_common::SymbolPath classPath;
    const CoreType *classType;
    const CoreTemplate *classTemplate;
    const CoreClass *superClass;
    lyo1::ClassFlags flags;
    tu_uint32 allocatorTrap;
    const CoreCall *classCtor;
    std::vector<CoreField *> members;
    std::vector<CoreCall *> methods;
};

struct CoreEnum {
    tu_uint32 enum_index;
    lyric_common::SymbolPath enumPath;
    const CoreType *enumType;
    const CoreEnum *superEnum;
    lyo1::EnumFlags flags;
    tu_uint32 allocatorTrap;
    const CoreCall *enumCtor;
    std::vector<CoreField *> members;
    std::vector<CoreCall *> methods;
};

struct CoreInstance {
    tu_uint32 instance_index;
    lyric_common::SymbolPath instancePath;
    const CoreType *instanceType;
    const CoreInstance *superInstance;
    lyo1::InstanceFlags flags;
    tu_uint32 allocatorTrap;
    const CoreCall *instanceCtor;
    std::vector<CoreField *> members;
    std::vector<CoreCall *> methods;
    std::vector<CoreImpl *> impls;
};

struct CoreStruct {
    tu_uint32 struct_index;
    lyric_common::SymbolPath structPath;
    const CoreType *structType;
    const CoreStruct *superStruct;
    lyo1::StructFlags flags;
    tu_uint32 allocatorTrap;
    const CoreCall *structCtor;
    std::vector<CoreField *> members;
    std::vector<CoreCall *> methods;
    std::vector<tu_uint32> sealedSubtypes;
};

struct CoreSymbol {
    lyric_common::SymbolPath symbolPath;
    lyo1::DescriptorSection section;
    tu_uint32 index;
};

struct BuilderState {

    std::vector<CoreType *> types;
    std::vector<CoreTemplate *> templates;
    std::vector<CoreExistential *> existentials;
    std::vector<CoreStatic *> statics;
    std::vector<CoreField *> fields;
    std::vector<CoreCall *> calls;
    std::vector<CoreAction *> actions;
    std::vector<CoreConcept *> concepts;
    std::vector<CoreImpl *> impls;
    std::vector<CoreClass *> classes;
    std::vector<CoreStruct *> structs;
    std::vector<CoreEnum *> enums;
    std::vector<CoreInstance *> instances;

    absl::flat_hash_map<lyric_common::SymbolPath,CoreSymbol *> symbols;

    absl::flat_hash_map<lyric_common::SymbolPath,CoreExistential *> existentialcache;
    absl::flat_hash_map<lyric_common::SymbolPath,CoreConcept *> conceptcache;
    absl::flat_hash_map<lyric_common::SymbolPath,CoreClass *> classcache;
    absl::flat_hash_map<lyric_common::SymbolPath,CoreStruct *> structcache;
    absl::flat_hash_map<lyric_common::SymbolPath,CoreEnum *> enumcache;
    absl::flat_hash_map<lyric_common::SymbolPath,CoreInstance *> instancecache;
    absl::flat_hash_map<int, lyric_common::SymbolPath> functionclasspaths;

    const absl::flat_hash_map<std::string,std::string> plugins;

    explicit BuilderState(const absl::flat_hash_map<std::string,std::string> &plugins);

    /*
     * type definitions
     */

    CoreType *addConcreteType(
        const CoreType *superType,
        lyo1::TypeSection concreteSection,
        tu_uint32 concreteDescriptor,
        const std::vector<const CoreType *> &parameters = {});

    CoreType *addUnionType(const std::vector<const CoreType *> &unionMembers);

    CoreTemplate *addTemplate(
        const lyric_common::SymbolPath &templatePath,
        const std::vector<CorePlaceholder> &placeholders,
        const std::vector<CoreConstraint> &constraints = {});
    /*
     *
     */
    CoreExistential *addExistential(
        const lyric_common::SymbolPath &existentialPath,
        lyo1::IntrinsicType intrinsicMapping,
        lyo1::ExistentialFlags existentialFlags,
        const CoreExistential *superExistential = nullptr);
    CoreExistential *addGenericExistential(
        const lyric_common::SymbolPath &existentialPath,
        const CoreTemplate *existentialTemplate,
        lyo1::IntrinsicType intrinsicMapping,
        lyo1::ExistentialFlags existentialFlags,
        const CoreExistential *superExistential = nullptr);

    /*
     * function definitions
     */
    CoreCall *addFunction(
        const lyric_common::SymbolPath &functionPath,
        const std::vector<CoreParam> &parameters,
        const lyric_object::BytecodeBuilder &code,
        const CoreType *returnType,
        bool isInline = false);
    CoreCall *addGenericFunction(
        const lyric_common::SymbolPath &functionPath,
        const CoreTemplate *functionTemplate,
        const std::vector<CoreParam> &parameters,
        const lyric_object::BytecodeBuilder &code,
        const CoreType *returnType,
        bool isInline = false);

    /*
     * concept definitions
     */

    CoreConcept *addConcept(
        const lyric_common::SymbolPath &conceptPath,
        lyo1::ConceptFlags conceptFlags,
        const CoreConcept *superConcept = nullptr);
    CoreConcept *addGenericConcept(
        const lyric_common::SymbolPath &conceptPath,
        const CoreTemplate *conceptTemplate,
        lyo1::ConceptFlags conceptFlags,
        const CoreConcept *superConcept = nullptr);
    CoreAction *addConceptAction(
        const std::string &actionName,
        const CoreConcept *receiver,
        const std::vector<std::pair<std::string, const CoreType *>> &parameters,
        const Option<std::pair<std::string, const CoreType *>> &rest,
        const CoreType *returnType);

    /*
     * class definitions
     */

    CoreClass *addClass(
        const lyric_common::SymbolPath &classPath,
        lyo1::ClassFlags classFlags,
        const CoreClass *superClass = nullptr);
    CoreClass *addGenericClass(
        const lyric_common::SymbolPath &classPath,
        const CoreTemplate *classTemplate,
        lyo1::ClassFlags classFlags,
        const CoreClass *superClass = nullptr);
    void setClassAllocator(const CoreClass *receiver, lyric_bootstrap::internal::BootstrapTrap allocatorTrap);
    CoreCall *addClassCtor(
        const CoreClass *receiver,
        const std::vector<CoreParam> &parameters,
        const lyric_object::BytecodeBuilder &code);
    CoreField *addClassMember(
        const std::string &memberName,
        const CoreClass *receiver,
        lyo1::FieldFlags fieldFlags,
        const CoreType *memberType);
    CoreCall *addClassMethod(
        const std::string &methodName,
        const CoreClass *receiver,
        lyo1::CallFlags callFlags,
        const std::vector<std::pair<std::string, const CoreType *>> &parameters,
        const Option<std::pair<std::string, const CoreType *>> &rest,
        const lyric_object::BytecodeBuilder &code,
        const CoreType *returnType,
        bool isInline = false);

    /*
     * struct definitions
     */

    CoreStruct *addStruct(
        const lyric_common::SymbolPath &classPath,
        lyo1::StructFlags structFlags,
        const CoreStruct *superStruct = nullptr);
    void setStructAllocator(const CoreStruct *receiver, lyric_bootstrap::internal::BootstrapTrap allocatorTrap);
    CoreCall *addStructCtor(
        const CoreStruct *receiver,
        const std::vector<CoreParam> &parameters,
        const lyric_object::BytecodeBuilder &code);
    CoreField *addStructMember(
        const std::string &memberName,
        const CoreStruct *receiver,
        lyo1::FieldFlags fieldFlags,
        const CoreType *memberType);
    CoreCall *addStructMethod(
        const std::string &methodName,
        const CoreStruct *receiver,
        lyo1::CallFlags callFlags,
        const std::vector<CoreParam> &parameters,
        const lyric_object::BytecodeBuilder &code,
        const CoreType *returnType,
        bool isInline = false);
    void addStructSealedSubtype(const CoreStruct *receiver, const CoreStruct *subtypeStruct);

    /*
     * instance definitions
     */

    CoreInstance *addInstance(
        const lyric_common::SymbolPath &instancePath,
        lyo1::InstanceFlags instanceFlags,
        const CoreInstance *superInstance = nullptr);
    void setInstanceAllocator(const CoreInstance *receiver, lyric_bootstrap::internal::BootstrapTrap allocatorTrap);
    CoreCall *addInstanceCtor(
        const CoreInstance *receiver,
        const std::vector<CoreParam> &parameters,
        const lyric_object::BytecodeBuilder &code);
    CoreField *addInstanceMember(
        const std::string &memberName,
        const CoreInstance *receiver,
        lyo1::FieldFlags fieldFlags,
        const CoreType *memberType);
    CoreCall *addInstanceMethod(
        const std::string &methodName,
        const CoreInstance *receiver,
        lyo1::CallFlags callFlags,
        const std::vector<std::pair<std::string, const CoreType *>> &parameters,
        const Option<std::pair<std::string, const CoreType *>> &rest,
        const lyric_object::BytecodeBuilder &code,
        const CoreType *returnType,
        bool isInline = false);

    /*
     * impl definitions
     */
    CoreImpl *addImpl(
        const lyric_common::SymbolPath &receiverPath,
        const CoreType *implType,
        const CoreConcept *implConcept);
    CoreCall *addImplExtension(
        const std::string &extensionName,
        const CoreImpl *receiver,
        const std::vector<std::pair<std::string, const CoreType *>> &parameters,
        const Option<std::pair<std::string, const CoreType *>> &rest,
        const lyric_object::BytecodeBuilder &code,
        const CoreType *returnType,
        bool isInline = false);

    /*
     * enum definitions
     */

    CoreEnum *addEnum(
        const lyric_common::SymbolPath &enumPath,
        lyo1::EnumFlags enumFlags,
        const CoreEnum *superEnum = nullptr);
    void setEnumAllocator(const CoreEnum *receiver, lyric_bootstrap::internal::BootstrapTrap allocatorTrap);
    CoreCall *addEnumCtor(
        const CoreEnum *receiver,
        const std::vector<CoreParam> &parameters,
        const lyric_object::BytecodeBuilder &code);
    CoreField *addEnumMember(
        const std::string &memberName,
        const CoreEnum *receiver,
        lyo1::FieldFlags fieldFlags,
        const CoreType *memberType);
    CoreCall *addEnumMethod(
        const std::string &methodName,
        const CoreEnum *receiver,
        lyo1::CallFlags callFlags,
        const std::vector<std::pair<std::string, const CoreType *>> &parameters,
        const Option<std::pair<std::string, const CoreType *>> &rest,
        const lyric_object::BytecodeBuilder &code,
        const CoreType *returnType,
        bool isInline = false);

    // serialize the state
    std::shared_ptr<const std::string> toBytes() const;
};

#endif // ZURI_CORE_BUILDER_STATE_H