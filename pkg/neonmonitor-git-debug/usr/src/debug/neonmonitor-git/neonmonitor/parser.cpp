#include "Parser.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <sys/statvfs.h> 
#include <cstring>

float Parser::CpuUsage() {
    std::ifstream file("/proc/stat");
    std::string line, cpu;
    long user, nice, system, idle, iowait, irq, softirq;
    std::getline(file, line);
    std::istringstream ss(line);
    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
    long total = user + nice + system + idle + iowait + irq + softirq;
    return 100.0 * (total - idle) / total;
}

float Parser::MemoryUsage() {
    std::ifstream file("/proc/meminfo");
    std::string key, unit;
    float value, total = 0, available = 0;
    while (file >> key >> value >> unit) {
        if (key == "MemTotal:") total = value;
        else if (key == "MemAvailable:") {
            available = value;
            break;
        }
    }
    if (total == 0) return 0;
    return 100.0 * (total - available) / total;
}

Parser::NetStats Parser::GetNetworkTraffic() {
    std::ifstream file("/proc/net/dev");
    std::string line;
    long total_rx = 0;
    long total_tx = 0;
    std::getline(file, line); 
    std::getline(file, line); 
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string iface;
        long rx, tx, junk;
        ss >> iface; 
        if (line.find("lo:") != std::string::npos) continue;
        ss >> rx;
        for (int i = 0; i < 7; ++i) ss >> junk;
        ss >> tx;
        total_rx += rx;
        total_tx += tx;
    }
    return {total_rx, total_tx};
}

// --- CONNECTIVITY CHECK ---
bool Parser::IsConnected() {
    DIR* dir = opendir("/sys/class/net");
    if (!dir) return false;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string iface = entry->d_name;
        // Skip loopback and parent dirs
        if (iface == "." || iface == ".." || iface == "lo") continue;

        std::string path = "/sys/class/net/" + iface + "/operstate";
        std::ifstream file(path);
        std::string state;
        // If ANY interface says "up", we are connected
        if (file >> state && state == "up") {
            closedir(dir);
            return true;
        }
    }
    closedir(dir);
    return false;
}

int Parser::GetBatteryPercentage() {
    int cap = -1; 
    std::vector<std::string> bats = {"BAT0", "BAT1", "BAT", "CMB0"};
    for (const auto& bat : bats) {
        std::string path = "/sys/class/power_supply/" + bat + "/capacity";
        std::ifstream file(path);
        if (file.is_open()) {
            file >> cap;
            if (cap >= 0 && cap <= 100) return cap;
        }
    }
    return -1;
}

std::vector<Parser::DiskStats> Parser::GetDiskUsage() {
    std::vector<Parser::DiskStats> disks;
    std::ifstream file("/proc/mounts");
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string device, mountpoint, fstype;
        ss >> device >> mountpoint >> fstype;

        if (mountpoint.find("/boot") == 0) continue;

        if (device.find("/dev/") == 0 && 
           (fstype == "ext4" || fstype == "btrfs" || fstype == "vfat" || fstype == "ntfs" || fstype == "xfs")) {
            
            struct statvfs stat;
            if (statvfs(mountpoint.c_str(), &stat) == 0) {
                long total = stat.f_blocks * stat.f_frsize;
                long available = stat.f_bavail * stat.f_frsize;
                long used = total - available;
                
                float percent = 0.0f;
                if (total > 0) percent = (float)used / (float)total * 100.0f;

                std::string name = mountpoint;
                if (mountpoint == "/") name = "ROOT";
                else if (mountpoint == "/home") name = "HOME";
                else if (mountpoint.find("/run/media/") == 0) {
                    size_t last_slash = mountpoint.find_last_of('/');
                    if (last_slash != std::string::npos) {
                        name = mountpoint.substr(last_slash + 1);
                    }
                }

                if (name.length() > 8 && name != "ROOT" && name != "HOME") {
                    name = name.substr(0, 6) + "..";
                }

                disks.push_back({name, total, used, percent});
            }
        }
    }
    return disks;
}

std::vector<int> Parser::Pids() {
    std::vector<int> pids;
    DIR* dir = opendir("/proc");
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            std::string name(entry->d_name);
            if (std::all_of(name.begin(), name.end(), ::isdigit)) {
                pids.push_back(std::stoi(name));
            }
        }
    }
    closedir(dir);
    return pids;
}

float Parser::ProcessCpuUsage(int pid) {
    std::ifstream file("/proc/" + std::to_string(pid) + "/stat");
    std::string value;
    std::vector<std::string> values;
    while (file >> value) values.push_back(value);
    if (values.size() < 22) return 0.0;
    long utime = stol(values[13]);
    long stime = stol(values[14]);
    long cutime = stol(values[15]);
    long cstime = stol(values[16]);
    long starttime = stol(values[21]);
    long total_time = utime + stime + cutime + cstime;
    long uptime;
    std::ifstream uptime_file("/proc/uptime");
    uptime_file >> uptime;
    long hertz = sysconf(_SC_CLK_TCK);
    float seconds = uptime - (starttime / hertz);
    return 100.0 * ((total_time / hertz) / seconds);
}

float Parser::ProcessMemoryUsage(int pid) {
    std::ifstream file("/proc/" + std::to_string(pid) + "/status");
    std::string line;
    float memory = 0;
    while (std::getline(file, line)) {
        if (line.find("VmRSS:") == 0) {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> memory;
            break;
        }
    }
    return memory / 1024; 
}

std::string Parser::Command(int pid) {
    std::ifstream file("/proc/" + std::to_string(pid) + "/cmdline");
    std::string cmd;
    std::getline(file, cmd);
    return cmd.empty() ? "[unknown]" : cmd;
}