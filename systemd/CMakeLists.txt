  set(SYSTEMD_CONFIGURATIONS_FILES_DIR "/etc/systemd/system" )

 configure_file(${CMAKE_SOURCE_DIR}/systemd/a1_broker.service.cmake ${PROJECT_BINARY_DIR}/systemd/a1_broker.service)
    message( STATUS "Configured systemd unit file:a1_broker.service" )

if(EXISTS ${SYSTEMD_CONFIGURATIONS_FILES_DIR})
        install(FILES ${PROJECT_BINARY_DIR}/systemd/a1_broker.service DESTINATION ${SYSTEMD_CONFIGURATIONS_FILES_DIR} )
endif(EXISTS ${SYSTEMD_CONFIGURATIONS_FILES_DIR})
   
message(STATUS "Unit files will be installed to ${SYSTEMD_CONFIGURATIONS_FILES_DIR} after make install" )

macro(INSTALL_EXEC_CMD _COMMAND _FAILMSG)
    install(CODE "
        execute_process(COMMAND ${_COMMAND} RESULT_VARIABLE install_exec_res)
        if (NOT \"\${install_exec_res}\" STREQUAL \"0\")
            message(WARNING \"WARNING: ${_COMMAND} failed: \${install_exec_res}\n${_FAILMSG}\n\")
        else (NOT \"\${install_exec_res}\" STREQUAL \"0\")
            message(STATUS \"${_COMMAND}: \${install_exec_res}\")
        endif (NOT \"\${install_exec_res}\" STREQUAL \"0\")
    ")
endmacro(INSTALL_EXEC_CMD)

install_exec_cmd("systemctl daemon-reload"
             "If you use systemd, please reload systemd manually or reboot the system")

install_exec_cmd("systemctl enable a1_broker.service"
             "If you use systemd, enable razerd.service manually")
 
