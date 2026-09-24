add_executable(meter_reader
    main.cpp
    m_service.cpp
    validator.cpp
    database.cpp
    config.cpp
    modbus_tcp.c
)

target_link_libraries(meter_reader PRIVATE
    ${MYSQLCPPCONN_LIBRARIES}
    pthread
)

set_target_properties(meter_reader PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    COMPILE_FLAGS "-fPIE"
    LINK_FLAGS "-fPIE -pie"
)