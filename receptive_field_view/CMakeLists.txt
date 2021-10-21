
add_executable(receptive_field_view
	main.cpp graph.h graph.cpp graph_view.h graph_view.cpp utils.h colors.h)

add_dependencies(receptive_field_view onnx)

set_property(TARGET receptive_field_view PROPERTY CXX_STANDARD 17)
set_property(TARGET receptive_field_view PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
target_link_libraries(receptive_field_view imgui glfw onnx)

message(PROTOBUF_LIBRARY=${PROTOBUF_LIBRARY})

if(WIN32)
	target_link_libraries(receptive_field_view opengl32 glu32)
elseif(UNIX)
	target_link_libraries(receptive_field_view GL GLU)
	target_link_libraries(receptive_field_view stdc++)
endif()
