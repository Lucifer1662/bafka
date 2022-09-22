project ("bafka")

 
file(GLOB SRC
     "*.h"
     "*.cpp"
)
add_executable (bafka_server "${SRC}" )


target_link_libraries(bafka_server bafka)
