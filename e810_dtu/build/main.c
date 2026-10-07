
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "../include/network.h"
#include "../include/modbus_client.h"

#define VERSION "1.0.0"
#define DEFAULT_E810_IP "192.168.4.101"
#define DEFAULT_E810_PORT 8886
#define DEFAULT_SLAVE_ID 1
#define DEFAULT_REGISTER 0

typedef struct {
    const char* e810_ip;
    int e810_port;
    int slave_id;
    int register_addr;
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

static void print_header(const char* test_name) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("  %s\n", test_name);
    printf("═══════════════════════════════════════════════════════\n");
}

static void print_result(bool success, const char* message) {
    printf("  [%s] %s\n", success ? "✓" : "✗", message);
}

static int run_ping_test(const TestConfig* config) {
    print_header("PING Test");
    
    printf("  Target: %s\n", config->e810_ip);
    printf("  Timeout: %d seconds\n\n", config->timeout);
    
    bool result = ping_host(config->e810_ip, config->timeout);
    
    print_result(result, 
                 result ? "E810-DTU is reachable" : "E810-DTU is NOT reachable");
    
    if (!result) {
        printf("\n  Troubleshooting:\n");
        printf("    - Check Ethernet cable is connected\n");
        printf("    - Verify E810-DTU is powered on\n");
        printf("    - Check IP address is correct\n");
        printf("    - Verify both devices on same subnet\n");
    }
    
    return result ? 0 : 1;
}

static int run_tcp_test(const TestConfig* config) {
    print_header("TCP Connection Test");
    
    printf("  Target: %s:%d\n", config->e810_ip, config->e810_port);
    printf("  Timeout: %d seconds\n\n", config->timeout);
    
    NetworkConfig net_config = {
        .host = config->e810_ip,
        .port = config->e810_port,
        .timeout_seconds = config->timeout
    };
    
    NetworkResult result = test_tcp_conn(&net_config);
    bool success = (result == NET_SUCCESS);
    
    print_result(success, network_msg(result));
    
    if (!success) {
        printf("\n  Troubleshooting:\n");
        printf("    - Verify E810-DTU web interface is accessible\n");
        printf("    - Check port %d is configured in E810-DTU\n", config->e810_port);
        printf("    - Ensure no firewall blocking connection\n");
    }
    
    return success ? 0 : 1;
}

static int run_modbus_test(const TestConfig* config) {
    print_header("Modbus Read Test");
    
    printf("  Target: %s:%d\n", config->e810_ip, config->e810_port);
    printf("  Slave ID: %d\n", config->slave_id);
    printf("  Register: %d\n\n", config->register_addr);
    
    ModbusClient* client = modbus_create();
    if (!client) {
        print_result(false, "Failed to create Modbus client");
        return 1;
    }
    
    if (!modbus_client_conn(client, config->e810_ip, config->e810_port, config->timeout)) {
        print_result(false, "Failed to connect to Modbus device");
        modbus_destroy(client);
        return 1;
    }
    
    uint16_t value;
    bool success = modbus_read_register(client, config->slave_id, config->register_addr, &value);
    
    if (success) {
        printf("  [✓] Register %d = %u (0x%04X)\n", 
               config->register_addr, value, value);
        printf("\n  Success! E810-DTU is working correctly.\n");
    } else {
        print_result(false, "Failed to read register");
        printf("\n  Troubleshooting:\n");
        printf("    - Verify slave ID matches your meter\n");
        printf("    - Check register address is valid\n");
        printf("    - Ensure Modbus TCP→RTU is enabled in E810-DTU\n");
        printf("    - Check RS485 wiring to meter\n");
    }
    
    modbus_client_disconn(client);
    modbus_destroy(client);
    
    return success ? 0 : 1;
}

int main(int argc, char* argv[]) {
    TestConfig config = {
        .e810_ip = DEFAULT_E810_IP,
        .e810_port = DEFAULT_E810_PORT,
        .slave_id = DEFAULT_SLAVE_ID,
        .register_addr = DEFAULT_REGISTER,
        .timeout = 5,
        .verbose = false,
        .test_ping = false,
        .test_tcp = false,
        .test_modbus = false
    };
    
    // Parse command-line options
    static struct option long_options[] = {
        {"ip",       required_argument, 0, 'i'},
        {"port",     required_argument, 0, 'p'},
        {"slave",    required_argument, 0, 's'},
        {"register", required_argument, 0, 'r'},
        {"timeout",  required_argument, 0, 't'},
        {"verbose",  no_argument,       0, 'v'},
        {"ping",     no_argument,       0,  1 },
        {"tcp",      no_argument,       0,  2 },
        {"modbus",   no_argument,       0,  3 },
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "i:p:s:r:t:vh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                config.e810_ip = optarg;
                break;
            case 'p':
                config.e810_port = atoi(optarg);
                break;
            case 's':
                config.slave_id = atoi(optarg);
                break;
            case 'r':
                config.register_addr = atoi(optarg);
                break;
            case 't':
                config.timeout = atoi(optarg);
                break;
            case 'v':
                config.verbose = true;
                break;
            case 1:
                config.test_ping = true;
                break;
            case 2:
                config.test_tcp = true;
                break;
            case 3:
                config.test_modbus = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // If no specific tests selected, run all
    if (!config.test_ping && !config.test_tcp && !config.test_modbus) {
        config.test_ping = true;
        config.test_tcp = true;
        config.test_modbus = true;
    }
    
    // Print configuration
    printf("E810-DTU Connection Test Tool v%s\n", VERSION);
    printf("───────────────────────────────────────────────────────\n");
    printf("Configuration:\n");
    printf("  E810-DTU IP:   %s\n", config.e810_ip);
    printf("  E810-DTU Port: %d\n", config.e810_port);
    printf("  Slave ID:      %d\n", config.slave_id);
    printf("  Register:      %d\n", config.register_addr);
    printf("  Timeout:       %d seconds\n", config.timeout);
    printf("───────────────────────────────────────────────────────\n");
    
    int failures = 0;
    
    // Run tests
    if (config.test_ping) {
        failures += run_ping_test(&config);
    }
    
    if (config.test_tcp) {
        failures += run_tcp_test(&config);
    }
    
    if (config.test_modbus) {
        failures += run_modbus_test(&config);
    }
    
    // Summary
    printf("\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("  Test Summary\n");
    printf("═══════════════════════════════════════════════════════\n");
    
    if (failures == 0) {
        printf("  ✓ All tests passed!\n");
        printf("  E810-DTU is ready for meter reading.\n");
    } else {
        printf("  ✗ %d test(s) failed\n", failures);
        printf("  Please fix the issues above before proceeding.\n");
    }
    
    printf("═══════════════════════════════════════════════════════\n");
    
    return (failures == 0) ? 0 : 1;
}