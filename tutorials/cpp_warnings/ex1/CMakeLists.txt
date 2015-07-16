project(ex1)

add_executable(ex1_wrong ExampleClass_wrong.cxx main.cxx)

add_executable(ex1_wrong1 ExampleClass_wrong1.cxx main.cxx)

add_executable(ex1_wrong2 ExampleClass_wrong2.cxx main.cxx)

add_executable(ex1_wrong3 ExampleClass_wrong3.cxx main.cxx)

add_executable(ex1_correct ExampleClass_correct.cxx main.cxx)

add_executable(ex1_correct2 ExampleClass_correct2.cxx main.cxx)

add_executable(ex1_1 main1.cxx)

Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Weffc++ -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-ignored-qualifiers -Wno-overloaded-virtual")
