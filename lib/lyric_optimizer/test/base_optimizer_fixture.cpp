#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/block_handle.h>
#include <lyric_runtime/static_loader.h>

#include "base_optimizer_fixture.h"

#include <lyric_optimizer/parse_proc.h>

BaseOptimizerFixture::BaseOptimizerFixture()
{
    m_staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    m_testerOptions.fallbackLoader = m_staticLoader;
    m_testerOptions.overrides = lyric_build::TaskSettings(tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
            {"sourceBaseUrl", tempo_config::ConfigValue("/src")},
        }},
    });
}

tempo_utils::Status
BaseOptimizerFixture::configure()
{
    m_tester = std::make_unique<lyric_test::LyricTester>(m_testerOptions);
    TU_RETURN_IF_NOT_OK (m_tester->configure());

    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    auto localModuleCache = lyric_importer::ModuleCache::create(staticLoader);
    auto sharedModuleCache = m_tester->getRunner()->getBuilder()->getSharedModuleCache();
    auto recorder = tempo_tracing::TraceRecorder::create();
    tempo_tracing::ScopeManager scopeManager(recorder);
    m_objectState = std::make_unique<lyric_assembler::ObjectState>(
        location, localModuleCache, sharedModuleCache, &scopeManager);

    TU_ASSIGN_OR_RAISE (m_objectRoot, m_objectState->defineRoot());

    return {};
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
BaseOptimizerFixture::declareFunction(
    const std::string &name,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    auto *rootBlock = m_objectRoot->rootBlock();
    return rootBlock->declareFunction(name, access, templateParameters);
}

lyric_optimizer::ControlFlowGraph
BaseOptimizerFixture::parseProc(lyric_assembler::ProcHandle *procHandle)
{
    auto *code = procHandle->procCode();
    auto *root = code->rootFragment();

    TU_LOG_INFO;
    TU_LOG_INFO << "parsing proc:";
    for (auto it = root->statementsBegin(); it != root->statementsEnd(); it++) {
        const auto &instruction = it->instruction;
        TU_LOG_INFO << "  " << instruction->toString();
    }

    TU_LOG_INFO;
    TU_LOG_INFO << "constructed cfg:";
    lyric_optimizer::ControlFlowGraph cfg;
    TU_ASSIGN_OR_RAISE (cfg, lyric_optimizer::parse_proc(procHandle));
    cfg.print();

    TU_LOG_INFO;
    TU_LOG_INFO << "declared variables:";
    for (int i = 0; i < cfg.numArguments(); i++) {
        auto variable = cfg.getArgument(i);
        TU_LOG_INFO << "  argument " << variable.getName();
        for (int j = 0; j < variable.numInstances(); j++) {
            auto instance = variable.getInstance(j);
            TU_LOG_INFO << "    " << instance.toString();
        }
    }
    for (int i = 0; i < cfg.numLocals(); i++) {
        auto variable = cfg.getLocal(i);
        TU_LOG_INFO << "  local " << variable.getName();
        for (int j = 0; j < variable.numInstances(); j++) {
            auto instance = variable.getInstance(j);
            TU_LOG_INFO << "    " << instance.toString();
        }
    }
    for (int i = 0; i < cfg.numLexicals(); i++) {
        auto variable = cfg.getLexical(i);
        TU_LOG_INFO << "  lexical " << variable.getName();
        for (int j = 0; j < variable.numInstances(); j++) {
            auto instance = variable.getInstance(j);
            TU_LOG_INFO << "    " << instance.toString();
        }
    }

    return cfg;
}
