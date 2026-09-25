#ifndef METER_READER_CONFIG_HPP
#define METER_READER_CONFIG_HPP

#include "m_reader/types.hpp"
#include <string>

namespace meter_reader {

m_reader::AppConfig load_config(const std::string& config_path);

}

#endif // METER_READER_CONFIG_HPP