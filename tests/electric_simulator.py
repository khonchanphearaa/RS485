#!/usr/bin/env python3
"""
Modbus TCP Electric Meter Simulator
Simulates energy consumption (kWh) for testing C++ meter_reader
"""

import asyncio
import logging
from pymodbus import ModbusDeviceIdentification
from pymodbus.server import ModbusTcpServer
from pymodbus.simulator.simdata import DataType, SimData
from pymodbus.simulator.simdevice import SimDevice

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class ElectricMeterSimulator:
    def __init__(self, initial_value: int = 125000, increment: int = 100, update_interval: int = 60):
        """
        Electric meter simulator.
        
        Args:
            initial_value: Initial register value (x0.01 = kWh)
                         Default: 125000 = 1250.00 kWh
            increment: Increment per update (x0.01 = kWh)
                      Default: 100 = 1.00 kWh per minute
            update_interval: Seconds between increments
        """
        self.initial_value = initial_value
        self.current_value = initial_value
        self.increment = increment
        self.update_interval = update_interval

    async def _update_task(self, server: ModbusTcpServer, slave_id: int):
        """Periodically increments the electric meter reading in the background."""
        while True:
            await asyncio.sleep(self.update_interval)
            self.current_value += self.increment
            await server.context.async_setValues(slave_id, 3, 0, [self.current_value])
            logger.info(
                f"[Simulated Update] Register 0 incremented to: {self.current_value} "
                f"({self.current_value * 0.01:.2f} kWh) "
                f"[+{self.increment * 0.01:.2f} kWh]"
            )

    async def run(self, host: str = "0.0.0.0", port: int = 8887, slave_id: int = 1):
        sim_data = SimData(
            address=0,
            values=[self.current_value] + [0] * 99,
            datatype=DataType.REGISTERS
        )
        sim_device = SimDevice(slave_id, simdata=sim_data)


        identity = ModbusDeviceIdentification()
        identity.VendorName = "Electric Meter Simulator"
        identity.ProductCode = "EM-001"
        identity.ModelName = "Virtual Energy Meter"
        identity.MajorMinorRevision = "1.0"

        logger.info("-" * 60)
        logger.info("Modbus TCP Electric Meter Simulator")
        logger.info("-" * 60)
        logger.info(f"Listening on: {host}:{port}")
        logger.info(f"Slave ID: {slave_id}")
        logger.info(f"Register 0: {self.current_value} ({self.current_value * 0.01:.2f} kWh)")
        logger.info(f"Increment: +{self.increment} ({self.increment * 0.01:.2f} kWh) every {self.update_interval}s")
        logger.info("-" * 60)
        logger.info("")
        logger.info("Your C++ app should connect to:")
        logger.info(f"  gateway_ip: {host}")
        logger.info(f"  gateway_port: {port}")
        logger.info(f"  meter_slave_id: {slave_id}")
        logger.info(f"  register_address: 0")
        logger.info(f"  multiplier: 0.01")
        logger.info(f"  unit: kWh")
        logger.info("")
        logger.info("Press Ctrl+C to stop")
        logger.info("-" * 60)

        # create Modbus TCP server
        server = ModbusTcpServer(
            context=sim_device,
            address=(host, port),
            identity=identity
        )

        asyncio.create_task(self._update_task(server, slave_id))
        await server.serve_forever()


async def main():
    simulator = ElectricMeterSimulator(initial_value=125000, increment=100, update_interval=60)
    try:
        await simulator.run(host="0.0.0.0", port=8887, slave_id=1)
    except (KeyboardInterrupt, asyncio.CancelledError):
        logger.info("\nSimulator stopped")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("\nSimulator stopped")