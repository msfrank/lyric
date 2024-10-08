#ifndef LYRIC_TEST_TEST_RUN_H
#define LYRIC_TEST_TEST_RUN_H

#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_test/abstract_tester.h>

namespace lyric_test {

    class TestComputation {

    public:
        TestComputation();
        TestComputation(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        TestComputation(const TestComputation &other);
        TestComputation(TestComputation &&other) noexcept;

        TestComputation& operator=(const TestComputation &other);
        TestComputation& operator=(TestComputation &&other) noexcept;

        std::shared_ptr<AbstractTester> getTester() const;
        bool hasComputation() const;
        lyric_build::TargetComputation getComputation() const;
        std::shared_ptr<lyric_build::BuildDiagnostics> getDiagnostics() const;

    private:
        std::shared_ptr<AbstractTester> m_tester;
        lyric_build::TargetComputation m_computation;
        std::shared_ptr<lyric_build::BuildDiagnostics> m_diagnostics;
    };

    class SymbolizeModule : public TestComputation {

    public:
        SymbolizeModule();
        SymbolizeModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
            const lyric_object::LyricObject &object);
        SymbolizeModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        SymbolizeModule(const SymbolizeModule &other);

        bool hasModule() const;
        lyric_object::LyricObject getModule() const;

    private:
        bool m_hasModule;
        lyric_object::LyricObject m_object;
    };

    class AnalyzeModule : public TestComputation {

    public:
        AnalyzeModule();
        AnalyzeModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
            const lyric_object::LyricObject &object);
        AnalyzeModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        AnalyzeModule(const AnalyzeModule &other);

        bool hasModule() const;
        lyric_object::LyricObject getModule() const;

    private:
        bool m_hasModule;
        lyric_object::LyricObject m_object;
    };

    class CompileModule : public TestComputation {

    public:
        CompileModule();
        CompileModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
            const lyric_object::LyricObject &object);
        CompileModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        CompileModule(const CompileModule &other);

        bool hasModule() const;
        lyric_object::LyricObject getModule() const;

    private:
        bool m_hasModule;
        lyric_object::LyricObject m_object;
    };

    class BuildModule : public TestComputation {

    public:
        BuildModule();
        BuildModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
            const lyric_common::ModuleLocation &location);
        BuildModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        BuildModule(const BuildModule &other);

        bool hasLocation() const;
        lyric_common::ModuleLocation getLocation() const;

    private:
        bool m_hasLocation;
        lyric_common::ModuleLocation m_location;
    };

    class PackageModule : public TestComputation {

    public:
        PackageModule();
        PackageModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
            const tempo_utils::Url &uri);
        PackageModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        PackageModule(const PackageModule &other);

        bool hasURI() const;
        tempo_utils::Url getURI() const;

    private:
        bool m_hasUrl;
        tempo_utils::Url m_url;
    };

    class RunModule : public TestComputation {

    public:
        RunModule();
        RunModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
            std::shared_ptr<lyric_runtime::InterpreterState> state,
            const lyric_runtime::InterpreterExit &exit);
        RunModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        RunModule(const RunModule &other);

        bool hasInterpreterState() const;
        std::weak_ptr<lyric_runtime::InterpreterState> getInterpreterState() const;
        lyric_runtime::InterpreterExit getInterpreterExit() const;

    private:
        std::shared_ptr<lyric_runtime::InterpreterState> m_state;
        lyric_runtime::InterpreterExit m_exit;
    };
}

#endif // LYRIC_TEST_TEST_RUN_H