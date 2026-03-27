cmake_minimum_required(VERSION 3.10)

project(OpenGLEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(FREETYPE REQUIRED freetype2)

# Include directories
include_directories(${GLFW_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} /usr/include/glm ${CMAKE_SOURCE_DIR}/include)

# Link directories
link_directories(${GLFW_LIBRARY_DIRS} ${FREETYPE_LIBRARY_DIRS})

# Add your source files
file(GLOB SOURCES "src/*.cpp" "src/glad.c")
add_executable(OpenGLEngine ${SOURCES})

# Copy assets to the build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Link libraries
target_link_libraries(OpenGLEngine ${GLFW_LIBRARIES} ${FREETYPE_LIBRARIES} GL m dl pthread)

