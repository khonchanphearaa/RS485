#ifndef METER_READER_TYPES_HPP
#define METER_READER_TYPES_HPP

#include <string>

namespace m_reader {

struct MeterConfig {
    std::string meter_id;
    std::string gateway_ip;
    int gateway_port;
    unsigned char meter_slave_id;
    unsigned short register_address;
    double multiplier;
    std::string unit;
    int polling_interval_seconds;
    double deadband_threshold;
    double max_value;
    double max_rate_of_change;
};

struct DatabaseConfig {
    std::string host;
    int port;
    std::string database;
    std::string user;
    std::string password;
};

struct AppConfig {
    MeterConfig meter;
    DatabaseConfig database;
    std::string log_level;
};

} // namespace m_reader

#endif