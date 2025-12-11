// GCC 15 COMPATIBLE - SPIDERWEB SHATTER EDITION
#include <cstdarg> 
#include <cstdio>
#include <cstdlib> 
#include <ctime>   

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

// --- SPIDERWEB FRACTURE ENGINE ---
// --- SPIDERWEB FRACTURE ENGINE (REALISTIC EDITION) ---
struct CrackBranch {
    ImVec2 start;
    ImVec2 end;
    float thickness;
    float alpha_mult;      // visibility
    ImVec2 velocity;       // velocity vector for motion
    float angle_vel;       // optional rotation speed
};

struct CrackEffect {
    ImVec2 origin;
    float start_time;
    std::vector<CrackBranch> branches;
};

std::vector<CrackEffect> active_cracks;

float RandFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

ImVec2 RotatePoint(ImVec2 point, ImVec2 center, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    point.x -= center.x;
    point.y -= center.y;
    float xnew = point.x * c - point.y * s;
    float ynew = point.x * s + point.y * c;
    point.x = xnew + center.x;
    point.y = ynew + center.y;
    return point;
}

void AddShatterEffect(float x, float y) {
    CrackEffect effect;
    effect.origin = ImVec2(x, y);
    effect.start_time = ImGui::GetTime();

    int core_shards = 20;
    for (int i = 0; i < core_shards; i++) {
        float angle = RandFloat(0, 6.28f);
        float len = RandFloat(2.0f, 15.0f);
        ImVec2 vel = ImVec2(cos(angle)*RandFloat(30,70), sin(angle)*RandFloat(30,70));
        effect.branches.push_back({ImVec2(x, y), ImVec2(x + cos(angle)*len, y + sin(angle)*len),
                                   RandFloat(1.0f,2.5f), 1.0f, vel, RandFloat(-2.0f,2.0f)});
    }

    int spokes = RandFloat(7,12);
    std::vector<ImVec2> spoke_dirs;
    float max_radius = 300.0f;

    for (int i = 0; i < spokes; i++) {
        float angle = (6.28f / spokes) * i + RandFloat(-0.3f,0.3f);
        ImVec2 dir = ImVec2(cos(angle), sin(angle));
        spoke_dirs.push_back(dir);

        ImVec2 start = ImVec2(x,y);
        ImVec2 current = start;
        int segments = 5;

        for(int j=0;j<segments;j++) {
            float zig = RandFloat(-5.0f,5.0f);
            float step = max_radius / segments;
            ImVec2 next = ImVec2(current.x + dir.x * step + dir.y * zig, current.y + dir.y * step - dir.x * zig);
            ImVec2 vel = ImVec2(dir.x*RandFloat(20,60), dir.y*RandFloat(20,60));
            effect.branches.push_back({current, next, RandFloat(1.5f,2.5f), 0.9f, vel, RandFloat(-1.5f,1.5f)});
            current = next;
        }
    }

    // Concentric rings
    int rings = RandFloat(3,6);
    for(int r=1;r<=rings;r++){
        float dist = (max_radius/(rings+1))*r + RandFloat(-20,20);
        for(size_t i=0;i<spoke_dirs.size();i++){
            ImVec2 dir1 = spoke_dirs[i];
            ImVec2 dir2 = spoke_dirs[(i+1)%spoke_dirs.size()];

            ImVec2 p1 = ImVec2(x + dir1.x*dist, y + dir1.y*dist);
            ImVec2 p2 = ImVec2(x + dir2.x*dist, y + dir2.y*dist);

            if(RandFloat(0,1)>0.2f){
                float mid_x = (p1.x+p2.x)/2.0f; 
                float mid_y = (p1.y+p2.y)/2.0f;
                mid_x = x + (mid_x - x)*0.9f;
                mid_y = y + (mid_y - y)*0.9f;

                ImVec2 vel1 = ImVec2(RandFloat(-30,30), RandFloat(-30,30));
                ImVec2 vel2 = ImVec2(RandFloat(-30,30), RandFloat(-30,30));
                effect.branches.push_back({p1, ImVec2(mid_x, mid_y), RandFloat(1.0f,1.5f), 0.7f, vel1, RandFloat(-1.0f,1.0f)});
                effect.branches.push_back({ImVec2(mid_x, mid_y), p2, RandFloat(1.0f,1.5f), 0.7f, vel2, RandFloat(-1.0f,1.0f)});
            }
        }
    }

    active_cracks.push_back(effect);
}

