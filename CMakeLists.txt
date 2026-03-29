cmake_minimum_required(VERSION 3.10)

project(OpenGLEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(FREETYPE REQUIRED freetype2)
find_package(nlohmann_json QUIET)

if(NOT nlohmann_json_FOUND)
	message(STATUS "nlohmann_json not found, fetching it with FetchContent")
	FetchContent_Declare(
		nlohmann_json
		URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
	)
	FetchContent_MakeAvailable(nlohmann_json)
endif()

# Include directories
include_directories(${GLFW_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} /usr/include/glm ${CMAKE_SOURCE_DIR}/include)

# Link directories
link_directories(${GLFW_LIBRARY_DIRS} ${FREETYPE_LIBRARY_DIRS})

# Add your source files
file(GLOB SOURCES "src/*.cpp" "src/glad.c")
add_executable(OpenGLEngine ${SOURCES})

# Copy assets to the build directory after each build so JSON/shader edits are reflected at runtime.
add_custom_command(TARGET OpenGLEngine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets
		${CMAKE_BINARY_DIR}/assets
)

# Link libraries
target_link_libraries(OpenGLEngine ${GLFW_LIBRARIES} ${FREETYPE_LIBRARIES} nlohmann_json::nlohmann_json GL m dl pthread)

