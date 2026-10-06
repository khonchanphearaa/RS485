
import os
from pathlib import Path
from dotenv import load_dotenv

load_dotenv()


class Config:
    
    DB_HOST: str = os.getenv("DB_HOST", "")
    DB_PORT: int = int(os.getenv("DB_PORT", ""))
    DB_NAME: str = os.getenv("DB_NAME", "")
    DB_USER: str = os.getenv("DB_USER", "")
    DB_PASSWORD: str = os.getenv("DB_PASSWORD", "")

    
    # monitor check duration minutes real-time
    CHECK_INTERVAL_SECONDS: int = int(os.getenv("CHECK_INTERVAL", "60"))
    STALE_THRESHOLD_MINUTES: int = int(os.getenv("STALE_THRESHOLD", "15"))


    # alert msg
    ALERT_EMAIL: str = os.getenv("ALERT_EMAIL", "")
    ALERT_WEBHOOK_URL: str = os.getenv("ALERT_WEBHOOK_URL", "")


    @classmethod
    def validate(cls) -> bool:
        if not cls.DB_PASSWORD:
            raise ValueError("DB_PASSWORD env is required")
        return True