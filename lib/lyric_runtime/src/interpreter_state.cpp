
#include <lyric_object/symbol_walker.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

#include "lyric_runtime/string_ref.h"

/**
 * Default no-args constructor which creates an invalid InterpreterState instance. This is only useful
 * for mocking the class when testing.
 */
lyric_runtime::InterpreterState::InterpreterState()
    : m_loop(nullptr),
      m_loadEpochMillis(0),
      m_statusCode(tempo_utils::StatusCode::kUnknown),
      m_active(false)
{
}

lyric_runtime::InterpreterState::InterpreterState(
    uv_loop_t *loop,
    const lyric_common::ModuleLocation &preludeLocation,
    std::shared_ptr<AbstractLoader> systemLoader,
    std::shared_ptr<AbstractLoader> applicationLoader,
    std::shared_ptr<AbstractHeap> heap)
    : m_loop(loop),
      m_preludeLocation(preludeLocation),
      m_systemLoader(std::move(systemLoader)),
      m_applicationLoader(std::move(applicationLoader)),
      m_heap(std::move(heap)),
      m_loadEpochMillis(0),
      m_statusCode(tempo_utils::StatusCode::kUnknown),
      m_active(false)
{
    TU_ASSERT (m_loop != nullptr);
    TU_ASSERT (m_systemLoader != nullptr);
    TU_ASSERT (m_applicationLoader != nullptr);
    TU_ASSERT (m_heap != nullptr);
}

lyric_runtime::InterpreterState::~InterpreterState()
{
    m_heapManager.reset();
    m_portMultiplexer.reset();
    m_systemScheduler.reset();
    m_subroutineManager.reset();
    m_typeManager.reset();
    m_segmentManager.reset();

    if (m_loop != nullptr) {
        uv_loop_close(m_loop);
        free(m_loop);
    }
}

tempo_utils::Result<std::shared_ptr<lyric_runtime::InterpreterState>>
lyric_runtime::InterpreterState::create(
    std::shared_ptr<AbstractLoader> systemLoader,
    std::shared_ptr<AbstractLoader> applicationLoader,
    const InterpreterStateOptions &options)
{
    TU_ASSERT (systemLoader != nullptr);
    TU_ASSERT (applicationLoader != nullptr);

    lyric_common::ModuleLocation preludeLocation;
    uv_loop_t *loop = nullptr;

    // if heap is not specified in options then allocate one
    std::shared_ptr<AbstractHeap> heap;
    if (options.heap == nullptr) {
        heap = GCHeap::create();
    } else {
        heap = options.heap;
    }

    // allocate a new UV main loop
    loop = new uv_loop_t;

    // initialize the main loop here, but note that SystemScheduler takes ownership of the loop
    // once it is constructed. in particular, the SystemScheduler owns the loop.data pointer.
    auto ret = uv_loop_init(loop);
    if (ret != 0) {
        uv_loop_close(loop);
        delete loop;
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "failed to initialize main loop");
    }

    /*
     * WARNING: loop is not a managed pointer so we can't fail or exit early until we have transferred
     * ownership to the interpreter state!
     */

    // allocate the interpreter state
    auto state = std::shared_ptr<InterpreterState>(new InterpreterState(loop, options.preludeLocation,
        std::move(systemLoader), std::move(applicationLoader), std::move(heap)));

    // capture pointer to interpreter state in the loop data field
    loop->data = state.get();

    // if main location was specified then load it
    if (options.mainLocation.isValid()) {
        TU_RETURN_IF_NOT_OK(state->load(options.mainLocation, options.mainArguments));
    }

    return state;
}

static tempo_utils::Status
detect_bootstrap(
    lyric_runtime::SegmentManager *segmentManager,
    const lyric_common::ModuleLocation &mainLocation,
    lyric_common::ModuleLocation &bootstrapLocation)
{
    TU_ASSERT (segmentManager != nullptr);
    TU_ASSERT (mainLocation.isValid());

    // load segment for the main module from the specified location
    auto *segment = segmentManager->getOrLoadSegment(mainLocation, /* useSystemLoader= */ false);
    if (segment == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kMissingObject,
             "missing object {}", mainLocation.toString());

    auto mainObject = segment->getObject();

    // find the system bootstrap module
    lyric_object::ImportWalker bootstrapImport;;
    for (int i = 0; i < mainObject.numImports(); i++) {
        auto currentImport = mainObject.getImport(i);
        if (!currentImport.isSystemBootstrap())
            continue;
        if (bootstrapImport.isValid())
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "duplicate system bootstrap detected");
        bootstrapImport = currentImport;
    }
    if (!bootstrapImport.isValid())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "missing system bootstrap import");

    auto importLocation = bootstrapImport.getImportLocation();
    if (!importLocation.isValid())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kMissingObject, bootstrapLocation.toString());
    TU_LOG_V << "found system bootstrap " << importLocation;

    bootstrapLocation = importLocation;

    return {};
}

