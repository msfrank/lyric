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
            const lyric_object::LyricObject &assembly);
        SymbolizeModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        SymbolizeModule(const SymbolizeModule &other);

        bool hasAssembly() const;
        lyric_object::LyricObject getAssembly() const;

    private:
        bool m_hasAssembly;
        lyric_object::LyricObject m_assembly;
    };

    class AnalyzeModule : public TestComputation {

    public:
        AnalyzeModule();
        AnalyzeModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
            const lyric_object::LyricObject &assembly);
        AnalyzeModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        AnalyzeModule(const AnalyzeModule &other);

        bool hasAssembly() const;
        lyric_object::LyricObject getAssembly() const;

    private:
        bool m_hasAssembly;
        lyric_object::LyricObject m_assembly;
    };

    class CompileModule : public TestComputation {

    public:
        CompileModule();
        CompileModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
            const lyric_object::LyricObject &assembly);
        CompileModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        CompileModule(const CompileModule &other);

        bool hasAssembly() const;
        lyric_object::LyricObject getAssembly() const;

    private:
        bool m_hasAssembly;
        lyric_object::LyricObject m_assembly;
    };

    class BuildModule : public TestComputation {

    public:
        BuildModule();
        BuildModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics,
            const lyric_common::AssemblyLocation &location);
        BuildModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        BuildModule(const BuildModule &other);

        bool hasLocation() const;
        lyric_common::AssemblyLocation getLocation() const;

    private:
        bool m_hasLocation;
        lyric_common::AssemblyLocation m_location;
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
            const lyric_runtime::Return &ret);
        RunModule(
            std::shared_ptr<AbstractTester> tester,
            const lyric_build::TargetComputation &computation,
            std::shared_ptr<lyric_build::BuildDiagnostics> diagnostics);
        RunModule(const RunModule &other);

        bool hasReturn() const;
        lyric_runtime::Return getReturn() const;

    private:
        bool m_hasReturn;
        lyric_runtime::Return m_ret;
    };


}

#endif // LYRIC_TEST_TEST_RUN_H