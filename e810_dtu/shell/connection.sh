#!/bin/bash
#
# E810-DTU Full Connection Test Script
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
E810_IP="${E810_IP:-192.168.4.101}"
E810_PORT="${E810_PORT:-8886}"
TIMEOUT="${TIMEOUT:-5}"

echo "----------------------------------------"
echo "  E810-DTU Connection Test"
echo "----------------------------------------"
echo ""
echo "Configuration:"
echo "  E810-DTU IP:   $E810_IP"
echo "  E810-DTU Port: $E810_PORT"
echo "  Timeout:       $TIMEOUT seconds"
echo ""

# Function to print colored output
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}[✓]${NC} $2"
    else
        echo -e "${RED}[✗]${NC} $2"
    fi
}

# Test 1: Check network interface
echo "Test 1: Network Interface"
echo "───────────────────────────────────────────────────────"
if ip addr show | grep -q "inet.*$(echo $E810_IP | cut -d. -f1-3)"; then
    print_status 0 "Network interface is on same subnet"
else
    print_status 1 "Network interface may not be on same subnet"
    echo "  Your MinIPC IP should be 192.168.4.X (same as E810-DTU)"
fi
echo ""

# Test 2: Ping test
echo "Test 2: Ping Test"
echo "───────────────────────────────────────────────────────"
if ping -c 1 -W $TIMEOUT $E810_IP > /dev/null 2>&1; then
    print_status 0 "E810-DTU is reachable via ICMP"
else
    print_status 1 "E810-DTU is NOT reachable via ICMP"
    echo "  Troubleshooting:"
    echo "    - Check Ethernet cable"
    echo "    - Verify E810-DTU is powered on"
    echo "    - Check IP address configuration"
fi
echo ""

# Test 3: TCP port test
echo "Test 3: TCP Port Test"
echo "───────────────────────────────────────────────────────"
if nc -zv -w $TIMEOUT $E810_IP $E810_PORT 2>&1 | grep -q "succeeded"; then
    print_status 0 "TCP port $E810_PORT is open"
else
    print_status 1 "TCP port $E810_PORT is NOT accessible"
    echo "  Troubleshooting:"
    echo "    - Verify E810-DTU web interface"
    echo "    - Check port configuration in E810-DTU"
    echo "    - Ensure no firewall blocking"
fi
echo ""

# Test 4: Web interface
echo "Test 4: Web Interface (Optional)"
echo "───────────────────────────────────────────────────────"
if command -v curl > /dev/null 2>&1; then
    if curl -s -o /dev/null -w "%{http_code}" --connect-timeout $TIMEOUT http://$E810_IP | grep -q "200\|302"; then
        print_status 0 "Web interface is accessible"
        echo "  URL: http://$E810_IP"
    else
        print_status 1 "Web interface is NOT accessible"
    fi
else
    echo -e "${YELLOW}[!]${NC} curl not installed, skipping web test"
fi
echo ""

# Test 5: Run C test tool
echo "Test 5: Modbus Read Test"
echo "───────────────────────────────────────────────────────"
if [ -f "bin/test-e810" ]; then
    echo "Running test-e810 tool..."
    ./bin/test-e810 --modbus -i $E810_IP -p $E810_PORT
else
    echo -e "${YELLOW}[!]${NC} test-e810 not built, run 'make' first"
fi
echo ""

echo "----------------------------------------"
echo "  Test Complete"
echo "----------------------------------------"