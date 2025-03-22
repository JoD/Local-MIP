cmake_minimum_required(VERSION 3.0)

project(Local-MIP)
set(CMAKE_CXX_STANDARD 20)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall")




file(GLOB_RECURSE SOURCES "code/*.cpp" "code/*.cc")

set(INCLUDES ${PROJECT_SOURCE_DIR}/code)


add_executable(${PROJECT_NAME} ${SOURCES})


include_directories(${INCLUDES})

target_link_libraries(Local-MIP pthread -lpthread -lquadmath z -lz boost_thread boost_date_time boost_system)