static tempo_utils::Status
allocate_type_manager(
    lyric_runtime::SegmentManager *segmentManager,
    lyric_runtime::BytecodeSegment *preludeSegment,
    const lyric_object::LyricObject &preludeObject,
    std::unique_ptr<lyric_runtime::TypeManager> &typeManagerPtr)
{
    TU_ASSERT (segmentManager != nullptr);
    TU_ASSERT (preludeSegment != nullptr);

    std::vector<lyric_runtime::DataCell> intrinsiccache;
    intrinsiccache.resize(static_cast<int>(lyric_object::IntrinsicType::NUM_INTRINSIC_TYPES));

    // perform the bootstrapping process
    for (int i = 0; i < preludeObject.numExistentials(); i++) {
        auto existential = preludeObject.getExistential(i);
        if (existential.getIntrinsicType() == lyric_object::IntrinsicType::Invalid)
            continue;
        auto index = static_cast<int>(existential.getIntrinsicType());
        if (intrinsiccache[index].isValid())
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant,
                "duplicate intrinsic mapping detected");

        auto type = existential.getExistentialType();
        if (!type.isValid())
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant,
                "invalid intrinsic mapping detected");

        auto *typeEntry = preludeSegment->lookupType(type.getDescriptorOffset());
        intrinsiccache[index] = lyric_runtime::DataCell::forType(typeEntry);
    }

    typeManagerPtr = std::make_unique<lyric_runtime::TypeManager>(
        std::move(intrinsiccache), segmentManager);

    return {};
}

