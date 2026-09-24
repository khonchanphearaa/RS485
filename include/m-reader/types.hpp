#ifndef METER_READER_TYPES_HPP
#define METER_READER_TYPES_HPP


#include <cstdint>
#include <string>
#include <chrono>

namespace m_reader {
    
    /*
    define types struct meter_reading data strcuture
    */
    struct Reading {
        std::string meter_id;
        double value;
        std::chrono::system_clock::time_point timestamp;
        std::string units;
    };


    struct MeterConfig {
        std::string meter_id;
        std::string gateway_ip;
        int gateway_port;
        uint8_t meter_slave_id;
        uint16_t register_address;
        double multiplier;
        std::string units;
        int polling_interval_second;
        double deadband_threshold;
        double max_value;
        double max_rate_of_change;
    };


    /*
    * Database info configs
    */
    struct DatabaseConfig {
        std::string host;
        int port;
        std::string db_name;
        std::string username;
        std::string db_password;
    };


    struct AppConfig {
        MeterConfig meter;
        DatabaseConfig database;
        std::string log_level;
    };

}// namespace m_reader

#endif
