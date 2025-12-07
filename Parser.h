#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

namespace Parser {
    struct NetStats {
        long rx_bytes;
        long tx_bytes;
    };

    struct DiskStats {
        std::string name;
        long total_bytes;
        long used_bytes;
        float percent_used;
    };

    float CpuUsage();
    float MemoryUsage();
    NetStats GetNetworkTraffic();
    int GetBatteryPercentage();
    std::vector<DiskStats> GetDiskUsage();
    std::vector<int> Pids();
    float ProcessCpuUsage(int pid);
    float ProcessMemoryUsage(int pid);
    std::string Command(int pid);
}

#endif