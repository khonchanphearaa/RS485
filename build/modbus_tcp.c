/**
 *  @file modbus_tcp.c
 *  @brief that i define with C programming:
 *  define the Modbus tcp protocol implementation that based on low-level
 * socket operations Socket program that keeps the protocol layter minimal, fast
 * and reliable in embedded systems and IoT applications--but the specific rely
 * on a mix soild engineering truths and sligthly1
 */

#include "m-reader/modbus_tcp.hpp"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


#define MODBUS_DEFAULT_PORT 502
#define MODBUS_MAX_RESPONSE 256
#define SOCKET_TIMEOUT_SEC 5      /* define timeout socket 5 seconds */

struct ModbusTcpClient {
    int sockfd;
    uint16_t transaction_id;
    bool connected;
};

ModbusTcpClient* modbus_tcp_create(void) {
    ModbusTcpClient* client = (ModbusTcpClient*)calloc(1, sizeof(ModbusTcpClient));
    if (!client) {
        fprintf(stderr, "[Error]: failed to allocate client");
        return NULL;
    }

        client->sockfd = -1;
        client->transaction_id = 0;
        client->connected = false;
        return client;
}


void modbus_tcp_destroy(ModbusTcpClient* client) {
    if (client) {
        modbus_tcp_disconnect(client);
        free(client);
    }
}

bool modbus_tcp_connect(ModbusTcpClient* client, const char* ip, int port) {
    if(!client || !ip){
        return false;
    }


    /* create tcp socket */
    client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->sockfd < 0) {
        perror("[Error]: socket create failed");
        return false;
    }

    /* setup server address */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) < 0) {
        perror("invalid ip address");

        close(client->sockfd);
        client->sockfd = -1;
        return false;
    }


    /* set socket timeouts  */
    struct timeval timeout;
    timeout.tv_sec = SOCKET_TIMEOUT_SEC;
    timeout.tv_usec = 0;

    if (setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <0){
        perror("failed to set socket timeout");

        close(client->sockfd);
        client->sockfd = -1;
        return false;
    }

    if (setsockopt(client->sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("failed t set send timeout");
        
        close(client->sockfd);
        client->sockfd = -1;
        return false;
    }

    /* connected to gateway */
    if (connect(client->sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("connection failed");
        
        close(client->sockfd);
        client->sockfd = -1;
        return false;
    }

    client-> connected = true;
    printf("connected to Modbus gateway at %s:%d\n", ip, port);
    return true;

}

static uint16_t htobe16(uint16_t value) {
    return ((value & 0x00FF) << 8 | ((value & 0xFF00) >> 8));
}

static uint16_t betoh16(uint16_t value){
    return htobe16(value);
}

bool modbus_tcp_read_register(
    ModbusTcpClient* client,
    uint8_t slave_id,
    uint16_t register_addr,
    uint16_t* value
){
    if (!client || !client->connected || !value) return false;


    /* build Modbus tcp frame 12bytes */
    uint8_t request[12];
    client->transaction_id++;

    /* MBAP header 7bytes */
    uint16_t tid_be = htobe16(client->transaction_id);
    memcpy(&request[0], &tid_be, 2);
    
    request[2] = 0x00;
    request[3] = 0x00;
    request[4] = 0x00;
    request[5] = 0x06;
    request[6] = slave_id;

    /* PDU read holding register func 03 */
    request[7] = 0x03;
    uint16_t addr_be = htobe16(register_addr);
    memcpy(&request[8], &addr_be, 2);
    uint16_t quantity_be = htobe16(1);
    memcpy(&request[10], &quantity_be, 2);



    /* send request 12 bytes over the network */
    ssize_t bytes_sent = send(client->sockfd, request, sizeof(request), 0);
    if (bytes_sent != 12) {
        perror("failed to send Modbus request");
        return false;
    }

    /* receive response that minimum 9 bytes */
    uint8_t response[MODBUS_MAX_RESPONSE];
    ssize_t bytes_received = recv(client->sockfd, response, sizeof(response), 0);

    if (bytes_received < 9) {
        fprintf(stderr, "Response too short: %zd bytes\n", bytes_received);
        return false;
    }

    /* validation tran_id, proto_id, func_code */

    uint16_t resp_tid;
    memcpy(&resp_tid, &response[0], 2);
    resp_tid = betoh16(resp_tid);
    
    if (resp_tid != client->transaction_id) {
        fprintf(stderr, "Transaction ID mismatch: expected %u, got %u\n", client->transaction_id, resp_tid);
        return false;
    }
    
    uint16_t proto_id;
    memcpy(&proto_id, &response[2], 2);
    proto_id = betoh16(proto_id);
    
    if (proto_id != 0) {
        fprintf(stderr, "Invalid Protocol ID: %u\n", proto_id);
        return false;
    }

    uint16_t func_code = response[7];
    if (func_code == 0x83) {
        fprintf(stderr, "Modbus exception code: %u\n", response[8]);
        return false;
    }

    if (func_code != 0x03) {
        fprintf(stderr, "Unexpected function code: 0x%02X\n", func_code);
        return false;
    }

    uint16_t raw_value;
    memcpy(&raw_value, &response[9], 2);
    *value = betoh16(raw_value);

    return true;

}

void modbus_tcp_disconnect(ModbusTcpClient *client) {
    if (client && client->sockfd >= 0) {
        close(client->sockfd);
        client->sockfd = -1;
        client->connected = false;

        printf("disconnected from Modbus geteways\n");
    }
}

bool modbus_tcp_is_connected(ModbusTcpClient *client) {
    return client && client->connected;
}