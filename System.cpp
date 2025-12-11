#include "System.h"
#include "Parser.h"
#include <signal.h> // Needed for sending signals

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
    return { (float)rx_delta / 1024.0f, (float)tx_delta / 1024.0f };
}

bool System::IsConnected() {
    return Parser::IsConnected();
}

int System::GetBattery() {
    return Parser::GetBatteryPercentage();
}

// --- PROCESS CONTROL IMPLEMENTATION ---
void System::TerminateProcess(int pid) {
    // SIGTERM (15): Asks the program to stop nicely (saves data)
    kill(pid, SIGTERM);
}

void System::KillProcess(int pid) {
    // SIGKILL (9): Instantly rips the process from memory (use if frozen)
    kill(pid, SIGKILL);
}

std::vector<Parser::DiskStats> System::GetDisks() {
    return Parser::GetDiskUsage();
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