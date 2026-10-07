#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "../include/network.h"
#include "../include/modbus_client.h"


#define VERSION "1.0.0"
#define DEFAULT_E810_IP "192.168.4.101"
#define DEFAULT_E810_PORT 886
#define DEFAULT_SLAVE_ID 1
#define DEFAULT_REGISTER 0

typedef struct {
    const char* e810_ip;
    int e810_port;
    int slave_id;
    int reigster_addr;
    int timeout;
    bool verbose;
    bool test_ping;
    bool test_tcp;
    bool test_modbus;
} TestConfig;


static void print_usage(const char* program) {
    printf("E810-DTU Connection Test Tool v%s\n\n", VERSION);
    printf("Usage: %s [OPTIONS]\n\n", program);
    printf("Options:\n");
    printf("  -i, --ip IP          E810-DTU IP address (default: %s)\n", DEFAULT_E810_IP);
    printf("  -p, --port PORT      E810-DTU port (default: %d)\n", DEFAULT_E810_PORT);
    printf("  -s, --slave ID       Modbus slave ID (default: %d)\n", DEFAULT_SLAVE_ID);
    printf("  -r, --register ADDR  Modbus register address (default: %d)\n", DEFAULT_REGISTER);
    printf("  -t, --timeout SEC    Connection timeout (default: 5)\n");
    printf("  -v, --verbose        Verbose output\n");
    printf("  --ping               Test ping only\n");
    printf("  --tcp                Test TCP connection only\n");
    printf("  --modbus             Test Modbus read only\n");
    printf("  -h, --help           Show this help\n");
    printf("\nExamples:\n");
    printf("  %s -i 192.168.4.101 -p 8886\n", program);
    printf("  %s --ping --tcp --modbus\n", program);
    printf("\nExit codes:\n");
    printf("  0 - All tests passed\n");
    printf("  1 - One or more tests failed\n");
}