import time
import logging
from datetime import datetime
from typing import Callable, List


from .meter_service import MeterService
from ..db import Database
from ..models import MeterStatus
from ..config import Config

logger = logging.getLogger(__name__)


class LogMonitor:

    def __int__(self, db: Database, on_alert: Callable[[MeterStatus], None] | None = None):
        
        self.db = db
        self.meter_service = MeterService(db)
        self.on_alert = on_alert
        self.running = False

    
    def get_all_meter_status(self) -> List[MeterStatus]:
        
        query = "SELECT DISTINCT meter_id FROM meters"
        
        with self.db.get_cursor() as cursor:
            cursor.execute(query)
            meter_ids = [row['meter_id'] for row in cursor.fetchall()]

        # get status for each meter
        statuses = []
        for meter_id in meter_ids:
            status = self.meter_service.get_meter_status(
                meter_id,
                stale_threshold_minutes=Config.STALE_THRESHOLD_MINUTES
            )
            statuses.append(status)

        return statuses


    def log_status(self, statuses: List[MeterStatus]):
        
        logger.info("-" * 60)
        logger.info(f"Meter heath check - {datetime.now}")
        logger.info("-" * 60)

        for status in statuses:
            emoji = "✅" if status.healtz() else "⚠️" 
            last_value = status.last_reading.value if status.last_reading else "N/A"

            last_unit = status.last_reading.unit if status.last_reading else ""

            logger.info(
                f"{emoji} {status.meter_id:<20} | "
                f"Status: {status.status:<8} | "
                f"Last: {last_value} {last_unit} | "
                f"{status.minutes_since_last or 0} min ago"
            )

        logger.info("-" * 60)

    
    def check_alerts(self, statuses: List[MeterStatus]):
        for status in statuses:
            if not status.healtz() and self.on_alert:
                self.on_alert(status)
    

    def run(self, interval_seconds: int):
        interval = interval_seconds or Config.CHECK_INTERVAL_SECONDS
        self.running = True

        logger.info(f"status meter monitor [interval: {interval_seconds}s]")

        try:
            while self.running:
                statuses = self.get_all_meter_status()
                self.log_status(statuses)
                self.check_alerts(statuses)
                
                time.sleep(interval)
        except KeyboardInterrupt:
            logger.info("monitor stopped by user")

        finally: 
            self.running = False


    def stop(self):
        self.running = False

