from conan import ConanFile

class ConanTutorialRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("boost/1.83.0")
        self.requires("openssl/3.2.0")
        self.requires("libcurl/8.4.0")