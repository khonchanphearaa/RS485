#ifndef METER_READER_MODBUS_TCP_HPP
#define METER_READER_MODBUS_TCP_HPP

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ModbusTcpClient ModbusTcpClient;

ModbusTcpClient* modbus_tcp_create(void);
void modbus_tcp_destroy(ModbusTcpClient* client);
bool modbus_tcp_connect(ModbusTcpClient* client, const char* ip, int port);
bool modbus_tcp_read_register(ModbusTcpClient* client, uint8_t slave_id, uint16_t register_addr, uint16_t* value);
void modbus_tcp_disconnect(ModbusTcpClient* client);
bool modbus_tcp_is_connected(ModbusTcpClient* client);

#ifdef __cplusplus
}
#endif

#endif