
#include <lyric_runtime/abstract_plugin.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

/**
 * Default no-args constructor which creates an invalid InterpreterState instance. This is only useful
 * for mocking the class when testing.
 */
lyric_runtime::InterpreterState::InterpreterState()
    : m_loop(nullptr),
      m_segmentManager(nullptr),
      m_typeManager(nullptr),
      m_subroutineManager(nullptr),
      m_systemScheduler(nullptr),
      m_portMultiplexer(nullptr),
      m_heapManager(nullptr)
{
}

lyric_runtime::InterpreterState::InterpreterState(
    uv_loop_t *loop,
    std::shared_ptr<AbstractHeap> heap,
    SegmentManager *segmentManager,
    TypeManager *typeManager,
    SubroutineManager *subroutineManager,
    SystemScheduler *systemScheduler,
    PortMultiplexer *portMultiplexer,
    HeapManager *heapManager)
    : m_loop(loop),
      m_heap(std::move(heap)),
      m_segmentManager(segmentManager),
      m_typeManager(typeManager),
      m_subroutineManager(subroutineManager),
      m_systemScheduler(systemScheduler),
      m_portMultiplexer(portMultiplexer),
      m_heapManager(heapManager)
{
    TU_ASSERT (m_loop != nullptr);
    TU_ASSERT (m_heap != nullptr);
    TU_ASSERT (m_segmentManager != nullptr);
    TU_ASSERT (m_typeManager != nullptr);
    TU_ASSERT (m_subroutineManager != nullptr);
    TU_ASSERT (m_systemScheduler != nullptr);
    TU_ASSERT (m_portMultiplexer != nullptr);
    TU_ASSERT (m_heapManager != nullptr);
}

lyric_runtime::InterpreterState::~InterpreterState()
{
    delete m_heapManager;
    delete m_portMultiplexer;
    delete m_systemScheduler;
    delete m_subroutineManager;
    delete m_typeManager;
    delete m_segmentManager;

    if (m_loop != nullptr) {
        uv_loop_close(m_loop);
        free(m_loop);
    }
}

static tempo_utils::Status
detect_bootstrap(
    lyric_runtime::SegmentManager *segmentManager,
    const lyric_common::AssemblyLocation &mainLocation,
    lyric_common::AssemblyLocation *bootstrapLocation)
{
    TU_ASSERT (segmentManager != nullptr);
    TU_ASSERT (mainLocation.isValid());
    TU_ASSERT (bootstrapLocation != nullptr);

    // load segment for the main assembly from the specified location
    auto *segment = segmentManager->loadAssembly(mainLocation);
    if (segment == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kMissingAssembly,
             "missing assembly {}", mainLocation.toString());

    auto mainObject = segment->getObject().getObject();

    // find the system bootstrap assembly
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

    *bootstrapLocation = bootstrapImport.getImportLocation();
    if (!bootstrapLocation->isValid())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kMissingAssembly, bootstrapLocation->toString());

    TU_LOG_V << "found system bootstrap " << *bootstrapLocation;

    return lyric_runtime::InterpreterStatus::ok();
}

static tempo_utils::Status
allocate_type_manager(
    lyric_runtime::SegmentManager *segmentManager,
    const lyric_common::AssemblyLocation &preludeLocation,
    lyric_runtime::TypeManager **typeManagerPtr)
{
    TU_ASSERT (segmentManager != nullptr);
    TU_ASSERT (typeManagerPtr != nullptr);

    // load the prelude assembly
    // FIXME: import hash should be required for prelude assembly
    auto *bootstrapSegment = segmentManager->loadAssembly(preludeLocation);
    if (bootstrapSegment == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "failed to load bootstrap assembly {}", preludeLocation.toString());

    auto bootstrapObject = bootstrapSegment->getObject().getObject();
    if (!bootstrapObject.isValid())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "bootstrap assembly {} is invalid", preludeLocation.toString());

    TU_LOG_V << "loaded bootstrap assembly " << preludeLocation;

    std::vector<lyric_runtime::DataCell> intrinsiccache;
    intrinsiccache.resize(static_cast<int>(lyric_object::IntrinsicType::NUM_INTRINSIC_TYPES));

    // perform the bootstrapping process
    for (int i = 0; i < bootstrapObject.numExistentials(); i++) {
        auto existential = bootstrapObject.getExistential(i);
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

        intrinsiccache[index] = lyric_runtime::DataCell::forType(
            bootstrapSegment->getSegmentIndex(), type.getDescriptorOffset());
    }

    *typeManagerPtr = new lyric_runtime::TypeManager(
        std::move(intrinsiccache), segmentManager);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Result<std::shared_ptr<lyric_runtime::InterpreterState>>
