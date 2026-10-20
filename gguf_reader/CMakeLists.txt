add_executable(gguf_test gguf_test.cpp)
add_library(gguf_reader gguf_reader.h gguf_reader.cpp)

set_property(TARGET gguf_reader PROPERTY CXX_STANDARD 17)
set_property(TARGET gguf_test PROPERTY CXX_STANDARD 17)

add_dependencies(gguf_test gguf_reader)
target_link_libraries(gguf_test gguf_reader)