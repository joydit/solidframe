add_executable (example_ipcstress ipcstress.cpp)

target_link_libraries (example_ipcstress
	solid_frame_ipc
	solid_frame_aio
	solid_frame_aio_openssl
	solid_frame_core
	solid_serialization
	solid_utility
	solid_system
	${OPENSSL_LIBS}
	${BOOST_PROGRAM_OPTIONS_LIB}
	${BOOST_SYSTEM_LIB}
	${SYS_BASIC_LIBS}
	${SYS_DYNAMIC_LOAD_LIB}
)


add_executable (example_ipcserialization ipcserialization.cpp)

target_link_libraries (example_ipcserialization
	solid_frame_ipc
	solid_frame_aio
	solid_frame_aio_openssl
	solid_frame_core
	solid_serialization
	solid_utility
	solid_system
	${BOOST_PROGRAM_OPTIONS_LIB}
	${BOOST_SYSTEM_LIB}
	${SYS_BASIC_LIBS}
	${SYS_DYNAMIC_LOAD_LIB}
)

add_executable (example_ipcgateway ipcgateway.cpp)

target_link_libraries (example_ipcgateway
	solid_frame_ipc
	solid_frame_aio
	solid_frame_aio_openssl
	solid_frame_core
	solid_serialization
	solid_utility
	solid_system
	${BOOST_PROGRAM_OPTIONS_LIB}
	${BOOST_SYSTEM_LIB}
	${SYS_BASIC_LIBS}
	${SYS_DYNAMIC_LOAD_LIB}
)



