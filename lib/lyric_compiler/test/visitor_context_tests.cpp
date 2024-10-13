#include <gtest/gtest.h>

#include <lyric_assembler/object_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_compiler/base_grouping.h>
#include <lyric_compiler/compiler_scan_driver.h>
#include <lyric_importer/module_cache.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_test/status_matchers.h>
#include <tempo_tracing/scope_manager.h>
#include <tempo_tracing/trace_recorder.h>

#include "compiler_mocks.h"

