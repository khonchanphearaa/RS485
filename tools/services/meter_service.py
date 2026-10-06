
from typing import List, Optional
from datetime import datetime, timedelta
import logging

from ..db import Database
from ..models import MeterReading, MeterStatus, MeterType

logger = logging.getLogger(__name__)


class MeterService:

    
    def __init__(self, db: Database):
        self.db = db
    
    def get_latest_readings(self) -> List[MeterReading]:


        """Get latest reading for each meter."""
        query = """
            SELECT 
                r.id, r.meter_id, r.value, r.unit, r.timestamp, r.created_at
            FROM meter_readings r
            INNER JOIN (
                SELECT meter_id, MAX(timestamp) as max_ts
                FROM meter_readings
                GROUP BY meter_id
            ) latest ON r.meter_id = latest.meter_id 
                     AND r.timestamp = latest.max_ts
            ORDER BY r.meter_id
        """
        
        with self.db.get_cursor() as cursor:
            cursor.execute(query)
            rows = cursor.fetchall()
            
            return [
                MeterReading(
                    id=row['id'],
                    meter_id=row['meter_id'],
                    value=float(row['value']),
                    unit=row['unit'],
                    timestamp=row['timestamp'],
                    created_at=row['created_at']
                )
                for row in rows
            ]
    
    def get_meter_status(self, meter_id: str, stale_threshold_minutes: int = 15) -> MeterStatus:


        """Get health status for a specific meter."""
        query = """
            SELECT 
                m.meter_id, m.meter_type,
                r.value, r.unit, r.timestamp, r.created_at,
                TIMESTAMPDIFF(MINUTE, r.timestamp, NOW()) as minutes_ago
            FROM meters m
            LEFT JOIN meter_readings r ON m.meter_id = r.meter_id
            WHERE m.meter_id = %s
            ORDER BY r.timestamp DESC
            LIMIT 1
        """
        
        with self.db.get_cursor() as cursor:
            cursor.execute(query, (meter_id,))
            row = cursor.fetchone()
            
            if not row or not row['timestamp']:
                return MeterStatus(
                    meter_id=meter_id,
                    meter_type=MeterType.WATER,  # default
                    last_reading=None,
                    minutes_since_last=None,
                    status="OFFLINE"
                )
            
            minutes_ago = row['minutes_ago'] or 0
            status = "OK" if minutes_ago < stale_threshold_minutes else "STALE"
            
            last_reading = MeterReading(
                id=row['id'] or 0,
                meter_id=row['meter_id'],
                value=float(row['value']) if row['value'] else 0.0,
                unit=row['unit'] or "",
                timestamp=row['timestamp'],
                created_at=row['created_at']
            )
            
            return MeterStatus(
                meter_id=meter_id,
                meter_type=MeterType(row['meter_type']),
                last_reading=last_reading,
                minutes_since_last=minutes_ago,
                status=status
            )
    
    def get_hourly_consumption(self, meter_id: str, hours: int = 24) -> List[dict]:

        
        """Get hourly aggregated consumption."""
        query = """
            SELECT 
                DATE_FORMAT(timestamp, '%Y-%m-%d %H:00') AS hour,
                AVG(value) AS avg_value,
                unit
            FROM meter_readings
            WHERE meter_id = %s
              AND timestamp >= NOW() - INTERVAL %s HOUR
            GROUP BY DATE_FORMAT(timestamp, '%Y-%m-%d %H:00')
            ORDER BY hour
        """
        
        with self.db.get_cursor() as cursor:
            cursor.execute(query, (meter_id, hours))
            return cursor.fetchall()