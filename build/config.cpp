#include "m_reader/config.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace meter_reader {

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

static std::string extractValue(const std::string& json, const std::string& key) {
    std::string search_key = "\"" + key + "\"";
    size_t pos = json.find(search_key);
    if (pos == std::string::npos) {
        throw std::runtime_error("Key not found: " + key);
    }
    
    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        throw std::runtime_error("Invalid JSON format");
    }
    
    pos++;
    
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) {
        pos++;
    }
    
    if (pos >= json.size()) {
        throw std::runtime_error("Unexpected end of JSON");
    }
    
    if (json[pos] == '"') {
        pos++;
        size_t end = json.find('"', pos);
        if (end == std::string::npos) {
            throw std::runtime_error("Unterminated string");
        }
        return json.substr(pos, end - pos);
    }
    
    size_t end = pos;
    while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != '\n') {
        end++;
    }
    
    return trim(json.substr(pos, end - pos));
}

m_reader::AppConfig load_config(const std::string& config_path) {
    
    /* Read file */
    std::ifstream file(config_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + config_path);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    file.close();
    
    m_reader::AppConfig config;
    
    /* Parse meter config */
    config.meter.meter_id = extractValue(json, "meter_id");
    config.meter.gateway_ip = extractValue(json, "gateway_ip");
    config.meter.gateway_port = std::stoi(extractValue(json, "gateway_port"));
    config.meter.meter_slave_id = static_cast<unsigned char>(std::stoi(extractValue(json, "meter_slave_id")));
    config.meter.register_address = static_cast<unsigned short>(std::stoi(extractValue(json, "register_address")));
    config.meter.multiplier = std::stod(extractValue(json, "multiplier"));
    config.meter.unit = extractValue(json, "unit");
    config.meter.polling_interval_seconds = std::stoi(extractValue(json, "polling_interval_seconds"));
    config.meter.deadband_threshold = std::stod(extractValue(json, "deadband_threshold"));
    config.meter.max_value = std::stod(extractValue(json, "max_value"));
    config.meter.max_rate_of_change = std::stod(extractValue(json, "max_rate_of_change"));
    
    /* Parse database config */
    config.database.host = extractValue(json, "host");
    config.database.port = std::stoi(extractValue(json, "port"));
    config.database.database = extractValue(json, "database");
    config.database.user = extractValue(json, "user");
    config.database.password = extractValue(json, "password");
    
    /* logs info */
    try {
        config.log_level = extractValue(json, "log_level");
    } catch (...) {
        config.log_level = "INFO";
    }
    
    std::cout << "Loaded config for meter: " << config.meter.meter_id << std::endl;
    return config;
}

} // namespace meter_reader