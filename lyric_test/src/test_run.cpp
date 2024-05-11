
#include <lyric_test/test_run.h>

lyric_test::TestComputation::TestComputation()
{
}

lyric_test::TestComputation::TestComputation(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics)
    : m_tester(tester),
      m_computation(computation),
      m_diagnostics(diagnostics)
{
    TU_ASSERT (m_tester != nullptr);
}

lyric_test::TestComputation::TestComputation(const TestComputation &other)
    : m_tester(other.m_tester),
      m_computation(other.m_computation),
      m_diagnostics(other.m_diagnostics)
{
}

lyric_test::TestComputation::TestComputation(TestComputation &&other) noexcept
{
    m_tester.swap(other.m_tester);
    m_computation = std::move(other.m_computation);
    m_diagnostics = std::move(other.m_diagnostics);
}

lyric_test::TestComputation&
lyric_test::TestComputation::operator=(const TestComputation &other)
{
    m_tester = other.m_tester;
    m_computation = other.m_computation;
    m_diagnostics = other.m_diagnostics;
    return *this;
}

lyric_test::TestComputation&
lyric_test::TestComputation::operator=(TestComputation &&other) noexcept
{
    if (this != &other) {
        m_tester.swap(other.m_tester);
        m_computation = std::move(other.m_computation);
        m_diagnostics = std::move(other.m_diagnostics);
    }
    return *this;
}

std::shared_ptr<lyric_test::AbstractTester>
lyric_test::TestComputation::getTester() const
{
    return m_tester;
}

bool
lyric_test::TestComputation::hasComputation() const
{
    return m_tester != nullptr;
}

lyric_build::TargetComputation
lyric_test::TestComputation::getComputation() const
{
    return m_computation;
}

std::shared_ptr<lyric_build::BuildDiagnostics>
lyric_test::TestComputation::getDiagnostics() const
{
    return m_diagnostics;
}

lyric_test::SymbolizeModule::SymbolizeModule()
    : TestComputation(),
      m_hasAssembly(false)
{
}

lyric_test::SymbolizeModule::SymbolizeModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
    const lyric_object::LyricObject &assembly)
    : TestComputation(tester, computation, diagnostics),
      m_hasAssembly(true),
      m_assembly(assembly)
{
}

lyric_test::SymbolizeModule::SymbolizeModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics)
    : TestComputation(tester, computation, diagnostics),
      m_hasAssembly(false)
{
}

lyric_test::SymbolizeModule::SymbolizeModule(const SymbolizeModule &other)
    : TestComputation(other),
      m_hasAssembly(other.m_hasAssembly),
      m_assembly(other.m_assembly)
{
}

bool
lyric_test::SymbolizeModule::hasAssembly() const
{
    return m_hasAssembly;
}

lyric_object::LyricObject
lyric_test::SymbolizeModule::getAssembly() const
{
    return m_assembly;
}

lyric_test::AnalyzeModule::AnalyzeModule()
    : TestComputation(),
      m_hasAssembly(false)
{
}

lyric_test::AnalyzeModule::AnalyzeModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
    const lyric_object::LyricObject &assembly)
    : TestComputation(tester, computation, diagnostics),
      m_hasAssembly(true),
      m_assembly(assembly)
{
}

lyric_test::AnalyzeModule::AnalyzeModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics)
    : TestComputation(tester, computation, diagnostics),
      m_hasAssembly(false)
{
}

lyric_test::AnalyzeModule::AnalyzeModule(const AnalyzeModule &other)
    : TestComputation(other),
      m_hasAssembly(other.m_hasAssembly),
      m_assembly(other.m_assembly)
{
}

bool
lyric_test::AnalyzeModule::hasAssembly() const
{
    return m_hasAssembly;
}

lyric_object::LyricObject
lyric_test::AnalyzeModule::getAssembly() const
{
    return m_assembly;
}

lyric_test::CompileModule::CompileModule()
    : TestComputation(),
      m_hasAssembly(false)
{
}

lyric_test::CompileModule::CompileModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
    const lyric_object::LyricObject &assembly)
    : TestComputation(tester, computation, diagnostics),
      m_hasAssembly(true),
      m_assembly(assembly)
{
}

lyric_test::CompileModule::CompileModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics)
    : TestComputation(tester, computation, diagnostics),
      m_hasAssembly(false)
{
}

lyric_test::CompileModule::CompileModule(const CompileModule &other)
    : TestComputation(other),
      m_hasAssembly(other.m_hasAssembly),
      m_assembly(other.m_assembly)
{
}

bool
lyric_test::CompileModule::hasAssembly() const
{
    return m_hasAssembly;
}

lyric_object::LyricObject
lyric_test::CompileModule::getAssembly() const
{
    return m_assembly;
}

lyric_test::BuildModule::BuildModule()
    : TestComputation(),
      m_hasLocation(false)
{
}

lyric_test::BuildModule::BuildModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
    const lyric_common::AssemblyLocation &location)
    : TestComputation(tester, computation, diagnostics),
      m_hasLocation(true),
      m_location(location)
{
}

lyric_test::BuildModule::BuildModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics)
    : TestComputation(tester, computation, diagnostics),
      m_hasLocation(false)
{
}

lyric_test::BuildModule::BuildModule(const BuildModule &other)
    : TestComputation(other),
      m_hasLocation(other.m_hasLocation),
      m_location(other.m_location)
{
}

bool
lyric_test::BuildModule::hasLocation() const
{
    return m_hasLocation;
}

lyric_common::AssemblyLocation
lyric_test::BuildModule::getLocation() const
{
    return m_location;
}

lyric_test::PackageModule::PackageModule()
    : TestComputation(),
      m_hasUrl(false)
{
}

lyric_test::PackageModule::PackageModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
    const tempo_utils::Url &url)
    : TestComputation(tester, computation, diagnostics),
      m_hasUrl(true),
      m_url(url)
{
}

lyric_test::PackageModule::PackageModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics)
    : TestComputation(tester, computation, diagnostics),
      m_hasUrl(false)
{
}

lyric_test::PackageModule::PackageModule(const PackageModule &other)
    : TestComputation(other),
      m_hasUrl(other.m_hasUrl),
      m_url(other.m_url)
{
}

bool
lyric_test::PackageModule::hasURI() const
{
    return m_hasUrl;
}

tempo_utils::Url
lyric_test::PackageModule::getURI() const
{
    return m_url;
}

lyric_test::RunModule::RunModule()
    : TestComputation()
{
}

lyric_test::RunModule::RunModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
    std::shared_ptr<lyric_runtime::InterpreterState> state,
    const lyric_runtime::InterpreterExit &exit)
    : TestComputation(tester, computation, diagnostics),
      m_state(state),
      m_exit(exit)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_test::RunModule::RunModule(
    std::shared_ptr<AbstractTester> tester,
    const lyric_build::TargetComputation &computation,
    std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics)
    : TestComputation(tester, computation, diagnostics)
{
}

lyric_test::RunModule::RunModule(const RunModule &other)
    : TestComputation(other),
      m_state(other.m_state),
      m_exit(other.m_exit)
{
}

bool
lyric_test::RunModule::hasInterpreterState() const
{
    return m_state != nullptr;
}

std::weak_ptr<lyric_runtime::InterpreterState>
lyric_test::RunModule::getInterpreterState() const
{
    return m_state;
}

lyric_runtime::InterpreterExit
lyric_test::RunModule::getInterpreterExit() const
{
    return m_exit;
}
