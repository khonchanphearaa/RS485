#ifndef METER_READER_METER_SERVICE_HPP
#define METER_READER_METER_SERVICE_HPP

#include "m_reader/types.hpp"
#include "m_reader/database.hpp"

namespace meter_reader {

/**
 * @brief High-level meter reading service
 * 
 * Orchestrates Modbus communication, validation, and database storage.
 * This is the main service class used by main.cpp
 */
class MeterService {
public:
    /**
     * @brief Create meter service
     * @param config Meter configuration
     * @param db Database connection (must be connected)
     */
    MeterService(const m_reader::MeterConfig& config, Database& db);
    ~MeterService();
    
    /**
     * @brief Read meter, validate, and save to database
     * 
     * This is the main polling function called every N seconds.
     * Handles all errors internally and logs appropriately.
     */
    void readAndSave();
    
private:
    m_reader::MeterConfig config_;
    Database& db_;
    void* modbus_client_;  // ModbusTcpClient* (opaque pointer)
    double last_stored_value_;
    
    /**
     * @brief Convert raw register value to engineering units
     */
    double toEngineeringUnits(uint16_t raw_value) const;
};

} // namespace meter_reader

#endif // METER_READER_METER_SERVICE_HPP