# NeonMonitor ⚡🖥️

**NeonMonitor** is a lightweight, high-performance Linux system monitor written in **C++17** with a **Dear ImGui / SDL2 + OpenGL GUI**. It displays real-time system information using direct kernel data sources like `/proc` and `/sys`.

---

## 🚀 Features

- 💻 **CPU usage** — total + per-core
- 🧠 **Memory usage**
- 📊 **Process list** — PID, CPU %, Memory %
- 🛠️ **Process control** — terminate/kill
- 🌐 **Network traffic** — RX/TX
- 📁 **Disk usage**
- 🔋 **Battery percentage**
- 📡 **Connectivity status**
- 🎨 Simple Dear ImGui GUI

---

## 🧠 Why NeonMonitor?

- Minimal overhead
- Fast GUI
- Direct access to Linux kernel data
- Lightweight and portable

---

## 🐧 Platform Support

✔ **Linux only** (no Windows/macOS)

Reads system stats from `/proc` and `/sys`.

---

## 🧩 Dependencies

| Dependency | Notes |
|---|---|
| C++17 compiler | GCC or Clang |
| CMake ≥ 3.10 | Build system |
| SDL2 dev libraries | GUI backend |
| OpenGL dev libraries | Rendering |
| pthread / dl | Usually pre-installed |

---

## ⚡ Installation & Build

### 🔹 Method 1 — Prebuilt `.deb` (Debian/Ubuntu)

```bash
sudo dpkg -i neonmonitor-1.0.0-Linux.deb
sudo apt-get install -f
system_monitor_gui
```

---

### 🔹 Method 2 — Portable `.tar.gz`

```bash
cd ~/Downloads
tar -xzvf neonmonitor-1.0.0-Linux.tar.gz

mkdir -p ~/.local/bin
cp neonmonitor-1.0.0-Linux/bin/system_monitor_gui ~/.local/bin/neonmonitor
chmod +x ~/.local/bin/neonmonitor

echo 'export PATH=$HOME/.local/bin:$PATH' >> ~/.bashrc
echo 'export PATH=$HOME/.local/bin:$PATH' >> ~/.zshrc

source ~/.bashrc
neonmonitor
```

---

### 🔹 Method 3 — Build from Source *(Recommended)*

**1️⃣ Install Dependencies**

Ubuntu / Debian:
```bash
sudo apt update
sudo apt install -y build-essential cmake libsdl2-dev libgl1-mesa-dev
```

Arch Linux:
```bash
sudo pacman -S --needed base-devel cmake sdl2 mesa
```

**2️⃣ Clone the Repository**

```bash
git clone https://github.com/GitGuru29/NeonMonitor.git
cd NeonMonitor
```

**3️⃣ Build**

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -j
```

**4️⃣ Run**

```bash
./system_monitor_gui
```

> ⚠️ Some features (e.g. killing processes) may require elevated permissions:
> ```bash
> sudo ./system_monitor_gui
> ```

---

## 🏗️ How It Works

NeonMonitor polls Linux kernel files directly for all metrics:

| Source | Data |
|---|---|
| `/proc/stat` | CPU usage |
| `/proc/meminfo` | Memory usage |
| `/proc/net/dev` | Network traffic |
| `/proc/[pid]/` | Per-process metrics |
| `/sys/class/net/*/operstate` | Interface status |
| `/sys/class/power_supply/*/capacity` | Battery level |

---

## 🗺️ Architecture

```
┌───────────────────────────┐
│   ImGui Renderer (SDL2)   │
└───────────────┬───────────┘
                │
       ┌────────▼────────┐
       │ Data Collection │
       │  (Poll Files)   │
       └────────┬────────┘
                │
   ┌────────────▼────────────┐
   │  Linux Kernel Interfaces │
   │     (/proc, /sys)        │
   └──────────────────────────┘
```

---

## 📈 Roadmap

- [ ] Per-core CPU graphs + history
- [ ] Temperature monitoring (hwmon)
- [ ] GPU utilization
- [ ] Process filtering/search
- [ ] Export metrics logging
- [ ] Themes & UI customization

---

## 🧑‍💻 Contributing

Contributions are welcome!

- Open issues for new feature modules
- Provide sample `/proc` outputs for testing
- Tag system performance edge cases

---

## 📄 License

[MIT License](LICENSE)

---

## 👨‍💻 Author

**Siluna Nusal Dangalla** — [GitHub: @GitGuru29](https://github.com/GitGuru29)
