#ifndef METER_READER_VALIDATOR_HPP
#define METER_READER_VALIDATOR_HPP

#include "m_reader/types.hpp"
#include <optional>
#include <chrono>

namespace meter_reader {


class Validator {
public:
    explicit Validator(const m_reader::MeterConfig& config);
    
    std::optional<std::string> validate(
        double value,
        std::chrono::system_clock::time_point timestamp
    );
    
    bool shouldStore(double value);
    
private:
    m_reader::MeterConfig config_;
    double last_stored_value_;
    std::chrono::system_clock::time_point last_valid_timestamp_;
    bool has_previous_reading_;
    
    bool checkRange(double value) const;
    bool checkRateOfChange(double value, std::chrono::system_clock::time_point now);
    bool checkMonotonic(double value);
};

} // namespace meter_reader

#endif // METER_READER_VALIDATOR_HPP