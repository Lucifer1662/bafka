project ("bafka")

find_package(CapnProto CONFIG REQUIRED)
target_link_libraries(idl)

add_executable (bafka_client "main.cpp")

target_link_libraries(bafka_client CapnProto::capnp)
