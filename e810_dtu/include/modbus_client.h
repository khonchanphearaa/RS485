
#ifndef MODBUS_CLIENT_H
#define MODBUS_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct ModbusClient ModbusClient;

ModbusClient* modbus_create(void);

void modbus_destroy(ModbusClient* client);


/*
* Conn to modbus devices on tcp follow param:
*
* @param client handle
* @param host ip addr
* @param port tcp
* @param timout_seconds 
* @return true on success
*/
bool modbus_client_conn(
    ModbusClient* client,
    const char* host,
    int port,
    int timeout_seconds
);


/*
* read holding register (Function 03)
* 
* @param client handle
* @param slave_id Modbus slave ID (1-247)
* @param register_addr
* @param value output: register value
* @param true on success
*/
bool modbus_read_register(
    ModbusClient* client,
    uint8_t slave_id,
    uint16_t register_addr,
    uint16_t* value
);


void modbus_client_disconn(ModbusClient* client);
void modbus_client_is_conn(ModbusClient* client);


#endif // MODBUS_CLIENT_H