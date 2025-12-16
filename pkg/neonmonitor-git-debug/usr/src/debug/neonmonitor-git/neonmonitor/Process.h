#ifndef PROCESS_H
#define PROCESS_H

#include <string>

class Process {
public:
    Process(int pid);

    int pid;
    float cpuUsage;
    float memoryUsage;
    std::string command;

private:
    void Update();
};

#endif
