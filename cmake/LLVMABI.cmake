# - Build llvm-abi support library.

# TODO: provide a way to use a custom build of llvm-abi.

set(LLVMABI_INSTALL_DIR "${PROJECT_BINARY_DIR}/llvm-abi-install")
set(LLVMABI_INCLUDE_LOCATION "${LLVMABI_INSTALL_DIR}/include")
set(LLVMABI_LIB_LOCATION "${LLVMABI_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}llvm-abi${CMAKE_STATIC_LIBRARY_SUFFIX}")

include(ExternalProject)

set(LLVMABI_REPO "https://github.com/scross99/llvm-abi.git")

set(LLVMABI_CMAKE_ARGS
	"-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
	"-DCMAKE_INSTALL_PREFIX=${LLVMABI_INSTALL_DIR}"
)

# Pull llvm-abi from repository and build it.
ExternalProject_Add(LLVMABIProject
	GIT_REPOSITORY "${LLVMABI_REPO}"
	UPDATE_COMMAND ""
	CMAKE_ARGS ${LLVMABI_CMAKE_ARGS}
	INSTALL_DIR "${LLVMABI_INSTALL_DIR}"
)

# Create imported library target.
add_library(llvm-abi STATIC IMPORTED GLOBAL)

set_target_properties(llvm-abi PROPERTIES
	IMPORTED_LOCATION "${LLVMABI_LIB_LOCATION}"
)

add_dependencies(llvm-abi LLVMABIProject)

message(STATUS "Building llvm-abi from ${LLVMABI_REPO}")

set(LLVMABI_DEPENDENCIES llvm-abi)
set(LLVMABI_INCLUDE_DIRS "${LLVMABI_INCLUDE_LOCATION}")
set(LLVMABI_LIBRARIES llvm-abi)
