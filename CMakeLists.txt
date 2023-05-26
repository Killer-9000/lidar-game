cmake_minimum_required ( VERSION 3.10 )

# Set the project name
project ( lidargame )

# Specify the C++ standard
set ( CMAKE_CXX_STANDARD 20 )
set ( CMAKE_CXX_STANDARD_REQUIRED True )
set ( USE_FOLDERS ON)
set ( PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")
set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Compiler options
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI /MP /Zf")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL")
endif()

add_compile_definitions(SOURCE_DIR="${CMAKE_SOURCE_DIR}")

set ( ENABLE_DX12 OFF)
set ( ENABLE_VULKAN OFF)

file(GLOB_RECURSE common_sources ${CMAKE_SOURCE_DIR}/src/lidar-common/*)
file(GLOB_RECURSE editor_sources ${CMAKE_SOURCE_DIR}/src/lidar-editor/*)
file(GLOB_RECURSE game_sources ${CMAKE_SOURCE_DIR}/src/lidar-game/*)

# Add dependencies
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor)

find_package(assimp REQUIRED)
find_package(entt REQUIRED)
find_package(fmt REQUIRED)
find_package(glm REQUIRED)
find_package(tsl-robin-map REQUIRED)
find_package(SDL2 REQUIRED)

# Add our executables and libraries
add_library ( lidarcommon
    ${common_sources}
)
add_executable ( lidareditor
    ${editor_sources}
)
add_executable ( lidargame
    ${game_sources}
)

# Include directories.
target_include_directories ( lidarcommon
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/lidar-common
        ${CMAKE_SOURCE_DIR}/src/lidar-common/graphics/imgui
        ${CMAKE_SOURCE_DIR}/src/lidar-common/graphics/ffx-cacao
        ${GLM_INCLUDE_DIRS}
)
target_include_directories ( lidareditor
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/lidar-editor
)
target_include_directories ( lidargame
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/lidar-game
)

target_link_libraries(lidarcommon
    PUBLIC
        assimp::assimp
        Diligent-GraphicsEngineVk-static
        Diligent-TargetPlatform
        Diligent-Common
		EnTT::EnTT
        fmt::fmt
		glm::glm
        tsl::robin_map
        SDL2::SDL2-static
        Vulkan::Headers
)
target_link_libraries(lidareditor
    PRIVATE
        lidarcommon
)
target_link_libraries(lidargame
    PRIVATE
        lidarcommon
)

# Copy resource files.
#add_custom_command(
#    TARGET lidargame PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E rm -R
#            ${CMAKE_CURRENT_BINARY_DIR}/resources/
#)
#add_custom_command(
#    TARGET lidargame PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#            ${CMAKE_SOURCE_DIR}/resources/
#            ${CMAKE_CURRENT_BINARY_DIR}/resources/
#)
