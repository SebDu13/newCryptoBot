from conan import ConanFile

class ConanTutorialRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain", "cmake_find_package"

    def requirements(self):
        self.requires("boost/1.83.0")