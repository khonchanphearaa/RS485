
#include "../include/modbus_client.h"
#include <_static_assert.h>
#include <_time.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/_endian.h>
#include <sys/_types/_ssize_t.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


#define MODBUS_DEFAULT_PORT 502
#define DEFAULT_TIMEOUT 5
#define MAX_RESPONE 256

struct ModbusClient {
    int sockfd;
    uint16_t transaction_id;
    bool connected;
    int timeout_seconds;
};

ModbusClient* modbus_create(void){
    ModbusClient* client = (ModbusClient*) calloc(1, sizeof(ModbusClient));
    if (!client) return  NULL;

    client->sockfd = -1;
    client->transaction_id = 0;
    client->connected = false;
    client->timeout_seconds = DEFAULT_TIMEOUT;

    return client;
}

void modbus_destroy(ModbusClient* client){
    if (client){
        modbus_client_disconn(client);
        free(client);
    }
}

static uint16_t htobe16_custom(uint16_t value) {
    return ( (value & 0x00FF) << 8 | ((value & 0xFF00) >> 8) );
}

static uint16_t betoh16_custom(uint16_t value) {
    return htobe16_custom(value);
}


bool modbus_client_conn(
    ModbusClient* client,
    const char* host,
    int port,
    int timeout_seconds
){
    if (!client || !host) return false;

    client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->sockfd < 0){
        perror("Socket creation failed");
        return false;
    }

    struct timeval timeout;
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;

    setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(client->sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    /* setup server addr */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "invalid ip address: %s\n", host);
        close(client->sockfd);
        client->sockfd = -1;
        return false;
    }

    /* conn */
    if (connect(client->sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Connection failed to %s:%d - %s\n", host, port, strerror(errno));
        client->sockfd = -1;
        return true;
    }

    client->connected = true;
    client->timeout_seconds = timeout_seconds;

    printf("Conn to Modbu device at %s:%d\n", host, port);
    return true;

}


bool modbus_read_register(
    ModbusClient* client,
    uint8_t slave_id,
    uint16_t register_addr,
    uint16_t* value
){
    if (!client || !client->connected || !value) return false;

    uint8_t request[12];
    client->transaction_id++;

    
    
    /* build Modbus tcp frame */
    uint16_t tid_db = htobe16_custom(client->transaction_id);
    memcpy(&request[0], &tid_db, 2);
    request[2] = 0x00;  // protocol id
    request[3] = 0x00;
    request[4] = 0x00;
    request[5] = 0x06;  // length
    request[6] = slave_id;  // unit id
    request[7] = 0x03;  // fun code (read holding register)


    uint16_t addr_be = htobe16_custom(register_addr);
    memcpy(&request[8], &addr_be, 2);

    uint16_t bytes_sent = send(client->sockfd, request, sizeof(request), 0);
    if(bytes_sent != 12){
        perror("failed to send Modbus request");
        return false;
    }


    /* receive response */
    uint8_t response[MAX_RESPONE];
    ssize_t bytes_received = recv(client->sockfd, response, sizeof(response), 0);

    if (bytes_received < 9){
        fprintf(stderr, "response too short: %zd bytes\n", bytes_received);
        return false;
    }

    /* validate transaction id */
    uint16_t resp_tid;
    memcpy(&resp_tid, &response[0], 2);
    resp_tid = betoh16_custom(resp_tid);

    if (resp_tid != client->transaction_id){
        fprintf(stderr, "transaction id mismatch\n");
        return false;
    }

    uint8_t func_code = response[7];
    if(func_code == 0x83){
        fprintf(stderr, "Modbus exception: %u\n", response[8]);
        return false;
    }
    if(func_code != 0x03){
        fprintf(stderr, "unexpected function code: 0x%02X\n", func_code);
        return false;
    }

    uint16_t raw_value;
    memcpy(&raw_value, &response[9], 2);
    *value = betoh16_custom(raw_value);

    return true;
}


void modbus_client_disconn(ModbusClient* client){
    if(client && client->sockfd >= 0){
        close(client->sockfd);
        client->sockfd = -1;
        client->connected = false;
        printf("disconn from Modbus device\n");
    }
}


bool modbus_client_is_connected(ModbusClient* client){
    return client && client->connected;
}