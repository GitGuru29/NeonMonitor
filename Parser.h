#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

namespace Parser {
    struct NetStats {
        long rx_bytes;
        long tx_bytes;
    };

    float CpuUsage();
    float MemoryUsage();
    NetStats GetNetworkTraffic();
    std::vector<int> Pids();
    float ProcessCpuUsage(int pid);
    float ProcessMemoryUsage(int pid);
    std::string Command(int pid);
}

#endif