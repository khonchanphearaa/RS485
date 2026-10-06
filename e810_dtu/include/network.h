/*
* @file network.h
* @brief define network utitilties tests e810-dtu conn
*/

#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    NET_SUCCESS = 0,
    NET_ERROR_INVALID_IP,
    NET_ERROR_DNS,
    NET_ERROR_CONNECT,
    NET_ERROR_TIMEOUT,
    NET_ERROR_SOCKET
} NetworkResult;


typedef struct {
    const char* host;
    int port;
    int timeout_seconds;
} NetworkConfig;



/*
* tests tcp conn to host:port
* @param privide config network config
* @return NET_SUCCESS on success, error code on failed
*/
NetworkResult test_tcp_conn(const NetworkConfig* Config);


/*
* ping to host by: (ICMP echo) logs tests
* host ip address/hostname, timeout(ms), 
* true if host reachable conn
*/
bool ping_host(const char* host, int timeout_seconds);

const char* network_msg(NetworkResult result);

/*
* @brief define network interface about the static_ip 
* on devices are follows on parameters:
* @param interface network name (e.g., "eth0",...)
* @param ip_addr static ip that devices set
* @param netmask subnet mask
* @param true on success
*/
bool config_static_ip(
    const char* interface,
    const char* ip_addr,
    const char* netmask
);

#endif // NETWORK_H

