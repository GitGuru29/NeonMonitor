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

#include "System.h" 
#include "Process.h"

// --- CUSTOM WIDGET: RADIAL PROGRESS ---
void DrawRadialProgress(const char* label, float value, float max, ImVec2 center, float radius, ImVec4 color, const char* format) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImDrawList* draw_list = window->DrawList;

    // Background Ring
    draw_list->AddCircle(center, radius, ImColor(30, 30, 30), 64, 8.0f);

    // Foreground Arc
    float angle_start = -1.570796f; // Top
    float percentage = value / max;
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 1.0f) percentage = 1.0f;
    
    float angle_end = angle_start + (percentage * 6.283185f);

    draw_list->PathArcTo(center, radius, angle_start, angle_end, 64);
    draw_list->PathStroke(ImColor(color), 0, 8.0f);

    // Text in Center
    char text_buf[32];
    sprintf(text_buf, format, value);
    ImVec2 text_size = ImGui::CalcTextSize(text_buf);
    ImVec2 text_pos = ImVec2(center.x - text_size.x * 0.5f, center.y - text_size.y * 0.5f - 5);
    draw_list->AddText(text_pos, ImColor(255, 255, 255), text_buf);
    
    // Label Below
    ImVec2 label_size = ImGui::CalcTextSize(label);
    ImVec2 label_pos = ImVec2(center.x - label_size.x * 0.5f, center.y + 15);
    draw_list->AddText(label_pos, ImColor(150, 150, 150), label);
}

void SetCyberpunkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);
}

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return -1;

    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("CyberMonitor V3.1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1100, 700, window_flags);
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
    float cached_cpu = 0.0f;
    float cached_mem = 0.0f;
    std::pair<float, float> cached_net = {0,0};
    std::vector<Process> cached_procs;

    // 10 MB/s limit for the visual gauge
    float max_net_speed_kb = 10240.0f; 

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
            cached_cpu = system.GetCpuUsage();
            cached_mem = system.GetMemoryUsage();
            cached_net = system.GetNetworkStats();
            cached_procs = system.GetProcesses();
            last_tick = SDL_GetTicks();
        }

        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Dashboard", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        ImGui::Text("SYSTEM_MONITOR // V3.1 // FULL_NET_SUITE");
        ImGui::Separator();
        ImGui::Spacing();

        // --- 4 COLUMNS FOR GAUGES ---
        ImGui::Columns(4, "gauges", false); 
        
        float circle_radius = 55.0f; 
        float y_pos = ImGui::GetCursorScreenPos().y + circle_radius + 20;

        // 1. CPU
        ImVec2 center_cpu = ImVec2(ImGui::GetColumnOffset(0) + ImGui::GetColumnWidth(0)/2, y_pos);
        DrawRadialProgress("CPU LOAD", cached_cpu, 100.0f, center_cpu, circle_radius, ImVec4(0,1,1,1), "%.0f%%");
        ImGui::NextColumn();

        // 2. RAM
        ImVec2 center_mem = ImVec2(ImGui::GetColumnOffset(1) + ImGui::GetColumnWidth(1)/2, y_pos);
        DrawRadialProgress("RAM USAGE", cached_mem, 100.0f, center_mem, circle_radius, ImVec4(1,0,1,1), "%.0f%%");
        ImGui::NextColumn();

        // 3. NET DOWN
        ImVec2 center_down = ImVec2(ImGui::GetColumnOffset(2) + ImGui::GetColumnWidth(2)/2, y_pos);
        DrawRadialProgress("NET DOWN", cached_net.first, max_net_speed_kb, center_down, circle_radius, ImVec4(0,1,0.5,1), "%.0f KB/s");
        ImGui::NextColumn();

        // 4. NET UP
        ImVec2 center_up = ImVec2(ImGui::GetColumnOffset(3) + ImGui::GetColumnWidth(3)/2, y_pos);
        DrawRadialProgress("NET UP", cached_net.second, max_net_speed_kb, center_up, circle_radius, ImVec4(1,0.5,0,1), "%.0f KB/s");
        
        ImGui::Columns(1);
        ImGui::Dummy(ImVec2(0, circle_radius * 2 + 30));

        ImGui::Separator();
        
        // --- PROCESS TABLE ---
        ImGui::Text("ACTIVE PROCESSES [%lu]", cached_procs.size());
        if (ImGui::BeginTable("proc_table", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("CPU", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("COMMAND");
            ImGui::TableHeadersRow();

            std::sort(cached_procs.begin(), cached_procs.end(), [](const Process& a, const Process& b) {
                return a.cpuUsage > b.cpuUsage;
            });

            for (size_t i = 0; i < cached_procs.size() && i < 20; i++) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", cached_procs[i].pid);
                ImGui::TableSetColumnIndex(1);
                
                if (cached_procs[i].cpuUsage > 50.0f) 
                    ImGui::TextColored(ImVec4(1,0,0,1), "%.1f %%", cached_procs[i].cpuUsage);
                else 
                    ImGui::Text("%.1f %%", cached_procs[i].cpuUsage);
                
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", cached_procs[i].command.c_str());
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