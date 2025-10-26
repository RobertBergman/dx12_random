# Graphics Engine - File Structure

## Project Layout

```
memory-reader/
├── Core Framework
│   ├── main.cpp                    # Application entry point & window management
│   ├── Engine.h                    # Engine class (application logic)
│   ├── Engine.cpp                  # Engine implementation
│   ├── IRenderer.h                 # Renderer interface (pure virtual)
│   └── Logger.h                    # Logging utilities
│
├── Renderer Implementations
│   ├── GDIRenderer.h               # GDI renderer header
│   ├── GDIRenderer.cpp             # GDI renderer implementation
│   ├── DX12Renderer.h              # DirectX 12 renderer header
│   └── DX12Renderer.cpp            # DirectX 12 renderer implementation
│
├── Legacy (Keep for reference, not compiled)
│   ├── IGraphicsEngine.h           # Old monolithic engine interface
│   ├── GDIEngine.h                 # Old GDI engine (logic + rendering)
│   ├── GDIEngine.cpp
│   ├── DX12Engine.h                # Old DX12 engine (logic + rendering)
│   └── DX12Engine.cpp
│
├── Build Configuration
│   ├── CMakeLists.txt              # CMake build configuration
│   └── d3dx12.h                    # DirectX 12 helper header
│
├── Assets
│   ├── arial24.spritefont          # Small font (24pt) for labels
│   └── arial120.spritefont         # Large font (120pt) for number
│
├── Dependencies
│   └── directxtk12_desktop_2019.2025.7.10.1/   # DirectXTK12 NuGet package
│       ├── include/                # Headers
│       └── native/lib/x64/Release/ # Prebuilt libraries
│
├── Build Output
│   └── build/
│       ├── Release/
│       │   ├── GraphicsEngine.exe  # Main executable
│       │   ├── arial24.spritefont  # Font files (copied)
│       │   ├── arial120.spritefont
│       │   └── graphics_engine_log.txt
│       └── [CMake generated files]
│
└── Documentation
    ├── ARCHITECTURE.md             # Architecture overview
    ├── FILE_STRUCTURE.md           # This file
    └── README.md                   # (optional) Getting started guide
```

## File Responsibilities

### Core Framework Files

#### `main.cpp` - Application Entry & Window Management
- **Purpose**: Win32 window creation and message loop
- **Responsibilities**:
  - Window creation and management
  - Message handling (WM_PAINT, WM_KEYDOWN, etc.)
  - Renderer switching (G/D keys)
  - Command line parsing
  - Timer management for GDI mode
- **Dependencies**: Engine, GDIRenderer, DX12Renderer, Logger

#### `Engine.h` / `Engine.cpp` - Application Logic Layer
- **Purpose**: Manages application state and coordinates rendering
- **Responsibilities**:
  - Random number generation (0-9999)
  - Update timing (5 second intervals)
  - Renderer lifecycle management
  - Scene composition (what to draw, where)
  - Renderer switching at runtime
- **Key Methods**:
  ```cpp
  void Initialize(HWND hwnd, std::unique_ptr<IRenderer> renderer)
  void Update()              // Update random number every 5 seconds
  void Render()              // Compose and draw the scene
  void SwitchRenderer(...)   // Switch renderer at runtime
  ```
- **Dependencies**: IRenderer

#### `IRenderer.h` - Rendering Interface
- **Purpose**: Pure abstract interface for all renderers
- **Responsibilities**:
  - Define rendering contract
  - No implementation, no logic
- **Key Methods**:
  ```cpp
  virtual void Initialize(HWND, UINT width, UINT height) = 0
  virtual void BeginFrame() = 0
  virtual void Clear(float r, float g, float b) = 0
  virtual void DrawText(...) = 0
  virtual void MeasureText(...) = 0
  virtual void EndFrame() = 0
  virtual void OnDestroy() = 0
  virtual const char* GetName() const = 0
  ```
- **Dependencies**: None (Windows.h only)

#### `Logger.h` - Logging Utilities
- **Purpose**: Debug logging to file and OutputDebugString
- **Responsibilities**:
  - Write to graphics_engine_log.txt
  - Show message boxes for errors
  - OutputDebugString for VS debugger
- **Dependencies**: None

### Renderer Implementation Files

#### `GDIRenderer.h` / `GDIRenderer.cpp` - GDI Backend
- **Purpose**: Software rendering using Windows GDI
- **Technology**:
  - GDI (Graphics Device Interface)
  - Double buffering with memory DC
  - GradientFill for backgrounds
  - CreateFont/DrawText for text
- **Features**:
  - Gradient backgrounds
  - Anti-aliased text
  - Variable font sizes
  - Immediate mode rendering
- **Dependencies**: IRenderer, wingdi.h

#### `DX12Renderer.h` / `DX12Renderer.cpp` - DirectX 12 Backend
- **Purpose**: Hardware-accelerated rendering using DirectX 12
- **Technology**:
  - DirectX 12 pipeline
  - DirectXTK12 for 2D rendering
  - SpriteBatch for text
  - SpriteFont for bitmap fonts
  - Command lists and synchronization