void RenderCracks() {
    ImDrawList* fg = ImGui::GetForegroundDrawList();
    float current_time = ImGui::GetTime();
    float max_heal = 3.0f;

    auto it = active_cracks.begin();
    while(it!=active_cracks.end()){
        float age = current_time - it->start_time;
        if(age>max_heal) { it = active_cracks.erase(it); continue; }

        float fade = 1.0f - (age/max_heal);
        fade = powf(fade,0.5f);

        for(auto& branch : it->branches){
            float dt = ImGui::GetIO().DeltaTime;
            branch.start.x += branch.velocity.x*dt;
            branch.start.y += branch.velocity.y*dt;
            branch.end.x   += branch.velocity.x*dt;
            branch.end.y   += branch.velocity.y*dt;
            branch.velocity.x *= 0.95f;
            branch.velocity.y *= 0.95f;

            branch.start = RotatePoint(branch.start,it->origin,branch.angle_vel*dt);
            branch.end   = RotatePoint(branch.end,it->origin,branch.angle_vel*dt);

            float dist = sqrtf(pow(branch.end.x - it->origin.x,2) + pow(branch.end.y - it->origin.y,2));
            float fade_tip = std::max(0.0f,1.0f - dist/300.0f);
            float alpha = fade * branch.alpha_mult * fade_tip;

            ImU32 col = ImColor(220,240,255,(int)(alpha*200));
            ImU32 shadow = ImColor(0,0,0,(int)(alpha*150));
            fg->AddLine(ImVec2(branch.start.x+1,branch.start.y+1), ImVec2(branch.end.x+1,branch.end.y+1), shadow, branch.thickness);
            fg->AddLine(branch.start, branch.end, col, branch.thickness);

            // Sub-splinter lines
            if(RandFloat(0,1)<0.1f){
                ImVec2 mid = ImVec2(branch.start.x + (branch.end.x-branch.start.x)/2 + RandFloat(-5,5),
                                    branch.start.y + (branch.end.y-branch.start.y)/2 + RandFloat(-5,5));
                fg->AddLine(mid, branch.end, ImColor(220,240,255,(int)(alpha*120)), branch.thickness*0.5f);
            }

            // Dynamic branching over time
            if(RandFloat(0,1)<0.02f){
                ImVec2 new_start = branch.end;
                float angle = RandFloat(0,6.28f);
                float len = RandFloat(5,20);
                ImVec2 vel = ImVec2(cos(angle)*RandFloat(20,50), sin(angle)*RandFloat(20,50));
                it->branches.push_back({new_start, ImVec2(new_start.x + cos(angle)*len, new_start.y + sin(angle)*len), RandFloat(0.5f,1.5f), 0.7f, vel, RandFloat(-2.0f,2.0f)});
            }
        }
        ++it;
    }
}

// --- UI HELPERS ---
void DrawRadialProgress(const char* label, float value, float max, ImVec2 center, float radius, ImVec4 color, const char* format, const char* subtext1 = "", const char* subtext2 = "") {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImDrawList* draw_list = window->DrawList;

    draw_list->AddCircle(center, radius, ImColor(255, 255, 255, 30), 64, 8.0f);
    
    float angle_start = -1.570796f; 
    float percentage = value / max;
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 1.0f) percentage = 1.0f;
    
    float angle_end = angle_start + (percentage * 6.283185f);

    draw_list->PathArcTo(center, radius, angle_start, angle_end, 64);
    draw_list->PathStroke(ImColor(color), 0, 8.0f);
    draw_list->PathArcTo(center, radius, angle_start, angle_end, 64);
    draw_list->PathStroke(ImColor(color.x, color.y, color.z, 0.4f), 0, 15.0f);

    char text_buf[32];
    sprintf(text_buf, format, value);
    ImVec2 text_size = ImGui::CalcTextSize(text_buf);
    
    float y_offset = (subtext1[0] != '\0') ? -15.0f : 0.0f;
    ImVec2 text_pos = ImVec2(center.x - text_size.x * 0.5f, center.y - text_size.y * 0.5f + y_offset);
    draw_list->AddText(text_pos, ImColor(255, 255, 255), text_buf);
    
    if (subtext1[0] != '\0') {
        ImVec2 s1_size = ImGui::CalcTextSize(subtext1);
        ImVec2 s1_pos = ImVec2(center.x - s1_size.x * 0.5f, center.y + 2); 
        draw_list->AddText(NULL, 13.0f, s1_pos, ImColor(220, 220, 220), subtext1);
    }
    if (subtext2[0] != '\0') {
        ImVec2 s2_size = ImGui::CalcTextSize(subtext2);
        ImVec2 s2_pos = ImVec2(center.x - s2_size.x * 0.5f, center.y + 15); 
        draw_list->AddText(NULL, 13.0f, s2_pos, ImColor(220, 220, 220), subtext2);
    }
    
    ImVec2 label_size = ImGui::CalcTextSize(label);
    ImVec2 label_pos = ImVec2(center.x - label_size.x * 0.5f, center.y + radius + 15);
    draw_list->AddText(label_pos, ImColor(180, 180, 180), label);
}

void SetGlassTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f; 
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.05f, 0.80f); 
    style.Colors[ImGuiCol_Border] = ImVec4(0.8f, 0.9f, 1.0f, 0.3f); 
    style.WindowBorderSize = 1.0f;
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 1.0f, 1.0f, 0.15f); 
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.35f); 
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.08f, 0.95f); 
}

std::string FormatBytes(long bytes) {
    float gb = bytes / (1024.0f * 1024.0f * 1024.0f);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (gb > 1000) ss << (gb / 1024.0f) << " TB";
    else ss << gb << " GB";
    return ss.str();
}

