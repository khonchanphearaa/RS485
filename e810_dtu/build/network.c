
#include "../include/network.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <net/if.h>
#include <sys/ioctl.h>
// #include <linux/sockios.h>


#define DEFAULT_TIMEOUT 5

NetworkResult test_tcp_conn(const NetworkConfig* config) {
    if (!config || !config->host) return NET_ERROR_INVALID_IP;
    
    int sockfd = -1;
    struct sockaddr_in server_addr;
    struct timeval timeout;

    /* create socket server */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return NET_ERROR_SOCKET;
    }

    //  set timeout
    timeout.tv_sec = config->timeout_seconds;
    timeout.tv_usec = 0;
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Failed to set receive");
        close(sockfd);
        return NET_ERROR_TIMEOUT;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0){
        perror("Failed to set send timeout");
        close(sockfd);
        return NET_ERROR_TIMEOUT;
    }

    /* setup server address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config->port);
    
    /* conver ip addr */
    if(inet_pton(AF_INET, config->host, &server_addr.sin_addr) <= 0){
        fprintf(stderr, "invalid ip address: %s\n", config->host);
        close(sockfd);
        return NET_ERROR_INVALID_IP;
    }

    /* conn */
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <0 ){
        
        fprintf(stderr, "conn failed to %s:%d - %s\n", config->host, config->port, strerror(errno));
        close(sockfd);
        return NET_ERROR_CONNECT;
    }
        
    close(sockfd);
    return NET_SUCCESS;
}


bool ping_host(const char* host, int timeout_seconds){
    
    /* implement using system ping for production, raw ICMP sockets */
    char command[256];
    snprintf(
        command, sizeof(command),
        "ping -c 1 -W %d %s > /dev/null 2>&1",
        timeout_seconds, host
    );

    int result = system(command);
    return (result == 0);   
}


const char* network_msg(NetworkResult result){
    switch (result) {
        case NET_SUCCESS: 
            return "Success";
        case NET_ERROR_INVALID_IP: 
            return "Invalid ip address";
        case NET_ERROR_DNS:
            return "DNS resolution failed";
        case NET_ERROR_CONNECT:
            return "Conn failed";
        case NET_ERROR_TIMEOUT:
            return "Conn timeout";
        case NET_ERROR_SOCKET:
            return "Socket creation failed";
        default:
            return "Unknown error";
    }
}


bool config_static_ip(
    const char* interface,
    const char* ip_addr,
    const char* netmask
){
    if(!interface || !ip_addr || !netmask){
        fprintf(stderr, "Invalid network conifg parameter\n");
        return  false;
    }

    /* use system commands, netlink sockets */
    char cmd[512];
    snprintf(
        cmd, sizeof(cmd),
        "sudo ifconfig %s %s netmark %s up",
        interface, ip_addr, netmask
    );

    if (system(cmd) != 0){
        fprintf(stderr, "Faild to conifg ip address\n");
        return false;
    }

    printf("conifg %s with IP %s netmark %s\n", interface, ip_addr, netmask);
    return  true;
}