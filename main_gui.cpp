#include <cstdarg> 
#include <cstdio>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "System.h" 
#include "Process.h"

// --- UPDATED WIDGET: Supports 2 lines of subtext ---
void DrawRadialProgress(const char* label, float value, float max, ImVec2 center, float radius, ImVec4 color, const char* format, const char* subtext1 = "", const char* subtext2 = "") {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImDrawList* draw_list = window->DrawList;

    // Background Ring
    draw_list->AddCircle(center, radius, ImColor(40, 40, 50), 64, 8.0f);
    
    // Foreground Arc
    float angle_start = -1.570796f; 
    float percentage = value / max;
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 1.0f) percentage = 1.0f;
    
    float angle_end = angle_start + (percentage * 6.283185f);

    draw_list->PathArcTo(center, radius, angle_start, angle_end, 64);
    draw_list->PathStroke(ImColor(color), 0, 8.0f);

    // Center Value (e.g. "50%")
    char text_buf[32];
    sprintf(text_buf, format, value);
    ImVec2 text_size = ImGui::CalcTextSize(text_buf);
    
    // Move main value UP if we have subtext
    float y_offset = (subtext1[0] != '\0') ? -12.0f : 0.0f;
    ImVec2 text_pos = ImVec2(center.x - text_size.x * 0.5f, center.y - text_size.y * 0.5f + y_offset);
    draw_list->AddText(text_pos, ImColor(255, 255, 255), text_buf);
    
    // Subtext Line 1 (e.g. "102 GB")
    if (subtext1[0] != '\0') {
        ImVec2 s1_size = ImGui::CalcTextSize(subtext1);
        ImVec2 s1_pos = ImVec2(center.x - s1_size.x * 0.5f, center.y + 5); 
        draw_list->AddText(NULL, 13.0f, s1_pos, ImColor(200, 200, 200), subtext1);
    }

    // Subtext Line 2 (e.g. "of 500 GB")
    if (subtext2[0] != '\0') {
        ImVec2 s2_size = ImGui::CalcTextSize(subtext2);
        ImVec2 s2_pos = ImVec2(center.x - s2_size.x * 0.5f, center.y + 18); // Below line 1
        draw_list->AddText(NULL, 13.0f, s2_pos, ImColor(200, 200, 200), subtext2);
    }
    
    // Label
    ImVec2 label_size = ImGui::CalcTextSize(label);
    ImVec2 label_pos = ImVec2(center.x - label_size.x * 0.5f, center.y + radius + 15);
    draw_list->AddText(label_pos, ImColor(150, 150, 150), label);
}

void SetCyberpunkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);
}

std::string FormatBytes(long bytes) {
    float gb = bytes / (1024.0f * 1024.0f * 1024.0f);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (gb > 1000) {
        ss << (gb / 1024.0f) << " TB";
    } else {
        ss << gb << " GB";
    }
    return ss.str();
}

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return -1;
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("CyberMonitor V4.2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); 

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    SetCyberpunkTheme();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    System system;
    Uint32 last_tick = SDL_GetTicks();
    
    float c_cpu = 0;
    float c_mem = 0;
    std::pair<float,float> c_net = {0,0};
    int c_bat = -1;
    std::vector<Parser::DiskStats> c_disks;
    std::vector<Process> c_procs;

    float max_net_kb = 10240.0f; 

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) done = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (SDL_GetTicks() - last_tick > 1000) {
            c_cpu = system.GetCpuUsage();
            c_mem = system.GetMemoryUsage();
            c_net = system.GetNetworkStats();
            c_bat = system.GetBattery();
            c_disks = system.GetDisks();
            c_procs = system.GetProcesses();
            last_tick = SDL_GetTicks();
        }

        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Dash", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        ImGui::Text("SYSTEM_MONITOR // V4.2 // FIXED_ALIGNMENT");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTable("GaugesTable", 4)) {
            float radius = 55.0f;
            float cell_height = radius * 2 + 50;

            ImGui::TableNextColumn();
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImVec2 center = ImVec2(p.x + ImGui::GetColumnWidth()/2, p.y + radius + 10);
            DrawRadialProgress("CPU", c_cpu, 100.0f, center, radius, ImVec4(0,1,1,1), "%.0f%%");
            ImGui::Dummy(ImVec2(0, cell_height));

            ImGui::TableNextColumn();
            p = ImGui::GetCursorScreenPos();
            center = ImVec2(p.x + ImGui::GetColumnWidth()/2, p.y + radius + 10);
            DrawRadialProgress("RAM", c_mem, 100.0f, center, radius, ImVec4(1,0,1,1), "%.0f%%");

            ImGui::TableNextColumn();
            p = ImGui::GetCursorScreenPos();
            center = ImVec2(p.x + ImGui::GetColumnWidth()/2, p.y + radius + 10);
            DrawRadialProgress("DOWN", c_net.first, max_net_kb, center, radius, ImVec4(0,1,0.5,1), "%.0f KB/s");

            ImGui::TableNextColumn();
            p = ImGui::GetCursorScreenPos();
            center = ImVec2(p.x + ImGui::GetColumnWidth()/2, p.y + radius + 10);
            DrawRadialProgress("UP", c_net.second, max_net_kb, center, radius, ImVec4(1,0.5,0,1), "%.0f KB/s");

            if (c_bat >= 0) {
                ImGui::TableNextColumn();
                p = ImGui::GetCursorScreenPos();
                center = ImVec2(p.x + ImGui::GetColumnWidth()/2, p.y + radius + 10);
                ImVec4 bat_col = (c_bat > 20) ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1);
                DrawRadialProgress("BATTERY", (float)c_bat, 100.0f, center, radius, bat_col, "%d%%");
                ImGui::Dummy(ImVec2(0, cell_height));
            }

            for (const auto& disk : c_disks) {
                ImGui::TableNextColumn();
                p = ImGui::GetCursorScreenPos();
                center = ImVec2(p.x + ImGui::GetColumnWidth()/2, p.y + radius + 10);
                
                std::string used = FormatBytes(disk.used_bytes);
                std::string total = "" + FormatBytes(disk.total_bytes);

                // Pass used and total as separate lines!
                DrawRadialProgress(disk.name.c_str(), disk.percent_used, 100.0f, center, radius, ImVec4(1,0.8,0,1), "%.0f%%", used.c_str(), total.c_str());
                ImGui::Dummy(ImVec2(0, cell_height));
            }

            ImGui::EndTable();
        }

        ImGui::Separator();
        
        ImGui::Text("ACTIVE PROCESSES [%lu]", c_procs.size());
        if (ImGui::BeginTable("proc_table", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("CPU", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("COMMAND");
            ImGui::TableHeadersRow();

            std::sort(c_procs.begin(), c_procs.end(), [](const Process& a, const Process& b) {
                return a.cpuUsage > b.cpuUsage;
            });

            for (size_t i = 0; i < c_procs.size() && i < 20; i++) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", c_procs[i].pid);
                ImGui::TableSetColumnIndex(1);
                
                if (c_procs[i].cpuUsage > 50.0f) 
                    ImGui::TextColored(ImVec4(1,0,0,1), "%.1f %%", c_procs[i].cpuUsage);
                else 
                    ImGui::Text("%.1f %%", c_procs[i].cpuUsage);
                
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", c_procs[i].command.c_str());
            }
            ImGui::EndTable();
        }

        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}