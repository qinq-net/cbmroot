project(ex5)

add_executable(ex5 ExampleClass.cxx main.cxx)
add_executable(ex5_correct ExampleClass1.cxx main1.cxx)

Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Weffc++ -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-ignored-qualifiers -Wno-overloaded-virtual")
