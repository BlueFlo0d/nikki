aux_source_directory(. WKDIR_SRCS)

set(RL_LIBS readline)
find_library(CHECK_RL ${RL_LIBS})
if(CHECK_RL)
 message(status " libreadline found.")
else(CHECK_RL)
 message(FATAL_ERROR " libreadline not found")
endif(CHECK_RL)

add_executable(nikki ${WKDIR_SRCS})
target_link_libraries(nikki qtdiary)
target_link_libraries(nikki ${RL_LIBS})
install(TARGETS nikki RUNTIME DESTINATION bin)
