cmake_minimum_required(VERSION 3.10)
project(LinuxSystemMonitor)

# 1. Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Find System Packages (SDL2, OpenGL)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# 3. Include ImGui Source Files
# (We list them manually because ImGui doesn't come with a CMake file by default)
set(IMGUI_DIR "imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# 4. Define Your Source Files
set(SOURCES
    main_gui.cpp
    System.cpp
    parser.cpp
    Process.cpp
    ${IMGUI_SOURCES}
)

# 5. Create the Executable
add_executable(system_monitor_gui ${SOURCES})

# 6. Link Libraries
# We link SDL2, OpenGL, and 'dl' (needed for dynamic loading)
target_link_libraries(system_monitor_gui 
    ${SDL2_LIBRARIES} 
    OpenGL::GL 
    dl
    pthread
)

# 7. Include Directories
target_include_directories(system_monitor_gui PRIVATE 
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
    ${SDL2_INCLUDE_DIRS}
)