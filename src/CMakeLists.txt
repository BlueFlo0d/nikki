cmake_minimum_required (VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Debug build.")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")

aux_source_directory(. WKDIR_SRCS)
set(NETTLE_LIBS nettle hogweed gmp)
find_library(CHECK_NETTLE ${NETTLE_LIBS})
if(CHECK_NETTLE)
 message(status " libnettle found.")
else(CHECK_NETTLE)
 message(FATAL_ERROR " libnettle not found")
endif(CHECK_NETTLE)

add_library(qtdiary SHARED ${WKDIR_SRCS})
target_link_libraries(qtdiary ${NETTLE_LIBS})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g --std=c99")

install(TARGETS qtdiary LIBRARY DESTINATION lib)
