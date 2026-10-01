from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout

class GP_Conan(ConanFile):
    name = "GraphicsProgramming"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "build_tests" : [True, False],
    }
    # Default options that are not part of the graph will be silently ignored (see glad).
    # We set this in default_options as requirements will run before configure.
    default_options = {
        "build_tests": False
    }

    def requirements(self):
        # --- Core Dependencies ---
        self.requires("spdlog/1.17.0")
        self.requires("assimp/6.0.5")
        self.requires("stb/cci.20240531", force=True)
        self.requires("sdl/3.4.8")
        self.requires("glad/2.0.8")

        # --- Unit Testing dependencies ---
        if self.options.build_tests:
            self.requires("gtest/1.17.0")

    def configure(self):
        # --- Configurations ---
        for dep in ["spdlog","assimp","sdl","gtest"]:
            if dep in self.options:
                self.options[dep].shared = False
        self.options["glad"].gl_version = "4.6"
        self.options["glad"].gl_profile = "core"

        # Linux display backend. Enable both, SDL picks at runtime.
        if self.settings.os == "Linux":
            self.options["sdl"].x11 = True
            self.options["sdl"].wayland = True

    def layout(self):
        cmake_layout(self)
        self.folders.build_folder_vars = ["settings.os", "settings.compiler", "settings.build_type"]

    def generate(self):
        tc = CMakeToolchain(self)
        tc.presets_prefix = f"conan-{str(self.settings.os).lower()}"
        tc.cache_variables["CMAKE_CONFIGURATION_TYPES"] = "Debug;Release"
        tc.cache_variables["CMAKE_INTERPROCEDURAL_OPTIMIZATION"] = "OFF"
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
