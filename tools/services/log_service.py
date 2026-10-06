from multiprocessing import set_forkserver_preload
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