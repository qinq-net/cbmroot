project(ex4)

add_executable(ex4_correct ExampleClass_correct.cxx main.cxx)
add_executable(ex4_correct2 ExampleClass_correct2.cxx main.cxx)

Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Weffc++ -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-ignored-qualifiers -Wno-overloaded-virtual")
