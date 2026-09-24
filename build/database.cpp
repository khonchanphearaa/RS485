#include "m_reader/database.hpp"
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace meter_reader {

Database::Database()
    : sql_driver_(nullptr)
    , sql_connection_(nullptr)
    , connected_(false)
{
}

Database::~Database() {
    disconnect();
}

bool Database::connect(const m_reader::DatabaseConfig& config) {
    try {
        // Get driver instance (Debian cppconn API)
        sql::Driver* driver = get_driver_instance();

        if (!driver) {
            std::cerr << "Failed to get MySQL driver" << std::endl;
            return false;
        }

        // Build TCP connection URL
        std::ostringstream url;
        url << "tcp://" << config.host << ":" << config.port;

        // Connect with separate user/password (cppconn API)
        sql::Connection* conn = driver->connect(url.str(), config.user, config.password);

        if (!conn) {
            std::cerr << "Failed to connect to database" << std::endl;
            return false;
        }

        // Select database
        conn->setSchema(config.database);
        conn->setAutoCommit(true);

        sql_connection_ = conn;
        connected_ = true;
        std::cout << "Connected to MySQL at " << config.host << ":" << config.port << std::endl;
        return true;
        
    } catch (const sql::SQLException& e) {
        std::cerr << "MySQL error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

bool Database::saveReading(
    const std::string& meter_id,
    double value,
    std::chrono::system_clock::time_point timestamp,
    const std::string& unit
) {
    if (!connected_ || !sql_connection_) {
        std::cerr << "Database not connected" << std::endl;
        return false;
    }
    
    try {
        sql::Connection* conn = static_cast<sql::Connection*>(sql_connection_);
        
        // Create prepared statement
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "INSERT INTO meter_readings (meter_id, value, timestamp, unit) "
                "VALUES (?, ?, ?, ?)"
            )
        );
        
        // Convert timestamp to string
        auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
        std::tm tm_val;
        gmtime_r(&time_t_val, &tm_val);
        
        std::ostringstream ts_str;
        ts_str << std::put_time(&tm_val, "%Y-%m-%d %H:%M:%S");
        
        // Bind parameters
        stmt->setString(1, meter_id);
        stmt->setDouble(2, value);
        stmt->setString(3, ts_str.str());
        stmt->setString(4, unit);
        
        // Execute
        stmt->executeUpdate();
        
        return true;
        
    } catch (const sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

void Database::disconnect() {
    if (sql_connection_) {
        delete static_cast<sql::Connection*>(sql_connection_);
        sql_connection_ = nullptr;
        connected_ = false;
        std::cout << "Disconnected from MySQL" << std::endl;
    }
}

} // namespace meter_reader