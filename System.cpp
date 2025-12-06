#include "System.h"
#include "Parser.h"

float System::GetCpuUsage() {
    return Parser::CpuUsage();
}

float System::GetMemoryUsage() {
    return Parser::MemoryUsage();
}

std::pair<float, float> System::GetNetworkStats() {
    Parser::NetStats current = Parser::GetNetworkTraffic();

    long rx_delta = current.rx_bytes - last_rx_bytes;
    long tx_delta = current.tx_bytes - last_tx_bytes;

    last_rx_bytes = current.rx_bytes;
    last_tx_bytes = current.tx_bytes;

    // Convert bytes to KB
    return { (float)rx_delta / 1024.0f, (float)tx_delta / 1024.0f };
}

std::vector<Process> System::GetProcesses() {
    std::vector<Process> processes;
    std::vector<int> pids = Parser::Pids();
    for (int pid : pids) {
        Process proc(pid);
        processes.push_back(proc);
    }
    return processes;
}