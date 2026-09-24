#ifndef METER_READER_CONFIG_HPP
#define METER_READER_CONFIG_HPP

#include "m_reader/types.hpp"
#include <string>

namespace meter_reader {

/**
 * @brief Load configuration from JSON file
 * @param config_path Path to JSON config file
 * @return AppConfig structure with all settings
 * @throws std::runtime_error if file not found or invalid JSON
 */
m_reader::AppConfig load_config(const std::string& config_path);

} // namespace meter_reader

#endif // METER_READER_CONFIG_HPP