static const lyric_runtime::ExistentialTable *
resolve_bootstrap_existential_table(
    lyric_runtime::SegmentManager *segmentManager,
    lyric_runtime::BytecodeSegment *preludeSegment,
    const lyric_object::LyricObject &preludeObject,
    const lyric_common::SymbolPath &symbolPath,
    tempo_utils::Status &status)
{
    auto symbol = preludeObject.findSymbol(symbolPath);
    auto descriptor = segmentManager->resolveDescriptor(preludeSegment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    return segmentManager->resolveExistentialTable(descriptor, status);
}

static const lyric_runtime::VirtualTable *
resolve_bootstrap_virtual_table(
    lyric_runtime::SegmentManager *segmentManager,
    lyric_runtime::BytecodeSegment *preludeSegment,
    const lyric_object::LyricObject &preludeObject,
    lyric_common::SymbolPath symbolPath,
    tempo_utils::Status &status)
{
    auto symbol = preludeObject.findSymbol(symbolPath);
    auto descriptor = segmentManager->resolveDescriptor(preludeSegment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    switch (descriptor.type) {
        case lyric_runtime::DataCellType::CLASS:
            return segmentManager->resolveClassVirtualTable(descriptor, status);
        case lyric_runtime::DataCellType::ENUM:
            return segmentManager->resolveEnumVirtualTable(descriptor, status);
        case lyric_runtime::DataCellType::INSTANCE:
            return segmentManager->resolveInstanceVirtualTable(descriptor, status);
        case lyric_runtime::DataCellType::STRUCT:
            return segmentManager->resolveStructVirtualTable(descriptor, status);
        default:
            return nullptr;
    }
}

static tempo_utils::Status
allocate_heap_manager(
    lyric_runtime::SegmentManager *segmentManager,
    lyric_runtime::SystemScheduler *systemScheduler,
    lyric_runtime::BytecodeSegment *preludeSegment,
    const lyric_object::LyricObject &preludeObject,
    std::shared_ptr<lyric_runtime::AbstractHeap> heap,
    std::unique_ptr<lyric_runtime::HeapManager> &heapManagerPtr)
{
    TU_ASSERT (segmentManager != nullptr);
    TU_ASSERT (systemScheduler != nullptr);
    TU_ASSERT (preludeSegment != nullptr);
    TU_ASSERT (heap != nullptr);

    lyric_runtime::PreludeTables preludeTables;
    tempo_utils::Status status;

    // resolve existential tables
    preludeTables.BytesTable = resolve_bootstrap_existential_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("Bytes"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.RestTable = resolve_bootstrap_existential_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("Rest"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.StringTable = resolve_bootstrap_existential_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("String"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.UrlTable = resolve_bootstrap_existential_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("Url"), status);
    TU_RETURN_IF_NOT_OK (status);

    // resolve virtual tables
    preludeTables.CancelledTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("Cancelled"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.InvalidArgumentTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("InvalidArgument"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.DeadlineExceededTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("DeadlineExceeded"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.NotFoundTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("NotFound"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.AlreadyExistsTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("AlreadyExists"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.PermissionDeniedTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("PermissionDenied"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.UnauthenticatedTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("Unauthenticated"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.ResourceExhaustedTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("ResourceExhausted"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.FailedPreconditionTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("FailedPrecondition"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.AbortedTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("Aborted"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.UnavailableTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("Unavailable"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.OutOfRangeTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("OutOfRange"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.UnimplementedTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("Unimplemented"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.InternalTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("Internal"), status);
    TU_RETURN_IF_NOT_OK (status);
    preludeTables.UnknownTable = resolve_bootstrap_virtual_table(segmentManager,
        preludeSegment, preludeObject, lyric_common::SymbolPath::fromString("Unknown"), status);
    TU_RETURN_IF_NOT_OK (status);

    heapManagerPtr = std::make_unique<lyric_runtime::HeapManager>(
        preludeTables, segmentManager, systemScheduler, std::move(heap));

    return {};
}

lyric_common::ModuleLocation
lyric_runtime::InterpreterState::getMainLocation() const
{
    return m_mainLocation;
}

lyric_runtime::DataCell
lyric_runtime::InterpreterState::getMainArgument(int index) const
{
    if (0 <= index && index < m_mainArguments.size())
        return m_mainArguments.at(index);
    return {};
}

int
lyric_runtime::InterpreterState::numMainArguments() const
{
    return m_mainArguments.size();
}

tu_uint64
lyric_runtime::InterpreterState::getLoadEpochMillis() const
{
    return m_loadEpochMillis;
}

tempo_utils::StatusCode
lyric_runtime::InterpreterState::getStatusCode() const
{
    return m_statusCode;
}

bool
lyric_runtime::InterpreterState::isActive() const
{
    return m_active;
}

lyric_runtime::StackfulCoroutine *
lyric_runtime::InterpreterState::currentCoro() const
{
    return m_systemScheduler->currentCoro();
}

lyric_runtime::SegmentManager *
lyric_runtime::InterpreterState::segmentManager() const
{
    return m_segmentManager.get();
}

lyric_runtime::TypeManager *
lyric_runtime::InterpreterState::typeManager() const
{
    return m_typeManager.get();
}

lyric_runtime::SubroutineManager *
lyric_runtime::InterpreterState::subroutineManager() const
{
    return m_subroutineManager.get();
}

lyric_runtime::SystemScheduler *
lyric_runtime::InterpreterState::systemScheduler() const
{
    return m_systemScheduler.get();
}

lyric_runtime::PortMultiplexer *
lyric_runtime::InterpreterState::portMultiplexer() const
{
    return m_portMultiplexer.get();
}

lyric_runtime::HeapManager *
lyric_runtime::InterpreterState::heapManager() const
{
    return m_heapManager.get();
}

uv_loop_t *
lyric_runtime::InterpreterState::mainLoop() const
{
    return m_loop;
}

tempo_utils::Status
lyric_runtime::InterpreterState::initialize(const lyric_common::ModuleLocation &mainLocation)
{
    TU_ASSERT (m_segmentManager == nullptr);
    TU_ASSERT (mainLocation.isValid());

    if (m_systemLoader == nullptr)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid system loader");
    if (m_applicationLoader == nullptr)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid application loader");

    // allocate a new segment manager
    auto segmentManager = std::make_unique<SegmentManager>(m_systemLoader, m_applicationLoader);
    TU_RETURN_IF_NOT_OK (segmentManager->setOrigin(mainLocation));

    // the prelude must either be explicitly specified or inferred from the main location
    lyric_common::ModuleLocation preludeLocation;
    if (!m_preludeLocation.isValid()) {
        if (!mainLocation.isValid())
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "either main location or prelude location must be specified");
        TU_RETURN_IF_NOT_OK (detect_bootstrap(
            segmentManager.get(), mainLocation, preludeLocation));
    } else {
        preludeLocation = m_preludeLocation;
    }

    // get the prelude segment
    auto *preludeSegment = segmentManager->getOrLoadSegment(preludeLocation, /* useSystemLoader= */ true);
    if (preludeSegment == nullptr)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "failed to load prelude {}", preludeLocation.toString());

    auto preludeObject = preludeSegment->getObject();
    if (!preludeObject.isValid())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "prelude {} is invalid", preludeLocation.toString());

    TU_LOG_V << "loaded prelude " << preludeLocation;

    // allocate the type manager using intrinsics from the specified prelude
    std::unique_ptr<TypeManager> typeManager;
    TU_RETURN_IF_NOT_OK (allocate_type_manager(
        segmentManager.get(), preludeSegment, preludeObject, typeManager));

    auto portMultiplexer = std::make_unique<PortMultiplexer>();
    auto subroutineManager = std::make_unique<SubroutineManager>(segmentManager.get());
    auto systemScheduler = std::make_unique<SystemScheduler>(m_loop);

    // allocate the remaining subsystems
    std::unique_ptr<HeapManager> heapManager;
    TU_RETURN_IF_NOT_OK (allocate_heap_manager(segmentManager.get(), systemScheduler.get(),
        preludeSegment, preludeObject, m_heap, heapManager));

    // transfer ownership to this
    m_segmentManager = std::move(segmentManager);
    m_preludeLocation = std::move(preludeLocation);
    m_typeManager = std::move(typeManager);
    m_portMultiplexer = std::move(portMultiplexer);
    m_subroutineManager = std::move(subroutineManager);
    m_systemScheduler = std::move(systemScheduler);
    m_heapManager = std::move(heapManager);

    return {};
}

tempo_utils::Status
lyric_runtime::InterpreterState::load(
    const lyric_common::ModuleLocation &mainLocation,
    const std::vector<std::string> &mainArguments)
{
    if (!mainLocation.isValid())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid main location");
    if (!mainLocation.isAbsolute())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot initialize interpreter; main location {} must be absolute location",
            mainLocation.toString());

    // if state is not yet initialized then initialize it
    if (m_segmentManager == nullptr) {
        TU_RETURN_IF_NOT_OK (initialize(mainLocation));
    }

    // load segment for the main module from the specified main location
    auto *segment = m_segmentManager->getOrLoadSegment(mainLocation, /* useSystemLoader= */ false);
    if (segment == nullptr)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kMissingObject,
            "missing object {}", mainLocation.toString());

    // update the origin
    TU_RETURN_IF_NOT_OK (m_segmentManager->setOrigin(mainLocation));

    auto mainObject = segment->getObject();

    // set *IP to the entry proc of the main module
    auto symbol = mainObject.findSymbol(lyric_common::SymbolPath::entrySymbol());
    if (!symbol.isValid())
        return InterpreterStatus::forCondition(
            InterpreterCondition::kMissingSymbol, lyric_common::SymbolPath::entrySymbol().toString());
    if (symbol.getLinkageSection() != lyric_object::LinkageSection::Call)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "entry symbol must be a CALL");
    auto callIndex = symbol.getLinkageIndex();

    auto call = mainObject.getCall(callIndex);
    if (!call.isValid())
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid entry symbol");
    TU_LOG_V << "entry symbol is " << call.getSymbolPath().toString();

    // parse the proc
    auto procOffset = call.getProcOffset();
    auto bytecode = segment->getBytecode();
    lyric_object::ProcInfo procInfo;
    TU_RETURN_IF_NOT_OK (lyric_object::parse_proc_info(bytecode, procOffset, procInfo));

    // the entry symbol must not expect any arguments or lexicals
    if (procInfo.num_arguments != 0 || procInfo.num_lexicals != 0)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid proc header");

    // construct the entry activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(),
        procOffset, segment->getSegmentIndex(), {}, true,
        0, 0, 0, procInfo.num_locals, 0, {});

    lyric_object::BytecodeIterator ip(procInfo.code);

    // reset the main task and initialize it with the new entry call
    auto *mainTask = m_systemScheduler->mainTask();
    auto *coro = mainTask->stackfulCoroutine();
    coro->reset();
    coro->pushCall(frame, ip, segment);
    TU_LOG_V << "initialized ip to " << ip;

    // move main task to the ready queue
    m_systemScheduler->resumeTask(mainTask);
    m_systemScheduler->selectNextReady();

    // store internal copy of main arguments list
    m_mainArguments.resize(mainArguments.size());
    for (int i = 0; i < mainArguments.size(); ++i) {
        auto arg = m_heapManager->allocateString(mainArguments.at(i));
        arg.data.str->setPermanent();
        m_mainArguments[i] = std::move(arg);
    }

    // initialize process information
    m_mainLocation = mainLocation;
    m_loadEpochMillis = ToUnixMillis(absl::Now());
    m_statusCode = tempo_utils::StatusCode::kUnknown;
    m_active = true;

    return {};
}

tempo_utils::Status
lyric_runtime::InterpreterState::halt(tempo_utils::StatusCode statusCode)
{
    if (!m_active)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "cannot halt inactive interpreter");

    m_statusCode = statusCode;
    m_active = false;

    return {};
}

lyric_runtime::RefHandle
lyric_runtime::InterpreterState::createHandle(const DataCell &ref)
{
    if (ref.type != DataCellType::REF)
        return {};
    void *handle = m_heap->createHandle(ref.data.ref);
    return RefHandle(shared_from_this(), handle);
}