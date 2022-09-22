project ("bafka")

find_package(CapnProto CONFIG REQUIRED)

target_link_libraries(idl2)

 
set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC
     "*.cpp"
)

add_library(bafka ${SRC}) 

set_target_properties(bafka PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(bafka CapnProto::capnp)
target_include_directories(bafka INTERFACE ${CMAKE_SOURCE_DIR}/src )
target_compile_definitions(bafka PRIVATE)