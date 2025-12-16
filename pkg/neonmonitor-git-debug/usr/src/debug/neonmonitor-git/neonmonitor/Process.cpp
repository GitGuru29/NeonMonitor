#include "Process.h"
#include "Parser.h"

Process::Process(int pid) : pid(pid) {
    Update();
}

void Process::Update() {
    cpuUsage = Parser::ProcessCpuUsage(pid);
    memoryUsage = Parser::ProcessMemoryUsage(pid);
    command = Parser::Command(pid);
}
