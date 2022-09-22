
cmake_minimum_required (VERSION 3.8)

project(idl2)

find_package(CapnProto CONFIG REQUIRED)


capnp_generate_cpp(CAPNP_SOURCES CAPNP_HEADERS idl.capnp)

add_library(idl2 ${CAPNP_SOURCES} ${CAPNP_HEADERS})

