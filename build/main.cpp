#include "m_reader/config.hpp"
#include "m_reader/meter_reader.hpp"
#include "m_reader/database.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>

std::atomic<bool> g_running{true};

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down..." << std::endl;
        g_running = false;
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        std::string config_path = (argc > 1) ? argv[1] : "config/meter_config.json";
        auto config = meter_reader::load_config(config_path);
        
        std::cout << "Starting Meter Reader v1.0" << std::endl;
        
        meter_reader::Database db;
        if (!db.connect(config.database)) {
            std::cerr << "Failed to connect to database" << std::endl;
            return 1;
        }
        
        meter_reader::MeterService service(config.meter, db);
        
        while (g_running) {
            service.readAndSave();
            if (!g_running) break;
            std::this_thread::sleep_for(std::chrono::seconds(config.meter.polling_interval_seconds));
        }
        
        std::cout << "Stopped gracefully" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}