int main(int, char**) {
    srand(static_cast<unsigned>(time(0)));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return -1;
    const char* glsl_version = "#version 130";
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); 

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Monitor V8.2 (Spiderweb Shatter)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); 

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    SetGlassTheme();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    System system;
    Uint32 last_tick = SDL_GetTicks();
    
    float c_cpu = 0; float c_mem = 0;
    std::pair<float,float> c_net = {0,0};
    int c_bat = -1; bool c_online = false;
    std::vector<Parser::DiskStats> c_disks;
    std::vector<Process> c_procs;
    float max_net_kb = 10240.0f; 
    int selected_pid = -1; 
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

        RenderCracks(); // Draw shatter over everything

        if (SDL_GetTicks() - last_tick > 1000) {
            c_cpu = system.GetCpuUsage();
            c_mem = system.GetMemoryUsage();
            c_net = system.GetNetworkStats();
            c_online = system.IsConnected();
            c_bat = system.GetBattery();
            c_disks = system.GetDisks();
            c_procs = system.GetProcesses();
            last_tick = SDL_GetTicks();
        }

        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Dash", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "GLASS MONITOR // V8.2 // SHATTERED_WAY");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTable("GaugesTable", 4)) {
            float radius = 55.0f;
            float cell_height = radius * 2 + 50;
            ImGui::TableNextColumn();
            DrawRadialProgress("CPU", c_cpu, 100.0f, ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetColumnWidth()/2, ImGui::GetCursorScreenPos().y + radius + 10), radius, ImVec4(0,1,1,1), "%.0f%%");
            ImGui::Dummy(ImVec2(0, cell_height));
            ImGui::TableNextColumn();
            DrawRadialProgress("RAM", c_mem, 100.0f, ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetColumnWidth()/2, ImGui::GetCursorScreenPos().y + radius + 10), radius, ImVec4(1,0,1,1), "%.0f%%");
            
            if (c_online) {
                ImGui::TableNextColumn();
                DrawRadialProgress("DOWN", c_net.first, max_net_kb, ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetColumnWidth()/2, ImGui::GetCursorScreenPos().y + radius + 10), radius, ImVec4(0,1,0.5,1), "%.0f KB/s");
                ImGui::TableNextColumn();
                DrawRadialProgress("UP", c_net.second, max_net_kb, ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetColumnWidth()/2, ImGui::GetCursorScreenPos().y + radius + 10), radius, ImVec4(1,0.5,0,1), "%.0f KB/s");
            }
            if (c_bat >= 0) {
                ImGui::TableNextColumn();
                ImVec4 bat_col = (c_bat > 20) ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1);
                DrawRadialProgress("BATTERY", (float)c_bat, 100.0f, ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetColumnWidth()/2, ImGui::GetCursorScreenPos().y + radius + 10), radius, bat_col, "%.0f%%");
            }
            for (const auto& disk : c_disks) {
                ImGui::TableNextColumn();
                std::string used = FormatBytes(disk.used_bytes);
                std::string total = "/ " + FormatBytes(disk.total_bytes);
                DrawRadialProgress(disk.name.c_str(), disk.percent_used, 100.0f, ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetColumnWidth()/2, ImGui::GetCursorScreenPos().y + radius + 10), radius, ImVec4(1,0.8,0,1), "%.0f%%", used.c_str(), total.c_str());
                ImGui::Dummy(ImVec2(0, cell_height));
            }
            ImGui::EndTable();
        }

        ImGui::Separator();
        ImGui::Text("ACTIVE PROCESSES");
        
        if (ImGui::BeginTable("proc_table", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("CPU", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("COMMAND");
            ImGui::TableHeadersRow();

            std::sort(c_procs.begin(), c_procs.end(), [](const Process& a, const Process& b) { return a.cpuUsage > b.cpuUsage; });

            for (size_t i = 0; i < c_procs.size() && i < 30; i++) {
                ImGui::PushID(c_procs[i].pid); 
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                char label[32];
                sprintf(label, "%d", c_procs[i].pid);
                bool is_selected = (selected_pid == c_procs[i].pid);
                if (ImGui::Selectable(label, is_selected, ImGuiSelectableFlags_SpanAllColumns)) selected_pid = c_procs[i].pid;

                if (ImGui::BeginPopupContextItem("context_menu")) {
                    ImGui::Text("System Actions: %d", c_procs[i].pid);
                    ImGui::Separator();
                    
                    if (ImGui::MenuItem("Terminate")) {
                        system.TerminateProcess(c_procs[i].pid);
                        ImVec2 m = ImGui::GetMousePos();
                        AddShatterEffect(m.x, m.y);
                    }
                    
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                    if (ImGui::MenuItem("SHATTER (KILL)")) {
                        system.KillProcess(c_procs[i].pid);
                        ImVec2 m = ImGui::GetMousePos();
                        AddShatterEffect(m.x, m.y); // Creates the spiderweb!
                    }
                    ImGui::PopStyleColor();
                    ImGui::EndPopup();
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.1f %%", c_procs[i].cpuUsage);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", c_procs[i].command.c_str());
                ImGui::PopID(); 
            }
            ImGui::EndTable();
        }

        ImGui::End();
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f); 
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