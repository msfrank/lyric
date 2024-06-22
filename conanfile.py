from os.path import join

from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy

class Lyric(ConanFile):
    name = 'lyric'
    version = '0.0.1'
    license = 'BSD-3-Clause'
    url = 'https://github.com/msfrank/lyric'
    description = 'Build system and runtime for the Zuri project'

    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {'shared': [True, False], 'compiler.cppstd': ['17', '20'], 'build_type': ['Debug', 'Release']}
    default_options = {'shared': True, 'compiler.cppstd': '20', 'build_type': 'Debug'}

    exports_sources = (
        'CMakeLists.txt',
        'bin/*',
        'cmake/*',
        'lib/*',
        )

    requires = (
        'tempo/0.0.1',
        # requirements from timbre
        'absl/20230802.1@timbre',
        'antlr/4.9.3@timbre',
        'boost/1.84.0@timbre',
        'fmt/9.1.0@timbre',
        'flatbuffers/23.5.26@timbre',
        'gtest/1.14.0@timbre',
        'icu/74.1@timbre',
        'openssl/3.2.0@timbre',
        'rocksdb/8.5.3@timbre',
        'uv/1.44.1@timbre',
        )

    def validate(self):
        check_min_cppstd(self, "20")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        antlr = self.dependencies['antlr'].buildenv_info.vars(self)
        flatbuffers = self.dependencies['flatbuffers'].buildenv_info.vars(self)

        tc = CMakeToolchain(self)
        tc.variables['LYRIC_PACKAGE_VERSION'] = self.version
        tc.variables['ANTLR_TOOL_JAR'] = antlr.get('ANTLR_TOOL_JAR')
        tc.variables['FLATBUFFERS_FLATC'] = flatbuffers.get('FLATBUFFERS_FLATC')
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "none")
        self.cpp_info.builddirs.append(join("lib", "cmake", "lyric"))
