add_executable (example_sslclient sslclient.cpp)
set_source_files_properties(sslclient.cpp COMPILE_FLAGS -DOSSL_SOURCE_PATH="\\\"${EXTERN_PATH}\\\"")
target_link_libraries (example_sslclient ssl crypto solid_utility solid_system ${SYS_BASIC_LIBS} ${SYS_DYNAMIC_LOAD_LIB})

if(HAS_EPOLL)
	add_executable (example_sslserver sslserver.cpp)
	set_source_files_properties(sslserver.cpp COMPILE_FLAGS -DOSSL_SOURCE_PATH="\\\"${EXTERN_PATH}\\\"")
	target_link_libraries (example_sslserver ssl crypto solid_utility solid_system ${SYS_BASIC_LIBS} ${SYS_DYNAMIC_LOAD_LIB})
endif(HAS_EPOLL)
