#include "m_reader/meter_reader.hpp"
#include "m_reader/modbus_tcp.hpp"
#include "m_reader/validator.hpp"
#include <iostream>
#include <chrono>

namespace meter_reader {

MeterService::MeterService(const m_reader::MeterConfig& config, Database& db)
    : config_(config)
    , db_(db)
    , modbus_client_(nullptr)
    , last_stored_value_(0.0)
{
    modbus_client_ = modbus_tcp_create();
    
    if (!modbus_client_) {
        throw std::runtime_error("Failed to create Modbus client");
    }
    
    if (!modbus_tcp_connect(static_cast<ModbusTcpClient*>(modbus_client_), config_.gateway_ip.c_str(), config_.gateway_port)) {
        throw std::runtime_error("Failed to connect to Modbus gateway");
    }
}

MeterService::~MeterService() {
    if (modbus_client_) {
        modbus_tcp_disconnect(static_cast<ModbusTcpClient*>(modbus_client_));
        modbus_tcp_destroy(static_cast<ModbusTcpClient*>(modbus_client_));
        modbus_client_ = nullptr;
    }
}

void MeterService::readAndSave() {
    try {
        uint16_t raw_value;
        if (!modbus_tcp_read_register(
                static_cast<ModbusTcpClient*>(modbus_client_),
                config_.meter_slave_id,
                config_.register_address,
                &raw_value)) {
            std::cerr << "Failed to read from meter" << std::endl;
            return;
        }
        
        double value = toEngineeringUnits(raw_value);
        auto timestamp = std::chrono::system_clock::now();
        
        Validator validator(config_);
        auto error = validator.validate(value, timestamp);
        
        if (error.has_value()) {
            std::cerr << "Validation failed: " << error.value() << std::endl;
            return;
        }
        
        if (!validator.shouldStore(value)) {
            std::cout << "Value unchanged (within deadband), skipping" << std::endl;
            return;
        }
        
        if (db_.saveReading(config_.meter_id, value, timestamp, config_.unit)) {
            std::cout << "Saved: " << config_.meter_id << " = " << value << " " << config_.unit << std::endl;
            last_stored_value_ = value;
        } else {
            std::cerr << "Failed to save to database" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error in readAndSave: " << e.what() << std::endl;
    }
}

double MeterService::toEngineeringUnits(uint16_t raw_value) const {
    return static_cast<double>(raw_value) * config_.multiplier;
}

} // namespace meter_reader