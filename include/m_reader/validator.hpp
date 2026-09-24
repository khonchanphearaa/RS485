#ifndef METER_READER_VALIDATOR_HPP
#define METER_READER_VALIDATOR_HPP

#include "m_reader/types.hpp"
#include <optional>
#include <chrono>

namespace meter_reader {

/**
 * @brief Data validation service
 * 
 * Validates meter readings for anomalies, outliers, and physical limits.
 */
class Validator {
public:
    explicit Validator(const m_reader::MeterConfig& config);
    
    /**
     * @brief Validate a reading
     * @param value Reading value in engineering units
     * @param timestamp Reading timestamp
     * @return std::nullopt if valid, error message if invalid
     */
    std::optional<std::string> validate(
        double value,
        std::chrono::system_clock::time_point timestamp
    );
    
    /**
     * @brief Check if value should be saved (deadband filter)
     * @param value Current reading
     * @return true if should save, false if within deadband
     */
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