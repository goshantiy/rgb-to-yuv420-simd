file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable (testElecard ${HEADERS} ${SRC})
