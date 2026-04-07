cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0091 NEW)

project(receptive_field)

#set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
set(ONNX_BUILD_TESTS OFF)
#set(ONNX_USE_LITE_PROTO ON)
#set(ONNX_CUSTOM_PROTOC_EXECUTABLE ${CMAKE_BINARY_DIR}/protobuf/cmake/protoc.exe)
set(GLFW_BUILD_DOCS OFF)

add_definitions(-Dprotobuf_BUILD_TESTS=OFF)
add_definitions(-DONNX_ML=1)

add_subdirectory(protobuf/cmake)
add_subdirectory(onnx)
add_subdirectory(glfw)

if(WIN32)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_library(imgui
	imgui/imconfig.h
	imgui/imgui.cpp
	imgui/imgui.h
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_internal.h
	imgui/imgui_widgets.cpp
	imgui/imstb_rectpack.h
	imgui/imstb_textedit.h
	imgui/imstb_truetype.h
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/backends/imgui_impl_opengl3.h
	imgui/backends/imgui_impl_opengl3.cpp
	imgui/backends/imgui_impl_glfw.h
	imgui/backends/imgui_impl_glfw.cpp
	imgui/misc/cpp/imgui_stdlib.h
	imgui/misc/cpp/imgui_stdlib.cpp
	glfw/deps/glad_gl.c)

include_directories(
	onnx
	${CMAKE_BINARY_DIR}/onnx
	protobuf/src
	imgui
	imgui/backends
	imgui/misc/cpp
	glfw/include
	glfw/deps)

add_subdirectory(receptive_field_view)
