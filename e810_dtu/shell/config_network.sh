#!/bin/bash
#
# Configure MinIPC network for direct E810-DTU connection
# Senior-level: Safe, documented, reversible
#

set -e

# Configuration
INTERFACE="${INTERFACE:-eth0}"
STATIC_IP="192.168.4.100"
NETMASK="255.255.255.0"
GATEWAY="192.168.4.1"

echo "-----------------------------------------------------"
echo "  Configure Network for Direct E810-DTU Connection"
echo "-----------------------------------------------------"
echo ""
echo "This will configure:"
echo "  Interface: $INTERFACE"
echo "  IP:        $STATIC_IP"
echo "  Netmask:   $NETMASK"
echo ""

# Backup current configuration
echo "Backing up current network configuration..."
if [ -f /etc/network/interfaces.bak ]; then
    echo "  Backup already exists: /etc/network/interfaces.bak"
else
    sudo cp /etc/network/interfaces /etc/network/interfaces.bak
    echo "  Backup created: /etc/network/interfaces.bak"
fi
echo ""

# Configure static IP
echo "Configuring static IP..."
sudo ifconfig $INTERFACE $STATIC_IP netmask $NETMASK up

# Verify configuration
echo ""
echo "Verifying configuration..."
ip addr show $INTERFACE

echo ""
echo "-----------------------------------------------------"
echo "  Configuration Complete"
echo "-----------------------------------------------------"
echo ""
echo "Your MinIPC IP: $STATIC_IP"
echo "E810-DTU IP:    192.168.4.101"
echo "Both on subnet: 192.168.4.0/24"
echo ""
echo "Test connection:"
echo "  ping 192.168.4.101"
echo "  ./bin/test-e810"