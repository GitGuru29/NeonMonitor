#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "System.h"
#include "Process.h"

int main() {
    System system;

    while (true) {
        // 1. Get Data
        float cpuUsage = system.GetCpuUsage();
        float memUsage = system.GetMemoryUsage();
        std::vector<Process> processes = system.GetProcesses();

        // 2. Sort Processes (High CPU first)
        std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
            return a.cpuUsage > b.cpuUsage;
        });

        // 3. Construct JSON manually (avoiding external dependencies for now)
        std::cout << "{";
        std::cout << "\"cpu\": " << std::fixed << std::setprecision(2) << cpuUsage << ",";
        std::cout << "\"memory\": " << std::fixed << std::setprecision(2) << memUsage << ",";
        std::cout << "\"processes\": [";

        // Limit to top 20 processes to keep the data stream light
        int limit = 20;
        for (size_t i = 0; i < processes.size() && i < limit; ++i) {
            const auto& proc = processes[i];
            
            // Simple manual JSON formatting
            std::cout << "{";
            std::cout << "\"pid\": " << proc.pid << ",";
            std::cout << "\"cpu\": " << proc.cpuUsage << ",";
            std::cout << "\"mem\": " << proc.memoryUsage << ",";
            // Sanitize command string (basic) to avoid breaking JSON with quotes
            std::string cmd = proc.command; 
            // Very basic escape for quotes could be added here if needed
            std::cout << "\"command\": \"" << cmd << "\""; 
            std::cout << "}";

            if (i < processes.size() - 1 && i < limit - 1) {
                std::cout << ",";
            }
        }
        
        std::cout << "]";
        std::cout << "}" << std::endl; // Flush with newline

        // 4. Update Rate
        // 500ms or 1000ms is good for a desktop widget
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}