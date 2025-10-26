# Graphics Engine - Random Number Display

A Windows graphics engine demonstrating clean separation of application logic and rendering, with swappable GDI and DirectX 12 backends.

## 📁 Project Structure

```
memory-reader/
│
├── 📂 src/                         # Source implementations
│   ├── 📂 core/                    # Core application logic
│   │   ├── main.cpp                # Entry point & window management
│   │   └── Engine.cpp              # Application logic (random numbers, timing)
│   │
│   ├── 📂 renderers/               # Renderer implementations
│   │   ├── GDIRenderer.cpp         # GDI software renderer
│   │   └── DX12Renderer.cpp        # DirectX 12 hardware renderer
│   │
│   └── 📂 legacy/                  # Legacy monolithic implementations
│
├── 📂 include/                     # Header files
│   ├── 📂 core/                    # Core headers
│   │   ├── Engine.h                # Engine class
│   │   ├── IRenderer.h             # Renderer interface
│   │   └── Logger.h                # Logging utilities
│   │
│   ├── 📂 renderers/               # Renderer headers
│   │   ├── GDIRenderer.h
│   │   └── DX12Renderer.h
│   │
│   └── 📂 legacy/                  # Legacy headers (not compiled)
│
├── 📂 assets/                      # Runtime assets
│   ├── arial24.spritefont          # Small font for labels
│   └── arial120.spritefont         # Large font for numbers
│
├── 📂 docs/                        # Documentation
│   ├── ARCHITECTURE.md
│   └── FILE_STRUCTURE.md
│
└── CMakeLists.txt                  # Build configuration
```

## 🏗️ Architecture - Clean Separation

```
Application (Engine)  →  Uses  →  IRenderer Interface
                                       ↓ Implements
                                  ┌────┴─────┐
                                  ↓          ↓
                            GDIRenderer  DX12Renderer
```

**Benefits**: Single responsibility, swappable backends, easy to extend

## 🚀 Quick Start

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
cd Release && ./GraphicsEngine.exe
```

## 🎮 Controls

- **G** - Switch to GDI renderer
- **D** - Switch to DirectX 12 renderer  
- **ESC** - Exit

## 📝 Features

- Random number (0-9999) updates every 5 seconds
- Runtime renderer switching
- Hardware (DX12) and Software (GDI) rendering
- Organized directory structure
- Automatic asset copying
