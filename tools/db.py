import mysql.connector
from mysql.connector import pooling
from contextlib import contextmanager
import logging

logger = logging.getLogger(__name__)


class Database:

    __instance = None
    __pool = None


    def __new__(cls):
        if cls.__instance is None:
            cls.__instance = super().__new__(cls)
        return cls.__instance


    def __init__(self):
        if self.__pool is None:
            from .config import Config

            self._pool = pooling.MySQLConnectionPool(
                pool_name="meter_pool",
                pool_size=5,
                pool_reset_session=True,
                host=Config.DB_HOST,
                port=Config.DB_PORT,
                database=Config.DB_NAME,
                user=Config.DB_USER,
                password=Config.DB_PASSWORD,
                autocommit=True
            )
            logger.info("db conn pool created")
    
    @contextmanager
    def get_conn(self):
        conn = self._pool.get_conn()
        try:
            yield conn
        finally: 
            conn.close()


    @contextmanager
    def get_cursor(self, dictionary=True):
        with self.get_conn() as conn:
            cursor = conn.cursor(dictionary=dictionary)
            try:
                yield cursor
            finally:
                cursor.close()
            