lyric_runtime::InterpreterState::create(
    const InterpreterStateOptions &options,
    const lyric_common::AssemblyLocation &mainLocation)
{
    lyric_common::AssemblyLocation preludeLocation;
    uv_loop_t *loop = nullptr;
    SegmentManager *segmentManager = nullptr;
    TypeManager *typeManager = nullptr;
    SubroutineManager *subroutineManager = nullptr;
    SystemScheduler *systemScheduler = nullptr;
    PortMultiplexer *portMultiplexer = nullptr;
    HeapManager *heapManager = nullptr;
    InterpreterState *stateptr = nullptr;
    std::shared_ptr<InterpreterState> state;

    tempo_utils::Status status;

    if (options.loader == nullptr)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "loader must be specified");

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
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "failed to initialize main loop");
        goto err;
    }

    // allocate a new segment manager
    segmentManager = new SegmentManager(options.loader);

    // the prelude must either be explicitly specified or inferred from the main location
    if (!options.preludeLocation.isValid()) {
        if (mainLocation.isValid()) {
            status = detect_bootstrap(segmentManager, mainLocation, &preludeLocation);
        } else {
            status = InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "either main location or prelude location must be specified");
        }
        if (status.notOk()) {
            goto err;
        }
    } else {
        preludeLocation = options.preludeLocation;
    }

    // allocate the type manager using intrinsics from the specified prelude
    status = allocate_type_manager(segmentManager, preludeLocation, &typeManager);
    if (status.notOk()) {
        goto err;
    }

    // allocate the remaining subsystems
    subroutineManager = new SubroutineManager(segmentManager);
    systemScheduler = new SystemScheduler(loop);
    portMultiplexer = new PortMultiplexer();
    heapManager = new HeapManager(segmentManager, systemScheduler, heap);

    // allocate the interpreter state
    stateptr = new InterpreterState(loop, std::move(heap), segmentManager, typeManager,
        subroutineManager, systemScheduler, portMultiplexer, heapManager);
    state = std::shared_ptr<InterpreterState>(stateptr);

    // if main location was specified then load it
    if (mainLocation.isValid()) {
        TU_RETURN_IF_NOT_OK(state->reload(mainLocation));
    }
    return state;

err:
    delete heapManager;
    delete portMultiplexer;
    delete systemScheduler;
    delete subroutineManager;
    delete segmentManager;
    uv_loop_close(loop);
    delete loop;
    return status;
}

lyric_runtime::StackfulCoroutine *
lyric_runtime::InterpreterState::currentCoro() const
{
    return m_systemScheduler->currentCoro();
}

lyric_runtime::SegmentManager *
lyric_runtime::InterpreterState::segmentManager() const
{
    return m_segmentManager;
}

lyric_runtime::TypeManager *
lyric_runtime::InterpreterState::typeManager() const
{
    return m_typeManager;
}

lyric_runtime::SubroutineManager *
lyric_runtime::InterpreterState::subroutineManager() const
{
    return m_subroutineManager;
}

lyric_runtime::SystemScheduler *
lyric_runtime::InterpreterState::systemScheduler() const
{
    return m_systemScheduler;
}

lyric_runtime::PortMultiplexer *
lyric_runtime::InterpreterState::portMultiplexer() const
{
    return m_portMultiplexer;
}

lyric_runtime::HeapManager *
lyric_runtime::InterpreterState::heapManager() const
{
    return m_heapManager;
}

uv_loop_t *
lyric_runtime::InterpreterState::mainLoop() const
{
    return m_loop;
}

tempo_utils::Status
lyric_runtime::InterpreterState::reload(const lyric_common::AssemblyLocation &mainLocation)
{
    // load segment for the main assembly from the specified main location
    auto *segment = m_segmentManager->loadAssembly(mainLocation);
    if (segment == nullptr)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kMissingAssembly,
            "missing assembly {}", mainLocation.toString());

    auto mainObject = segment->getObject().getObject();

    // set *IP to the entry proc of the main assembly
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

    //auto procOffset = call->bytecode_offset();
    auto procOffset = call.getProcOffset();

    // proc offset must be within the segment bytecode
    auto *bytecodeData = segment->getBytecodeData();
    auto bytecodeSize = segment->getBytecodeSize();
    if (bytecodeSize <= procOffset)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid proc offset");

    const uint8_t *header = bytecodeData + procOffset;
    auto stackGuard = 0;

    // read the call proc header
    tempo_utils::read_u32_and_advance(header);  // procSize
    auto numArguments = tempo_utils::read_u16_and_advance(header);
    auto numLocals = tempo_utils::read_u16_and_advance(header);
    auto numLexicals = tempo_utils::read_u16_and_advance(header);

    // the entry symbol must not accept any arguments
    if (numArguments != 0)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid proc header");

    // construct the entry activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(), procOffset, segment->getSegmentIndex(),
        {}, stackGuard, 0, 0, numLocals, numLexicals, {});

    //lyric_object::BytecodeIterator ip(header, codeSize);
    auto ip = call.getBytecodeIterator();

    // reset the main task and initialize it with the new entry call
    auto *mainTask = m_systemScheduler->mainTask();
    auto *coro = mainTask->stackfulCoroutine();
    coro->reset();
    coro->pushCall(frame, ip, segment);
    TU_LOG_V << "initialized ip to " << ip;

    // move main task to the ready queue
    m_systemScheduler->resumeTask(mainTask);
    m_systemScheduler->selectNextReady();

    return InterpreterStatus::ok();
}

lyric_runtime::RefHandle
lyric_runtime::InterpreterState::createHandle(const DataCell &ref)
{
    if (ref.type != DataCellType::REF)
        return {};
    void *handle = m_heap->createHandle(ref.data.ref);
    return RefHandle(shared_from_this(), handle);
}
