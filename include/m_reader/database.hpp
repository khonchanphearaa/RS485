#ifndef METER_READER_DATABASE_HPP
#define METER_READER_DATABASE_HPP

#include "m_reader/types.hpp"
#include <string>
#include <chrono>

namespace meter_reader {

/**
 * @brief MySQL database connection and operations
 * 
 * Uses MySQL Connector/C++ for database access
 * Thread-safe: each instance manages its own connection
 * define with running dirver on docker-images
 */
class Database {
public:
    Database();
    ~Database();
    

    bool connect(const m_reader::DatabaseConfig& config);    
    bool saveReading(
        const std::string& meter_id,
        double value,
        std::chrono::system_clock::time_point timestamp,
        const std::string& unit
    );
    
    bool isConnected() const { return connected_; }
    void disconnect();
    
private:
    void* sql_driver_;
    void* sql_connection_;
    bool connected_;
    

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
};

} // namespace meter_reader

#endif // METER_READER_DATABASE_HPP