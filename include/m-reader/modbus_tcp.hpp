#ifndef METER_READER_MODBUS_TCP_HPP
#define METER_READER_MODBUS_TCP_HPP

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct ModbusTcpClient ModbusTcpClient;

    /**
    * @brief create new Modbus TCP client
    * @param handle to client, or NULL during on failure
    */
    ModbusTcpClient* modbus_tcp_create(void);

    /**
     * @brief destroy Modbus TCP client and free resources
     * @param client Client handle
     */
    void modbus_tcp_destroy(ModbusTcpClient* client);

    /**
    * @brief connect to Modbus gateway
    * @param client Client handle
    * @param ip gateway ip addr
    * @param port getway port
    */
    bool modbus_tcp_connect(ModbusTcpClient* client, const char* ip, int port);

    /**
    * Register with from meter hardware RS485/Ethernet converters
    * @param client 
    * @param slave_id
    * @param reg_addr
    * @param value output: register value 
    * @param true on success, false on error
    */

    bool modbus_tcp_read_register(
        ModbusTcpClient* client,
        uint8_t slave_id,
        uint16_t register_addr,
        uint16_t* value
    );

    void modbus_tcp_disconnect(ModbusTcpClient* client);
    bool modbus_tcp_is_connected(ModbusTcpClient* client);

#ifdef __cplusplus
}
#endif

#endif // METER_READER_MODBUS_TCP_HPP