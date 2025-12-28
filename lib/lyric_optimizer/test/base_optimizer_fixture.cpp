#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/block_handle.h>
#include <lyric_runtime/static_loader.h>

#include "base_optimizer_fixture.h"

#include <lyric_optimizer/parse_proc.h>

BaseOptimizerFixture::BaseOptimizerFixture()
{
    m_staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    m_testerOptions.bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    m_testerOptions.fallbackLoader = m_staticLoader;
}

tempo_utils::Status
BaseOptimizerFixture::configure()
{
    m_tester = std::make_unique<lyric_test::LyricTester>(m_testerOptions);
    TU_RETURN_IF_NOT_OK (m_tester->configure());

    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    auto localModuleCache = lyric_importer::ModuleCache::create(staticLoader);
    auto *builder = m_tester->getRunner()->getBuilder();
    auto sharedModuleCache = builder->getSharedModuleCache();
    auto shortcutResolver = builder->getShortcutResolver();
    auto recorder = tempo_tracing::TraceRecorder::create();
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("tester://", tempo_utils::UUID::randomUUID().toString()));

    m_objectState = std::make_unique<lyric_assembler::ObjectState>(
        location, origin, localModuleCache, sharedModuleCache, shortcutResolver);

    TU_ASSIGN_OR_RAISE (m_objectRoot, m_objectState->defineRoot());

    return {};
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
BaseOptimizerFixture::declareFunction(
    const std::string &name,
    bool isHidden,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    auto *rootBlock = m_objectRoot->rootBlock();
    return rootBlock->declareFunction(name, isHidden, templateParameters);
}

lyric_optimizer::ControlFlowGraph
BaseOptimizerFixture::parseProc(lyric_assembler::ProcHandle *procHandle)
{
    auto *fragment = procHandle->procFragment();

    TU_LOG_INFO;
    TU_LOG_INFO << "parsing proc:";
    for (auto it = fragment->statementsBegin(); it != fragment->statementsEnd(); it++) {
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
