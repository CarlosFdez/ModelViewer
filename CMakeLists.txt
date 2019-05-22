# Project Info

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(VERSION 3.13)
project(ModelViewer
    VERSION 1.0.0.0
    LANGUAGES C CXX
)

# ===================================================

# Settings

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_SUPPRESS_REGENERATION true)
set(DCMAKE_GENERATOR_PLATFORM "x64")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
if(NOT CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX d)
endif()

# =================================================================

# Dependencies

# CrossWindow
add_subdirectory(external/crosswindow ${CMAKE_BINARY_DIR}/crosswindow)
set_property(TARGET CrossWindow PROPERTY FOLDER "Dependencies")

# imgui
# IMGUI
message(STATUS "Installing ImGUI via submodule")
set(
  IMGUI_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_draw.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
)
add_library(
  IMGUI
  "${IMGUI_SOURCES}"
)
target_include_directories(
  IMGUI
  # PUBLIC "exteral/stb" # figure out if required
  PUBLIC "external/imgui"
)

# GLM
message(STATUS "Installing glm via submodule")
set(BUILD_STATIC_LIBS ON)
add_subdirectory(external/glm/glm)
set_property(TARGET glm_static PROPERTY FOLDER "Dependencies")

# =================================================================

# Sources

file(GLOB_RECURSE FILE_SOURCES RELATIVE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/XMain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

# =============================================================


# Finalize App (Use CrossWindow's function)

xwin_add_executable(
    ${PROJECT_NAME}
    "${FILE_SOURCES}"
)

#==================================================================

# Finish Dependencies

target_link_libraries(
    ${PROJECT_NAME}
    CrossWindow
	glm_static
	IMGUI
)


target_include_directories(
	${PROJECT_NAME}
	PUBLIC "external/glm"
)

add_dependencies(
	${PROJECT_NAME}
	glm_static
	IMGUI
)

# Change output dir to bin
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Change working directory to top dir to access `assets` folder
# Note disabled because we use a symlink based solution instead
message(STATUS "Compiler ID is ${CMAKE_CXX_COMPILER_ID}")
 if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
     set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
     set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

# Only works on Windows in version 3.13 and later.
set (source "${CMAKE_SOURCE_DIR}/assets")
set (destination "${CMAKE_BINARY_DIR}/bin/assets")
add_custom_command(
     TARGET ${PROJECT_NAME} POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination} 
     DEPENDS ${destination}
     COMMENT "symbolic link resources folder from ${source} => ${destination}"
)