#!/bin/bash
set -euo pipefail

echo "Installing Meter Reader..."

# Create system user
sudo useradd --system --no-create-home --shell /bin/false meter-reader || true

# Build
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Install
sudo cmake --install .

# Create config directory
sudo mkdir -p /etc/meter-reader
sudo cp ../config/meter_config.example.json /etc/meter-reader/meter_config.json

# Set permissions
sudo chown -R meter-reader:meter-reader /etc/meter-reader

# Install systemd service
sudo cp ../scripts/meter-reader.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable meter-reader

echo "Installation complete!"
echo "Edit /etc/meter-reader/meter_config.json with your settings"
echo "Start service: sudo systemctl start meter-reader"