- **Features**:
  - GPU-accelerated rendering
  - Sprite font rendering
  - Descriptor heaps
  - Double buffering with swap chain
- **Dependencies**: IRenderer, DirectXTK12, d3d12.h, dxgi.h

### Legacy Files (Not Compiled)

These files represent the old monolithic architecture where rendering and logic were combined:

- `IGraphicsEngine.h` - Old interface (logic + rendering)
- `GDIEngine.h/.cpp` - Old GDI implementation
- `DX12Engine.h/.cpp` - Old DX12 implementation

**Keep for reference but exclude from CMakeLists.txt**

## Dependency Flow

```
main.cpp
  ↓
  ├─→ Engine (owns application logic)
  │     ↓
  │     └─→ IRenderer (interface)
  │           ↓
  │           ├─→ GDIRenderer (implementation)
  │           └─→ DX12Renderer (implementation)
  │                 ↓
  │                 └─→ DirectXTK12 (library)
  └─→ Logger
```

## Build System

### `CMakeLists.txt` Structure
```cmake
# 1. Project setup
cmake_minimum_required(VERSION 3.15)
project(GraphicsEngine)

# 2. DirectXTK12 from NuGet
set(DIRECTXTK12_DIR "${CMAKE_SOURCE_DIR}/directxtk12_desktop_2019.2025.7.10.1")
add_library(DirectXTK12 STATIC IMPORTED)

# 3. Main executable
add_executable(GraphicsEngine
    main.cpp
    Engine.cpp Engine.h
    IRenderer.h
    GDIRenderer.cpp GDIRenderer.h
    DX12Renderer.cpp DX12Renderer.h
    Logger.h
)

# 4. Link libraries
target_link_libraries(GraphicsEngine
    DirectXTK12
    d3d12.lib dxgi.lib d3dcompiler.lib
    gdi32.lib user32.lib kernel32.lib msimg32.lib
)
```

## Runtime Files

### Files Required in Executable Directory
```
build/Release/
├── GraphicsEngine.exe      # Main executable
├── arial24.spritefont      # Required for DirectX renderer
└── arial120.spritefont     # Required for DirectX renderer
```

**Note**: Font files must be in same directory as .exe

## Class Relationships

```
┌─────────────────────────────────────────┐
│            main.cpp                     │
│  (Window, Message Loop, User Input)     │
└─────────────────┬───────────────────────┘
                  │
                  │ creates & owns
                  ↓
         ┌─────────────────┐
         │     Engine      │
         │  (Logic Only)   │
         └────────┬────────┘
                  │
                  │ uses (composition)
                  ↓
         ┌─────────────────┐
         │   IRenderer     │
         │  <<interface>>  │
         └────────┬────────┘
                  │
          ┌───────┴───────┐
          │               │
          ↓               ↓
  ┌──────────────┐  ┌──────────────┐
  │ GDIRenderer  │  │DX12Renderer  │
  │(Implements)  │  │(Implements)  │
  └──────────────┘  └──────┬───────┘
                           │
                           │ uses
                           ↓
                    ┌──────────────┐
                    │DirectXTK12   │
                    │  (Library)   │
                    └──────────────┘
```

## Adding a New Renderer

To add a new renderer (e.g., Vulkan):

1. **Create header**: `VulkanRenderer.h`
   - Inherit from `IRenderer`
   - Declare all pure virtual methods

2. **Create implementation**: `VulkanRenderer.cpp`
   - Implement all IRenderer methods
   - Initialize Vulkan pipeline in `Initialize()`
   - Handle rendering in `BeginFrame/Clear/DrawText/EndFrame`

3. **Update CMakeLists.txt**:
   ```cmake
   add_executable(GraphicsEngine
       ...
       VulkanRenderer.cpp VulkanRenderer.h
   )
   target_link_libraries(GraphicsEngine
       ...
       vulkan-1.lib  # Add Vulkan library
   )
   ```

4. **Update main.cpp**:
   ```cpp
   #include "VulkanRenderer.h"

   case RendererType::Vulkan:
       return std::make_unique<VulkanRenderer>();
   ```

That's it! The Engine class doesn't need any changes.

## Build Commands

```bash
# Configure
cd build
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release

# Run
cd Release
./GraphicsEngine.exe

# Run with specific renderer
./GraphicsEngine.exe -gdi    # Start with GDI
./GraphicsEngine.exe -dx12   # Start with DirectX 12
```

## Key Design Patterns

1. **Interface Segregation**: IRenderer defines only rendering operations
2. **Dependency Inversion**: Engine depends on IRenderer abstraction, not concrete implementations
3. **Strategy Pattern**: Renderers are interchangeable strategies for drawing
4. **Single Responsibility**: Each class has one clear purpose
5. **Composition over Inheritance**: Engine uses IRenderer via composition

## Notes

- Font files (`.spritefont`) are binary and should not be edited
- Logger creates `graphics_engine_log.txt` automatically
- DirectXTK12 is linked statically from NuGet package
- Old legacy files kept for reference but not compiled
