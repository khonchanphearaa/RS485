#ifndef METER_READER_METER_SERVICE_HPP
#define METER_READER_METER_SERVICE_HPP

#include "m_reader/types.hpp"
#include "m_reader/database.hpp"

namespace meter_reader {

class MeterService {
public:

    MeterService(const m_reader::MeterConfig& config, Database& db);
    ~MeterService();
    
    void readAndSave();
    
private:
    m_reader::MeterConfig config_;
    Database& db_;
    void* modbus_client_;
    double last_stored_value_;

    double toEngineeringUnits(uint16_t raw_value) const;
};

}

#endif // METER_READER_METER_SERVICE_HPP