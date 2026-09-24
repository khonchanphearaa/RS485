#include "m_reader/modbus_tcp.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct ModbusTcpClient {
    int sockfd;
    uint16_t transaction_id;
    bool connected;
};

ModbusTcpClient* modbus_tcp_create(void) {
    ModbusTcpClient* client = (ModbusTcpClient*)calloc(1, sizeof(ModbusTcpClient));
    if (!client) return NULL;
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
    if (!client || !ip) return false;
    
    client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->sockfd < 0) {
        perror("Socket creation failed");
        return false;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(client->sockfd);
        client->sockfd = -1;
        return false;
    }
    
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(client->sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    if (connect(client->sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client->sockfd);
        client->sockfd = -1;
        return false;
    }
    
    client->connected = true;
    printf("Connected to Modbus gateway at %s:%d\n", ip, port);
    return true;
}


bool modbus_tcp_read_register(ModbusTcpClient* client, uint8_t slave_id, uint16_t register_addr, uint16_t* value) {
    if (!client || !client->connected || !value) return false;
    
    uint8_t request[12];
    client->transaction_id++;
    
    uint16_t tid_be = htons(client->transaction_id);
    memcpy(&request[0], &tid_be, 2);
    request[2] = 0x00;
    request[3] = 0x00;
    request[4] = 0x00;
    request[5] = 0x06;
    request[6] = slave_id;
    request[7] = 0x03;
    
    uint16_t addr_be = htons(register_addr);
    memcpy(&request[8], &addr_be, 2);
    
    uint16_t quantity_be = htons(1);
    memcpy(&request[10], &quantity_be, 2);
    
    ssize_t bytes_sent = send(client->sockfd, request, sizeof(request), 0);
    if (bytes_sent != 12) {
        perror("Failed to send Modbus request");
        return false;
    }
    
    uint8_t response[256];
    ssize_t bytes_received = recv(client->sockfd, response, sizeof(response), 0);
    
    if (bytes_received < 9) {
        fprintf(stderr, "Response too short: %zd bytes\n", bytes_received);
        return false;
    }
    
    uint16_t resp_tid;
    memcpy(&resp_tid, &response[0], 2);
    resp_tid = ntohs(resp_tid);
    
    if (resp_tid != client->transaction_id) {
        fprintf(stderr, "Transaction ID mismatch\n");
        return false;
    }
    
    uint8_t func_code = response[7];
    if (func_code == 0x83) {
        fprintf(stderr, "Modbus exception: %u\n", response[8]);
        return false;
    }
    if (func_code != 0x03) {
        fprintf(stderr, "Unexpected function code: 0x%02X\n", func_code);
        return false;
    }
    
    uint16_t raw_value;
    memcpy(&raw_value, &response[9], 2);
    *value = ntohs(raw_value);
    
    return true;
}

void modbus_tcp_disconnect(ModbusTcpClient* client) {
    if (client && client->sockfd >= 0) {
        close(client->sockfd);
        client->sockfd = -1;
        client->connected = false;
        printf("Disconnected from Modbus gateway\n");
    }
}

bool modbus_tcp_is_connected(ModbusTcpClient* client) {
    return client && client->connected;
}