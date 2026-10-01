#!/usr/bin/env python3
"""
Modbus TCP Server Simulator for testing C++ meter_reader
Runs on port 8886
"""

import asyncio
import logging
from pymodbus import ModbusDeviceIdentification
from pymodbus.server import ModbusTcpServer
from pymodbus.simulator.simdata import DataType, SimData
from pymodbus.simulator.simdevice import SimDevice

# Config logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class WaterMeterSimulator:
    def __init__(self, initial_value: int = 2600, increment: int = 50, update_interval: int = 60):
        self.initial_value = initial_value
        self.current_value = initial_value
        self.increment = increment
        self.update_interval = update_interval

    async def _update_task(self, server: ModbusTcpServer, slave_id: int):
        """Periodically increments the water meter reading in the background."""
        while True:
            await asyncio.sleep(self.update_interval)
            self.current_value += self.increment
            await server.context.async_setValues(slave_id, 3, 0, [self.current_value])
            logger.info(
                f"[Simulated Update] Register 0 incremented to: {self.current_value} "
                f"({self.current_value * 0.001:.3f} m³)"
            )

    async def run(self, host: str = "0.0.0.0", port: int = 8886, slave_id: int = 1):
        # Register 0 holding register: current value + padding
        sim_data = SimData(
            address=0,
            values=[self.current_value] + [0] * 99,
            datatype=DataType.REGISTERS
        )
        sim_device = SimDevice(slave_id, simdata=sim_data)

        # Device identification
        identity = ModbusDeviceIdentification()
        identity.VendorName = "water_meter simulator"
        identity.ProductCode = "WM-001"
        identity.ModelName = "virtual water meter"
        identity.MajorMinorRevision = "1.0"

        logger.info("-" * 60)
        logger.info("Modbus TCP water meter simulator")
        logger.info("-" * 60)
        logger.info(f"Listening on: {host}:{port}")
        logger.info(f"Slave ID: {slave_id}")
        logger.info(f"Register 0: {self.current_value} ({self.current_value * 0.001:.3f} m³)")
        logger.info(f"Increment: +{self.increment} every {self.update_interval} seconds")
        logger.info("-" * 60)
        logger.info("")
        logger.info("Your C++ app should connect to:")
        logger.info(f"  gateway_ip: {host}")
        logger.info(f"  gateway_port: {port}")
        logger.info(f"  meter_slave_id: {slave_id}")
        logger.info(f"  register_address: 0")
        logger.info("")
        logger.info("Press Ctrl+C to stop")
        logger.info("-" * 60)

        # Create Modbus TCP server instance
        server = ModbusTcpServer(
            context=sim_device,
            address=(host, port),
            identity=identity
        )

        # Start periodic increment task in the background
        asyncio.create_task(self._update_task(server, slave_id))

        # Start serving requests
        await server.serve_forever()


async def main():
    simulator = WaterMeterSimulator(initial_value=2600, increment=50, update_interval=60)
    try:
        await simulator.run(host="0.0.0.0", port=8886, slave_id=1)
    except (KeyboardInterrupt, asyncio.CancelledError):
        logger.info("\nSimulator stopped")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("\nSimulator stopped")