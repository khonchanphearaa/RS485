#include "m_reader/validator.hpp"
#include <cmath>
#include <sstream>

namespace meter_reader {
    Validator::Validator(const m_reader::MeterConfig& config)
    : config_(config), last_stored_value_(0.0), last_valid_timestamp_(), has_previous_reading_(false)
    {}

    std::optional<std::string> Validator::validate(
        double value,
        std::chrono::system_clock::time_point timestamp
    ) {
        /* range check physical limits */
        if (!checkRange(value)) {
            std::ostringstream msg;
            msg << "value" << value << "exceeds maximum allowed" << config_.max_value;
            return msg.str();
        }

        /* rate-of-change check */
        if (!checkRateOfChange(value, timestamp)) {
            std::ostringstream msg;
            msg << "rate of change too high for value " << value;
            return msg.str();
        }

        /* monotoic check  (cumulative meters only) */
        if (!checkMonotonic(value)) {
            return "Non-monotonic reading detected (meter may have reset)";
        }

        /* all-checks passed */
        last_valid_timestamp_ = timestamp;
        return std::nullopt;  //valid
    }


    bool Validator::shouldStore(double current_value) {
        double dalta = std::abs(current_value - last_stored_value_);

        if (dalta >= config_.deadband_threshold) {
            last_stored_value_ = current_value;
            return true;
        }

        return false;
    }

    bool Validator::checkRange(double value) const {
        return value >= 0.0 && value <= config_.max_value;
    }

    bool Validator::checkRateOfChange(
        double value,
        std::chrono::system_clock::time_point now
    ) {
        if (last_valid_timestamp_ == std::chrono::system_clock::time_point{}){
            return true; /*  check if first reading, no baseline */
        }

        auto elapsed_minute = std::chrono::duration_cast<std::chrono::minutes> (
            now - last_valid_timestamp_
        ).count();

        if (elapsed_minute <= 0) return true;
        double actual_change = std::abs(value - last_stored_value_);
        double max_allowed_change = elapsed_minute * config_.max_rate_of_change;
        return actual_change <= max_allowed_change;
    }

    bool Validator::checkMonotonic(double value) {
        if(value < last_stored_value_){
            if (value < 100.0 && last_stored_value_ > config_.max_value * 0.99) {
                return true;
            }
            return false;
        }
        return true;
    }
}



