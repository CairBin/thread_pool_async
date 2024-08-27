project(ThreadPoolAsync)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)
include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(${PROJECT_NAME} SHARED ${SRC})
