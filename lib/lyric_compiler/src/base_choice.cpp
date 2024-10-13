
#include <lyric_compiler/base_choice.h>

lyric_compiler::BaseChoice::BaseChoice(CompilerScanDriver *driver)
    : m_block(nullptr),
      m_driver(driver)
{
    TU_ASSERT (m_driver != nullptr);
}

lyric_compiler::BaseChoice::BaseChoice(lyric_assembler::BlockHandle *block, CompilerScanDriver *driver)
    : m_block(block),
      m_driver(driver)
{
    TU_ASSERT (m_block != nullptr);
    TU_ASSERT (m_driver != nullptr);
}

lyric_assembler::BlockHandle *
lyric_compiler::BaseChoice::getBlock()
{
    return m_block;
}

lyric_compiler::CompilerScanDriver *
lyric_compiler::BaseChoice::getDriver()
{
    return m_driver;
}