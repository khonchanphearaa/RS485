
#include "../include/network.h"
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
#include <linux/sockios.h>


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