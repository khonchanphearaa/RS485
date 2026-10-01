-- ============================================================
-- Meter Reader Database Schema
-- ============================================================

-- Use the database
USE meter_db;

-- Create meter_readings table (time-series data)
CREATE TABLE IF NOT EXISTS meter_readings (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    meter_id VARCHAR(50) NOT NULL COMMENT 'Unique meter identifier',
    value DECIMAL(12,4) NOT NULL COMMENT 'Reading value in engineering units',
    timestamp DATETIME NOT NULL COMMENT 'Reading timestamp',
    unit VARCHAR(10) NOT NULL COMMENT 'Measurement unit (m³, kWh, etc.)',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Record creation time',
    
    -- Indexes for efficient queries
    INDEX idx_meter_time (meter_id, timestamp),
    INDEX idx_timestamp (timestamp),
    INDEX idx_meter (meter_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
  COMMENT='Meter reading time-series data';

-- Create meters metadata table (optional, for documentation)
CREATE TABLE IF NOT EXISTS meters (
    meter_id VARCHAR(50) PRIMARY KEY,
    meter_type ENUM('water', 'electric', 'gas') NOT NULL COMMENT 'Type of meter',
    location VARCHAR(255) COMMENT 'Physical location description',
    gateway_ip VARCHAR(45) COMMENT 'E810-DTU gateway IP address',
    gateway_port INT COMMENT 'E810-DTU gateway port',
    slave_id INT COMMENT 'Meter RS485 slave ID',
    register_address INT COMMENT 'Modbus register address',
    multiplier DECIMAL(10,6) COMMENT 'Raw value to engineering units multiplier',
    unit VARCHAR(10) COMMENT 'Measurement unit',
    installed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'Installation date',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    INDEX idx_type (meter_type),
    INDEX idx_location (location)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
  COMMENT='Meter metadata and configuration';

-- Create view for daily aggregations (optional, for reporting)
CREATE OR REPLACE VIEW daily_readings AS
SELECT 
    meter_id,
    DATE(timestamp) AS reading_date,
    MIN(value) AS min_value,
    MAX(value) AS max_value,
    MAX(value) - MIN(value) AS daily_consumption,
    unit,
    COUNT(*) AS reading_count
FROM meter_readings
GROUP BY meter_id, DATE(timestamp), unit
ORDER BY meter_id, reading_date DESC;

-- Insert example meter metadata (update with your actual meter info)
INSERT INTO meters (
    meter_id, 
    meter_type, 
    location, 
    gateway_ip, 
    gateway_port, 
    slave_id, 
    register_address, 
    multiplier, 
    unit
) VALUES (
    'WATER_001',           -- meter_id
    'water',               -- meter_type
    'Main building inlet', -- location
    '192.168.4.101',       -- gateway_ip
    8886,                  -- gateway_port
    1,                     -- slave_id (check your meter's manual)
    0,                     -- register_address (check your meter's datasheet)
    0.001,                 -- multiplier (raw × 0.001 = m³)
    'm³'                   -- unit
)
ON DUPLICATE KEY UPDATE 
    meter_type = VALUES(meter_type),
    location = VALUES(location),
    gateway_ip = VALUES(gateway_ip),
    gateway_port = VALUES(gateway_port),
    slave_id = VALUES(slave_id),
    register_address = VALUES(register_address),
    multiplier = VALUES(multiplier),
    unit = VALUES(unit);

-- Grant permissions to meter_user (if not already done)
-- Note: Run this as root or admin user
-- GRANT SELECT, INSERT ON meter_db.meter_readings TO 'meter_user'@'%';
-- GRANT SELECT ON meter_db.meters TO 'meter_user'@'%';
-- GRANT SELECT ON meter_db.daily_readings TO 'meter_user'@'%';
-- FLUSH PRIVILEGES;

-- ============================================================
-- Example Queries (for testing)
-- ============================================================

-- View latest 10 readings
-- SELECT * FROM meter_readings ORDER BY timestamp DESC LIMIT 10;

-- View today's readings
-- SELECT * FROM meter_readings WHERE DATE(timestamp) = CURDATE() ORDER BY timestamp;

-- View daily consumption
-- SELECT * FROM daily_readings WHERE meter_id = 'WATER_001' LIMIT 30;

-- View total consumption this month
-- SELECT SUM(daily_consumption) AS total_consumption, unit
-- FROM daily_readings 
-- WHERE meter_id = 'WATER_001' 
--   AND MONTH(reading_date) = MONTH(CURDATE())
--   AND YEAR(reading_date) = YEAR(CURDATE());