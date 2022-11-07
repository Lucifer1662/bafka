project ("context")



file(GLOB SRC
{    "*.cpp"
     "*.cc"
)
add_executable (app "${SRC}" )
set(SFML_STATIC_LIBRARIES TRUE)
target_link_libraries(app sfml-graphics sfml-window)

