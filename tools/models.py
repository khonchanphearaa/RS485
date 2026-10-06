
from multiprocessing import set_forkserver_preload
from dataclasses import dataclass
from datetime import datetime
from typing import Optional, List
from enum import Enum


class MeterType(str, Enum):
    WATER = "water"
    ELEC = "electric"
    # GAS = "gas"

@dataclass
class MeterReading:
    id: int
    meter_id: str
    value: float
    unit: str
    timestamp: datetime
    created_at: datetime

    def to_dict(self) -> dict:
        
        # convert to dictionary (JSON)
        return{
            "id": self.id,
            "meter_id": self.meter_id,
            "value": self.value,
            "unit": self.unit,
            "timestamp": self.timestamp.isoformat(),
            "created_at": self.crate_at.isoformat()
        }


@dataclass
class MeterStatus:
    meter_id: str
    meter_type: MeterType
    last_reading: Optional[MeterReading]
    minutes_since_last: Optional[int]
    status: str     # OK, STALE, OFFLINE

    def healtz(self) -> bool:
        return self.status == "OK"

