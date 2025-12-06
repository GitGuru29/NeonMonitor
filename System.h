#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>
#include <utility>
#include "Parser.h"
#include "Process.h"

class System {
private:
    long last_rx_bytes = 0;
    long last_tx_bytes = 0;

public:
    float GetCpuUsage();
    float GetMemoryUsage();
    std::pair<float, float> GetNetworkStats();
    std::vector<Process> GetProcesses();
};

#endif